
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_ImageComposition_Layer.h>


// Layer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ImageComposition_Layer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_ImageComposition_Layer,
			pThis, (new SSmartObject( new SGLImageComposition::Layer )) ) ;

	LQT_RETURN_VOID() ;
}

// Layer( const EntisGLS4.ImageComposition.Layer layer )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_ImageComposition_Layer,1)
{
	LQT_FUNC_ARG_LIST ;

	LObjPtr	pLayerObj = LQT_ARG_OBJECT( 1 ) ;
	std::shared_ptr<LEntisGLS4_ImageComposition_Layer>
		pLayerRef = LNativeObj::GetNative<LEntisGLS4_ImageComposition_Layer>( pLayerObj.Ptr() ) ;
	LQT_VERIFY_NULL_PTR( pLayerRef ) ;
	SGLImageComposition::Layer *	pLayer = pLayerRef->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LEntisGLS4_ImageComposition_Layer	refLayer ;
	refLayer.SetSmartReference( new SGLImageComposition::Layer( *pLayer ) ) ;

	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis, (refLayer) ) ;

	LQT_RETURN_VOID() ;
}

// ulong getTotalLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getTotalLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LQT_RETURN_ULONG( pLayer->GetTotalLayerCount() ) ;
}

// String getLayerName( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LQT_RETURN_STRING( pLayer->m_name ) ;
}

// void setLayerName( String name )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_STRING( name ) ;

	pLayer->m_name = name.c_str() ;

	LQT_RETURN_VOID() ;
}

// uint getLayerType( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LQT_RETURN_UINT( pLayer->m_type ) ;
}

// void setLayerType( uint type )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_UINT( type ) ;

	pLayer->m_type = (PSD::LayerRecord::LayerType) type ;

	LQT_RETURN_VOID() ;
}

// GenVector2<int,long>* getLayerPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	SGLPoint	valRet = pLayer->m_position ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setLayerPosition( const GenVector2<int,long>* pos )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_STRUCT( SGLPoint, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;

	pLayer->m_position = *pos ;

	LQT_RETURN_VOID() ;
}

// uint getLayerBlendMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerBlendMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LQT_RETURN_UINT( pLayer->m_blend ) ;
}

// void setLayerBlendMode( uint blend )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerBlendMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_UINT( blend ) ;

	pLayer->m_blend = blend ;

	LQT_RETURN_VOID() ;
}

// boolean isLayerVisible( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_isLayerVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LQT_RETURN_BOOL( pLayer->m_visible ) ;
}

// void setLayerVisible( boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	pLayer->m_visible = visible ;

	LQT_RETURN_VOID() ;
}

// uint getLayerTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getLayerTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LQT_RETURN_UINT( pLayer->m_transparency ) ;
}

// void setLayerTransparency( uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_setLayerTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	pLayer->m_transparency = transparency ;

	LQT_RETURN_VOID() ;
}

// ulong getSubLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getSubLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LQT_RETURN_ULONG( pLayer->m_layers.GetLength() ) ;
}

// EntisGLS4.ImageComposition.Layer getSubLayerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_getSubLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	SGLImageComposition::Layer *	pSub = pLayer->m_layers.GetAt( (size_t) index ) ;
	if ( pSub == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ImageComposition.Layer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ImageComposition_Layer>(pSub) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean layerSizeOf( ImageRect* rect, int threshold ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_Layer_layerSizeOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition_Layer, pThis ) ;
	SGLImageComposition::Layer *	pLayer = pThis->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rect ) ;
	LQT_VERIFY_NULL_PTR( rect ) ;
	LQT_FUNC_ARG_INT( threshold ) ;

	SGLRect		rectLayer ;
	LBoolean	valRet =
		SGLImageComposition::LayerSizeOf( rectLayer, *pLayer, threshold ) ;
	*rect = rectLayer ;

	LQT_RETURN_BOOL( valRet ) ;
}



