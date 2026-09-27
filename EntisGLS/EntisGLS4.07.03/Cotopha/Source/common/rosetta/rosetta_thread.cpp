
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_smart_buffer.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_thread.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// Runnable クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRunnableClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRunnableClass::RSRunnableClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRunnableClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"run", NULL, L"", NULL, NULL, NULL ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Thread オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( Rosetta::RSThread, RSObject, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSThread::RSThread
	( RSVirtualMachine * pVM, RSObject * pRunnable, RSClass * pClass )
	: RSObject( pClass, typeOther )
{
	m_pChainPrev = NULL ;
	m_pChainNext = NULL ;
	//
	m_pVM = pVM ;
	pVM->AddRef() ;
	//
	m_pRunnable = pRunnable ;
	RSObject::AddRef( pRunnable ) ;
	//
	m_pContext = new RSContext( pVM, NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSThread::~RSThread( void )
{
	AbortThread() ;
	//
	m_pVM->ReleaseRef() ;
	m_pVM = NULL ;
	//
	RSObject::ReleaseRef( m_pRunnable ) ;
	m_pRunnable = NULL ;
	//
	delete	m_pContext ;
	m_pContext = NULL ;
}

// オブジェクト解放処理
//////////////////////////////////////////////////////////////////////////////
void RSThread::Finalize( RSContext& context )
{
	RSObject::Finalize( context ) ;
	//
	AbortThread() ;
	//
	QuickLock() ;
	delete	m_pContext ;
	m_pContext = NULL ;
	QuickUnlock() ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThread::CloneObject( RSContext& context ) const
{
	return	new RSThread( m_pVM, m_pRunnable, GetRSClass() ) ;
}

// Runnable オブジェクト関連付け
//////////////////////////////////////////////////////////////////////////////
void RSThread::AttachRunnable( RSObject * pRunnable )
{
	RSObject::AddRef( pRunnable ) ;
	if ( m_pContext != NULL )
	{
		m_pContext->ReleaseObjectRef( m_pRunnable ) ;
	}
	else
	{
		RSObject::ReleaseRef( m_pRunnable ) ;
	}
	m_pRunnable = pRunnable ;
}

// スレッド開始
//////////////////////////////////////////////////////////////////////////////
void RSThread::StartThread( void )
{
	m_thread.BeginThread( this ) ;
}

// スレッド強制終了
//////////////////////////////////////////////////////////////////////////////
bool RSThread::AbortThread( void )
{
	if ( (m_pContext != NULL) && m_thread.IsRunning() )
	{
		for ( int i = 0; i < 10000; i ++ )
		{
			QuickLock() ;
			if ( m_pContext != NULL )
			{
				m_pContext->SetAbort() ;
			}
			QuickUnlock() ;
			if ( m_thread.Wait( 1 ) == errSuccess )
			{
				break ;
			}
		}
	}
	return	true ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void RSThread::Run( void )
{
	if ( m_pContext != NULL )
	{
		if ( m_pVM != NULL )
		{
			m_pVM->AddRunningThread( this ) ;
		}
		m_pContext->ReleaseObjectRef
			( m_pContext->CallMethod( this, L"run", NULL, 0, false ) ) ;
		//
		SParserErrorTracer	perr ;
		m_pContext->OutputExceptionError( perr ) ;
	}
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void RSThread::Prepare( void )
{
	if ( m_pContext != NULL )
	{
		m_pContext->AttachThreadObject( this ) ;
	}
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void RSThread::Finalize( void )
{
	if ( m_pContext != NULL )
	{
		if ( m_pVM != NULL )
		{
			m_pVM->DetachRunningThread( this ) ;
		}
		m_pContext->ReleaseObjectRef( m_pRunnable ) ;
		m_pRunnable = NULL ;
		//
		QuickLock() ;
		delete	m_pContext ;
		m_pContext = NULL ;
		QuickUnlock() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// SSystem::SProcedure -> Runnable
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSThread::RunnableProcedure, SProcedure )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSThread::RunnableProcedure::RunnableProcedure
	( RSVirtualMachine * pVM,
		RSThread * pThread, RSObject * pRunnable, bool fAutoDelete )
{
	m_pVM = pVM ;
	m_pThread = pThread ;
	m_pRunnable = pRunnable ;
	m_fAutoDelete = fAutoDelete ;
	//
	pVM->AddRef() ;
	RSObject::AddRef( pThread ) ;
	RSObject::AddRef( pRunnable ) ;
}

// 実行関数
//////////////////////////////////////////////////////////////////////////////
void RSThread::RunnableProcedure::Run( void )
{
	RSContext	context( m_pVM, m_pThread ) ;
	context.ReleaseObjectRef
		( context.CallMethod( m_pRunnable, L"run", NULL, 0, false ) ) ;
	context.ReleaseObjectRef( m_pRunnable ) ;
	context.ReleaseObjectRef( m_pThread ) ;
	context.ReleaseObjectRef( m_pVM ) ;
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void RSThread::RunnableProcedure::Finalize( void )
{
	if ( m_fAutoDelete )
	{
		delete	this ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// Thread クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSThreadClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSThreadClass::RSThreadClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSThreadClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSThread( context.GetVM(), NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"Runnable target",
					NULL, &RSThreadClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"start", NULL, L"",
					NULL, &RSThreadClass::method_start, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"run", NULL, L"",
					NULL, &RSThreadClass::method_run, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"join", NULL, L"",
					NULL, &RSThreadClass::method_join, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"join", NULL, L"long millis",
					NULL, &RSThreadClass::method_join, NULL ) ;
	//
	AddFunctionDescriptiveAs
		( context, perr, L"currentThread", L"Thread", L"",
				NULL, &RSThreadClass::method_currentThread, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"sleep", NULL, L"long millis",
				NULL, &RSThreadClass::method_sleep, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"dumpStack", NULL, L"",
				NULL, &RSThreadClass::method_dumpStack, NULL ) ;
}

// void <init>( Runnable target )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThreadClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSThread *	pObj = ESLTypeCast<RSThread>( pThis ) ;
	RSObject *	pRunnable = arg.ObjectAt( 0 ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Thread.<init> の this が Thread ではありません" ) ;
		return	NULL ;
	}
	pObj->AttachRunnable( pRunnable ) ;
	return	NULL ;
}

// void start()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThreadClass::method_start
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSThread *	pObj = ESLTypeCast<RSThread>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Thread.start の this が Thread ではありません" ) ;
		return	NULL ;
	}
	pObj->StartThread() ;
	return	NULL ;
}

// void run()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThreadClass::method_run
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSThread *	pObj = ESLTypeCast<RSThread>( pThis ) ;
	if ( pObj != NULL )
	{
		RSObject *	pRunnable = pObj->m_pRunnable ;
		if ( pRunnable != NULL )
		{
			context.ReleaseObjectRef
				( context.CallMethod
					( pRunnable, L"run", NULL, 0, false ) ) ;
		}
	}
	return	NULL ;
}

// void join( long millis )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThreadClass::method_join
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSThread *	pObj = ESLTypeCast<RSThread>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Thread.join の this が Thread ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	nTimeout = arg.LongAt( 0 ) ;
	if ( nTimeout <= 0 )
	{
		nTimeout = SSystem::SSynchronism::Infinite ;
	}
	context.WaitSynchronism( pObj->m_thread, nTimeout ) ;
	return	NULL ;
}

// static Thread currentThread()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThreadClass::method_currentThread
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSObject *	pThread = context.GetThreadObject() ;
	RSObject::AddRef( pThread ) ;
	return	pThread ;
}

// static void sleep( long millis )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThreadClass::method_sleep
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	nMillis = arg.LongAt( 0 ) ;
	context.SleepMilliSec( nMillis ) ;
	return	NULL ;
}

// static void dumpStack()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSThreadClass::method_dumpStack
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SSmartBuffer	sbuf ;
	context.DebugDumpStack( sbuf, NULL ) ;
	sbuf.Seek( 0 ) ;
	//
	SBufferedFile	bfile( &sbuf, &sbuf, false ) ;
	for ( ; ; )
	{
		SString	strLine ;
		if ( bfile.ReadStringLine( strLine ) == 0 )
		{
			break ;
		}
		Trace( "%s", strLine.ToCharArray().GetConstArray() ) ;
	}
	Trace( "\n" ) ;
	//
	return	NULL ;
}

