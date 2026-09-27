
#include <loquaty.h>
#include "EntisGLS4_ImageComposition_Layer.h"

using namespace Loquaty ;


// Layer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ImageComposition_Layer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// Layer( const EntisGLS4.ImageComposition.Layer layer )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_ImageComposition_Layer,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, layer ) ;
	LQT_VERIFY_NULL_PTR( layer ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_ImageComposition_Layer>( /* construction-arg-list */ ) ) ;

	LQT_RETURN_VOID() ;
}

// ulong getTotalLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getTotalLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getTotalLayerCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getLayerName( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getLayerName(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// void setLayerName( String name )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_STRING( name ) ;

	// pThis->setLayerName(...) ;

	LQT_RETURN_VOID() ;
}

// PSD.LayerType getLayerType( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getLayerType(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setLayerType( PSD.LayerType type )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_UINT( type ) ;

	// pThis->setLayerType(...) ;

	LQT_RETURN_VOID() ;
}

// Point* getLayerPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LPoint	valRet ;
	// valRet = pThis->getLayerPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLayerPosition( const Point* pos )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LPoint, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;

	// pThis->setLayerPosition(...) ;

	LQT_RETURN_VOID() ;
}

// uint getLayerBlendMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerBlendMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getLayerBlendMode(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setLayerBlendMode( uint blend )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerBlendMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_UINT( blend ) ;

	// pThis->setLayerBlendMode(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isLayerVisible( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_isLayerVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isLayerVisible(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setLayerVisible( boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	// pThis->setLayerVisible(...) ;

	LQT_RETURN_VOID() ;
}

// uint getLayerTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getLayerTransparency(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setLayerTransparency( uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	// pThis->setLayerTransparency(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getSubLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getSubLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getSubLayerCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.ImageComposition.Layer getSubLayerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getSubLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ImageComposition.Layer) ) ) ;
	// valRet = pThis->getSubLayerAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ImageComposition_Layer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ImageComposition_Layer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean layerSizeOf( ImageRect* rect, int threshold ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_layerSizeOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rect ) ;
	LQT_VERIFY_NULL_PTR( rect ) ;
	LQT_FUNC_ARG_INT( threshold ) ;

	LBoolean	valRet ;
	// valRet = pThis->layerSizeOf(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



