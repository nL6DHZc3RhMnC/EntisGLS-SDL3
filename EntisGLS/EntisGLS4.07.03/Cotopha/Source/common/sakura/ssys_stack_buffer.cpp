
#include <sakura/sakura.h>
#include <sakura/ssys_stack_buffer.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 積層バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SStackBuffer, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SStackBuffer::SStackBuffer( void )
{
	m_nBlockSize = 0x1000 ;
	m_iUsingBlock = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SStackBuffer::~SStackBuffer( void )
{
}

// メモリ確保
//////////////////////////////////////////////////////////////////////////////
uint8_t * SStackBuffer::Allocate( size_t nBytes )
{
	if ( nBytes <= (m_nBlockSize >> 1) )
	{
		Buffer *	pBuf ;
		nBytes = (nBytes + 0x0F) & ~0x0F ;
		#if	defined(__DEBUG__)
		const size_t	nNeedBytes = nBytes + 0x10 ;
		#else
		const size_t	nNeedBytes = nBytes  ;
		#endif
		for ( ; ; )
		{
			pBuf = m_stackBlock.GetAt( m_iUsingBlock ) ;
			if ( pBuf == NULL )
			{
				ESLAssert( m_iUsingBlock == m_stackBlock.GetLength() ) ;
				#if	!defined(__DISABLED_EXCEPTION__)
				try
				#endif
				{
					pBuf = new Buffer ;
					if ( pBuf == NULL )
					{
						return	NULL ;
					}
					pBuf->SetLength( m_nBlockSize ) ;
				}
				#if	!defined(__DISABLED_EXCEPTION__)
				catch ( ... )
				{
					return	NULL ;
				}
				#endif
				pBuf->Initialize() ;
				m_stackBlock.Add( pBuf ) ;
			}
			else if ( pBuf->m_nUsed + nNeedBytes > pBuf->GetLength() )
			{
				m_iUsingBlock ++ ;
			}
			else
			{
				break ;
			}
		}
		uint8_t *	pMem = pBuf->GetArray() + pBuf->m_nUsed ;
		ESLAssert( (pMem[0] == 'E') && (pMem[1] == 'N') && (pMem[2] == 'T') && (pMem[3] == 'S') ) ;
		pBuf->m_nUsed += nBytes ;
		//
		#if	defined(__DEBUG__)
		uint8_t *	pNextMem = pBuf->GetArray() + pBuf->m_nUsed ;
		pNextMem[0] = (uint8_t) 'E' ;
		pNextMem[1] = (uint8_t) 'N' ;
		pNextMem[2] = (uint8_t) 'T' ;
		pNextMem[3] = (uint8_t) 'S' ;
		#endif
		//
		pBuf->FinishArray() ;
		return	pMem ;
	}
	else
	{
//		ESLTrace( "allocate large stack memory.(%08x)\n", (unsigned int) nBytes ) ;
		Buffer *	pBuf = new Buffer ;
		uint8_t *	pMem ;
		pBuf->SetLength( nBytes + 0x10 ) ;
		pBuf->Initialize() ;
		pMem = pBuf->GetArray() + pBuf->m_nUsed ;
		pBuf->FinishArray() ;
		pBuf->m_nUsed += nBytes ;
		m_stackLargeBlock.Add( pBuf ) ;
		return	pMem ;
	}
}

// メモリ解放
//////////////////////////////////////////////////////////////////////////////
SError SStackBuffer::Free( uint8_t * ptrBuf )
{
	Buffer *	pBuf = m_stackBlock.GetAt( m_iUsingBlock ) ;
	if ( pBuf != NULL )
	{
		long_ptr_t	nOffset =
			(long_ptr_t) ptrBuf - (long_ptr_t) pBuf->GetConstArray() ;
		if ( (nOffset >= 0) & (nOffset < (long_ptr_t) pBuf->m_nUsed) )
		{
			pBuf->m_nUsed = (size_t) nOffset ;
			//
			#if	defined(__DEBUG__)
			uint8_t *	pMem = pBuf->GetArray() + pBuf->m_nUsed ;
			pMem[0] = (uint8_t) 'E' ;
			pMem[1] = (uint8_t) 'N' ;
			pMem[2] = (uint8_t) 'T' ;
			pMem[3] = (uint8_t) 'S' ;
			pBuf->FinishArray() ;
			#endif
			return	errSuccess ;
		}
	}
	return	errFailed ;
}

// 全メモリ解放
//////////////////////////////////////////////////////////////////////////////
SError SStackBuffer::FreeAll( void )
{
	size_t	nCount = m_stackBlock.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Buffer *	pBuf = m_stackBlock.GetAt( i ) ;
		if ( pBuf != NULL )
		{
			pBuf->FinishArray() ;
			pBuf->Initialize() ;
		}
	}
	m_iUsingBlock = 0 ;
	m_stackLargeBlock.RemoveAll() ;
	return	errSuccess ;
}

// 使用中のメモリサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t SStackBuffer::GetUsedSize( void ) const
{
	size_t	nBytes = 0 ;
	size_t	i ;
	size_t	nCount = m_stackBlock.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		Buffer *	pBuf = m_stackBlock.GetAt( i ) ;
		if ( pBuf != NULL )
		{
			nBytes += pBuf->m_nUsed ;
		}
	}
	nCount = m_stackLargeBlock.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		Buffer *	pBuf = m_stackLargeBlock.GetAt( i ) ;
		if ( pBuf != NULL )
		{
			nBytes += pBuf->m_nUsed ;
		}
	}
	return	nBytes ;
}

// メモリブロックサイズ設定
//////////////////////////////////////////////////////////////////////////////
void SStackBuffer::SetBlockSize( size_t nBytes )
{
	if ( nBytes < 0x100 )
	{
		nBytes = 0x100 ;
	}
	m_nBlockSize = (nBytes + 0x0F) & ~0x0F ;
}



