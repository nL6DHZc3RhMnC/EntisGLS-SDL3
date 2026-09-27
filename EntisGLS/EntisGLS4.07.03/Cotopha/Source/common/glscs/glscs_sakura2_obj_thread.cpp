
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// スレッド・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::ThreadObject, BufferObject, ContextShell )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ThreadObject::ThreadObject( void )
{
	m_thread = NULL ;
	m_addrProc = 0 ;
	m_statusThread = statusNoExecution ;
	m_countPostArgStack = 0 ;
	m_fAbort = false ;
	m_signalSuspending.Initialize( false ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ThreadObject::~ThreadObject( void )
{
	AbortThread() ;
	m_signalSuspending.Delete() ;
}

// 実行中のスレッドを強制的に終了させる
//////////////////////////////////////////////////////////////////////////////
void ThreadObject::AbortThread( void )
{
	if ( m_thread != NULL )
	{
		Trace( "aborting thread #%08X...\n", m_dwHighAddr ) ;
		do
		{
			m_fAbort = true ;
			m_status = xsHalt ;
			AtomicOr( &m_maskException, interruptChangeStatus ) ;
		}
		while ( m_thread->Wait( 1000 ) != errSuccess ) ;
		Trace( "aborted thread #%08X\n", m_dwHighAddr ) ;
		//
		delete	m_thread ;
		m_thread = NULL ;
		m_fAbort = false ;
	}
	//
	// スレッドデタッチ通知
	//
	if ( m_pSakura2VM != NULL )
	{
		m_pSakura2VM->OnThreadDetached( this ) ;
		m_pSakura2VM = NULL ;
		m_pThread = NULL ;
	}
}

// スレッドは強制終了中か？
//////////////////////////////////////////////////////////////////////////////
bool ThreadObject::IsThreadAborting( void ) const
{
	return	m_fAbort ;
}

// スレッドをサスペンド状態にする（以前の実行ステータスを返す）
//////////////////////////////////////////////////////////////////////////////
Context::ExecutionStatus ThreadObject::SuspendThread( void )
{
	SSystem::QuickLock() ;
	if ( (m_statusThread != statusNoExecution)
		&& (m_status == xsExecution)
		&& (m_thread != NULL)
		&& (SThread::GetCurrentThread() != m_thread) )
	{
		ExecutionStatus	status = ChangeExecutionStatus( xsSuspend ) ;
		SSystem::QuickUnlock() ;
		if ( m_signalSuspending.Wait( 10000 ) != errSuccess )
		{
			Trace( "failed to suspend Sakura2VM thread.\n" ) ;
		}
		return	status ;
	}
	SSystem::QuickUnlock() ;
	return	m_status ;
}

// コンテキスト初期化
//////////////////////////////////////////////////////////////////////////////
void ThreadObject::InitializeContext( VirtualMachine* pVM )
{
	if ( m_dwHighAddr == 0 )
	{
		//
		// アドレス割り当て
		//
		pVM->AllocateHeapObjectAddress( this, mallocModeThread ) ;
		ESLAssert( m_dwHighAddr != 0 ) ;
	}
	//
	// コンテキスト初期化
	//
	InitializeProcessor
		( (((INT64)m_dwHighAddr) << 32) | Sakura2StackLimit ) ;
	m_pSakura2VM = pVM ;
	m_pThread = this ;
	//
	// スレッドアタッチ通知
	//
	pVM->OnThreadAttached( this ) ;
}

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::CallFunction
		( INT64 addrFunc, const Register *pArg, int nArgCount )
{
	//
	// 関数実行開始
	//
	const wchar_t *	pwszErr = BeginFunction( addrFunc, pArg, nArgCount ) ;
	if ( pwszErr != NULL )
	{
		return	pwszErr ;
	}
	if ( m_status == xsHalt )
	{
		return	NULL ;
	}
	//
	// 一時停止した場合などには、関数が完了するまでループ
	//
	do
	{
		pwszErr = ExecuteShell() ;
		if ( pwszErr != NULL )
		{
			// 例外エラー
			m_pSakura2VM->HandleExceptionError( this, pwszErr ) ;
			return	pwszErr ;
		}
	}
	while ( m_status != xsHalt ) ;
	//
	// 関数終了後は引数の分だけスタックを解放
	//
	FreeStack( nArgCount ) ;

	return	NULL ;
}

// 仮想関数呼び出し
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::CallVirtualFunction
		( INT64 addrObj, int iVirtual, const Register *pArg, int nArgCount )
{
	INT64 *	pObj = (INT64*) AtomicTranslateAddress( addrObj, sizeof(INT64) ) ;
	if ( pObj != NULL )
	{
		INT64 *	pVector =
			(INT64*) AtomicTranslateAddress
				( *pObj + iVirtual * sizeof(INT64), sizeof(INT64) ) ;
		if ( pVector != NULL )
		{
			return	CallFunction( *pVector, pArg, nArgCount ) ;
		}
		else
		{
			SSystem::Trace
				( "invalid index %08X:%08X to call virtual #%d\n",
					(DWORD) (addrObj >> 32), (DWORD) addrObj, iVirtual ) ;
		}
	}
	else
	{
		SSystem::Trace
			( "invalid object %08X:%08X to call virtual #%d\n",
				(DWORD) (addrObj >> 32), (DWORD) addrObj, iVirtual ) ;
	}
	return	NULL ;
}

// フレーム駆動スレッド実行継続処理
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::ContinueFrameThread( void )
{
	//
	// 一時停止した場合などには、関数が完了するまでループ
	//
	while ( (m_status != xsHalt) && (m_status != xsPending) )
	{
		const wchar_t *	pwszErr = ExecuteShell() ;
		if ( pwszErr != NULL )
		{
			// 例外エラー
			m_pSakura2VM->HandleExceptionError( this, pwszErr ) ;
			m_addrProc = 0 ;
			return	pwszErr ;
		}
	}
	if ( m_status == xsHalt )
	{
		//
		// 関数完了処理
		//
		return	FinalizeFrameThread() ;
	}
	return	NULL ;
}

// フレーム駆動スレッド実行完了処理
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::FinalizeFrameThread( void )
{
	if ( m_countPostArgStack > 0 )
	{
		FreeStack( (int) m_countPostArgStack ) ;
	}
	//
	Register	regAddrProc ;
	regAddrProc.i = m_addrProc ;
	//
	const wchar_t * pwszErr =
		CallVirtualFunction
			( m_addrProc, procVectorFinalize, &regAddrProc, 1 ) ;
	m_addrProc = 0 ;
	return	pwszErr ;
}

// スレッド開始（スクリプトインターフェース）
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::syscallBeginThread
		( ECSSakura2Processor::Context * context, INT64 addrProc )
{
	AbortThread() ;
	AssertLock() ;
	InitializeContext( context->m_pSakura2VM ) ;
	AssertUnlock() ;
	//
	m_thread = new SThread ;
	m_addrProc = addrProc ;
	m_statusThread = statusNoExecution ;
	//
	context->m_regset[regAcc].i = m_thread->BeginThread( this ) ;
	//
	return	NULL ;
}

// フレーム駆動スレッド開始（スクリプトインターフェース）
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::syscallBeginFrameThread
	( ECSSakura2Processor::Context * context,
					INT64 addrProc, DWORD dwInitStack )
{
	//
	// スレッド初期化
	//
	dwInitStack = (dwInitStack + 0x0F) & ~0x0F ;
	AbortThread() ;
	AssertLock() ;
	if ( BufferObject::GetLength() < dwInitStack )
	{
		BufferObject::ResizeBuffer
			( dwInitStack, Sakura2StackLimit - dwInitStack ) ;
	}
	InitializeContext( context->m_pSakura2VM ) ;
	AssertUnlock() ;
	//
	m_addrProc = addrProc ;
	m_statusThread = statusNoExecution ;
	//
	// 準備関数を呼び出し
	//
	Register	regAddrProc ;
	regAddrProc.i = m_addrProc ;
	context->m_regset[regAcc].i = SSystem::errPending ;
	//
	const wchar_t *	pwszErr =
		CallVirtualFunction
			( m_addrProc, procVectorPrepare, &regAddrProc, 1 ) ;
	m_countPostArgStack = 1 ;
	if ( pwszErr != NULL )
	{
		m_addrProc = 0 ;
		return	pwszErr ;
	}
	if ( m_status == xsHalt )
	{
		m_countPostArgStack = 0 ;
	}
	//
	// メイン関数開始
	//
	pwszErr = BeginVirtualFunction
			( m_addrProc, procVectorRun, &regAddrProc, 1 ) ;
	if ( pwszErr != NULL )
	{
		m_pSakura2VM->HandleExceptionError( this, pwszErr ) ;
		m_addrProc = 0 ;
		return	pwszErr ;
	}
	if ( m_status == xsPending )
	{
		context->m_regset[regAcc].i = SSystem::errPending ;
		return	NULL ;
	}
	//
	// 一時停止した場合などには、関数が完了するまでループ
	//
	pwszErr = ContinueFrameThread() ;
	if ( pwszErr != NULL )
	{
		return	pwszErr ;
	}
	context->m_regset[regAcc].i = SSystem::errSuccess ;
	if ( m_status == xsPending )
	{
		context->m_regset[regAcc].i = SSystem::errPending ;
	}
	return	NULL ;
}

// フレーム駆動スレッド継続（スクリプトインターフェース）
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::syscallContinueFrameThread
		( ECSSakura2Processor::Context * context )
{
	if ( m_addrProc == 0 )
	{
		return	NULL ;
	}
	context->m_regset[regAcc].i = SSystem::errPending ;
	const wchar_t *	pwszErr = ExecuteShell() ;
	if ( pwszErr != NULL )
	{
		// 例外エラー
		m_pSakura2VM->HandleExceptionError( this, pwszErr ) ;
		m_addrProc = 0 ;
		return	pwszErr ;
	}
	if ( m_status == xsPending )
	{
		context->m_regset[regAcc].i = SSystem::errPending ;
		return	NULL ;
	}
	pwszErr = ContinueFrameThread() ;
	if ( pwszErr != NULL )
	{
		return	pwszErr ;
	}
	context->m_regset[regAcc].i = SSystem::errSuccess ;
	if ( m_status == xsPending )
	{
		context->m_regset[regAcc].i = SSystem::errPending ;
	}
	return	NULL ;
}

// フレーム駆動スレッドへ例外スロー（スクリプトインターフェース）
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::syscallThrowException
		( ECSSakura2Processor::Context * context, INT64 addrErr )
{
	const uint16_t *	pwszErr =
			(const uint16_t *)
				AtomicTranslateAddress( addrErr, sizeof(uint16_t) ) ;
	SString	strErr = pwszErr ;
	return	ThrowException( strErr ) ;
}

// 実行中か？
//////////////////////////////////////////////////////////////////////////////
bool ThreadObject::IsRunning( void ) const
{
	return	(m_statusThread != statusNoExecution)
				&& (m_thread != NULL) && m_thread->IsRunning() ;
}

// 待機
//////////////////////////////////////////////////////////////////////////////
SError ThreadObject::Wait( int64_t msecTimeout )
{
	if ( m_thread != NULL )
	{
		return	m_thread->Wait( msecTimeout ) ;
	}
	return	errFailed ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ThreadObject::GetTypeName( void ) const
{
	return	L"SSystem::Thread" ;
}

// 保存準備処理
//////////////////////////////////////////////////////////////////////////////
SError ThreadObject::PrepareSave
	( VirtualMachine * vm, Context * context )
{
	m_statusPrevSave = SuspendThread() ;
	//
	return	BufferObject::PrepareSave( vm, context ) ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError ThreadObject::SaveStatic
	( SFileInterface * file, VirtualMachine * vm, Context * context )
{
	SError	err ;
	err = BufferObject::SaveStatic( file, vm, context ) ;
	if ( err )
	{
		ChangeExecutionStatus( m_statusPrevSave ) ;
		return	err ;
	}
	//
	SAVE_STATUS	svst ;
	svst.lowIP				= m_ip ;
	svst.highIP				= m_ipSegment ;
	svst.maskException		= m_maskException ;
	svst.idSystemCall		= m_idSystemCall ;
	svst.idInteruption		= m_idInteruption ;
	svst.dwContextStatus	= m_statusPrevSave ;
	//
	svst.addrProc			= m_addrProc ;
	svst.dwThreadStatus		= m_statusThread ;
	//
	if ( file->Write
		( &m_regset[0], sizeof(Register) * 0x100 ) < sizeof(Register) * 0x100 )
	{
		ChangeExecutionStatus( m_statusPrevSave ) ;
		return	errFailed ;
	}
	if ( file->Write( &svst, sizeof(SAVE_STATUS) ) < sizeof(SAVE_STATUS) )
	{
		ChangeExecutionStatus( m_statusPrevSave ) ;
		return	errFailed ;
	}
	//
	uint32_t	countStorage = (uint32_t) m_ssaLocalStorage.GetLength() ;
	if ( file->Write( &countStorage, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		ChangeExecutionStatus( m_statusPrevSave ) ;
		return	errFailed ;
	}
	for ( size_t i = 0; i < countStorage; i ++ )
	{
		const SString *	pstrID = m_ssaLocalStorage.GetTagAt( i ) ;
		int64_t *		pValue = m_ssaLocalStorage.GetAt( i ) ;
		if ( pstrID != NULL )
		{
			file->WriteString( *pstrID ) ;
		}
		else
		{
			SString	strNull ;
			file->WriteString( strNull ) ;
		}
		if ( pValue != NULL )
		{
			file->Write( pValue, sizeof(int64_t) ) ;
		}
		else
		{
			int64_t	valueNull = 0 ;
			file->Write( &valueNull, sizeof(int64_t) ) ;
		}
	}
	//
	ChangeExecutionStatus( m_statusPrevSave ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError ThreadObject::LoadStatic
	( SFileInterface * file, VirtualMachine * vm, Context * context )
{
	AbortThread() ;
	InitializeProcessor
		( (((INT64)m_dwHighAddr) << 32) | Sakura2StackLimit ) ;
	//
	SError	err ;
	err = BufferObject::LoadStatic( file, vm, context ) ;
	if ( err )
	{
		return	err ;
	}
	if ( file->Read
		( &m_regset[0], sizeof(Register) * 0x100 ) < sizeof(Register) * 0x100 )
	{
		return	errFailed ;
	}
	SAVE_STATUS	svst ;
	if ( file->Read( &svst, sizeof(SAVE_STATUS) ) < sizeof(SAVE_STATUS) )
	{
		return	errFailed ;
	}
	m_ip			= svst.lowIP ;
	m_ipSegment		= svst.highIP ;
	m_maskException	= svst.maskException & ~interruptAssertLock ;
	m_idSystemCall	= svst.idSystemCall ;
	m_idInteruption	= svst.idInteruption ;
	m_status		= (ExecutionStatus) svst.dwContextStatus ;
	//
	m_addrProc		= svst.addrProc ;
	m_statusThread	= (ThreadExecutionStatus) svst.dwThreadStatus ;
	//
	uint32_t	countStorage ;
	if ( file->Read( &countStorage, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	errFailed ;
	}
	m_ssaLocalStorage.RemoveAll() ;
	for ( size_t i = 0; i < countStorage; i ++ )
	{
		SString	strID ;
		int64_t	nValue ;
		file->ReadString( strID ) ;
		file->Read( &nValue, sizeof(int64_t) ) ;
		m_ssaLocalStorage.SetAs( strID, nValue ) ;
	}
	//
	return	errSuccess ;
}

// 復元後の後のスクリプト処理
//////////////////////////////////////////////////////////////////////////////
SError ThreadObject::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	SError	err ;
	err = BufferObject::OnLoadedDynamic( vm, context ) ;
	if ( err )
	{
		return	err ;
	}
	m_pSakura2VM = vm ;
	m_pThread = this ;
	//
	if ( m_statusThread != statusNoExecution )
	{
		m_thread = new SThread ;
		m_thread->BeginThread( this ) ;
	}
	return	errSuccess ;
}

// 一時停止処理（ループの外側で処理する場合には errAbort を返却）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ThreadObject::OnSuspendContext( void )
{
	SError	err ;
	m_signalSuspending.SetSignal() ;
	err = ContextShell::OnSuspendContext() ;
	m_signalSuspending.ResetSignal() ;
	return	err ;
}

// スタック拡張例外処理
//////////////////////////////////////////////////////////////////////////////
DWORD ThreadObject::HandleExceptionExtendStack( DWORD maskException )
{
	AtomicAnd( &m_maskException, ~exceptionExtendStack ) ;
	maskException &= ~exceptionExtendStack ;
	//
	if ( m_regset[regSP].l32 <= Sakura2StackLimit )
	{
		AssertLock() ;
		m_pSakura2VM->Lock() ;
		//
		DWORD	sp = m_regset[regSP].l32 & ~0x0FFF ;
		DWORD	nSize = Sakura2StackLimit - sp ;
		ESLAssert( (SDWORD) nSize >= 0 ) ;
		//
		const DWORD	nDefStackSize = (DWORD) m_pSakura2VM->GetDefaultStackSize() ;
		ESLAssert( nDefStackSize > 0 ) ;
		DWORD	nOldSize = BufferObject::GetLength() ;
		DWORD	nNewSize = nOldSize << 1 ;
		if ( nNewSize < nDefStackSize )
		{
			nNewSize = nDefStackSize ;
		}
		while ( nNewSize < nSize )
		{
			nNewSize <<= 1 ;
		}
		DWORD	nExpandOffset = nNewSize - nOldSize ;
		BufferObject::ResizeBuffer( nNewSize, Sakura2StackLimit - nNewSize ) ;
		//
		BYTE *	pbytBuf = BufferObject::GetBuffer() ;
		eslMoveMemory( pbytBuf + nExpandOffset, pbytBuf, nOldSize ) ;
		//
		m_pSakura2VM->Unlock() ;
		AssertUnlock() ;
	}
	else
	{
		// スタック・オーバーフロー例外
		AtomicOr( &m_maskException, exceptionStackOverflow ) ;
		maskException |= exceptionStackOverflow ;
	}
	return	maskException ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void ThreadObject::Run( void )
{
	const wchar_t *	pwszErr ;
	if ( m_statusThread != statusNoExecution )
	{
		//
		// ロードされたスレッド（途中から実行を再開）
		//
		pwszErr = ExecuteShell() ;
		if ( pwszErr != NULL )
		{
			if ( m_statusThread == statusPrepare )
			{
				m_statusThread = statusNoExecution ;
				return ;
			}
		}
		if ( m_status == xsHalt )
		{
			m_regset[regSP].l32 += sizeof(Register) * 1 ;
		}
	}
	//
	// スレッド関数を順次実行
	//
	Register	regAddrProc ;
	regAddrProc.i = m_addrProc ;
	//
	switch ( m_statusThread )
	{
	case	statusNoExecution:
	default:
		m_statusThread = statusPrepare ;
		pwszErr = CallVirtualFunction
				( m_addrProc, procVectorPrepare, &regAddrProc, 1 ) ;
		if ( pwszErr != NULL )
		{
			m_statusThread = statusNoExecution ;
			return ;
		}
		if ( m_fAbort )
		{
			break ;
		}

	case	statusPrepare:
		m_statusThread = statusRun ;
		pwszErr = CallVirtualFunction
				( m_addrProc, procVectorRun, &regAddrProc, 1 ) ;
		if ( m_fAbort )
		{
			break ;
		}

	case	statusRun:
		m_statusThread = statusFinalize ;
		pwszErr = CallVirtualFunction
				( m_addrProc, procVectorFinalize, &regAddrProc, 1 ) ;

	case	statusFinalize:
		break ;
	}
	m_statusThread = statusNoExecution ;
}

// 128 bit アライメント new
//////////////////////////////////////////////////////////////////////////////
void * ThreadObject::operator new ( size_t nBytes )
{
	size_t	offsetContext =
		offsetof(ThreadObject,m_regset) - offsetof(Context,m_regset) ;
	//
	return	AllocateContext( nBytes, offsetContext ) ;
}

void ThreadObject::operator delete ( void * pObj )
{
	FreeContext( pObj ) ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Thread
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_Thread, context, cls_id )
{
	return	ECSSakura2::ThreadObject::NewContext() ;
}

// bool SSystem::Thread::IsRunning( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_IsRunning, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, ThreadObject, pThread, pArg, Thread::IsRunning ) ;
	pContext->m_regset[regAcc].i = pThread->IsRunning() ? -1 : 0 ;
	return	NULL ;
}

// SSystem::SError SSystem::Thread::Wait( int64_t msecTimeout = Infinite ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_Wait, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, ThreadObject, pThread, pArg, Thread::Wait ) ;
	pContext->m_regset[regAcc].i = pThread->Wait( pArg[1].i ) ;
	return	NULL ;
}

// SSystem::SError SSystem::Thread::BeginThread( SProcedure * pProc ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_BeginThread, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, ThreadObject, pThread, pArg, Thread::BeginThread ) ;
	return	pThread->syscallBeginThread( pContext, pArg[1].i ) ;
}

// SSystem::SError SSystem::Thread::BeginFrameThread
//						( SProcedure * pProc, size_t nInitStack ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_BeginFrameThread, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, ThreadObject, pThread, pArg, Thread::BeginFrameThread ) ;
	return	pThread->syscallBeginFrameThread
						( pContext, pArg[1].i, pArg[2].l32 ) ;
}

// SSystem::SError SSystem::Thread::ContinueFrameThread( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_ContinueFrameThread, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, ThreadObject, pThread, pArg, Thread::ContinueFrameThread ) ;
	return	pThread->syscallContinueFrameThread( pContext ) ;
}

// SSystem::SError SSystem::Thread::ThrowException( const wchar_t * pszErrMsg = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_ThrowException, pContext, pArg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( pContext, pVM, ThreadObject, pThread, pArg, Thread::ThrowException ) ;
	return	pThread->syscallThrowException( pContext, pArg[1].i ) ;
}

// static SSystem::Thread * SSystem::Thread::GetCurrentThread( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SSystem_Thread_GetCurrentThread, pContext, pArg )
{
	pContext->m_regset[regAcc].i = 0 ;
	//
	if ( pContext->m_pThread != NULL )
	{
		pContext->m_regset[regAcc].i =
				((INT64)pContext->m_pThread->m_dwHighAddr) << 32 ;
	}
	return	NULL ;
}

// static SProcedure::Thread * SSystem::Thread::GetCurrentThreadProc( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SSystem_Thread_GetCurrentThreadProc, pContext, pArg )
{
	pContext->m_regset[regAcc].i = 0 ;
	//
	ThreadObject *	pThread =
		ESLTypeCast<ThreadObject>( pContext->m_pThread ) ;
	if ( pThread != NULL )
	{
		pContext->m_regset[regAcc].i = pThread->GetThreadProcedureAddress() ;
	}
	return	NULL ;
}

// static ESLObject * GetLocalStorageAs( const wchar_t * pwszID ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_GetLocalStorageAs, pContext, pArg )
{
	pContext->m_regset[regAcc].i = 0 ;
	//
	ThreadObject *	pThread =
			ESLTypeCast<ThreadObject>( pContext->m_pThread ) ;
	if ( pThread != NULL )
	{
		uint16_t *	pszID =
			(uint16_t*) pContext->AtomicTranslateAddress
									( pArg[0].i, sizeof(uint16_t) ) ;
		SString		strID = pszID ;
		int64_t *	pValue = pThread->m_ssaLocalStorage.GetAs( strID ) ;
		if ( pValue != NULL )
		{
			pContext->m_regset[regAcc].i = *pValue ;
		}
	}
	return	NULL ;
}

// static ESLObject * SetLocalStorageAs( const wchar_t * pwszID, ESLObject * pObj ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Thread_SetLocalStorageAs, pContext, pArg )
{
	pContext->m_regset[regAcc].i = 0 ;
	//
	ThreadObject *	pThread =
			ESLTypeCast<ThreadObject>( pContext->m_pThread ) ;
	if ( pThread != NULL )
	{
		uint16_t *	pszID =
			(uint16_t*) pContext->AtomicTranslateAddress
									( pArg[0].i, sizeof(uint16_t) ) ;
		SString		strID = pszID ;
		int64_t *	pValue = pThread->m_ssaLocalStorage.GetAs( strID ) ;
		if ( pValue != NULL )
		{
			pContext->m_regset[regAcc].i = *pValue ;
			*pValue = pArg[1].i ;
		}
		else
		{
			pThread->m_ssaLocalStorage.SetAs( strID, pArg[11].i ) ;
		}
	}
	return	NULL ;
}

#endif
