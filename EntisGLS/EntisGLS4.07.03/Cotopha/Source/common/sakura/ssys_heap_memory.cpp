
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakura/sakura.h>
#include <sakura/ssys_heap_memory.h>

#if	!defined(__COTOPHA__)
#include <stdio.h>
#endif

#if	defined(__PLATFORM_UNIX_LIKE__)
#include <sys/mman.h>
#include <malloc.h>
#endif

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// ヒープメモリ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SHeapManager::SHeapManager( uint8_t * pbytBuf, size_t nBufBytes )
{
	Initialize( pbytBuf, nBufBytes ) ;
}

SHeapManager::SHeapManager( void )
{
	m_pbytBuf = NULL ;
	m_nBufBytes = 0 ;
	m_pFreeFirst = NULL ;
}

// ヒープ領域割り当て初期化
//////////////////////////////////////////////////////////////////////////////
void SHeapManager::Initialize( uint8_t * pbytBuf, size_t nBufBytes )
{
	nBufBytes = ((nBufBytes >> blockSizeScale) << blockSizeScale) ;
	m_pbytBuf = pbytBuf ;
	m_nBufBytes = nBufBytes ;
	ESLAssert( nBufBytes >= sizeMinDouble ) ;
	//
	BLOCK_HEADER *	pFirstBlock = GetFirstBlock() ;
	pFirstBlock->dwSizeFlags =
			(uint32_t) ((m_nBufBytes - sizeof(BLOCK_HEADER)) >> blockSizeScale)
				| (blockFreeFlag | blockLastFlag
						| blockFirstFlag | blockSignature) ;
	pFirstBlock->dwPrevSize = 0 ;
	//
	m_pFreeFirst = (FREE_BLOCK*) pFirstBlock->GetBlockBody() ;
	m_pFreeFirst->dwPrevOffset = 0 ;
	m_pFreeFirst->dwNextOffset = 0 ;
}

// メモリブロック確保
//////////////////////////////////////////////////////////////////////////////
int32_t SHeapManager::Allocate( uint32_t nBytes )
{
	FREE_BLOCK *	pNextFree ;
	int32_t			nAddr = -1 ;
	pNextFree = m_pFreeFirst ;
	while ( pNextFree != NULL )
	{
		BLOCK_HEADER *	pBlock =
			(BLOCK_HEADER*) (((BYTE*)pNextFree) - sizeof(BLOCK_HEADER)) ;
		ESLAssert( pBlock->IsFreeBlock() ) ;
		ESLAssert( pBlock->IsValidBlock() ) ;
		if ( pBlock->GetBlockSize() >= nBytes )
		{
			pBlock = AllocateBlock( pBlock, nBytes ) ;
			//
			#if	defined(__DEBUG__)
			eslFillMemory
				( pBlock->GetBlockBody(), 0xCC, pBlock->GetBlockSize() ) ;
			#endif
			nAddr = (int32_t) ((ulong_ptr_t) pBlock->GetBlockBody()
											- (ulong_ptr_t) m_pbytBuf) ;
			break ;
		}
		if ( pNextFree->dwNextOffset == 0 )
		{
			break ;
		}
		pNextFree = pNextFree->GetNextFreeBlock() ;
	}
	return	nAddr ;
}

// メモリブロック再確保
//////////////////////////////////////////////////////////////////////////////
int32_t SHeapManager::Reallocate( uint32_t nAddr, uint32_t nBytes )
{
	ESLAssert( m_pbytBuf != NULL ) ;
	BLOCK_HEADER *	pBlock =
		(BLOCK_HEADER*) (m_pbytBuf + (nAddr - sizeof(BLOCK_HEADER))) ;
	ESLAssert( pBlock->IsValidBlock() ) ;
	ESLAssert( !pBlock->IsFreeBlock() ) ;
	//
	if ( (pBlock->dwSizeFlags
			& (blockFreeFlag | blockSignature)) == blockSignature )
	{
		const uint32_t	dwCurBlockBytes = pBlock->GetBlockSize() ;
		if ( nBytes <= dwCurBlockBytes )
		{
			//
			// ブロックサイズの縮小は行わない
			// ※ヒープ処理で確保するメモリサイズは
			// 　元々大きくないサイズに限定されるため
			// 　ブロックサイズを縮小する事に利点は
			// 　あまりない
			//
			return	(int32_t) nAddr ;
		}
		else
		{
			//
			// ブロックサイズの拡張
			//
			if ( !pBlock->IsLastBlock() )
			{
				BLOCK_HEADER *	pNextBlock = pBlock->GetNextBlock() ;
				const uint32_t	dwNextBlockBytes = pNextBlock->GetBlockSize() ;
				const uint32_t	dwMergeBytes =
						dwCurBlockBytes + sizeof(BLOCK_HEADER)
													+ dwNextBlockBytes ;
				if ( pNextBlock->IsFreeBlock() && (dwMergeBytes >= nBytes) )
				{
					//
					// アドレスを変更しないで拡張できる
					//
					const uint32_t	dwNormalSize =
							(nBytes + blockSizeOddMask) >> blockSizeScale ;
					const uint32_t	dwNormalBytes =
							(dwNormalSize << blockSizeScale) ;
					const uint32_t	dwCurFlags =
							(pBlock->dwSizeFlags & blockFlagMask) ;
					const uint32_t	dwNextFlags =
							(pNextBlock->dwSizeFlags & blockFlagMask) ;
					if ( dwMergeBytes > dwNormalBytes + sizeMinDouble )
					{
						DetachFreeBlockChain
							( (FREE_BLOCK*) pNextBlock->GetBlockBody() ) ;
						//
						pBlock->dwSizeFlags = dwNormalSize | dwCurFlags ;
						//
						pNextBlock = pBlock->GetNextBlock() ;
						pNextBlock->dwSizeFlags =
							dwNextFlags
								| ((dwMergeBytes - dwNormalBytes
									- sizeof(BLOCK_HEADER)) >> blockSizeScale) ;
						pNextBlock->dwPrevSize = dwNormalSize ;
						//
						NormalizeNextBlock( pNextBlock ) ;
						AttachFreeBlockChain
							( (FREE_BLOCK*) pNextBlock->GetBlockBody() ) ;
					}
					else
					{
						DetachFreeBlockChain
							( (FREE_BLOCK*) pNextBlock->GetBlockBody() ) ;
						//
						pBlock->dwSizeFlags =
							(dwMergeBytes >> blockSizeScale)
								| (dwCurFlags | (dwNextFlags & blockLastFlag)) ;
						//
						NormalizeNextBlock( pBlock ) ;
					}
					return	(int32_t) nAddr ;
				}
			}
			//
			// アドレスを変更しないで拡張できないので
			// メモリを再確保しコピーする
			//
			int32_t	nNewAddr = Allocate( nBytes ) ;
			if ( nNewAddr >= 0 )
			{
				ESLAssert( nBytes >= dwCurBlockBytes ) ;
				eslMoveMemory
					( m_pbytBuf + nNewAddr,
						m_pbytBuf + nAddr, dwCurBlockBytes ) ;
				Free( nAddr ) ;
				//
				return	nNewAddr ;
			}
		}
	}
	return	-1 ;
}

