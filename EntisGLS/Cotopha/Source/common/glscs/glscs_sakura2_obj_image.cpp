
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <glscs/glscs_sakura2_obj_image.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// 画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( ECSSakura2::ECSImageObject, ECSVolatileObject, SGLMultiImage )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSImageObject::ECSImageObject( void )
{
	m_typeProvider = providerNoData ;
	m_iSelFrame = -1 ;
	//
	m_ripRef.dwRefImage = 0 ;
	m_cbpCreate.dwFlags = 0 ;
	m_cbpCreate.dwFrames = 0 ;
	m_cbpCreate.nTimeLong = 0 ;
}

ECSImageObject::ECSImageObject
	( const ECSImageObject& img, const SGLImageRect * pClip, int iSide  )
: SGLMultiImage( img, pClip, iSide )
{
	m_typeProvider = providerNoData ;
	m_iSelFrame = -1 ;
	//
	m_ripRef.dwRefImage = 0 ;
	m_cbpCreate.dwFlags = 0 ;
	m_cbpCreate.dwFrames = 0 ;
	m_cbpCreate.nTimeLong = 0 ;
}

// フレーム選択
//////////////////////////////////////////////////////////////////////////////
SGLError ECSImageObject::SelectFrame( size_t iFrame, int iSide )
{
	m_iSelFrame = (ssize_t) iFrame ;
	m_iSelSide = iSide ;
	return	SGLMultiImage::SelectFrame( iFrame, iSide ) ;
}

// 画像バッファへの参照生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * ECSImageObject::NewReference
	( const SGLImageRect * pClip, ssize_t iFrame, int iSide )
{
	ECSImageObject *	pRefImage = NULL ;
	if ( iFrame < 0 )
	{
		pRefImage = new ECSImageObject( *this, pClip, iSide ) ;
	}
	else
	{
		SGLImageBuffer *	pImage = m_paImages.GetAt( iFrame ) ;
		if ( pImage == NULL )
		{
			return	NULL ;
		}
		SGLImageBuffer *	pRef =
				sglCreateReferenceImageBuffer( pImage, pClip, 0, iSide ) ;
		if ( pRef == NULL )
		{
			return	NULL ;
		}
		pRefImage = new ECSImageObject ;
		pRefImage->SetImageBuffer( pRef ) ;
	}
	pRefImage->m_typeProvider = providerRefImage ;
	pRefImage->m_ripRef.dwRefImage = m_dwHighAddr ;
	pRefImage->m_ripRef.iFrame = (DWORD) iFrame ;
	pRefImage->m_ripRef.iSide = iSide ;
	if ( pClip != NULL )
	{
		pRefImage->m_ripRef.rctClip = *pClip ;
	}
	else
	{
		pRefImage->m_ripRef.rctClip.SetPosition( SGLPoint( 0, 0 ) ) ;
		pRefImage->m_ripRef.rctClip.SetSize( GetImageSize() ) ;
	}
	return	pRefImage ;
}

// テクスチャのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError ECSImageObject::NormalizeToTexture( uint32_t nFlags )
{
	m_cbpCreate.dwFlags |= SGLImageObject::bufferForTexture | nFlags ;
	return	SGLMultiImage::NormalizeToTexture( nFlags ) ;
}

SGLError ECSImageObject::NormalizeToMipmapTexture( uint32_t nFlags )
{
	m_cbpCreate.dwFlags |= SGLImageObject::bufferForMipmapTexture | nFlags ;
	return	SGLMultiImage::NormalizeToMipmapTexture( nFlags ) ;
}

// レンダリング・ターゲットのための正規化
//////////////////////////////////////////////////////////////////////////////
SGLError ECSImageObject::NormalizeToRenderTarget( uint32_t nFlags )
{
	m_cbpCreate.dwFlags |= SGLImageObject::bufferForRenderTarget | nFlags ;
	return	SGLMultiImage::NormalizeToRenderTarget( nFlags ) ;
}

// 画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError ECSImageObject::CreateBuffer
	( const SGLImageInfo& imginf,
		int nFlags, size_t countFrame, uint64_t msecLong )
{
	SGLError	err = SGLMultiImage::CreateBuffer
						( imginf, nFlags, countFrame, msecLong ) ;
	if ( !err )
	{
		m_typeProvider = providerCreateBuffer ;
		m_cbpCreate.dwFlags = (DWORD) nFlags ;
		m_cbpCreate.dwFrames = (DWORD) countFrame ;
		m_cbpCreate.nTimeLong = msecLong ;
	}
	return	err ;
}

