
#include <sakura/sakura.h>
#include <sakura/ssys_smart_buffer.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 断片化バッファ・ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSmartBuffer, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSmartBuffer::SSmartBuffer( void )
{
	m_position = 0 ;
	m_length = 0 ;
}

SSmartBuffer::SSmartBuffer( const SSmartBuffer& sbufSrc )
{
	m_position = 0 ;
	m_length = 0 ;
	//
	CopyReferenceBuffer( sbufSrc ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SSmartBuffer::~SSmartBuffer( void )
{
	SSmartBuffer::ReleaseBuffer() ;
}

// バッファ長設定
//////////////////////////////////////////////////////////////////////////////
void SSmartBuffer::SetLength( size_t nLength )
{
	m_length = nLength ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void SSmartBuffer::ReleaseBuffer( void )
{
	m_buffers.RemoveAll() ;
	m_position = 0 ;
	m_length = 0 ;
}

// バッファ読み取り
//////////////////////////////////////////////////////////////////////////////
size_t SSmartBuffer::ReadBuffer
	( size_t nPos, void * ptrBuf, size_t nBytes ) const
{
	size_t	nReadBytes = 0 ;
	if ( nPos + nBytes > m_length )
	{
		if ( nPos >= m_length )
		{
			return	0 ;
		}
		nBytes = m_length - nPos ;
	}
	while ( nBytes > 0 )
	{
		size_t	offsetPos = nPos & PAGE_OFFSET_MASK ;
		size_t	iPage = (nPos >> PAGE_SCALE) ;
		size_t	nPageBytes = nBytes ;
		if ( nPageBytes + offsetPos > PAGE_BYTES )
		{
			nPageBytes = PAGE_BYTES - offsetPos ;
			if ( nPageBytes > nBytes )
			{
				nPageBytes = nBytes ;
			}
		}
		SByteBuffer *	pbufPage = GetPageAt( iPage ) ;
		if ( pbufPage != NULL )
		{
			pbufPage->ReadBuffer( offsetPos, ptrBuf, nPageBytes ) ;
		}
		else
		{
			eslFillMemory( ptrBuf, 0, nPageBytes ) ;
		}
		nPos += nPageBytes ;
		ptrBuf = ((uint8_t*)ptrBuf) + nPageBytes ;
		nBytes -= nPageBytes ;
		nReadBytes += nPageBytes ;
	}
	return	nReadBytes ;
}

// バッファ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SSmartBuffer::WriteBuffer
	( size_t nPos, const void * ptrBuf, size_t nBytes )
{
	size_t	nWrittenBytes = 0 ;
	while ( nBytes > 0 )
	{
		size_t	offsetPos = nPos & PAGE_OFFSET_MASK ;
		size_t	iPage = (nPos >> PAGE_SCALE) ;
		size_t	nPageBytes = nBytes ;
		if ( nPageBytes + offsetPos > PAGE_BYTES )
		{
			nPageBytes = PAGE_BYTES - offsetPos ;
			if ( nPageBytes > nBytes )
			{
				nPageBytes = nBytes ;
			}
		}
		SByteBuffer *	pbufPage = GetLoadedPageAt( iPage ) ;
		if ( pbufPage != NULL )
		{
			pbufPage->WriteBuffer( offsetPos, ptrBuf, nPageBytes ) ;
		}
		nPos += nPageBytes ;
		ptrBuf = ((const uint8_t*)ptrBuf) + nPageBytes ;
		nBytes -= nPageBytes ;
		nWrittenBytes += nPageBytes ;
	}
	if ( nPos > m_length )
	{
		m_length = nPos ;
	}
	return	nWrittenBytes ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SSmartBuffer::ReadFromStream( SInputStream& file, ssize_t nBytes )
{
	size_t	nPos = 0 ;
	for ( ; ; )
	{
		size_t	offsetPos = nPos & PAGE_OFFSET_MASK ;
		size_t	iPage = (nPos >> PAGE_SCALE) ;
		size_t	nPageBytes = PAGE_BYTES - offsetPos ;
		//
		SByteBuffer *	pbufPage = GetLoadedPageAt( iPage ) ;
		if ( pbufPage != NULL )
		{
			if ( (nBytes >= 0) & (nPageBytes > (size_t) nBytes) )
			{
				nPageBytes = nBytes ;
			}
			size_t	nReadBytes =
				file.Read( pbufPage->GetArray() + offsetPos, nPageBytes ) ;
			pbufPage->FinishArray() ;
			//
			if ( nReadBytes == 0 )
			{
				break ;
			}
			nPos += nReadBytes ;
			if ( nBytes >= 0 )
			{
				nBytes -= (ssize_t) nReadBytes ;
				if ( nBytes <= 0 )
				{
					break ;
				}
			}
		}
		else
		{
			break ;
		}
	}
	m_length = nPos ;
	return	nPos ;
}

// ファイルへ書き出し
//////////////////////////////////////////////////////////////////////////////
size_t SSmartBuffer::WriteToStream
	( SOutputStream& file, ssize_t nBytes ) const
{
	if ( nBytes < 0 )
	{
		nBytes = (ssize_t) m_length ;
	}
	size_t	nPos = 0 ;
	for ( ; ; )
	{
		size_t	offsetPos = nPos & PAGE_OFFSET_MASK ;
		size_t	iPage = (nPos >> PAGE_SCALE) ;
		size_t	nPageBytes = PAGE_BYTES - offsetPos ;
		//
		SByteBuffer *	pbufPage = GetPageAt( iPage ) ;
		if ( pbufPage != NULL )
		{
			ESLAssert( pbufPage->GetLength() == PAGE_BYTES ) ;
			if ( (nBytes >= 0) & (nPageBytes > (size_t) nBytes) )
			{
				nPageBytes = nBytes ;
			}
			size_t	nWrittenBytes =
				file.Write( pbufPage->GetConstArray() + offsetPos, nPageBytes ) ;
			if ( nWrittenBytes == 0 )
			{
				break ;
			}
			nPos += nWrittenBytes ;
			if ( nBytes >= 0 )
			{
				nBytes -= (ssize_t) nWrittenBytes ;
				if ( nBytes <= 0 )
				{
					break ;
				}
			}
		}
		else
		{
			break ;
		}
	}
	return	nPos ;
}

// バッファの複製参照
//////////////////////////////////////////////////////////////////////////////
void SSmartBuffer::CopyReferenceBuffer( const SSmartBuffer& sbufSrc )
{
	const size_t	countPage = sbufSrc.m_buffers.GetLength() ;
	m_buffers.RemoveAll() ;
	for ( size_t i = 0; i < countPage; i ++ )
	{
		SSyncReference *	pRef = sbufSrc.m_buffers.GetAt( i ) ;
		if ( pRef != NULL )
		{
			m_buffers.SetAt
				( i, new SSyncReference( pRef->GetReference() ) ) ;
		}
	}
	m_position = sbufSrc.m_position ;
	m_length = sbufSrc.m_length ;
}

// ページ取得
//////////////////////////////////////////////////////////////////////////////
SByteBuffer * SSmartBuffer::GetPageAt( size_t iPage ) const
{
	return	ESLTypeCast<SByteBuffer>( m_buffers.GetAt(iPage) ) ;
}

// ロード済みページ取得
//////////////////////////////////////////////////////////////////////////////
SByteBuffer * SSmartBuffer::GetLoadedPageAt( size_t iPage )
{
	SByteBuffer *	pBuf =
		ESLTypeCast<SByteBuffer>( m_buffers.GetAt( iPage ) ) ;
	if ( pBuf == NULL )
	{
		pBuf = new SByteBuffer ;
		pBuf->SetLength( PAGE_BYTES ) ;
		//
		m_buffers.SetAt
			( iPage, new SSyncReference
						( new SSmartObject( (SFileOpener*) pBuf ) ) ) ;
	}
	return	pBuf ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SSmartBuffer::Duplicate( void ) const
{
	return	new SSmartBuffer( *this ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SSmartBuffer::Read( void * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes = ReadBuffer( m_position, ptrBuf, nBytes ) ;
	m_position += nReadBytes ;
	return	nReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SSmartBuffer::Write( const void * ptrBuf, size_t nBytes )
{
	size_t	nWrittenBytes = WriteBuffer( m_position, ptrBuf, nBytes ) ;
	m_position += nWrittenBytes ;
	return	nWrittenBytes ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SSmartBuffer::IsSeekable( void ) const
{
	return	true ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SSmartBuffer::GetLength( void ) const
{
	return	m_length ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SSmartBuffer::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	switch ( seekFrom )
	{
	case	FromBegin:
	default:
		break ;
	case	FromCurrent:
		posFile += m_position ;
		break ;
	case	FromEnd:
		posFile += m_length ;
		break ;
	}
	if ( posFile < 0 )
	{
		posFile = 0 ;
	}
	m_position = (size_t) posFile ;
	return	m_position ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SSmartBuffer::GetPosition( void ) const
{
	return	m_position ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SSmartBuffer::SetEndOfFile( void )
{
	m_length = m_position ;
	return	errSuccess ;
}

