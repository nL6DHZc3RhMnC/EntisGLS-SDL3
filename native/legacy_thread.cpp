
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
    Copyright (c) 2003-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include "legacy_thread.h"
#include "../tools/legacy_serialization.h"

IMPLEMENT_CLASS_INFO(LegacyThreadHost, SSystem::SThread)

LegacyThreadHost::LegacyThreadHost()
    : m_completion(NULL), m_procedure(this) {}

LegacyThreadHost::~LegacyThreadHost() { CloseThread(); }

ESLError LegacyThreadHost::BeginThread() {
    if (m_completion != NULL) return eslErrGeneral;
    m_completion = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_completion == NULL) return eslErrGeneral;
    if (SSystem::SThread::BeginThread(&m_procedure) != SSystem::errSuccess) {
        SSystem::SThread::Delete();
        ::CloseHandle(m_completion);
        m_completion = NULL;
        return eslErrGeneral;
    }
    return eslErrSuccess;
}

ESLError LegacyThreadHost::CloseThread() {
    if (IsCurrentThread()) return eslErrPending;
    SSystem::SThread::Delete();
    if (m_completion != NULL) {
        ::CloseHandle(m_completion);
        m_completion = NULL;
    }
    return eslErrSuccess;
}

void LegacyThreadHost::Procedure::Run() {
    // Signal both normal completion and stack-unwinding completion. SThread
    // subsequently publishes its own completion signal and supports joining.
    struct Completion {
        HANDLE event;
        ~Completion() { ::SetEvent(event); }
    } completed{m_owner->m_completion};
    m_owner->ThreadProc();
}


//////////////////////////////////////////////////////////////////////////////
// スレッドオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSThread, ECSObject, LegacyThreadHost )

ECSContext::ExecutionStatus ECSThread::ThreadContext::SetStatus(ExecutionStatus status) {
    std::lock_guard<std::recursive_mutex> guard(m_owner.m_controlMutex);
    if (status == xsExecution || status == xsSuspend) {
        if (m_owner.m_abortRequested) status = xsHalt;
        else if (m_owner.GetSuspendCount() != 0) status = xsSuspend;
    }
    return ECSContext::SetStatus(status);
}