// 保有リソース解放
//////////////////////////////////////////////////////////////////////////////
void ECSImageObject::ReleaseBuffer( void )
{
	m_typeProvider = providerNoData ;
	m_cbpCreate.dwFlags = 0 ;
}

// NormalizeFormat で結合されたアニメーション画像への参照を生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * ECSImageObject::NewAnimationReference
	( SGLImageRect* pFrameRects, size_t nRectsCount )
{
	SGLImageObject *	pImage =
		SGLMultiImage::NewAnimationReference( pFrameRects, nRectsCount ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	SGLSmartImage *	psiImage = ESLTypeCast<SGLSmartImage>( pImage ) ;
	if ( psiImage == NULL )
	{
		delete	pImage ;
		return	NULL ;
	}
	SGLImageBuffer *	pImageBuf = psiImage->GetImage() ;
	if ( pImageBuf == NULL )
	{
		delete	pImage ;
		return	NULL ;
	}
	ECSImageObject *	pRefImage = new ECSImageObject ;
	sglAddReferenceImageBuffer( pImageBuf ) ;
	pRefImage->SetImageBuffer( pImageBuf ) ;
	return	pRefImage ;
}

// 画像ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLError ECSImageObject::LoadImageFile
	( SEnvironmentInterface * pEnv,
		const wchar_t * pszFilePath,
		const wchar_t * pszMIME, size_t nLimitFrames )
{
	ReleaseBuffer() ;
	//
	SSmartPointer<SFileInterface>
		pFile = pEnv->NewOpenFile( pszFilePath, SFileOpener::shareRead ) ;
	if ( pFile != NULL )
	{
		SString	strFilePath = pszFilePath ;
		SString	strFileExt = strFilePath.GetFileExtensionPart() ;
		//
		SGLImageDecoderInterface *	pDecoder =
			SGLImageDecoderManager::FindDecoder( strFileExt ) ;
		if ( pDecoder != NULL )
		{
			if ( !pDecoder->ReadImage( *this, *pFile, nLimitFrames ) )
			{
				m_typeProvider = providerLoadImage ;
				m_strFilePath = strFilePath ;
				m_strMIMEType.FreeArray() ;
				m_nLimitFrames = (uint32_t) nLimitFrames ;
				return	sglErrSuccess ;
			}
			pFile->Seek( 0 ) ;
		}
		if ( !ReadImageFile( pFile, pszMIME, nLimitFrames ) )
		{
			m_typeProvider = providerLoadImage ;
			m_strFilePath = strFilePath ;
			m_strMIMEType = pszMIME ;
			m_nLimitFrames = (uint32_t) nLimitFrames ;
			return	sglErrSuccess ;
		}
	}
	return	sglErrFailed ;
}

SGLError ECSImageObject::ReadImageFile
	( SFileInterface * pFile,
		const wchar_t * pszMIME, size_t nLimitFrames )
{
	ReleaseBuffer() ;
	//
	SGLImageDecoderInterface *	pDecoder =
		SGLImageDecoderManager::FindDecoderAsMIME( pszMIME ) ;
	if ( pDecoder != NULL )
	{
		if ( !pDecoder->ReadImage( *this, *pFile, nLimitFrames ) )
		{
			return	sglErrSuccess ;
		}
		pFile->Seek( 0 ) ;
	}
	return	SGLImageDecoderManager::ReadImage( *this, *pFile, nLimitFrames ) ;
}

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
		ECSImageObject::GetSegmentBuffer( LinearAddressCache & seg )
{
	if ( m_pImage != NULL )
	{
		seg.baseOffset = 0 ;
		seg.limitSegment = m_pImage->pitchLine * m_pImage->height ;
		seg.pbytBuffer = m_pImage->ptrBuffer ;
		return	&seg;
	}
	return	NULL ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSImageObject::GetTypeName( void ) const
{
	return	L"SakuraGL::Image" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError ECSImageObject::SaveDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSVolatileObject::SaveDynamic( file, vm, context ) ;
	//
	DWORD	dwType = (DWORD) m_typeProvider ;
	file->Write( &dwType, sizeof(DWORD) ) ;
	file->Write( &m_iSelFrame, sizeof(ssize_t) ) ;
	file->Write( &m_iSelSide, sizeof(int) ) ;
	file->Write( &m_cbpCreate, sizeof(CREATE_BUFFER_PARAM) ) ;
	//
	if ( m_typeProvider == providerRefImage )
	{
		file->Write( &m_ripRef, sizeof(REF_IMAGE_PARAM) ) ;
	}
	else if ( m_typeProvider == providerLoadImage )
	{
		file->WriteString( m_strFilePath ) ;
		file->WriteString( m_strMIMEType ) ;
		file->Write( &m_nLimitFrames, sizeof(uint32_t)  ) ;
	}
	else if ( m_typeProvider == providerCreateBuffer )
	{
		SGLImageInfo	imginf ;
		if ( m_pImage != NULL )
		{
			imginf = *m_pImage ;
		}
		file->Write( &imginf, sizeof(SGLImageInfo) ) ;
	}
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError ECSImageObject::LoadDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSVolatileObject::LoadDynamic( file, vm, context ) ;
	//
	DWORD				dwType ;
	CREATE_BUFFER_PARAM	cbpCreate ;
	SGLImageInfo		imginf ;
	SString				strFilePath ;
	//
	file->Read( &dwType, sizeof(DWORD) ) ;
	file->Read( &m_iSelFrame, sizeof(ssize_t) ) ;
	file->Read( &m_iSelSide, sizeof(int) ) ;
	file->Read( &cbpCreate, sizeof(CREATE_BUFFER_PARAM) ) ;
	//
	switch ( dwType )
	{
	case	providerRefImage:
		m_typeProvider = providerRefImage ;
		file->Read( &m_ripRef, sizeof(REF_IMAGE_PARAM) ) ;
		break ;

	case	providerLoadImage:
		file->ReadString( strFilePath ) ;
		file->ReadString( m_strMIMEType ) ;
		file->Read( &m_nLimitFrames, sizeof(uint32_t)  ) ;
		LoadImageFile
			( vm->GetEnvironment(),
				strFilePath, m_strMIMEType, m_nLimitFrames ) ;
		//
		if ( cbpCreate.dwFlags & SGLImageObject::bufferForTexture )
		{
			NormalizeToTexture( cbpCreate.dwFlags & SGLImageObject::bufferNonPowerOf2 ) ;
		}
		if ( cbpCreate.dwFlags & SGLImageObject::bufferForMipmapTexture )
		{
			NormalizeToMipmapTexture( cbpCreate.dwFlags & SGLImageObject::bufferNonPowerOf2 ) ;
		}
		if ( cbpCreate.dwFlags & SGLImageObject::bufferForRenderTarget )
		{
			NormalizeToRenderTarget( cbpCreate.dwFlags & SGLImageObject::bufferNonPowerOf2 ) ;
		}
		break ;

	case	providerCreateBuffer:
		file->Read( &imginf, sizeof(SGLImageInfo) ) ;
		CreateBuffer
			( imginf, (int) cbpCreate.dwFlags,
				(size_t) cbpCreate.dwFrames, cbpCreate.nTimeLong ) ;
		break ;
	}
	return	errSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SError ECSImageObject::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	if ( m_typeProvider == providerRefImage )
	{
		ECSImageObject *	pImageObj =
			ESLTypeCast<ECSImageObject>
				( vm->ObjectFromAddress( m_ripRef.dwRefImage ) ) ;
		if ( pImageObj != NULL )
		{
			if ( (SDWORD) m_ripRef.iFrame < 0 )
			{
				CreateReferenceFrom
					( *pImageObj, &(m_ripRef.rctClip), m_ripRef.iSide ) ;
			}
			else
			{
				SGLImageBuffer *	pImage =
						pImageObj->m_paImages.GetAt( m_ripRef.iFrame ) ;
				if ( pImage != NULL )
				{
					SGLImageBuffer *	pRef =
						sglCreateReferenceImageBuffer
							( pImage, &(m_ripRef.rctClip), 0, m_ripRef.iSide ) ;
					if ( pRef != NULL )
					{
						SetImageBuffer( pRef ) ;
					}
				}
			}
		}
	}
	else if ( m_typeProvider != providerNoData )
	{
		SelectFrame( m_iSelFrame, m_iSelSide ) ;
	}
	return	errSuccess ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::Image
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_Image,context,cls_id)
{
	return	new ECSImageObject ;
}

// size_t GetFrameCount( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetFrameCount,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::GetFrameCount ) ;
	//
	context->m_regset[regAcc].i = pImage->GetFrameCount() ;
	//
	return	NULL ;
}

// size_t GetSequenceLength( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetSequenceLength,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::GetFrameCount ) ;
	//
	context->m_regset[regAcc].i = pImage->GetSequenceLength() ;
	//
	return	NULL ;
}

