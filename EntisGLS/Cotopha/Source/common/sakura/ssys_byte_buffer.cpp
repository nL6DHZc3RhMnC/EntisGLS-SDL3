
#include <sakura/sakura.h>
#include <sakura/ssys_queue_buffer.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// ただの BYTE バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SByteBuffer, SFileInterface )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SByteBuffer::~SByteBuffer( void )
{
#if	defined(__COTOPHA__)
	delete	m_pMemFile ;
	m_pMemFile = NULL ;
#endif
}

// File 変換
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
File* SByteBuffer::GetFileObject( void )
{
	if ( m_pMemFile == NULL )
	{
		m_pMemFile = new MemoryReferenceFile ;
	}
	m_pMemFile->AttachMemory
		( SArray<uint8_t>::m_ptrArray, SArray<uint8_t>::m_nLength ) ;
	m_pMemFile->Seek( m_posBuf ) ;
	return	m_pMemFile ;
}
#endif

// バッファ読み取り
//////////////////////////////////////////////////////////////////////////////
size_t SByteBuffer::ReadBuffer
	( size_t nPos, void * ptrBuf, size_t nBytes ) const
{
	if ( nPos >= SArray<uint8_t>::m_nLength )
	{
		return	0 ;
	}
	if ( nPos + nBytes > SArray<uint8_t>::m_nLength )
	{
		nBytes = SArray<uint8_t>::m_nLength - nPos ;
	}
	eslMoveMemory( ptrBuf, SArray<uint8_t>::m_ptrArray + nPos, nBytes ) ;
	return	nBytes ;
}

// バッファ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SByteBuffer::WriteBuffer
	( size_t nPos, const void * ptrBuf, size_t nBytes )
{
	if ( nPos >= SArray<uint8_t>::m_nLength )
	{
		return	0 ;
	}
	if ( nPos + nBytes > SArray<uint8_t>::m_nLength )
	{
		nBytes = SArray<uint8_t>::m_nLength - nPos ;
	}
	eslMoveMemory( SArray<uint8_t>::m_ptrArray + nPos, ptrBuf, nBytes ) ;
	return	nBytes ;
}

// 入力ストリームから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SByteBuffer::ReadFromStream( SInputStream& file, ssize_t nBytes )
{
	SQueueBuffer	qbuf ;
	qbuf.ReadFromStream( file, nBytes ) ;
	//
	nBytes = (ssize_t) qbuf.GetLength() ;
	SetLength( (size_t) nBytes ) ;
	size_t	nReadBytes = qbuf.Read( GetArray(), (size_t) nBytes ) ;
	FinishArray() ;
	return	nReadBytes ;
}

size_t SByteBuffer::ReadFromTextStream( SInputStream& file, ssize_t nBytes )
{
	SQueueBuffer	qbuf ;
	qbuf.ReadFromTextStream( file, nBytes ) ;
	//
	nBytes = (ssize_t) qbuf.GetLength() ;
	SetLength( (size_t) nBytes ) ;
	size_t	nReadBytes = qbuf.Read( GetArray(), (size_t) nBytes ) ;
	FinishArray() ;
	return	nReadBytes ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SByteBuffer::ReadFromFile( SFileInterface& file, ssize_t nBytes )
{
	if ( nBytes < 0 )
	{
		nBytes = (ssize_t) file.GetLength() ;
		if ( nBytes <= 0 )
		{
			nBytes = -1 ;
		}
	}
	if ( nBytes < 0 )
	{
		SQueueBuffer	qbuf ;
		qbuf.ReadFromStream( file ) ;
		//
		size_t	nBytes = (size_t) qbuf.GetLength() ;
		SetLength( nBytes ) ;
		size_t	nReadBytes = qbuf.Read( GetArray(), nBytes ) ;
		FinishArray() ;
		return	nReadBytes ;
	}
	else
	{
		SetLength( (size_t) nBytes ) ;
		size_t	nReadBytes = file.Read( GetArray(), (size_t) nBytes ) ;
		FinishArray() ;
		return	nReadBytes ;
	}
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SByteBuffer::Duplicate( void ) const
{
	return	new SByteBuffer( *this ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SByteBuffer::Read( void * ptrBuf, size_t nBytes )
{
	if ( m_nLength <= m_posBuf )
	{
		return	0 ;
	}
	if ( m_nLength < m_posBuf + nBytes )
	{
		nBytes = m_nLength - m_posBuf ;
	}
	eslMoveMemory( ptrBuf, m_ptrArray + m_posBuf, nBytes ) ;
	m_posBuf += (uint32_t) nBytes ;
	return	nBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SByteBuffer::Write( const void * ptrBuf, size_t nBytes )
{
	if ( m_nLength <= m_posBuf )
	{
		return	0 ;
	}
	if ( m_nLength < m_posBuf + nBytes )
	{
		nBytes = m_nLength - m_posBuf ;
	}
	eslMoveMemory( m_ptrArray + m_posBuf, ptrBuf, nBytes ) ;
	m_posBuf += (uint32_t) nBytes ;
	return	nBytes ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SByteBuffer::IsSeekable( void ) const
{
	return	true ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SByteBuffer::GetLength( void ) const
{
	return	m_nLength ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SByteBuffer::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	switch ( seekFrom )
	{
	case	FromBegin:
	default:
		break ;
	case	FromCurrent:
		posFile += m_posBuf ;
		break ;
	case	FromEnd:
		posFile += m_nLength ;
		break ;
	}
	if ( posFile < 0 )
	{
		m_posBuf = 0 ;
	}
	else if ( posFile > m_nLength )
	{
		m_posBuf = m_nLength ;
	}
	else
	{
		m_posBuf = (uint32_t) posFile ;
	}
	return	m_posBuf ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SByteBuffer::GetPosition( void ) const
{
	return	m_posBuf ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SByteBuffer::SetEndOfFile( void )
{
	SetLength( m_posBuf ) ;
	return	errSuccess ;
}

