
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Image.h>


// EntisGLS4.Image buildAtlas( EntisGLS4.Image[] images, const EntisGLS4.Image.BuildAtlasParam* param, ulong* pUsedCount, ulong* pUsedIndices )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_buildAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, images ) ;
	LQT_VERIFY_NULL_PTR( images ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Image_BuildAtlasParam, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;
	LQT_FUNC_ARG_POINTER( LUint64, pUsedCount ) ;
	LQT_FUNC_ARG_POINTER_N( LUint64, pUsedIndices, images->GetElementCount() ) ;

	SPointerArray<SGLImageObject>	aImages ;
	for ( size_t i = 0; i < images->GetElementCount(); i ++ )
	{
		LObjPtr	pElement( images->GetElementAt( i ) ) ;
		std::shared_ptr<LEntisGLS4_Image>
			pRefImage = LNativeObj::GetNative<LEntisGLS4_Image>( pElement.Ptr() ) ;
		if ( pRefImage != nullptr )
		{
			aImages.Add( pRefImage->GetRef<SGLImageObject>() ) ;
		}
	}
	size_t				nUsedCount ;
	SArray<size_t>		aUsedIndex ;
	SGLImageObject *	pImage =
		SGLImageObject::BuildAtlas
			( aImages.GetConstArray(), aImages.GetLength(),
				*param, &nUsedCount, aUsedIndex.GetArray( aImages.GetLength() ) ) ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	std::shared_ptr<LEntisGLS4_Image>
				pImagePtr = std::make_shared<LEntisGLS4_Image>() ;
	pImagePtr->SetSmartReference( pImage ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( pImagePtr ) ;
	LQT_RETURN_OBJECT( valRet ) ;
}

// void makeAdditionalTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeAdditionalTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	SGLImageObject::MakeAdditionalTone( pTone, v ) ;

	LQT_RETURN_VOID() ;
}

// void makeBrightnessTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeBrightnessTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	SGLImageObject::MakeBrightnessTone( pTone, v ) ;

	LQT_RETURN_VOID() ;
}

// void makeGammaTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeGammaTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	SGLImageObject::MakeGammaTone( pTone, v ) ;

	LQT_RETURN_VOID() ;
}

// void makeMultipleTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeMultipleTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	SGLImageObject::MakeMultipleTone( pTone, v ) ;

	LQT_RETURN_VOID() ;
}

// void makeOffsetMultipleTone( uint8* pTone, float v )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_makeOffsetMultipleTone)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pTone ) ;
	LQT_FUNC_ARG_FLOAT( v ) ;

	SGLImageObject::MakeOffsetMultipleTone( pTone, v ) ;

	LQT_RETURN_VOID() ;
}

// Image( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Image)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_Image, pThis, (new SSmartObject( new SGLImage )) ) ;

	LQT_RETURN_VOID() ;
}

// ulong getFrameCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getFrameCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_ULONG( pImage->GetFrameCount() ) ;
}

// ulong getSequenceLength( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getSequenceLength)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_ULONG( pImage->GetSequenceLength() ) ;
}

// ulong getSequenceTable( uint* pSeq, ulong nCount ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getSequenceTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_POINTER_N( LUint32, pSeq, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LQT_RETURN_ULONG( pImage->GetSequenceTable( pSeq, (size_t) nCount ) ) ;
}

// ulong getTotalTime( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getTotalTime)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_ULONG( pImage->GetTotalTime() ) ;
}

// ulong frameFromMilliSec( ulong msec ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_frameFromMilliSec)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_ULONG( msec ) ;

	LQT_RETURN_ULONG( pImage->FrameFromMilliSec( msec ) ) ;
}

// boolean selectFrame( ulong iFrame, int iSide )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_selectFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_ULONG( iFrame ) ;
	LQT_FUNC_ARG_INT( iSide ) ;

	LQT_RETURN_BOOL( pImage->SelectFrame( (size_t) iFrame, iSide ) ) ;
}