// size_t GetSequenceTable( uint32_t * pSeq, size_t nCount ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetSequenceTable,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::GetSequenceTable ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, uint32_t, pSeq, arg[1].i, arg[2].i, Image::GetSequenceTable ) ;
	//
	context->m_regset[regAcc].i =
		pImage->GetSequenceTable( pSeq, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// uint64_t GetTotalTime( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetTotalTime,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::GetTotalTime ) ;
	//
	context->m_regset[regAcc].i = pImage->GetTotalTime() ;
	//
	return	NULL ;
}

// size_t FrameFromMilliSec( uint64_t msec ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_FrameFromMilliSec,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::FrameFromMilliSec ) ;
	//
	context->m_regset[regAcc].i = pImage->FrameFromMilliSec( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError SelectFrame( size_t iFrame, int iSide = stereoImageRight ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_SelectFrame,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::SelectFrame ) ;
	//
	context->m_regset[regAcc].i =
		pImage->SelectFrame( (size_t) arg[1].i, (int) arg[2].i ) ;
	//
	return	NULL ;
}

// size_t GetSelectedFrame( int * pSide = NULL ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetSelectedFrame,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::GetSelectedFrame ) ;
	//
	int64_t *	pSide =
		(int64_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(int64_t) ) ;
	int	iSide ;
	context->m_regset[regAcc].i = pImage->GetSelectedFrame( &iSide ) ;
	//
	if ( pSide != NULL )
	{
		*pSide = iSide ;
	}
	return	NULL ;
}

