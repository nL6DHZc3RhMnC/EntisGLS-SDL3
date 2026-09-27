
#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_heap_memory.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 画像バッファオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImageBufferInterface, ESLObject )

// クリア通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageBufferInterface::ClearBuffer
			( SGLImageBuffer * pImageBuf, SGLPalette pxClear )
{
	return	UpdateBuffer( pImageBuf ) ;
}

// 画像バッファの再確保通知
//////////////////////////////////////////////////////////////////////////////
bool SGLImageBufferInterface::OnImageReBuffered( SGLImageBuffer * pImageBuf )
{
	return	true ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像バッファ
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( SGLImageBuffer *	SGLImageBuffer::ptrFirstBufChain = NULL ) ;

#if	defined(__DEBUG__)

// チェーンに追加
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::AddIntoChainFirst( void )
{
	ESLAssert( ptrPrevBufChain == NULL ) ;
	ESLAssert( ptrNextBufChain == NULL ) ;
	ESLAssert( ptrFirstBufChain != this ) ;
	QuickLock() ;
	ptrNextBufChain = ptrFirstBufChain ;
	ptrFirstBufChain = this ;
	if ( ptrNextBufChain != NULL )
	{
		ptrNextBufChain->ptrPrevBufChain = this ;
	}
	QuickUnlock() ;
}

// チェーンから分離
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::DetachFromChain( void )
{
	if ( (ptrPrevBufChain != NULL)
		|| (ptrNextBufChain != NULL)
		|| (ptrFirstBufChain == this) )
	{
		QuickLock() ;
		if ( ptrFirstBufChain == this )
		{
			ESLAssert( ptrPrevBufChain == NULL ) ;
			ptrFirstBufChain = ptrNextBufChain ;
		}
		if ( ptrNextBufChain != NULL )
		{
			ptrNextBufChain->ptrPrevBufChain = ptrPrevBufChain ;
		}
		if ( ptrPrevBufChain != NULL )
		{
			ptrPrevBufChain->ptrNextBufChain = ptrNextBufChain ;
		}
		ptrNextBufChain = NULL ;
		ptrPrevBufChain = NULL ;
		QuickUnlock() ;
	}
}

// 画像バッファチェーンをデバッグ出力へダンプ
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::DumpAllBufferChain( void )
{
	SGLImageBuffer *	pNextBuf ;
	size_t	countBufs = 0 ;
	int64_t	nBufBytes = 0 ;
	QuickLock() ;
	pNextBuf = ptrFirstBufChain ;
	while ( pNextBuf != NULL )
	{
		Trace( "image buffer #%08X: %dx%d[pixels], "
				"%d[bpp] (format:%08X), pos(%d,%d), ref to #%08X, ref-ed %d\n",
			pNextBuf,
			pNextBuf->width, pNextBuf->height,
			pNextBuf->depth, pNextBuf->format,
			pNextBuf->ptOrigin.x, pNextBuf->ptOrigin.y,
			pNextBuf->ptrRefOriginal, pNextBuf->countRef ) ;
		nBufBytes += (pNextBuf->depth >> 3)
						* pNextBuf->width * pNextBuf->height ;
		pNextBuf = pNextBuf->ptrNextBufChain ;
		countBufs ++ ;
	}
	if ( countBufs != 0 )
	{
		Trace( "there are %d image buffers, %d [kB]\n\n",
					countBufs, (size_t) (nBufBytes >> 10) ) ;
	}
	QuickUnlock() ;
}

// 画像バッファチェーンを切る
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer * SGLImageBuffer::SaveBufferChain( void )
{
	SGLImageBuffer *	pSaveBuf = NULL ;
	QuickLock() ;
	pSaveBuf = ptrFirstBufChain ;
	ptrFirstBufChain = NULL ;
	QuickUnlock() ;
	//
	sglAddReferenceImageBuffer( pSaveBuf ) ;
	return	pSaveBuf ;
}

// 画像バッファチェーンを復元する
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::RestoreBufferChain( SGLImageBuffer * pImageBuf )
{
	if ( pImageBuf != NULL )
	{
		SGLImageBuffer *	pNextBuf ;
		SGLImageBuffer *	pLastBuf = NULL ;
		QuickLock() ;
		pNextBuf = ptrFirstBufChain ;
		while ( pNextBuf != NULL )
		{
			pLastBuf = pNextBuf ;
			pNextBuf = pNextBuf->ptrNextBufChain ;
		}
		if ( pLastBuf != NULL )
		{
			pLastBuf->ptrNextBufChain = pImageBuf ;
			pImageBuf->ptrPrevBufChain = pLastBuf ;
		}
		QuickUnlock() ;
		//
		sglReleaseImageBuffer( pImageBuf ) ;
	}
}

#endif

// バッファ矩形参照
//////////////////////////////////////////////////////////////////////////////
bool SGLImageBuffer::GetClippedBuffer
	( const SGLImageBuffer& imgbuf, const SGLImageRect & rect )
{
	SGLImageInfo::operator = ( imgbuf ) ;
	//
	ptrPalette = imgbuf.ptrPalette ;
	ptrBuffer = imgbuf.ptrBuffer ;
	flagsBuffer = imgbuf.flagsBuffer ;
	//
	SGLRect	rctClipped( 0, 0, imgbuf.width - 1, imgbuf.height - 1 ) ;
	if ( rctClipped &= SGLRect( rect ) )
	{
		width = rctClipped.GetWidth() ;
		height = rctClipped.GetHeight() ;
		//
		if ( ptrBuffer != NULL )
		{
			ptrBuffer += pitchLine * rctClipped.top
							+ pitchPixel * rctClipped.left ;
		}
		rctRefOriginal = rctClipped ;
		return	true ;
	}
	else
	{
		width = 0 ;
		height = 0 ;
		return	false ;
	}
}

// 画像バッファのクリア通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageBuffer::NotifyClearImageObject( SGLPalette pxClear )
{
	QuickLock() ;
	SGLError	errResult = sglErrSuccess ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	QuickUnlock() ;
	while ( pNext != NULL )
	{
		SGLError	err = pNext->ClearBuffer( this, pxClear ) ;
		if ( err )
		{
			errResult = err ;
		}
		QuickLock() ;
		pNext = pNext->m_ptrNext ;
		QuickUnlock() ;
	}
	if ( ptrRefOriginal != NULL )
	{
		return	ptrRefOriginal->NotifyClearImageObject( pxClear ) ;
	}
	return	errResult ;
}

// 画像バッファの更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageBuffer::UpdateImageObject( const SGLImageRect * pRect )
{
	QuickLock() ;
	SGLError	errResult = sglErrSuccess ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	QuickUnlock() ;
	while ( pNext != NULL )
	{
		SGLError	err = pNext->UpdateBuffer( this, pRect ) ;
		if ( err )
		{
			errResult = err ;
		}
		QuickLock() ;
		pNext = pNext->m_ptrNext ;
		QuickUnlock() ;
	}
	if ( ptrRefOriginal != NULL )
	{
		SGLImageRect	rctSub = GetImageRect() ;
		if ( pRect != NULL )
		{
			rctSub = *pRect ;
		}
		rctSub += rctRefOriginal.GetPosition() ;
		return	ptrRefOriginal->UpdateImageObject( &rctSub ) ;
	}
	return	errResult ;
}

// 画像バッファの更新を画像オブジェクトへ反映
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageBuffer::CommitImageObject( void )
{
	QuickLock() ;
	SGLError	errResult = sglErrSuccess ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	QuickUnlock() ;
	while ( pNext != NULL )
	{
		SGLError	err = pNext->CommitBuffer( this ) ;
		if ( err )
		{
			errResult = err ;
		}
		QuickLock() ;
		pNext = pNext->m_ptrNext ;
		QuickUnlock() ;
	}
	if ( ptrRefOriginal != NULL )
	{
		return	ptrRefOriginal->CommitImageObject() ;
	}
	return	errResult ;
}

// 画像オブジェクトの更新を画像バッファへ反映
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageBuffer::ReflectImageObject( const SGLImageRect * pRect )
{
	QuickLock() ;
	SGLError	errResult = sglErrSuccess ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	QuickUnlock() ;
	while ( pNext != NULL )
	{
		SGLError	err = pNext->ReflectBuffer( this, pRect ) ;
		if ( err )
		{
			errResult = err ;
		}
		QuickLock() ;
		pNext = pNext->m_ptrNext ;
		QuickUnlock() ;
	}
	if ( ptrRefOriginal != NULL )
	{
		SGLImageRect	rctSub = GetImageRect() ;
		if ( pRect != NULL )
		{
			rctSub = *pRect ;
		}
		rctSub += rctRefOriginal.GetPosition() ;
		return	ptrRefOriginal->ReflectImageObject( &rctSub ) ;
	}
	return	errResult ;
}

// ミップマップ化の設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageBuffer::MakeMipmap( void )
{
	QuickLock() ;
	SGLError	errResult = sglErrSuccess ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	QuickUnlock() ;
	while ( pNext != NULL )
	{
		SGLError	err = pNext->MakeMipmap() ;
		if ( err )
		{
			errResult = err ;
		}
		QuickLock() ;
		pNext = pNext->m_ptrNext ;
		QuickUnlock() ;
	}
	if ( ptrRefOriginal != NULL )
	{
		SGLError	err = ptrRefOriginal->MakeMipmap() ;
		if ( err )
		{
			errResult = err ;
		}
	}
	return	errResult ;
}

// 画像オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
SGLImageBufferInterface *
	SGLImageBuffer::GetImageObject
		( uint32_t idType, SGLImageBuffer*& pImageRef,
				SGLImageRect& rectRef, bool fNoRefImage ) const
{
	QuickLock() ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	while ( pNext != NULL )
	{
		if ( pNext->m_typeObject == idType )
		{
			pImageRef = (SGLImageBuffer*) this ;
			rectRef = GetImageRect() ;
			QuickUnlock() ;
			return	pNext ;
		}
		pNext = pNext->m_ptrNext ;
	}
	if ( !fNoRefImage & (ptrRefOriginal != NULL) )
	{
		SGLImageBufferInterface *	pBuf =
			ptrRefOriginal->GetImageObject
					( idType, pImageRef, rectRef, false ) ;
		if ( pBuf != NULL )
		{
			pImageRef = (SGLImageBuffer*) this ;
			rectRef.x += rctRefOriginal.x ;
			rectRef.y += rctRefOriginal.y ;
			rectRef.w = rctRefOriginal.w ;
			rectRef.h = rctRefOriginal.h ;
			QuickUnlock() ;
			return	pBuf ;
		}
	}
	QuickUnlock() ;
	return	NULL ;
}

// 画像オブジェクト追加
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::AddImageObject( SGLImageBufferInterface * pObject )
{
	QuickLock() ;
	pObject->m_ptrNext = ptrInterface ;
	ptrInterface = pObject ;
	QuickUnlock() ;
}

// 画像オブジェクト分離
//////////////////////////////////////////////////////////////////////////////
bool SGLImageBuffer::DetachImageObject( SGLImageBufferInterface * pObject )
{
	QuickLock() ;
	SGLImageBufferInterface *	pLast = NULL ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	while ( pNext != NULL )
	{
		if ( pNext == pObject )
		{
			if ( pLast != NULL )
			{
				pLast->m_ptrNext = pNext->m_ptrNext ;
			}
			else
			{
				ptrInterface = pNext->m_ptrNext ;
			}
			pNext->m_ptrNext = NULL ;
			QuickUnlock() ;
			return	true ;
		}
		pLast = pNext ;
		pNext = pNext->m_ptrNext ;
	}
	QuickUnlock() ;
	return	false ;
}

// 画像オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::DeleteImageObject( uint32_t idType )
{
	QuickLock() ;
	SGLImageBufferInterface *	pLast = NULL ;
	SGLImageBufferInterface *	pNext = ptrInterface ;
	while ( pNext != NULL )
	{
		if ( pNext->m_typeObject == idType )
		{
			if ( pLast != NULL )
			{
				pLast->m_ptrNext = pNext->m_ptrNext ;
			}
			else
			{
				ptrInterface = pNext->m_ptrNext ;
			}
			QuickUnlock() ;
			delete	pNext ;
			return ;
		}
		pLast = pNext ;
		pNext = pNext->m_ptrNext ;
	}
	QuickUnlock() ;
}

void SGLImageBuffer::DeleteAllImageObject( void )
{
	QuickLock() ;
	SGLImageBufferInterface *	pObj = ptrInterface ;
	ptrInterface = NULL ;
	QuickUnlock() ;
	//
	while ( pObj != NULL )
	{
		QuickLock() ;
		ptrInterface = pObj->m_ptrNext ;
		pObj->m_ptrNext = NULL ;
		QuickUnlock() ;
		//
		delete	pObj ;
		pObj = ptrInterface ;
	}
}

// 画像バッファの更新通知・削除
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::NotifyReBufferedImage( void )
{
	if ( ptrInterface != NULL )
	{
		QuickLock() ;
		SGLImageBufferInterface *	pLast = NULL ;
		SGLImageBufferInterface *	pNext = ptrInterface ;
		while ( pNext != NULL )
		{
			SGLImageBufferInterface *	pTemp = pNext ;
			pNext = pNext->m_ptrNext ;
			//
			if ( pTemp->OnImageReBuffered( this ) )
			{
				if ( pLast != NULL )
				{
					pLast->m_ptrNext = pNext ;
				}
				else
				{
					ptrInterface = pNext ;
				}
				QuickUnlock() ;
				delete	pTemp ;
				QuickLock() ;
				pTemp = pLast ;
			}
			pLast = pTemp ;
		}
		QuickUnlock() ;
	}
}

// オブジェクト削除通知（関連オブジェクトの削除）
//////////////////////////////////////////////////////////////////////////////
void SGLImageBuffer::NotifyObjectDestroy( ESLObject * pObj )
{
	if ( ptrInterface != NULL )
	{
		QuickLock() ;
		SGLImageBufferInterface *	pLast = NULL ;
		SGLImageBufferInterface *	pNext = ptrInterface ;
		while ( pNext != NULL )
		{
			SGLImageBufferInterface *	pTemp = pNext ;
			pNext = pNext->m_ptrNext ;
			//
			if ( pTemp->OnDestroyObject( pObj ) )
			{
				if ( pLast != NULL )
				{
					pLast->m_ptrNext = pNext ;
				}
				else
				{
					ptrInterface = pNext ;
				}
				QuickUnlock() ;
				delete	pTemp ;
				QuickLock() ;
				pTemp = pLast ;
			}
			pLast = pTemp ;
		}
		QuickUnlock() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// ただの画像メモリ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLImageSystemMemory, SGLImageBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageSystemMemory::SGLImageSystemMemory( void )
{
	m_typeObject = imageObjectEntisGLS4Memory ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageSystemMemory::~SGLImageSystemMemory( void )
{
}

// クリア通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageSystemMemory::ClearBuffer
	( SGLImageBuffer * pImageBuf, SGLPalette pxClear )
{
	return	sglErrSuccess ;
}

// 更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageSystemMemory::UpdateBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	return	sglErrSuccess ;
}

// 更新確定処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageSystemMemory::CommitBuffer( SGLImageBuffer * pImageBuf )
{
	m_imginf = *pImageBuf ;
	m_imginf.pitchLine = m_imginf.width * m_imginf.pitchPixel ;
	//
	size_t	sizeMemory = m_imginf.height * m_imginf.pitchLine ;
	if ( sizeMemory > m_bufImage.GetLength() )
	{
		m_bufImage.SetLength( sizeMemory ) ;
	}
	return	sglErrSuccess ;
}

// 反映処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageSystemMemory::ReflectBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	return	sglErrSuccess ;
}

// ミップマップ化通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageSystemMemory::MakeMipmap( void )
{
	return	sglErrSuccess ;
}

// 関連オブジェクトの削除処理
//////////////////////////////////////////////////////////////////////////////
bool SGLImageSystemMemory::OnDestroyObject( ESLObject * pObj )
{
	return	false ;
}

// SGLImageObject からメモリ生成／取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * SGLImageSystemMemory::CommitMemoryOf
	( SGLImageBuffer * pImageBuf, SGLImageInfo*& pRefImage )
{
	if ( pImageBuf == NULL )
	{
		return	NULL ;
	}
	if ( pImageBuf->ptrBuffer != NULL )
	{
		pRefImage = pImageBuf ;
		return	pImageBuf->ptrBuffer ;
	}
	SGLImageBuffer *		pImageRef = NULL ;
	SGLImageRect			rectRef ;
	SGLImageSystemMemory *	pMemory =
		ESLTypeCast<SGLImageSystemMemory>
			( pImageBuf->GetImageObject
				( imageObjectEntisGLS4Memory, pImageRef, rectRef, true ) ) ;
	if ( pMemory == NULL )
	{
		ESLTrace( "create temporary image bufer by SGLImageSystemMemory::GetMemoryOf\n" ) ;
		pMemory = new SGLImageSystemMemory ;
		pImageBuf->AddImageObject( pMemory ) ;
		pImageRef = pImageBuf ;
	}
	pMemory->CommitBuffer( pImageRef ) ;
	//
	uint8_t *	pbytBuf = pMemory->m_bufImage.GetArray() ;
	if ( pbytBuf == NULL )
	{
		return	NULL ;
	}
	pRefImage = &(pMemory->m_imginf) ;
	pMemory->m_bufImage.FinishArray() ;
	return	pbytBuf + (rectRef.y * pMemory->m_imginf.pitchLine
						+ rectRef.x * pMemory->m_imginf.pitchPixel) ;
}

// SGLImageBuffer からメモリ取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * SGLImageSystemMemory::GetMemoryOf
	( SGLImageBuffer * pImageBuf, SGLImageInfo*& pRefImage )
{
	if ( pImageBuf == NULL )
	{
		return	NULL ;
	}
	if ( pImageBuf->ptrBuffer != NULL )
	{
		pRefImage = pImageBuf ;
		return	pImageBuf->ptrBuffer ;
	}
	SGLImageBuffer *		pImageRef = NULL ;
	SGLImageRect			rectRef ;
	SGLImageSystemMemory *	pMemory =
		ESLTypeCast<SGLImageSystemMemory>
			( pImageBuf->GetImageObject
				( imageObjectEntisGLS4Memory, pImageRef, rectRef, true ) ) ;
	if ( pMemory == NULL )
	{
		return	NULL ;
	}
	pMemory->CommitBuffer( pImageRef ) ;
	//
	uint8_t *	pbytBuf = pMemory->m_bufImage.GetArray() ;
	if ( pbytBuf == NULL )
	{
		return	NULL ;
	}
	pRefImage = &(pMemory->m_imginf) ;
	pMemory->m_bufImage.FinishArray() ;
	return	pbytBuf + (rectRef.y * pMemory->m_imginf.pitchLine
						+ rectRef.x * pMemory->m_imginf.pitchPixel) ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像バッファ管理関数群
//////////////////////////////////////////////////////////////////////////////

// 2^n サイズ正規化
//////////////////////////////////////////////////////////////////////////////
uint32_t SakuraGL::sglNormalizeScalePowerBy2( uint32_t nSize )
{
	if ( nSize & (nSize - 1) )
	{
		// nSize は 0 でも 2 の累乗でもない
		uint32_t	temp = nSize | (nSize >> 1) ;
		temp = temp | (temp >> 2) ;
		temp = temp | (temp >> 4) ;
		temp = temp | (temp >> 8) ;
		temp = temp | (temp >> 16) ;
		return	temp + 1 ;
	}
	return	nSize ;
}

// デフォルト RGBA 形式の設定
//////////////////////////////////////////////////////////////////////////////
ESL_DLL_DECL( uint32_t	SakuraGL::g_defaultImageFormat = formatImageARGB ) ;
ESL_DLL_DECL( uint32_t	SakuraGL::g_defaultImageDepth = 32 ) ;

SGLError SakuraGL::sglSetDefaultImageFormat( uint32_t format, uint32_t depth )
{
	g_defaultImageFormat = format ;
	g_defaultImageDepth = depth ;
	return	sglErrSuccess ;
}

// 画像バッファに使用されたメモリ情報
//////////////////////////////////////////////////////////////////////////////
ESL_DLL_DECL( atomic_int_t	SakuraGL::g_countUsedImageBuffer = 0 ) ;
ESL_DLL_DECL( atomic_int_t	SakuraGL::g_bytesUsedImageBuffer = 0 ) ;
ESL_DLL_DECL( atomic_int_t	SakuraGL::g_bytesMaxUsedImageBuffer = 0 ) ;

// 画像バッファメモリ関数
//////////////////////////////////////////////////////////////////////////////
uint8_t * SakuraGL::sglAllocateMemory( size_t nBytes )
{
	atomic_int_t *	pBuf =
	#if	defined(__DEBUG__)
		(atomic_int_t*) eslHeapAllocate
			( nBytes + sizeof(atomic_int_t) * 4,
					SSystem::SHeapMemory::flagNoRetryAlloc ) ;
		if ( pBuf != nullptr )
		{
			pBuf[0] = 0x454E5453 ;
			pBuf[1] = 0xCCCCCCCC ;
			pBuf += 2 ;
		}
	#else
		(atomic_int_t*) eslHeapAllocate
			( nBytes + sizeof(atomic_int_t) * 2,
					SSystem::SHeapMemory::flagNoRetryAlloc ) ;
	#endif
	if ( pBuf == nullptr )
	{
		return	nullptr ;
	}
	pBuf[0] = 1 ;
	pBuf[1] = (atomic_int_t) nBytes ;
	//
	AtomicAdd( &g_countUsedImageBuffer, 1 ) ;
	atomic_int_t	bytesUsed =
		AtomicAdd( &g_bytesUsedImageBuffer, (atomic_int_t) nBytes ) ;
	if ( bytesUsed > g_bytesMaxUsedImageBuffer )
	{
		g_bytesMaxUsedImageBuffer = bytesUsed ;
	}
	//
	return	(uint8_t*) (pBuf + 2) ;
}

wchar_t * SakuraGL::sglAllocateStringMemory( const wchar_t * pwszString )
{
	if ( pwszString == NULL )
	{
		return	NULL ;
	}
	size_t	nLen = 0 ;
	while ( pwszString[nLen] != 0 )
	{
		nLen ++ ;
	}
	const size_t	nBytes = (nLen + 1) * sizeof(wchar_t) ;
	wchar_t *	pBuf = (wchar_t*) sglAllocateMemory( nBytes ) ;
	eslCopyMemory( pBuf, pwszString, nBytes ) ;
	return	pBuf ;
}

void SakuraGL::sglFreeMemory( uint8_t * pbytMem )
{
	if ( pbytMem != NULL )
	{
		atomic_int_t *	pBuf = (atomic_int_t*) pbytMem ;
		#if	defined(__DEBUG__)
		ESLAssert( pBuf[-4] == 0x454E5453 ) ;
		#endif
		if ( AtomicSub( pBuf - 2, 1 ) == 0 )
		{
			ESLVerify( AtomicSub( &g_countUsedImageBuffer, 1 ) >= 0 ) ;
			ESLVerify( AtomicSub( &g_bytesUsedImageBuffer, (atomic_int_t) pBuf[-1] ) >= 0 ) ;
			//
			#if	defined(__DEBUG__)
			eslHeapFree( pBuf - 4 ) ;
			#else
			eslHeapFree( pBuf - 2 ) ;
			#endif
		}
	}
}

void SakuraGL::sglAddRefMemory( uint8_t * pbytMem )
{
	atomic_int_t *	pBuf = (atomic_int_t*) pbytMem ;
	#if	defined(__DEBUG__)
	ESLAssert( pBuf[-4] == 0x454E5453 ) ;
	#endif
	AtomicAdd( pBuf - 2, 1 ) ;
}

// 画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer * SakuraGL::sglCreateImageBuffer
	( const SGLImageInfo& imginf, uint64_t nFlags )
{
	//
	// 画像情報正規化
	//
	SGLImageBuffer *	pBuffer = new SGLImageBuffer( imginf ) ;
	#if	defined(__DEBUG__)
	pBuffer->AddIntoChainFirst() ;
	#endif
	pBuffer->flagsBuffer = nFlags ;
	pBuffer->nFrameCount = 1 ;
	pBuffer->sizeFrame.w = imginf.width ;
	pBuffer->sizeFrame.h = imginf.height ;
	//
	SGLSize	sizeBuffer = imginf.GetImageSize() ;
	if ( (imginf.format & formatImageTypeMask) == formatImageDefaultRGBA )
	{
		pBuffer->format = g_defaultImageFormat ;
		pBuffer->depth = g_defaultImageDepth ;
		nFlags &= ~flagImageNormalDepth ;
	}
	if ( nFlags & flagImageNormalDepth )
	{
		pBuffer->depth = 32 ;
	}
	if ( imginf.format & formatImageFlagSideBySide )
	{
		sizeBuffer.w = sizeBuffer.w * 2 ;
	}
	if ( pBuffer->pitchPixel < (int32_t) ((pBuffer->depth + 0x07) >> 3) )
	{
		pBuffer->pitchPixel = (int32_t) ((pBuffer->depth + 0x07) >> 3) ;
	}
	const uint32_t	pitchNormalLine = ((pBuffer->depth * sizeBuffer.w + 0x07) >> 3) ;
	if ( pBuffer->pitchLine < (int32_t) pitchNormalLine )
	{
		pBuffer->pitchLine = (int32_t) (pitchNormalLine + 0x07) & ~0x07 ;
	}
	if ( nFlags & flagImageNormalSize )
	{
		sizeBuffer.w = sglNormalizeScalePowerBy2( sizeBuffer.w ) ;
		sizeBuffer.h = sglNormalizeScalePowerBy2( sizeBuffer.h ) ;
		pBuffer->width = sizeBuffer.w ;
		pBuffer->height = sizeBuffer.h ;
		pBuffer->pitchLine = pBuffer->pitchPixel * sizeBuffer.w ;
	}
	else if ( nFlags & flagImageNormalPitch )
	{
		pBuffer->pitchLine = pitchNormalLine ;
	}
	if ( sizeBuffer.w == 0 )
	{
		sizeBuffer.w = 1 ;
		pBuffer->width = sizeBuffer.w ;
		pBuffer->pitchLine = pBuffer->pitchPixel * sizeBuffer.w ;
	}
	if ( sizeBuffer.h == 0 )
	{
		sizeBuffer.h = 1 ;
		pBuffer->height = sizeBuffer.h ;
	}
	//
	// バッファ確保
	//
	if ( pBuffer->format & formatImageFlagPalette )
	{
		SGLPalette *	pPalette =
				pBuffer->ptrPalette = new SGLPalette[0x100] ;
		for ( int i = 0; i < 0x100; i ++ )
		{
			pPalette->argb.Blue		= (uint8_t) i ;
			pPalette->argb.Green	= (uint8_t) i ;
			pPalette->argb.Red		= (uint8_t) i ;
			pPalette->argb.Alpha	= (uint8_t) i ;
			pPalette ++ ;
		}
	}
	if ( !(nFlags & flagImageNoBuffer) )
	{
		size_t	bytesImage = pBuffer->pitchLine * sizeBuffer.h ;
		pBuffer->ptrBuffer = sglAllocateMemory( bytesImage ) ;
		if ( pBuffer->ptrBuffer != nullptr )
		{
			eslFillMemory( pBuffer->ptrBuffer, 0, bytesImage ) ;
		}
	}
	//
	// 正規化バッファ参照
	//
	if ( ((uint32_t) sizeBuffer.w != imginf.width)
		|| ((uint32_t) sizeBuffer.h != imginf.height) )
	{
		SGLImageRect	rctClip = imginf.GetImageRect() ;
		pBuffer->countRef = 0 ;
		return	sglCreateReferenceImageBuffer( pBuffer, &rctClip ) ;
	}
	else
	{
		pBuffer->countRef = 1 ;
	}
	return	pBuffer ;
}

// 画像参照生成
//////////////////////////////////////////////////////////////////////////////
SGLImageBuffer * SakuraGL::sglCreateReferenceImageBuffer
	( SGLImageBuffer * pImage,
		const SGLImageRect * pRect, int iFrame, int iSide )
{
	SGLImageBuffer *	pBuffer = new SGLImageBuffer ;
	#if	defined(__DEBUG__)
	pBuffer->AddIntoChainFirst() ;
	#endif
	iFrame = 0 ;
	if ( pImage->ptrRefOriginal != NULL )
	{
		bool	fStereoBoth =
			(iSide == stereoImageBoth)
				& ((pImage->ptrRefOriginal->format
							& formatImageFlagSideBySide) != 0) ;
		if ( (iSide < 0) | (iSide >= 2)
			| !(pImage->ptrRefOriginal->format & formatImageFlagSideBySide) )
		{
			iSide = 0 ;
		}
		SGLImageRect	irctClip = pImage->rctRefOriginal ;
		if ( pRect != NULL )
		{
			SGLRect	rectClip = *pRect ;
			rectClip += pImage->rctRefOriginal.GetPosition() ;
			rectClip &= SGLRect( irctClip ) ;
			irctClip = rectClip ;
		}
		if ( pBuffer->GetClippedBuffer
			( *(pImage->ptrRefOriginal), irctClip ) )
		{
			SGLSize	sizeRefOrg = pImage->ptrRefOriginal->GetImageSize() ;
			pBuffer->ptrRefOriginal = pImage->ptrRefOriginal ;
			pBuffer->ptrBuffer +=
				pBuffer->pitchPixel * (iSide * sizeRefOrg.w)
					+ pBuffer->pitchLine * (iFrame * sizeRefOrg.h) ;
		}
		else
		{
//			pBuffer->ptrRefOriginal = NULL ;
		}
		pBuffer->flagsBuffer =
			(pBuffer->flagsBuffer & ~(flagImageCubeIndexMask | flagImageLayerIndexMask))
			| (pImage->flagsBuffer & (flagImageCubeIndexMask | flagImageLayerIndexMask)) ;
		if ( fStereoBoth )
		{
			pBuffer->width <<= 1 ;
		}
	}
	else
	{
		bool	fStereoBoth =
			(iSide == stereoImageBoth)
				& ((pImage->format & formatImageFlagSideBySide) != 0) ;
		if ( (iSide < 0) | (iSide >= 2)
			| !(pImage->format & formatImageFlagSideBySide) )
		{
			iSide = 0 ;
		}
		pBuffer->ptrRefOriginal = pImage ;
		if ( pRect != NULL )
		{
			pBuffer->GetClippedBuffer( *pImage, *pRect ) ;
		}
		else
		{
			SGLImageRect	rectImage = pImage->GetImageRect() ;
			pBuffer->GetClippedBuffer( *pImage, rectImage ) ;
			pBuffer->rctRefOriginal = pImage->GetImageRect() ;
		}
		SGLSize	sizeRefOrg = pImage->GetImageSize() ;
		if ( pBuffer->ptrBuffer != NULL )
		{
			pBuffer->ptrBuffer +=
				pBuffer->pitchPixel * (iSide * sizeRefOrg.w)
					+ pBuffer->pitchLine * (iFrame * sizeRefOrg.h) ;
		}
		if ( fStereoBoth )
		{
			pBuffer->width <<= 1 ;
		}
	}
	if ( pBuffer->ptrRefOriginal != NULL )
	{
		sglAddReferenceImageBuffer( pBuffer->ptrRefOriginal ) ;
	}
	pBuffer->countRef = 1 ;
	return	pBuffer ;
}

// 画像参照先変更
//////////////////////////////////////////////////////////////////////////////
SGLError SakuraGL::sglMakeReferenceImageBuffer
	( SGLImageBuffer * pImage,
		SGLImageBuffer * pRefImage,
		const SGLImageRect * pRect, int iFrame, int iSide )
{
	//
	// 現在保持しているリソースを解放
	//
	if ( pImage->ptrRefOriginal != NULL )
	{
		SGLImageBuffer *	pOldRef = pImage->ptrRefOriginal ;
		pImage->ptrRefOriginal = NULL ;
		sglReleaseImageBuffer( pOldRef ) ;
	}
	else
	{
		delete []	pImage->ptrPalette ;
		//
		if ( pImage->ptrBuffer != NULL )
		{
			sglFreeMemory( pImage->ptrBuffer ) ;
			pImage->ptrBuffer = NULL ;
		}
	}
	//
	// 参照先正規化
	//
	SGLImageRect	rectRef ;
	if ( pRect != NULL )
	{
		rectRef = *pRect ;
	}
	else
	{
		rectRef.x = 0 ;
		rectRef.y = 0 ;
		rectRef.w = (int32_t) pRefImage->width ;
		rectRef.h = (int32_t) pRefImage->height ;
	}
	iFrame = 0 ;
	while ( pRefImage->ptrRefOriginal != NULL )
	{
		if ( (iSide < 0) | (iSide >= 2)
			| !(pRefImage->ptrRefOriginal->format & formatImageFlagSideBySide) )
		{
			iSide = 0 ;
		}
		rectRef += pImage->rctRefOriginal.GetPosition() ;
		pRefImage = pRefImage->ptrRefOriginal ;
	}
	//
	// 参照設定
	//
	pImage->GetClippedBuffer( *pRefImage, rectRef ) ;
	pImage->ptrRefOriginal = pRefImage ;
	//
	sglAddReferenceImageBuffer( pRefImage ) ;
	//
	return	sglErrSuccess ;
}

// 画像バッファ正規化
//////////////////////////////////////////////////////////////////////////////
SGLError SakuraGL::sglNormalizeImageBuffer
	( SGLImageBuffer * pImage, uint64_t nFlags )
{
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// 情報正規化
	//
	SGLImageBuffer *	pOrg = pImage ;
	QuickLock() ;
	while ( pOrg->ptrRefOriginal != NULL )
	{
		pOrg = pOrg->ptrRefOriginal ;
	}
	QuickUnlock() ;
	//
	SGLSize	szNormal( pOrg->width, pOrg->height ) ;
	int		pxdNormal = pOrg->pitchPixel ;
	bool	flagReBuffer = false ;
	if ( !(pImage->format & formatImageFlagS3TC) )
	{
		if ( nFlags & flagImageNormalDepth )
		{
			pxdNormal = (pxdNormal + 0x03) & ~0x03 ;
		}
		if ( nFlags & flagImageNormalSize )
		{
			szNormal.w = sglNormalizeScalePowerBy2( szNormal.w ) ;
			szNormal.h = sglNormalizeScalePowerBy2( szNormal.h ) ;
		}
		else if ( (nFlags & flagImageNormalPitch) && (pOrg->ptrBuffer != nullptr) )
		{
			flagReBuffer = (pOrg->pitchLine != (szNormal.w * pxdNormal)) ;
		}
	}
	const uint64_t	nAddFlagMask = flagImageMipmap
									| flagImageCompressedTexture
									| flagImageNeedSampleNoSmooth
									| flagImageNeedSampleTiling ;
	pImage->flagsBuffer |= (nFlags & nAddFlagMask) ;
	pOrg->flagsBuffer |= nFlags & nAddFlagMask ;
	if ( nFlags & flagImageCompressedTexture )
	{
		pImage->flagsBuffer &= ~flagImageCompressionFormatMask ;
		pOrg->flagsBuffer &= ~flagImageCompressionFormatMask ;
		pImage->flagsBuffer |= (nFlags & flagImageCompressionFormatMask) ;
		pOrg->flagsBuffer |= (nFlags & flagImageCompressionFormatMask) ;
	}
	if ( ((uint32_t) szNormal.w != pOrg->width)
		| ((uint32_t) szNormal.h != pOrg->height)
		| (pxdNormal != pOrg->pitchPixel) | flagReBuffer )
	{
		ESLAssert( !(pImage->format & formatImageFlagS3TC) ) ;
		//
		// 画像バッファ生成
		//
		SGLImageInfo	infNormal ;
		infNormal.format = pOrg->format ;
		infNormal.depth = pxdNormal * 8 ;
		infNormal.width = szNormal.w ;
		infNormal.height = szNormal.h ;
		infNormal.ptOrigin = pOrg->ptOrigin ;
		infNormal.pitchPixel = pxdNormal ;
		infNormal.pitchLine = pxdNormal * szNormal.w ;
		//
		int	nImageFlags = (int) pImage->flagsBuffer ;
		if ( pOrg->ptrBuffer == NULL )
		{
			nImageFlags |= flagImageNoBuffer ;
		}
		SGLImageBuffer *
			pNormalized = sglCreateImageBuffer( infNormal, nImageFlags ) ;
		if ( pNormalized == NULL )
		{
			return	sglErrFailed ;
		}
		SGLSmartImage	simgNormalized = pNormalized ;
		//
		// 画像複製
		//
		if ( pOrg->ptrBuffer != NULL )
		{
			sglConvertImageBuffer( *pNormalized, *pImage ) ;
			if ( pOrg == pImage )
			{
				sglFreeMemory( pImage->ptrBuffer ) ;
				pImage->ptrBuffer = NULL ;
			}
		}
		if ( (pNormalized->ptrPalette != NULL)
			&& (pImage->ptrPalette != NULL) )
		{
			eslMoveMemory
				( pNormalized->ptrPalette,
					pImage->ptrPalette, sizeof(SGLPalette) * 0x100 ) ;
		}
		//
		// 古い画像オブジェクト削除
		//
		pImage->NotifyReBufferedImage() ;
		//
		if ( pImage->ptrRefOriginal != NULL )
		{
			sglReleaseImageBuffer( pImage->ptrRefOriginal ) ;
			pImage->ptrRefOriginal = NULL ;
		}
		else
		{
			delete []	pImage->ptrPalette ;
			pImage->ptrPalette = NULL ;
		}
		//
		// 参照設定
		//
		SGLImageRect	irctOrg = pImage->GetImageRect() ;
		if ( !pImage->GetClippedBuffer( *pNormalized, irctOrg ) )
		{
			return	sglErrFailed ;
		}
		sglAddReferenceImageBuffer( pNormalized ) ;
		pImage->ptrRefOriginal = pNormalized ;
	}
	return	sglErrSuccess ;
}

// 参照カウンタ加算
//////////////////////////////////////////////////////////////////////////////
SGLError SakuraGL::sglAddReferenceImageBuffer( SGLImageBuffer * pImage )
{
	if ( pImage == NULL )
	{
		return	sglErrInvalidParam ;
	}
	AtomicAdd( &(pImage->countRef), 1 ) ;
	return	sglErrSuccess ;
}

// 参照カウンタ減算／画像バッファ削除
//////////////////////////////////////////////////////////////////////////////
SGLError SakuraGL::sglReleaseImageBuffer( SGLImageBuffer * pImage )
{
	if ( pImage == NULL )
	{
		return	sglErrInvalidParam ;
	}
	if ( AtomicSub( &(pImage->countRef), 1 ) <= 0 )
	{
		pImage->DeleteAllImageObject() ;
		//
		SGLImageBuffer *	pRefImage = pImage->ptrRefOriginal ;
		pImage->ptrRefOriginal = NULL ;
		if ( pRefImage != NULL )
		{
			sglReleaseImageBuffer( pRefImage ) ;
		}
		else
		{
			size_t	bytesImage = pImage->pitchLine * pImage->height ;
			//
			delete []	pImage->ptrPalette ;
			//
			if ( pImage->ptrBuffer != NULL )
			{
				sglFreeMemory( pImage->ptrBuffer ) ;
			}
		}
		pImage->ptrPalette = NULL ;
		pImage->ptrBuffer = NULL ;
		//
		if ( pImage->pwszIdentity != NULL )
		{
			sglFreeMemory( (uint8_t*) pImage->pwszIdentity ) ;
			pImage->pwszIdentity = NULL ;
		}
		//
		delete	pImage ;
	}
	return	sglErrSuccess ;
}

// 画像バッファ（内部）の参照カウンタを加算しポインタを取得する（sglFreeMemory で解放）
//////////////////////////////////////////////////////////////////////////////
uint8_t * SakuraGL::LockImageBuffer( SGLImageBuffer * pImage )
{
	SGLImageBuffer *	pOrg = pImage ;
	uint8_t *	ptrBuf = NULL ;
	QuickLock() ;
	while ( pOrg->ptrRefOriginal != NULL )
	{
		pOrg = pOrg->ptrRefOriginal ;
	}
	if ( pOrg->ptrBuffer == NULL )
	{
		QuickUnlock() ;
		return	NULL ;
	}
	sglAddRefMemory( pOrg->ptrBuffer ) ;
	ptrBuf = pOrg->ptrBuffer ;
	QuickUnlock() ;
	return	ptrBuf ;
}

// 画像バッファ識別子設定
//////////////////////////////////////////////////////////////////////////////
void SakuraGL::SetImageBufferIdentity( SGLImageBuffer * pImage, const wchar_t * pwszID )
{
	QuickLock() ;
	if ( pImage->pwszIdentity != NULL )
	{
		sglFreeMemory( (uint8_t*) pImage->pwszIdentity ) ;
		pImage->pwszIdentity = NULL ;
	}
	pImage->pwszIdentity = sglAllocateStringMemory( pwszID ) ;
	QuickUnlock() ;
}

// 交差矩形参照
//////////////////////////////////////////////////////////////////////////////
SGLError SakuraGL::sglGetImageBufferIntersection
	( SGLImageBuffer& imgIsDst,
		SGLImageBuffer& imgIsSrc,
		const SGLImageBuffer& imgDstBuf,
		const SGLImageBuffer& imgSrcBuf,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	SGLRect	rectSrc = imgSrcBuf.GetImageRect() ;
	if ( pSrcRect != NULL )
	{
		if ( !(rectSrc &= SGLRect( *pSrcRect )) )
		{
			return	sglErrFailed ;
		}
	}
	SGLImageRect	irctDst
		( xPos, yPos, rectSrc.GetWidth(), rectSrc.GetHeight() ) ;
	if ( irctDst.x < 0 )
	{
		rectSrc.left += - irctDst.x ;
		irctDst.x = 0 ;
		irctDst.w = rectSrc.GetWidth() ;
	}
	if ( irctDst.y < 0 )
	{
		rectSrc.top += - irctDst.y ;
		irctDst.y = 0 ;
		irctDst.h = rectSrc.GetHeight() ;
	}
	if ( irctDst.x + irctDst.w > (int32_t) imgDstBuf.width )
	{
		irctDst.w = imgDstBuf.width - irctDst.x ;
	}
	if ( irctDst.y + irctDst.h > (int32_t) imgDstBuf.height )
	{
		irctDst.h = imgDstBuf.height - irctDst.y ;
	}
	if ( irctDst.IsEmpty() )
	{
		return	sglErrFailed ;
	}
	SGLImageRect	irctSrc
		( rectSrc.left, rectSrc.top, irctDst.w, irctDst.h ) ;
	if ( !imgIsDst.GetClippedBuffer( imgDstBuf, irctDst ) )
	{
		return	sglErrFailed ;
	}
	if ( !imgIsSrc.GetClippedBuffer( imgSrcBuf, irctSrc ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 画像バッファフィル
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglFillImageBuffer
	( const SGLImageBuffer& imgbuf,
		const SGLPalette& pxcmp, const SGLImageRect * pRect )
{
	const SGLImageBuffer *	pDst = &imgbuf ;
	SGLImageBuffer			imgTemp ;
	if ( pRect != NULL )
	{
		if ( !imgTemp.GetClippedBuffer( imgbuf, *pRect ) )
		{
			return	sglErrFailed ;
		}
		pDst = &imgTemp ;
	}
	SGLImageInfo	infDst = *pDst ;
	if ( pDst->pitchPixel == 4 )
	{
		uint32_t	pxFill = pxcmp.ui32 ;
		uint8_t *	pNextLine = pDst->ptrBuffer ;
		for ( uint32_t y = 0; y < infDst.height; ++ y )
		{
			uint32_t *	pNextPixel = (uint32_t*) pNextLine ;
			for ( uint32_t x = 0; x < infDst.width; ++ x )
			{
				*(pNextPixel ++) = pxFill ;
			}
			pNextLine += infDst.pitchLine ;
		}
	}
	else if ( pDst->pitchPixel == 3 )
	{
		uint8_t		pxFill0 = (uint8_t) pxcmp.ui32 ;
		uint8_t		pxFill1 = (uint8_t) (pxcmp.ui32 >> 8) ;
		uint8_t		pxFill2 = (uint8_t) (pxcmp.ui32 >> 16) ;
		uint8_t *	pNextLine = pDst->ptrBuffer ;
		for ( uint32_t y = 0; y < infDst.height; ++ y )
		{
			uint8_t *	pNextPixel = pNextLine ;
			for ( uint32_t x = 0; x < infDst.width; ++ x )
			{
				pNextPixel[0] = pxFill0 ;
				pNextPixel[1] = pxFill1 ;
				pNextPixel[2] = pxFill2 ;
				pNextPixel += 3 ;
			}
			pNextLine += infDst.pitchLine ;
		}
	}
	else if ( pDst->pitchPixel == 2 )
	{
		uint16_t	pxFill = (uint16_t) pxcmp.ui32 ;
		uint8_t *	pNextLine = pDst->ptrBuffer ;
		for ( uint32_t y = 0; y < infDst.height; ++ y )
		{
			uint16_t *	pNextPixel = (uint16_t*) pNextLine ;
			for ( uint32_t x = 0; x < infDst.width; ++ x )
			{
				*(pNextPixel ++) = pxFill ;
			}
			pNextLine += infDst.pitchLine ;
		}
	}
	else if ( pDst->pitchPixel == 1 )
	{
		uint32_t	widthBytes = infDst.width ;
		if ( infDst.depth < 8 )
		{
			widthBytes = (infDst.width * infDst.depth + 0x07) >> 3 ;
		}
		uint8_t		pxFill = (uint8_t) pxcmp.ui32 ;
		uint8_t *	pNextLine = pDst->ptrBuffer ;
		for ( uint32_t y = 0; y < infDst.height; ++ y )
		{
			uint8_t *	pNextPixel = pNextLine ;
			for ( uint32_t x = 0; x < widthBytes; ++ x )
			{
				*(pNextPixel ++) = pxFill ;
			}
			pNextLine += infDst.pitchLine ;
		}
	}
	else
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}
#endif

// 画像バッファ複製
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglCopyImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( imgDst.pitchPixel != imgSrc.pitchPixel )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nLineBytes =
		(uint32_t) (infDst.pitchPixel * infDst.width) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t	nPixelBytes = nLineBytes ;
		uint32_t	nPixelQQWords = (nPixelBytes >> 5) ;
		uint32_t	nPixelDWords = (nPixelBytes & 0x1F) >> 2 ;
		nPixelBytes &= 0x03 ;
		//
		uint64_t *	pDstNextPixel64 = (uint64_t*) pDstNextLine ;
		uint64_t *	pSrcNextPixel64 = (uint64_t*) pSrcNextLine ;
		if ( !(((ulong_ptr_t) pDstNextLine
				| (ulong_ptr_t) pSrcNextLine) & 0x07) )
		{
			while ( nPixelQQWords != 0 )
			{
				pDstNextPixel64[0] = pSrcNextPixel64[0] ;
				pDstNextPixel64[1] = pSrcNextPixel64[1] ;
				pDstNextPixel64[2] = pSrcNextPixel64[2] ;
				pDstNextPixel64[3] = pSrcNextPixel64[3] ;
				pDstNextPixel64 += 4 ;
				pSrcNextPixel64 += 4 ;
				nPixelQQWords -- ;
			}
		}
		else
		{
			nPixelDWords += nPixelQQWords << 3 ;
		}
		uint32_t *	pDstNextPixel32 = (uint32_t*) pDstNextPixel64 ;
		uint32_t *	pSrcNextPixel32 = (uint32_t*) pSrcNextPixel64 ;
		while ( nPixelDWords != 0 )
		{
			*(pDstNextPixel32 ++) = *(pSrcNextPixel32 ++) ;
			nPixelDWords -- ;
		}
		uint8_t *	pDstNextPixel = (uint8_t*) pDstNextPixel32 ;
		uint8_t *	pSrcNextPixel = (uint8_t*) pSrcNextPixel32 ;
		while ( nPixelBytes != 0 )
		{
			*(pDstNextPixel ++) = *(pSrcNextPixel ++) ;
			nPixelBytes -- ;
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 画像バッファ描画（ARGB 標準合成）
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglBlendImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel != 4)
			| (imgDst.pitchPixel != imgSrc.pitchPixel) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nLineBytes =
		(uint32_t) (infDst.pitchPixel * infDst.width) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t *	pDstNextPixel = (uint32_t*) pDstNextLine ;
		uint32_t *	pSrcNextPixel = (uint32_t*) pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; x ++ )
		{
			uint32_t	argbSrc = *(pSrcNextPixel ++) ;
			if ( argbSrc != 0 )
			{
				uint32_t	argbDstPixel = argbSrc ;
				if ( (argbSrc & 0xFF000000) != 0xFF000000 )
				{
				#if	defined(__COTOPHA__)
					argbDstPixel = *pDstNextPixel ;
					asm
					{
						REG LOAD	argbDstPixel
						REG LOAD	argbSrc
						REG ALLOC	dalpha : uint32
						REG ALLOC	argbDst : uint32
						REG ALLOC	rgbMask : uint32
						psrl.d		dalpha, argbSrc, 24
						xor			dalpha, #ff
						move		rgbMask, 0x00FFFFFF
						padd.d		dalpha, #one
						move		argbDst, 0xFF000000
						and			argbSrc, rgbMask
						xor			argbDst, argbDstPixel
						pshuf.w		dalpha, dalpha, 0
						punpack.lbw	argbDst, #zero
						pmul.lw		argbDst, dalpha
						move		argbDstPixel, 0xFF000000
						psrl.w		argbDst, argbDst, 8
						pcvt.uswb	argbDst, #zero
						padd.ub		argbDst, argbSrc
						xor			argbDstPixel, argbDst
						REG FLUSH	argbDstPixel
					}
				#else
					argbDstPixel =
						sglPackedColorBlend( *pDstNextPixel, argbSrc ) ;
				#endif
				}
				*pDstNextPixel = argbDstPixel ;
			}
			pDstNextPixel ++ ;
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 画像バッファ描画（ARGB 標準合成逆順）
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglBlendBackImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel != 4)
			| (imgDst.pitchPixel != imgSrc.pitchPixel) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nLineBytes =
		(uint32_t) (infDst.pitchPixel * infDst.width) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t *	pDstNextPixel = (uint32_t*) pDstNextLine ;
		uint32_t *	pSrcNextPixel = (uint32_t*) pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; x ++ )
		{
			uint32_t	argbSrc = *(pSrcNextPixel ++) ;
			uint32_t	argbDst = *pDstNextPixel ;
			uint32_t	argbDstPixel = argbSrc ;
			if ( argbDst != 0 )
			{
				argbDstPixel = argbDst ;
				if ( (argbDst & 0xFF000000) != 0xFF000000 )
				{
					argbDstPixel = sglPackedColorBlend( argbSrc, argbDst ) ;
				}
			}
			*(pDstNextPixel ++) = argbDstPixel ;
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 画像バッファ加算（ARGB 加算合成）
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglAdditionalBlendImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel != 4)
			| (imgDst.pitchPixel != imgSrc.pitchPixel) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nLineBytes =
		(uint32_t) (infDst.pitchPixel * infDst.width) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t *	pDstNextPixel = (uint32_t*) pDstNextLine ;
		uint32_t *	pSrcNextPixel = (uint32_t*) pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; x ++ )
		{
			uint32_t	argbSrc = *(pSrcNextPixel ++) ;
			if ( argbSrc != 0 )
			{
			#if	defined(__COTOPHA__)
				asm
				{
					REG LOAD	pDstNextPixel
					REG LOAD	argbSrc
					load.uint32		acc, [pDstNextPixel]
					padd.ub			argbSrc, acc
					store.uint32	[pDstNextPixel], acc
				}
			#else
				*pDstNextPixel =
					sglPackedColorAdd( *pDstNextPixel, argbSrc ) ;
			#endif
			}
			pDstNextPixel ++ ;
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 画像バッファ乗算（ARGBxARGB 乗算合成）
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglMultiplierBlendImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel != 4)
			| (imgDst.pitchPixel != imgSrc.pitchPixel) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nLineBytes =
		(uint32_t) (infDst.pitchPixel * infDst.width) ;
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == NULL) | (pSrcNextLine == NULL) )
	{
		return	sglErrInvalidParam ;
	}
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t *	pDstNextPixel = (uint32_t*) pDstNextLine ;
		uint32_t *	pSrcNextPixel = (uint32_t*) pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; x ++ )
		{
			uint32_t	argbSrc = *pSrcNextPixel ;
			if ( argbSrc != 0xFFFFFFFF )
			{
			#if	defined(__COTOPHA__)
				asm
				{
					REG LOAD		pDstNextPixel
					REG LOAD		argbSrc
					load.uint32		acc, [pDstNextPixel]
					move			r1, 0x0001000100010001
					punpack.lbw		argbSrc, #zero
					punpack.lbw		acc, #zero
					padd.w			argbSrc, r1
					pmul.lw			acc, argbSrc
					psrl.w			acc, acc, 8
					pcvt.uswb		acc, acc
					store.uint32	[pDstNextPixel], acc
				}
			#else
				uint8_t*	pDstPixel = (uint8_t*) pDstNextPixel ;
				uint8_t*	pSrcPixel = (uint8_t*) pSrcNextPixel ;
				pDstPixel[0] =
					(uint8_t) (((pDstPixel[0] + 1) * pSrcPixel[0]) >> 8) ;
				pDstPixel[1] =
					(uint8_t) (((pDstPixel[1] + 1) * pSrcPixel[1]) >> 8) ;
				pDstPixel[2] =
					(uint8_t) (((pDstPixel[2] + 1) * pSrcPixel[2]) >> 8) ;
				pDstPixel[3] =
					(uint8_t) (((pDstPixel[3] + 1) * pSrcPixel[3]) >> 8) ;
			#endif
			}
			pSrcNextPixel ++ ;
			pDstNextPixel ++ ;
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 画像バッファ乗算（ARGBxGray 乗算合成）
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglMultiplierARGBxGrayBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc,
		int xPos, int yPos, const SGLImageRect * pSrcRect )
{
	if ( (imgDst.pitchPixel != 4) || (imgSrc.pitchPixel != 1) )
	{
		return	sglErrInvalidParam ;
	}
	SGLImageBuffer	infDst, infSrc ;
	SGLError	err =
		sglGetImageBufferIntersection
			( infDst, infSrc, imgDst, imgSrc, xPos, yPos, pSrcRect ) ;
	if ( err )
	{
		return	err ;
	}
	uint8_t *	pDstNextLine = infDst.ptrBuffer ;
	uint8_t *	pSrcNextLine = infSrc.ptrBuffer ;
	if ( (pDstNextLine == nullptr) | (pSrcNextLine == nullptr) )
	{
		return	sglErrInvalidParam ;
	}
	for ( uint32_t y = 0; y < infDst.height; ++ y )
	{
		uint32_t *	pDstNextPixel = (uint32_t*) pDstNextLine ;
		uint8_t *	pSrcNextPixel = pSrcNextLine ;
		for ( uint32_t x = 0; x < infDst.width; x ++ )
		{
			uint32_t	graySrc = *pSrcNextPixel ;
			if ( graySrc != 0xFF )
			{
				uint8_t*	pDstPixel = (uint8_t*) pDstNextPixel ;
				pDstPixel[0] =
					(uint8_t) (((pDstPixel[0] + 1) * graySrc) >> 8) ;
				pDstPixel[1] =
					(uint8_t) (((pDstPixel[1] + 1) * graySrc) >> 8) ;
				pDstPixel[2] =
					(uint8_t) (((pDstPixel[2] + 1) * graySrc) >> 8) ;
				pDstPixel[3] =
					(uint8_t) (((pDstPixel[3] + 1) * graySrc) >> 8) ;
			}
			pSrcNextPixel ++ ;
			pDstNextPixel ++ ;
		}
		pDstNextLine += infDst.pitchLine ;
		pSrcNextLine += infSrc.pitchLine ;
	}
	return	sglErrSuccess ;
}
#endif