// ulong getSelectedFrame( int* pSide ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getSelectedFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_POINTER( LInt32, pSide ) ;

	int	nSide ;
	LUint64	valRet = pImage->GetSelectedFrame( &nSide ) ;
	if ( pSide != nullptr )
	{
		*pSide = (LInt32) nSide ;
	}

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean getImageInfo( EntisGLS4.ImageInfo* imginf ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ImageInfo, imginf ) ;
	LQT_VERIFY_NULL_PTR( imginf ) ;

	LQT_RETURN_BOOL( pImage->GetImageInfo( *imginf ) == sglErrSuccess ) ;
}

// GenVector2<int,long>* getImageSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ImageInfo, imginf ) ;

	SGLSize	valRet = pImage->GetImageSize() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// uint getImageWidth( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageWidth)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_UINT( pImage->GetImageWidth() ) ;
}

// uint getImageHeight( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageHeight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_UINT( pImage->GetImageHeight() ) ;
}

// uint getImageFormat( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getImageFormat)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_UINT( pImage->GetImageFormat() ) ;
}

// uint getPaletteTable( ARGB8* pPalette, uint nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getPaletteTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT_N( LARGB8, pPalette, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pPalette ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;

	LQT_RETURN_UINT( (LUint) pImage->GetPaletteTable( pPalette, (size_t) nCount ) ) ;
}

// uint8* lockBuffer( EntisGLS4.ImageInfo* imginf, int flags, const ImageRect* pRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_lockBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ImageInfo, imginf ) ;
	LQT_VERIFY_NULL_PTR( imginf ) ;
	LQT_FUNC_ARG_INT( flags ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pRect ) ;

	uint8_t *	pbytBuf = pImage->LockBuffer( *imginf, flags, pRect ) ;
	if ( pbytBuf == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	const int32_t	height = ((pRect != nullptr)
								? pRect->h : (int32_t) imginf->height) ;
	std::shared_ptr<LArrayBufAlias>	pBuf ;
	if ( imginf->pitchLine >= 0 )
	{
		pBuf = std::make_shared<LArrayBufAlias>
				( pbytBuf, (size_t) (height * imginf->pitchLine) ) ;
	}
	else
	{
		pBuf = std::make_shared<LArrayBufAlias>
				( pbytBuf + ((int32_t)(height - 1) * imginf->pitchLine),
								(size_t) (height * -imginf->pitchLine) ) ;
	}
	LQT_RETURN_POINTER_BUF( pBuf ) ;
}

// boolean flushBuffer( int flags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_flushBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_INT( flags ) ;

	LQT_RETURN_BOOL( pImage->FlushBuffer( flags ) == sglErrSuccess ) ;
}

// boolean unlockBuffer( int flags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_unlockBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_INT( flags ) ;

	LQT_RETURN_BOOL( pImage->UnlockBuffer( flags ) == sglErrSuccess ) ;
}

// EntisGLS4.Image newReference( const ImageRect* pClip, long iFrame, int iSide )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_newReference)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pClip ) ;
	LQT_FUNC_ARG_LONG( iFrame ) ;
	LQT_FUNC_ARG_INT( iSide ) ;

	SGLImageObject *	pRefImage =
			pImage->NewReference( pClip, (ssize_t) iFrame, iSide ) ;
	if ( pRefImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	std::shared_ptr<LEntisGLS4_Image>
				pRefImagePtr = std::make_shared<LEntisGLS4_Image>() ;
	pRefImagePtr->SetSmartReference( pRefImage ) ;
	valRet->SetNative( pRefImagePtr ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean normalizeToTexture( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeToTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LQT_RETURN_BOOL( pImage->NormalizeToTexture( nFlags ) == sglErrSuccess ) ;
}

// boolean normalizeToMipmapTexture( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeToMipmapTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LQT_RETURN_BOOL( pImage->NormalizeToMipmapTexture( nFlags ) == sglErrSuccess ) ;
}

// boolean normalizeToRenderTarget( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeToRenderTarget)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LQT_RETURN_BOOL( pImage->NormalizeToRenderTarget( nFlags ) == sglErrSuccess ) ;
}

// boolean denormalizeForTexture( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_denormalizeForTexture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	LQT_RETURN_BOOL( pImage->DenormalizeForTexture( nFlags ) == sglErrSuccess ) ;
}