// SGLError GetImageInfo( SGLImageInfo & imginf ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetImageInfo,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::GetImageInfo ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLImageInfo, pInf, arg[1].i, Image::GetImageInfo ) ;
	//
	context->m_regset[regAcc].i = pImage->GetImageInfo( *pInf ) ;
	//
	return	NULL ;
}

// size_t GetPaletteTable( SGLPalette * pPalette, size_t nCount ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetPaletteTable,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::GetPaletteTable ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, SGLPalette, pPalette, arg[1].i, arg[2].i, Image::GetPaletteTable ) ;
	//
	context->m_regset[regAcc].i =
			pImage->GetPaletteTable( pPalette, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// uint8_t * LockBuffer( SGLImageInfo & imginf,
//	int flags = bufferReadWrite, const SGLImageRect * pRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_LockBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSmartImage, pImage, arg, Image::LockBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLImageInfo, pInf, arg[1].i, Image::LockBuffer ) ;
	const int				flags = (int) arg[2].i ;
	const SGLImageRect *	pRect = NULL ;
	if ( arg[3].i != 0 )
	{
		pRect = (const SGLImageRect*)
					context->AtomicTranslateAddress
						( arg[3].i, sizeof(SGLImageRect) ) ;
	}
	uint8_t *			pBuf = pImage->LockBuffer( *pInf, flags, pRect ) ;
	SGLImageBuffer *	pImageBuf = pImage->GetImage() ;
	if ( (pBuf != NULL) && (pImageBuf != NULL) )
	{
		context->m_regset[regAcc].i =
			arg[0].i + ((ULONG_PTR) pBuf
							- (ULONG_PTR) pImageBuf->ptrBuffer) ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// SGLError FlushBuffer( int flags = bufferReadWrite ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_FlushBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::FlushBuffer ) ;
	//
	context->m_regset[regAcc].i = pImage->FlushBuffer( (int) arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError UnlockBuffer( int flags = bufferReadWrite ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_UnlockBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::UnlockBuffer ) ;
	//
	context->m_regset[regAcc].i = pImage->UnlockBuffer( (int) arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError ReadFrameBuffer
//	( SGLImageInfo & imginf, uint8_t * ptrBuffer,
//			size_t iFrame = 0, int iSide = stereoImageRight ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_ReadFrameBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSmartImage, pImage, arg, Image::ReadFrameBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLImageInfo, pInf, arg[1].i, Image::ReadFrameBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, uint8_t, ptrBuffer, arg[2].i, Image::ReadFrameBuffer ) ;
	//
	const size_t		iFrame = (size_t) arg[3].i ;
	const int			iSide = (int) arg[4].i ;
	//
	if ( (ptrBuffer != NULL) && (pImage != NULL) )
	{
		context->m_regset[regAcc].i =
			pImage->ReadFrameBuffer( *pInf, ptrBuffer, iFrame, iSide ) ;
	}
	else
	{
		context->m_regset[regAcc].i = sglErrFailed ;
	}
	return	NULL ;
}

// Image * NewReference( const SGLImageRect * pClip = NULL,
//			ssize_t iFrame = -1, int iSide = stereoImageRight ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_NewReference,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::NewReference ) ;
	const SGLImageRect *	pClip = NULL ;
	const ssize_t			iFrame = (ssize_t) arg[2].i ;
	const int				iSide = (int) arg[3].i ;
	if ( arg[1].i != 0 )
	{
		pClip = (const SGLImageRect*)
					context->AtomicTranslateAddress
						( arg[1].i, sizeof(SGLImageRect) ) ;
	}
	SGLImageObject *
		pRefImage = pImage->NewReference( pClip, iFrame, iSide ) ;
	if ( pRefImage != NULL )
	{
		Object *	pObj = ESLTypeCast<Object>( pRefImage ) ;
		if ( pObj != NULL )
		{
			AssertLock() ;
			context->m_regset[regAcc].i =
					vm->AllocateHeapObjectAddress( pObj ) ;
			AssertUnlock() ;
		}
		else
		{
			delete	pRefImage ;
			context->m_regset[regAcc].i = 0 ;
		}
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// SGLError NormalizeToTexture( uint32_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_NormalizeToTexture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::NormalizeToTexture ) ;
	//
	context->m_regset[regAcc].i = pImage->NormalizeToTexture( arg[1].l32 ) ;
	//
	return	NULL ;
}

// SGLError NormalizeToMipmapTexture( uint32_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_NormalizeToMipmapTexture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::NormalizeToMipmapTexture ) ;
	//
	context->m_regset[regAcc].i = pImage->NormalizeToMipmapTexture( arg[1].l32 ) ;
	//
	return	NULL ;
}

// SGLError NormalizeToRenderTarget( uint32_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_NormalizeToRenderTarget,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::NormalizeToRenderTarget ) ;
	//
	context->m_regset[regAcc].i = pImage->NormalizeToRenderTarget( arg[1].l32 ) ;
	//
	return	NULL ;
}

// SGLError DenormalizeForTexture( uint32_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_DenormalizeForTexture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::NormalizeToRenderTarget ) ;
	//
	context->m_regset[regAcc].i = pImage->DenormalizeForTexture( arg[1].l32 ) ;
	//
	return	NULL ;
}

