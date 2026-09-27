
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// 単純なバッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::Buffer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
Buffer::Buffer( const Buffer & buf )
{
	m_pbytBuf = NULL ;
	m_nBufSize = 0 ;
	m_nBufBase = 0 ;
	m_nBufLimit = 0 ;
	//
	CopyBufferFrom( buf ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
Buffer::~Buffer( void )
{
	FreeBuffer() ;
}


// バッファ生成
//////////////////////////////////////////////////////////////////////////////
SError Buffer::CreateBuffer( DWORD nBytes, DWORD nBase )
{
	FreeBuffer() ;
	//
	m_nBufSize = nBytes ;
	m_nBufBase = nBase ;
	m_nBufLimit = (nBytes + 0x0F) & ~0x0F ;
	m_pbytBuf = AllocateMemory( m_nBufLimit ) ;
	if ( m_pbytBuf == NULL )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// バッファリサイズ
//////////////////////////////////////////////////////////////////////////////
SError Buffer::ResizeBuffer( DWORD nBytes, DWORD nBase )
{
	if ( nBytes > m_nBufLimit )
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
		m_nBufLimit = (nBytes + 0x0F) & ~0x0F ;
		m_pbytBuf = ReallocateMemory( m_pbytBuf, m_nBufLimit ) ;
		if ( m_pbytBuf == NULL )
		{
			return	errFailed ;
		}
	}
	else
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
	}
	return	errSuccess ;
}

// バッファリミット設定
//////////////////////////////////////////////////////////////////////////////
SError Buffer::ResizeBufferLimit( DWORD nLimit )
{
	if ( nLimit >= m_nBufSize )
	{
		nLimit = (nLimit + 0x0F) & ~0x0F ;
		if ( nLimit != m_nBufLimit )
		{
			m_pbytBuf = ReallocateMemory( m_pbytBuf, nLimit ) ;
			m_nBufLimit = nLimit ;
			if ( m_pbytBuf == NULL )
			{
				return	errFailed ;
			}
		}
	}
	return	errSuccess ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void Buffer::FreeBuffer( void )
{
	if ( m_pbytBuf != NULL )
	{
		FreeMemory( m_pbytBuf ) ;
		m_pbytBuf = NULL ;
	}
	m_nBufSize = 0 ;
	m_nBufLimit = 0 ;
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SError Buffer::CopyBufferFrom( const Buffer & buf )
{
	if ( CreateBuffer( buf.m_nBufSize ) != errSuccess )
	{
		return	errFailed ;
	}
	if ( m_pbytBuf != NULL )
	{
		eslMoveMemory( m_pbytBuf, buf.m_pbytBuf, buf.m_nBufSize ) ;
	}
	return	errSuccess ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError Buffer::SaveBuffer( SFileInterface * file )
{
	BUFFER_HEADER	hdr ;
	hdr.nBufSize = m_nBufSize ;
	hdr.nBufBase = m_nBufBase ;
	if ( file->Write
		( &hdr, sizeof(BUFFER_HEADER) ) < sizeof(BUFFER_HEADER) )
	{
		return	errFailed ;
	}
	if ( file->Write( m_pbytBuf, m_nBufSize ) < m_nBufSize )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError Buffer::LoadBuffer( SFileInterface * file )
{
	BUFFER_HEADER	hdr ;
	if ( file->Read
		( &hdr, sizeof(BUFFER_HEADER) ) < sizeof(BUFFER_HEADER) )
	{
		return	errFailed ;
	}
	if ( hdr.nBufBase > 0 )
	{
		if ( CreateBuffer( hdr.nBufSize, hdr.nBufBase ) != errSuccess )
		{
			return	errFailed ;
		}
		if ( file->Read( m_pbytBuf, m_nBufSize ) < m_nBufSize )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// メモリアロケーション
//////////////////////////////////////////////////////////////////////////////
BYTE * Buffer::AllocateMemory( DWORD nBytes )
{
	return	(BYTE*) esl_malloc( nBytes ) ;
}

BYTE * Buffer::ReallocateMemory( BYTE * pbytBuf, DWORD nBytes )
{
	if ( pbytBuf != NULL )
	{
		return	(BYTE*) esl_realloc( pbytBuf, nBytes ) ;
	}
	else
	{
		return	(BYTE*) esl_malloc( nBytes ) ;
	}
}

void Buffer::FreeMemory( BYTE * pbytBuf )
{
	esl_free( pbytBuf ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 二重バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::DualBuffer, Buffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
DualBuffer::DualBuffer( const DualBuffer & buf )
{
	m_pbytShadow = NULL ;
	//
	CopyBufferFrom( buf ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
DualBuffer::~DualBuffer( void )
{
	FreeBuffer() ;
}

// バッファリサイズ
//////////////////////////////////////////////////////////////////////////////
SError DualBuffer::ResizeBuffer( DWORD nBytes, DWORD nBase )
{
	if ( nBytes > m_nBufLimit )
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
		m_nBufLimit = (nBytes + 0x0F) & ~0x0F ;
		m_pbytBuf = ReallocateMemory( m_pbytBuf, m_nBufLimit ) ;
		if ( m_pbytBuf == NULL )
		{
			return	errFailed ;
		}
		if ( m_pbytShadow != NULL )
		{
			m_pbytShadow = ReallocateMemory( m_pbytShadow, m_nBufLimit ) ;
			if ( m_pbytShadow == NULL )
			{
				return	errFailed ;
			}
		}
		if ( m_pbytCode != NULL )
		{
			m_pbytCode = ReallocateMemory( m_pbytCode, m_nBufLimit ) ;
			if ( m_pbytCode == NULL )
			{
				return	errFailed ;
			}
		}
	}
	else
	{
		m_nBufSize = nBytes ;
		m_nBufBase = nBase ;
	}
	return	errSuccess ;
}

// バッファリミット設定
//////////////////////////////////////////////////////////////////////////////
SError DualBuffer::ResizeBufferLimit( DWORD nLimit )
{
	if ( nLimit >= m_nBufSize )
	{
		nLimit = (nLimit + 0x0F) & ~0x0F ;
		if ( nLimit != m_nBufLimit )
		{
			m_pbytBuf = ReallocateMemory( m_pbytBuf, nLimit ) ;
			m_nBufLimit = nLimit ;
			if ( m_pbytBuf == NULL )
			{
				return	errFailed ;
			}
			if ( m_pbytShadow != NULL )
			{
				m_pbytShadow = ReallocateMemory( m_pbytShadow, m_nBufLimit ) ;
				if ( m_pbytShadow == NULL )
				{
					return	errFailed ;
				}
			}
		}
	}
	return	errSuccess ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void DualBuffer::FreeBuffer( void )
{
	if ( m_pbytShadow != NULL )
	{
		FreeMemory( m_pbytShadow ) ;
		m_pbytShadow = NULL ;
	}
	if ( m_pbytCode != NULL )
	{
		FreeMemory( m_pbytCode ) ;
		m_pbytCode = NULL ;
	}
	Buffer::FreeBuffer() ;
}

// シャドウバッファ生成
//////////////////////////////////////////////////////////////////////////////
SError DualBuffer::CreateShadowBuffer( void )
{
	if ( (m_pbytBuf != NULL) && (m_pbytShadow == NULL) )
	{
		m_pbytShadow = AllocateMemory( m_nBufLimit ) ;
		if ( m_pbytShadow == NULL )
		{
			return	errFailed ;
		}
		if ( m_pbytCode == NULL )
		{
			m_pbytCode = AllocateMemory( m_nBufLimit ) ;
			if ( m_pbytCode == NULL )
			{
				return	errFailed ;
			}
		}
	}
	return	errSuccess ;
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SError DualBuffer::CopyBufferFrom( const DualBuffer & buf )
{
	if ( Buffer::CopyBufferFrom( buf ) != errSuccess )
	{
		return	errFailed ;
	}
	if ( buf.m_pbytShadow != NULL )
	{
		SError	err = CreateShadowBuffer() ;
		if ( m_pbytShadow != NULL )
		{
			eslMoveMemory
				( m_pbytShadow, buf.m_pbytShadow, buf.m_nBufSize ) ;
		}
		if ( (m_pbytCode != NULL) && (buf.m_pbytCode != NULL) )
		{
			eslMoveMemory
				( m_pbytCode, buf.m_pbytCode, buf.m_nBufSize ) ;
		}
		return	err ;
	}
	return	errSuccess ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError DualBuffer::SaveBuffer( SFileInterface * file )
{
	SError	err = Buffer::SaveBuffer( file ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwFlags = (m_pbytShadow != NULL) ? 0x01 : 0x00 ;
	if ( m_pbytCode != NULL )
	{
		dwFlags |= 0x02 ;
	}
	if ( file->Write( &dwFlags, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	if ( m_pbytShadow != NULL )
	{
		if ( file->Write( m_pbytShadow, m_nBufSize ) < m_nBufSize )
		{
			return	errFailed ;
		}
	}
	if ( m_pbytCode != NULL )
	{
		if ( file->Write( m_pbytCode, m_nBufSize ) < m_nBufSize )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError DualBuffer::LoadBuffer( SFileInterface * file )
{
	SError	err = Buffer::LoadBuffer( file ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwFlags ;
	if ( file->Read( &dwFlags, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	if ( dwFlags & 0x01 )
	{
		if ( CreateShadowBuffer() )
		{
			return	errFailed ;
		}
		if ( (m_pbytShadow == NULL)
			|| (file->Read( m_pbytShadow, m_nBufSize ) < m_nBufSize) )
		{
			return	errFailed ;
		}
		if ( dwFlags & 0x02 )
		{
			if ( (m_pbytShadow == NULL)
				|| (file->Read( m_pbytCode, m_nBufSize ) < m_nBufSize) )
			{
				return	errFailed ;
			}
		}
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 単純なバッファ・オブジェクト
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Buffer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_Buffer, context, cls_id )
{
	return	new ECSSakura2::BufferObject ;
}

#endif

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::BufferObject, Object, Buffer )

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
	BufferObject::GetSegmentBuffer( LinearAddressCache & seg )
{
	seg.baseOffset = m_nBufBase ;
	seg.limitSegment = m_nBufSize ;
	seg.pbytBuffer = m_pbytBuf ;
	return	&seg ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * BufferObject::GetTypeName( void ) const
{
	return	L"SSystem::Buffer" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError BufferObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	Buffer::SaveBuffer( file ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError BufferObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	Buffer::LoadBuffer( file ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 二重バッファ・オブジェクト
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::DualBuffer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_DualBuffer, context, cls_id )
{
	return	new ECSSakura2::DualBufferObject ;
}

#endif

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::DualBufferObject, Object, DualBuffer )

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
	DualBufferObject::GetSegmentBuffer( LinearAddressCache & seg )
{
	seg.baseOffset = m_nBufBase ;
	seg.limitSegment = m_nBufSize ;
	seg.pbytBuffer = m_pbytBuf ;
	return	&seg ;
}

BYTE * DualBufferObject::GetSegmentShadowBuffer( int iShadow )
{
	switch ( iShadow )
	{
	case	0:
		return	m_pbytShadow ;
	case	1:
		return	m_pbytCode ;
	}
	return	NULL ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * DualBufferObject::GetTypeName( void ) const
{
	return	L"SSystem::DualBuffer" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError DualBufferObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	DualBuffer::SaveBuffer( file ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError DualBufferObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	DualBuffer::LoadBuffer( file ) ;
}

