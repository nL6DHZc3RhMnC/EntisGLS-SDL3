
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_ImageComposition.h>


// ImageComposition( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ImageComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_ImageComposition,
			pThis, (new SSmartObject( new SGLImageComposition )) ) ;

	LQT_RETURN_VOID() ;
}

// GenVector2<int,long>* getCanvasSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getCanvasSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;

	SGLSize	valRet = pImageComp->GetCanvasSize() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// uint getImageMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getImageMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;

	LQT_RETURN_UINT( pImageComp->GetImageMode() ) ;
}

// uint getChannelCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getChannelCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;

	LQT_RETURN_UINT( pImageComp->GetChannelCount() ) ;
}

// void setCanvasInfo( const GenVector2<int,long>* size, uint channels, uint mode )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_setCanvasInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_STRUCT( SGLSize, size ) ;
	LQT_VERIFY_NULL_PTR( size ) ;
	LQT_FUNC_ARG_UINT( channels ) ;
	LQT_FUNC_ARG_UINT( mode ) ;

	pImageComp->SetCanvasInfo( *size, channels, (PSD::ImageMode) mode ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadPSDFile( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_loadPSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LQT_RETURN_BOOL( pImageComp->LoadPSDFile( path.c_str() ) == sglErrSuccess ) ;
}

// boolean readPSDFile( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_readPSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, pFile ) ;
	LQT_VERIFY_NULL_PTR( pFile ) ;

	SLoquatyFile	file( pFile ) ;
	LQT_RETURN_BOOL( pImageComp->ReadPSDFile( file ) == sglErrSuccess ) ;
}

// boolean savePSDFile( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_savePSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LQT_RETURN_BOOL( pImageComp->SavePSDFile( path.c_str() ) == sglErrSuccess ) ;
}

// boolean writePSDFile( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_writePSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, pFile ) ;
	LQT_VERIFY_NULL_PTR( pFile ) ;

	SLoquatyFile	file( pFile ) ;
	LQT_RETURN_BOOL( pImageComp->WritePSDFile( file ) == sglErrSuccess ) ;
}

// ulong getLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;

	LQT_RETURN_ULONG( pImageComp->GetLayerCount() ) ;
}

// EntisGLS4.ImageComposition.Layer getLayerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	SGLImageComposition::Layer *
			pLayer = pImageComp->GetLayerAt( (size_t) index ) ;
	if ( pLayer == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ImageComposition.Layer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ImageComposition_Layer>(pLayer) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getTreeLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getTreeLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;

	LQT_RETURN_ULONG( pImageComp->GetTreeLayerCount() ) ;
}

// EntisGLS4.ImageComposition.Layer getTreeLayerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getTreeLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	SGLImageComposition::Layer *
			pLayer = pImageComp->GetTreeLayerAt( (size_t) index ) ;
	if ( pLayer == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ImageComposition.Layer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ImageComposition_Layer>(pLayer) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void insertLayerAt( ulong index, EntisGLS4.ImageComposition.Layer layer, EntisGLS4.ImageComposition.Layer parent )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_insertLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, layer ) ;
	LQT_VERIFY_NULL_PTR( layer ) ;
	SGLImageComposition::Layer *	pLayer = layer->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, parent ) ;
	SGLImageComposition::Layer *	pParent = nullptr ;
	if ( parent != nullptr )
	{
		pParent = parent->GetRef<SGLImageComposition::Layer>() ;
	}
	if ( layer->IsSmartReference() )
	{
		layer->DetachReference() ;
		layer->SetReference( pLayer ) ;

		pImageComp->InsertLayerAt( (size_t) index, pLayer, pParent ) ;
	}
	LQT_RETURN_VOID() ;
}

// void appendLayer( EntisGLS4.ImageComposition.Layer layer, EntisGLS4.ImageComposition.Layer parent )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_appendLayer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, layer ) ;
	LQT_VERIFY_NULL_PTR( layer ) ;
	SGLImageComposition::Layer *	pLayer = layer->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, parent ) ;
	SGLImageComposition::Layer *	pParent = nullptr ;
	if ( parent != nullptr )
	{
		pParent = parent->GetRef<SGLImageComposition::Layer>() ;
	}
	if ( layer->IsSmartReference() )
	{
		layer->DetachReference() ;
		layer->SetReference( pLayer ) ;

		pImageComp->AppendLayer( pLayer, pParent ) ;
	}
	LQT_RETURN_VOID() ;
}

// long localIndexToGlobal( EntisGLS4.ImageComposition.Layer parent, ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_localIndexToGlobal)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, parent ) ;
	SGLImageComposition::Layer *	pParent = nullptr ;
	if ( parent != nullptr )
	{
		pParent = parent->GetRef<SGLImageComposition::Layer>() ;
	}
	LQT_FUNC_ARG_ULONG( index ) ;

	LQT_RETURN_LONG
		( pImageComp->LocalIndexToGlobal( pParent, (size_t) index ) ) ;
}

// boolean removeLayer( EntisGLS4.ImageComposition.Layer layer )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_removeLayer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	SGLImageComposition *	pImageComp = pThis->GetRef<SGLImageComposition>() ;
	LQT_VERIFY_NULL_PTR( pImageComp ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, layer ) ;
	LQT_VERIFY_NULL_PTR( layer ) ;
	SGLImageComposition::Layer *	pLayer = layer->GetRef<SGLImageComposition::Layer>() ;
	LQT_VERIFY_NULL_PTR( pLayer ) ;

	LBoolean	valRet = (pImageComp->DetachLayer( pLayer ) == sglErrSuccess) ;
	if ( valRet )
	{
		layer->SetSmartReference( pLayer ) ;
	}

	LQT_RETURN_BOOL( valRet ) ;
}