// SGLError CreateBuffer( const SGLImageInfo& imginf,
//	int nFlags = bufferOnMemory, size_t countFrame = 1, uint64_t msecLong = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_CreateBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::CreateBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLImageInfo, pInf, arg[1].i, Image::CreateBuffer ) ;
	//
	context->m_regset[regAcc].i =
		pImage->CreateBuffer
			( *pInf, (int) arg[2].i, (size_t) arg[3].i, arg[4].i ) ;
	//
	return	NULL ;
}

// void ReleaseBuffer( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_ReleaseBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::CreateBuffer ) ;
	//
	pImage->ReleaseBuffer() ;
	//
	return	NULL ;
}

// int GetBufferFlags( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_GetBufferFlags,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::CreateBuffer ) ;
	//
	context->m_regset[regAcc].i = (uint32_t) pImage->GetBufferFlags() ;
	//
	return	NULL ;
}

// size_t SetPaletteTable( const SGLPalette * pPalette, size_t nCount ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_SetPaletteTable,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::SetPaletteTable ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, SGLPalette, pPalette, arg[1].i, arg[2].i, Image::SetPaletteTable ) ;
	//
	context->m_regset[regAcc].i =
		pImage->SetPaletteTable( pPalette, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// void SetSequenceTable( const uint32_t * pSeq, size_t nCount ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_SetSequenceTable,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::SetSequenceTable ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, const uint32_t, pSeq, arg[1].i, arg[2].i, Image::SetSequenceTable ) ;
	//
	pImage->SetSequenceTable( pSeq, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// void SetImageOrigin( int x, int y ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_SetImageOrigin,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::SetSequenceTable ) ;
	//
	pImage->SetImageOrigin( (int) arg[0].i, (int) arg[1].i ) ;
	//
	return	NULL ;
}

// void SetAnimationDuration( int nDuration ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_SetAnimationDuration,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLImageObject, pImage, arg, Image::SetSequenceTable ) ;
	//
	pImage->SetAnimationDuration( arg[0].i ) ;
	//
	return	NULL ;
}

