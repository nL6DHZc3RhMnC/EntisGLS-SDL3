
#include <sakura/sakura.h>
#include <sakura/ssys_frame_thread.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// フレーム・スレッド
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SSystem::SFrameThread, SObject, SThread )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFrameThread::SFrameThread( SFrameThreadGroup * pGroup )
	: m_refGroup( pGroup )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SFrameThread::~SFrameThread( void )
{
	if ( IsRunning() )
	{
		Terminate() ;
	}
}

// フレーム駆動スレッド起動
//////////////////////////////////////////////////////////////////////////////
SError SFrameThread::Begin( SProcedure * pProc, size_t nInitStack )
{
	ESLAssert( !IsRunning() ) ;
	return	BeginFrameThread( pProc, (nInitStack != 0 ? nInitStack : 0x1000) ) ;
}

// 即時スレッド終了
//////////////////////////////////////////////////////////////////////////////
SError SFrameThread::Terminate( void )
{
	ESLAssert( IsRunning() ) ;

	SFrameThreadGroup *	pGroup = m_refGroup.GetRef<SFrameThreadGroup>() ;
	ESLAssert( pGroup != nullptr ) ;

	if ( pGroup != nullptr )
	{
		pGroup->LockFrame() ;
	}
	ThrowException( L"exit frame thread" ) ;
	Sync() ;
	if ( pGroup != nullptr )
	{
		pGroup->UnlockFrame() ;
	}

	while ( !IsRunning() )
	{
		if ( pGroup != nullptr )
		{
			pGroup->LockFrame() ;
		}
		ThrowException( L"exit frame thread" ) ;
		Sync() ;
		if ( pGroup != nullptr )
		{
			pGroup->UnlockFrame() ;
		}
	}

	return	errSuccess ;
}

// フレーム駆動
//////////////////////////////////////////////////////////////////////////////
SError SFrameThread::Continue( void )
{
	return	ContinueFrameThread() ;
}

SError SFrameThread::SyncContinue( void )
{
	SError	err = ContinueFrameThread() ;
	ESLAssert( err == errContinue ) ;
	return	Sync() ;
}

// フレーム同期
//////////////////////////////////////////////////////////////////////////////
SError SFrameThread::Sync( void )
{
	SError	err = SyncFrameThread() ;
	if ( err == errSuccess )
	{
		SFrameThreadGroup *	pGroup = m_refGroup.GetRef<SFrameThreadGroup>() ;
		ESLAssert( pGroup != nullptr ) ;
		if ( pGroup != nullptr )
		{
			pGroup->Detach( this ) ;
		}
	}
	return	err ;
}




//////////////////////////////////////////////////////////////////////////////
// フレーム・スレッド・グループ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SFrameThreadGroup, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFrameThreadGroup::SFrameThreadGroup( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SFrameThreadGroup::~SFrameThreadGroup( void )
{
}

// スレッド作成
//////////////////////////////////////////////////////////////////////////////
SSmartRef<SFrameThread>
	SFrameThreadGroup::BeginThread( SProcedure * pProc, size_t nInitStack )
{
	SFrameThread *	pThread = new SFrameThread( this ) ;
	pThread->Begin( pProc, nInitStack ) ;

	m_csSync.Lock() ;
	m_raThreads.SmartAdd( pThread ) ;
	m_csSync.Unlock() ;

	return	SSmartRef<SFrameThread>( pThread ) ;
}

// スレッドを遅延終了させる
//////////////////////////////////////////////////////////////////////////////
bool SFrameThreadGroup::DelayTerminate( SFrameThread * pThread )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	ssize_t	i = m_raThreads.FindPtr( pThread ) ;
	if ( i < 0 )
	{
		return	false ;
	}
	ESLVerify( m_raThreads.DetachAt( (size_t) i ) == pThread ) ;
	pThread->ThrowException( L"exit frame thread" ) ;
	m_aTrashThreads.Add( pThread ) ;
	return	true ;
}

// フレーム駆動
//////////////////////////////////////////////////////////////////////////////
SError SFrameThreadGroup::SyncFrame( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csOnFrame ) ;

	m_csSync.Lock() ;
	{
		SReferenceArray<SFrameThread>::Iterator	iter( &m_raThreads ) ;
		while ( iter.HasNext() )
		{
			SFrameThread *	pThread = iter.Next() ;
			if ( pThread != nullptr )
			{
				pThread->Continue() ;
			}
		}
	}
	{
		SReferenceArray<SFrameThread>::Iterator	iter( &m_raThreads ) ;
		while ( iter.HasNext() )
		{
			SFrameThread *	pThread = iter.Next() ;
			if ( pThread != nullptr )
			{
				m_csSync.Unlock() ;
				if ( pThread->Sync() == errSuccess )
				{
					m_csSync.Lock() ;
					m_raThreads.RemoveAt( iter.Index() ) ;
					m_csSync.Unlock() ;
				}
				m_csSync.Lock() ;
			}
			else
			{
				m_raThreads.RemoveAt( iter.Index() ) ;
			}
		}
	}
	for ( size_t i = 0; i < m_aTrashThreads.GetLength(); i ++ )
	{
		SFrameThread *	pThread = m_aTrashThreads.GetAt( i ) ;
		if ( pThread != nullptr )
		{
			m_csSync.Unlock() ;
			SError	err = pThread->SyncFrameThread() ;
			m_csSync.Lock() ;
			if ( err == errSuccess )
			{
				ESLAssert( m_aTrashThreads.GetAt(i) == pThread ) ;
				m_aTrashThreads.SetAt( i, nullptr ) ;
			}
		}
	}
	m_aTrashThreads.TrimEmpty() ;

	m_csSync.Unlock() ;

	return	errSuccess ;
}

// グループから離脱する
//////////////////////////////////////////////////////////////////////////////
bool SFrameThreadGroup::Detach( SFrameThread * pThread )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( pThread != nullptr )
	{
		ssize_t	i = m_raThreads.FindPtr( pThread ) ;
		if ( i >= 0 )
		{
			m_raThreads.DetachAt( (size_t) i ) ;
			return	true ;
		}
	}
	return	false ;
}

// フレーム駆動中排他処理
//////////////////////////////////////////////////////////////////////////////
void SFrameThreadGroup::LockFrame( void )
{
	m_csOnFrame.Lock() ;
}

void SFrameThreadGroup::UnlockFrame( void )
{
	m_csOnFrame.Unlock() ;
}