// 画像 1/2 拡大
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglEnlargeHalfImageBuffer
	( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc )
{
	if ( ((imgDst.pitchPixel != 4) | (imgSrc.pitchPixel != 4))
		& ((imgDst.pitchPixel != 1) | (imgSrc.pitchPixel != 1)) )
	{
		return	sglErrInvalidParam ;
	}
	ESLAssert( imgDst.pitchPixel == imgSrc.pitchPixel ) ;
	//
	SGLSize	sizeDst( (imgSrc.width + 1) / 2, (imgSrc.height + 1) / 2 ) ;
	if ( sizeDst.w > (int32_t) imgDst.width )
	{
		sizeDst.w = (int32_t) imgDst.width ;
	}
	if ( sizeDst.h > (int32_t) imgDst.height )
	{
		sizeDst.h = (int32_t) imgDst.height ;
	}
	SGLSize	sizeDstIn = sizeDst ;
	if ( sizeDstIn.w * 2 > (int32_t) imgSrc.width )
	{
		sizeDstIn.w -- ;
	}
	if ( sizeDstIn.h * 2 > (int32_t) imgSrc.height )
	{
		sizeDstIn.h -- ;
	}
	int			x, y ;
	uint8_t *	pbytSrcLine = imgSrc.ptrBuffer ;
	uint8_t *	pbytDstLine = imgDst.ptrBuffer ;
	int32_t		pitchSrcLine = imgSrc.pitchLine ;
	int32_t		pitchDstLine = imgDst.pitchLine ;
	if ( imgDst.pitchPixel == 4 )
	{
		for ( y = 0; y < sizeDstIn.h; y ++ )
		{
			uint32_t *	pdwSrc1 = (uint32_t*) pbytSrcLine ;
			uint32_t *	pdwSrc2 = (uint32_t*) (pbytSrcLine + pitchSrcLine) ;
			uint32_t *	pdwDst = (uint32_t*) pbytDstLine ;
			for ( x = 0; x < sizeDstIn.w; x ++ )
			{
				uint32_t	p0 = pdwSrc1[0] ;
				uint32_t	p1 = pdwSrc1[1] ;
				uint32_t	p2 = pdwSrc2[0] ;
				uint32_t	p3 = pdwSrc2[1] ;
				p0 = ((p0 >> 1) & 0x7F7F7F7F)
						+ ((p1 >> 1) & 0x7F7F7F7F)
						+ ((p0 & p1) & 0x01010101) ;
				p2 = ((p2 >> 1) & 0x7F7F7F7F)
						+ ((p3 >> 1) & 0x7F7F7F7F)
						+ ((p2 & p3) & 0x01010101) ;
				p0 = ((p0 >> 1) & 0x7F7F7F7F)
						+ ((p2 >> 1) & 0x7F7F7F7F)
						+ ((p0 & p2) & 0x01010101) ;
				pdwSrc1 += 2 ;
				pdwSrc2 += 2 ;
				*(pdwDst ++) = p0 ;
			}
			if ( x < sizeDst.w )
			{
				uint32_t	p0 = pdwSrc1[0] ;
				uint32_t	p2 = pdwSrc2[0] ;
				p0 = ((p0 >> 1) & 0x7F7F7F7F)
						+ ((p2 >> 1) & 0x7F7F7F7F)
						+ ((p0 & p2) & 0x01010101) ;
				*pdwDst = p0 ;
			}
			pbytSrcLine += pitchSrcLine * 2 ;
			pbytDstLine += pitchDstLine ;
		}
		if ( y < sizeDst.h )
		{
			uint32_t *	pdwSrc1 = (uint32_t*) pbytSrcLine ;
			uint32_t *	pdwDst = (uint32_t*) pbytDstLine ;
			for ( x = 0; x < sizeDstIn.w; x ++ )
			{
				uint32_t	p0 = pdwSrc1[0] ;
				uint32_t	p1 = pdwSrc1[1] ;
				p0 = ((p0 >> 1) & 0x7F7F7F7F)
						+ ((p1 >> 1) & 0x7F7F7F7F)
						+ ((p0 & p1) & 0x01010101) ;
				pdwSrc1 += 2 ;
				*(pdwDst ++) = p0 ;
			}
			if ( x < sizeDst.w )
			{
				*pdwDst = *pdwSrc1 ;
			}
		}
	}
	else if ( imgDst.pitchPixel == 1 )
	{
		for ( y = 0; y < sizeDstIn.h; y ++ )
		{
			uint8_t *	pbSrc1 = (uint8_t*) pbytSrcLine ;
			uint8_t *	pbSrc2 = (uint8_t*) (pbytSrcLine + pitchSrcLine) ;
			uint8_t *	pbDst = (uint8_t*) pbytDstLine ;
			for ( x = 0; x < sizeDstIn.w; x ++ )
			{
				uint32_t	p0 = pbSrc1[0] ;
				uint32_t	p1 = pbSrc1[1] ;
				uint32_t	p2 = pbSrc2[0] ;
				uint32_t	p3 = pbSrc2[1] ;
				p0 = ((p0 + p1) + (p2 + p3)) >> 2 ;
				pbSrc1 += 2 ;
				pbSrc2 += 2 ;
				*(pbDst ++) = (uint8_t) p0 ;
			}
			if ( x < sizeDst.w )
			{
				uint32_t	p0 = pbSrc1[0] ;
				uint32_t	p2 = pbSrc2[0] ;
				p0 = (p0 + p2) >> 1 ;
				*pbDst = (uint8_t) p0 ;
			}
			pbytSrcLine += pitchSrcLine * 2 ;
			pbytDstLine += pitchDstLine ;
		}
		if ( y < sizeDst.h )
		{
			uint8_t *	pbSrc1 = (uint8_t*) pbytSrcLine ;
			uint8_t *	pbDst = (uint8_t*) pbytDstLine ;
			for ( x = 0; x < sizeDstIn.w; x ++ )
			{
				uint32_t	p0 = pbSrc1[0] ;
				uint32_t	p1 = pbSrc1[1] ;
				p0 = (p0 + p1) >> 1 ;
				pbSrc1 += 2 ;
				*(pbDst ++) = (uint8_t) p0 ;
			}
			if ( x < sizeDst.w )
			{
				*pbDst = *pbSrc1 ;
			}
		}
	}
	return	sglErrSuccess ;
}
#endif

