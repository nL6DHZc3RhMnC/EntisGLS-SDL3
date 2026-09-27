
#include <loquaty.h>
#include "EntisGLS4_Image.h"

using namespace Loquaty ;


// EntisGLS4.Image buildAtlas( EntisGLS4.Image[] images, const EntisGLS4.Image.BuildAtlasParam* param, ulong* pUsedCount, ulong* pUsedIndices )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_buildAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, images ) ;
	LQT_VERIFY_NULL_PTR( images ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Image_BuildAtlasParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;
	LQT_FUNC_ARG_POINTER( LUint64, pUsedCount ) ;
	LQT_VERIFY_NULL_PTR( pUsedCount ) ;
	LQT_FUNC_ARG_POINTER( LUint64, pUsedIndices ) ;
	LQT_VERIFY_NULL_PTR( pUsedIndices ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = buildAtlas(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void makeAdditionalTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeAdditionalTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER( LUint8, pTone ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	// makeAdditionalTone(...) ;

	LQT_RETURN_VOID() ;
}

// void makeBrightnessTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeBrightnessTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER( LUint8, pTone ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	// makeBrightnessTone(...) ;

	LQT_RETURN_VOID() ;
}

// void makeGammaTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeGammaTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER( LUint8, pTone ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	// makeGammaTone(...) ;

	LQT_RETURN_VOID() ;
}

// void makeMultipleTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeMultipleTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER( LUint8, pTone ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	// makeMultipleTone(...) ;

	LQT_RETURN_VOID() ;
}

// void makeOffsetMultipleTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeOffsetMultipleTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER( LUint8, pTone ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	// makeOffsetMultipleTone(...) ;

	LQT_RETURN_VOID() ;
}

