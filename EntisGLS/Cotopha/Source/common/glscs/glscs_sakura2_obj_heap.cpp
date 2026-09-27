
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// オブジェクト・ヒープ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::ObjectHeap, Object ) ;

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ObjectHeap::GetTypeName( void ) const
{
	return	L"ObjectHeap" ;
}

// オブジェクトを割り当てる
//////////////////////////////////////////////////////////////////////////////
int ObjectHeap::AllocateObject( Object * pObj )
{
	if ( m_nHeapCount >= m_nLength * 3 / 4  )
	{
		// 空き領域が 1/4 以下になったら拡張
		if ( m_nLength == 0 )
		{
			SetLength( 0x10 ) ;
		}
		else
		{
			SetLength( m_nLength * 2 ) ;
		}
		m_iHeapNext = m_nHeapCount ;
	}
	Object**	pObjList = m_ptrArray ;
	ESLAssert( pObjList != NULL ) ;
	const size_t	nHeapLimit = m_nLength ;
	if ( m_iHeapNext >= nHeapLimit )
	{
		m_iHeapNext = 0 ;
	}
	// m_iHeapNext から順次空きを検索
	if ( pObjList[m_iHeapNext] != NULL )
	{
		unsigned int	i ;
		for ( i = m_iHeapNext + 1; i < nHeapLimit; i ++ )
		{
			if ( pObjList[i] == NULL )
			{
				m_iHeapNext = i ;
				break ;
			}
		}
		if ( pObjList[m_iHeapNext] != NULL )
		{
			// 先頭から順次空きを検索
			for ( i = 0; i < nHeapLimit; i ++ )
			{
				if ( pObjList[i] == NULL )
				{
					m_iHeapNext = i ;
					break ;
				}
			}
			ESLAssert( pObjList[m_iHeapNext] == NULL ) ;
			if ( pObjList[m_iHeapNext] != NULL )
			{
				SetLength( nHeapLimit + 1 ) ;
				m_iHeapNext = (unsigned int) nHeapLimit ;
			}
		}
	}
	int	iAllocated = m_iHeapNext ;
	SetAt( iAllocated, pObj ) ;
	pObj->m_dwHighAddr = (m_nSelector << 24) | (iAllocated & 0x00FFFFFF) ;
	//
	m_iHeapNext ++ ;
	m_nHeapCount ++ ;
	//
	return	iAllocated ;
}

// オブジェクトを割り当てる
//////////////////////////////////////////////////////////////////////////////
int ObjectHeap::AllocateObjectAt( int nIndex, Object * pObj )
{
	if ( GetAt( nIndex ) == pObj )
	{
		return	nIndex ;
	}
	ESLAssert( GetAt( nIndex ) == NULL ) ;
	if ( GetAt( nIndex ) == NULL )
	{
		SetAt( nIndex, pObj ) ;
		pObj->m_dwHighAddr = (m_nSelector << 24) | (nIndex & 0x00FFFFFF) ;
		return	nIndex ;
	}
	ESLTrace( "miss ObjectHeap::AllocateObjectAt(%d)\n", nIndex ) ;
	return	AllocateObject( pObj ) ;
}

// オブジェクトを解放する
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::FreeObjectAt
	( int nIndex, VirtualMachine * vm, Context * context )
{
	Object *	pObj = GetAt( nIndex ) ;
	if ( pObj != NULL )
	{
		pObj->OnDestruction( vm, context ) ;
		SetAt( nIndex, NULL ) ;
		//
		m_nHeapCount -- ;
		//
		if ( (unsigned int) nIndex < m_iHeapNext )
		{
			m_iHeapNext = nIndex ;
		}
		return	errSuccess ;
	}
	return	errFailed ;
}

// オブジェクトを分離する
//////////////////////////////////////////////////////////////////////////////
Object * ObjectHeap::DetachObjectAt( int nIndex )
{
	Object *	pObj = ExchangeAt( nIndex, NULL ) ;
	if ( pObj != NULL )
	{
		m_nHeapCount -- ;
		if ( (unsigned int) nIndex < m_iHeapNext )
		{
			m_iHeapNext = nIndex ;
		}
	}
	return	pObj ;
}