// boolean createImage( uint width, uint height, uint format, uint depth, ulong nFlags, ulong countFrame, ulong msecLong )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( format ) ;
	LQT_FUNC_ARG_UINT( depth ) ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;
	LQT_FUNC_ARG_ULONG( countFrame ) ;
	LQT_FUNC_ARG_ULONG( msecLong ) ;

	LQT_RETURN_BOOL
		( pImage->CreateImage
			( width, height, format, depth,
				nFlags, (size_t) countFrame, msecLong ) == sglErrSuccess ) ;
}

// void releaseBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_releaseBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	pImage->ReleaseBuffer() ;

	LQT_RETURN_VOID() ;
}

// ulong getBufferFlags( )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getBufferFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_ULONG( pImage->GetBufferFlags() ) ;
}

// uint setPaletteTable( const ARGB8* pPalette, uint nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setPaletteTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT_N( LARGB8, pPalette, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pPalette ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;

	LQT_RETURN_UINT( pImage->SetPaletteTable( pPalette, (size_t) nCount ) ) ;
}

// void setSequenceTable( const uint* pSeq, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setSequenceTable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_POINTER_N( LUint32, pSeq, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	pImage->SetSequenceTable( pSeq, (size_t) nCount ) ;

	LQT_RETURN_VOID() ;
}

// void setImageOrigin( int x, int y )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setImageOrigin)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_INT( x ) ;
	LQT_FUNC_ARG_INT( y ) ;

	pImage->SetImageOrigin( x, y ) ;

	LQT_RETURN_VOID() ;
}

// void setAnimationDuration( long msecDuration )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_setAnimationDuration)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_LONG( msecDuration ) ;

	pImage->SetAnimationDuration( msecDuration ) ;

	LQT_RETURN_VOID() ;
}

// boolean normalizeFormat( uint format, uint depth, uint nFlags, uint width, uint height )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_normalizeFormat)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( format ) ;
	LQT_FUNC_ARG_UINT( depth ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;

	LQT_RETURN_BOOL
		( pImage->NormalizeFormat
			( format, depth, nFlags, width, height ) == sglErrSuccess ) ;
}

// EntisGLS4.Image newAnimationReference( ImageRect* pFrameRects, ulong nRectsCount )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_newAnimationReference)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT_N( LImageRect, pFrameRects, LQT_ARG_LONG(2) ) ;
	LQT_VERIFY_NULL_PTR( pFrameRects ) ;
	LQT_FUNC_ARG_ULONG( nRectsCount ) ;

	SGLImageObject *	pRefImage =
		pImage->NewAnimationReference( pFrameRects, (size_t) nRectsCount ) ;
	if ( pRefImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	std::shared_ptr<LEntisGLS4_Image>
						pRefImagePtr = std::make_shared<LEntisGLS4_Image>() ;
	pRefImagePtr->SetSmartReference( pRefImage ) ;
	valRet->SetNative( pRefImagePtr ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean getReferenceRectOfAtlas( ImageRect* rect, long iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getReferenceRectOfAtlas)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rect ) ;
	LQT_VERIFY_NULL_PTR( rect ) ;
	LQT_FUNC_ARG_LONG( iFrame ) ;

	LQT_RETURN_BOOL
		( pImage->GetReferenceRectOfAtlas
					( *rect, (ssize_t) iFrame ) == sglErrSuccess ) ;
}

// boolean loadImage( String strFilePath, String strMIME, uint nLimitFrames )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_loadImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRING( strFilePath ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_UINT( nLimitFrames ) ;

	LQT_RETURN_BOOL
		( pImage->LoadImage
			( strFilePath.c_str(),
				strMIME.c_str(), nLimitFrames ) == sglErrSuccess ) ;
}

// boolean readImage( File file, String strMIME, uint nLimitFrames )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_readImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_UINT( nLimitFrames ) ;

	SLoquatyFile	lfile( file ) ;
	LQT_RETURN_BOOL
		( pImage->ReadImage
			( &lfile, strMIME.c_str(), nLimitFrames ) == sglErrSuccess ) ;
}