// SGLError NormalizeFormat( uint32_t format = 0, uint32_t depth = 0,
//		uint32_t nFlags = 0, uint32_t width = 0, uint32_t height = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_NormalizeFormat,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ECSImageObject, pImage, arg, Image::NormalizeFormat ) ;
	//
	context->m_regset[regAcc].i =
		pImage->NormalizeFormat
			( (uint32_t) arg[1].i, (uint32_t) arg[2].i,
				(uint32_t) arg[3].i,
				(uint32_t) arg[4].i, (uint32_t) arg[4].i ) ;
	//
	return	NULL ;
}

// Image * NewAnimationReference( SGLImageRect* pFrameRects, size_t nRectsCount ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_NewAnimationReference,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ECSImageObject, pImage, arg, Image::NormalizeFormat ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, SGLImageRect, pFrameRects,
				arg[1].i, arg[2].i, Image::NewAnimationReference ) ;
	//
	SGLImageObject *	pRefImage =
		pImage->NewAnimationReference( pFrameRects, (size_t) arg[2].i ) ;
	if ( pRefImage != NULL )
	{
		Object *	pObj = ESLTypeCast<Object>( pRefImage ) ;
		if ( pObj != NULL )
		{
			AssertLock() ;
			context->m_regset[regAcc].i =
					vm->AllocateHeapObjectAddress( pObj ) ;
			AssertUnlock() ;
		}
		else
		{
			delete	pRefImage ;
			context->m_regset[regAcc].i = 0 ;
		}
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// SGLError LoadImage
//	( const wchar_t * pszFilePath, const wchar_t * pszMIME = NULL, size_t nLimitFrames = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_LoadImage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ECSImageObject, pImage, arg, Image::LoadImage ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszFilePath, arg[1].i, Image::LoadImage ) ;
	const uint16_t *	pszMIME =
		(const uint16_t *)
			context->AtomicTranslateAddress( arg[2].i, sizeof(uint16_t) ) ;
	//
	SString	strFilePath = pszFilePath ;
	SString	strMIME = pszMIME ;
	context->m_regset[regAcc].i =
		pImage->LoadImageFile
			( vm->GetEnvironment(),
				strFilePath, strMIME, (size_t) arg[3].i ) ;
	//
	return	NULL ;
}

// SGLError ReadImage( File * file, const wchar_t * pszMIME = NULL, size_t nLimitFrames = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_ReadImage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ECSImageObject, pImage, arg, Image::ReadImage ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, SFileInterface, pFile, arg[1].i, Image::ReadImage ) ;
	const uint16_t *	pszMIME =
		(const uint16_t *)
			context->AtomicTranslateAddress( arg[2].i, sizeof(uint16_t) ) ;
	//
	SString	strMIME = pszMIME ;
	context->m_regset[regAcc].i =
		pImage->ReadImageFile( pFile, strMIME, (size_t) arg[3].i ) ;
	//
	return	NULL ;
}

// static bool IsLoadableFileExtension( const wchar_t * pszExt ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_IsLoadableFileExtension,context,arg)
{
	const uint16_t *	pszExt =
		(const uint16_t *)
			context->AtomicTranslateAddress( arg[0].i, sizeof(uint16_t) ) ;
	SString	strExt = pszExt ;
	//
	context->m_regset[regAcc].i =
		(SGLImageDecoderManager::FindDecoder( strExt ) != NULL) ? -1 : 0 ;
	//
	return	NULL ;
}

// static bool IsLoadableMIMEType( const wchar_t * pszMIME ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_IsLoadableMIMEType,context,arg)
{
	const uint16_t *	pszMIME =
		(const uint16_t *)
			context->AtomicTranslateAddress( arg[0].i, sizeof(uint16_t) ) ;
	SString	strMIME = pszMIME ;
	//
	context->m_regset[regAcc].i =
		(SGLImageDecoderManager::FindDecoderAsMIME( strMIME ) != NULL) ? -1 : 0 ;
	//
	return	NULL ;
}

