
#include <sakura/sakura.h>
#include <sakuragl/sgl_window.h>

#if	defined(__PLATFORM_UNIX_LIKE__)
#include <linux/futex.h>
#include <sys/syscall.h>

#if	defined(__PLATFORM_ANDROID__)
static int futex
( int *uaddr, int op, int val,
	const struct timespec *timeout, int *uaddr2, int val3 )
{
	return	syscall( __NR_futex, uaddr, op, val, timeout, uaddr2, val3 ) ;
}
#endif

#if	defined(__POINTER64__) || (ANDROID_NDK_VER >= 15)
atomic_int_t __atomic_swap( atomic_int_t n, volatile atomic_int_t * p )
{
	atomic_int_t	temp ;
	__atomic_exchange( p, &n, &temp, memory_order_seq_cst ) ;
	return	temp ;
}
#endif
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 同期基底クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSynchronismInterface, ESLObject )
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSynchronism, SSynchronismInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSynchronism::SSynchronism( void )
{
	#if	defined(__COTOPHA__)
		m_pSync = NULL ;

	#elif	defined(__PLATFORM_WINDOWS__)
		m_hSync = NULL ;
		m_value = 0 ;

	#else
		m_mutex = futexWaked ;
		m_value = 0 ;
		m_wait = 0 ;
	#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SSynchronism::~SSynchronism( void )
{
	Delete() ;
}

// 同期オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void SSynchronism::Delete( void )
{
	#if	defined(__COTOPHA__)
		delete	m_pSync ;
		m_pSync = NULL ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( m_hSync != NULL )
		{
			::CloseHandle( m_hSync ) ;
			m_hSync = NULL ;
		}
	#endif
}

// シグナル値取得
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SSynchronism::Value( void ) const
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pSync != NULL ) ;
		return	m_pSync->Value() ;
	#else
		return	m_value ;
	#endif
}

// 待機
//////////////////////////////////////////////////////////////////////////////
SError SSynchronism::Wait( int64_t msecTimeout )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pSync != NULL ) ;
		if ( m_pSync != NULL )
		{
			if ( msecTimeout == Infinite )
			{
				while ( m_pSync->Wait( Infinite ) != errSuccess )
				{
				}
				return	errSuccess ;
			}
			else
			{
				return	m_pSync->Wait( msecTimeout ) ;
			}
		}
		return	errFailed ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( m_hSync == NULL )
		{
			return	errFailed ;
		}
		DWORD	dwTimeout = (DWORD) msecTimeout ;
		if ( (msecTimeout == Infinite) || (msecTimeout >= INT_MAX) )
		{
			dwTimeout = INFINITE ;
			//
			DWORD	dwWaitResult =
						::WaitForSingleObject( m_hSync, 10 ) ;
			if ( dwWaitResult == WAIT_OBJECT_0 )
			{
				OnSignal() ;
				return	errSuccess ;
			}
			/*
			QuickLock() ;
			SGLAbstractWindow *
				pWindow = SGLAbstractWindow::GetDefaultWindow() ;
			while ( pWindow != NULL )
			{
				// 画面更新待ちでデッドロックを避けるため WM_PAINT だけ処理
				DWORD	dwProcessID ;
				DWORD	dwThreadId =
					GetWindowThreadProcessId
						( pWindow->GetWindowHandle(), &dwProcessID ) ;
				if ( dwThreadId == ::GetCurrentThreadId() )
				{
					QuickUnlock() ;
					HWND	hwnd = pWindow->GetWindowHandle() ;
					for ( ; ; )
					{
						dwWaitResult =
									::WaitForSingleObject( m_hSync, 10 ) ;
						if ( dwWaitResult == WAIT_OBJECT_0 )
						{
							OnSignal() ;
							return	errSuccess ;
						}
						if ( ::GetUpdateRect( hwnd, NULL, FALSE ) )
						{
							Trace( "Peek WM_PAINT message in Wait(%d).\n", (int) msecTimeout ) ;
							MSG	msg ;
							if ( ::PeekMessage
								( &msg, hwnd, WM_PAINT, WM_PAINT, PM_REMOVE ) )
							{
								Trace( "Dispatch WM_PAINT message in Wait.\n" ) ;
								::TranslateMessage( &msg ) ;
								::DispatchMessage( &msg ) ;
							}
						}
					}
					QuickLock() ;
				}
				pWindow = pWindow->EnumerateNextWindow() ;
			}
			QuickUnlock() ;
			*/
		}
		//
		DWORD	dwWaitResult = ::WaitForSingleObject( m_hSync, dwTimeout ) ;
		//
		if ( dwWaitResult == WAIT_TIMEOUT )
		{
			return	errTimeout ;
		}
		else if ( dwWaitResult != WAIT_OBJECT_0 )
		{
			return	errFailed ;
		}
		OnSignal() ;
		return	errSuccess ;

	#else
		uint64_t	timeStart = CurrentMilliSec() ;
		LockSimpleMutex( &m_mutex ) ;
		for ( ; ; )
		{
			atomic_int_t	nOldValue ;
			if ( m_value > 0 )
			{
				OnSignal() ;
				UnlockSimpleMutex( &m_mutex ) ;
				return	errSuccess ;
			}
			if ( msecTimeout != Infinite )
			{
				uint64_t	timePast = CurrentMilliSec() - timeStart ;
				if ( timePast >= (uint64_t) msecTimeout )
				{
					UnlockSimpleMutex( &m_mutex ) ;
					return	errTimeout ;
				}
			}
			nOldValue = m_value ;
			m_wait ++ ;
			UnlockSimpleMutex( &m_mutex ) ;
			//
			timespec	ts ;
			ts.tv_sec = 0 ;
			ts.tv_nsec = 1000000 ;		// 1[ms]
			//
			futex( (int*)&m_value, FUTEX_WAIT, nOldValue, &ts, NULL, 0 ) ;
			//
			LockSimpleMutex( &m_mutex ) ;
			m_wait -- ;
		}

	#endif
}