// メモリブロック解放
//////////////////////////////////////////////////////////////////////////////
void SHeapManager::Free( uint32_t nAddr )
{
	BLOCK_HEADER *	pBlock ;
	ESLAssert( m_pbytBuf != NULL ) ;
	pBlock = (BLOCK_HEADER*) (m_pbytBuf + (nAddr - sizeof(BLOCK_HEADER))) ;
	ESLAssert( pBlock->IsValidBlock() ) ;
	ESLAssert( !pBlock->IsFreeBlock() ) ;
	//
	if ( (pBlock->dwSizeFlags
			& (blockSignature | blockFreeFlag)) == blockSignature )
	{
		//
		// 未使用ブロックに変更
		//
		#if	defined(__DEBUG__)
		eslFillMemory
			( pBlock->GetBlockBody(), 0xCC, pBlock->GetBlockSize() ) ;
		#endif
		pBlock->dwSizeFlags |= blockFreeFlag ;
		//
		AttachFreeBlockChain( (FREE_BLOCK*) pBlock->GetBlockBody() ) ;
		//
		// 後ろに未使用のブロックがある場合には結合
		//
		while ( MergeFreeBlock( pBlock ) )
		{
		}
		//
		// 前に未使用のブロックがある場合には結合
		//
		for ( ; ; )
		{
			if ( pBlock->IsFirstBlock() )
			{
				break ;
			}
			BLOCK_HEADER *	pPrevBlock = pBlock->GetPrevBlock() ;
			ESLAssert( pPrevBlock->IsValidBlock() ) ;
			if ( !pPrevBlock->IsFreeBlock() )
			{
				break ;
			}
			pBlock = pPrevBlock ;
			if ( !MergeFreeBlock( pBlock ) )
			{
				break ;
			}
		}
	}
}

// メモリブロックのサイズ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SHeapManager::GetBlockLength( uint32_t nAddr ) const
{
	BLOCK_HEADER *	pBlock ;
	uint32_t		dwBlockBytes = 0 ;
	//
	ESLAssert( m_pbytBuf != NULL ) ;
	pBlock = (BLOCK_HEADER*) (m_pbytBuf + (nAddr - sizeof(BLOCK_HEADER))) ;
	ESLAssert( pBlock->IsValidBlock() ) ;
	ESLAssert( !pBlock->IsFreeBlock() ) ;
	//
	if ( (pBlock->dwSizeFlags
			& (blockSignature | blockFreeFlag)) == blockSignature )
	{
		dwBlockBytes = pBlock->GetBlockSize() ;
	}
	return	dwBlockBytes ;
}

// ヒープメモリが空か判定
//////////////////////////////////////////////////////////////////////////////
bool SHeapManager::IsEmpty( void ) const
{
	BLOCK_HEADER *	pBlock = GetFirstBlock() ;
	while ( !pBlock->IsLastBlock() )
	{
		if ( !pBlock->IsFreeBlock() )
		{
			return	false ;
		}
		pBlock = pBlock->GetNextBlock() ;
	}
	return	pBlock->IsFreeBlock() ;
}

// メモリ領域アドレス取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * SHeapManager::GetMemoryAddress( uint32_t nAddr ) const
{
	if ( nAddr < m_nBufBytes )
	{
		return	m_pbytBuf + nAddr ;
	}
	return	NULL ;
}

int32_t SHeapManager::GetMemoryOffset( uint8_t * pbytBlock ) const
{
	ulong_ptr_t	ptrBlock = (ulong_ptr_t) pbytBlock ;
	ulong_ptr_t	ptrBufBase = (ulong_ptr_t) m_pbytBuf ;
	if ( (ptrBufBase <= ptrBlock)
		&& (ptrBlock < ptrBufBase + m_nBufBytes) )
	{
		return	(int32_t) (ptrBlock - ptrBufBase) ;
	}
	return	-1 ;
}

// メモリ領域全体の長さ
//////////////////////////////////////////////////////////////////////////////
size_t SHeapManager::GetHeapLength( void ) const
{
	return	m_nBufBytes ;
}

// 確保メモリブロックのダンプ
//////////////////////////////////////////////////////////////////////////////
size_t SHeapManager::DumpMemoryBlocks( size_t nDumpLimit ) const
{
	char	szDumpBuf[0x200] ;
	size_t	nMemBlocks = 0 ;
	//
	BLOCK_HEADER *	pBlock = GetFirstBlock() ;
	while ( nMemBlocks < nDumpLimit )
	{
		ESLAssert( pBlock->IsValidBlock() ) ;
		if ( !pBlock->IsFreeBlock() )
		{
			uint32_t	nBytes = pBlock->GetBlockSize() ;
			uint8_t *	pBody = pBlock->GetBlockBody() ;
			size_t		iDump = 0 ;
			if ( sizeof(pBody) > sizeof(int32_t) )
			{
				uint64_t	ptrBody = (uint64_t) pBody ;
				#if	defined(__COTOPHA__) || (_MSC_VER >= 1400)
					iDump += sprintf_s
						( &szDumpBuf[iDump],
							0xFF - iDump, "[%08X:%08X] (%08X) ",
							(uint32_t) (ptrBody >> 32), (uint32_t) ptrBody, nBytes ) ;
				#else
					iDump += sprintf
						( szDumpBuf + iDump,
							"[%08X:%08X] (%08X)",
							(uint32_t) (ptrBody >> 32), (uint32_t) ptrBody, nBytes ) ;
				#endif
			}
			else
			{
				#if	defined(__COTOPHA__) || (_MSC_VER >= 1400)
					iDump += sprintf_s
						( &szDumpBuf[iDump],
							0xFF - iDump, "[%08X] (%08X) ",
							(uint32_t) ((ulong_ptr_t) pBody), nBytes ) ;
				#else
					iDump += sprintf
						( szDumpBuf + iDump, "[%08X] (%08X) ",
							(uint32_t) ((ulong_ptr_t) pBody), nBytes ) ;
				#endif
			}
			size_t	i ;
			for ( i = 0; (i < 32) && (i < nBytes); i ++ )
			{
				#if	defined(__COTOPHA__) || (_MSC_VER >= 1400)
					iDump += sprintf_s
						( &szDumpBuf[iDump],
							0xFF - iDump, " %02X", pBody[i] ) ;
				#else
					iDump += sprintf
						( szDumpBuf + iDump, " %02X", pBody[i] ) ;
				#endif
			}
			for ( i = nBytes; i < 32; i ++ )
			{
				szDumpBuf[iDump ++] = ' ' ;
				szDumpBuf[iDump ++] = ' ' ;
				szDumpBuf[iDump ++] = ' ' ;
			}
			szDumpBuf[iDump ++] = ' ' ;
			szDumpBuf[iDump ++] = ' ' ;
			for ( i = 0; (i < 32) && (i < nBytes); i ++ )
			{
				if ( (pBody[i] >= 0x20) && (pBody[i] < 0x80) )
				{
					szDumpBuf[iDump ++] = pBody[i] ;
					if ( pBody[i] == '%' )
					{
						szDumpBuf[iDump ++] = '%' ;
					}
				}
				else
				{
					szDumpBuf[iDump ++] = '.' ;
				}
			}
			szDumpBuf[iDump ++] = '\n' ;
			szDumpBuf[iDump ++] = '\0' ;
			Trace( szDumpBuf ) ;
			nMemBlocks ++ ;
		}
		if ( pBlock->IsLastBlock() )
		{
			break ;
		}
		pBlock = pBlock->GetNextBlock() ;
		ESLAssert( (ulong_ptr_t) pBlock < (ulong_ptr_t) (m_pbytBuf + m_nBufBytes) ) ;
	}
	return	nMemBlocks ;
}