// 全て削除
//////////////////////////////////////////////////////////////////////////////
void ObjectHeap::RemoveAll( VirtualMachine * vm, Context * context )
{
	Object**	pObjArray = m_ptrArray ;
	const int	nLength = m_nLength ;
	for ( int i = 0; i < nLength; i ++ )
	{
		Object *	pObj = pObjArray[i] ;
		if ( pObj != NULL )
		{
			pObj->OnDestruction( vm, context ) ;
		}
	}
	SObjectArray<Object>::RemoveAll() ;
	m_nHeapCount = 0 ;
	m_iHeapNext = 0 ;
}

// ヒープの保存の準備処理
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::PrepareSave
	( VirtualMachine * vm, Context * context )
{
	Object**	pObjArray = m_ptrArray ;
	const int	nLength = m_nLength ;
	int	nErrorCount = 0 ;
	for ( int i = 0; i < nLength; i ++ )
	{
		Object *	pObj = pObjArray[i] ;
		if ( pObj != NULL )
		{
			vm->AddClassIdentity( pObj->GetTypeName() ) ;
			if ( pObj->PrepareSave( vm, context ) != errSuccess )
			{
				nErrorCount ++ ;
			}
		}
	}
	return	(nErrorCount > 0) ? errFailed : errSuccess ;
}

// オブジェクト・ヒープを保存する
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::SaveHeapStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	int	nErrorCount = 0 ;
	if ( SaveHeapHeader( file, vm, context ) != errSuccess )
	{
		nErrorCount ++ ;
	}
	Object**	pObjArray = m_ptrArray ;
	const int	nLength = m_nLength ;
	for ( int i = 0; i < nLength; i ++ )
	{
		Object *	pObj = pObjArray[i] ;
		if ( pObj != NULL )
		{
			int	clsid = vm->GetClassIdentity( pObj->GetTypeName() ) ;
			ESLAssert( clsid != VirtualMachine::clsidInvalid ) ;
			file->Write( &clsid, sizeof(int) ) ;
			//
			if ( pObj->SaveStatic( file, vm, context ) != errSuccess )
			{
				nErrorCount ++ ;
			}
		}
		else
		{
			int	clsid = VirtualMachine::clsidInvalid ;
			file->Write( &clsid, sizeof(int) ) ;
		}
	}
	return	(nErrorCount > 0) ? errFailed : errSuccess ;
}

SError ObjectHeap::SaveHeapDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	int	nErrorCount = 0 ;
	if ( SaveHeapHeader( file, vm, context ) != errSuccess )
	{
		nErrorCount ++ ;
	}
	Object**	pObjArray = m_ptrArray ;
	const int	nLength = m_nLength ;
	for ( int i = 0; i < nLength; i ++ )
	{
		Object *	pObj = pObjArray[i] ;
		if ( pObj != NULL )
		{
			int	clsid = vm->GetClassIdentity( pObj->GetTypeName() ) ;
			ESLAssert( clsid != VirtualMachine::clsidInvalid ) ;
			file->Write( &clsid, sizeof(int) ) ;
			//
			if ( pObj->SaveDynamic( file, vm, context ) != errSuccess )
			{
				nErrorCount ++ ;
			}
		}
		else
		{
			int	clsid = VirtualMachine::clsidInvalid ;
			file->Write( &clsid, sizeof(int) ) ;
		}
	}
	return	(nErrorCount > 0) ? errFailed : errSuccess ;
}

// オブジェクト・ヒープを復元する
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::LoadHeapStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	int	nErrorCount = 0 ;
	if ( LoadHeapHeader( file, vm, context ) != errSuccess )
	{
		nErrorCount ++ ;
	}
	Object**		pObjArray = m_ptrArray ;
	const int		nLength = m_nLength ;
	unsigned int	nValidObjCount = 0 ;
	for ( int i = 0; i < nLength; i ++ )
	{
		int	clsid ;
		file->Read( &clsid, sizeof(int) ) ;
		if ( clsid != VirtualMachine::clsidInvalid )
		{
			Object *	pObj = vm->NewObjectByIdentity( context, clsid ) ;
			ESLAssert( pObj != NULL ) ;
			pObjArray[i] = pObj ;
			pObj->m_dwHighAddr = (m_nSelector << 24) | (i & 0x00FFFFFF) ;
			if ( pObj != NULL )
			{
				nValidObjCount ++ ;
				if ( pObj->LoadStatic( file, vm, context ) != errSuccess )
				{
					nErrorCount ++ ;
				}
			}
		}
	}
	if ( nValidObjCount != m_nHeapCount )
	{
		m_nHeapCount = nValidObjCount ;
		ESLTrace( "missmatch m_nHeapCount at ObjectHeap::LoadHeapStatic\n" ) ;
	}
	return	(nErrorCount > 0) ? errFailed : errSuccess ;
}

