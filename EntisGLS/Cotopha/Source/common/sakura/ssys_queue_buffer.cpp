
#include <sakura/sakura.h>
#include <sakura/ssys_queue_buffer.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// キューバッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SQueueBuffer, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SQueueBuffer::SQueueBuffer( void )
{
	m_nLength = 0 ;
	m_nGetting = 0 ;
	m_nPutting = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SQueueBuffer::~SQueueBuffer( void )
{
}

// バッファの全削除
//////////////////////////////////////////////////////////////////////////////
void SQueueBuffer::ClearAll( void )
{
	m_queBuffer.RemoveAll() ;
	m_nLength = 0 ;
	m_nGetting = 0 ;
	m_nPutting = 0 ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SQueueBuffer::ReadFromStream
	( SInputStream& file, ssize_t nBytes )
{
	size_t	nFileBytes = 0 ;
	for ( ; ; )
	{
		size_t	nBlockBytes = 0x400 ;
		if ( nBytes >= 0 )
		{
			if ( nFileBytes >= (size_t) nBytes )
			{
				break ;
			}
			nBlockBytes = nBytes - nFileBytes ;
			if ( nBlockBytes >= 0x400 )
			{
				nBlockBytes = 0x400 ;
			}
		}
		uint8_t *	pbytBuf = PutBuffer( nBlockBytes ) ;
		size_t		nReadBytes = file.Read( pbytBuf, nBlockBytes ) ;
		FlushBuffer( nReadBytes ) ;
		nFileBytes += nReadBytes ;
		if ( nReadBytes == 0 )
		{
			break ;
		}
	}
	return	nFileBytes ;
}

size_t SQueueBuffer::ReadFromTextStream
	( SInputStream& file, ssize_t nBytes )
{
	size_t	nFileBytes = 0 ;
	for ( ; ; )
	{
		size_t	nBlockBytes = 0x400 ;
		if ( nBytes >= 0 )
		{
			if ( nFileBytes >= (size_t) nBytes )
			{
				break ;
			}
			nBlockBytes = nBytes - nFileBytes ;
			if ( nBlockBytes >= 0x400 )
			{
				nBlockBytes = 0x400 ;
			}
		}
		bool		fEndOfFile = false ;
		uint8_t *	pbytBuf = PutBuffer( nBlockBytes ) ;
		size_t		nReadBytes = file.Read( pbytBuf, nBlockBytes ) ;
		for ( size_t i = 0; i < nReadBytes; i ++ )
		{
			if ( pbytBuf[i] == 0x1A )
			{
				fEndOfFile = true ;
				nReadBytes = i ;
				break ;
			}
		}
		FlushBuffer( nReadBytes ) ;
		nFileBytes += nReadBytes ;
		if ( fEndOfFile || (nReadBytes == 0) )
		{
			break ;
		}
	}
	return	nFileBytes ;
}

// 読み取りバッファ確保
//////////////////////////////////////////////////////////////////////////////
const uint8_t * SQueueBuffer::GetBuffer( size_t& nBytes )
{
	Fragment *	pBuf = m_queBuffer.GetAt( 0 ) ;
	if ( pBuf == NULL )
	{
		nBytes = 0 ;
		return	NULL ;
	}
	if ( nBytes <= (size_t) pBuf->m_nStuffed )
	{
		m_nGetting = nBytes ;
		return	pBuf->GetConstArray() + pBuf->m_nUsed ;
	}
	if ( pBuf->m_nUsed > 0 )
	{
		pBuf->Remove( 0, pBuf->m_nUsed ) ;
	}
	pBuf->m_nUsed = 0 ;
	if ( nBytes > m_nLength )
	{
		nBytes = m_nLength ;
	}
	pBuf->SetLimit( nBytes ) ;
	while ( (size_t) pBuf->m_nStuffed < nBytes )
	{
		Fragment *	pNext = m_queBuffer.GetAt( 1 ) ;
		if ( pNext == NULL )
		{
			break ;
		}
		pBuf->AddArray
			( pNext->GetConstArray() + pNext->m_nUsed, pNext->m_nStuffed ) ;
		pBuf->m_nStuffed += pNext->m_nStuffed ;
		//
		m_queBuffer.RemoveAt( 1 ) ;
	}
	m_nGetting = nBytes ;
	return	pBuf->GetConstArray() + pBuf->m_nUsed ;
}

// 読み取りバッファ解放
//////////////////////////////////////////////////////////////////////////////
void SQueueBuffer::ReleaseBuffer( ssize_t nBytes )
{
	if ( nBytes < 0 )
	{
		nBytes = (ssize_t) m_nGetting ;
	}
	else if ( (size_t) nBytes > m_nGetting )
	{
		nBytes = (ssize_t) m_nGetting ;
	}
	m_nGetting = 0 ;
	//
	Fragment *	pBuf = m_queBuffer.GetAt( 0 ) ;
	if ( pBuf == NULL )
	{
		return ;
	}
	pBuf->m_nUsed += nBytes ;
	pBuf->m_nStuffed -= nBytes ;
	m_nLength -= nBytes ;
	if ( pBuf->m_nStuffed <= 0 )
	{
		m_queBuffer.RemoveAt( 0 ) ;
	}
}

// 書き込みバッファ確保
//////////////////////////////////////////////////////////////////////////////
uint8_t * SQueueBuffer::PutBuffer( size_t nBytes )
{
	if ( nBytes == 0 )
	{
		return	NULL ;
	}
	Fragment *	pBuf = m_queBuffer.GetLastAt( 0 ) ;
	if ( (pBuf == NULL) || (m_nGetting != 0)
		|| (pBuf->GetLength() + nBytes > pBuf->GetLimit()) )
	{
		pBuf = new Fragment ;
		if ( nBytes < 0x400 )
		{
			pBuf->SetLimit( 0x400 ) ;
		}
		else
		{
			pBuf->SetLimit( (nBytes + 0x3F) & ~0x3F ) ;
		}
		m_queBuffer.Add( pBuf ) ;
	}
	m_nPutting = nBytes ;
	ESLAssert( pBuf->m_nUsed + pBuf->m_nStuffed == pBuf->GetLength() ) ;
	ESLAssert( pBuf->GetLength() + nBytes <= pBuf->GetLimit() ) ;
	pBuf->SetLength( pBuf->GetLength() + nBytes ) ;
	return	pBuf->GetArray() + (pBuf->GetLength() - nBytes) ;
}

// 書き込みバッファ解放
//////////////////////////////////////////////////////////////////////////////
void SQueueBuffer::FlushBuffer( size_t nBytes )
{
	if ( nBytes > m_nPutting )
	{
		nBytes = m_nPutting ;
	}
	Fragment *	pBuf = m_queBuffer.GetLastAt( 0 ) ;
	if ( pBuf != NULL )
	{
		ESLAssert( pBuf->m_nStuffed + nBytes <= pBuf->GetLimit() ) ;
		pBuf->m_nStuffed += (ssize_t) nBytes ;
		m_nLength += nBytes ;
		pBuf->FinishArray() ;
		pBuf->SetLength( pBuf->m_nUsed + pBuf->m_nStuffed ) ;
	}
	m_nPutting = 0 ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SQueueBuffer::Duplicate( void ) const
{
	return	new SQueueBuffer ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SQueueBuffer::Read( void * ptrBuf, size_t nBytes )
{
	size_t		nReadBytes = 0 ;
	uint8_t *	ptrNext = (uint8_t*) ptrBuf ;
	while ( (nBytes != 0) && (m_queBuffer.GetLength() > 0) )
	{
		Fragment *	pBuf = m_queBuffer.GetAt( 0 ) ;
		if ( pBuf != NULL )
		{
			size_t	nNext = pBuf->m_nStuffed ;
			if ( nNext > nBytes )
			{
				nNext = nBytes ;
			}
			eslMoveMemory
				( ptrNext, pBuf->GetConstArray() + pBuf->m_nUsed, nNext ) ;
			ptrNext += nNext ;
			pBuf->m_nUsed += nNext ;
			pBuf->m_nStuffed -= (ssize_t) nNext ;
			nReadBytes += nNext ;
			nBytes -= nNext ;
			m_nLength -= nNext ;
			//
			if ( pBuf->m_nStuffed <= 0 )
			{
				m_queBuffer.RemoveAt( 0 ) ;
			}
		}
		else
		{
			m_queBuffer.RemoveAt( 0 ) ;
		}
	}
	m_nGetting = 0 ;
	m_nPutting = 0 ;
	return	nReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SQueueBuffer::Write( const void * ptrBuf, size_t nBytes )
{
	if ( m_nPutting != 0 )
	{
		FlushBuffer( m_nPutting ) ;
	}
	size_t			nWrittenBytes = 0 ;
	const uint8_t *	ptrNext = (const uint8_t*) ptrBuf ;
	Fragment *		pBuf = m_queBuffer.GetLastAt( 0 ) ;
	if ( pBuf != NULL )
	{
		size_t	nNext = pBuf->GetLimit() - pBuf->GetLength() ;
		if ( nNext > nBytes )
		{
			nNext = nBytes ;
		}
		pBuf->SetLength( pBuf->GetLength() + nNext ) ;
		//
		eslMoveMemory
			( pBuf->GetArray()
				+ (pBuf->GetLength() - nNext), ptrNext, nNext ) ;
		pBuf->FinishArray() ;
		//
		ptrNext += nNext ;
		pBuf->m_nStuffed += (ssize_t) nNext ;
		m_nLength += nNext ;
		nWrittenBytes += nNext ;
		nBytes -= nNext ;
	}
	if ( nBytes != 0 )
	{
		eslMoveMemory( PutBuffer(nBytes), ptrNext, nBytes ) ;
		FlushBuffer( nBytes ) ;
		nWrittenBytes += nBytes ;
	}
	return	nWrittenBytes ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SQueueBuffer::IsSeekable( void ) const
{
	return	false ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SQueueBuffer::GetLength( void ) const
{
#if	defined(__DEBUG__)
	size_t	nLength = 0 ;
	for ( size_t i = 0; i < m_queBuffer.GetLength(); i ++ )
	{
		Fragment *	pBuf = m_queBuffer.GetAt( i ) ;
		if ( pBuf != NULL )
		{
			nLength += pBuf->m_nStuffed ;
		}
	}
	ESLAssert( m_nLength == nLength ) ;
#endif
	return	m_nLength ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SQueueBuffer::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	0 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SQueueBuffer::GetPosition( void ) const
{
	return	0 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SQueueBuffer::SetEndOfFile( void )
{
	return	errFailed ;
}


