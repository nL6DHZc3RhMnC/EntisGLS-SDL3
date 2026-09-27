
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_PaintContext.h>


// PaintContext( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_PaintContext)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_PaintContext, pThis,
			( new SSmartObject( (SGLPaintContextInterface*) new SGLPaintBuffer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getTargetImage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getTargetImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	SGLImageObject *	pImage = pPaint->GetTargetImage() ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>(pImage) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Image getTargetZBuffer( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getTargetZBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	SGLImageObject *	pZBuffer = pPaint->GetTargetZBuffer() ;
	if ( pZBuffer == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>(pZBuffer) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean getViewPort( ImageRect* rectView ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getViewPort)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectView ) ;
	LQT_VERIFY_NULL_PTR( rectView ) ;

	LQT_RETURN_BOOL( pPaint->GetViewPort( *rectView ) == sglErrSuccess ) ;
}

// boolean attachTargetImage( EntisGLS4.Image image, EntisGLS4.Image zbuf, const ImageRect* pView )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_attachTargetImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	SGLImageObject *	pImage = image->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, zbuf ) ;
	SGLImageObject *	pZBuffer = nullptr ;
	if ( zbuf != nullptr )
	{
		pZBuffer = zbuf->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_STRUCT( LImageRect, pView ) ;

	LQT_RETURN_BOOL
		( pPaint->AttachTargetImage( pImage, pZBuffer, pView ) == sglErrSuccess ) ;
}

// boolean detachTargetImage( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_detachTargetImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_BOOL( pPaint->DetachTargetImage() == sglErrSuccess ) ;
}

// boolean appendTransformation( const Affine* affine, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_appendTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_STRUCT( LAffine, affine ) ;
	LQT_VERIFY_NULL_PTR( affine ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	SGLAffine	sglAffine = affine->ToSGLAffine() ;

	LQT_RETURN_BOOL
		( pPaint->AppendTransformation( sglAffine, transparency ) == sglErrSuccess ) ;
}

// boolean setTransformation( const Affine* affine, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_STRUCT( LAffine, affine ) ;
	LQT_VERIFY_NULL_PTR( affine ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	SGLAffine	sglAffine = affine->ToSGLAffine() ;

	LQT_RETURN_BOOL
		( pPaint->SetTransformation( sglAffine, transparency ) == sglErrSuccess ) ;
}

// boolean currentAffine( Affine* affine ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_currentAffine)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_STRUCT( LAffine, affine ) ;
	LQT_VERIFY_NULL_PTR( affine ) ;

	SGLAffine	sglAffine ;
	LBoolean	valRet = (pPaint->CurrentAffine( sglAffine ) == sglErrSuccess) ;
	*affine = sglAffine ;

	LQT_RETURN_BOOL( valRet ) ;
}

// uint currentTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_currentTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_UINT( pPaint->CurrentTransparency() ) ;
}

// boolean pushTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_pushTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_BOOL( pPaint->PushTransformation() == sglErrSuccess ) ;
}

// boolean popTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_popTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_BOOL( pPaint->PopTransformation() == sglErrSuccess ) ;
}

// boolean resetTransformation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_resetTransformation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_BOOL( pPaint->ResetTransformation() == sglErrSuccess ) ;
}

// void setPaintFlags( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setPaintFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	pPaint->SetPaintFlags( nFlags ) ;

	LQT_RETURN_VOID() ;
}

// ulong getPaintFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_getPaintFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_ULONG( pPaint->GetPaintFlags() ) ;
}

// boolean fillClearTarget( uint argb, ulong flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillClearTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_ULONG( flags ) ;

	LQT_RETURN_BOOL( pPaint->FillClearTarget( argb, flags ) == sglErrSuccess ) ;
}

// boolean fillRectangle( int x, int y, int width, int height, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_INT( x ) ;
	LQT_FUNC_ARG_INT( y ) ;
	LQT_FUNC_ARG_INT( width ) ;
	LQT_FUNC_ARG_INT( height ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pPaint->FillRectangle
			( x, y, width, height, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean fillPolygon( const Vector2* vertices, uint count, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillPolygon)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, vertices, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( vertices ) ;
	LQT_FUNC_ARG_UINT( count ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pPaint->FillPolygon
			( vertices, count, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean drawImage( const EntisGLS4.PaintParam param, EntisGLS4.Image pSrcImage, const ImageRect* pSrcClip )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_PaintParam, pParam ) ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcClip ) ;

	SGLPaintParam	param ;
	SGLAffine		affine ;
	GetLPaintParam( param, affine, pParam ) ;

	LQT_RETURN_BOOL
		( pPaint->DrawImage( param, pSrcImage, pSrcClip ) == sglErrSuccess ) ;
}

// boolean drawMesh( const Vector2* pDstMesh, const Vector2* pSrcMesh, ulong widthMesh, ulong heightMesh, const EntisGLS4.PaintParam param, EntisGLS4.Image pSrcImage, const ImageRect* pSrcClip )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawMesh)
{
	LQT_FUNC_ARG_LIST ;
	const size_t	nMeshArrayLength =
						(size_t) ((LQT_ARG_LONG(3) + 1) * (LQT_ARG_LONG(4) + 1)) ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, pDstMesh, nMeshArrayLength ) ;
	LQT_VERIFY_NULL_PTR( pDstMesh ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, pSrcMesh, nMeshArrayLength ) ;
	LQT_VERIFY_NULL_PTR( pSrcMesh ) ;
	LQT_FUNC_ARG_ULONG( widthMesh ) ;
	LQT_FUNC_ARG_ULONG( heightMesh ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_PaintParam, pParam ) ;
	LQT_VERIFY_NULL_PTR( pParam ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcClip ) ;

	SGLPaintParam	param ;
	SGLAffine		affine ;
	GetLPaintParam( param, affine, pParam ) ;

	LQT_RETURN_BOOL
		( pPaint->DrawMesh
			( pDstMesh, pSrcMesh,
				(size_t) widthMesh, (size_t) heightMesh,
				param, pSrcImage, pSrcClip ) == sglErrSuccess ) ;
}