// boolean saveImage( String strFilePath, String strMIME, const EntisGLS4.Image.EncoderOptions* pOpt )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_saveImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRING( strFilePath ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Image_EncoderOptions, pOpt ) ;

	LQT_RETURN_BOOL
		( pImage->SaveImage
			( strFilePath.c_str(), strMIME.c_str(), pOpt ) == sglErrSuccess ) ;
}

// boolean writeImage( File file, String strMIME, const EntisGLS4.Image.EncoderOptions* pOpt )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_writeImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( strMIME ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Image_EncoderOptions, pOpt ) ;

	SLoquatyFile	lfile( file ) ;
	LQT_RETURN_BOOL
		( pImage->WriteImage( &lfile, strMIME.c_str(), pOpt ) == sglErrSuccess ) ;
}

// boolean getPixelARGB( ARGB8* argb, int xPos, int yPos )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_getPixelARGB)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT( LARGB8, argb ) ;
	LQT_VERIFY_NULL_PTR( argb ) ;
	LQT_FUNC_ARG_INT( xPos ) ;
	LQT_FUNC_ARG_INT( yPos ) ;

	LQT_RETURN_BOOL( pImage->GetPixelRGBA( *argb, xPos, yPos ) == sglErrSuccess ) ;
}

// boolean fillImage( uint packedPixel, const ImageRect* pRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_fillImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( packedPixel ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pRect ) ;

	LQT_RETURN_BOOL
		( pImage->FillImage( SGLPalette(packedPixel), pRect ) == sglErrSuccess ) ;
}

