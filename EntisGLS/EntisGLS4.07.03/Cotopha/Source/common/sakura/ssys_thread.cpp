
#include <sakura/sakura.h>

#if	defined(__PLATFORM_ANDROID__)
#include <esl/esl_java_object.h>
#endif


using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 関数抽象クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SSystem::SProcedure )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SProcedure::~SProcedure( void )
{
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void SProcedure::Prepare( void )
{
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SProcedure::Finalize( void )
{
}

// 処理の即時終了要求
//////////////////////////////////////////////////////////////////////////////
void SProcedure::RequestQuit( SProcedure::RequestQuitLevel rql )
{
}



//////////////////////////////////////////////////////////////////////////////
// 関数呼び出しクラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SProcedureCaller, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SProcedureCaller::SProcedureCaller
		( SProcedureCaller::PFUNC_PTR pfnFunc, void * pInstance )
	: m_pfnFunc(pfnFunc), m_pInstance(pInstance)
{
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SProcedureCaller::Run( void )
{
	m_pfnFunc( m_pInstance ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 同期関数
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SSyncProcedure, ESLObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSyncProcedure::SSyncProcedure( SProcedure * pProc, bool flagOwner )
{
	m_flagAutoDelete = false ;
	m_flagFinished = false ;
	m_flagProcOwner = flagOwner ;
	m_countWaitRef = 0 ;
	m_pProc = pProc ;
	m_done.Initialize( false ) ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SSyncProcedure::Run( void )
{
	if ( m_pProc != NULL )
	{
		m_pProc->Run() ;
	}
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void SSyncProcedure::Prepare( void )
{
	if ( m_pProc != NULL )
	{
		m_pProc->Prepare() ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void SSyncProcedure::Finalize( void )
{
	if ( m_pProc != NULL )
	{
		m_pProc->Finalize() ;
		if ( m_flagProcOwner )
		{
			delete	m_pProc ;
			m_pProc = NULL ;
		}
	}
	m_csSync.Lock() ;
	m_done.SetSignal() ;
	if ( m_flagAutoDelete && (m_countWaitRef == 0) )
	{
		m_csSync.Unlock() ;
		delete	this ;
	}
	else
	{
		m_flagFinished = true ;
		m_csSync.Unlock() ;
	}
}

// 終了後に自身を消去するか設定する
//////////////////////////////////////////////////////////////////////////////
void SSyncProcedure::SetAutoDelete( bool flagAutoDelete )
{
	m_csSync.Lock() ;
	m_flagAutoDelete = flagAutoDelete ;
	if ( m_flagFinished && flagAutoDelete )
	{
		m_csSync.Unlock() ;
		delete	this ;
	}
	else
	{
		m_csSync.Unlock() ;
	}
}

// 完了待ち
//////////////////////////////////////////////////////////////////////////////
SError SSyncProcedure::WaitDone( int64_t timeout )
{
	m_csSync.Lock() ;
	m_countWaitRef ++ ;
	m_csSync.Unlock() ;
	//
	SError	err = m_done.Wait( timeout ) ;
	//
	m_csSync.Lock() ;
	m_countWaitRef -- ;
	if ( m_flagFinished && m_flagAutoDelete && (m_countWaitRef == 0) )
	{
		m_csSync.Unlock() ;
		delete	this ;
	}
	else
	{
		m_csSync.Unlock() ;
	}
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// 例外クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SException, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SException::SException( void )
	: m_nError(0)
{
}

SException::SException( const SException& e )
	: m_nError(e.m_nError), m_strMessage(e.m_strMessage)
{
}

SException::SException( int64_t nError, const wchar_t * pwszMessage )
	: m_nError(nError), m_strMessage(pwszMessage)
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SException::~SException( void )
{
}

// エラーコード取得
//////////////////////////////////////////////////////////////////////////////
int64_t SException::GetError( void ) const
{
	return	m_nError ;
}

// メッセージ取得
//////////////////////////////////////////////////////////////////////////////
const SString& SException::GetMessage( void ) const
{
	return	m_strMessage ;
}


//////////////////////////////////////////////////////////////////////////////
// スレッドオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SThread, SSynchronism )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SThread::SThread( void )
{
	m_pProc = NULL ;

	#if	defined(__COTOPHA__)
	m_pThread = NULL ;

	#else
		#if	defined(__PLATFORM_WINDOWS__)
			m_countPending = 0 ;
			m_dwThreadID = 0 ;

			#else
			m_flagThread = false ;
			m_idThread = 0 ;
		#endif
		m_flagFinished = true ;

	#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SThread::~SThread( void )
{
	SThread::Delete() ;
}

#if	defined(__COTOPHA__)

// 詞葉 Sakura2 のスレッド関数
//////////////////////////////////////////////////////////////////////////////
void SThread::SThreadProcecure::Run( void )
{
	SProcedure *	pProc = m_pThread->m_pProc ;
	pProc->Prepare() ;
	pProc->Run() ;
	pProc->Finalize() ;
}

#else

// Windows/POSIX のスレッド関数
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_WINDOWS__)
DWORD WINAPI SThread::ThreadProc( LPVOID lpParameter )
#else
void * SThread::ThreadProc( void * lpParameter )
#endif
{
	SThread *	pThread = (SThread*) lpParameter ;
	pThread->AttachCurrentThread() ;
	//
	SProcedure *	pProc = pThread->m_pProc ;
	pProc->Prepare() ;
	pProc->Run() ;
	pProc->Finalize() ;
	//
	pThread->DetachCurrentThread() ;

#if	defined(__PLATFORM_WINDOWS__)
	return	0 ;
#else
	return	nullptr ;
#endif
}

#if	defined(__PLATFORM_WINDOWS__)
DWORD WINAPI SThread::FrameThreadProc( LPVOID lpParameter )
#else
void * SThread::FrameThreadProc( void * lpParameter )
#endif
{
	SThread *	pThread = (SThread*) lpParameter ;
	pThread->AttachCurrentThread() ;
//	pThread->m_signalMaster.SetSignal() ;
//	pThread->m_signalFrame.Wait( SSynchronism::Infinite ) ;
//	pThread->m_signalFrame.ResetSignal() ;
	//
	SProcedure *	pProc = pThread->m_pProc ;
	try
	{
		SleepFrame( 1 ) ;
		pProc->Prepare() ;
		pProc->Run() ;
		pProc->Finalize() ;
	}
	catch ( SException& e )
	{
		SSystem::Trace
			( "処理されない例外がフレーム駆動スレッドで投げられました\n%s\n",
				e.GetMessage().ToCharArray().GetConstArray() ) ;
	}
	pThread->m_flagFinished = true ;
	pThread->m_signalMaster.SetSignal() ;
	pThread->DetachCurrentThread() ;

#if	defined(__PLATFORM_WINDOWS__)
	return	0 ;
#else
	return	nullptr ;
#endif
}

#endif

// 同期オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void SThread::Delete( void )
{
	#if	defined(__COTOPHA__)
		if ( m_pThread != NULL )
		{
			delete	m_pThread ;
			m_pThread = NULL ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		m_signalMaster.Delete() ;
		m_signalFrame.Delete() ;

	#else
		if ( m_flagThread )
		{
			void *	ret_val ;
			m_signalDone.Wait() ;
			pthread_join( m_idThread, &ret_val ) ;
			m_signalDone.Delete() ;
			m_flagThread = false ;
		}

	#endif
	m_pProc = NULL ;
	SSynchronism::Delete() ;
}

// シグナル値取得
//////////////////////////////////////////////////////////////////////////////
atomic_int_t SThread::Value( void ) const
{
	#if	defined(__COTOPHA__)
		if ( m_pThread != NULL )
		{
			return	(m_pThread->Wait(0) == errSuccess) ;
		}
		return	0 ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( ::WaitForSingleObject( m_hSync, 0 ) == WAIT_OBJECT_0 )
		{
			return	1 ;
		}
		return	0 ;

	#else
		return	m_signalDone.Value() ;
	#endif
}

#if	!defined(__PLATFORM_WINDOWS__)

// 待機
//////////////////////////////////////////////////////////////////////////////
SError SThread::Wait( int64_t msecTimeout )
{
	#if	defined(__COTOPHA__)
		if ( m_pThread != NULL )
		{
			return	m_pThread->Wait( msecTimeout ) ;
		}
		return	errFailed ;

	#else
		return	m_signalDone.Wait( msecTimeout ) ;
	#endif
}

#endif

// スレッド実行中？
//////////////////////////////////////////////////////////////////////////////
bool SThread::IsRunning( void ) const
{
	#if	defined(__COTOPHA__)
		if ( m_pThread != NULL )
		{
			return	m_pThread->IsRunning() ;
		}
		return	false ;

	#elif	defined(__PLATFORM_WINDOWS__)
		if ( m_hSync != NULL )
		{
			return	(::WaitForSingleObject( m_hSync, 0 ) != WAIT_OBJECT_0) ;
		}
		return	false ;

	#else
		if ( m_idThread != 0 )
		{
			return	(m_signalDone.Value() == 0) ;
		}
		return	false ;
	#endif
}

// スレッド起動
//////////////////////////////////////////////////////////////////////////////
SError SThread::BeginThread( SProcedure * pProc )
{
	if ( m_pProc != NULL )
	{
		return	errFailed ;
	}
	m_pProc = pProc ;

	#if	defined(__COTOPHA__)
		m_pThread = new Thread ;
		m_procRedirect.m_pThread = this ;
		return	m_pThread->BeginThread( &m_procRedirect ) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		m_hSync = ::CreateThread
			( NULL, 0, &SThread::ThreadProc, this, 0, &m_dwThreadID ) ;
		if ( m_hSync == NULL )
		{
			return	errFailed ;
		}

	#else
		m_flagThread = true ;
		m_signalDone.Initialize( false ) ;
		if ( pthread_create( &m_idThread, NULL, &SThread::ThreadProc, this ) )
		{
			m_flagThread = false ;
			m_signalDone.Delete() ;
			return	errFailed ;
		}

	#endif

	return	errSuccess ;
}

#if	defined(__COTOPHA__)

// フレーム駆動スレッド起動
//////////////////////////////////////////////////////////////////////////////
SError SThread::BeginFrameThread( SProcedure * pProc, size_t nInitStack )
{
	if ( m_pProc != NULL )
	{
		return	errFailed ;
	}
	m_pProc = pProc ;
	//
	m_pThread = new Thread ;
	m_procRedirect.m_pThread = this ;
	return	m_pThread->BeginFrameThread( &m_procRedirect, nInitStack ) ;
}

// フレーム駆動
//////////////////////////////////////////////////////////////////////////////
SError SThread::ContinueFrameThread( void )
{
	if ( m_pThread == NULL )
	{
		return	errFailed ;
	}
	return	m_pThread->ContinueFrameThread() ;
}

// フレーム同期
//////////////////////////////////////////////////////////////////////////////
SError SThread::SyncFrameThread( void )
{
	return	errPending ;
}

// 例外をスロー
//////////////////////////////////////////////////////////////////////////////
SError SThread::ThrowException( const wchar_t * pszErrMsg )
{
	if ( m_pThread == NULL )
	{
		return	errFailed ;
	}
	return	m_pThread->ThrowException( pszErrMsg ) ;
}

#else

// フレーム駆動スレッド起動
//////////////////////////////////////////////////////////////////////////////
SError SThread::BeginFrameThread( SProcedure * pProc, size_t nInitStack )
{
	//
	// スレッド準備
	//
	if ( m_pProc != NULL )
	{
		return	errFailed ;
	}
	m_pProc = pProc ;
	//
	m_countPending = 0 ;
	m_flagThrow = false ;
	m_flagFinished = false ;
	m_signalMaster.Initialize( false ) ;
	m_signalFrame.Initialize( false ) ;
	//
	// スレッド起動
	//
	#if	defined(__PLATFORM_WINDOWS__)
		m_hSync = ::CreateThread
			( NULL, 0, &SThread::FrameThreadProc, this, 0, &m_dwThreadID ) ;
		if ( m_hSync == NULL )
		{
			return	errFailed ;
		}
	#else
		m_flagThread = true ;
		m_signalDone.Initialize( false ) ;
		if ( pthread_create( &m_idThread, NULL, &SThread::FrameThreadProc, this ) )
		{
			m_flagThread = false ;
			m_signalDone.Delete() ;
			return	errFailed ;
		}
	#endif
	//
	// スレッド起動同期
	//
	m_signalMaster.Wait( SSynchronism::Infinite ) ;
	m_signalMaster.ResetSignal() ;
	return	errContinue ;
}

// フレーム駆動
//////////////////////////////////////////////////////////////////////////////
SError SThread::ContinueFrameThread( void )
{
	if ( (m_pProc == NULL) | m_flagFinished )
	{
		return	errFailed ;
	}
	//
	// 待機カウンタ更新
	//
	if ( m_countPending > 0 )
	{
		if ( AtomicSub( &m_countPending, 1 ) > 0 )
		{
			return	errPending ;
		}
	}
	//
	// 実行再開
	//
	m_signalFrame.SetSignal() ;
	return	errContinue ;
}

// フレーム同期
//////////////////////////////////////////////////////////////////////////////
SError SThread::SyncFrameThread( void )
{
	if ( m_flagFinished )
	{
		return	errSuccess ;
	}
	while ( m_countPending == 0 )
	{
		if ( m_signalMaster.Wait( 10 ) == errSuccess )
		{
			m_signalMaster.ResetSignal() ;
		}
		if ( m_flagFinished )
		{
			return	errSuccess ;
		}
	}
	return	errPending ;
}

// 例外をスロー
//////////////////////////////////////////////////////////////////////////////
SError SThread::ThrowException( const wchar_t * pszErrMsg )
{
	if ( (m_pProc == NULL) | m_flagFinished )
	{
		return	errFailed ;
	}
	ESLAssert( m_countPending > 0 ) ;	// スレッドは停止中
	//
	// 例外を設定
	//
	m_strException = pszErrMsg ;
	m_flagThrow = true ;
	m_countPending = 0 ;
	m_signalFrame.SetSignal() ;
	//
	return	errSuccess ;
}

#endif

// 現在のスレッド取得
//////////////////////////////////////////////////////////////////////////////
SThread * SThread::GetCurrentThread( void )
{
	#if	defined(__COTOPHA__)
		SProcedure *	pProc = Thread::GetCurrentThreadProc() ;
		if ( pProc != NULL )
		{
			return	((SThreadProcecure*)pProc)->m_pThread ;
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		return	(SThread*) ::TlsGetValue( g_tlsThread ) ;

	#else
		return	(SThread*) pthread_getspecific( g_keyThread ) ;

	#endif

	return	NULL ;
}

SProcedure * SThread::GetCurrentThreadProc( void )
{
	#if	defined(__COTOPHA__)
		SProcedure *	pProc = Thread::GetCurrentThreadProc() ;
		if ( pProc != NULL )
		{
			SThread *	pThread = ((SThreadProcecure*)pProc)->m_pThread ;
			if ( pThread != NULL )
			{
				return	pThread->m_pProc ;
			}
		}

	#elif	defined(__PLATFORM_WINDOWS__)
		SThread *	pThread = (SThread*) ::TlsGetValue( g_tlsThread ) ;
		if ( pThread != NULL )
		{
			return	pThread->m_pProc ;
		}

	#else
		SThread *	pThread = (SThread*) pthread_getspecific( g_keyThread ) ;
		if ( pThread != NULL )
		{
			return	pThread->m_pProc ;
		}

	#endif

	return	NULL ;
}

// 現在のスレッド判定
//////////////////////////////////////////////////////////////////////////////
bool SThread::IsCurrentThread( void )
{
	#if	defined(__COTOPHA__)
		return	(Thread::GetCurrentThread() == m_pThread) ;

	#elif	defined(__PLATFORM_WINDOWS__)
		return	(::GetCurrentThreadId() == m_dwThreadID) ;

	#else
		return	(pthread_self() == m_idThread) ;

	#endif
}

// スレッド識別子
//////////////////////////////////////////////////////////////////////////////
SThread::IdType SThread::GetCurrentId( void )
{
#if	defined(__COTOPHA__)
	return	(IdType) Thread::GetCurrentThread() ;

#elif	defined(__PLATFORM_WINDOWS__)
	return	::GetCurrentThreadId() ;

#else
	return	gettid() ;
#endif
}


#if	!defined(__COTOPHA__)

// スレッド関連付け
//////////////////////////////////////////////////////////////////////////////
void SThread::AttachCurrentThread( void )
{
	#if	defined(__PLATFORM_WINDOWS__)
		::TlsSetValue( g_tlsThread, this ) ;
		::TlsSetValue( g_tlsStorage, NULL ) ;
		::CoInitialize( NULL ) ;

	#else
		pthread_setspecific( g_keyThread, this ) ;
		pthread_setspecific( g_keyStorage, NULL ) ;

		#if	defined(__PLATFORM_ANDROID__)
		if ( JNI::g_JavaVM->AttachCurrentThread( &m_pJNIEnv, NULL ) != JNI_OK )
		{
			Trace( "Failed to g_JavaVM->AttachCurrentThread" ) ;
		}
		#endif

	#endif
}

// スレッド関連付け解除
//////////////////////////////////////////////////////////////////////////////
void SThread::DetachCurrentThread( void )
{
#if	!defined(__PLATFORM_WINDOWS__)
	m_signalDone.SetSignal() ;
#endif

	ReleaseLocalStorage() ;
}

#endif


// スレッド関数実行
//////////////////////////////////////////////////////////////////////////////
SThread * SThread::BeginStockThread
		( SThread::THREAD_PROCEDURE pfnProc, void * pInstance )
{
	QuickLock() ;
	StockThreadProcedure *	pStock = m_pStockThread ;
	if ( pStock == NULL )
	{
		Trace( "new StockThread\n" ) ;
		pStock = new StockThreadProcedure ;
		SError	err = pStock->BeginThread() ;
		if ( err )
		{
			QuickUnlock() ;
			return	NULL ;
		}
		AtomicAdd( &m_countRunningStockThread, 1 ) ;
	}
	else
	{
		m_pStockThread = pStock->m_pNext ;
		m_countStockedThread -- ;
		AtomicAdd( &m_countRunningStockThread, 1 ) ;
	}
	pStock->m_procStart = pfnProc ;
	pStock->m_ptrInstance = pInstance ;
	pStock->m_evStart.SetSignal() ;
	QuickUnlock() ;
	//
	if ( pfnProc == NULL )
	{
		pStock->m_pThread->Wait(10000) ;
		delete	pStock ;
		return	NULL ;
	}
	return	pStock->m_pThread ;
}

// ストックされている全スレッドを終了させる
//////////////////////////////////////////////////////////////////////////////
void SThread::ExitAllStockedThread( void )
{
	while ( m_pStockThread != NULL )
	{
		QuickLock() ;
		StockThreadProcedure *	pStock = m_pStockThread ;
		if ( pStock != NULL )
		{
			m_pStockThread = pStock->m_pNext ;
			pStock->m_procStart = NULL ;
			pStock->m_evStart.SetSignal() ;
			m_countStockedThread -- ;
		}
		QuickUnlock() ;
		//
		if ( pStock != NULL )
		{
			pStock->m_pThread->Wait(10000) ;
			delete	pStock ;
		}
	}
}

// スレッド切り替え性能テスト（評価値は１回のスレッド同期切り替えのミリ秒）
//////////////////////////////////////////////////////////////////////////////
double SThread::TestSwitchingPerformance( void )
{
	SSignalEvent	signalDone ;
	SSignalEvent	signalReady ;
	SSignalEvent	signalEvent ;
	signalDone.Initialize( false ) ;
	signalReady.Initialize( false ) ;
	signalEvent.Initialize( false ) ;
	//
	TEST_SWITCHING_PARAM	tsp ;
	tsp.nThreads = (atomic_int_t) GetLogicalProcessorCount() ;
	tsp.nReady = 0 ;
	tsp.nProcessed = 0 ;
	tsp.fQuitEvent = false ;
	tsp.pSignalDone = &signalDone ;
	tsp.pSignalReady = &signalReady ;
	tsp.pSignalEvent = &signalEvent ;
	//
	SPointerArray<SThread>	aThreads ;
	for ( int i = 0; i < tsp.nThreads; i ++ )
	{
		aThreads.Add
			( BeginStockThread
				( &SThread::TestSwitchingPerformanceProc, &tsp ) ) ;
	}
	signalReady.Wait() ;
	tsp.nReady = 0 ;
	signalReady.ResetSignal() ;
	//
	STimeCounter	timer ;
	size_t			nLoopCount = 0 ;
	for ( ; ; )
	{
		tsp.nProcessed = 0 ;
		tsp.fQuitEvent = ((++ nLoopCount >= 10000) || (timer.GetTime() >= 50)) ;
		signalEvent.SetSignal() ;
		//
		signalDone.Wait() ;				// 全スレッド signalEvent 通過
		signalDone.ResetSignal() ;
		signalEvent.ResetSignal() ;
		if ( tsp.fQuitEvent )
		{
			break ;
		}

		signalReady.SetSignal() ;
		signalDone.Wait() ;				// 全スレッド signalReady 通過
		signalDone.ResetSignal() ;
		//
		tsp.nReady = 0 ;
		signalReady.ResetSignal() ;
	}
	//
	double	msecTime = timer.GetRealTime() ;
	s_msecSwitchingPerformance = msecTime / nLoopCount ;
	Trace( "switching thread %d times, %f [ms].\n", nLoopCount, msecTime ) ;
	Trace( "%f [ms] per thread switching\n", s_msecSwitchingPerformance ) ;
	return	s_msecSwitchingPerformance ;
}

double	SThread::s_msecSwitchingPerformance = 0.0 ;

void SThread::TestSwitchingPerformanceProc( void * pInstance )
{
	TEST_SWITCHING_PARAM *	ptsp = (TEST_SWITCHING_PARAM*) pInstance ;
	if ( AtomicAdd( &(ptsp->nReady), 1 ) == ptsp->nThreads )
	{
		ptsp->pSignalReady->SetSignal() ;
	}
	for ( ; ; )
	{
		ptsp->pSignalEvent->Wait() ;
		if ( ptsp->fQuitEvent )
		{
			break ;
		}
		if ( AtomicAdd( &(ptsp->nProcessed), 1 ) == ptsp->nThreads )
		{
			ptsp->pSignalDone->SetSignal() ;
		}
		ptsp->pSignalReady->Wait() ;
		if ( AtomicAdd( &(ptsp->nReady), 1 ) == ptsp->nThreads )
		{
			ptsp->pSignalDone->SetSignal() ;
		}
	}
	if ( AtomicAdd( &(ptsp->nProcessed), 1 ) == ptsp->nThreads )
	{
		ptsp->pSignalDone->SetSignal() ;
	}
}


// ストックスレッド
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( SThread::StockThreadProcedure *
							SThread::m_pStockThread = NULL ) ;
ESL_DLL_DECL( atomic_int_t	SThread::m_countStockedThread = 0 ) ;
ESL_DLL_DECL( atomic_int_t	SThread::m_countRunningStockThread = 0 ) ;

// 構築関数
SThread::StockThreadProcedure::StockThreadProcedure( void )
{
	m_pNext = NULL ;
	m_pThread = new SThread ;
	m_evStart.Initialize( false ) ;
	m_procStart = NULL ;
	m_ptrInstance = NULL ;
}

// 消滅関数
SThread::StockThreadProcedure::~StockThreadProcedure( void )
{
	m_evStart.Delete() ;
	delete	m_pThread ;
	m_pThread = NULL ;
}

// スレッド開始
SError SThread::StockThreadProcedure::BeginThread( void )
{
	m_evStart.ResetSignal() ;
	return	m_pThread->BeginThread( this ) ;
}

// スレッド関数
void SThread::StockThreadProcedure::Run( void )
{
	const int	maxStockedThread = (int) GetLogicalProcessorCount() * 2 + 8 ;
	while ( m_evStart.Wait() == errSuccess )
	{
		if ( m_procStart == NULL )
		{
			break ;
		}
		m_evStart.ResetSignal() ;
		//
		m_procStart( m_ptrInstance ) ;
		//
		ESLVerify( AtomicSub( &m_countRunningStockThread, 1 ) >= 0 ) ;
		m_procStart = NULL ;
		m_ptrInstance = NULL ;
		//
		QuickLock() ;
		if ( SThread::m_countStockedThread >= maxStockedThread )
		{
			QuickUnlock() ;
			if ( SThread::BeginStockThread
				( &DelayDeleteThread, this ) != NULL )
			{
				break ;
			}
			QuickLock() ;
		}
		m_pNext = SThread::m_pStockThread ;
		SThread::m_pStockThread = this ;
		SThread::m_countStockedThread ++ ;
		QuickUnlock() ;
	}
}

// 完了後の処理
void SThread::StockThreadProcedure::Finalize( void )
{
}

// 遅延削除関数
void SThread::StockThreadProcedure::DelayDeleteThread( void * pInstance )
{
	StockThreadProcedure *
		pStock = (StockThreadProcedure*) pInstance ;
	pStock->m_pThread->Wait(10000) ;
	delete	pStock ;
	Trace( "delete StockThread\n" ) ;
}


// スレッドローカルストレージ
//////////////////////////////////////////////////////////////////////////////

// 関連付けアイテム取得
//////////////////////////////////////////////////////////////////////////////
ESLObject * SThread::GetLocalStorageAs( const wchar_t * pwszID )
{
#if		defined(__COTOPHA__)
	return	Thread::GetLocalStorageAs( pwszID ) ;

#else
	SStrSortArray<ESLObject*>*	pStorage = NULL ;
	#if	defined(__PLATFORM_WINDOWS__)
		pStorage = (SStrSortArray<ESLObject*>*)
							::TlsGetValue( g_tlsStorage ) ;
	#else
		pStorage = (SStrSortArray<ESLObject*>*)
							pthread_getspecific( g_keyStorage ) ;
	#endif
	if ( pStorage != NULL )
	{
		ESLObject**	ppObj = pStorage->GetAs( pwszID ) ;
		if ( ppObj != NULL )
		{
			return	*ppObj ;
		}
	}
	return	NULL ;
#endif
}

// 関連付けアイテム設定
//////////////////////////////////////////////////////////////////////////////
ESLObject *
	SThread::SetLocalStorageAs( const wchar_t * pwszID, ESLObject * pObj )
{
#if		defined(__COTOPHA__)
	return	Thread::SetLocalStorageAs( pwszID, pObj ) ;

#else
	SStrSortArray<ESLObject*>*	pStorage = NULL ;
	#if	defined(__PLATFORM_WINDOWS__)
		pStorage = (SStrSortArray<ESLObject*>*)
							::TlsGetValue( g_tlsStorage ) ;
	#else
		pStorage = (SStrSortArray<ESLObject*>*)
							pthread_getspecific( g_keyStorage ) ;
	#endif
	if ( pStorage == NULL )
	{
		pStorage = new SStrSortArray<ESLObject*> ;
		#if	defined(__PLATFORM_WINDOWS__)
			::TlsSetValue( g_tlsStorage, pStorage ) ;
		#else
			pthread_setspecific( g_keyStorage, pStorage ) ;
		#endif
	}
	ESLObject**	ppObj = pStorage->GetAs( pwszID ) ;
	if ( ppObj != NULL )
	{
		ESLObject *	pLast = *ppObj ;
		*ppObj = pObj ;
		return	pLast ;
	}
	pStorage->SetAs( pwszID, pObj ) ;
	return	NULL ;
#endif
}

// スレッドローカルストレージ解放
//////////////////////////////////////////////////////////////////////////////
void SThread::ReleaseLocalStorage( void )
{
#if	!defined(__COTOPHA__)
	SStrSortArray<ESLObject*>*	pStorage = nullptr ;

	#if	defined(__PLATFORM_WINDOWS__)
		pStorage = (SStrSortArray<ESLObject*>*)
							::TlsGetValue( g_tlsStorage ) ;
		::TlsSetValue( g_tlsStorage, nullptr ) ;

	#else
		#if	defined(__PLATFORM_ANDROID__)
//		if ( JNI::g_JavaVM->DetachCurrentThread() != JNI_OK )
//		{
//			Trace( "Failed to g_JavaVM->DetachCurrentThread" ) ;
//		}
		#endif

		pStorage = (SStrSortArray<ESLObject*>*)
							pthread_getspecific( g_keyStorage ) ;
		pthread_setspecific( g_keyStorage, nullptr ) ;
	#endif

	delete	pStorage ;
#endif
}




//////////////////////////////////////////////////////////////////////////////
// 並列処理用スレッド関数
//////////////////////////////////////////////////////////////////////////////

atomic_int_t	SParallelProcedure::m_countReadyThread = 0 ;
unsigned int	SParallelProcedure::m_countProcessor = 0 ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SSystem::SParallelProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SParallelProcedure::SParallelProcedure( void )
{
//	m_atomSpinLock = 0 ;
	m_countRunning = 0 ;
	m_signalFinished.Initialize( false ) ;
}

// 指定数のスレッドで並列処理を実行する
//////////////////////////////////////////////////////////////////////////////
void SParallelProcedure::Start( void** pInstance, size_t countThread )
{
	QuickLock() ;
	if ( m_countProcessor == 0 )
	{
		m_countProcessor = GetLogicalProcessorCount() ;
		if ( m_countProcessor > 1 )
		{
			m_countReadyThread = (atomic_int_t) m_countProcessor - 1 ;
		}
	}
	QuickUnlock() ;
	//
	if ( countThread > 32 )
	{
		countThread = 32 ;
	}
	THREAD_PARAM	tp[32] ;
	size_t			iThread = 1 ;
	m_signalFinished.SetSignal() ;
	for ( ; ; )
	{
		//
		// 並列スレッドを起動する
		//
		while ( iThread < countThread )
		{
			if ( AtomicSub( &m_countReadyThread, 1 ) >= 0 )
			{
				tp[iThread].pThis = this ;
				tp[iThread].pInstance = pInstance[iThread] ;
				//
				SpinLock() ;
				AtomicAdd( &m_countRunning, 1 ) ;
				m_signalFinished.ResetSignal() ;
				SpinUnlock() ;
				//
				if ( SThread::BeginStockThread
					( &SParallelProcedure::ParallelThreadProc, &(tp[iThread]) ) != NULL )
				{
					iThread ++ ;
				}
				else
				{
					AtomicAdd( &m_countReadyThread, 1 ) ;
					if ( AtomicSub( &m_countRunning, 1 ) == 0 )
					{
						m_signalFinished.SetSignal() ;
					}
					break ;
				}
			}
			else
			{
				AtomicAdd( &m_countReadyThread, 1 ) ;
				break ;
			}
		}
		//
		// 主スレッドで実行
		//
		SpinLock() ;
		if ( Continue( pInstance[0] ) )
		{
			SpinUnlock() ;
			RunParallel( pInstance[0] ) ;
		}
		else
		{
			SpinUnlock() ;
			break ;
		}
	}
	//
	// 全並列スレッドが完了するのを待つ
	//
	/*
	STimeCounter	timer ;
	while ( m_countRunning != 0 )
	{
		if ( timer.GetRealTime() >= 0.001 )
		{
			break ;
		}
	}
	*/
	m_signalFinished.Wait() ;
	SpinLock() ;
	m_signalFinished.ResetSignal() ;
	SpinUnlock() ;
}

// スピンロック
//////////////////////////////////////////////////////////////////////////////
void SParallelProcedure::SpinLock( void )
{
	m_csFastLock.Lock() ;
//	while ( AtomicXchg( &m_atomSpinLock, 1 ) != 0 )
//	{
//	}
}

void SParallelProcedure::SpinUnlock( void )
{
//	ESLVerify( AtomicXchg( &m_atomSpinLock, 0 ) == 1 ) ;
	m_csFastLock.Unlock() ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SParallelProcedure::ParallelThreadProc( void * pInstance )
{
	THREAD_PARAM	tp = *((THREAD_PARAM*)pInstance) ;
	for ( ; ; )
	{
		tp.pThis->SpinLock() ;
		if ( tp.pThis->Continue( tp.pInstance ) )
		{
			tp.pThis->SpinUnlock() ;
			tp.pThis->RunParallel( tp.pInstance ) ;
		}
		else
		{
			tp.pThis->SpinUnlock() ;
			break ;
		}
	}
	tp.pThis->SpinLock() ;
	if ( AtomicSub( &(tp.pThis->m_countRunning), 1 ) == 0 )
	{
		tp.pThis->m_signalFinished.SetSignal() ;
	}
	tp.pThis->SpinUnlock() ;
	AtomicAdd( &m_countReadyThread, 1 ) ;
}




//////////////////////////////////////////////////////////////////////////////
// 実行キュー Dispatcher
//////////////////////////////////////////////////////////////////////////////

SProcedureQueue::Dispatcher::Dispatcher( SProcedureQueue& que )
	: m_queue( que ), m_flagQuit( false ), m_pcRunning( nullptr )
{
}

void SProcedureQueue::Dispatcher::Invoke( ProcContainer& pc )
{
	if ( pc.m_pProc != nullptr )
	{
		m_queue.DetachFinallyProcedur( pc.m_pProc ) ;
		//
		pc.m_pProc->Prepare() ;
		//
		m_queue.Lock() ;
		m_pcRunning = &pc ;
		if ( m_flagQuit )
		{
			pc.m_pProc->RequestQuit( m_reqQuit ) ;
		}
		m_queue.Unlock() ;
		//
		pc.m_pProc->Run() ;
		//
		m_queue.Lock() ;
		m_pcRunning = nullptr ;
		m_queue.Unlock() ;
		//
		pc.m_pProc->Finalize() ;
	}
	if ( pc.m_pDone != nullptr )
	{
		pc.m_pDone->SetSignal() ;
	}
	if ( pc.m_flagAutoDelete )
	{
		delete	pc.m_pProc ;
		pc.m_pProc = nullptr ;
	}
}

void SProcedureQueue::Dispatcher::RequestQuit( SProcedure::RequestQuitLevel rql )
{
	m_queue.Lock() ;
	if ( m_pcRunning != nullptr )
	{
		ESLAssert( m_pcRunning->m_pProc != nullptr ) ;
		if ( m_pcRunning->m_pProc != nullptr )
		{
			m_pcRunning->m_pProc->RequestQuit( rql ) ;
		}
	}
	m_reqQuit = rql ;
	m_flagQuit = true ;
	m_queue.Unlock() ;
}



//////////////////////////////////////////////////////////////////////////////
// 実行キュー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SSystem::SProcedureQueue )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SProcedureQueue::SProcedureQueue( void )
	: m_nPendingProc( 0 ), m_flagQuit( false ), m_flagFence( false )
{
	m_signalDispatch.Initialize( false ) ;
	m_signalProc.Initialize( false ) ;
	m_signalEmpty.Initialize( true ) ;
	m_signalAllDone.Initialize( true ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SProcedureQueue::~SProcedureQueue( void )
{
	RequestQuit( SProcedure::quitAbort ) ;
	ESLVerify( WaitAllRunLoops( 1000 ) == errSuccess ) ;
	m_csLock.Lock() ;
	m_csLock.Unlock() ;
}

// 初期化（再使用）
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::Reset( void )
{
	RequestQuit( SProcedure::quitAbort ) ;
	WaitAllRunLoops() ;
	//
	m_csLock.Lock() ;
	SLinkedListEntry<ProcContainer> *	pListProc = m_listProc.GetFirst() ;
	while ( pListProc != nullptr )
	{
		if ( pListProc->m_flagAutoDelete )
		{
			delete	pListProc->m_pProc ;
			pListProc->m_pProc = nullptr ;
		}
		pListProc = pListProc->GetNext() ;
	}
	m_listProc.DeleteAllEntries() ;
	m_signalDispatch.ResetSignal() ;
	m_signalProc.ResetSignal() ;
	m_signalEmpty.SetSignal() ;
	m_signalAllDone.SetSignal() ;
	m_nPendingProc = 0 ;
	m_flagQuit = false ;
	m_flagFence = false ;
	m_csLock.Unlock() ;
}

// キューに追加
//////////////////////////////////////////////////////////////////////////////
SProcedureQueue::ProcIdentity
	SProcedureQueue::AddProcedure
		( SProcedure * pProc, SSignalEvent * pDoneSignal,
						bool flagAutoDelete, bool flagFence )
{
	SLinkedListEntry<ProcContainer> *	pListProc =
		new SLinkedListEntry<ProcContainer>
				( ProcContainer( pProc, pDoneSignal, flagAutoDelete, flagFence ) ) ;
	//
	m_csLock.Lock() ;
	if ( m_listProc.GetFirst() == nullptr )
	{
		m_signalEmpty.ResetSignal() ;
	}
	m_listProc.InsertLastEntry( pListProc ) ;
	//
	CheckPendingSignal() ;
	m_csLock.Unlock() ;
	//
	return	pListProc ;
}

// 未実行ならキューから削除
//////////////////////////////////////////////////////////////////////////////
bool SProcedureQueue::CancelProcedure( SProcedureQueue::ProcIdentity procId )
{
	bool	flagCanceled = false ;
	m_csLock.Lock() ;
	SLinkedListEntry<ProcContainer> *	pListProc = m_listProc.GetLast() ;
	while ( pListProc != nullptr )
	{
		if ( ((ProcIdentity) pListProc) == procId )
		{
			m_listProc.DeleteEntry( pListProc ) ;
			flagCanceled = true ;
			break ;
		}
		pListProc = pListProc->GetPrev() ;
	}
	m_csLock.Unlock() ;
	return	flagCanceled ;
}

// キューの実行順序をフェンスする
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::SetFence( void )
{
	m_csLock.Lock() ;
	SLinkedListEntry<ProcContainer> *	pListProc = m_listProc.GetLast() ;
	if ( pListProc != nullptr )
	{
		pListProc->m_flagFence = true ;
	}
	m_csLock.Unlock() ;
}

// キューから取り出す
//////////////////////////////////////////////////////////////////////////////
SProcedureQueue::ProcContainer SProcedureQueue::GetProcedure( int64_t msecTimeout )
{
	SProcedureQueue::ProcContainer	pc = PeekProcedure() ;
	while ( !m_flagQuit && pc.IsEmpty() )
	{
		SError	err = m_signalProc.Wait( msecTimeout ) ;
		pc = PeekProcedure() ;
		if ( err == errTimeout )
		{
			break ;
		}
	}
	return	pc ;
}

SProcedureQueue::ProcContainer SProcedureQueue::PeekProcedure( void )
{
	ProcContainer	pc ;
	m_csLock.Lock() ;
	SLinkedListEntry<ProcContainer> *	pListProc = m_listProc.GetFirst() ;
	if ( !m_flagFence && (pListProc != nullptr) )
	{
		pc = *pListProc ;
		m_listProc.DeleteEntry( pListProc ) ;
		//
		m_flagFence = pc.m_flagFence ;
		//
		if ( !pc.IsEmpty() )
		{
			m_nPendingProc ++ ;
		}
		if ( !m_flagQuit && (m_flagFence || (m_listProc.GetFirst() == nullptr)) )
		{
			m_signalProc.ResetSignal() ;
		}
	}
	m_csLock.Unlock() ;
	return	pc ;
}

// キュー同期
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::Lock( void ) const
{
	m_csLock.Lock() ;
}

void SProcedureQueue::Unlock( void ) const
{
	m_csLock.Unlock() ;
}

// キューを実行
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::Run( void )
{
	Dispatcher *	pDispatcher = GetDispatcher() ;
	while ( !m_flagQuit )
	{
		ProcContainer	pc = GetProcedure() ;
		if ( !pc.IsEmpty() )
		{
			pDispatcher->Invoke( pc ) ;
			//
			m_csLock.Lock() ;
			if ( -- m_nPendingProc <= 0 )
			{
				CheckEmptySignal() ;
			}
			m_csLock.Unlock() ;
		}
		if ( pc.m_flagFence )
		{
			m_csLock.Lock() ;
			ESLAssert( m_flagFence ) ;
			m_flagFence = false ;
			CheckPendingSignal() ;
			m_csLock.Unlock() ;
		}
	}
	//
	m_csLock.Lock() ;
	while ( m_aFinallyProcs.GetLength() > 0 )
	{
		SProcedure *	pProc = m_aFinallyProcs.GetAt(0) ;
		m_aFinallyProcs.RemoveAt(0) ;
		m_csLock.Unlock() ;
		if ( pProc != nullptr )
		{
			pProc->Prepare() ;
			pProc->Run() ;
			pProc->Finalize() ;
		}
		m_csLock.Lock() ;
	}
	m_csLock.Unlock() ;
	//
	ReleaseDispatcher( pDispatcher ) ;
}

void SProcedureQueue::RunAll( void )
{
	Run() ;
	Flush() ;
}

// 別スレッドでキューを実行
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::AsyncRun( void )
{
	m_csLock.Lock() ;
	m_signalDispatch.ResetSignal() ;
	m_csLock.Unlock() ;
	SThread::BeginStockThread( &SProcedureQueue::AsyncRunProc, this ) ;
	m_signalDispatch.Wait() ;
}

// キューを実行（キューが空になるまで）
//////////////////////////////////////////////////////////////////////////////
size_t SProcedureQueue::Flush( void )
{
	Dispatcher *	pDispatcher = GetDispatcher() ;
	size_t			nProcCount = 0 ;
	ProcContainer	pc = PeekProcedure() ;
	while ( !pc.IsEmpty() )
	{
		pDispatcher->Invoke( pc ) ;
		nProcCount ++ ;
		//
		pc = PeekProcedure() ;
	}
	ReleaseDispatcher( pDispatcher ) ;
	return	nProcCount ;
}

// キューが空か？
//////////////////////////////////////////////////////////////////////////////
bool SProcedureQueue::IsEmpty( void ) const
{
	bool	flagEmpty = false ;
	m_csLock.Lock() ;
	flagEmpty = (m_listProc.GetFirst() == nullptr) ;
	m_csLock.Unlock() ;
	return	flagEmpty ;
}

// キューが空になるのを待つ
//////////////////////////////////////////////////////////////////////////////
SError SProcedureQueue::WaitUntilEmpty( int64_t msecTimeout )
{
	return	m_signalEmpty.Wait( msecTimeout ) ;
}

// 全ての実行ループが完了するのを待つ
//////////////////////////////////////////////////////////////////////////////
SError SProcedureQueue::WaitAllRunLoops( int64_t msecTimeout )
{
	return	m_signalAllDone.Wait( msecTimeout ) ;
}

// Dispatcher 取得
//////////////////////////////////////////////////////////////////////////////
SProcedureQueue::Dispatcher * SProcedureQueue::GetDispatcher( void )
{
	Dispatcher *	pDispatcher = new Dispatcher( *this ) ;
	m_csLock.Lock() ;
	m_dispatchers.Add( pDispatcher ) ;
	m_signalEmpty.ResetSignal() ;
	m_signalDispatch.SetSignal() ;
	m_signalAllDone.ResetSignal() ;
	m_csLock.Unlock() ;
	return	pDispatcher ;
}

// Dispatcher 解放
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::ReleaseDispatcher( Dispatcher * pDispatcher )
{
	m_csLock.Lock() ;
	ssize_t	iDisp = m_dispatchers.FindPtr( pDispatcher ) ;
	ESLAssert( iDisp >= 0 ) ;
	if ( iDisp >= 0 )
	{
		m_dispatchers.RemoveAt( (size_t) iDisp ) ;
		//
		CheckAllDoneSignal() ;
	}
	m_csLock.Unlock() ;
}

// 終了フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::RequestQuit( SProcedure::RequestQuitLevel rql )
{
	m_csLock.Lock() ;
	for ( size_t i = 0; i < m_dispatchers.GetLength(); i ++ )
	{
		Dispatcher *	pDispatcher = m_dispatchers.GetAt( i ) ;
		ESLAssert( pDispatcher != nullptr ) ;
		if ( pDispatcher != nullptr )
		{
			pDispatcher->RequestQuit( rql ) ;
		}
	}
	m_flagQuit = true ;
	m_reqQuitLevel = rql ;
	m_signalProc.SetSignal() ;
	m_csLock.Unlock() ;
}

// 終了フラグ判定
//////////////////////////////////////////////////////////////////////////////
bool SProcedureQueue::GetQuitFlag( void ) const
{
	return	m_flagQuit ;
}

// Run（AsyncRun）完了時に Flush を呼び出さなくても必ず実行すべき SProcedure
//////////////////////////////////////////////////////////////////////////////
SError SProcedureQueue::AttachFinallyProcedur( SProcedure * pProc )
{
	SError	err = errFailed ;
	if ( pProc != nullptr )
	{
		m_csLock.Lock() ;
		if ( m_aFinallyProcs.FindPtr( pProc ) < 0 )
		{
			m_aFinallyProcs.Add( pProc ) ;
			err = errSuccess ;
		}
		m_csLock.Unlock() ;
	}
	return	err ;
}

SError SProcedureQueue::DetachFinallyProcedur( SProcedure * pProc )
{
	SError	err = errFailed ;
	if ( pProc != nullptr )
	{
		m_csLock.Lock() ;
		ssize_t	i = m_aFinallyProcs.FindPtr( pProc ) ;
		if ( i >= 0 )
		{
			m_aFinallyProcs.RemoveAt( (size_t) i ) ;
			err = errSuccess ;
		}
		m_csLock.Unlock() ;
	}
	return	err ;
}

// 別スレッドでキューを実行する関数
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::AsyncRunProc( void * pInstance )
{
	reinterpret_cast<SProcedureQueue*>(pInstance)->Run() ;
}

// 実行キューシグナル更新
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::CheckPendingSignal( void )
{
	if ( !m_flagFence && (m_listProc.GetFirst() != nullptr) )
	{
		m_signalProc.SetSignal() ;
	}
}

// 実行キュー空シグナル更新
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::CheckEmptySignal( void )
{
	if ( (m_nPendingProc <= 0)
		&& (m_flagQuit || (m_listProc.GetFirst() == nullptr)) )
	{
		m_signalEmpty.SetSignal() ;
	}
}

// 実行ループ全完了シグナル更新
//////////////////////////////////////////////////////////////////////////////
void SProcedureQueue::CheckAllDoneSignal( void )
{
	if ( m_dispatchers.GetLength() == 0 )
	{
		m_signalAllDone.SetSignal() ;
	}
}