// native void FlushImageObject( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Image_FlushImageObject,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ECSImageObject, pImage, arg, Image::FlushImageObject ) ;
	//
	pImage->FlushImageObject() ;
	//
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// 画像バッファ関数
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

struct	ECS_SGL_IMAGE_BUFFER	: public SGLImageInfo
{
	int64_t	ptrPalette ;
	int64_t	ptrBuffer ;

	void ToImageBuffer
		( ECSSakura2Processor::Context * context,
								SGLImageBuffer& imgbuf ) const
	{
		imgbuf = (const SGLImageInfo&) *this ;
		imgbuf.ptrPalette = NULL ;
		imgbuf.ptrBuffer = NULL ;
		if ( ptrPalette != 0 )
		{
			imgbuf.ptrPalette =
				(SGLPalette*) context->AtomicTranslateAddress
									( ptrPalette, sizeof(SGLPalette) ) ;
		}
		if ( ptrBuffer != 0 )
		{
			imgbuf.ptrBuffer =
				(uint8_t*) context->AtomicTranslateAddress( ptrBuffer, 0 ) ;
		}
	}
} ;

// SGLError sglFillImageBuffer
//	( const SGLImageBuffer& imgbuf,
//		const SGLPalette& pxcmp, const SGLImageRect * pRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglFillImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgbuf,
				arg[0].i, imgbuf of sglFillImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLPalette, pxcmp,
				arg[1].i, pxcmp of sglFillImageBuffer ) ;
	SGLImageRect *	pRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[2].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgbuf ;
	pimgbuf->ToImageBuffer( context, imgbuf ) ;
	//
	context->m_regset[regAcc].i =
		sglFillImageBuffer( imgbuf, *pxcmp, pRect ) ;
	//
	return	NULL ;
}

