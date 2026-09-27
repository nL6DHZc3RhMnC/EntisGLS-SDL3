
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Material.h>


// Material( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Material)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_Material,
			pThis, (new SSmartObject( new S3DMaterial )) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SurfaceAttribute* faceAttr( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_faceAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	LQT_RETURN_POINTER_BUF
		( std::make_shared<LArrayBufAlias>
			( (uint8_t*) &(pMaterial->m_attrSurface),
							sizeof(pMaterial->m_attrSurface) ) ) ;
}

// EntisGLS4.SurfaceAttribute* backAttr( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_backAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	LQT_RETURN_POINTER_BUF
		( std::make_shared<LArrayBufAlias>
			( (uint8_t*) &(pMaterial->m_attrBack),
							sizeof(pMaterial->m_attrBack) ) ) ;
}

// boolean isEnabledBackAttr( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_isEnabledBackAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	LQT_RETURN_BOOL( pMaterial->IsEnabledBackSurfaceAttribute() ) ;
}

// void enableBackAttr( boolean enableBack )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_enableBackAttr)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_BOOL( enableBack ) ;

	pMaterial->EnableBackSurfaceAttribute( enableBack ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getTexture( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	SGLImageObject *	pImage = pMaterial->GetTexture( index ) ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>( pImage ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Image getBackTexture( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	SGLImageObject *	pImage = pMaterial->GetBackTexture( index ) ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>( pImage ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// uint getTextureType( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTextureType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LQT_RETURN_UINT( pMaterial->GetTextureType( index ) ) ;
}

// uint getBackTextureType( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTextureType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LQT_RETURN_UINT( pMaterial->GetBackTextureType( index ) ) ;
}

// float getTextureApply( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTextureApply)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LQT_RETURN_FLOAT( pMaterial->GetTextureApplication( index ) ) ;
}

// float getBackTextureApply( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTextureApply)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LQT_RETURN_FLOAT( pMaterial->GetBackTextureApplication( index ) ) ;
}

// float getTextureParam( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getTextureParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LQT_RETURN_FLOAT( pMaterial->GetTextureParameter( index ) ) ;
}

// float getBackTextureParam( int index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_getBackTextureParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( index ) ;

	LQT_RETURN_FLOAT( pMaterial->GetBackTextureParameter( index ) ) ;
}

// int findTextureTypeOf( int type ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_findTextureTypeOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( type ) ;

	LQT_RETURN_INT( pMaterial->FindTextureTypeOf( type ) ) ;
}

// int findBackTextureTypeOf( int type ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Material_findBackTextureTypeOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_INT( type ) ;

	LQT_RETURN_INT( pMaterial->FindBackTextureTypeOf( type ) ) ;
}

// void setTexture( EntisGLS4.Image image, int index, int type, float apply, float param )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_setTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	SGLImageObject *	pImage = nullptr ;
	if ( image != nullptr )
	{
		pImage = image->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_INT( index ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_FLOAT( apply ) ;
	LQT_FUNC_ARG_FLOAT( param ) ;

	pMaterial->SetTexture( pImage, index, type, apply, param ) ;

	LQT_RETURN_VOID() ;
}

// void setBackTexture( EntisGLS4.Image image, int index, int type, float apply, float param )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_setBackTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	SGLImageObject *	pImage = nullptr ;
	if ( image != nullptr )
	{
		pImage = image->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_INT( index ) ;
	LQT_FUNC_ARG_INT( type ) ;
	LQT_FUNC_ARG_FLOAT( apply ) ;
	LQT_FUNC_ARG_FLOAT( param ) ;

	pMaterial->SetBackTexture( pImage, index, type, apply, param ) ;

	LQT_RETURN_VOID() ;
}

// void updateBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_Material_updateBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Material, pThis ) ;
	S3DMaterial *	pMaterial = pThis->GetRef<S3DMaterial>() ;
	LQT_VERIFY_NULL_PTR( pMaterial ) ;

	pMaterial->SetUpdateMaterialBuffer() ;

	LQT_RETURN_VOID() ;
}