// Image( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Image)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_Image, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// ulong getFrameCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getFrameCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getFrameCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getSequenceLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getSequenceLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getSequenceLength(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getSequenceTable( uint* pSeq, ulong nCount ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getSequenceTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_POINTER( LUint32, pSeq ) ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LUint64	valRet ;
	// valRet = pThis->getSequenceTable(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getTotalTime( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getTotalTime)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getTotalTime(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong frameFromMilliSec( ulong msec ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_frameFromMilliSec)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_ULONG( msec ) ;

	LUint64	valRet ;
	// valRet = pThis->frameFromMilliSec(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean selectFrame( ulong iFrame, EntisGLS4.StereoImageIndex iSide )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_selectFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_ULONG( iFrame ) ;
	LQT_FUNC_ARG_INT( iSide ) ;

	LBoolean	valRet ;
	// valRet = pThis->selectFrame(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong getSelectedFrame( int* pSide ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getSelectedFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_POINTER( LInt32, pSide ) ;
	LQT_VERIFY_NULL_PTR( pSide ) ;

	LUint64	valRet ;
	// valRet = pThis->getSelectedFrame(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean getImageInfo( EntisGLS4.ImageInfo* imginf ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ImageInfo, imginf ) ;
	LQT_VERIFY_NULL_PTR( imginf ) ;

	LBoolean	valRet ;
	// valRet = pThis->getImageInfo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// Size* getImageSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LSize	valRet ;
	// valRet = pThis->getImageSize(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// uint getImageWidth( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageWidth)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getImageWidth(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint getImageHeight( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageHeight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getImageHeight(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// EntisGLS4.ImageFormatFlag getImageFormat( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageFormat)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getImageFormat(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint getPaletteTable( ARGB8* pPalette, uint nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getPaletteTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, pPalette ) ;
	LQT_VERIFY_NULL_PTR( pPalette ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;

	LUint32	valRet ;
	// valRet = pThis->getPaletteTable(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint8* lockBuffer( EntisGLS4.ImageInfo* imginf, EntisGLS4.Image.LockBufferMethod flags, const ImageRect* pRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_lockBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ImageInfo, imginf ) ;
	LQT_VERIFY_NULL_PTR( imginf ) ;
	LQT_FUNC_ARG_INT( flags ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pRect ) ;
	LQT_VERIFY_NULL_PTR( pRect ) ;

	LUint8	valRet ;
	// valRet = pThis->lockBuffer(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean flushBuffer( EntisGLS4.Image.LockBufferMethod flags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_flushBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_INT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->flushBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean unlockBuffer( EntisGLS4.Image.LockBufferMethod flags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_unlockBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_INT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->unlockBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Image newReference( const ImageRect* pClip, long iFrame, int iSide )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_newReference)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pClip ) ;
	LQT_VERIFY_NULL_PTR( pClip ) ;
	LQT_FUNC_ARG_LONG( iFrame ) ;
	LQT_FUNC_ARG_INT( iSide ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->newReference(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean normalizeToTexture( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeToTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LBoolean	valRet ;
	// valRet = pThis->normalizeToTexture(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean normalizeToMipmapTexture( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeToMipmapTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LBoolean	valRet ;
	// valRet = pThis->normalizeToMipmapTexture(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean normalizeToRenderTarget( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeToRenderTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LBoolean	valRet ;
	// valRet = pThis->normalizeToRenderTarget(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean denormalizeForTexture( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_denormalizeForTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LBoolean	valRet ;
	// valRet = pThis->denormalizeForTexture(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean createImage( uint width, uint height, EntisGLS4.ImageFormatFlag format, uint depth, EntisGLS4.Image.BufferTypeFlag nFlags, ulong countFrame, ulong msecLong )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( format ) ;
	LQT_FUNC_ARG_UINT( depth ) ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;
	LQT_FUNC_ARG_ULONG( countFrame ) ;
	LQT_FUNC_ARG_ULONG( msecLong ) ;

	LBoolean	valRet ;
	// valRet = pThis->createImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void releaseBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_releaseBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	// pThis->releaseBuffer(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image.BufferTypeFlag getBufferFlags( )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getBufferFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getBufferFlags(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// uint setPaletteTable( const ARGB8* pPalette, uint nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setPaletteTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, pPalette ) ;
	LQT_VERIFY_NULL_PTR( pPalette ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;

	LUint32	valRet ;
	// valRet = pThis->setPaletteTable(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setSequenceTable( const uint* pSeq, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setSequenceTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_POINTER( LUint32, pSeq ) ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	// pThis->setSequenceTable(...) ;

	LQT_RETURN_VOID() ;
}

// void setImageOrigin( int x, int y )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setImageOrigin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_INT( x ) ;
	LQT_FUNC_ARG_INT( y ) ;

	// pThis->setImageOrigin(...) ;

	LQT_RETURN_VOID() ;
}

// void setAnimationDuration( long msecDuration )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setAnimationDuration)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_LONG( msecDuration ) ;

	// pThis->setAnimationDuration(...) ;

	LQT_RETURN_VOID() ;
}

// boolean normalizeFormat( EntisGLS4.ImageFormatFlag format, uint depth, EntisGLS4.Image.NormalizeFormatFlag nFlags, uint width, uint height )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeFormat)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( format ) ;
	LQT_FUNC_ARG_UINT( depth ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;

	LBoolean	valRet ;
	// valRet = pThis->normalizeFormat(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Image newAnimationReference( ImageRect* pFrameRects, ulong nRectsCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_newAnimationReference)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pFrameRects ) ;
	LQT_VERIFY_NULL_PTR( pFrameRects ) ;
	LQT_FUNC_ARG_ULONG( nRectsCount ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->newAnimationReference(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean getReferenceRectOfAtlas( ImageRect* rect, long iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getReferenceRectOfAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rect ) ;
	LQT_VERIFY_NULL_PTR( rect ) ;
	LQT_FUNC_ARG_LONG( iFrame ) ;

	LBoolean	valRet ;
	// valRet = pThis->getReferenceRectOfAtlas(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean loadImage( String strFilePath, String strMIME, uint nLimitFrames )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_loadImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRING( strFilePath ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_UINT( nLimitFrames ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean readImage( File file, String strMIME, uint nLimitFrames )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_readImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_UINT( nLimitFrames ) ;

	LBoolean	valRet ;
	// valRet = pThis->readImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean saveImage( String strFilePath, String strMIME, const EntisGLS4.Image.EncoderOptions* pOpt )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_saveImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRING( strFilePath ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Image_EncoderOptions, pOpt ) ;
	LQT_VERIFY_NULL_PTR( pOpt ) ;

	LBoolean	valRet ;
	// valRet = pThis->saveImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean writeImage( File file, String strMIME, const EntisGLS4.Image.EncoderOptions* pOpt )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_writeImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Image_EncoderOptions, pOpt ) ;
	LQT_VERIFY_NULL_PTR( pOpt ) ;

	LBoolean	valRet ;
	// valRet = pThis->writeImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getPixelARGB( ARGB8* argb, int xPos, int yPos )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getPixelARGB)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, argb ) ;
	LQT_VERIFY_NULL_PTR( argb ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yPos ) ;

	LBoolean	valRet ;
	// valRet = pThis->getPixelARGB(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean fillImage( uint packedPixel, const ImageRect* pRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_fillImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( packedPixel ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pRect ) ;
	LQT_VERIFY_NULL_PTR( pRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->fillImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean copyImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_copyImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->copyImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean blendImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->blendImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean blendBackImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendBackImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->blendBackImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean blendAddImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendAddImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->blendAddImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean blendMulImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendMulImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->blendMulImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean halfBlendImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_halfBlendImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->halfBlendImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean convertImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_convertImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->convertImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean blendImageBackgroundColor( uint packedBackColor )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendImageBackgroundColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_UINT( packedBackColor ) ;

	LBoolean	valRet ;
	// valRet = pThis->blendImageBackgroundColor(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean multiplyImageRGBAlpha( )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_multiplyImageRGBAlpha)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->multiplyImageRGBAlpha(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean putImageChannelTo( int iDstChannel, EntisGLS4.Image pSrcImage, int iSrcChannel, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_putImageChannelTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_INT( iDstChannel ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( iSrcChannel ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->putImageChannelTo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean putImageMAddChannelTo( int iDstChannel, EntisGLS4.Image pSrcImage, uint packedColorMul, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_putImageMAddChannelTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_INT( iDstChannel ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_UINT( packedColorMul ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->putImageMAddChannelTo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean blendWithAlphaChannel( EntisGLS4.Image pAlphaImage, int fxAlphaCoefficient, int fxAlphaIntercept, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendWithAlphaChannel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pAlphaImage ) ;
	LQT_VERIFY_NULL_PTR( pAlphaImage ) ;
	LQT_FUNC_ARG_INT( fxAlphaCoefficient ) ;
	LQT_FUNC_ARG_INT( fxAlphaIntercept ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;
	LQT_VERIFY_NULL_PTR( pSrcRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->blendWithAlphaChannel(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean applyToneFilter( const uint8* pRedTone, const uint8* pGreenTone, const uint8* pBlueTone, const uint8* pAlphaTone, const ImageRect* pDstRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_applyToneFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pRedTone ) ;
	LQT_VERIFY_NULL_PTR( pRedTone ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pGreenTone ) ;
	LQT_VERIFY_NULL_PTR( pGreenTone ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pBlueTone ) ;
	LQT_VERIFY_NULL_PTR( pBlueTone ) ;
	LQT_FUNC_ARG_POINTER( LUint8, pAlphaTone ) ;
	LQT_VERIFY_NULL_PTR( pAlphaTone ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pDstRect ) ;
	LQT_VERIFY_NULL_PTR( pDstRect ) ;

	LBoolean	valRet ;
	// valRet = pThis->applyToneFilter(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean enlargeHalfImage( EntisGLS4.Image pSrcImage )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_enlargeHalfImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;

	LBoolean	valRet ;
	// valRet = pThis->enlargeHalfImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean orthogonalRotate( EntisGLS4.Image pSrcImage, int degRotateAngle )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_orthogonalRotate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImage ) ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( degRotateAngle ) ;

	LBoolean	valRet ;
	// valRet = pThis->orthogonalRotate(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean createColorImageFromGrayscale( EntisGLS4.Image pGrayscale, uint packedColor )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createColorImageFromGrayscale)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pGrayscale ) ;
	LQT_VERIFY_NULL_PTR( pGrayscale ) ;
	LQT_FUNC_ARG_UINT( packedColor ) ;

	LBoolean	valRet ;
	// valRet = pThis->createColorImageFromGrayscale(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean createFilledPolygonShape( Vector2* vMakedOffset, const Vector2* pVertices, uint nCount, float fpUnit )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createFilledPolygonShape)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, vMakedOffset ) ;
	LQT_VERIFY_NULL_PTR( vMakedOffset ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pVertices ) ;
	LQT_VERIFY_NULL_PTR( pVertices ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;
	LQT_FUNC_ARG_FLOAT( fpUnit ) ;

	LBoolean	valRet ;
	// valRet = pThis->createFilledPolygonShape(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean createFilledBezierShape( Vector2* vMakedOffset, const Vector2* pVertices, uint nCount, float fpUnit )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createFilledBezierShape)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, vMakedOffset ) ;
	LQT_VERIFY_NULL_PTR( vMakedOffset ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pVertices ) ;
	LQT_VERIFY_NULL_PTR( pVertices ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;
	LQT_FUNC_ARG_FLOAT( fpUnit ) ;

	LBoolean	valRet ;
	// valRet = pThis->createFilledBezierShape(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