SError ObjectHeap::LoadHeapDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	int	nErrorCount = 0 ;
	if ( LoadHeapHeader( file, vm, context ) != errSuccess )
	{
		nErrorCount ++ ;
	}
	Object**		pObjArray = m_ptrArray ;
	const int		nLength = m_nLength ;
	unsigned int	nValidObjCount = 0 ;
	for ( int i = 0; i < nLength; i ++ )
	{
		int	clsid ;
		file->Read( &clsid, sizeof(int) ) ;
		if ( clsid != VirtualMachine::clsidInvalid )
		{
			Object *	pObj = vm->NewObjectByIdentity( context, clsid ) ;
			ESLAssert( pObj != NULL ) ;
			pObjArray[i] = pObj ;
			pObj->m_dwHighAddr = (m_nSelector << 24) | (i & 0x00FFFFFF) ;
			if ( pObj != NULL )
			{
				nValidObjCount ++ ;
				if ( pObj->LoadDynamic( file, vm, context ) != errSuccess )
				{
					nErrorCount ++ ;
				}
			}
		}
	}
	if ( nValidObjCount != m_nHeapCount )
	{
		m_nHeapCount = nValidObjCount ;
		ESLTrace( "missmatch m_nHeapCount at ObjectHeap::LoadHeapDynamic\n" ) ;
	}
	return	(nErrorCount > 0) ? errFailed : errSuccess ;
}

// ヒープの復元後処理
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	Object**	pObjArray = m_ptrArray ;
	const int	nLength = m_nLength ;
	int	nErrorCount = 0 ;
	for ( int i = 0; i < nLength; i ++ )
	{
		Object *	pObj = pObjArray[i] ;
		if ( pObj != NULL )
		{
			if ( pObj->CommitAfterLoad( vm, context ) != errSuccess )
			{
				nErrorCount ++ ;
			}
		}
	}
	return	(nErrorCount > 0) ? errFailed : errSuccess ;
}

// ヒープの復元後の後のスクリプト処理
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	Object**	pObjArray = m_ptrArray ;
	const int	nLength = m_nLength ;
	int	nErrorCount = 0 ;
	for ( int i = 0; i < nLength; i ++ )
	{
		Object *	pObj = pObjArray[i] ;
		if ( pObj != NULL )
		{
			if ( pObj->OnLoadedDynamic( vm, context ) != errSuccess )
			{
				nErrorCount ++ ;
			}
		}
	}
	return	(nErrorCount > 0) ? errFailed : errSuccess ;
}

// ヒープヘッダの保存
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::SaveHeapHeader
	( SFileInterface * file, VirtualMachine * vm, Context * context )
{
	HEAP_HEADER	hdr ;
	hdr.nHeapCount = m_nHeapCount ;
	hdr.iHeapNext = m_iHeapNext ;
	hdr.nHeapLimit = m_nLength ;
	//
	const uint32_t	nHeaderSize = sizeof(HEAP_HEADER) ;
	file->Write( &nHeaderSize, sizeof(uint32_t) ) ;
	if ( file->Write( &hdr, nHeaderSize ) < nHeaderSize )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// ヒープヘッダの復元
//////////////////////////////////////////////////////////////////////////////
SError ObjectHeap::LoadHeapHeader
	( SFileInterface * file, VirtualMachine * vm, Context * context )
{
	RemoveAll( vm, context ) ;
	//
	uint32_t	nHeaderSize ;
	if ( file->Read( &nHeaderSize, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	errFailed ;
	}
	HEAP_HEADER	hdr ;
	if ( file->Read( &hdr, sizeof(HEAP_HEADER) ) < sizeof(HEAP_HEADER) )
	{
		return	errFailed ;
	}
	if ( nHeaderSize > sizeof(HEAP_HEADER) )
	{
		file->Seek
			( nHeaderSize - sizeof(HEAP_HEADER),
						SFileInterface::FromCurrent ) ;
	}
	//
	SetLength( hdr.nHeapLimit ) ;
	//
	m_nHeapCount = hdr.nHeapCount ;
	m_iHeapNext = hdr.iHeapNext ;
	return	errSuccess ;
}