// 未使用ブロックに使用ブロックを割り当て
//////////////////////////////////////////////////////////////////////////////
SHeapManager::BLOCK_HEADER *
	SHeapManager::AllocateBlock
		( SHeapManager::BLOCK_HEADER * pBlock, uint32_t nBytes )
{
	ESLAssert( nBytes < (uint32_t) blockMaxBytes ) ;
	ESLAssert( pBlock->IsValidBlock() ) ;
	ESLAssert( pBlock->IsFreeBlock() ) ;
	FREE_BLOCK *	pFreeBlock = (FREE_BLOCK*) pBlock->GetBlockBody() ;
	//
	const uint32_t	dwBlockBytes = pBlock->GetBlockSize() ;
	ESLAssert( dwBlockBytes >= nBytes ) ;
	//
	uint32_t	dwAllocSize = (nBytes + blockSizeOddMask) >> blockSizeScale ;
	if ( dwAllocSize <= 0 )
	{
		dwAllocSize = 1 ;
	}
	const uint32_t	dwAllocBytes = dwAllocSize << blockSizeScale ;
	if ( dwBlockBytes - dwAllocBytes > sizeMinDouble )
	{
		//
		// 未使用ブロックを２つに分割し片方に使用領域を割り当てる
		//
		BLOCK_HEADER *	pNextBlock = NULL ;
		const uint32_t	dwOrgFlags = pBlock->dwSizeFlags & blockFlagMask ;
		if ( !pBlock->IsLastBlock() )
		{
			pNextBlock = pBlock->GetNextBlock() ;
		}
		const uint32_t	dwPrevBytes =
						dwBlockBytes - (dwAllocBytes + sizeof(BLOCK_HEADER)) ;
		const uint32_t	dwPrevSize = (dwPrevBytes >> blockSizeScale) ;
		pBlock->dwSizeFlags = dwPrevSize | (dwOrgFlags & ~blockLastFlag) ;
		//
		BLOCK_HEADER *	pAllocBlock =
				(BLOCK_HEADER*) (pBlock->GetBlockBody() + dwPrevBytes) ;
		pAllocBlock->dwSizeFlags =
				dwAllocSize | ((dwOrgFlags & blockLastFlag) | blockSignature) ;
		pAllocBlock->dwPrevSize = dwPrevSize ;
		//
		NormalizeNextBlock( pAllocBlock ) ;
		//
		ESLAssert( (pNextBlock == NULL)
				|| (pAllocBlock->GetNextBlock() == pNextBlock) ) ;
		return	pAllocBlock ;
	}
	else
	{
		//
		// 未使用ブロックをそのまま使用ブロックに変更する
		//
		DetachFreeBlockChain( pFreeBlock ) ;
		pBlock->dwSizeFlags &= (uint32_t) ~blockFreeFlag ;
		return	pBlock ;
	}
}

// 2つの連続する未使用ブロックを結合
//////////////////////////////////////////////////////////////////////////////
bool SHeapManager::MergeFreeBlock( SHeapManager::BLOCK_HEADER * pBlock )
{
	ESLAssert( pBlock->IsValidBlock() ) ;
	ESLAssert( pBlock->IsFreeBlock() ) ;
	if ( pBlock->IsLastBlock() )
	{
		return	false ;
	}
	BLOCK_HEADER *	pNextBlock = pBlock->GetNextBlock() ;
	ESLAssert( pNextBlock->IsValidBlock() ) ;
	if ( !pNextBlock->IsFreeBlock() )
	{
		return	false ;
	}
	DetachFreeBlockChain( (FREE_BLOCK*) pNextBlock->GetBlockBody() ) ;
	//
	const uint32_t	dwBlockFlags = pBlock->dwSizeFlags & blockFlagMask ;
	const uint32_t	dwNextBlockFlags = pNextBlock->dwSizeFlags & blockFlagMask ;
	const uint32_t	dwNewBlockBytes =
		pBlock->GetBlockSize()
			+ sizeof(BLOCK_HEADER) + pNextBlock->GetBlockSize() ;
	pBlock->dwSizeFlags =
			(dwNewBlockBytes >> blockSizeScale)
					| (dwBlockFlags | dwNextBlockFlags) ;
	//
	NormalizeNextBlock( pBlock ) ;
	return	true ;
}

// 次のブロックとの結合を正規化する
//////////////////////////////////////////////////////////////////////////////
void SHeapManager::NormalizeNextBlock( SHeapManager::BLOCK_HEADER * pBlock )
{
	if ( !pBlock->IsLastBlock() )
	{
		BLOCK_HEADER *	pNextBlock = pBlock->GetNextBlock() ;
		pNextBlock->dwPrevSize = pBlock->dwSizeFlags & blockSizeMask ;
	}
}

// 未使用ブロックをチェインの先頭に追加
//////////////////////////////////////////////////////////////////////////////
void SHeapManager::AttachFreeBlockChain( SHeapManager::FREE_BLOCK * pFreeBlock )
{
	pFreeBlock->dwPrevOffset = 0 ;
	pFreeBlock->dwNextOffset = 0 ;
	//
	if ( m_pFreeFirst != NULL )
	{
		m_pFreeFirst->dwPrevOffset =
			(int32_t) ((ulong_ptr_t) pFreeBlock - (ulong_ptr_t) m_pFreeFirst) ;
		pFreeBlock->dwNextOffset =
			(int32_t) ((ulong_ptr_t) m_pFreeFirst - (ulong_ptr_t) pFreeBlock) ;
	}
	m_pFreeFirst = pFreeBlock ;
}

