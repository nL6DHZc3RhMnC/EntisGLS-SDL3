
#include <loquaty.h>
#include "EntisGLS4_PaintContext.h"

using namespace Loquaty ;


// PaintContext( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_PaintContext)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_PaintContext, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getTargetImage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getTargetImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getTargetImage(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Image getTargetZBuffer( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getTargetZBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getTargetZBuffer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean getViewPort( ImageRect* rectView ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getViewPort)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectView ) ;
	LQT_VERIFY_NULL_PTR( rectView ) ;

	LBoolean	valRet ;
	// valRet = pThis->getViewPort(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean attachTargetImage( EntisGLS4.Image image, EntisGLS4.Image zbuf, const ImageRect* pView )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_attachTargetImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, zbuf ) ;
	LQT_VERIFY_NULL_PTR( zbuf ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pView ) ;
	LQT_VERIFY_NULL_PTR( pView ) ;

	LBoolean	valRet ;
	// valRet = pThis->attachTargetImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean detachTargetImage( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_detachTargetImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->detachTargetImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean appendTransformation( const Affine* affine, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_appendTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LAffine, affine ) ;
	LQT_VERIFY_NULL_PTR( affine ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	LBoolean	valRet ;
	// valRet = pThis->appendTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setTransformation( const Affine* affine, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LAffine, affine ) ;
	LQT_VERIFY_NULL_PTR( affine ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	LBoolean	valRet ;
	// valRet = pThis->setTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean currentAffine( Affine* affine ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_currentAffine)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LAffine, affine ) ;
	LQT_VERIFY_NULL_PTR( affine ) ;

	LBoolean	valRet ;
	// valRet = pThis->currentAffine(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// uint currentTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_currentTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->currentTransparency(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// boolean pushTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_pushTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->pushTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean popTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_popTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->popTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean resetTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_resetTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->resetTransformation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setPaintFlags( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setPaintFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	// pThis->setPaintFlags(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getPaintFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getPaintFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getPaintFlags(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean fillClearTarget( uint argb, EntisGLS4.PaintContext.ClearTargetFlag flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillClearTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->fillClearTarget(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean fillRectangle( int x, int y, int width, int height, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_INT( x ) ;
	LQT_FUNC_ARG_INT( y ) ;
	LQT_FUNC_ARG_INT( width ) ;
	LQT_FUNC_ARG_INT( height ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->fillRectangle(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean fillPolygon( const Vector2* vertices, uint count, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillPolygon)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, vertices ) ;
	LQT_VERIFY_NULL_PTR( vertices ) ;
	LQT_FUNC_ARG_UINT( count ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->fillPolygon(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawImage( const EntisGLS4.PaintParam param, EntisGLS4.Image pSrcImage, const ImageRect* pSrcClip )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_PaintParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcClip ) ;
	LQT_VERIFY_NULL_PTR( pSrcClip ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawMesh( const Vector2* pDstMesh, const Vector2* pSrcMesh, ulong widthMesh, ulong heightMesh, const EntisGLS4.PaintParam param, EntisGLS4.Image pSrcImage, const ImageRect* pSrcClip )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pDstMesh ) ;
	LQT_VERIFY_NULL_PTR( pDstMesh ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pSrcMesh ) ;
	LQT_VERIFY_NULL_PTR( pSrcMesh ) ;
	LQT_FUNC_ARG_ULONG( widthMesh ) ;
	LQT_FUNC_ARG_ULONG( heightMesh ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_PaintParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcClip ) ;
	LQT_VERIFY_NULL_PTR( pSrcClip ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawMesh(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean flush( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_flush)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->flush(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean finish( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_finish)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->finish(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawPoints( const Vector2* pPoints, ulong nPoints, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawPoints)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pPoints ) ;
	LQT_VERIFY_NULL_PTR( pPoints ) ;
	LQT_FUNC_ARG_ULONG( nPoints ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawPoints(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawThinLine( int x0, int y0, int x1, int y1, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawThinLine)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_INT( x0 ) ;
	LQT_FUNC_ARG_INT( y0 ) ;
	LQT_FUNC_ARG_INT( x1 ) ;
	LQT_FUNC_ARG_INT( y1 ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawThinLine(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawThinLines( const Vector2* pLines, ulong nLines, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawThinLines)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pLines ) ;
	LQT_VERIFY_NULL_PTR( pLines ) ;
	LQT_FUNC_ARG_ULONG( nLines ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawThinLines(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawEllipse( float xCenter, float yCenter, float width, float height, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawEllipse)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawEllipse(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawArc( float xCenter, float yCenter, float width, float height, float radFirst, float radEnd, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawArc)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_FLOAT( radFirst ) ;
	LQT_FUNC_ARG_FLOAT( radEnd ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawArc(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean drawBezier( const Vector2* pPoints, ulong nPoints, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawBezier)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pPoints ) ;
	LQT_VERIFY_NULL_PTR( pPoints ) ;
	LQT_FUNC_ARG_ULONG( nPoints ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->drawBezier(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean fillEllipse( float xCenter, float yCenter, float width, float height, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillEllipse)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->fillEllipse(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean fillArc( float xCenter, float yCenter, float width, float height, float radFirst, float radEnd, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillArc)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_FLOAT( radFirst ) ;
	LQT_FUNC_ARG_FLOAT( radEnd ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->fillArc(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void freeGradation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_freeGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;

	// pThis->freeGradation(...) ;

	LQT_RETURN_VOID() ;
}

// boolean setLinearGradation( float x0, float y0, float x1, float y1, const ARGB8* pGradation, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setLinearGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_FLOAT( x0 ) ;
	LQT_FUNC_ARG_FLOAT( y0 ) ;
	LQT_FUNC_ARG_FLOAT( x1 ) ;
	LQT_FUNC_ARG_FLOAT( y1 ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, pGradation ) ;
	LQT_VERIFY_NULL_PTR( pGradation ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LBoolean	valRet ;
	// valRet = pThis->setLinearGradation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setRingedGradation( float xCenter, float yCenter, float radAngle, const ARGB8* pGradation, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setRingedGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( radAngle ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, pGradation ) ;
	LQT_VERIFY_NULL_PTR( pGradation ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LBoolean	valRet ;
	// valRet = pThis->setRingedGradation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



