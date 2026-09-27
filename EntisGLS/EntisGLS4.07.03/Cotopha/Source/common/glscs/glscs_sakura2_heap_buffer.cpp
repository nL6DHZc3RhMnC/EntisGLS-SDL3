
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// ヒープ用バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( HeapBuffer, Buffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
HeapBuffer::HeapBuffer( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
HeapBuffer::~HeapBuffer( void )
{
}

// バッファ生成
//////////////////////////////////////////////////////////////////////////////
SError HeapBuffer::CreateBuffer( DWORD dwBytes, DWORD dwBase )
{
	dwBytes = ((dwBytes + SHeapManager::blockSizeOddMask)
					>> SHeapManager::blockSizeScale)
								<< SHeapManager::blockSizeScale ;
	if ( dwBytes < SHeapManager::sizeMinDouble )
	{
		dwBytes = SHeapManager::sizeMinDouble ;
	}
	SError	err = Buffer::CreateBuffer( dwBytes, dwBase ) ;
	if ( err )
	{
		return	err ;
	}
	m_heapManager.Initialize( m_pbytBuf, (size_t) dwBytes ) ;
	//
	return	errSuccess ;
}

// メモリブロック確保
//////////////////////////////////////////////////////////////////////////////
bool HeapBuffer::AllocateHeapBlock( DWORD& dwAddr, DWORD dwBytes )
{
	bool	fAllocated = false ;
	int32_t	nAddr ;
	m_csMutex.Lock() ;
	nAddr = m_heapManager.Allocate( dwBytes ) ;
	if ( nAddr >= 0 )
	{
		dwAddr = (DWORD) nAddr ;
		fAllocated = true ;
	}
	m_csMutex.Unlock() ;
	return	fAllocated ;
}

// メモリブロック再確保
//////////////////////////////////////////////////////////////////////////////
bool HeapBuffer::ReallocateHeapBlock( DWORD& dwAddr, DWORD dwBytes )
{
	bool	fAllocated = false ;
	int32_t	nAddr ;
	m_csMutex.Lock() ;
	nAddr = m_heapManager.Reallocate( dwAddr, dwBytes ) ;
	if ( nAddr >= 0 )
	{
		dwAddr = (DWORD) nAddr ;
		fAllocated = true ;
	}
	m_csMutex.Unlock() ;
	return	fAllocated ;
}

// メモリブロック解放
//////////////////////////////////////////////////////////////////////////////
void HeapBuffer::FreeHeapBlock( DWORD dwAddr )
{
	m_csMutex.Lock() ;
	m_heapManager.Free( dwAddr ) ;
	m_csMutex.Unlock() ;
}

// メモリブロックのサイズ取得
//////////////////////////////////////////////////////////////////////////////
DWORD HeapBuffer::GetHeapBlockLength( DWORD dwAddr ) const
{
	return	m_heapManager.GetBlockLength( dwAddr ) ;
}

// 空のヒープブロックか判定
//////////////////////////////////////////////////////////////////////////////
bool HeapBuffer::IsEmptyHeap( void ) const
{
	return	m_heapManager.IsEmpty() ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError HeapBuffer::SaveBuffer( SFileInterface * file )
{
	SError	err = Buffer::SaveBuffer( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	m_heapManager.SaveContext( *file ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError HeapBuffer::LoadBuffer( SFileInterface * file )
{
	SError	err = Buffer::LoadBuffer( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	m_heapManager.LoadContext( *file ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ヒープ用バッファオブジェクト
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::HeapBuffer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_HeapBuffer, context, cls_id )
{
	return	new ECSSakura2::HeapBufferObject ;
}

#endif

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( HeapBufferObject, Object, HeapBuffer )

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
	HeapBufferObject::GetSegmentBuffer( LinearAddressCache & seg )
{
	seg.baseOffset = m_nBufBase ;
	seg.limitSegment = m_nBufSize ;
	seg.pbytBuffer = m_pbytBuf ;
	return	&seg ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * HeapBufferObject::GetTypeName( void ) const
{
	return	L"SSystem::HeapBuffer" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError HeapBufferObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	HeapBuffer::SaveBuffer( file ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError HeapBufferObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	HeapBuffer::LoadBuffer( file ) ;
}