// 未使用ブロックをチェインから分離
//////////////////////////////////////////////////////////////////////////////
void SHeapManager::DetachFreeBlockChain( SHeapManager::FREE_BLOCK * pFreeBlock )
{
	if ( pFreeBlock->dwPrevOffset != 0 )
	{
		ESLAssert( m_pFreeFirst != pFreeBlock ) ;
		FREE_BLOCK *	pPrevBlock = pFreeBlock->GetPrevFreeBlock() ;
		if ( pFreeBlock->dwNextOffset != 0 )
		{
			pPrevBlock->dwNextOffset += pFreeBlock->dwNextOffset ;
			//
			FREE_BLOCK *	pNextBlock = pFreeBlock->GetNextFreeBlock() ;
			pNextBlock->dwPrevOffset += pFreeBlock->dwPrevOffset ;
		}
		else
		{
			pPrevBlock->dwNextOffset = 0 ;
		}
	}
	else
	{
		ESLAssert( m_pFreeFirst == pFreeBlock ) ;
		if ( pFreeBlock->dwNextOffset != 0 )
		{
			m_pFreeFirst = pFreeBlock->GetNextFreeBlock() ;
			m_pFreeFirst->dwPrevOffset = 0 ;
		}
		else
		{
			m_pFreeFirst = NULL ;
		}
	}
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError SHeapManager::SaveContext( SFileInterface& file )
{
	uint32_t	nHeapHdr[2] ;
	nHeapHdr[0] = (uint32_t) GetMemoryOffset( (uint8_t*) m_pFreeFirst ) ;
	nHeapHdr[1] = 0 ;
	if ( file.Write
		( &nHeapHdr[0], sizeof(uint32_t) * 2 ) < sizeof(uint32_t) * 2 )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError SHeapManager::LoadContext( SFileInterface& file )
{
	uint32_t	nHeapHdr[2] ;
	if ( file.Read
		( &nHeapHdr[0], sizeof(uint32_t) * 2 ) < sizeof(uint32_t) * 2 )
	{
		return	errFailed ;
	}
	m_pFreeFirst = (FREE_BLOCK*) (m_pbytBuf + nHeapHdr[0]) ;
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// ヒープメモリ統合管理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SHeapMemory, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SHeapMemory::SHeapMemory( void )
{
	m_sizePageUnit = GetMemoryPageSize() ;
	m_sizeHeapPage = (m_sizePageUnit < sizeMinPage)
						? (size_t) sizeMinPage : (size_t) m_sizePageUnit ;
	m_nCommitCharge = 0 ;
	m_maxCommitCharge = 0 ;
	m_pFirstHeap = NULL ;
	m_pLargeHeap = NULL ;
	m_pCacheHeap = NULL ;
	//
	m_pHeapEntries = NULL ;
	m_nHeapCount = 0 ;
	m_nHeapCountLimit = 0 ;
	m_nHeapBufBytes = 0 ;
	//
	m_nPageTableBytes = sizeof(PageIndexedTable) ;
	m_pIndexedPage = (PageIndexedTable*) AllocatePage( m_nPageTableBytes ) ;
	eslFillMemory( m_pIndexedPage, 0, sizeof(PageIndexedTable) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SHeapMemory::~SHeapMemory( void )
{
	SubHeapEntry *	pHeap ;
	pHeap = m_pFirstHeap ;
	while ( pHeap != NULL )
	{
		SubHeapEntry *	pNext = pHeap->m_pNext ;
		FreePage( (uint8_t*) pHeap, pHeap->m_sizeMemPage ) ;
		pHeap = pNext ;
	}
	pHeap = m_pLargeHeap ;
	while ( pHeap != NULL )
	{
		SubHeapEntry *	pNext = pHeap->m_pNext ;
		FreePage( (uint8_t*) pHeap, pHeap->m_sizeMemPage ) ;
		pHeap = pNext ;
	}
	if ( m_pCacheHeap != NULL )
	{
		FreePage( (uint8_t*) m_pCacheHeap, m_pCacheHeap->m_sizeMemPage ) ;
	}
	//
	m_pFirstHeap = NULL ;
	m_pLargeHeap = NULL ;
	m_pCacheHeap = NULL ;
	//
	FreePage( (uint8_t*) m_pIndexedPage, m_nPageTableBytes ) ;
	m_pIndexedPage = NULL ;
}

// メモリブロック確保
//////////////////////////////////////////////////////////////////////////////
uint8_t * SHeapMemory::Allocate( uint32_t nBytes, uint32_t nFlags )
{
	uint8_t *	pbytMem = NULL ;
	m_csSync.Lock() ;
	if ( nBytes <= m_sizeHeapPage
					- (sizePageHeader
							+ SHeapManager::sizeMinDouble + 0x10) )
	{
		//
		// 小さなサイズのメモリ
		//
		SubHeapEntry *	pHeap = m_pFirstHeap ;
		while ( pHeap != NULL )
		{
			int32_t	nAddr = pHeap->Allocate( nBytes ) ;
			if ( nAddr >= 0 )
			{
				pbytMem = pHeap->GetMemoryAddress( (uint32_t) nAddr ) ;
				ESLAssert( GetHeapEntryOf( pbytMem ) == pHeap ) ;
				m_csSync.Unlock() ;
				return	pbytMem ;
			}
			pHeap = pHeap->m_pNext ;
		}
		//
		// 拡張
		//
		#if	!defined(__COTOPHA__)
		SSystem::TestMemoryTightness() ;
		#endif
		//
		pHeap = m_pCacheHeap ;
		m_pCacheHeap = NULL ;
		if ( pHeap == NULL )
		{
			size_t	nSizeHeap = m_sizeHeapPage ;
			pHeap = (SubHeapEntry*) AllocatePage( nSizeHeap ) ;
			while ( pHeap == NULL )
			{
				#if	!defined(__COTOPHA__)
				NotifyMemoryTightness() ;
				#endif
				pHeap = (SubHeapEntry*) AllocatePage( nSizeHeap ) ;
			}
			m_nCommitCharge += nSizeHeap ;
			if ( m_maxCommitCharge < m_nCommitCharge )
			{
				if ( (m_maxCommitCharge >> 20) < (m_nCommitCharge >> 20) )
				{
					ESLTrace
						( "max used heap memory: %d [MB]\n",
							(unsigned int) (m_nCommitCharge >> 20) ) ;
				}
				m_maxCommitCharge = m_nCommitCharge ;
			}
			//
			pHeap->m_sizeMemPage = nSizeHeap ;
			pHeap->Initialize
				( ((uint8_t*)pHeap) + sizePageHeader,
								nSizeHeap - sizePageHeader ) ;
			//
			RegisterHeapIndex( pHeap ) ;
		}
		SubHeapEntry *	pFirstHeap = m_pFirstHeap ;
		pHeap->m_pPrev = NULL ;
		pHeap->m_pNext = pFirstHeap ;
		pHeap->m_flagLargeHeap = false ;
		if ( pFirstHeap != NULL )
		{
			pFirstHeap->m_pPrev = pHeap ;
		}
		m_pFirstHeap = pHeap ;
		//
		int32_t	nAddr = pHeap->Allocate( nBytes ) ;
		ESLAssert( nAddr >= 0 ) ;
		pbytMem = pHeap->GetMemoryAddress( (uint32_t) nAddr ) ;
		ESLAssert( GetHeapEntryOf( pbytMem ) == pHeap ) ;
	}
	else
	{
		//
		// 大きなサイズのメモリ
		//
		ESLAssert( nBytes < (uint32_t) SHeapManager::blockMaxBytes ) ;
		//
		#if	!defined(__COTOPHA__)
		SSystem::TestMemoryTightness() ;
		#endif
		//
		size_t	nSizeHeap = ((nBytes + 0x0F) & ~0x0F)
							+ sizePageHeader + SHeapManager::sizeMinDouble ;
		SubHeapEntry *	pHeap = (SubHeapEntry*) AllocatePage( nSizeHeap ) ;
		while ( pHeap == NULL )
		{
			#if	!defined(__COTOPHA__)
			NotifyMemoryTightness() ;
			#endif
			pHeap = (SubHeapEntry*) AllocatePage( nSizeHeap ) ;
			if ( pHeap == NULL )
			{
				Trace( "Failed to AllocatePage(%d) by thread #%08X\n",
									nSizeHeap, SThread::GetCurrentId() ) ;
				if ( nFlags & flagNoRetryAlloc )
				{
					m_csSync.Unlock() ;
					return	nullptr ;
				}
				int	nResult = MessageBox
					( L"メモリを確保できませんでした",
									L"エラー", msgboxStyleRetryCancel ) ;
				if ( nResult == msgboxResultCancel )
				{
					m_csSync.Unlock() ;
					return	nullptr ;
				}
			}
		}
		m_nCommitCharge += nSizeHeap ;
		if ( m_maxCommitCharge < m_nCommitCharge )
		{
			if ( (m_maxCommitCharge >> 20) < (m_nCommitCharge >> 20) )
			{
				ESLTrace
					( "max used heap memory: %d [MB]\n",
							(unsigned int) (m_nCommitCharge >> 20) ) ;
			}
			m_maxCommitCharge = m_nCommitCharge ;
		}
		//
		pHeap->m_sizeMemPage = nSizeHeap ;
		pHeap->Initialize
			( ((uint8_t*)pHeap) + sizePageHeader,
							nSizeHeap - sizePageHeader ) ;
		//
		RegisterHeapIndex( pHeap ) ;
		//
		SubHeapEntry *	pFirstHeap = m_pLargeHeap ;
		pHeap->m_pPrev = NULL ;
		pHeap->m_pNext = pFirstHeap ;
		pHeap->m_flagLargeHeap = true ;
		if ( pFirstHeap != NULL )
		{
			pFirstHeap->m_pPrev = pHeap ;
		}
		m_pLargeHeap = pHeap ;
		//
		ESLAssert( nBytes <= nSizeHeap - sizePageHeader - SHeapManager::sizeMinDouble ) ;
		int32_t	nAddr = pHeap->Allocate
			( (uint32_t) (nSizeHeap - sizePageHeader - SHeapManager::sizeMinDouble) ) ;
		ESLAssert( nAddr >= 0 ) ;
		pbytMem = pHeap->GetMemoryAddress( (uint32_t) nAddr ) ;
		ESLAssert( GetHeapEntryOf( pbytMem ) == pHeap ) ;
	}
	m_csSync.Unlock() ;
	return	pbytMem ;
}

// メモリブロック再確保
//////////////////////////////////////////////////////////////////////////////
uint8_t * SHeapMemory::Reallocate
		( uint8_t * pMemBlock, uint32_t nBytes, uint32_t nFlags )
{
	SubHeapEntry *	pHeap = GetHeapEntryOf( pMemBlock ) ;
	uint8_t *		pbytMem = NULL ;
	ESLAssert( pHeap != NULL ) ;
	m_csSync.Lock() ;
	int32_t	nMemAddr = pHeap->GetMemoryOffset( pMemBlock ) ;
	ESLAssert( nMemAddr >= 0 ) ;
	int32_t	nAddr = pHeap->Reallocate( (uint32_t) nMemAddr, nBytes ) ;
	if ( nAddr >= 0 )
	{
		pbytMem = pHeap->GetMemoryAddress( (uint32_t) nAddr ) ;
	}
	else
	{
		m_csSync.Unlock() ;
		pbytMem = Allocate( nBytes, nFlags ) ;
		m_csSync.Lock() ;
		//
		uint32_t	nOrgLen = pHeap->GetBlockLength( (uint32_t) nMemAddr ) ;
		if ( nBytes > nOrgLen )
		{
			nBytes = nOrgLen ;
		}
		eslMoveMemory( pbytMem, pMemBlock, nBytes ) ;
		Free( pMemBlock ) ;
	}
	m_csSync.Unlock() ;
	return	pbytMem ;
}

// メモリブロック解放
//////////////////////////////////////////////////////////////////////////////
void SHeapMemory::Free( uint8_t * pMemBlock )
{
	SubHeapEntry *	pHeap = GetHeapEntryOf( pMemBlock ) ;
	int32_t			nMemAddr = pHeap->GetMemoryOffset( pMemBlock ) ;
	ESLAssert( nMemAddr >= 0 ) ;
	//
	m_csSync.Lock() ;
	//
	pHeap->Free( (uint32_t) nMemAddr ) ;
	//
	if ( pHeap->IsEmpty() )
	{
		if ( pHeap->m_flagLargeHeap )
		{
			// 大きなサイズのメモリ用ヒープは即座に解放する
			if ( pHeap->m_pPrev == NULL )
			{
				ESLAssert( m_pLargeHeap == pHeap ) ;
				m_pLargeHeap = pHeap->m_pNext ;
			}
			pHeap->Detach() ;
			//
			ESLAssert( m_nCommitCharge >= pHeap->m_sizeMemPage ) ;
			m_nCommitCharge -= pHeap->m_sizeMemPage ;
			//
			UnregisterHeapIndex( pHeap ) ;
			FreePage( (uint8_t*) pHeap, pHeap->m_sizeMemPage ) ;
		}
		else
		{
			// 小さなサイズのメモリ用ヒープはひとつだけキャッシュする
			if ( pHeap->m_pPrev == NULL )
			{
				ESLAssert( m_pFirstHeap == pHeap ) ;
				m_pFirstHeap = pHeap->m_pNext ;
			}
			pHeap->Detach() ;
			//
			if ( m_pCacheHeap == NULL )
			{
				m_pCacheHeap = pHeap ;
			}
			else
			{
				ESLAssert( m_nCommitCharge >= pHeap->m_sizeMemPage ) ;
				m_nCommitCharge -= pHeap->m_sizeMemPage ;
				//
				UnregisterHeapIndex( pHeap ) ;
				FreePage( (uint8_t*) pHeap, pHeap->m_sizeMemPage ) ;
			}
		}
	}
	else
	{
		// 対象になっているヒープをリストの先頭に移動する
		if ( pHeap->m_pPrev != NULL )
		{
			pHeap->Detach() ;
			//
			if ( pHeap->m_flagLargeHeap )
			{
				SubHeapEntry *	pFirstHeap = m_pLargeHeap ;
				pHeap->m_pNext = pFirstHeap ;
				if ( pFirstHeap != NULL )
				{
					pFirstHeap->m_pPrev = pHeap ;
				}
				m_pLargeHeap = pHeap ;
			}
			else
			{
				SubHeapEntry *	pFirstHeap = m_pFirstHeap ;
				pHeap->m_pNext = pFirstHeap ;
				if ( pFirstHeap != NULL )
				{
					pFirstHeap->m_pPrev = pHeap ;
				}
				m_pFirstHeap = pHeap ;
			}
		}
	}
	m_csSync.Unlock() ;
}

// メモリブロックのサイズ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SHeapMemory::GetBlockLength( uint8_t * pMemBlock )
{
	SubHeapEntry *	pHeap = GetHeapEntryOf( pMemBlock ) ;
	int32_t			nMemAddr = pHeap->GetMemoryOffset( pMemBlock ) ;
	ESLAssert( nMemAddr >= 0 ) ;
	return	pHeap->GetBlockLength( (uint32_t) nMemAddr ) ;
}

// ページ割り当てメモリ総量取得
//////////////////////////////////////////////////////////////////////////////
size_t SHeapMemory::GetCommitCharge( void ) const
{
	return	m_nCommitCharge ;
}

// ページ割り当て履歴最大メモリ総量取得
//////////////////////////////////////////////////////////////////////////////
size_t SHeapMemory::GetMaxCommitCharge( void ) const
{
	return	m_maxCommitCharge ;
}

// 確保メモリブロックのダンプ
//////////////////////////////////////////////////////////////////////////////
size_t SHeapMemory::DumpMemoryBlocks( size_t nDumpLimit ) const
{
	size_t	nMemBlocks = 0 ;
	m_csSync.Lock() ;
	SubHeapEntry *	pHeap = m_pLargeHeap ;
	while ( (nDumpLimit > nMemBlocks) && (pHeap != NULL) )
	{
		nMemBlocks += pHeap->DumpMemoryBlocks( nDumpLimit - nMemBlocks ) ;
		pHeap = pHeap->m_pNext ;
	}
	pHeap = m_pFirstHeap ;
	while ( (nDumpLimit > nMemBlocks) && (pHeap != NULL) )
	{
		nMemBlocks += pHeap->DumpMemoryBlocks( nDumpLimit - nMemBlocks ) ;
		pHeap = pHeap->m_pNext ;
	}
	m_csSync.Unlock() ;
	return	nMemBlocks ;
}

// メモリブロックを含むヒープエントリ
//////////////////////////////////////////////////////////////////////////////
SHeapMemory::SubHeapEntry * SHeapMemory::GetHeapEntryOf( uint8_t * pMemBlock )
{
	SubHeapEntry *		pHeap ;
	const ulong_ptr_t	addrMemBlock = (ulong_ptr_t) pMemBlock ;
	const ulong_ptr_t	iPageBlock = (addrMemBlock >> scaleMinPage) & numberIndexMask ;
	PageIndexedTable *	ppit = m_pIndexedPage ;
	pHeap = ppit->pHeap[iPageBlock] ;
	if ( (pHeap != NULL) && (pHeap->GetMemoryOffset( pMemBlock ) >= 0) )
	{
		return	pHeap ;
	}

	size_t	i = OrderIndexOfHeapAddress( addrMemBlock ) ;
	if ( i > 0 )
	{
		ESLAssert( i - 1 < m_nHeapCount ) ;
		pHeap = (SubHeapEntry*) m_pHeapEntries[i - 1] ;
		if ( pHeap->GetMemoryOffset( pMemBlock ) >= 0 )
		{
			ppit->pHeap[iPageBlock] = pHeap ;
			return	pHeap ;
		}
	}
	if ( i < m_nHeapCount )
	{
		pHeap = (SubHeapEntry*) m_pHeapEntries[i] ;
		if ( pHeap->GetMemoryOffset( pMemBlock ) >= 0 )
		{
			ppit->pHeap[iPageBlock] = pHeap ;
			return	pHeap ;
		}
	}
	return	NULL ;
}

// ヒープエントリを逆引きテーブルに登録
//////////////////////////////////////////////////////////////////////////////
void SHeapMemory::RegisterHeapIndex( SHeapMemory::SubHeapEntry * pHeap )
{
	ulong_ptr_t	addrHeap = (ulong_ptr_t) pHeap ;
	ulong_ptr_t	iPageBlock = addrHeap >> scaleMinPage ;
	size_t		nLength =
					(size_t) ((addrHeap + pHeap->m_sizeMemPage - 1)
										>> scaleMinPage) - iPageBlock + 1 ;
	ESLAssert( nLength > 0 ) ;
	PageIndexedTable *	ppit = m_pIndexedPage ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		size_t	j = (iPageBlock + i) & numberIndexMask ;
		if ( ppit->pHeap[j] == NULL )
		{
			ppit->pHeap[j] = pHeap ;
		}
	}

	for ( size_t i = 1; m_nHeapCount >= m_nHeapCountLimit; i ++ )
	{
		size_t		nNewBytes =
						m_nHeapCountLimit
							* sizeof(ulong_ptr_t) + m_sizePageUnit * i ;
		uint8_t *	pNewBuf = AllocatePage( nNewBytes ) ;
		//
		size_t	nHeapCountLimit = nNewBytes / sizeof(ulong_ptr_t) ;
		if ( nHeapCountLimit > m_nHeapCountLimit )
		{
			if ( m_pHeapEntries != NULL )
			{
				eslCopyMemory
					( pNewBuf, m_pHeapEntries,
						m_nHeapCount * sizeof(ulong_ptr_t) ) ;
				FreePage( (uint8_t*) m_pHeapEntries, m_nHeapBufBytes ) ;
			}
			m_pHeapEntries = (ulong_ptr_t*) pNewBuf ;
			m_nHeapCountLimit = nHeapCountLimit ;
			m_nHeapBufBytes = nNewBytes ;
			break ;
		}
		//
		FreePage( pNewBuf, nNewBytes ) ;
	}

	size_t	i = OrderIndexOfHeapAddress( addrHeap ) ;
	eslMoveMemory
		( m_pHeapEntries + i + 1,
			m_pHeapEntries + i,
			(m_nHeapCount - i) * sizeof(ulong_ptr_t) ) ;
	m_pHeapEntries[i] = addrHeap ;
	m_nHeapCount ++ ;
}

// ヒープエントリを逆引きテーブルから削除
//////////////////////////////////////////////////////////////////////////////
void SHeapMemory::UnregisterHeapIndex( SHeapMemory::SubHeapEntry * pHeap )
{
	ulong_ptr_t	addrHeap = (ulong_ptr_t) pHeap ;
	ulong_ptr_t	iPageBlock = addrHeap >> scaleMinPage ;
	size_t		nLength =
					(size_t) ((addrHeap + pHeap->m_sizeMemPage - 1)
										>> scaleMinPage) - iPageBlock + 1 ;
	ESLAssert( nLength > 0 ) ;
	PageIndexedTable *	ppit = m_pIndexedPage ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		size_t	j = (iPageBlock + i) & numberIndexMask ;
		if ( ppit->pHeap[j] == pHeap )
		{
			ppit->pHeap[j] = NULL ;
		}
	}

	size_t	i = OrderIndexOfHeapAddress( addrHeap ) ;
	ESLAssert( i < m_nHeapCount ) ;
	if ( i < m_nHeapCount )
	{
		eslMoveMemory
			( m_pHeapEntries + i,
				m_pHeapEntries + i + 1,
				(m_nHeapCount - (i + 1)) * sizeof(ulong_ptr_t) ) ;
		m_nHeapCount -- ;
	}
}

// ヒープエントリ配列挿入指標検索
//////////////////////////////////////////////////////////////////////////////
size_t SHeapMemory::OrderIndexOfHeapAddress( ulong_ptr_t addrHeap ) const
{
	ssize_t			iFirst, iEnd, iMiddle = 0 ;
	ulong_ptr_t *	pArray = m_pHeapEntries ;
	ulong_ptr_t		nElement ;
	iFirst = 0 ;
	iEnd = (ssize_t) m_nHeapCount - 1 ;
	//
	while ( iFirst <= iEnd )
	{
		iMiddle = ((iFirst + iEnd) >> 1) ;
		nElement = pArray[iMiddle] ;
		//
		if ( nElement > addrHeap )
		{
			iEnd = iMiddle - 1 ;
		}
		else if ( nElement < addrHeap )
		{
			iFirst = iMiddle + 1 ;
		}
		else
		{
			return	(size_t) iMiddle ;
		}
	}
	return	iFirst ;
}

// ページサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t SHeapMemory::GetMemoryPageSize( void )
{
#if	defined(__COTOPHA__)
	return	sizeMinPage ;
#elif	defined(__PLATFORM_WINDOWS__)
	SYSTEM_INFO	sysinf ;
	::eslFillMemory( &sysinf, 0, sizeof(SYSTEM_INFO) ) ;
	::GetSystemInfo( &sysinf ) ;
	if ( sysinf.dwPageSize > 0 )
	{
		return	(size_t) sysinf.dwPageSize ;
	}
	return	sizeMinPage ;
#else
	size_t	sizePageUnit = (size_t) sysconf( _SC_PAGE_SIZE ) ;
	if ( (sizePageUnit == 0) || (sizePageUnit == (size_t) -1) )
	{
		sizePageUnit = PAGE_SIZE ;
	}
	return	sizePageUnit ;
#endif
}

// ページメモリ確保
//////////////////////////////////////////////////////////////////////////////
uint8_t * SHeapMemory::AllocatePage( size_t& nSize )
{
	size_t	sizeMap = nSize + m_sizePageUnit - 1 ;
	sizeMap -= sizeMap % m_sizePageUnit ;
	nSize = sizeMap ;
	//
#if	defined(__COTOPHA__)
	return	(uint8_t*) malloc( nSize ) ;
#elif	defined(__PLATFORM_WINDOWS__)
	return	(uint8_t*) VirtualAlloc
				( NULL, sizeMap, MEM_COMMIT, PAGE_READWRITE ) ;
#else
	void *	ptrMapped =
		mmap( NULL, sizeMap,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) ;
	if ( ptrMapped == MAP_FAILED )
	{
		return NULL ;
	}
	nSize = sizeMap ;
	return	(uint8_t*) ptrMapped ;
#endif
}

// ページメモリ解放
//////////////////////////////////////////////////////////////////////////////
void SHeapMemory::FreePage( uint8_t * pbytPage, size_t nSize )
{
#if	defined(__COTOPHA__)
	free( pbytPage ) ;
#elif	defined(__PLATFORM_WINDOWS__)
	VirtualFree( pbytPage, 0, MEM_RELEASE ) ;
#else
	munmap( pbytPage, nSize ) ;
#endif
}

// アロケーション
//////////////////////////////////////////////////////////////////////////////
void * SHeapMemory::operator new ( size_t nBytes )
{
	return	malloc( nBytes ) ;
}

void SHeapMemory::operator delete( void * ptrMem )
{
	free( ptrMem ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 標準ヒープ
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( SHeapMemory *	SSystem::g_pStdHeap = NULL ) ;

// ヒープの準備
//////////////////////////////////////////////////////////////////////////////
void SSystem::eslHeapInitialize( void )
{
	if ( SSystem::g_pStdHeap == NULL )
	{
		SSystem::g_pStdHeap = new SHeapMemory ;
		//
		esl_stub_malloc = &SSystem::eslHeapAllocate ;
		esl_stub_relloc = &SSystem::eslHeapReallocate ;
		esl_stub_free = &SSystem::eslHeapFree ;
		//
		#if	defined(__PLATFORM_WINDOWS__)
		HMODULE	hModule = ::GetModuleHandle( NULL ) ;
		FARPROC	procMalloc = ::GetProcAddress( hModule, "esl_stub_malloc" ) ;
		if ( procMalloc != NULL )
		{
			esl_stub_malloc = *((ESL_FUNCPTR_MALLOC*)procMalloc) ;
		}
		FARPROC	procRealloc = ::GetProcAddress( hModule, "esl_stub_relloc" ) ;
		if ( procRealloc != NULL )
		{
			esl_stub_relloc = *((ESL_FUNCPTR_REALLOC*)procRealloc) ;
		}
		FARPROC	procFree = ::GetProcAddress( hModule, "esl_stub_free" ) ;
		if ( procFree != NULL )
		{
			esl_stub_free = *((ESL_FUNCPTR_FREE*)procFree) ;
		}
		#endif
	}
}

// ヒープの終了
//////////////////////////////////////////////////////////////////////////////
void SSystem::eslHeapUninitialize( void )
{
	if ( g_pStdHeap != NULL )
	{
		#if	defined(__DEBUG__)
		if ( g_pStdHeap->DumpMemoryBlocks( 0x100 ) > 0 )
		{
			Trace( "memory leaks\n" ) ;
		}
		#endif
		//
		delete	g_pStdHeap ;
		g_pStdHeap = NULL ;
		//
		esl_stub_malloc = &::malloc ;
		esl_stub_relloc = &::realloc ;
		esl_stub_free = &::free ;
	}
}

// メモリ確保
//////////////////////////////////////////////////////////////////////////////
void * SSystem::eslHeapAllocate( size_t nBytes )
{
	ESLAssert( g_pStdHeap != NULL ) ;
	ESLAssert( nBytes < (uint32_t) SHeapManager::blockMaxBytes ) ;
	return	g_pStdHeap->Allocate( (uint32_t) nBytes ) ;
}

void * SSystem::eslHeapAllocate( size_t nBytes, uint32_t nFlags )
{
	ESLAssert( g_pStdHeap != NULL ) ;
	ESLAssert( nBytes < (uint32_t) SHeapManager::blockMaxBytes ) ;
	return	g_pStdHeap->Allocate( (uint32_t) nBytes, nFlags ) ;
}

// メモリ再確保
//////////////////////////////////////////////////////////////////////////////
void * SSystem::eslHeapReallocate( void * pMemBlock, size_t nBytes )
{
	ESLAssert( g_pStdHeap != NULL ) ;
	ESLAssert( nBytes < (uint32_t) SHeapManager::blockMaxBytes ) ;
	if ( pMemBlock == NULL )
	{
		return	g_pStdHeap->Allocate( (uint32_t) nBytes ) ;
	}
	else
	{
		return	g_pStdHeap->Reallocate
					( (uint8_t*) pMemBlock, (uint32_t) nBytes, 0 ) ;
	}
}

// メモリ解放
//////////////////////////////////////////////////////////////////////////////
void SSystem::eslHeapFree( void * pMemBlock )
{
	if ( pMemBlock != NULL )
	{
		ESLAssert( g_pStdHeap != NULL ) ;
		g_pStdHeap->Free( (uint8_t*) pMemBlock ) ;
	}
}

// メモリサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t SSystem::eslHeapSizeOf( void * pMemBlock )
{
	if ( pMemBlock != NULL )
	{
		ESLAssert( g_pStdHeap != NULL ) ;
		return	g_pStdHeap->GetBlockLength( (uint8_t*) pMemBlock ) ;
	}
	return	0 ;
}

// コミットチャージ取得
//////////////////////////////////////////////////////////////////////////////
size_t SSystem::eslHeapCommitCharge( void )
{
	ESLAssert( g_pStdHeap != NULL ) ;
	return	g_pStdHeap->GetCommitCharge() ;
}

// 最大コミットチャージ取得
//////////////////////////////////////////////////////////////////////////////
size_t SSystem::eslHeapMaxCommitCharge( void )
{
	ESLAssert( g_pStdHeap != NULL ) ;
	return	g_pStdHeap->GetMaxCommitCharge() ;
}


//////////////////////////////////////////////////////////////////////////////
// 標準的なメモリアロケーター
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

bool			SSystem::g_eslMemoryTightness = false ;
SSystem::NOTIFY_MEMORY_TIGHTNESS_CHAIN *
				SSystem::m_pnmtcMemTightnessChain = NULL ;


// メモリ残量が逼迫しているか？
//////////////////////////////////////////////////////////////////////////////
bool SSystem::IsMemoryTightness( void )
{
	return	g_eslMemoryTightness ;
}

// メモリ残量が逼迫しているかテスト
//////////////////////////////////////////////////////////////////////////////
bool SSystem::TestMemoryTightness( void )
{
	if ( !g_eslMemoryTightness )
	{
	#if	defined(__PLATFORM_ANDROID__)
		if ( JNI::g_JavaVM != NULL )
		{
			JNI::JSmartClass	jsclsEntisGLS
				( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
			jmethodID	jmidIsLowMemory =
				jsclsEntisGLS.GetStaticMethodID( "isLowMemory", "()Z" ) ;
			g_eslMemoryTightness =
				jsclsEntisGLS.CallStaticBooleanMethod( jmidIsLowMemory ) ;
			if ( g_eslMemoryTightness )
			{
				NotifyMemoryTightness() ;
				//
				g_eslMemoryTightness =
					jsclsEntisGLS.CallStaticBooleanMethod( jmidIsLowMemory ) ;
				//
				if ( g_eslMemoryTightness )
				{
					SString	strAppName ;
					Environment::GetApplicationName( strAppName ) ;
					MessageBox
						( L"メモリ残量が逼迫しています",
									strAppName, msgboxStyleOk ) ;
				}
			}
		}
	#endif
	}
	return	g_eslMemoryTightness ;
}

// メモリ逼迫通知関数の登録
//////////////////////////////////////////////////////////////////////////////
void SSystem::AddMemoryTightnessNotification
		( SSystem::PTR_FUNC_NOTIFY_TIGHTNESS pfnNotify, void * pInstance )
{
	NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtc = new NOTIFY_MEMORY_TIGHTNESS_CHAIN ;
	QuickLock() ;
	pnmtc->pNextChain = m_pnmtcMemTightnessChain ;
	pnmtc->pfnNotify = pfnNotify ;
	pnmtc->pInstance = pInstance ;
	m_pnmtcMemTightnessChain = pnmtc ;
	QuickUnlock() ;
}

void SSystem::RemoveMemoryTightnessNotification
		( SSystem::PTR_FUNC_NOTIFY_TIGHTNESS pfnNotify, void * pInstance )
{
	QuickLock() ;
	NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtcLast = NULL ;
	NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtc = m_pnmtcMemTightnessChain ;
	while ( pnmtc != NULL )
	{
		NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtcNext = pnmtc->pNextChain ;
		if ( (pnmtc->pfnNotify == pfnNotify)
			|| (pnmtc->pInstance == pInstance) )
		{
			if ( pnmtcLast != NULL )
			{
				pnmtcLast->pNextChain = pnmtcNext ;
			}
			else
			{
				m_pnmtcMemTightnessChain = pnmtcNext ;
			}
			delete	pnmtc ;
		}
		else
		{
			pnmtcLast = pnmtc ;
		}
		pnmtc = pnmtcNext ;
	}
	QuickUnlock() ;
}

void SSystem::RemoveAllMemoryTightnessNotification( void )
{
	QuickLock() ;
	NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtc = m_pnmtcMemTightnessChain ;
	while ( pnmtc != NULL )
	{
		NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtcNext = pnmtc->pNextChain ;
		delete	pnmtc ;
		pnmtc = pnmtcNext ;
	}
	m_pnmtcMemTightnessChain = NULL ;
	QuickUnlock() ;
}

void SSystem::NotifyMemoryTightness( int nReserved )
{
	QuickLock() ;
	NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtc = m_pnmtcMemTightnessChain ;
	while ( pnmtc != NULL )
	{
		NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pnmtcNext = pnmtc->pNextChain ;
		pnmtc->pfnNotify( pnmtc->pInstance, nReserved ) ;
		pnmtc = pnmtcNext ;
	}
	QuickUnlock() ;
}

#endif