// SGLError sglCopyImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglCopyImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglCopyImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglCopyImageBuffer ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[4].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglCopyImageBuffer
			( imgDst, imgSrc, (int) arg[2].i, (int) arg[3].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglBlendImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglBlendImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglBlendImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglBlendImageBuffer ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[4].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglBlendImageBuffer
			( imgDst, imgSrc, (int) arg[2].i, (int) arg[3].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglBlendBackImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglBlendBackImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglBlendImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglBlendImageBuffer ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[4].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglBlendBackImageBuffer
			( imgDst, imgSrc, (int) arg[2].i, (int) arg[3].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglAdditionalBlendImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglAdditionalBlendImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglAdditionalBlendImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglAdditionalBlendImageBuffer ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[4].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglAdditionalBlendImageBuffer
			( imgDst, imgSrc, (int) arg[2].i, (int) arg[3].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglMultiplierBlendImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglMultiplierBlendImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglMultiplierBlendImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglMultiplierBlendImageBuffer ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[4].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglMultiplierBlendImageBuffer
			( imgDst, imgSrc, (int) arg[2].i, (int) arg[3].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglEnlargeHalfImageBuffer
//	( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglEnlargeHalfImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglConvertImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglConvertImageBuffer ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglEnlargeHalfImageBuffer( imgDst, imgSrc ) ;
	//
	return	NULL ;
}

// SGLError sglOrthogonalRotateImageBuffer
//	( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc, int degRotateAngle ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglOrthogonalRotateImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglConvertImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglConvertImageBuffer ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglOrthogonalRotateImageBuffer( imgDst, imgSrc, (int) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError sglConvertImageBuffer
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglConvertImageBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglConvertImageBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglConvertImageBuffer ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[4].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglConvertImageBuffer
			( imgDst, imgSrc, (int) arg[2].i, (int) arg[3].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglMultiplyImageRGBAlpha( const SGLImageBuffer& imgbuf ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglMultiplyImageRGBAlpha,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglConvertImageBuffer ) ;
	//
	SGLImageBuffer	imgDst ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	//
	context->m_regset[regAcc].i = sglMultiplyImageRGBAlpha( imgDst ) ;
	//
	return	NULL ;
}

// SGLError sglMakeGrayImageFromRGB( const SGLImageBuffer& imgbuf ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglMakeGrayImageFromRGB,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglConvertImageBuffer ) ;
	//
	SGLImageBuffer	imgDst ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	//
	context->m_regset[regAcc].i = sglMakeGrayImageFromRGB( imgDst ) ;
	//
	return	NULL ;
}

// SGLError sglPutImageChannelTo
//	( const SGLImageBuffer& imgDst, int iDstChannel,
//		const SGLImageBuffer& imgSrc, int iSrcChannel,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglPutImageChannelTo,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglPutImageChannelTo ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[2].i, imgSrc of sglPutImageChannelTo ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[6].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgSrc ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	//
	context->m_regset[regAcc].i =
		sglPutImageChannelTo
			( imgDst, (int) arg[1].i,
				imgSrc, (int) arg[3].i,
				(int) arg[4].i, (int) arg[5].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglApplyToneImageFilter
//	( const SGLImageBuffer& imgbuf,
//		const SGLImageRect * pRect,
//		const uint8_t * pRedTone, const uint8_t * pGreenTone,
//		const uint8_t * pBlueTone, const uint8_t * pAlphaTone ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglApplyToneImageFilter,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgbuf,
				arg[0].i, imgbuf of sglApplyToneImageFilter ) ;
	SGLImageRect *	pRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(SGLImageRect) ) ;
	uint8_t *	pRedTone =
		(uint8_t*) context->AtomicTranslateAddress
							( arg[2].i, sizeof(uint8_t) * 0x100 ) ;
	uint8_t *	pGreenTone =
		(uint8_t*) context->AtomicTranslateAddress
							( arg[3].i, sizeof(uint8_t) * 0x100 ) ;
	uint8_t *	pBlueTone =
		(uint8_t*) context->AtomicTranslateAddress
							( arg[4].i, sizeof(uint8_t) * 0x100 ) ;
	uint8_t *	pAlphaTone =
		(uint8_t*) context->AtomicTranslateAddress
							( arg[5].i, sizeof(uint8_t) * 0x100 ) ;
	//
	SGLImageBuffer	imgbuf ;
	pimgbuf->ToImageBuffer( context, imgbuf ) ;
	//
	context->m_regset[regAcc].i =
		sglApplyToneImageFilter
			( imgbuf, pRect, pRedTone, pGreenTone, pBlueTone, pAlphaTone ) ;
	//
	return	NULL ;
}

// SGLError sglBlendWithAlphaChannel
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgAlpha,
//		int32_t fxAlphaCoefficient = 0x100,
//		int32_t fxAlphaIntercept = 0,
//		int xPos = 0, int yPos = 0,
//		const SGLImageRect * pSrcRect = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglBlendWithAlphaChannel,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglBlendWithAlphaChannel ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgAlpha,
				arg[1].i, imgAlpha of sglBlendWithAlphaChannel ) ;
	SGLImageRect *	pSrcRect =
		(SGLImageRect*) context->AtomicTranslateAddress
								( arg[6].i, sizeof(SGLImageRect) ) ;
	//
	SGLImageBuffer	imgDst, imgAlpha ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgAlpha->ToImageBuffer( context, imgAlpha ) ;
	//
	context->m_regset[regAcc].i =
		sglBlendWithAlphaChannel
			( imgDst, imgAlpha,
				(int32_t) arg[2].i, (int32_t) arg[3].i,
				(int) arg[4].i, (int) arg[5].i, pSrcRect ) ;
	//
	return	NULL ;
}

// SGLError sglGaussianBlur
//	( const SGLImageBuffer& imgDst,
//		const SGLImageBuffer& imgSrc,
//		const SGLImageBuffer& imgTemp,
//		float32_t fpGaussianValue, size_t nBlurWidth ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_sglGaussianBlur,context,arg)
{
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgDst,
				arg[0].i, imgDst of sglGaussianBlur ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgSrc,
				arg[1].i, imgSrc of sglGaussianBlur ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_IMAGE_BUFFER, pimgTemp,
				arg[2].i, imgTemp of sglGaussianBlur ) ;
	//
	SGLImageBuffer	imgDst, imgSrc, imgTemp ;
	pimgDst->ToImageBuffer( context, imgDst ) ;
	pimgSrc->ToImageBuffer( context, imgSrc ) ;
	pimgTemp->ToImageBuffer( context, imgTemp ) ;
	//
	context->m_regset[regAcc].i =
		sglGaussianBlur
			( imgDst, imgSrc, imgTemp,
				(float32_t) arg[3].f, (size_t) arg[4].i ) ;
	//
	return	NULL ;
}

#endif