// 90度単位回転
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglOrthogonalRotateImageBuffer
	( const SGLImageBuffer& imgDst,
		const SGLImageBuffer& imgSrc, int degRotateAngle )
{
	int	nOddAngle = (degRotateAngle < 0) ? -45 : 45 ;
	int	nAngleIn90 = ((degRotateAngle + nOddAngle) / 90) & 0x03 ;
	if ( nAngleIn90 & 0x01 )
	{
		if ( (imgDst.width != imgSrc.height)
			|| (imgDst.height != imgSrc.width) )
		{
			return	sglErrFailed ;
		}
	}
	else
	{
		if ( (imgDst.width != imgSrc.width)
			|| (imgDst.height != imgSrc.height) )
		{
			return	sglErrFailed ;
		}
	}
	if ( imgDst.depth != imgSrc.depth )
	{
		return	sglErrFailed ;
	}
	SGLPoint	ptIAxisX, ptIAxisY, ptIOrg ;
	switch ( nAngleIn90 )
	{
	case	0:
		ptIAxisX.x = 1 ;
		ptIAxisX.y = 0 ;
		ptIAxisY.x = 0 ;
		ptIAxisY.y = 1 ;
		ptIOrg.x = 0 ;
		ptIOrg.y = 0 ;
		break ;
	case	1:
		ptIAxisX.x = 0 ;
		ptIAxisX.y = 1 ;
		ptIAxisY.x = -1 ;
		ptIAxisY.y = 0 ;
		ptIOrg.x = imgSrc.width - 1 ;
		ptIOrg.y = 0 ;
		break ;
	case	2:
		ptIAxisX.x = -1 ;
		ptIAxisX.y = 0 ;
		ptIAxisY.x = 0 ;
		ptIAxisY.y = -1 ;
		ptIOrg.x = imgSrc.width - 1 ;
		ptIOrg.y = imgSrc.height - 1 ;
		break ;
	case	3:
		ptIAxisX.x = 0 ;
		ptIAxisX.y = -1 ;
		ptIAxisY.x = 1 ;
		ptIAxisY.y = 0 ;
		ptIOrg.x = 0 ;
		ptIOrg.y = imgSrc.height - 1 ;
		break ;
	}
	size_t		widthDst = imgDst.width ;
	size_t		heightDst = imgDst.height ;
	size_t		depthDst = imgDst.depth ;
	ssize_t		pitchDstLine = imgDst.pitchLine ;
	ssize_t		pitchDstPixel = imgDst.pitchPixel ;
	ssize_t		pitchSrcHorz =
					ptIAxisX.x * imgSrc.pitchPixel
							+ ptIAxisX.y * imgSrc.pitchLine ;
	ssize_t		pitchSrcVert =
					ptIAxisY.x * imgSrc.pitchPixel
							+ ptIAxisY.y * imgSrc.pitchLine ;
	uint8_t *	pbytDstPixel = imgDst.ptrBuffer ;
	uint8_t *	pbytSrcPixel =
					imgSrc.ptrBuffer
						+ (ptIOrg.x * imgSrc.pitchPixel
								+ ptIOrg.y * imgSrc.pitchLine) ;
	for ( size_t y = 0; y < heightDst; y ++ )
	{
		uint8_t *	pbytDstNext = pbytDstPixel ;
		uint8_t *	pbytSrcNext = pbytSrcPixel ;
		if ( depthDst == 32 )
		{
			for ( size_t x = 0; x < widthDst; x ++ )
			{
				*((uint32_t*)pbytDstNext) = *((uint32_t*)pbytSrcNext) ;
				pbytDstNext += pitchDstPixel ;
				pbytSrcNext += pitchSrcHorz ;
			}
		}
		else if ( depthDst == 24 )
		{
			for ( size_t x = 0; x < widthDst; x ++ )
			{
				pbytDstNext[0] = pbytSrcNext[0] ;
				pbytDstNext[1] = pbytSrcNext[1] ;
				pbytDstNext[2] = pbytSrcNext[2] ;
				pbytDstNext += pitchDstPixel ;
				pbytSrcNext += pitchSrcHorz ;
			}
		}
		else if ( depthDst == 16 )
		{
			for ( size_t x = 0; x < widthDst; x ++ )
			{
				*((uint16_t*)pbytDstNext) = *((uint16_t*)pbytSrcNext) ;
				pbytDstNext += pitchDstPixel ;
				pbytSrcNext += pitchSrcHorz ;
			}
		}
		else if ( depthDst == 8 )
		{
			for ( size_t x = 0; x < widthDst; x ++ )
			{
				*pbytDstNext = *pbytSrcNext ;
				pbytDstNext += pitchDstPixel ;
				pbytSrcNext += pitchSrcHorz ;
			}
		}
		else
		{
			return	sglErrFailed ;
		}
		pbytDstPixel += pitchDstLine ;
		pbytSrcPixel += pitchSrcVert ;
	}
	return	sglErrSuccess ;
}
#endif

