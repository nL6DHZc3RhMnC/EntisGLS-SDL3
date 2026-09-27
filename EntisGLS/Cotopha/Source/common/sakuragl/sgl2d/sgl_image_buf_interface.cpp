
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_buf_interface.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// SGLImageBuffer - Image 変換 SGLImageBufferInterface
//////////////////////////////////////////////////////////////////////////////

#if	defined(__COTOPHA__)

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::EntisGLS4ImageBufferInterface, SGLImageBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EntisGLS4ImageBufferInterface::EntisGLS4ImageBufferInterface( void )
{
	m_typeObject = imageObjectEntisGLS4Image ;
	m_pImage = new Image ;
	m_fUpdate = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EntisGLS4ImageBufferInterface::~EntisGLS4ImageBufferInterface( void )
{
	delete	m_pImage ;
	m_pImage = NULL ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError EntisGLS4ImageBufferInterface::Initialize( SGLImageBuffer * pImageBuf )
{
	SGLError	err = m_pImage->CreateBuffer( *pImageBuf ) ;
	if ( err )
	{
		return	err ;
	}
	if ( pImageBuf->ptrBuffer != NULL )
	{
		SGLImageBuffer	bufLocked ;
		uint8_t *	pbytBuf =
			m_pImage->LockBuffer
				( bufLocked, Image::bufferWrite, NULL ) ;
		if ( pbytBuf != NULL )
		{
			bufLocked.ptrBuffer = pbytBuf ;
			sglCopyImageBuffer( bufLocked, *pImageBuf ) ;
		}
		else
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

// 更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError EntisGLS4ImageBufferInterface::UpdateBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	SGLRect	rctUpdate ;
	if ( pRect != NULL )
	{
		rctUpdate = *pRect ;
	}
	else
	{
		rctUpdate = pImageBuf->GetImageRect() ;
	}
	if ( m_fUpdate )
	{
		m_rctUpdate |= rctUpdate ;
	}
	else
	{
		m_rctUpdate = rctUpdate ;
		m_fUpdate = true ;
	}
	return	sglErrSuccess ;
}

// 更新確定処理
//////////////////////////////////////////////////////////////////////////////
SGLError EntisGLS4ImageBufferInterface::CommitBuffer( SGLImageBuffer * pImageBuf )
{
	SGLError	errResult = sglErrSuccess ;
	if ( m_fUpdate )
	{
		SGLImageRect	irctUpdate( m_rctUpdate ) ;
		SGLImageBuffer	bufSrcImage ;
		if ( (pImageBuf->ptrBuffer != NULL)
			&& bufSrcImage.GetClippedBuffer( *pImageBuf, irctUpdate ) )
		{
			SGLImageBuffer	bufLocked ;
			uint8_t *	pbytBuf =
				m_pImage->LockBuffer
					( bufLocked, Image::bufferWrite, &irctUpdate ) ;
			if ( pbytBuf != NULL )
			{
				bufLocked.ptrBuffer = pbytBuf ;
				sglCopyImageBuffer( bufLocked, bufSrcImage ) ;
				m_pImage->UnlockBuffer( Image::bufferWrite ) ;
				m_fUpdate = false ;
			}
			else
			{
				errResult = sglErrFailed ;
			}
		}
		else
		{
			m_fUpdate = false ;
		}
	}
	return	errResult ;
}

// 反映処理
//////////////////////////////////////////////////////////////////////////////
SGLError EntisGLS4ImageBufferInterface::ReflectBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	SGLError		errResult = sglErrSuccess ;
	SGLImageBuffer	bufDstImage ;
	SGLImageRect	rctReflect = pImageBuf->GetImageRect() ;
	if ( pRect != NULL )
	{
		rctReflect = *pRect ;
	}
	if ( (pImageBuf->ptrBuffer != NULL)
		&& bufDstImage.GetClippedBuffer( *pImageBuf, rctReflect ) )
	{
		SGLImageBuffer	bufLocked ;
		uint8_t *	pbytBuf =
			m_pImage->LockBuffer
				( bufLocked, Image::bufferRead, &rctReflect ) ;
		if ( pbytBuf != NULL )
		{
			bufLocked.ptrBuffer = pbytBuf ;
			sglCopyImageBuffer( bufDstImage, bufLocked ) ;
			m_pImage->UnlockBuffer( Image::bufferRead ) ;
		}
		else
		{
			errResult = sglErrFailed ;
		}
	}
	return	errResult ;
}

// ミップマップ化通知
//////////////////////////////////////////////////////////////////////////////
SGLError EntisGLS4ImageBufferInterface::MakeMipmap( void )
{
	return	m_pImage->NormalizeToMipmapTexture( 0 ) ;
}

// 関連オブジェクトの削除処理
//////////////////////////////////////////////////////////////////////////////
bool EntisGLS4ImageBufferInterface::OnDestroyObject( ESLObject * pObj )
{
	return	false ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// SGLImageBuffer - SGLImageObject 変換 SGLImageBufferInterface
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImageObjectBufferInterface, SGLImageBufferInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageObjectBufferInterface::SGLImageObjectBufferInterface( void )
{
	m_typeObject = imageObjectEntisGLS4Temporary ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageObjectBufferInterface::~SGLImageObjectBufferInterface( void )
{
}

// 参照先設定
//////////////////////////////////////////////////////////////////////////////
void SGLImageObjectBufferInterface::SetImageReference( SGLImageObject * pImage )
{
	m_refImage.SetReference( pImage ) ;
}

void SGLImageObjectBufferInterface::SetSmartImageReference( SGLImageObject * pImage )
{
	m_refImage.SetSmartReference( pImage ) ;
}

// 参照先取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLImageObjectBufferInterface::GetImageReference( void ) const
{
	return	m_refImage.GetReference() ;
}

// 更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObjectBufferInterface::UpdateBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	return	sglErrSuccess ;
}

// 更新確定処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObjectBufferInterface::CommitBuffer( SGLImageBuffer * pImageBuf )
{
	return	sglErrSuccess ;
}

// 反映処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObjectBufferInterface::ReflectBuffer
	( SGLImageBuffer * pImageBuf, const SGLImageRect * pRect )
{
	return	sglErrSuccess ;
}

// ミップマップ化通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageObjectBufferInterface::MakeMipmap( void )
{
	return	sglErrSuccess ;
}

// 画像バッファの再確保通知
//////////////////////////////////////////////////////////////////////////////
bool SGLImageObjectBufferInterface::OnImageReBuffered( SGLImageBuffer * pImageBuf )
{
	return	false ;
}

// 関連オブジェクトの削除処理
//////////////////////////////////////////////////////////////////////////////
bool SGLImageObjectBufferInterface::OnDestroyObject( ESLObject * pObj )
{
	return	false ;
}