#if	!defined(__COTOPHA__) && !defined(__PLATFORM_WINDOWS__)

void SSynchronism::LockSimpleMutex( volatile atomic_int_t * mutex )
{
	atomic_int_t	valueLocked = __atomic_swap( futexLocked, mutex ) ;
	if ( valueLocked != futexWaked )
	{
		bool	fLocked = false ;
		for ( int i = 0; i < 0x1000; i ++ )
		{
			atomic_int_t	value = __atomic_swap( valueLocked, mutex ) ;
			if ( value == futexWaked )
			{
				fLocked = true ;
				break ;
			}
			if ( value == futexWait )
			{
				valueLocked = futexWait ;
			}
		}
		if ( !fLocked )
		{
			ESLTrace( "waiting for synchronize LockSimpleMutex\n" ) ;
			while ( __atomic_swap( futexWait, mutex ) != futexWaked )
			{
				timespec	ts ;
				ts.tv_sec = 0 ;
				ts.tv_nsec = 1000000 / 10 ;		// 0.1 [ms]
				//
				futex( (int*)mutex, FUTEX_WAIT, futexWait, &ts, NULL, 0 ) ;
			}
		}
	}
}

void SSynchronism::UnlockSimpleMutex( volatile atomic_int_t * mutex )
{
	if ( __atomic_swap( futexWaked, mutex ) == futexWait )
	{
		futex( (int*)mutex, FUTEX_WAKE, 1, NULL, NULL, 0 ) ;
	}
}

#endif

#if	!defined(__COTOPHA__)

void SSynchronism::OnSignal( void )
{
}

#endif


//////////////////////////////////////////////////////////////////////////////
// シグナルイベント同期クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSignalEvent, SSynchronism )

// 同期オブジェクト初期化
//////////////////////////////////////////////////////////////////////////////
void SSignalEvent::Initialize( bool fSignal )
{
	Delete() ;

	#if	defined(__COTOPHA__)
		m_pSync = SignalEvent::Create( fSignal ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		m_hSync = ::CreateEvent( NULL, TRUE, fSignal, NULL ) ;
		m_value = (fSignal ? 1 : 0) ;

	#else
		m_value = (fSignal ? 1 : 0) ;

	#endif
}

// シグナル値設定
//////////////////////////////////////////////////////////////////////////////
void SSignalEvent::SetSignal( void )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pSync != NULL ) ;
		((SignalEvent*)m_pSync)->SetSignal() ;

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hSync != NULL ) ;
		m_value = 1 ;
		::SetEvent( m_hSync ) ;

	#else
		bool	flagWake = (m_wait > 0) ;
		if ( __atomic_swap( 1, (atomic_int_t*) &m_value ) == 0 )
		{
			if ( flagWake )
			{
				futex( (int*)&m_value, FUTEX_WAKE, 0x20, NULL, NULL, 0 ) ;
			}
		}

	#endif
}

void SSignalEvent::ResetSignal( void )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pSync != NULL ) ;
		((SignalEvent*)m_pSync)->ResetSignal() ;

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hSync != NULL ) ;
		m_value = 0 ;
		::ResetEvent( m_hSync ) ;

	#else
		__atomic_swap( 0, (atomic_int_t*) &m_value ) ;

	#endif
}


#if	!defined(__COTOPHA__)

// 生成
//////////////////////////////////////////////////////////////////////////////
SSignalEvent * SSignalEvent::Create( bool fInitSignal )
{
	SSignalEvent *	pSignal = new SSignalEvent ;
	pSignal->Initialize( fInitSignal ) ;
	return	pSignal ;
}

#endif

#if	defined(__PLATFORM_WINDOWS__)