// boolean flush( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_flush)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_BOOL( pPaint->Flush() == sglErrSuccess ) ;
}

// boolean finish( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_finish)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLPaintContextInterface *	pPaint = pThis->GetRef<SGLPaintContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pPaint ) ;

	LQT_RETURN_BOOL( pPaint->Finish() == sglErrSuccess ) ;
}

// boolean drawPoints( const Vector2* pPoints, ulong nPoints, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawPoints)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_STRUCT_N( LVector2, pPoints, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pPoints ) ;
	LQT_FUNC_ARG_ULONG( nPoints ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->DrawPoints
			( pPoints, (size_t) nPoints, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean drawThinLine( int x0, int y0, int x1, int y1, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawThinLine)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_INT( x0 ) ;
	LQT_FUNC_ARG_INT( y0 ) ;
	LQT_FUNC_ARG_INT( x1 ) ;
	LQT_FUNC_ARG_INT( y1 ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->DrawThinLine
			( x0, y0, x1, y1, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean drawThinLines( const Vector2* pLines, ulong nLines, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawThinLines)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_STRUCT_N( LVector2, pLines, LQT_ARG_LONG(2)+1 ) ;
	LQT_VERIFY_NULL_PTR( pLines ) ;
	LQT_FUNC_ARG_ULONG( nLines ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->DrawThinLines
			( pLines, (size_t) nLines, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean drawEllipse( float xCenter, float yCenter, float width, float height, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawEllipse)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->DrawEllipse
			( xCenter, yCenter, width, height, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean drawArc( float xCenter, float yCenter, float width, float height, float radFirst, float radEnd, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawArc)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_FLOAT( radFirst ) ;
	LQT_FUNC_ARG_FLOAT( radEnd ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->DrawArc
			( xCenter, yCenter, width, height,
				radFirst, radEnd, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean drawBezier( const Vector2* pPoints, ulong nPoints, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_drawBezier)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_STRUCT_N( LVector2, pPoints, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pPoints ) ;
	LQT_FUNC_ARG_ULONG( nPoints ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->DrawBezier
			( pPoints, (size_t) nPoints, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean fillEllipse( float xCenter, float yCenter, float width, float height, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillEllipse)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->FillEllipse
			( xCenter, yCenter, width, height, argb, z, flags ) == sglErrSuccess ) ;
}

// boolean fillArc( float xCenter, float yCenter, float width, float height, float radFirst, float radEnd, uint argb, double z, uint flags )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_fillArc)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( width ) ;
	LQT_FUNC_ARG_FLOAT( height ) ;
	LQT_FUNC_ARG_FLOAT( radFirst ) ;
	LQT_FUNC_ARG_FLOAT( radEnd ) ;
	LQT_FUNC_ARG_UINT( argb ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LQT_RETURN_BOOL
		( pDraw->FillArc
			( xCenter, yCenter, width, height,
				radFirst, radEnd, argb, z, flags ) == sglErrSuccess ) ;
}

// void freeGradation( )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_freeGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw != nullptr )
	{
		pDraw->FreeGradation() ;
	}
	LQT_RETURN_VOID() ;
}

// boolean setLinearGradation( float x0, float y0, float x1, float y1, const ARGB8* pGradation, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setLinearGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_FLOAT( x0 ) ;
	LQT_FUNC_ARG_FLOAT( y0 ) ;
	LQT_FUNC_ARG_FLOAT( x1 ) ;
	LQT_FUNC_ARG_FLOAT( y1 ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, pGradation ) ;
	LQT_VERIFY_NULL_PTR( pGradation ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LQT_RETURN_BOOL
		( pDraw->SetLinearGradation
			( x0, y0, x1, y1, pGradation, (size_t) nCount ) == sglErrSuccess ) ;
}

// boolean setRingedGradation( float xCenter, float yCenter, float radAngle, const ARGB8* pGradation, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_PaintContext_setRingedGradation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_PaintContext, pThis ) ;
	SGLDrawContextInterface *	pDraw = pThis->GetRef<SGLDrawContextInterface>() ;
	if ( pDraw == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}
	LQT_FUNC_ARG_FLOAT( xCenter ) ;
	LQT_FUNC_ARG_FLOAT( yCenter ) ;
	LQT_FUNC_ARG_FLOAT( radAngle ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, pGradation ) ;
	LQT_VERIFY_NULL_PTR( pGradation ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LQT_RETURN_BOOL
		( pDraw->SetRingedGradation
			( xCenter, yCenter,
				radAngle, pGradation, (size_t) nCount ) == sglErrSuccess ) ;
}