// boolean copyImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_copyImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->CopyImage
			( pSrcImage, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean blendImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->BlendImage
			( pSrcImage, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean blendBackImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendBackImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->BlendBackImage
			( pSrcImage, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean blendAddImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendAddImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->BlendAddImage
			( pSrcImage, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean blendMulImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendMulImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->BlendMulImage
			( pSrcImage, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean halfBlendImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_halfBlendImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->HalfBlendImage
			( pSrcImage, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean convertImage( EntisGLS4.Image pSrcImage, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_convertImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->ConvertImage
			( pSrcImage, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean blendImageBackgroundColor( uint packedBackColor )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendImageBackgroundColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_UINT( packedBackColor ) ;

	LQT_RETURN_BOOL
		( pImage->BlendImageBackgroundColor
				( SGLPalette( packedBackColor ) ) == sglErrSuccess ) ;
}

// boolean multiplyImageRGBAlpha( )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_multiplyImageRGBAlpha)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;

	LQT_RETURN_BOOL( pImage->MultiplyImageRGBAlpha() == sglErrSuccess ) ;
}

// boolean putImageChannelTo( int iDstChannel, EntisGLS4.Image pSrcImage, int iSrcChannel, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_putImageChannelTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_INT( iDstChannel ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( iSrcChannel ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->PutImageChannelTo
			( iDstChannel, pSrcImage,
				iSrcChannel, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean putImageMAddChannelTo( int iDstChannel, EntisGLS4.Image pSrcImage, uint packedColorMul, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_putImageMAddChannelTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_INT( iDstChannel ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_UINT( packedColorMul ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->PutImageMAddChannelTo
			( iDstChannel, pSrcImage,
				SGLPalette( packedColorMul ), xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean blendWithAlphaChannel( EntisGLS4.Image pAlphaImage, int fxAlphaCoefficient, int fxAlphaIntercept, int xDst, int yDst, const ImageRect* pSrcRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_blendWithAlphaChannel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pAlphaImageObj ) ;
	LQT_VERIFY_NULL_PTR( pAlphaImageObj ) ;
	SGLImageObject *	pAlphaImage = pAlphaImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pAlphaImage ) ;
	LQT_FUNC_ARG_INT( fxAlphaCoefficient ) ;
	LQT_FUNC_ARG_INT( fxAlphaIntercept ) ;
	LQT_FUNC_ARG_INT( xDst ) ;
	LQT_FUNC_ARG_INT( yDst ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pSrcRect ) ;

	LQT_RETURN_BOOL
		( pThisImage->BlendWithAlphaChannel
			( pAlphaImage, fxAlphaCoefficient,
				fxAlphaIntercept, xDst, yDst, pSrcRect ) == sglErrSuccess ) ;
}

// boolean applyToneFilter( const uint8* pRedTone, const uint8* pGreenTone, const uint8* pBlueTone, const uint8* pAlphaTone, const ImageRect* pDstRect )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_applyToneFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pRedTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pRedTone ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pGreenTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pGreenTone ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pBlueTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pBlueTone ) ;
	LQT_FUNC_ARG_POINTER_N( LUint8, pAlphaTone, 0x100 ) ;
	LQT_VERIFY_NULL_PTR( pAlphaTone ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pDstRect ) ;

	LQT_RETURN_BOOL
		( pImage->ApplyToneFilter
			( pRedTone, pGreenTone, pBlueTone, pAlphaTone, pDstRect ) == sglErrSuccess ) ;
}

// boolean enlargeHalfImage( EntisGLS4.Image pSrcImage )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_enlargeHalfImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;

	LQT_RETURN_BOOL( pThisImage->EnlargeHalfImage( pSrcImage ) == sglErrSuccess ) ;
}

// boolean orthogonalRotate( EntisGLS4.Image pSrcImage, int degRotateAngle )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_orthogonalRotate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pSrcImageObj ) ;
	LQT_VERIFY_NULL_PTR( pSrcImageObj ) ;
	SGLImageObject *	pSrcImage = pSrcImageObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pSrcImage ) ;
	LQT_FUNC_ARG_INT( degRotateAngle ) ;

	LQT_RETURN_BOOL
		( pThisImage->OrthogonalRotate
			( pSrcImage, degRotateAngle ) == sglErrSuccess ) ;
}

// boolean createColorImageFromGrayscale( EntisGLS4.Image pGrayscale, uint packedColor )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createColorImageFromGrayscale)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, pGrayscaleObj ) ;
	LQT_VERIFY_NULL_PTR( pGrayscaleObj ) ;
	SGLImageObject *	pGrayscale = pGrayscaleObj->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pGrayscale ) ;
	LQT_FUNC_ARG_UINT( packedColor ) ;

	LQT_RETURN_BOOL
		( pThisImage->CreateColorImageFromGrayscale
			( pGrayscale, SGLPalette( packedColor ) ) == sglErrSuccess ) ;
}

// boolean createFilledPolygonShape( Vector2* vMakedOffset, const Vector2* pVertices, uint nCount, float fpUnit )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createFilledPolygonShape)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, vMakedOffset ) ;
	LQT_VERIFY_NULL_PTR( vMakedOffset ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, pVertices, LQT_ARG_LONG(3) ) ;
	LQT_VERIFY_NULL_PTR( pVertices ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;
	LQT_FUNC_ARG_FLOAT( fpUnit ) ;

	LQT_RETURN_BOOL
		( pThisImage->CreateFilledPolygonShape
			( *vMakedOffset, pVertices, (size_t) nCount, fpUnit ) == sglErrSuccess ) ;
}

// boolean createFilledBezierShape( Vector2* vMakedOffset, const Vector2* pVertices, uint nCount, float fpUnit )
IMPL_LOQUATY_FUNC(EntisGLS4_Image_createFilledBezierShape)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Image, pThis ) ;
	SGLImageObject *	pThisImage = pThis->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pThisImage ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, vMakedOffset ) ;
	LQT_VERIFY_NULL_PTR( vMakedOffset ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, pVertices, LQT_ARG_LONG(3) ) ;
	LQT_VERIFY_NULL_PTR( pVertices ) ;
	LQT_FUNC_ARG_UINT( nCount ) ;
	LQT_FUNC_ARG_FLOAT( fpUnit ) ;

	LQT_RETURN_BOOL
		( pThisImage->CreateFilledBezierShape
			( *vMakedOffset, pVertices, (size_t) nCount, fpUnit ) == sglErrSuccess ) ;
}