ESLError ECSThread::ThreadContext::ExecuteInstruction() {
    {
        std::lock_guard<std::recursive_mutex> guard(m_owner.m_controlMutex);
        if (m_owner.m_abortRequested) {
            SetStatus(xsHalt);
            return eslErrSuccess;
        }
        if (m_owner.GetSuspendCount() != 0) {
            SetStatus(xsSuspend);
            return eslErrSuccess;
        }
    }
    // Do not hold the controller mutex over script/native calls: a native wait
    // must remain interruptible by AbortThread/SuspendThread on another thread.
    return ECSContext::ExecuteInstruction();
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSThread::ECSThread( void ) : m_context(*this)
{
	m_pPrimaryContext = NULL ;
	m_pPrevThread = NULL ;
	m_pNextThread = NULL ;
	m_dwSuspendCount = 0 ;
	m_fLoadedContext = false ;
}

ECSThread::ECSThread( ECSContext & context ) : m_context(*this)
{
	m_pPrimaryContext = NULL ;
	m_pPrevThread = NULL ;
	m_pNextThread = NULL ;
	m_dwSuspendCount = 0 ;
	m_fLoadedContext = false ;
	//
	Initialize( context ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSThread::~ECSThread( void )
{
	if ( IsThreadRunning() )
	{
		AbortThread( ) ;
	}
	if ( m_pPrimaryContext != NULL )
	{
		ECotophaScript::Lock( ) ;
		if ( m_pPrimaryContext != NULL )
		{
			m_pPrimaryContext->RemoveThreadList( this ) ;
		}
		ECotophaScript::Unlock( ) ;
	}
}

// 実行イメージ関連付け
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Initialize( ECSContext & context )
{
	m_pPrimaryContext = &context ;
	context.AddThreadList( this ) ;
	return	m_context.InitializeContext( context.m_pcsxi, false ) ;
}

// スレッド関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::BeginThread
	( DWORD dwFuncAddr, const ECSObjArray<ECSObject> & lstArg )
{
	if ( IsThreadRunning() )
	{
		return	eslErrGeneral ;
	}
	if ( LegacyThreadHost::Handle() != NULL )
	{
		LegacyThreadHost::CloseThread( ) ;
	}
	// ThreadProc releases its object/naked stack pages on completion. A new
	// invocation must recreate them, including after cooperative cancellation.
	if (m_context.m_pcsxi == NULL) return eslErrGeneral;
	ESLError initialize = m_context.InitializeContext(m_context.m_pcsxi, false);
	if (initialize) return initialize;
	{
		std::lock_guard<std::recursive_mutex> guard(m_controlMutex);
		m_abortRequested = false;
		__atomic_store_n(&m_dwSuspendCount, 0u, __ATOMIC_SEQ_CST);
		m_context.ResetAbortWatingEvent();
		::ResetEvent(m_context.m_hSuspended);
	}
	m_context.m_ip = dwFuncAddr ;
	m_context.m_stack.RemoveAll( ) ;
	m_context.MarkCallStackFlag( ) ;
	m_context.m_arg.m_varArray.RemoveAll( ) ;
	for ( int i = 0; i < (int) lstArg.GetSize(); i ++ )
	{
		ECSObject *	pObj = lstArg.GetAt( i ) ;
		if ( pObj != NULL )
		{
			if ( pObj->m_vtType == csvtReference )
			{
				ECSReference *	pRef = new ECSReference() ;
				pRef->SetReferenceCastInterface
					( ((ECSReference*)pObj)->m_pRef,
								NULL, *((ECSReference*)pObj) ) ;
				m_context.m_arg.m_varArray.Add( pRef ) ;
			}
			else
			{
				m_context.m_arg.m_varArray.Add( pObj->Duplicate() ) ;
			}
		}
	}
	return	LegacyThreadHost::BeginThread( ) ;
}

ESLError ECSThread::BeginThread
	( const wchar_t * pwszFuncName, const ECSObjArray<ECSObject> & lstArg )
{
	if ( m_context.m_pcsxi == NULL )
	{
		return	eslErrGeneral ;
	}
	DWORD *	pdwFuncAddr =
		m_context.m_pcsxi->GetFunctionAddress( pwszFuncName ) ;
	if ( pdwFuncAddr == NULL )
	{
		return	eslErrGeneral ;
	}
	return	BeginThread( *pdwFuncAddr, lstArg ) ;
}

ESLError ECSThread::BeginThread( ECSObject * pThreadProc )
{
	if ( m_context.m_pcsxi == NULL )
	{
		return	eslErrGeneral ;
	}
	ECS_CAST_INTERFACE	ci ;
	ESLError	err =
		pThreadProc->OperateCastInterface( ci, L"ThreadProcedure" ) ;
	if ( err || (ci.pCastObject == NULL) )
	{
		return	eslErrGeneral ;
	}
	ECS_FUNCTION_POINTER	fptr ;
	err = ci.pCastObject->GetFunctionPointer
				( m_context, fptr, ci.iFuncOffset + 0 ) ;
	if ( err )
	{
		return	err ;
	}
	if ( fptr.m_ftType != ECS_FUNCTION_POINTER::funcScriptCall )
	{
		return	eslErrGeneral ;
	}
	m_refThreadProc.SetReferenceCastInterface( ci.pCastObject, NULL, ci ) ;
	//
	ECSReference *	pRefThis =
			new ECSReference( fptr.m_castThis.pCastObject ) ;
	//
	pRefThis->m_iVarOffset = fptr.m_castThis.iVarOffset ;
	pRefThis->m_nVarBounds = fptr.m_castThis.nVarBounds ;
	pRefThis->m_iFuncOffset = fptr.m_castThis.iFuncOffset ;
	//
	ECSObjArray<ECSObject>	lstArg ;
	lstArg.SetAt( 0, pRefThis ) ;
	//
	return	BeginThread( fptr.m_varFunc.addrScript, lstArg ) ;
}

// 実行を強制終了させる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::AbortThread( DWORD dwTimeout )
{
	if ( LegacyThreadHost::Handle() == NULL )
	{
		return	eslErrSuccess ;
	}
	{
		std::lock_guard<std::recursive_mutex> guard(m_controlMutex);
		m_abortRequested = true;
		m_context.SetStatus(m_context.xsHalt);
		m_context.AbortWatingEvent();
	}
	if ( LegacyThreadHost::IsCurrentThread() )
	{
		return	eslErrPending ;
	}
	if ( ::WaitForSingleObject
		( LegacyThreadHost::Handle(), dwTimeout ) == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
	LegacyThreadHost::CloseThread( ) ;
	return	eslErrSuccess ;
}

// スレッドは実行中か？
//////////////////////////////////////////////////////////////////////////////
bool ECSThread::IsThreadRunning( void ) const
{
	return	(LegacyThreadHost::Handle() != NULL)
				&& (::WaitForSingleObject
						( LegacyThreadHost::Handle(), 0 ) == WAIT_TIMEOUT) ;
}

// スクリプトの実行を一時停止する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::SuspendThread( DWORD dwTimeout )
{
	DWORD dwSuspended;
	{
		std::lock_guard<std::recursive_mutex> guard(m_controlMutex);
		dwSuspended = __atomic_fetch_add(&m_dwSuspendCount, 1u, __ATOMIC_SEQ_CST);
		if (dwSuspended == 0) m_context.SetStatus(ECSContext::xsSuspend);
	}
	if ( dwSuspended == 0 )
	{
		ESLError	err ;
		if ( LegacyThreadHost::IsCurrentThread() )
		{
			return	eslErrPending ;
		}
		m_context.AbortWatingEvent( ) ;
		err = m_context.WaitForSuspended( dwTimeout ) ;
		if ( !err )
		{
			m_context.ResetAbortWatingEvent( ) ;
		}
		return	err ;
	}
	return	eslErrSuccess ;
}

// 一時停止中のスクリプトを再開する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::ResumeThread( void )
{
	std::lock_guard<std::recursive_mutex> guard(m_controlMutex);
	DWORD count = __atomic_load_n(&m_dwSuspendCount, __ATOMIC_SEQ_CST);
	while (count > 0)
	{
		if (!__atomic_compare_exchange_n(&m_dwSuspendCount, &count, count - 1,
			true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) continue;
		if (count == 1)
		{
			m_context.ResetAbortWatingEvent( ) ;
			m_context.SetStatus( ECSContext::xsExecution ) ;
			if ( LegacyThreadHost::IsCurrentThread() )
			{
				return	eslErrPending ;
			}
		}
		break;
	}
	return	eslErrSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD ECSThread::ThreadProc( void )
{
	// Android thread/JNI setup is performed by SSystem::SThread.
	ECotophaScript::SetCurrentThread( this ) ;
	//
	// The virtual SetStatus consults persistent controller requests under the
	// same mutex used to change them, so startup cannot overwrite a suspension
	// or cancellation that arrived immediately after BeginThread returned.
	ESLError err = m_context.ResumeExecution(ECSContext::xsExecution);
	if ( err )
	{
		EString	strErrMsg = GetESLErrorMsg(err) ;
		if ( m_refThreadProc.m_pRef != NULL && m_refThreadProc.m_pRef->IsValidObject() )
		{
			ECS_FUNCTION_POINTER	fptr ;
			ESLError	err =
				m_refThreadProc.GetFunctionPointer( m_context, fptr, 1 ) ;
			if ( !err && (fptr.m_ftType == ECS_FUNCTION_POINTER::funcScriptCall) )
			{
				ECSReference *	pRefThis =
						new ECSReference( fptr.m_castThis.pCastObject ) ;
				//
				pRefThis->m_iVarOffset = fptr.m_castThis.iVarOffset ;
				pRefThis->m_nVarBounds = fptr.m_castThis.nVarBounds ;
				pRefThis->m_iFuncOffset = fptr.m_castThis.iFuncOffset ;
				//
				ECSObjArray<ECSObject>	lstArg ;
				lstArg.SetAt( 0, pRefThis ) ;
				lstArg.SetAt
					( 1, new ECSString( EWideString( strErrMsg ) ) ) ;
				m_context.CallFunction( fptr.m_varFunc.addrScript, lstArg ) ;
			}
		}
		else if ( !m_wstrExceptionFunc.IsEmpty()
					&& (m_context.m_pcsxi != NULL) )
		{
			DWORD *	pdwFuncAddr =
				m_context.m_pcsxi->GetFunctionAddress( m_wstrExceptionFunc ) ;
			if ( pdwFuncAddr != NULL )
			{
				ECSObjArray<ECSObject>	lstParam ;
				lstParam.Add
					( new ECSString( EWideString( strErrMsg ) ) ) ;
				m_context.CallFunction( *pdwFuncAddr, lstParam ) ;
			}
		}
		::OutputDebugString
			( "スレッドで例外が発生しました；\n" + strErrMsg + "\n\n" ) ;
	}
	__atomic_store_n(&m_dwSuspendCount, 0u, __ATOMIC_SEQ_CST);
	m_context.ReleaseContext( false ) ;
	// No legacy Win32 window TLS is created by this Android runtime.

	return	0 ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSThread::GetTypeName( void ) const
{
	return	L"Thread" ;
}

ECSObject * ECSThread::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"Thread" ) )
	{
		return	this ;
	}
	return	ECSObject::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSThread::Duplicate( void )
{
	return	new ECSThread ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Move( ECSContext & context, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない Thread への代入です" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない Thread の単項演算子です" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない Thread の演算子です" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない Thread の比較です" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::GetVariableIndex( int & nIndex, int iMember )
{
	nIndex = iMember ;
	return	eslErrSuccess ;
}

ESLError ECSThread::GetVariableIndex( int & nIndex, const wchar_t * pwszMember )
{
	if ( !EWideString::Compare( L"stack", pwszMember ) )
	{
		nIndex = 0 ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "定義されていないメンバへの参照です" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSThread::GetVariableAt( int nIndex )
{
	if ( nIndex == 0 )
	{
		return	&m_context.m_stack ;
	}
	else if ( nIndex == 1 )
	{
		// IndexAllMember serializes argument references under this hidden index.
		return &m_context.m_arg;
	}
	else if ( nIndex == -1 )
	{
		return	&m_refThreadProc ;
	}
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSThread::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "定義されていない Thread のメンバ関数への参照です" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex < m_staFuncName->GetSize() )
	{
		return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
	}
	return	ESLErrorMsg
		( "定義されていない Thread のメンバ関数を呼び出しています" ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSThread::IndexAllMember( void )
{
	m_refThreadProc.IndexAllMember() ;
	m_refThreadProc.m_pParent = this ;
	m_refThreadProc.m_nIndex = -1 ;
	//
	m_context.m_stack.m_pParent = this ;
	m_context.m_stack.m_nIndex = 0 ;
	m_context.m_stack.IndexAllMember( ) ;
	//
	m_context.m_arg.m_pParent = this ;
	m_context.m_arg.m_nIndex = 1 ;
	m_context.m_arg.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSThread::CleanupAllReference( ECSContext & context )
{
	m_refThreadProc.CleanupAllReference( context ) ;
	//
	m_context.m_stack.CleanupAllReference( context ) ;
	m_context.m_arg.CleanupAllReference( context ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
#include "legacy_thread_state.inc"

// セーブ処理を開始する
//////////////////////////////////////////////////////////////////////////////
void ECSThread::OnBeginningSave( ECSContext & context )
{
	if ( IsThreadRunning() )
	{
		SuspendThread( ) ;
	}
}

// セーブ処理が全て完了した
//////////////////////////////////////////////////////////////////////////////
void ECSThread::OnFinishedSave( ECSContext & context )
{
	if ( IsThreadRunning() )
	{
		ResumeThread( ) ;
	}
}

// ロード処理を開始する
//////////////////////////////////////////////////////////////////////////////
void ECSThread::OnBeginningLoad( ECSContext & context )
{
	if ( IsThreadRunning() )
	{
		SuspendThread( ) ;
	}
}

// ロード処理が全て完了した
//////////////////////////////////////////////////////////////////////////////
void ECSThread::OnFinishedLoad( ECSContext & context )
{
	if ( m_fLoadedContext
		&& (m_sdRestore.nStatus != ECSContext::xsHalt) )
	{
		m_fLoadedContext = false ;
		//
		if ( !IsThreadRunning() )
		{
			LegacyThreadHost::CloseThread( ) ;
			LegacyThreadHost::BeginThread( ) ;
			ResumeThread( ) ;
		}
		else
		{
			ResumeThread( ) ;
		}
	}
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strBuf ;
	if ( IsThreadRunning() )
	{
		strBuf = "実行中" ;
		buf.Write( strBuf.CharPtr(), strBuf.GetLength() ) ;
		strBuf = EString("\t") * nIndent + "\tstack " ;
		buf.Write( strBuf.CharPtr(), strBuf.GetLength() ) ;
		return	m_context.m_stack.DumpObject( buf, nIndent + 2, context ) ;
	}
	else
	{
		strBuf = "停止" ;
		buf.Write( strBuf.CharPtr(), strBuf.GetLength() ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSThread::m_staFuncName = NULL ;
const wchar_t *	ECSThread::m_pwszFuncName[5] =
{
	L"BeginThread", L"IsThreadRunning",
	L"GetThreadResult", L"SetExceptionHandler",
	NULL
} ;
const ECSThread::PFUNC_CALL	ECSThread::m_pfnCallFunc[4] =
{
	&ECSThread::Call_BeginThread,
	&ECSThread::Call_IsThreadRunning,
	&ECSThread::Call_GetThreadResult,
	&ECSThread::Call_SetExceptionHandler,
} ;

// メンバ関数：Integer BeginThread( String sFuncName, ... )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Call_BeginThread
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	ECSObject *		pThreadProc = lstArg.GetAt( 1 ) ;
	ECSObject *		pTarget = ECSObject::GetEntity( pThreadProc ) ;
	ECSWideString	wstrFuncName ;
	if ( (pTarget == NULL)
		|| (pTarget->m_vtType != csvtObject) )
	{
		if ( lstArg.GetSize() < 2 )
		{
			return	ESLErrorMsg( "関数の引数の数が一致しません。" ) ;
		}
		err = context.GetArgumentAsStr( wstrFuncName, lstArg, 1, NULL ) ;
		if ( err )
		{
			return	err ;
		}
		pThreadProc = NULL ;
	}
	if ( m_pPrimaryContext != &context )
	{
		Initialize( context ) ;
	}
	if ( pThreadProc != NULL )
	{
		err = BeginThread( pThreadProc ) ;
	}
	else
	{
		ECSObjArray<ECSObject>	lstParam( lstArg, 2 ) ;
		err = BeginThread( wstrFuncName, lstParam ) ;
	}
	return	context.PushObject( context.new_CSInteger(err) ) ;
}

// メンバ関数 : Integer IsThreadRunning()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Call_IsThreadRunning
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( context.new_CSInteger( - (INT64) IsThreadRunning() ) ) ;
}

// メンバ関数 : Reference GetThreadResult( Integer nTimeout = INFINITE )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Call_GetThreadResult
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 1, INFINITE ) ;
	if ( err )
	{
		return	err ;
	}
	if ( IsThreadRunning() )
	{
		err = context.WaitUntilEvent( LegacyThreadHost::Handle(), nTimeout ) ;
		if ( err )
		{
			return	context.PushObject( context.new_CSReference( NULL ) ) ;
		}
	}
	ECSObject *	pResult = NULL ;
	if ( m_context.m_pRetObj != NULL )
	{
		if ( m_context.m_pRetObj->m_vtType == csvtReference )
		{
			pResult = context.new_CSReference
				( ((ECSReference*)m_context.m_pRetObj)->m_pRef ) ;
		}
		else
		{
			pResult = m_context.m_pRetObj->Duplicate() ;
		}
	}
	else
	{
		pResult = context.new_CSReference( NULL ) ;
	}
	return	context.PushObject( pResult ) ;
}

// メンバ関数 : SetExceptionHandler( String sFuncName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThread::Call_SetExceptionHandler
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrFuncName ;
	err = context.GetArgumentAsStr( wstrFuncName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	SetExceptionFunction( wstrFuncName ) ;
	return	context.PushObject( context.new_CSInteger() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// スレッド待機イベント
//////////////////////////////////////////////////////////////////////////////

// クラス除法
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSThreadEvent, ECSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSThreadEvent::ECSThreadEvent( void )
{
	m_hEvent = NULL ;
	m_nEvent = -1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSThreadEvent::~ECSThreadEvent( void )
{
	DeleteEvent( ) ;
}

// イベントオブジェクト作成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::CreateEvent( bool fInitState )
{
	if ( m_hEvent != NULL )
	{
		return	eslErrGeneral ;
	}
	m_hEvent = ::CreateEvent( NULL, TRUE, fInitState, NULL ) ;
	if ( m_hEvent == NULL )
	{
		return	eslErrGeneral ;
	}
	m_nEvent = fInitState ? 1 : 0 ;
	return	eslErrSuccess ;
}

// イベントオブジェクト削除
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::DeleteEvent( void )
{
	if ( m_hEvent != NULL )
	{
		::CloseHandle( m_hEvent ) ;
		m_hEvent = NULL ;
	}
	m_nEvent = -1 ;
	return	eslErrSuccess ;
}

// イベントセット
//////////////////////////////////////////////////////////////////////////////
void ECSThreadEvent::SetEvent( void )
{
	if ( m_hEvent != NULL )
	{
		::SetEvent( m_hEvent ) ;
		m_nEvent = 1 ;
	}
}

// イベントリセット
//////////////////////////////////////////////////////////////////////////////
void ECSThreadEvent::ResetEvent( void )
{
	if ( m_hEvent != NULL )
	{
		::ResetEvent( m_hEvent ) ;
		m_nEvent = 0 ;
	}
}

// イベント待機
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::WaitEvent( DWORD dwTimeout, ECSContext & context )
{
	return	context.WaitUntilEvent( m_hEvent, dwTimeout ) ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSThreadEvent::GetTypeName( void ) const
{
	return	L"ThreadEvent" ;
}

ECSObject * ECSThreadEvent::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"ThreadEvent" ) )
	{
		return	this ;
	}
	return	ECSObject::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSThreadEvent::Duplicate( void )
{
	return	new ECSThreadEvent ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( (pEntity != NULL)
		&& (pEntity->m_vtType == csvtInteger) )
	{
		if ( ((ECSInteger*)pEntity)->GetValue() )
		{
			SetEvent( ) ;
		}
		else
		{
			ResetEvent( ) ;
		}
	}
	else
	{
		return	ESLErrorMsg( "定義されていない ThreadEvent への代入です" ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない ThreadEvent の単項演算子です" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない ThreadEvent の演算子です" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない ThreadEvent の比較です" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "定義されていない ThreadEvent のメンバ関数への参照です" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex < m_staFuncName->GetSize() )
	{
		return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
	}
	return	ESLErrorMsg
		( "定義されていない ThreadEvent のメンバ関数を呼び出しています" ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Save( ESLFileObject & file, ECSContext & context )
{
	file.Write( &m_nEvent, sizeof(m_nEvent) ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Load( ESLFileObject & file, ECSContext & context )
{
	DeleteEvent( ) ;
	//
	file.Read( &m_nEvent, sizeof(m_nEvent) ) ;
	if ( m_nEvent >= 0 )
	{
		CreateEvent( m_nEvent != 0 ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strBuf ;
	if ( m_nEvent >= 0 )
	{
		strBuf = m_nEvent ? "true" : "false" ;
		buf.Write( strBuf.CharPtr(), strBuf.GetLength() ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSThreadEvent::m_staFuncName = NULL ;
const wchar_t *	ECSThreadEvent::m_pwszFuncName[7] =
{
	L"Create", L"Delete", L"Wait", L"Set", L"Reset", L"Value",
	NULL
} ;
const ECSThreadEvent::PFUNC_CALL	ECSThreadEvent::m_pfnCallFunc[6] =
{
	&ECSThreadEvent::Call_Create,
	&ECSThreadEvent::Call_Delete,
	&ECSThreadEvent::Call_Wait,
	&ECSThreadEvent::Call_Set,
	&ECSThreadEvent::Call_Reset,
	&ECSThreadEvent::Call_Value,
} ;

// メンバ関数 : Integer Create( Integer fInitState = 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Call_Create
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nInitState ;
	err = context.GetArgumentAsInt( nInitState, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = CreateEvent( nInitState != 0 ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer Delete()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Call_Delete
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	err = DeleteEvent( ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer Wait( Integer nTimeout := INFINITE  )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Call_Wait
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 1, INFINITE ) ;
	if ( err )
	{
		return	err ;
	}
	err = WaitEvent( (DWORD) nTimeout, context ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Set( Integer nValue := 1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Call_Set
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nValue ;
	err = context.GetArgumentAsInt( nValue, lstArg, 1, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	if ( nValue )
	{
		SetEvent( ) ;
	}
	else
	{
		ResetEvent( ) ;
	}
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Reset()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Call_Reset
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ResetEvent( ) ;
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : Integer Value()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadEvent::Call_Value
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject( context.new_CSInteger( m_nEvent ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// スレッド待機（ミューテックス）イベント
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSThreadMutex, ECSThreadEvent )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSThreadMutex::ECSThreadMutex( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSThreadMutex::~ECSThreadMutex( void )
{
}

// イベントオブジェクト作成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::CreateEvent( bool fInitState )
{
	ECotophaScript::Lock( ) ;
	ESLError	err = ECSThreadEvent::CreateEvent( !fInitState ) ;
	if ( !err )
	{
		m_nEvent = 0 ;
		if ( fInitState )
		{
			m_nEvent ++ ;
			m_refOwnerThread.SetReference
				( ECotophaScript::GetCurrentThread(),
					ECotophaScript::GetPrimaryContext() ) ;
		}
		else
		{
			m_refOwnerThread.SetReference
				( NULL, ECotophaScript::GetPrimaryContext() ) ;
		}
	}
	ECotophaScript::Unlock( ) ;
	return	err ;
}

// イベントオブジェクト削除
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::DeleteEvent( void )
{
	ECotophaScript::Lock( ) ;
	ECSThreadEvent::DeleteEvent() ;
	m_refOwnerThread.SetReference( NULL ) ;
	ECotophaScript::Unlock( ) ;
	return	eslErrSuccess ;
}

// イベントセット
//////////////////////////////////////////////////////////////////////////////
void ECSThreadMutex::SetEvent( void )
{
}

// イベントリセット
//////////////////////////////////////////////////////////////////////////////
void ECSThreadMutex::ResetEvent( void )
{
	if ( m_nEvent > 0 )
	{
		if ( m_refOwnerThread.m_pRef == ECotophaScript::GetCurrentThread() )
		{
			if ( (-- m_nEvent) == 0 )
			{
				m_refOwnerThread.SetReference
					( NULL, ECotophaScript::GetPrimaryContext() ) ;
				::SetEvent( m_hEvent ) ;
			}
		}
	}
}

// イベント待機
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::WaitEvent( DWORD dwTimeout, ECSContext & context )
{
	if ( m_refOwnerThread.m_pRef == ECotophaScript::GetCurrentThread() )
	{
		ESLAssert( m_nEvent > 0 ) ;
		m_nEvent ++ ;
	}
	else
	{
		DWORD	dwBeginTime = ::timeGetTime( ) ;
		for ( ; ; )
		{
			DWORD	dwWaitTime = dwTimeout ;
			if ( dwWaitTime != INFINITE )
			{
				DWORD	dwPastTime = ::timeGetTime() - dwBeginTime ;
				if ( dwPastTime < dwWaitTime )
				{
					dwWaitTime -= dwPastTime ;
				}
				else
				{
					dwWaitTime = 0 ;
				}
			}
			ESLError	err = context.WaitUntilEvent( m_hEvent, dwWaitTime ) ;
			if ( !err )
			{
				ECotophaScript::Lock( ) ;
				if ( m_nEvent == 0 )
				{
					ESLAssert( m_refOwnerThread.m_pRef == NULL ) ;
					::ResetEvent( m_hEvent ) ;
					m_nEvent ++ ;
					m_refOwnerThread.SetReference
						( ECotophaScript::GetCurrentThread(),
							ECotophaScript::GetPrimaryContext() ) ;
					ECotophaScript::Unlock( ) ;
					break ;
				}
				ECotophaScript::Unlock( ) ;
			}
			else if ( err == eslErrAbort )
			{
				return	eslErrAbort ;
			}
			if ( (dwTimeout != INFINITE)
				&& (::timeGetTime() - dwBeginTime >= dwTimeout) )
			{
				return	eslErrTimeout ;
			}
		}
	}
	return	eslErrSuccess ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSThreadMutex::GetTypeName( void ) const
{
	return	L"ThreadMutex" ;
}

ECSObject * ECSThreadMutex::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"ThreadMutex" ) )
	{
		return	this ;
	}
	return	ECSThreadEvent::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSThreadMutex::Duplicate( void )
{
	return	new ECSThreadMutex ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::Move( ECSContext & context, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない ThreadMutex への代入です" ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSThreadMutex::IndexAllMember( void )
{
	m_refOwnerThread.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSThreadMutex::CleanupAllReference( ECSContext & context )
{
	m_refOwnerThread.CleanupAllReference( context ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::CommitAllReference( ECSContext & context )
{
	return	m_refOwnerThread.CommitAllReference( context ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::Save( ESLFileObject & file, ECSContext & context )
{
	file.Write( &m_nEvent, sizeof(m_nEvent) ) ;
	return	m_refOwnerThread.Save( file, context ) ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::Load( ESLFileObject & file, ECSContext & context )
{
	DeleteEvent( ) ;
	//
	file.Read( &m_nEvent, sizeof(m_nEvent) ) ;
	if ( m_refOwnerThread.Load( file, context ) )
	{
		return	eslErrGeneral ;
	}
	if ( m_nEvent >= 0 )
	{
		SDWORD	nSaveCount = m_nEvent ;
		CreateEvent( m_nEvent == 0 ) ;
		m_nEvent = nSaveCount ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSThreadMutex::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strBuf ;
	if ( m_nEvent >= 0 )
	{
		strBuf = EString(m_nEvent) ;
		buf.Write( strBuf.CharPtr(), strBuf.GetLength() ) ;
	}
	return	eslErrSuccess ;
}
