
#include <loquaty.h>
#include "EntisGLS4_ImageComposition.h"

using namespace Loquaty ;


// ImageComposition( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ImageComposition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_ImageComposition, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// Size* getCanvasSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getCanvasSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;

	LSize	valRet ;
	// valRet = pThis->getCanvasSize(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// PSD.ImageMode getImageMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getImageMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getImageMode(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint getChannelCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getChannelCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getChannelCount(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setCanvasInfo( const Size* size, uint channels, PSD.ImageMode mode )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_setCanvasInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LSize, size ) ;
	LQT_VERIFY_NULL_PTR( size ) ;
	LQT_FUNC_ARG_UINT( channels ) ;
	LQT_FUNC_ARG_UINT( mode ) ;

	// pThis->setCanvasInfo(...) ;

	LQT_RETURN_VOID() ;
}

// boolean loadPSDFile( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_loadPSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadPSDFile(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean readPSDFile( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_readPSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->readPSDFile(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean savePSDFile( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_savePSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LBoolean	valRet ;
	// valRet = pThis->savePSDFile(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean writePSDFile( File file )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_writePSDFile)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;

	LBoolean	valRet ;
	// valRet = pThis->writePSDFile(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong getLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getLayerCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.ImageComposition.Layer getLayerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ImageComposition.Layer) ) ) ;
	// valRet = pThis->getLayerAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ImageComposition_Layer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ImageComposition_Layer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// ulong getTreeLayerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getTreeLayerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getTreeLayerCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.ImageComposition.Layer getTreeLayerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_getTreeLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ImageComposition.Layer) ) ) ;
	// valRet = pThis->getTreeLayerAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ImageComposition_Layer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ImageComposition_Layer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void insertLayerAt( ulong index, EntisGLS4.ImageComposition.Layer layer, EntisGLS4.ImageComposition.Layer parent )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_insertLayerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, layer ) ;
	LQT_VERIFY_NULL_PTR( layer ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, parent ) ;
	LQT_VERIFY_NULL_PTR( parent ) ;

	// pThis->insertLayerAt(...) ;

	LQT_RETURN_VOID() ;
}

// void appendLayer( EntisGLS4.ImageComposition.Layer layer, EntisGLS4.ImageComposition.Layer parent )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_appendLayer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, layer ) ;
	LQT_VERIFY_NULL_PTR( layer ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, parent ) ;
	LQT_VERIFY_NULL_PTR( parent ) ;

	// pThis->appendLayer(...) ;

	LQT_RETURN_VOID() ;
}

// long localIndexToGlobal( EntisGLS4.ImageComposition.Layer parent, ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_localIndexToGlobal)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, parent ) ;
	LQT_VERIFY_NULL_PTR( parent ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LInt64	valRet ;
	// valRet = pThis->localIndexToGlobal(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// boolean removeLayer( EntisGLS4.ImageComposition.Layer layer )
IMPL_LOQUATY_FUNC(EntisGLS4_ImageComposition_removeLayer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ImageComposition, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ImageComposition_Layer, layer ) ;
	LQT_VERIFY_NULL_PTR( layer ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeLayer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