// シグナル値取得
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SSignalEvent::Value( void ) const
{
	if ( ::WaitForSingleObject( m_hSync, 0 ) == WAIT_OBJECT_0 )
	{
		return	1 ;
	}
	return	0 ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// セマフォ同期クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSemaphore, SSynchronism )

// 初期化
//////////////////////////////////////////////////////////////////////////////
void SSemaphore::Initialize( atomic_int_t nInitCount, atomic_int_t nMaxCount )
{
	Delete() ;

	#if	defined(__COTOPHA__)
		m_pSync = Semaphore::Create( nInitCount, nMaxCount ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		m_hSync = ::CreateSemaphore( NULL, nMaxCount, nMaxCount, NULL ) ;
		m_value = nMaxCount ;

	#else
		m_value = nMaxCount ;
		m_countMax = nMaxCount ;
	#endif
}

// セマフォ解放
//////////////////////////////////////////////////////////////////////////////
void SSemaphore::Release( void )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pSync != NULL ) ;
		if ( m_pSync != NULL )
		{
			((Semaphore*)m_pSync)->Release() ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		LONG	nPrevCount ;
		::ReleaseSemaphore( m_hSync, 1, &nPrevCount ) ;
		m_value = nPrevCount + 1 ;

	#else
		bool	flagWake = false ;
		LockSimpleMutex( &m_mutex ) ;
		if ( ++ m_value >= m_countMax )
		{
			m_value = m_countMax ;
		}
		flagWake = (m_wait > 0) ;
		UnlockSimpleMutex( &m_mutex ) ;

		if ( flagWake )
		{
			futex( (int*)&m_value, FUTEX_WAKE, 1, NULL, NULL, 0 ) ;
		}
	#endif
}

#if	!defined(__COTOPHA__) && !defined(__PLATFORM_WINDOWS__)

// シグナル時処理
//////////////////////////////////////////////////////////////////////////////
void SSemaphore::OnSignal( void )
{
	ESLAssert( m_value > 0 ) ;
	if ( m_value > 0 )
	{
		m_value -- ;
	}
}

#endif


#if	!defined(__COTOPHA__)

// オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
SSemaphore * SSemaphore::Create
	( atomic_int_t nInitCount, atomic_int_t nMaxCount )
{
	SSemaphore *	pSem = new SSemaphore ;
	pSem->Initialize( nInitCount, nMaxCount ) ;
	return	pSem ;
}

#endif



//////////////////////////////////////////////////////////////////////////////
// 軽量ミューテックス同期クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SCriticalSection, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SCriticalSection::SCriticalSection( void )
{
	#if	defined(__COTOPHA__)
		m_pMutex = new Mutex ;
	#elif	defined(__PLATFORM_WINDOWS__)
		::InitializeCriticalSection( &m_csMutex ) ;
		m_idOwnerThread = 0 ;
		m_countLocked = 0 ;
	#else
		m_mutex = SSynchronism::futexWaked ;
		m_value = SSynchronism::futexWaked ;
		m_wait = 0 ;
		m_pidOwner = 0 ;
		m_countLocked = 0 ;
	#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SCriticalSection::~SCriticalSection( void )
{
	#if	defined(__COTOPHA__)
		delete	m_pMutex ;
		m_pMutex = NULL ;
	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_countLocked == 0 ) ;
		::DeleteCriticalSection( &m_csMutex ) ;
	#else
		ESLAssert( m_countLocked == 0 ) ;
	#endif
}

// 同期
//////////////////////////////////////////////////////////////////////////////
void SCriticalSection::Lock( void ) const
{
	SCriticalSection *	pThis = (SCriticalSection*) this ;
	#if	defined(__COTOPHA__)
		while ( m_pMutex->Wait() != errSuccess )
		{
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		#if	defined(__DEBUG__)
			if ( !::TryEnterCriticalSection( &(pThis->m_csMutex) ) )
			{
				STimeCounter	timer ;
				BOOL	flagEntered = false ;
				while ( timer.GetRealTime() < 1000 )
				{
					::Sleep( 0 ) ;
					//
					flagEntered =
						::TryEnterCriticalSection( &(pThis->m_csMutex) ) ;
					if ( flagEntered )
					{
						break ;
					}
				}
				if ( !flagEntered )
				{
					ESLTrace( "thread #%08X is waiting CriticalSection (%08X) owned by thread #%08X.\n",
						SThread::GetCurrentId(), (ulong_ptr_t) this, m_idOwnerThread ) ;
					::EnterCriticalSection( &(pThis->m_csMutex) ) ;
					ESLTrace( "thread #%08X had CriticalSection (%08X).\n",
								SThread::GetCurrentId(), (ulong_ptr_t) this ) ;
				}
			}
		#else
			::EnterCriticalSection( &(pThis->m_csMutex) ) ;
		#endif

		pThis->m_idOwnerThread = ::GetCurrentThreadId() ;
		pThis->m_countLocked ++ ;

	#else
		pid_t	pidCurrent = gettid() ;
		SSynchronism::LockSimpleMutex( &(pThis->m_mutex) ) ;
		for ( ; ; )
		{
			if ( (m_value == SSynchronism::futexWaked)
				|| ((m_countLocked >= 1) && (pidCurrent == m_pidOwner)) )
			{
				pThis->m_pidOwner = pidCurrent ;
				pThis->m_countLocked ++ ;
				if ( pThis->m_value == SSynchronism::futexWaked )
				{
					pThis->m_value = SSynchronism::futexLocked ;
				}
				SSynchronism::UnlockSimpleMutex( &(pThis->m_mutex) ) ;
				return ;
			}
			pThis->m_wait ++ ;
			pThis->m_value = SSynchronism::futexWait ;
			SSynchronism::UnlockSimpleMutex( &(pThis->m_mutex) ) ;
			//
			timespec	ts ;
			ts.tv_sec = 0 ;
			ts.tv_nsec = 1000000/10 ;		// 0.1 [ms]
			//
			futex( (int*)&m_value, FUTEX_WAIT, SSynchronism::futexWait, &ts, NULL, 0 ) ;
			//
			SSynchronism::LockSimpleMutex( &(pThis->m_mutex) ) ;
			pThis->m_wait -- ;
		}

	#endif
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void SCriticalSection::Unlock( void ) const
{
	SCriticalSection *	pThis = const_cast<SCriticalSection*>( this ) ;
	#if	defined(__COTOPHA__)
		m_pMutex->Release() ;

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_countLocked > 0 ) ;
		ESLAssert( m_idOwnerThread == ::GetCurrentThreadId() ) ;
		if ( m_idOwnerThread == ::GetCurrentThreadId() )
		{
			if ( -- pThis->m_countLocked == 0 )
			{
				pThis->m_idOwnerThread = 0 ;
			}
		}
		::LeaveCriticalSection( &(pThis->m_csMutex) ) ;

	#else
		bool	flagWake = false ;
		pid_t	pidCurrent = gettid() ;
		SSynchronism::LockSimpleMutex( &(pThis->m_mutex) ) ;
		if ( (m_countLocked > 0) && (m_pidOwner == pidCurrent) )
		{
			pThis->m_countLocked -- ;
			if ( m_countLocked == 0 )
			{
				if ( (m_wait > 0) || (m_value == SSynchronism::futexWait) )
				{
					flagWake = true ;
				}
				pThis->m_pidOwner = 0 ;
				pThis->m_value = SSynchronism::futexWaked ;
			}
		}
		SSynchronism::UnlockSimpleMutex( &(pThis->m_mutex) ) ;
		//
		if ( flagWake )
		{
			futex( (int*)&m_value, FUTEX_WAKE, 1, NULL, NULL, 0 ) ;
		}
	#endif
}

#if	!defined(__COTOPHA__)
// 現在のスレッドが排他処理権を有しているか？
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SCriticalSection::TestLocked( void ) const
{
	#if	defined(__PLATFORM_WINDOWS__)
		if ( m_idOwnerThread == ::GetCurrentThreadId() )
		{
			return	m_countLocked ;
		}

	#else
		if ( m_pidOwner == gettid() )
		{
			return	m_countLocked ;
		}
	#endif
	return	0 ;
}

// 現在のスレッドが排他処理権を有している場合、全て解放する
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SCriticalSection::UnlockAll( void ) const
{
	atomic_int_t	nLocked = TestLocked() ;
	for ( int i = 0; i < nLocked; i ++ )
	{
		Unlock() ;
	}
	return	nLocked ;
}

// UnlockAll された排他処理を再度取得する
//////////////////////////////////////////////////////////////////////////////
void SCriticalSection::Relock( atomic_int_t nLock ) const
{
	for ( int i = 0; i < nLock; i ++ )
	{
		Lock() ;
	}
}

#endif


//////////////////////////////////////////////////////////////////////////////
// ミューテックス同期クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SMutex, SSynchronism )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SMutex::SMutex( void )
{
	#if	defined(__COTOPHA__)
	#elif	defined(__PLATFORM_WINDOWS__)
		m_countLocked = 0 ;
	#else
		m_pidOwner = 0 ;
		m_countLocked = 0 ;
	#endif
	#if	defined(__DEBUG__)
		m_pszSource = nullptr ;
		m_nLineNum = 0 ;
		m_nLockedTick = 0 ;
	#endif
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
void SMutex::Initialize( void )
{
	Delete() ;

	#if	defined(__COTOPHA__)
		m_pSync = Mutex::Create() ;

	#elif	defined(__PLATFORM_WINDOWS__)
		m_hSync = ::CreateMutex( NULL, false, NULL ) ;
		m_value = 1 ;
		m_idOwnerThread = 0 ;
		m_countLocked = 0 ;

	#else
		m_value = 1 ;
		m_pidOwner = 0 ;
		m_countLocked = 0 ;
	#endif
}

// Wait() の別名
//////////////////////////////////////////////////////////////////////////////
SError SMutex::LockTrace
		( const char * pszSource,
			size_t nLineNum, int64_t msecTimeout ) const
{
	#if	defined(__DEBUG__)
		SError	err = ((SMutex*)this)->Wait( msecTimeout ) ;
		if ( (err == errSuccess) && (m_countLocked == 1) )
		{
			((SMutex*)this)->m_pszSource = pszSource ;
			((SMutex*)this)->m_nLineNum = nLineNum ;
			((SMutex*)this)->m_nLockedTick = CurrentMilliSec() ;
		}
		return	err ;
	#else
		return	((SMutex*)this)->Wait( msecTimeout ) ;
	#endif
}

SError SMutex::Lock( int64_t msecTimeout ) const
{
	#if	defined(__DEBUG__)
		SError	err ;
		if ( msecTimeout == Infinite )
		{
			err = ((SMutex*)this)->Wait( 1000 ) ;
			if ( err != errSuccess )
			{
				SThread::IdType	idOwnThread = 0 ;
				#if	defined(__PLATFORM_WINDOWS__)
					idOwnThread = m_idOwnerThread ;
				#else
					idOwnThread = m_pidOwner ;
				#endif
				ESLTrace( "thread #%08X is waiting mutex (%08X) owned by thread #%08X.\n",
							SThread::GetCurrentId(), (ulong_ptr_t) this, idOwnThread ) ;
				err = ((SMutex*)this)->Wait( Infinite ) ;
				ESLTrace( "thread #%08X had mutex (%08X).\n",
							SThread::GetCurrentId(), (ulong_ptr_t) this ) ;
			}
		}
		else
		{
			err = ((SMutex*)this)->Wait( msecTimeout ) ;
		}
		if ( (err == errSuccess) && (m_countLocked == 1) )
		{
			((SMutex*)this)->m_pszSource = NULL ;
			((SMutex*)this)->m_nLineNum = 0 ;
			((SMutex*)this)->m_nLockedTick = CurrentMilliSec() ;
		}
		return	err ;
	#else
		return	((SMutex*)this)->Wait( msecTimeout ) ;
	#endif
}

// Release() の別名
//////////////////////////////////////////////////////////////////////////////
void SMutex::Unlock( void ) const
{
	((SMutex*)this)->Release() ;
}

// 排他的処理時間
//////////////////////////////////////////////////////////////////////////////
uint32_t SMutex::UnlockLatency( void ) const
{
	#if	defined(__DEBUG__)
		if ( TestLocked() > 0 )
		{
			uint32_t	msecLatency =
				(uint32_t) (CurrentMilliSec() - m_nLockedTick) ;
			((SMutex*)this)->Release() ;
			return	msecLatency ;
		}
	#else
		((SMutex*)this)->Release() ;
	#endif
	return	0 ;
}

// ミューテックス解放
//////////////////////////////////////////////////////////////////////////////
void SMutex::Release( void )
{
	#if	defined(__COTOPHA__)
		ESLAssert( m_pSync != NULL ) ;
		if ( m_pSync != NULL )
		{
			((Mutex*)m_pSync)->Release() ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hSync != NULL ) ;
		ESLAssert( m_countLocked > 0 ) ;
		ESLAssert( m_idOwnerThread == ::GetCurrentThreadId() ) ;
		atomic_int_t	countLocked = AtomicSub( &m_countLocked, 1 ) ;
		atomic_int_t	valueLast = m_value ;
		ESLVerify( countLocked >= 0 ) ;
		if ( countLocked == 0 )
		{
			m_value = 1 ;
		}
		if ( !::ReleaseMutex( m_hSync ) )
		{
			AtomicAdd( &m_countLocked, 1 ) ;
			m_value = valueLast ;
		}

	#else
		bool	flagWake = false ;
		pid_t	pidCurrent = gettid() ;
		LockSimpleMutex( &m_mutex ) ;
		if ( (m_countLocked > 0) && (m_pidOwner == pidCurrent) )
		{
			m_countLocked -- ;
			if ( m_countLocked == 0 )
			{
				m_pidOwner = 0 ;
				m_value = 1 ;
				flagWake = (m_wait > 0) ;
			}
		}
		UnlockSimpleMutex( &m_mutex ) ;
		//
		if ( flagWake )
		{
			futex( (int*)&m_value, FUTEX_WAKE, 1, NULL, NULL, 0 ) ;
		}
	#endif
}

// シグナル時処理
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)

#elif	defined(__PLATFORM_WINDOWS__)

void SMutex::OnSignal( void )
{
	m_idOwnerThread = ::GetCurrentThreadId() ;
	AtomicAdd( &m_countLocked, 1 ) ;
	m_value = 0 ;
}

#else

SError SMutex::Wait( int64_t msecTimeout )
{
	uint64_t	timeStart = CurrentMilliSec() ;
	pid_t		pidCurrent = gettid() ;
	LockSimpleMutex( &m_mutex ) ;
	for ( ; ; )
	{
		atomic_int_t	nOldValue ;
		if ( (m_value > 0)
			|| ((m_countLocked >= 1) && (pidCurrent == m_pidOwner)) )
		{
			OnSignal() ;
			UnlockSimpleMutex( &m_mutex ) ;
			return	errSuccess ;
		}
		if ( msecTimeout != Infinite )
		{
			uint64_t	timePast = CurrentMilliSec() - timeStart ;
			if ( timePast >= (uint64_t) msecTimeout )
			{
				UnlockSimpleMutex( &m_mutex ) ;
				return	errTimeout ;
			}
		}
		m_wait ++ ;
		nOldValue = m_value ;
		UnlockSimpleMutex( &m_mutex ) ;
		//
		timespec	ts ;
		ts.tv_sec = 0 ;
		ts.tv_nsec = 1000000 ;		// 1 [ms]
		//
		futex( (int*)&m_value, FUTEX_WAIT, nOldValue, &ts, NULL, 0 ) ;
		//
		LockSimpleMutex( &m_mutex ) ;
		m_wait -- ;
	}
}

void SMutex::OnSignal( void )
{
	m_pidOwner = gettid() ;
	m_countLocked ++ ;
	m_value = 0 ;
}

#endif


#if	!defined(__COTOPHA__)

// オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
SMutex * SMutex::Create( void )
{
	return	new SMutex ;
}

// 現在のスレッドが排他処理権を有しているか？
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SMutex::TestLocked( void ) const
{
	atomic_int_t	nLock = 0 ;

	#if	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hSync != NULL ) ;
		if ( (m_hSync != NULL)
			&& (m_idOwnerThread == ::GetCurrentThreadId()) )
		{
			if ( ::WaitForSingleObject(m_hSync,0) == WAIT_OBJECT_0 )
			{
				ESLAssert( m_countLocked >= 0 ) ;
				nLock = m_countLocked ;
				::ReleaseMutex( m_hSync ) ;
			}
		}

	#else
		pid_t	pidCurrent = gettid() ;
		LockSimpleMutex( &(((SMutex*)this)->m_mutex) ) ;
		if ( (m_countLocked > 0) && (m_pidOwner == pidCurrent) )
		{
			nLock = m_countLocked ;
		}
		UnlockSimpleMutex( &(((SMutex*)this)->m_mutex) ) ;

	#endif

	return	nLock ;
}

// 排他的処理時間チェック付き Release
//////////////////////////////////////////////////////////////////////////////
uint32_t SMutex::UnlockVerifyLatency( uint32_t msecMargin ) const
{
#if	defined(__DEBUG__)
	uint32_t	msecLatency = UnlockLatency() ;
	if ( msecLatency >= msecMargin )
	{
		if ( m_pszSource != NULL )
		{
			ESLTrace( "Verify locked from %s (%d) : latency %d[ms]\n",
							m_pszSource, m_nLineNum, msecLatency ) ;
		}
		else
		{
			ESLTrace( "Verify locked : latency %d[ms]\n", msecLatency ) ;
		}
	}
	return	msecLatency ;
#else
	Unlock() ;
	return	0 ;
#endif
}

// 現在のスレッドが排他処理権を有している場合、全て解放する
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SMutex::UnlockAll( void )
{
	atomic_int_t	nLock = 0 ;

	#if	defined(__PLATFORM_WINDOWS__)
		ESLAssert( m_hSync != NULL ) ;
		if ( (m_hSync != NULL)
			&& (m_idOwnerThread == ::GetCurrentThreadId()) )
		{
			if ( ::WaitForSingleObject(m_hSync,0) == WAIT_OBJECT_0 )
			{
				ESLAssert( m_countLocked >= 0 ) ;
				nLock = m_countLocked ;
				for ( atomic_int_t i = nLock; i > 0; i -- )
				{
					::ReleaseMutex( m_hSync ) ;
					ESLVerify( AtomicSub( &m_countLocked, 1 ) >= 0 ) ;
				}
				::ReleaseMutex( m_hSync ) ;
			}
		}

	#else
		pid_t	pidCurrent = gettid() ;
		LockSimpleMutex( &m_mutex ) ;
		if ( (m_countLocked > 0) && (m_pidOwner == pidCurrent) )
		{
			nLock = m_countLocked ;
			m_countLocked = 0 ;
			m_pidOwner = 0 ;
			m_value = 1 ;
		}
		UnlockSimpleMutex( &m_mutex ) ;

	#endif

	return	nLock ;
}

// UnlockAll された排他処理を再度取得する
//////////////////////////////////////////////////////////////////////////////
SError SMutex::Relock( atomic_int_t nLock )
{
	#if	defined(__PLATFORM_WINDOWS__)
		for ( atomic_int_t i = 0; i < nLock; i ++ )
		{
			if ( ::WaitForSingleObject(m_hSync,INFINITE) != WAIT_OBJECT_0 )
			{
				return	errFailed ;
			}
			OnSignal() ;
//			m_idOwnerThread = ::GetCurrentThreadId() ;
//			AtomicAdd( &m_countLocked, 1 ) ;
		}

	#else
		if ( nLock > 0 )
		{
			pid_t	pidCurrent = gettid() ;
			LockSimpleMutex( &m_mutex ) ;
			for ( ; ; )
			{
				if ( (m_value > 0)
					|| ((m_countLocked >= 1) && (pidCurrent == m_pidOwner)) )
				{
					for ( atomic_int_t i = 0; i < nLock; i ++ )
					{
						OnSignal() ;
					}
//					m_pidOwner = gettid() ;
//					m_countLocked = nLock ;
//					m_value = 0 ;
					UnlockSimpleMutex( &m_mutex ) ;
					return	errSuccess ;
				}
				m_wait ++ ;
				UnlockSimpleMutex( &m_mutex ) ;
				//
				timespec	ts ;
				ts.tv_sec = 0 ;
				ts.tv_nsec = 1000000 ;
				//
				futex( (int*)&m_value, FUTEX_WAIT, m_value, &ts, NULL, 0 ) ;
				//
				LockSimpleMutex( &m_mutex ) ;
				m_wait -- ;
			}
		}

	#endif
	return	errSuccess ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// 共有可能ミューテックス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSharableMutex, SMutex )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSharableMutex::SSharableMutex( void )
{
	m_nShareThreads = 0 ;
}

// 共有スレッド追加（既に他のスレッドが所有権を獲得している状態で呼び出す）
//////////////////////////////////////////////////////////////////////////////
SError SSharableMutex::SharedLock( void )
{
	SError	err = SMutex::Wait( 0 ) ;
//	ESLAssert( err != errSuccess ) ;
	if ( err == errTimeout )
	{
		QuickLock() ;
		ssize_t	iThread = FindSharedThread() ;
		if ( iThread >= 0 )
		{
			m_ShareThreads[iThread].nLocked ++ ;
		}
		else
		{
			ESLAssert( m_nShareThreads < maxThreadCount ) ;
			if ( m_nShareThreads >= maxThreadCount )
			{
				QuickUnlock() ;
				return	errFailed ;
			}
			iThread = (ssize_t) m_nShareThreads ++ ;
			//
			#if	defined(__PLATFORM_WINDOWS__)
				m_ShareThreads[iThread].idThread = ::GetCurrentThreadId() ;
			#else
				m_ShareThreads[iThread].idThread = gettid() ;
			#endif
			m_ShareThreads[iThread].nLocked = 1 ;
		}
		QuickUnlock() ;
	}
	return	errSuccess ;
}

// 共有スレッド削除（所有権を獲得しているスレッドが解放する前に呼び出す）
//////////////////////////////////////////////////////////////////////////////
SError SSharableMutex::SharedUnlock( void )
{
	SError	err = errSuccess ;
//	ESLAssert( m_nShareThreads >= 2 ) ;
	QuickLock() ;
	ssize_t	iThread = FindSharedThread() ;
//	ESLAssert( iThread > 0 ) ;
	if ( iThread > 0 )
	{
		UnlockSharedThread( (size_t) iThread ) ;
		QuickUnlock() ;
	}
	else if ( iThread == 0 )
	{
		UnlockSharedThread( (size_t) iThread ) ;
		QuickUnlock() ;
		ESLAssert( (m_countLocked > 1) || (m_nShareThreads == 0) ) ;
		SMutex::Release() ;
	}
	else
	{
		QuickUnlock() ;
		err = errFailed ;
	}
	return	err ;
}

// 共有スレッド判定
//////////////////////////////////////////////////////////////////////////////
ssize_t SSharableMutex::FindSharedThread( void ) const
{
	#if	defined(__PLATFORM_WINDOWS__)
		ThreadID	tid = ::GetCurrentThreadId() ;
	#else
		ThreadID	tid = gettid() ;
	#endif
	for ( size_t i = 0; i < m_nShareThreads; i ++ )
	{
		if ( m_ShareThreads[i].idThread == tid )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// スレッド・ロックカウンタ減算
//////////////////////////////////////////////////////////////////////////////
void SSharableMutex::UnlockSharedThread( size_t iThread )
{
	m_ShareThreads[iThread].nLocked -- ;
	if ( m_ShareThreads[iThread].nLocked == 0 )
	{
		for ( size_t i = iThread + 1; i < m_nShareThreads; i ++ )
		{
			m_ShareThreads[i - 1] = m_ShareThreads[i] ;
		}
		m_nShareThreads -- ;
	}
}

// ミューテックス解放
//////////////////////////////////////////////////////////////////////////////
void SSharableMutex::Release( void )
{
	ESLAssert( m_nShareThreads > 0 ) ;
	if ( m_nShareThreads > 0 )
	{
		QuickLock() ;
		ssize_t	iThread = FindSharedThread() ;
		ESLAssert( iThread >= 0 ) ;
		if ( iThread >= 0 )
		{
			ESLAssert( m_ShareThreads[iThread].nLocked > 0 ) ;
			UnlockSharedThread( (size_t) iThread ) ;
			QuickUnlock() ;
			//
			if ( iThread == 0 )
			{
				ESLAssert( (m_countLocked > 1) || (m_nShareThreads == 0) ) ;
				SMutex::Release() ;
			}
			return ;
		}
		QuickUnlock() ;
	}
}

// 所有権獲得
//////////////////////////////////////////////////////////////////////////////
SError SSharableMutex::Wait( int64_t msecTimeout )
{
	SError	err = SMutex::Wait( 0 ) ;
	if ( err == errSuccess )
	{
		return	err ;
	}
	QuickLock() ;
	ssize_t	iThread = FindSharedThread() ;
	if ( iThread >= 0 )
	{
		ESLAssert( iThread > 0 ) ;
		m_ShareThreads[iThread].nLocked ++ ;
		QuickUnlock() ;
		return	errSuccess ;
	}
	QuickUnlock() ;
	//
	return	SMutex::Wait( msecTimeout ) ;
}

// 所有権獲得時
//////////////////////////////////////////////////////////////////////////////
void SSharableMutex::OnSignal( void )
{
	SMutex::OnSignal() ;
	//
	ThreadID	tid ;
	#if	defined(__PLATFORM_WINDOWS__)
		tid = m_idOwnerThread ;
	#else
		tid = m_pidOwner ;
	#endif
	//
	QuickLock() ;
	if ( m_nShareThreads == 0 )
	{
		m_ShareThreads[0].idThread = tid ;
		m_ShareThreads[0].nLocked = 1 ;
		m_nShareThreads = 1 ;
	}
	else
	{
		ESLAssert( m_ShareThreads[0].idThread == tid ) ;
		if ( m_ShareThreads[0].idThread == tid )
		{
			m_ShareThreads[0].nLocked ++ ;
		}
	}
	QuickUnlock() ;
}

// 現在のスレッドが排他処理権を有しているか？
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SSharableMutex::TestLocked( void ) const
{
	QuickLock() ;
	ssize_t	iThread = FindSharedThread() ;
	if ( iThread >= 0 )
	{
		atomic_int_t	nLocked =
				(atomic_int_t) m_ShareThreads[iThread].nLocked ;
		QuickUnlock() ;
		return	nLocked ;
	}
	QuickUnlock() ;
	return	0 ;
}

// 現在のスレッドが排他処理権を有している場合、全て解放する
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SSharableMutex::UnlockAll( void )
{
	QuickLock() ;
	ESLAssert( m_nShareThreads <= 1 ) ;
	if ( m_nShareThreads != 1 )
	{
		ESLAssert( m_countLocked == 0 ) ;
		QuickUnlock() ;
		return	0 ;
	}
	ssize_t	iThread = FindSharedThread() ;
	if ( iThread == 0 )
	{
		m_nShareThreads = 0 ;
	}
	QuickUnlock() ;
	return	SMutex::UnlockAll() ;
}

// UnlockAll された排他処理を再度取得する
//////////////////////////////////////////////////////////////////////////////
SError SSharableMutex::Relock( atomic_int_t nLock )
{
	return	SMutex::Relock( nLock ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 状態別ミューテックス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SStateMutex, SSynchronismInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SStateMutex::SStateMutex( void )
	: m_state( 0 ), m_type( typeExclusive ),
		m_countLocked( 0 ), m_idThread( SThread::InvalidId )
{
	m_signalFree.Initialize( true ) ;
}

// 待機（デフォルトの状態でロックする）
//////////////////////////////////////////////////////////////////////////////
SError SStateMutex::Wait( int64_t msecTimeout )
{
	return	LockState( m_state, m_type, msecTimeout ) ;
}

// 指定の状態でロックする
//////////////////////////////////////////////////////////////////////////////
SError SStateMutex::LockState
	( int state, StateType type, int64_t msecTimeout )
{
	for ( ; ; )
	{
		m_csState.Lock() ;
		if ( (m_countLocked == 0)
			|| ((m_state == state) && (m_type == type)) )
		{
			SThread::IdType	idThread = SThread::GetCurrentId() ;
			if ( (m_countLocked == 0)
				|| (type == typeSharable)
				|| (m_idThread == idThread) )
			{
				if ( ++ m_countLocked == 1 )
				{
					m_signalFree.ResetSignal() ;
				}
				m_state = state ;
				m_type = type ;
				m_idThread = idThread ;
				m_csState.Unlock() ;
				return	errSuccess ;
			}
		}
		else if ( (m_countLocked > 0)
				&& (SThread::GetCurrentId() == m_idThread) )
		{
			// ※既に Lock しているスレッドは、同じステータスでロックしなければならない
			ESLAssert( (m_state == state) && (m_type == type) ) ;
			m_csState.Unlock() ;
			break ;
		}
		m_csState.Unlock() ;
		if ( msecTimeout == 0 )
		{
			break ;
		}
		SError	err = m_signalFree.Wait( msecTimeout ) ;
		if ( err != errSuccess )
		{
			return	err ;
		}
		if ( msecTimeout != Infinite )
		{
			msecTimeout = 0 ;
		}
	}
	return	errTimeout ;
}

// ロックを解除する
//////////////////////////////////////////////////////////////////////////////
void SStateMutex::Unlock( void )
{
	m_csState.Lock() ;
	ESLAssert( m_countLocked > 0 ) ;
	if ( -- m_countLocked <= 0 )
	{
		m_countLocked = 0 ;
		m_signalFree.SetSignal() ;
	}
	m_csState.Unlock() ;
}



//////////////////////////////////////////////////////////////////////////////
// Read/Write 状態別ミューテックス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SReadWriteMutex, SStateMutex )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SReadWriteMutex::SReadWriteMutex( void )
{
	m_state = stateWrite ;
}

// Read 用にロックする（他のスレッドも Read 出来る）
//////////////////////////////////////////////////////////////////////////////
SError SReadWriteMutex::LockRead( int64_t msecTimeout )
{
	return	SStateMutex::LockState( stateRead, typeSharable, msecTimeout ) ;
}

// Write 用にロックする（Unlock するまで他のスレッドはロックできない）
//////////////////////////////////////////////////////////////////////////////
SError SReadWriteMutex::LockWrite( int64_t msecTimeout )
{
	return	SStateMutex::LockState( stateWrite, typeExclusive, msecTimeout ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 全体ミューテックス
//////////////////////////////////////////////////////////////////////////////

SError SSystem::Lock( int64_t msecTimeout )
{
	#if	defined(__COTOPHA__)
		if ( msecTimeout == Synchronism::Infinite )
		{
			while ( LockSystem( msecTimeout ) != errSuccess )
			{
			}
			return	errSuccess ;
		}
		else
		{
			return	LockSystem( msecTimeout ) ;
		}
	#else
		ESLAssert( g_mutexGlobal != NULL ) ;
//		ESLAssert( !g_csmutexGlobal || (g_csmutexGlobal->TestLocked() == 0) ) ;
		return	g_mutexGlobal->Lock( msecTimeout ) ;
	#endif
}

SError SSystem::LockTrace
	( const char * pszSource, size_t nLineNum, int64_t msecTimeout )
{
	#if	defined(__COTOPHA__)
		if ( msecTimeout == Synchronism::Infinite )
		{
			while ( LockSystem( msecTimeout ) != errSuccess )
			{
			}
			return	errSuccess ;
		}
		else
		{
			return	LockSystem( msecTimeout ) ;
		}
	#else
		ESLAssert( g_mutexGlobal != NULL ) ;
//		ESLAssert( !g_csmutexGlobal || (g_csmutexGlobal->TestLocked() == 0) ) ;
		#if	defined(__DEBUG__)
			return	g_mutexGlobal->LockTrace( pszSource, nLineNum, msecTimeout ) ;
		#else
			return	g_mutexGlobal->Lock( msecTimeout ) ;
		#endif
	#endif
}

SError SSystem::Unlock( void )
{
	#if	defined(__COTOPHA__)
		return	UnlockSystem() ;
	#else
		ESLAssert( g_mutexGlobal != NULL ) ;
		g_mutexGlobal->Unlock() ;
		return	errSuccess ;
	#endif
}

atomic_int_t SSystem::UnlockAll( void )
{
	#if	defined(__COTOPHA__)
		return	UnlockAllSystem() ;
	#else
		ESLAssert( g_mutexGlobal != NULL ) ;
		return	g_mutexGlobal->UnlockAll() ;
	#endif
}

SError SSystem::Relock( atomic_int_t nLock )
{
	#if	defined(__COTOPHA__)
		for ( atomic_int_t i = 0; i < nLock; i ++ )
		{
			Lock() ;
		}
		return	errSuccess ;
	#else
		ESLAssert( g_mutexGlobal != NULL ) ;
		return	g_mutexGlobal->Relock( nLock ) ;
	#endif
}



//////////////////////////////////////////////////////////////////////////////
// イベント通知
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SEventSignalNotifier, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SEventSignalNotifier::SEventSignalNotifier( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SEventSignalNotifier::~SEventSignalNotifier( void )
{
}

// 通知対象追加
//////////////////////////////////////////////////////////////////////////////
void SEventSignalNotifier::AttachSignalEvent( SSignalEvent * pSignal )
{
	ESLAssert( pSignal != nullptr ) ;
	m_csSync.Lock() ;
	if ( pSignal != nullptr )
	{
		m_aSignals.Add( pSignal ) ;
	}
	m_csSync.Unlock() ;
}

// 通知対象解除
//////////////////////////////////////////////////////////////////////////////
void SEventSignalNotifier::DetachSignalEvent( SSignalEvent * pSignal )
{
	m_csSync.Lock() ;
	ssize_t	i = m_aSignals.FindPtr( pSignal ) ;
	if ( i >= 0 )
	{
		m_aSignals.RemoveAt( (size_t) i ) ;
	}
	m_csSync.Unlock() ;
}

void SEventSignalNotifier::DetachAllSignalEvents( void )
{
	m_csSync.Lock() ;
	m_aSignals.RemoveAll() ;
	m_csSync.Unlock() ;
}

// 通知
//////////////////////////////////////////////////////////////////////////////
void SEventSignalNotifier::NotifyEventSignal( bool afterDetachAll )
{
	if ( m_aSignals.GetLength() > 0 )
	{
		m_csSync.Lock() ;
		for ( size_t i = 0; i < m_aSignals.GetLength(); i ++ )
		{
			SSignalEvent *	pSignal = m_aSignals.GetAt( i ) ;
			ESLAssert( pSignal != nullptr ) ;
			if ( pSignal != nullptr )
			{
				pSignal->SetSignal() ;
			}
		}
		if ( afterDetachAll )
		{
			m_aSignals.RemoveAll() ;
		}
		m_csSync.Unlock() ;
	}
}