// ビットマスク画像を 1/8 縮小して Grayscale 画像に変換する
//////////////////////////////////////////////////////////////////////////////
#if	!defined(__COTOPHA__)
SGLError SakuraGL::sglMultisampleBitmaskToGrayscale
	( const SGLImageBuffer& imgDst, const SGLImageBuffer& imgSrc )
{
	static const BYTE	s_bytTone[0x100] =
	{
		0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C,
		0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C,
		0x40, 0x44, 0x48, 0x4C, 0x50, 0x55, 0x59, 0x5D,
		0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D,
		0x81, 0x85, 0x89, 0x8D, 0x91, 0x95, 0x99, 0x9D,
		0xA1, 0xA5, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA, 0xBE,
		0xC2, 0xC6, 0xCA, 0xCE, 0xD2, 0xD6, 0xDA, 0xDE,
		0xE2, 0xE6, 0xEA, 0xEE, 0xF2, 0xF6, 0xFA, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	} ;
	ESLAssert( imgDst.width == ((imgSrc.width + 7) >> 3) ) ;
	ESLAssert( imgDst.height == ((imgSrc.height + 7) >> 3) ) ;
	ESLAssert( imgSrc.depth == 1 ) ;
	ESLAssert( imgDst.depth == 8 ) ;
	ESLAssert( imgDst.pitchPixel == 1 ) ;
	if ( (imgDst.width != ((imgSrc.width + 7) >> 3))
		|| (imgDst.height != ((imgSrc.height + 7) >> 3))
		|| (imgSrc.depth != 1)
		|| (imgDst.depth != 8) || (imgDst.pitchPixel != 1) )
	{
		return	sglErrInvalidParam ;
	}
	uint8_t *		pbytDst = imgDst.ptrBuffer ;
	const uint8_t *	pbytSrc = imgSrc.ptrBuffer ;
	for ( size_t y = 0; y < imgDst.height; y ++ )
	{
		size_t	hSrcLines = 8 ;
		if ( y * 8 + hSrcLines > imgSrc.height )
		{
			hSrcLines = imgSrc.height - y * 8 ;
		}
		for ( size_t x = 0; x < imgDst.width; x ++ )
		{
			uint8_t	g = 0 ;
			ssize_t	iOffsetLine = 0 ;
			for ( size_t i = 0; i < hSrcLines; i ++ )
			{
				uint8_t	b = pbytSrc[x + iOffsetLine] ;
				b = (b & 0x55) + ((b & 0xAA) >> 1) ;
				b = (b & 0x33) + ((b & 0xCC) >> 2) ;
				g += (b & 0x0F) + ((b & 0xF0) >> 4) ;
				iOffsetLine += imgSrc.pitchLine ;
			}
			pbytDst[x] += s_bytTone[g] ;
		}
		pbytDst += imgDst.pitchLine ;
		pbytSrc += imgSrc.pitchLine * 8 ;
	}
	return	sglErrSuccess ;
}
#endif


