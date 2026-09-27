
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行コンテキスト
//////////////////////////////////////////////////////////////////////////////

const ECSContext::PFUNC_EXECUTE_INSTRUCTION
			ECSContext::m_pfnExecute[csicMax] =
{
	&ECSContext::ExecuteNew,
	&ECSContext::ExecuteFree,
	&ECSContext::ExecuteLoad,
	&ECSContext::ExecuteStore,
	&ECSContext::ExecuteEnter,
	&ECSContext::ExecuteLeave,
	&ECSContext::ExecuteJump,
	&ECSContext::ExecuteCJump,
	&ECSContext::ExecuteCall,
	&ECSContext::ExecuteReturn,
	&ECSContext::ExecuteElement,
	&ECSContext::ExecuteElementIndirect,
	&ECSContext::ExecuteOperate,
	&ECSContext::ExecuteUniOperate,
	&ECSContext::ExecuteCompare,
	/* extended 2.0 */
	&ECSContext::ExecuteExOperate,
	&ECSContext::ExecuteExUniOperate,
	&ECSContext::ExecuteExCall,
	&ECSContext::ExecuteExReturn,
	&ECSContext::ExecuteCallMember,
	&ECSContext::ExecuteCallNativeMember,
	&ECSContext::ExecuteSwap,
	/* extended 2.3 */
	&ECSContext::ExecuteCreateBuffer,
	&ECSContext::ExecuteCreateBufferVSize,
	&ECSContext::ExecutePointerToObject,
	&ECSContext::ExecutePointerToAddress,
	&ECSContext::ExecuteReferenceForPointer,
	&ECSContext::ExecuteReferenceForObjPointer,
	&ECSContext::ExecuteCallFunctionPointer,
	&ECSContext::ExecuteCallNativeFunction,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSContext, ContextShell )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSContext::ECSContext( void )
{
	m_module = NULL ;
	m_pcsxi = NULL ;
	m_vaStack = 0 ;
	m_ip = 0 ;
	m_pRetObj = NULL ;
	m_status = xsHalt ;
	//
	m_bufNakedStack = NULL ;
	m_vaNakedStack = 0 ;
	//
	m_ppic = NULL ;
	//
	m_nLastTickTime = 0 ;
	//
	m_hAbortEvent = NULL ;
	m_hSuspended = NULL ;
	m_hStatusEvent = NULL ;
	m_hExecutionMutex = NULL ;
	m_nLockedCount = 0 ;
	//
	m_pThreadList = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSContext::~ECSContext( void )
{
	ReleaseContext( false ) ;
	//
	delete	m_pRetObj ;
	//
	if ( m_hAbortEvent != NULL )
	{
		::CloseHandle( m_hAbortEvent ) ;
	}
	if ( m_hSuspended != NULL )
	{
		::CloseHandle( m_hSuspended ) ;
	}
	if ( m_hStatusEvent != NULL )
	{
		::CloseHandle( m_hStatusEvent ) ;
	}
	if ( m_hExecutionMutex != NULL )
	{
		::CloseHandle( m_hExecutionMutex ) ;
	}
	if ( m_ppic != NULL )
	{
		::eslHeapFree( NULL, m_ppic, 0 ) ;
	}
	if ( m_pThreadList != NULL )
	{
		ECSThread *	pNextThread ;
		ECSThread *	pLastThread ;
		//
		ECotophaScript::Lock( ) ;
		pNextThread = m_pThreadList ;
		while ( pNextThread != NULL )
		{
			pLastThread = pNextThread ;
			pNextThread = pNextThread->m_pNextThread ;
			pLastThread->m_pPrimaryContext = NULL ;
			pLastThread->m_pPrevThread = NULL ;
			pLastThread->m_pNextThread = NULL ;
		}
		ECotophaScript::Unlock( ) ;
	}
}

// 実行コンテキスト初期化
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::InitializeContext
	( ECSExecutionImage * pcsxi, bool fInitializeGlobal )
{
	m_module = ::GetModuleHandle( NULL ) ;
	m_pcsxi = pcsxi ;
	m_stack.RemoveAll( ) ;
	m_arg.m_varArray.RemoveAll( ) ;
	m_ip = 0 ;
	m_status = xsHalt ;
	m_strErrMsg.FreeString( ) ;
	m_strThis.m_varStr = L"this" ;
	//
	m_tsbufReference.SetLimit( 128 ) ;
	m_tsbufInteger.SetLimit( 128 ) ;
	m_tsbufReal.SetLimit( 128 ) ;
	m_tsbufString.SetLimit( 128 ) ;
	m_tsbufPointer.SetLimit( 128 ) ;
	m_tsbufPointerRef.SetLimit( 128 ) ;
	//
	if ( m_hAbortEvent == NULL )
	{
		m_hAbortEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	}
	if ( m_hSuspended == NULL )
	{
		m_hSuspended = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	}
	if ( m_hStatusEvent == NULL )
	{
		m_hStatusEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL ) ;
	}
	if ( m_hExecutionMutex == NULL )
	{
		m_hExecutionMutex = ::CreateMutex( NULL, FALSE, NULL ) ;
	}
	if ( fInitializeGlobal )
	{
		m_dwLastBaseTime = ::timeGetTime( ) ;
		//
		if ( ECotophaScript::GetPrimaryContext() == NULL )
		{
			ECotophaScript::SetPrimaryContext( this ) ;
		}
		//
		// 実行イメージの初期化
		//
		ESLError	err ;
		err = m_pcsxi->InitializeExecution( *this ) ;
		if ( err )
		{
			return	err ;
		}
		m_vaStack =
			m_pcsxi->AllocateVirtualAddressDirectory
								( &(m_stack.m_varArray) ) ;
		//
		// Sakura2 プロセッサ初期化
		//
		InitializeSakuraProcessor() ;
		//
		// 初期化関数呼び出し
		//
		err = CallPrologueFunctions() ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		//
		// Sakura2 プロセッサ初期化
		//
		if ( m_vaStack == 0 )
		{
			m_vaStack =
				m_pcsxi->AllocateVirtualAddressDirectory
									( &(m_stack.m_varArray) ) ;
		}
		InitializeSakuraProcessor() ;
	}
	return	eslErrSuccess ;
}

// 初期化関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::CallPrologueFunctions( void )
{
	int	i, nCount = m_pcsxi->m_pifPrologue.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSObjArray<ECSObject>	lstArg ;
		DWORD		dwFuncAddr = m_pcsxi->m_pifPrologue.GetAt( i ) ;
		ESLError	err = CallFunction( dwFuncAddr, lstArg ) ;
		if ( err || (m_status != xsHalt) )
		{
			return	err ;
		}
	}
	nCount = m_pcsxi->m_pifNakedPrologue.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		DWORD		dwFuncAddr = m_pcsxi->m_pifNakedPrologue.GetAt( i ) ;
		ESLError	err = CallNakedFunction( dwFuncAddr, NULL, 0 ) ;
		if ( err || (m_status != xsHalt) )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// 実行イメージ上型フォーマット情報から静的なオブジェクトを生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::CreateInstanceFromTypeInfo
				( ECSObject *& pObj, const ECSObject * pType )
{
	pObj = NULL ;
	if ( pType == NULL )
	{
		return	ESLErrorMsg( "void なオブジェクトは生成できません。" ) ;
	}
	switch ( pType->m_vtType )
	{
	case	csvtObject:
		{
			const ECSStructure *
				pStruct = ESLTypeCast<ECSStructure,ECSObject>( pType ) ;
			if ( (pStruct != NULL)
				&& (pStruct->m_pClassInf != NULL)
				&& !(pStruct->m_pClassInf->GetAttribute()
								& ECSTypeInfo::flagNativeObject) )
			{
				pObj = CreateClassObject( *(pStruct->m_pClassInf) ) ;
				if ( pObj == NULL )
				{
					m_strErrMsg = EString( pType->GetTypeName() )
							+ " クラスオブジェクトの生成に失敗しました。" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
			else
			{
				DWORD	dwFuncAddr = (DWORD) -1 ;
				pObj = CreateObject
					( csvtObject, pType->GetTypeName(), &dwFuncAddr ) ;
				if ( pObj == NULL )
				{
					m_strErrMsg = EString( pType->GetTypeName() )
							+ " オブジェクトの生成に失敗しました。" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
				if ( dwFuncAddr != (DWORD) -1 )
				{
					ECSObjArray<ECSObject>	lstArg ;
					lstArg.Add( new ECSReference( pObj ) ) ;
					ESLError	err = CallFunction( dwFuncAddr, lstArg ) ;
					if ( err || (m_status != xsHalt) )
					{
						return	err ;
					}
				}
			}
		}
		break ;

	case	csvtReference:
		pObj = new ECSReference ;
		break ;

	case	csvtArray:
		{
			ECSArray *	pArray = new ECSArray ;
			ECSObject *	pElementType = ((ECSArray*)pType)->m_pDefObj ;
			if ( pElementType != NULL )
			{
				ECSObject *	pElement ;
				ESLError	err =
					CreateInstanceFromTypeInfo( pElement, pElementType ) ;
				if ( err )
				{
					delete	pArray ;
					return	err ;
				}
				pArray->SetDefaultElement( pElement ) ;
			}
			pArray->SetBounds( ((ECSArray*)pType)->GetBounds() ) ;
			pArray->CopyFrom( *((ECSArray*)pType) ) ;
			pObj = pArray ;
		}
		break ;

	case	csvtHash:
		{
			ECSHash *	pHash = new ECSHash ;
			ECSObject *	pElementType = ((ECSHash*)pType)->m_pDefObj ;
			if ( pElementType != NULL )
			{
				ECSObject *	pElement ;
				ESLError	err =
					CreateInstanceFromTypeInfo( pElement, pElementType ) ;
				if ( err )
				{
					delete	pHash ;
					return	err ;
				}
				pHash->SetDefaultElement( pElement ) ;
			}
			pObj = pHash ;
		}
		break ;

	case	csvtInteger:
		pObj = new ECSInteger
			( ((ECSInteger*)pType)->GetValue(),
				((ECSInteger*)pType)->GetValueMask() ) ;
		break;

	case	csvtReal:
		pObj = new ECSReal( ((ECSReal*)pType)->m_varReal ) ;
		break ;

	case	csvtString:
		pObj = new ECSString( ((ECSString*)pType)->m_varStr ) ;
		break ;

	case	csvtPointer:
		pObj = new ECSPointer ;
		break ;

	default:
		return	ESLErrorMsg( "不正な型情報です。" ) ;
	}
	return	eslErrSuccess ;
}

// 実行コンテキストのリソース解放
//////////////////////////////////////////////////////////////////////////////
void ECSContext::ReleaseContext( bool fReleaseGlobal )
{
	m_stack.RemoveAll( ) ;
	m_arg.m_varArray.RemoveAll( ) ;
	m_ip = 0 ;
	m_status = xsHalt ;
	m_strErrMsg.FreeString( ) ;
	//
	if ( fReleaseGlobal )
	{
		//
		// エピローグ関数の呼び出し
		//
		CallEpilogueFunctions() ;
	}
	//
	// スタック削除
	//
	if ( (m_vaStack != 0) && (m_pcsxi != NULL) )
	{
		m_pcsxi->FreeVirtualAddressDirectory
				( m_vaStack, &(m_stack.m_varArray) ) ;
		m_vaStack = 0 ;
	}
	if ( (m_vaNakedStack != 0) && (m_pcsxi != NULL) )
	{
		m_pcsxi->FreeHeapObjectAddress( m_vaNakedStack, this ) ;
		m_vaNakedStack = 0 ;
	}
	m_bufNakedStack = NULL ;
	//
	if ( fReleaseGlobal )
	{
		//
		// 実行リソースの消去
		//
		if ( m_pcsxi != NULL )
		{
			m_pcsxi->ReleaseExecution( *this ) ;
		}
		if ( m_hAbortEvent != NULL )
		{
			::CloseHandle( m_hAbortEvent ) ;
			m_hAbortEvent = NULL ;
		}
		if ( ECotophaScript::GetPrimaryContext() == this )
		{
			ECotophaScript::SetPrimaryContext( NULL ) ;
		}
	}
}

// エピローグ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::CallEpilogueFunctions( void )
{
	if ( m_pcsxi == NULL )
	{
		return	eslErrSuccess ;
	}
	ESLError	err ;
	int	i, nCount = m_pcsxi->m_pifEpilogue.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSObjArray<ECSObject>	lstArg ;
		DWORD	dwFuncAddr = m_pcsxi->m_pifEpilogue.GetAt( i ) ;
		err = CallFunction( dwFuncAddr, lstArg ) ;
		if ( err || (m_status != xsHalt) )
		{
			return	err ;
		}
	}
	nCount = m_pcsxi->m_pifNakedEpilogue.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		DWORD	dwFuncAddr = m_pcsxi->m_pifNakedEpilogue.GetAt( i ) ;
		err = CallNakedFunction( dwFuncAddr, NULL, 0 ) ;
		if ( err || (m_status != xsHalt) )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::CallFunction
	( DWORD dwFuncAddr, const ECSObjArray<ECSObject> & lstArg )
{
	m_ip = dwFuncAddr ;
	m_ipSegment = (ECSExecutionImage::roasCode << 24) ;
	//
	MarkCallStackFlag( ) ;
	m_arg.m_varArray.RemoveAll( ) ;
	//
	for ( int i = 0; i < (int) lstArg.GetSize(); i ++ )
	{
		ECSObject *	pObj = lstArg.GetAt( i ) ;
		if ( pObj != NULL )
		{
			if ( pObj->m_vtType == csvtReference )
			{
				ECSReference *	pRef = new_CSReference() ;
				pRef->SetReferenceCastInterface
					( ((ECSReference*)pObj)->m_pRef,
							this, *((ECSReference*)pObj) ) ;
				m_arg.m_varArray.Add( pRef ) ;
			}
			else if ( pObj->m_vtType == csvtPointerReference )
			{
				ECSPointerReference *	pPtrRef =
					new_CSPointerReference( *((ECSPointerReference*)pObj) ) ;
				m_arg.m_varArray.Add( pPtrRef ) ;
			}
			else
			{
				m_arg.m_varArray.Add( pObj->Duplicate() ) ;
			}
		}
	}
	return	ResumeExecution( ) ;
}

ESLError ECSContext::CallNakedFunction
	( DWORD dwFuncAddr, const INT64 * pArg, int nArgCount )
{
	ESLAssert( sizeof(ECSSakura2Processor::Register) == sizeof(INT64) ) ;
	const wchar_t *	pwszErr =
		BeginFunction
			( (((INT64)ECSExecutionImage::roasCode) << 56) | dwFuncAddr,
				(ECSSakura2Processor::Register*) pArg, nArgCount ) ;
	if ( pwszErr != NULL )
	{
		m_strErrMsg = pwszErr ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( m_status != xsExecution )
	{
		return	eslErrSuccess ;
	}
	/*
	m_regset[ECSSakura2Processor::regSP].i -= (nArgCount + 1) * 8 ;
	//
	INT64 *	pStack =
		(INT64*) AtomicTranslateAddress
					( m_regset[ECSSakura2Processor::regSP].i, (nArgCount + 1) * 8 ) ;
	if ( pStack == NULL )
	{
		if ( HandleExceptionExtendStack
				( ECSSakura2Processor::exceptionExtendStack )
					& ECSSakura2Processor::exceptionStackOverflow )
		{
			return	ESLErrorMsg( "スタックオーバーフローが発生しました" ) ;
		}
		pStack = (INT64*) AtomicTranslateAddress
					( m_regset[ECSSakura2Processor::regSP].i, (nArgCount + 1) * 8 ) ;
		if ( pStack == NULL )
		{
			return	ESLErrorMsg( "スタックオーバーフローが発生しました" ) ;
		}
	}
	m_ip = dwFuncAddr ;
	m_ipSegment = (ECSExecutionImage::roasCode << 24) ;
	//
	for ( int i = 0; i < nArgCount; i ++ )
	{
		pStack[i + 1] = pArg[i] ;
	}
	pStack[0] = -1 ;
	*/
	//
	ESLError	err = ResumeExecution() ;
	if ( !err )
	{
		m_regset[ECSSakura2Processor::regSP].i += nArgCount * 8 ;
	}
	return	err ;
}

// 実行継続
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ResumeExecution( ExecutionStatus xsStatus )
{
	if ( m_hSuspended == NULL )
	{
		m_hSuspended = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	}
	if ( m_hStatusEvent == NULL )
	{
		m_hStatusEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL ) ;
	}
	::ResetEvent( m_hSuspended ) ;
	//
	ESLError		errResult = eslErrSuccess ;
	ExecutionStatus	statusSave = SetStatus( xsStatus ) ;
	if ( m_status == xsSuspend )
	{
		::SetEvent( m_hSuspended ) ;
		while ( m_status == xsSuspend )
		{
			::WaitForSingleObject( m_hStatusEvent, 100 ) ;
		}
		::ResetEvent( m_hSuspended ) ;
	}
	while ( m_status == xsExecution )
	{
		errResult = ExecuteInstruction( ) ;
		if ( errResult )
		{
			statusSave = xsHalt ;
			break ;
		}
		if ( m_status == xsSuspend )
		{
			::SetEvent( m_hSuspended ) ;
			while ( m_status == xsSuspend )
			{
				::WaitForSingleObject( m_hStatusEvent, 100 ) ;
			}
			::ResetEvent( m_hSuspended ) ;
		}
	}
	//
	::SetEvent( m_hSuspended ) ;
	if ( m_status != xsInterrupt )
	{
		SetStatus( statusSave ) ;
	}
	return	errResult ;
}

// １命令実行
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteInstruction( void )
{
	ESLError	err = eslErrSuccess ;
	ESLAssert( m_status == xsExecution ) ;
	ESLAssert( m_pcsxi != NULL ) ;
	ESLAssert( m_ip < m_pcsxi->m_dwImageSize ) ;
	BYTE	nInstCode = m_pcsxi->m_pImage[m_ip] ;
	if ( !(nInstCode & 0x80) )
	{
		//
		// object mode 命令
		//
		ESLAssert( nInstCode < csicMax ) ;
		if ( nInstCode >= csicMax )
		{
			return	ESLErrorMsg( "不正な命令コードです。" ) ;
		}
		m_ip ++ ;
		err = (this->*m_pfnExecute[nInstCode])( ) ;
	}
	else
	{
		//
		// naked mode 命令実行
		//
		const wchar_t *	pwszErr = ExecuteShell() ;
		if ( pwszErr != NULL )
		{
			m_strErrMsg = pwszErr ;
			err = ESLErrorMsg( m_strErrMsg ) ;
			//
			SSystem::SString		strErrMsg = pwszErr ;
			SSystem::SArray<char>	bufErrMsg ;
			SSystem::Trace( "unhandle exception error\n" ) ;
			SSystem::Trace( "%s\n\n", strErrMsg.EncodeDefaultTo(bufErrMsg) ) ;
			Context::TraceDumpRegister() ;
		}
	}
	//
	// object mode に例外スロー
	//
	if ( err )
	{
		ESLTrace( "cotopha exception : %s #%08X\n", GetESLErrorMsg(err), m_ip ) ;
		m_arg.m_varArray.RemoveAll( ) ;
		m_arg.m_varArray.Add( new ECSInteger( 0x80000000 ) ) ;
		m_arg.m_varArray.Add
			( new ECSString( ECSWideString( GetESLErrorMsg(err) )
						+ L" (ip=" + ECSWideString( (DWORD) m_ip, 8 ) + L")" ) ) ;
		if ( !ThrowExpression() )
		{
			err = eslErrSuccess ;
		}
	}
	return	err ;
}

// 実行が一時停止されるまで待機する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::WaitForSuspended( DWORD dwTimeout )
{
	DWORD	dwWaitResult =
		::WaitForSingleObject( m_hSuspended, dwTimeout ) ;
	if ( dwWaitResult != WAIT_OBJECT_0 )
	{
		return	eslErrTimeout ;
	}
	return	eslErrSuccess ;
}

// 実行コンテキストの処理権を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Lock( DWORD dwTimeout )
{
	DWORD	dwWaitResult =
		::WaitForSingleObject( m_hExecutionMutex, dwTimeout ) ;
	if ( dwWaitResult != WAIT_OBJECT_0 )
	{
		return	eslErrTimeout ;
	}
	return	eslErrSuccess ;
}

// 実行コンテキストの処理権を解放する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Unlock( void )
{
	::ReleaseMutex( m_hExecutionMutex ) ;
	return	eslErrSuccess ;
}

// 実行中のコンテキストを一時停止させ処理権を取得する
//	（別スレッドからの同期処理用）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::LockExecution( DWORD dwTimeout )
{
	if ( m_status == xsHalt )
	{
		return	eslErrGeneral ;
	}
	DWORD	dwWaitResult =
		::WaitForSingleObject( m_hExecutionMutex, dwTimeout ) ;
	if ( dwWaitResult != WAIT_OBJECT_0 )
	{
		return	eslErrTimeout ;
	}
	ExecutionStatus	statusSave = SetStatus( xsSuspend ) ;
	dwWaitResult = ::WaitForSingleObject( m_hSuspended, dwTimeout ) ;
	if ( dwWaitResult != WAIT_OBJECT_0 )
	{
		SetStatus( statusSave ) ;
		::ReleaseMutex( m_hExecutionMutex ) ;
		return	eslErrTimeout ;
	}
	::InterlockedIncrement( &m_nLockedCount ) ;
	return	eslErrSuccess ;
}

// LockExecution で取得した処理権を解放する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::UnlockExecution( void )
{
	ESLAssert( m_nLockedCount > 0 ) ;
	ESLAssert( m_status == xsSuspend ) ;
	::ReleaseMutex( m_hExecutionMutex ) ;
	if ( ::InterlockedDecrement( &m_nLockedCount ) == 0 )
	{
		SetStatus( xsExecution ) ;
	}
	return	eslErrSuccess ;
}

// スクリプトからファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSContext::OpenFileOnScript
	( const wchar_t * pwszFileName, long int nOpenFlags )
{
	EString				strFileName = pwszFileName ;
	ECSEnvironment *	pEnv = GetEnvironment() ;
	if ( pEnv != NULL )
	{
		return	pEnv->OpenFileObject( strFileName, nOpenFlags ) ; 
	}
	else
	{
		ERawFile *	pfile = new ERawFile ;
		if ( pfile->Open( strFileName, nOpenFlags ) )
		{
			delete	pfile ;
			return	NULL ;
		}
		return	pfile ;
	}
}

// 実行ステータスを設定
//////////////////////////////////////////////////////////////////////////////
ECSContext::ExecutionStatus
	ECSContext::SetStatus( ECSContext::ExecutionStatus status )
{
	ESLAssert( sizeof(m_status) == sizeof(long) ) ;
	ECSContext::ExecutionStatus	statusOld =
		(ECSContext::ExecutionStatus)
			::InterlockedExchange( (LPLONG) &m_status, status ) ;
	ECSSakura2Processor::AtomicOr
		( &m_maskException, ECSSakura2Processor::interruptChangeStatus ) ;
	::SetEvent( m_hStatusEvent ) ;
	return	statusOld ;
}

// イベント待機
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::WaitUntilEvent
	( HANDLE hEvent, DWORD dwTimeout, DWORD dwFlags )
{
	DWORD	dwWaitResult ;
	HANDLE	hEvents[2] ;
	DWORD	dwCount = 0 ;
	DWORD	dwAbort = 0 ;
	if ( hEvent != NULL )
	{
		hEvents[dwCount ++] = hEvent ;
	}
	if ( (dwFlags & wfAbortFlag) && (m_hAbortEvent != NULL) )
	{
		dwAbort = dwCount ;
		hEvents[dwCount ++] = m_hAbortEvent ;
	}
	else
	{
//		::ResetEvent( m_hAbortEvent ) ;
	}
	if ( dwCount > 0 )
	{
		dwWaitResult =
			::WaitForMultipleObjects( dwCount, hEvents, FALSE, dwTimeout ) ;
	}
	else
	{
		::Sleep( dwTimeout ) ;
		return	eslErrTimeout ;
	}
	if ( (dwWaitResult >= WAIT_OBJECT_0)
		&& (dwWaitResult < (WAIT_OBJECT_0 + dwCount)) )
	{
		if ( dwWaitResult - WAIT_OBJECT_0 == dwAbort )
		{
//			::ResetEvent( m_hAbortEvent ) ;
			return	eslErrAbort ;
		}
		return	eslErrSuccess ;
	}
	else if ( dwWaitResult == WAIT_TIMEOUT )
	{
		return	eslErrTimeout ;
	}
	return	eslErrGeneral ;
}

// イベント待機を中止する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::AbortWatingEvent( void )
{
	::SetEvent( m_hAbortEvent ) ;
	return	eslErrSuccess ;
}

// イベント待機の中止を解除する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ResetAbortWatingEvent( void )
{
	::ResetEvent( m_hAbortEvent ) ;
	return	eslErrSuccess ;
}

// 実行コンテキストを保存する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Save( ESLFileObject & file )
{
	//
	// ファイルヘッダを書き出す
	//
	EMCFile					emcfile ;
	EMCFile::FILE_HEADER	fhHdr ;
	emcfile.SetFileHeader
		( fhHdr, emcfile.fidUndefinedEMC, "Cotopha Script Context" ) ;
	if ( emcfile.Open( &file, &fhHdr ) )
	{
		return	ESLErrorMsg( "ファイルヘッダを書き込めませんでした。" ) ;
	}
	//
	// 全てのスレッドにセーブ開始を通知する
	//
	ECSThread *	pNextThread ;
	ECotophaScript::Lock( ) ;
	pNextThread = m_pThreadList ;
	while ( pNextThread != NULL )
	{
		pNextThread->OnBeginningSave( *this ) ;
		pNextThread = pNextThread->m_pNextThread ;
	}
	ECotophaScript::Unlock( ) ;
	//
	// メンバにインデックスを振る
	//
	ESLError	err ;
	m_pcsxi->m_csgGlobal.IndexAllMember( ) ;
	m_pcsxi->m_csgData.IndexAllMember( ) ;
	m_pcsxi->m_csaHeap.IndexAllMember( ) ;
	m_pcsxi->m_csaHeapShared.IndexAllMember( ) ;
	m_stack.IndexAllMember( ) ;
	m_arg.IndexAllMember( ) ;
	//
	m_pcsxi->m_heapGlobal.PrepareSave( m_pcsxi, this ) ;
	m_pcsxi->m_heapShared.PrepareSave( m_pcsxi, this ) ;
	m_pcsxi->m_heapThread.PrepareSave( m_pcsxi, this ) ;
	//
	// クラスベクタを書き出す
	//
	if ( emcfile.DescendRecord( (UINT64*) "classvec" ) )
	{
		return	ESLErrorMsg( "クラスベクタの書き出しに失敗しました。" ) ;
	}
	m_pcsxi->SaveClassVector( emcfile ) ;
	emcfile.AscendRecord( ) ;
	//
	// コンテキストを書き出す
	//
	if ( emcfile.DescendRecord( (UINT64*) "context " ) )
	{
		return	ESLErrorMsg( "コンテキストの書き出しに失敗しました。" ) ;
	}
	SaveProcessorContext( emcfile ) ;
	emcfile.AscendRecord( ) ;
	//
	// 大域変数を書き出す
	//
	if ( emcfile.DescendRecord( (UINT64*) "global  " ) )
	{
		return	ESLErrorMsg( "大域変数の書き出しに失敗しました。" ) ;
	}
	err = m_pcsxi->m_csgGlobal.Save( emcfile, *this ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// スタックを書き出す
	//
	if ( emcfile.DescendRecord( (UINT64*) "stack   " ) )
	{
		return	ESLErrorMsg( "スタックの書き出しに失敗しました。" ) ;
	}
	err = m_stack.Save( emcfile, *this ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 関数の引数
	//
	if ( emcfile.DescendRecord( (UINT64*) "argument" ) )
	{
		return	ESLErrorMsg( "引数の書き出しに失敗しました。" ) ;
	}
	err = m_arg.Save( emcfile, *this ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 自由領域
	//
	if ( emcfile.DescendRecord( (UINT64*) "heap    " ) )
	{
		return	ESLErrorMsg( "ヒープテーブルの書き出しに失敗しました。" ) ;
	}
	err = SaveHeapMemory( emcfile ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 拡張データ
	//
	err = SaveExtendedData( emcfile ) ;
	if ( err )
	{
		return	err ;
	}
	//
	emcfile.Close( ) ;
	//
	// 全てのスレッドにセーブ完了を通知する
	//
	ECotophaScript::Lock( ) ;
	pNextThread = m_pThreadList ;
	while ( pNextThread != NULL )
	{
		pNextThread->OnFinishedSave( *this ) ;
		pNextThread = pNextThread->m_pNextThread ;
	}
	ECotophaScript::Unlock( ) ;
	return	eslErrSuccess ;
}

// 実行コンテキストを保存する（プロセッサコンテキスト）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::SaveProcessorContext( ESLFileObject & file )
{
	file.Write( &m_ip, sizeof(m_ip) ) ;
	file.Write( &m_ipSegment, sizeof(m_ipSegment) ) ;
	file.Write( &m_status, sizeof(m_status) ) ;
	file.Write( &m_nLastTickTime, sizeof(m_nLastTickTime) ) ;
	//
	file.Write( &m_vaStack, sizeof(m_vaStack) ) ;
	file.Write( &m_vaNakedStack, sizeof(m_vaNakedStack) ) ;
	//
	file.Write( &m_regset, sizeof(m_regset) ) ;
	//
	SaveObject( file, m_pRetObj ) ;
	//
	DWORD	dwExtendFlags = 0 ;
	file.Write( &dwExtendFlags, sizeof(dwExtendFlags) ) ;
	//
	return	eslErrSuccess ;
}

// 自由領域を保存する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::SaveHeapMemory( ESLFileObject & file )
{
	return	m_pcsxi->SaveHeapMemory( *this, file ) ;
}

// 実行コンテキストを復元する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Load( ESLFileObject & file )
{
	//
	// ファイルヘッダを読み込む
	//
	ESLError	err ;
	EMCFile		emcfile ;
	if ( emcfile.Open( &file ) )
	{
		return	ESLErrorMsg( "ファイルヘッダを読み込めませんでした。" ) ;
	}
	//
	// 全てのスレッドにロード開始を通知する
	//
	ECSThread *	pNextThread ;
	ECotophaScript::Lock( ) ;
	pNextThread = m_pThreadList ;
	while ( pNextThread != NULL )
	{
		pNextThread->OnBeginningLoad( *this ) ;
		pNextThread = pNextThread->m_pNextThread ;
	}
	ECotophaScript::Unlock( ) ;
	//
	// クラスベクタを読み込む
	//
	if ( !emcfile.DescendRecord( (UINT64*) "classvec" ) )
	{
		m_pcsxi->LoadClassVector( emcfile ) ;
		emcfile.AscendRecord( ) ;
	}
	//
	// コンテキストを読み込む
	//
	if ( emcfile.DescendRecord( (UINT64*) "context " ) )
	{
		return	ESLErrorMsg( "コンテキストの読み込みに失敗しました。" ) ;
	}
	LoadProcessorContext( emcfile ) ;
	emcfile.AscendRecord( ) ;
	//
	// 大域変数を読み込む
	//
	m_arg.CleanupAllReference( *this ) ;
	m_stack.CleanupAllReference( *this ) ;
	m_pcsxi->m_csgGlobal.CleanupAllReference( *this ) ;
	//
	m_arg.m_varArray.RemoveAll( ) ;
	m_stack.RemoveAll( ) ;
	m_pcsxi->m_csgGlobal.RemoveAllVariable( ) ;
	//
	if ( emcfile.DescendRecord( (UINT64*) "global  " ) )
	{
		return	ESLErrorMsg( "大域変数の読み込みに失敗しました。" ) ;
	}
	err = m_pcsxi->m_csgGlobal.Load( emcfile, *this ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// スタックを読み込む
	//
	if ( emcfile.DescendRecord( (UINT64*) "stack   " ) )
	{
		return	ESLErrorMsg( "スタックの読み込みに失敗しました。" ) ;
	}
	err = m_stack.Load( emcfile, *this ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 関数の引数
	//
	if ( emcfile.DescendRecord( (UINT64*) "argument" ) )
	{
		return	ESLErrorMsg( "引数の読み込みに失敗しました。" ) ;
	}
	err = m_arg.Load( emcfile, *this ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 自由領域
	//
	if ( emcfile.DescendRecord( (UINT64*) "heap    " ) )
	{
		return	ESLErrorMsg( "ヒープテーブルの読み込みに失敗しました。" ) ;
	}
	err = LoadHeapMemory( emcfile ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// メンバ変数の参照を解決する
	//
	err = m_pcsxi->m_csgGlobal.CommitAllReference( *this ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_stack.CommitAllReference( *this ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_arg.CommitAllReference( *this ) ;
	if ( err )
	{
		return	err ;
	}
	err = CommitLoadedProcessorContext() ;
	if ( err )
	{
		return	err ;
	}
	//
	// 拡張データ
	//
	err = LoadExtendedData( emcfile ) ;
	if ( err )
	{
		return	err ;
	}
	//
	emcfile.Close( ) ;
	//
	// 全てのスレッドにロード完了を通知する
	//
	ECotophaScript::Lock( ) ;
	pNextThread = m_pThreadList ;
	while ( pNextThread != NULL )
	{
		pNextThread->OnFinishedLoad( *this ) ;
		pNextThread = pNextThread->m_pNextThread ;
	}
	ECotophaScript::Unlock( ) ;
	//
	return	eslErrSuccess ;
}

// 実行コンテキストを復元する（プロセッサコンテキスト）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::LoadProcessorContext( ESLFileObject & file )
{
	if ( (m_vaStack != 0) && (m_pcsxi != NULL) )
	{
		m_pcsxi->FreeVirtualAddressDirectory
					( m_vaStack, &(m_stack.m_varArray) ) ;
		m_vaStack = 0 ;
	}
	//
	UINT64	nCurrentTime = 0 ;
	file.Read( &m_ip, sizeof(m_ip) ) ;
	file.Read( &m_ipSegment, sizeof(m_ipSegment) ) ;
	file.Read( &m_status, sizeof(m_status) ) ;
	file.Read( &nCurrentTime, sizeof(nCurrentTime) ) ;
	m_nLastTickTime = nCurrentTime ;
	m_dwLastBaseTime = ::timeGetTime( ) ;
	//
	file.Read( &m_vaStack, sizeof(m_vaStack) ) ;
	file.Read( &m_vaNakedStack, sizeof(m_vaNakedStack) ) ;
	m_bufNakedStack = NULL ;
	//
	file.Read( &m_regset, sizeof(m_regset) ) ;
	//
	if ( m_pRetObj != NULL )
	{
		delete	m_pRetObj ;
		m_pRetObj = NULL ;
	}
	LoadObject( file, m_pRetObj ) ;
	//
	DWORD	dwExtendFlags = 0 ;
	file.Read( &dwExtendFlags, sizeof(dwExtendFlags) ) ;
	//
	if ( (m_vaStack != 0) && (m_pcsxi != NULL) )
	{
		m_vaStack =
			m_pcsxi->AllocateVirtualAddressDirectory
							( m_vaStack, &(m_stack.m_varArray) ) ;
	}
	return	eslErrSuccess ;
}

// 実行コンテキストの復元処理を確定する（プロセッサコンテキスト）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::CommitLoadedProcessorContext( void )
{
	if ( (m_vaNakedStack != 0) && (m_pcsxi != NULL) )
	{
		m_bufNakedStack =
			ESLTypeCast<ECSBuffer>
				( m_pcsxi->ObjectFromAddress
					( (DWORD) (m_vaNakedStack >> 32) ) ) ;
		ESLAssert( m_bufNakedStack != NULL ) ;
	}
	return	eslErrSuccess ;
}

// 自由領域を復元する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::LoadHeapMemory( ESLFileObject & file )
{
	return	m_pcsxi->LoadHeapMemory( *this, file ) ;
}


// オブジェクトを保存する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::SaveObject( ESLFileObject & file, ECSObject * pObj )
{
	//
	// メンバにインデックスを振る
	//
//	m_pcsxi->m_csgGlobal.IndexAllMember( ) ;
//	m_pcsxi->m_csgData.IndexAllMember( ) ;
//	m_stack.IndexAllMember( ) ;
//	m_arg.IndexAllMember( ) ;
	//
	if ( pObj != NULL )
	{
		pObj->IndexAllMember( ) ;
		//
		// オブジェクトを保存
		//
		BYTE	bytType = (BYTE) pObj->m_vtType ;
		if ( file.Write( &bytType, sizeof(BYTE) ) < sizeof(BYTE) )
		{
			return	ESLErrorMsg( "オブジェクトの保存に失敗しました。" ) ;
		}
		if ( bytType == csvtObject )
		{
			ECSWideString	wstrType = pObj->GetTypeName( ) ;
			DWORD	dwLength = wstrType.GetLength( ) ;
			if ( file.Write( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
			{
				return	ESLErrorMsg( "オブジェクトの保存に失敗しました。" ) ;
			}
			if ( dwLength > 0 )
			{
				if ( file.Write( wstrType.CharPtr(),
					dwLength * sizeof(wchar_t) ) < dwLength * sizeof(wchar_t) )
				{
					return	ESLErrorMsg( "オブジェクトの保存に失敗しました。" ) ;
				}
			}
			#if	defined(_DEBUG)
			else
			{
				ESLAssert( wstrType.GetLength() > 0 ) ;
			}
			#endif
		}
		return	pObj->Save( file, *this ) ;
	}
	else
	{
		BYTE	bytNull = 0xFF ;
		if ( file.Write( &bytNull, sizeof(BYTE) ) < sizeof(BYTE) )
		{
			return	ESLErrorMsg( "オブジェクトの保存に失敗しました。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// オブジェクトを復元する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::LoadObject( ESLFileObject & file, ECSObject *& pObj )
{
	BYTE	bytType ;
	if ( file.Read( &bytType, sizeof(BYTE) ) < sizeof(BYTE) )
	{
		return	ESLErrorMsg( "オブジェクトの読み込みに失敗しました。" ) ;
	}
	if ( bytType != 0xFF )
	{
		switch ( bytType )
		{
		case	csvtReference:
			pObj = new ECSReference ;
			break ;
		case	csvtInteger:
			pObj = new ECSInteger ;
			break ;
		case	csvtReal:
			pObj = new ECSReal ;
			break ;
		case	csvtString:
			pObj = new ECSString ;
			break ;
		case	csvtArray:
			pObj = new ECSArray ;
			break ;
		case	csvtHash:
			pObj = new ECSHash ;
			break ;
		case	csvtPointer:
			pObj = new ECSPointer ;
			break ;
		case	csvtPointerReference:
			pObj = new ECSPointerReference ;
			break ;
		case	csvtBuffer:
			pObj = new ECSBuffer ;
			break ;
		case	csvtObject:
			{
				ECSWideString	wstrType ;
				DWORD			dwLength ;
				if ( file.Read( &dwLength,
						sizeof(dwLength) ) < sizeof(dwLength) )
				{
					return	ESLErrorMsg
						( "オブジェクトの読み込みに失敗しました。" ) ;
				}
				ESLAssert( dwLength > 0 ) ;
				if ( dwLength > 0 )
				{
					if ( file.Read( wstrType.GetBuffer(dwLength),
						dwLength * sizeof(wchar_t) ) < dwLength * sizeof(wchar_t) )
					{
						return	ESLErrorMsg
							( "オブジェクトの読み込みに失敗しました。" ) ;
					}
					wstrType.ReleaseBuffer( dwLength ) ;
				}
				pObj = CreateObject( csvtObject, wstrType ) ;
			}
			break ;
		default:
			return	ESLErrorMsg( "オブジェクトの型が不正です。" ) ;
		}
		if ( pObj == NULL )
		{
			return	ESLErrorMsg( "オブジェクトを生成できませんでした。" ) ;
		}
		ESLError	err = pObj->Load( file, *this ) ;
		if ( err )
		{
			delete	pObj ;
			pObj = NULL ;
		}
		return	err ;
	}
	else
	{
		pObj = NULL ;
	}
	return	eslErrSuccess ;
}

// 拡張コンテキストデータを保存する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::SaveExtendedData( EMCFile & file )
{
	return	eslErrSuccess ;
}

// 拡張コンテキストデータを復元する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::LoadExtendedData( EMCFile & file )
{
	return	eslErrSuccess ;
}

// オブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSContext::CreateObject
	( CSVariableType csvtType, const wchar_t * pwszType, DWORD * pdwFuncAddr )
{
	struct	BASIC_TYPE_INFO
	{
		const wchar_t *	pwszTypeName ;
		CSVariableType	csvtType ;
		INT64			nSizeMask ;
	} ;
	static const BASIC_TYPE_INFO	btiTypeInOrder[] =
	{
		{ L"Array", csvtArray, 0 },
		{ L"Boolean", csvtInteger, (INT64) 0x8000000000000000 },
		{ L"Hash", csvtHash, 0 },
		{ L"Int", csvtInteger, -1 },
		{ L"Int16", csvtInteger, (INT64) 0x8000000000007FFF },
		{ L"Int32", csvtInteger, (INT64) 0x800000007FFFFFFF },
		{ L"Int64", csvtInteger, -1 },
		{ L"Int8", csvtInteger, (INT64) 0x800000000000007F },
		{ L"Integer", csvtInteger, -1 },
		{ L"Object", csvtReference, 0 },
		{ L"Pointer", csvtPointer, 0 },
		{ L"PointerReference", csvtPointerReference, 0 },
		{ L"Real", csvtReal, 0 },
		{ L"Reference", csvtReference, 0 },
		{ L"String", csvtString, 0 },
		{ L"Uint16", csvtInteger, 0xFFFF },
		{ L"Uint32", csvtInteger, 0xFFFFFFFF },
		{ L"Uint8", csvtInteger, 0xFF },
	} ;
	const int	nTypeCountInOrder =
		sizeof(btiTypeInOrder) / sizeof(btiTypeInOrder[0]) ;
	//
	INT64	nSizeMask = -1 ;
	if ( pdwFuncAddr != NULL )
	{
		*pdwFuncAddr = (DWORD) -1 ;
	}
	if ( csvtType == csvtObject )
	{
		int	iFirst = 0, iEnd = nTypeCountInOrder - 1 ;
		for ( ; ; )
		{
			int	iMiddle = (iFirst + iEnd) / 2 ;
			int	nCompare =
				EWideString::CompareNoCase
					( btiTypeInOrder[iMiddle].pwszTypeName, pwszType ) ;
			if ( nCompare > 0 )
			{
				iEnd = iMiddle - 1 ;
			}
			else if ( nCompare < 0 )
			{
				iFirst = iMiddle + 1 ;
			}
			else
			{
				csvtType = btiTypeInOrder[iMiddle].csvtType ;
				nSizeMask = btiTypeInOrder[iMiddle].nSizeMask ;
				break ;
			}
			if ( iEnd < iFirst )
			{
				break ;
			}
		}
	}
	switch ( csvtType )
	{
	case	csvtReference:
		return	new_CSReference() ;
	case	csvtInteger:
	case	csvtInteger64:
		return	new_CSInteger( 0, nSizeMask ) ;
	case	csvtReal:
		return	new_CSReal() ;
	case	csvtString:
		return	new_CSString() ;
	case	csvtArray:
		return	new_CSArray() ;
	case	csvtHash:
		return	new_CSHash() ;
	case	csvtObject:
		break ;
	case	csvtPointer:
		return	new_CSPointer() ;
//	case	csvtFunction:
//		return	new_CSFunction() ;
	case	csvtBoolean:
		return	new_CSInteger( 0, 0x8000000000000000 ) ;
	case	csvtInt8:
		return	new_CSInteger( 0, 0x800000000000007F ) ;
	case	csvtUint8:
		return	new_CSInteger( 0, 0xFF ) ;
	case	csvtInt16:
		return	new_CSInteger( 0, 0x8000000000007FFF ) ;
	case	csvtUint16:
		return	new_CSInteger( 0, 0xFFFF ) ;
	case	csvtInt32:
		return	new_CSInteger( 0, 0x800000007FFFFFFF ) ;
	case	csvtUint32:
		return	new_CSInteger( 0, 0xFFFFFFFF ) ;
	case	csvtArrayDimension:
	case	csvtHashContainer:
		break ;
	case	csvtReal32:
	case	csvtReal64:
		return	new_CSReal() ;
	case	csvtPointerReference:
		return	new_CSPointerReference() ;
	}
	ECSObject *	pObj = CreateExtendedObject( csvtType, pwszType, pdwFuncAddr ) ;
	if ( pObj != NULL )
	{
		return	pObj ;
	}
	ECSWideString	wstrFuncName = pwszType ;
	wstrFuncName += L"::" ;
	wstrFuncName += pwszType ;
	DWORD *	pdwAddr = m_pcsxi->GetFunctionAddress( wstrFuncName ) ;
	if ( pdwAddr != NULL )
	{
		if ( pdwFuncAddr != NULL )
		{
			*pdwFuncAddr = *pdwAddr ;
		}
		ECSStructure *	pStruct = new ECSStructure ;
		pStruct->m_pwszTag = GetConstantString( pwszType )->CharPtr() ;
		return	pStruct ;
	}
	return	NULL ;
}

// 拡張型オブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSContext::CreateExtendedObject
	( CSVariableType csvtType,
		const wchar_t * pwszType, DWORD * pdwFuncAddr )
{
	enum	ExtendTypeIndex
	{
		xtiBuffer, xtiFile, xtiGlobal,
		xtiInputFilter, xtiMessageSprite, xtiModelJoint,
		xtiMovieSprite, xtiParticleModel, xtiParticleSprite, xtiPolygonModel,
		xtiRenderSprite, xtiResource, xtiResourceManager, xtiSetup,
		xtiSprite, xtiSuperSprite, xtiThread, xtiThreadEvent, xtiThreadMutex,
		xtiToneFilter, xtiWindow, xtiMax
	} ;
	struct	EXTEND_TYPE_INFO
	{
		const wchar_t *	pwszTypeName ;
		ExtendTypeIndex	xtiType ;
	} ;
	static const EXTEND_TYPE_INFO	xtiTypeInOrder[] =
	{
		{ L"Buffer", xtiBuffer},
		{ L"File", xtiFile },
		{ L"Global", xtiGlobal },
		{ L"InputFilter", xtiInputFilter },
		{ L"MessageSprite", xtiMessageSprite },
		{ L"ModelJoint", xtiModelJoint },
		{ L"MovieSprite", xtiMovieSprite },
		{ L"ParticleModel", xtiParticleModel },
		{ L"ParticleSprite", xtiParticleSprite },
		{ L"PolygonModel", xtiPolygonModel },
		{ L"RenderSprite", xtiRenderSprite },
		{ L"Resource", xtiResource },
		{ L"ResourceManager", xtiResourceManager },
		{ L"Setup", xtiSetup },
		{ L"Sprite", xtiSprite },
		{ L"SuperSprite", xtiSuperSprite },
		{ L"Thread", xtiThread },
		{ L"ThreadEvent", xtiThreadEvent },
		{ L"ThreadMutex", xtiThreadMutex },
		{ L"ToneFilter", xtiToneFilter },
		{ L"Window", xtiWindow },
	} ;
	const int	nTypeCountInOrder =
		sizeof(xtiTypeInOrder) / sizeof(xtiTypeInOrder[0]) ;
#if nTypeCountInOrder != xtiMax
#error should match length of xtiTypeInOrder array and xtiMax
#endif
	ExtendTypeIndex	xtiType = xtiMax ;
	int	iFirst = 0, iEnd = nTypeCountInOrder - 1 ;
	for ( ; ; )
	{
		int	iMiddle = (iFirst + iEnd) / 2 ;
		int	nCompare =
			EWideString::CompareNoCase
				( xtiTypeInOrder[iMiddle].pwszTypeName, pwszType ) ;
		if ( nCompare > 0 )
		{
			iEnd = iMiddle - 1 ;
		}
		else if ( nCompare < 0 )
		{
			iFirst = iMiddle + 1 ;
		}
		else
		{
			xtiType = xtiTypeInOrder[iMiddle].xtiType ;
			break ;
		}
		if ( iEnd < iFirst )
		{
			break ;
		}
	}
	switch ( xtiType )
	{
	case	xtiBuffer:
		return	new ECSBuffer ;
	case	xtiFile:
		return	new ECSFile ;
	case	xtiGlobal:
		return	new ECSGlobal ;
	case	xtiInputFilter:
		return	new ECSInputFilter ;
	case	xtiMessageSprite:
		return	new ECSMessageSprite ;
	case	xtiModelJoint:
		return	new ECSModelJoint ;
	case	xtiMovieSprite:
		return	new ECSMovieSprite ;
	case	xtiParticleModel:
		return	new ECSParticleModel ;
	case	xtiParticleSprite:
		return	new ECSParticleSprite ;
	case	xtiPolygonModel:
		return	new ECSPolygonModel ;
	case	xtiRenderSprite:
		return	new ECSRenderSprite ;
	case	xtiResource:
		return	new ECSResource ;
	case	xtiResourceManager:
		return	new ECSResourceManager ;
	case	xtiSetup:
		return	new ECSSetup ;
	case	xtiSprite:
		return	new ECSSprite ;
	case	xtiSuperSprite:
		return	new ECSSuperSprite ;
	case	xtiThread:
		return	new ECSThread( *this ) ;
	case	xtiThreadEvent:
		return	new ECSThreadEvent ;
	case	xtiThreadMutex:
		return	new ECSThreadMutex ;
	case	xtiToneFilter:
		return	new ECSToneFilter ;
	case	xtiWindow:
		return	new ECSWindow ;
		break ;
	}
	ECSClassInfo *	pClassInfo = m_pcsxi->GetClassInfoAs( pwszType ) ;
	if ( pClassInfo != NULL )
	{
		if ( pClassInfo->GetAttribute() & ECSTypeInfo::flagEnumerator )
		{
			return	CreateEnumeratorObject( *pClassInfo ) ;
		}
		else if ( pClassInfo->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
//			return	CreateObject( csvtObject, clsinf.GetGlobalName(), NULL ) ;
		}
		else
		{
			return	*(CreateUserClassObject( *pClassInfo )) ;
		}
	}
	if ( m_module != NULL )
	{
		typedef	ECSObject * (*API_CreateExtendedObject)
			( ECSContext & context, const wchar_t * pwszTypeName ) ;
		static API_CreateExtendedObject	pfnCreate = NULL ;
		static bool	fFirstFindFunc = false ;
		if ( (pfnCreate == NULL) && !fFirstFindFunc )
		{
			pfnCreate = (API_CreateExtendedObject)
				::GetProcAddress( m_module, "ecs_CreateExtendedObject" ) ;
			fFirstFindFunc = true ;
		}
		if ( pfnCreate != NULL )
		{
			ECSObject *	pObj = (*pfnCreate)( *this, pwszType ) ;
			if ( pObj != NULL )
			{
				return	pObj ;
			}
		}
	}
	ECSEnvironment *	pEnv = GetEnvironment() ;
	if ( pEnv != NULL )
	{
		for ( int i = 0; i < (int) pEnv->m_lstModule.GetSize(); i ++ )
		{
			ECSEnvironment::EPlugin *
				ppi = pEnv->m_lstModule.GetAt( i ) ;
			if ( (ppi != NULL) && (ppi->m_ppiet != NULL) )
			{
				ECS_OBJECT *	instance =
					ppi->m_ppiet->pfnCreateObject
						( GetContextInterface(), pwszType ) ;
				if ( instance != NULL )
				{
					ECSObject::PLUGIN_OBJECT *
						ppio = (ECSObject::PLUGIN_OBJECT*) instance ;
					ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
					return	ppio->pBackLink ;
				}
			}
		}
	}
	return	NULL ;
}

// クラス情報からオブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSContext::CreateClassObject( const ECSClassInfo & clsinf )
{
	if ( clsinf.GetAttribute() & ECSTypeInfo::flagEnumerator )
	{
		return	CreateEnumeratorObject( clsinf ) ;
	}
	else if ( clsinf.GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		return	CreateObject( csvtObject, clsinf.GetGlobalName(), NULL ) ;
	}
	return	*(CreateUserClassObject( clsinf )) ;
}

// 列挙型オブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSContext::CreateEnumeratorObject( const ECSClassInfo & clsinf )
{
	ESLAssert( clsinf.GetAttribute() & ECSTypeInfo::flagEnumerator ) ;
	ECSClassInfo::ParentClass *
		pParentType = clsinf.GetParentClassAt( 0 ) ;
	if ( (pParentType != NULL) && (pParentType->pClassInf != NULL) )
	{
		return	CreateObject
			( csvtObject, pParentType->pClassInf->GetGlobalName() ) ;
	}
	return	new_CSReference() ;
}

// ユーザー定義クラスのオブジェクトを生成する
//////////////////////////////////////////////////////////////////////////////
ECSStructureInterface *
	ECSContext::CreateUserClassObject( const ECSClassInfo & clsinf )
{
	ESLAssert( !(clsinf.GetAttribute()
		& (ECSTypeInfo::flagNativeObject | ECSTypeInfo::flagAbstract)) ) ;
	if ( clsinf.GetAttribute()
		& (ECSTypeInfo::flagNativeObject | ECSTypeInfo::flagAbstract) )
	{
		return	NULL ;
	}
	if ( clsinf.IsNakedMemoryClass() )
	{
		ECSBufferStructure *	pObj = new ECSBufferStructure( &clsinf ) ;
		BuildNakedUserClassObject( *pObj, clsinf ) ;
		return	pObj ;
	}
	else
	{
		ECSStructure *	pObj = new ECSStructure( &clsinf ) ;
		if ( BuildUserClassObject( *pObj, clsinf ) )
		{
			delete	pObj ;
			return	NULL ;
		}
		return	pObj ;
	}
}

ESLError ECSContext::BuildNakedUserClassObject
		( ECSBufferStructure & obj, const ECSClassInfo & clsinf )
{
	obj.m_pClassInf = &clsinf ;
	obj.CreateBuffer( clsinf.GetNakedMemorySize() ) ;
	//
	const int	nNakedSize = clsinf.GetNakedMemorySize() ;
	void *		ptrBuf = obj.GetBuffer( 0, nNakedSize, true ) ;
	if ( ptrBuf != NULL )
	{
		const void *	ptrInitImage =
				m_pcsxi->m_pImage + clsinf.m_dwNakedInitAddr ;
		eslMoveMemory( ptrBuf, ptrInitImage, nNakedSize ) ;
		//
		obj.FlushBuffer( 0, nNakedSize, ptrBuf, true ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSContext::BuildUserClassObject
		( ECSStructure & obj, const ECSClassInfo & clsinf )
{
	obj.m_pClassInf = &clsinf ;
	obj.m_pwszTag = clsinf.GetGlobalName() ;
	//
	unsigned int	nVarCount = clsinf.GetVariableCount() ;
	obj.m_varArray.SetLimit( nVarCount ) ;
	obj.SetBounds( nVarCount ) ;
	//
	for ( unsigned int i = 0; i < nVarCount; i ++ )
	{
		ECSTypeInfo *	pVarType = clsinf.GetVariableAt( i ) ;
		ESLAssert( pVarType != NULL ) ;
		if ( pVarType == NULL )
		{
			return	eslErrFailed ;
		}
		if ( pVarType->m_pValue != NULL )
		{
			const ECSObject *	pType = pVarType->m_pValue ;
			ECSObject *	pMember ;
			ESLError	err = CreateInstanceFromTypeInfo( pMember, pType ) ;
			if ( err || (pMember == NULL) )
			{
				return	eslErrFailed ;
			}
			obj.m_varArray.SetAt( i, pMember ) ;
		}
		else
		{
			return	eslErrFailed ;
		}
	}
	return	eslErrSuccess ;
}

// Sakura2 共通オブジェクト生成インターフェース
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSContext::Sakura2DefaultNewObject
		( ECSSakura2Processor::Context * pcontext, int cls_id )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	const ECSClassInfo *
		pClassInf = context->m_pcsxi->GetClassInfoAt( cls_id ) ;
	if ( pClassInf != NULL )
	{
		return	context->CreateClassObject( *pClassInf ) ;
	}
	return	NULL ;
}

// 拡張ネイティブ関数を取得する
//////////////////////////////////////////////////////////////////////////////
API_ECS_FUNC ECSContext::GetImportNativeFunction( const wchar_t * pwszFuncName )
{
	return	(API_ECS_FUNC)
				::GetProcAddress
					( m_module, "ecs_" + EString(pwszFuncName) ) ;
}

// グローバル関数を呼び出す
//////////////////////////////////////////////////////////////////////////////
static ESLError call_api
	( long int & nRetValue, FARPROC pfnFunc, EStreamBuffer & bufArg )
{
	DWORD		dwBufBytes = bufArg.GetLength() ;
	void *		ptrBuf = bufArg.ModifyBuffer( 0, dwBufBytes ) ;
	DWORD		dwSaveESP, dwRetValue ;
	ESLError	err = eslErrSuccess ;
	__try
	{
		__asm
		{
			mov		dwSaveESP, esp
			sub		esp, dwBufBytes
			mov		esi, ptrBuf
			mov		edi, esp
			mov		ecx, dwBufBytes
			pushf
			cld
			rep		movsb
			popf
			call	[pfnFunc]
			mov		esp, dwSaveESP
			mov		dwRetValue, eax
		}
		nRetValue = dwRetValue ;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		err = eslErrGeneral ;
	}
	return	err ;
}

ESLError ECSContext::CallGlobalFunction( const wchar_t * pwszFuncName )
{
	//
	// スクリプトグローバル関数
	//
	DWORD *	pdwFuncAddr = m_pcsxi->GetFunctionAddress( pwszFuncName ) ;
	if ( pdwFuncAddr != NULL )
	{
		PushObject( new ECSInteger( m_ip ) ) ;
		m_ip = *pdwFuncAddr ;
		MarkCallStackFlag( ) ;
		return	eslErrSuccess ;
	}
	//
	// システム関数
	//
	if ( m_staFuncName != NULL )
	{
		int	nIndex = m_staFuncName->FindIndex( pwszFuncName ) ;
		if ( (nIndex >= 0) && (nIndex < (int) m_staFuncName->GetSize()) )
		{
			ESLError	err =
				(this->*m_pfnCallFunc[nIndex])( m_arg.m_varArray ) ;
			m_arg.m_varArray.RemoveAll( ) ;
			return	err ;
		}
	}
	//
	// システムグローバル関数
	//
	API_ECS_FUNC	apiCallFunc = GetImportNativeFunction( pwszFuncName ) ;
	if ( apiCallFunc != NULL )
	{
		ESLError	err = apiCallFunc( *this, m_arg.m_varArray ) ;
		m_arg.m_varArray.RemoveAll( ) ;
		return	err ;
	}
	ECSEnvironment *	pEnv = GetEnvironment() ;
	if ( pEnv != NULL )
	{
		//
		// モジュール API 呼び出し
		//
		EString	strFuncName = pwszFuncName ;
		FARPROC	pfnFunc = FindPluginedFunction( strFuncName ) ;
		if ( pfnFunc != NULL )
		{
			return	CallFunctionWin32x86API( pfnFunc, m_arg.m_varArray ) ;
		}
	}
	m_strErrMsg = "関数 \'" + EString(pwszFuncName) + "\' が見つかりません。" ;
	return	ESLErrorMsg( m_strErrMsg ) ;
}

// DLL 関数を検索する
//////////////////////////////////////////////////////////////////////////////
FARPROC ECSContext::FindPluginedFunction( const char * pszFuncName )
{
	int		i ;
	FARPROC	pfnFunc = NULL ;
	ECSEnvironment *	pEnv = GetEnvironment() ;
	for ( i = 0; i < (int) pEnv->m_lstModule.GetSize(); i ++ )
	{
		ECSEnvironment::EPlugin *	ppi = pEnv->m_lstModule.GetAt( i ) ;
		if ( (ppi != NULL) && (ppi->m_hModule != NULL) )
		{
			pfnFunc = ::GetProcAddress( ppi->m_hModule, pszFuncName ) ;
			if ( pfnFunc != NULL )
			{
				break ;
			}
		}
	}
	return	pfnFunc ;
}

// Windows API を呼び出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::CallFunctionWin32x86API
	( FARPROC pfnFunc, ECSObjArray<ECSObject> & lstArg )
{
	EObjArray<EString>		lstStrBuf ;
	EObjArray<INT64>		lstIntBuf ;
	EPtrArray				lstNakedBufRef ;
	EStreamBuffer			bufArg ;
	int	i ;
	for ( i = 0; i < (int) lstArg.GetSize(); i ++ )
	{
		ECSObject *	pArg = lstArg.GetAt( i ) ;
		ECSObject *	obj = ECSObject::GetEntity( pArg ) ;
		if ( obj == NULL )
		{
			continue ;
		}
		INT64 *		ptrInt ;
		double *	ptrDouble ;
		char *		pszBuf ;
		void *		ptrBuf ;
		switch ( obj->m_vtType )
		{
		case	csvtInteger:
			if ( pArg->m_vtType == csvtReference )
			{
				lstIntBuf[i] = ((ECSInteger*)obj)->GetValue( ) ;
				ptrInt = lstIntBuf.GetAt( i ) ;
				bufArg.Write( &ptrInt, sizeof(ptrInt) ) ;
			}
			else
			{
				int	nValue = ((ECSInteger*)obj)->GetInt( ) ;
				bufArg.Write( &nValue, sizeof(int) ) ;
			}
			break ;
		case	csvtReal:
			ptrDouble = &(((ECSReal*)obj)->m_varReal) ;
			if ( pArg->m_vtType == csvtReference )
			{
				bufArg.Write( &ptrDouble, sizeof(ptrDouble) ) ;
			}
			else
			{
				bufArg.Write
					( ptrDouble, sizeof(((ECSReal*)obj)->m_varReal) ) ;
			}
			break ;
		case	csvtString:
			lstStrBuf[i] = ((ECSString*)obj)->m_varStr ;
			pszBuf = lstStrBuf[i].GetBuffer( 0x1000 ) ;
			bufArg.Write( &pszBuf, sizeof(pszBuf) ) ;
			break ;
		case	csvtPointer:
			ptrBuf = obj->GetBuffer( 0, 0, true ) ;
			lstNakedBufRef.SetAt( i, ptrBuf ) ;
			bufArg.Write( &ptrBuf, sizeof(ptrBuf) ) ;
			break ;
		}
	}
	long int	nRetValue ;
	if ( !call_api( nRetValue, pfnFunc, bufArg ) )
	{
		for ( i = 0; i < (int) lstIntBuf.GetSize(); i ++ )
		{
			INT64 *	ptrInt = lstIntBuf.GetAt( i ) ;
			if ( ptrInt == NULL )
			{
				continue ;
			}
			ECSInteger *	pObjInt =
				ESLTypeCast<ECSInteger>
					( ECSObject::GetEntity( lstArg.GetAt(i) ) ) ;
			ESLAssert( pObjInt != NULL ) ;
			if ( pObjInt != NULL )
			{
				pObjInt->SetValue( *ptrInt ) ;
			}
		}
		for ( i = 0; i < (int) lstStrBuf.GetSize(); i ++ )
		{
			EString *	pstrBuf = lstStrBuf.GetAt( i ) ;
			if ( pstrBuf == NULL )
			{
				continue ;
			}
			ECSString *	pObjStr =
				ESLTypeCast<ECSString>
					( ECSObject::GetEntity( lstArg.GetAt(i) ) ) ;
			ESLAssert( pObjStr != NULL ) ;
			if ( pObjStr != NULL )
			{
				pstrBuf->ReleaseBuffer( ) ;
				pObjStr->m_varStr = *pstrBuf ;
			}
		}
		for ( i = 0; i < (int) lstNakedBufRef.GetSize(); i ++ )
		{
			void *	ptrBuf = lstNakedBufRef.GetAt( i ) ;
			if ( ptrBuf == NULL )
			{
				continue ;
			}
			ECSObject *	pArg = lstArg.GetAt( i ) ;
			ECSObject *	obj = ECSObject::GetEntity( pArg ) ;
			if ( obj == NULL )
			{
				continue ;
			}
			obj->FlushBuffer( 0, 0, ptrBuf, true ) ;
		}
		return	PushObject( new ECSInteger( nRetValue ) ) ;
	}
	return	ESLErrorMsg( "外部 API 呼び出しで例外エラーが発生しました。" ) ;
}

// コンテキストをダンプする
//////////////////////////////////////////////////////////////////////////////
void ECSContext::DumpContext( EStreamBuffer & buf )
{
	IndexAllMember( ) ;
	//
	EString	strDump ;
	strDump = "\n[stack]\n" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	m_stack.DumpObject( buf, 0, *this ) ;
	//
	if ( m_pcsxi != NULL )
	{
		strDump = "\n[global]\n" ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
		m_pcsxi->m_csgGlobal.DumpObject( buf, 0, *this ) ;
		//
		strDump = "\n[data]\n" ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
		m_pcsxi->m_csgData.DumpObject( buf, 0, *this ) ;
	}
	//
	strDump = "\n[Sakura2 processor context]\n" ;
	strDump += "ip = " + EString( (DWORD) m_ipSegment, 8 )
							+ ":" + EString( (DWORD) m_ip, 8 ) + "\n" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	for ( int i = 0; i < 0x80; i += 4 )
	{
		wsprintf( strDump.GetBuffer(0x100),
			"r%d=%08X%08X r%d=%08X%08X r%d=%08X%08X r%d=%08X%08X\n",
			i, (DWORD) m_regset[i].h32, (DWORD) m_regset[i].l32,
			i+1, (DWORD) m_regset[i+1].h32, (DWORD) m_regset[i+1].l32,
			i+2, (DWORD) m_regset[i+2].h32, (DWORD) m_regset[i+2].l32,
			i+3, (DWORD) m_regset[i+3].h32, (DWORD) m_regset[i+3].l32 ) ;
		strDump.ReleaseBuffer() ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	}
}

// 全ての変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSContext::IndexAllMember( void )
{
	if ( m_pcsxi != NULL )
	{
		m_pcsxi->m_csgGlobal.IndexAllMember( ) ;
		m_pcsxi->m_csgData.IndexAllMember( ) ;
	}
	m_stack.IndexAllMember( ) ;
	m_arg.IndexAllMember( ) ;
}

// スレッドリストに追加する
//////////////////////////////////////////////////////////////////////////////
void ECSContext::AddThreadList( ECSThread * pThread )
{
	ECotophaScript::Lock( ) ;
	if ( (pThread->m_pPrevThread != NULL) || (pThread->m_pNextThread != NULL) )
	{
		if ( pThread->m_pPrimaryContext != NULL )
		{
			pThread->m_pPrimaryContext->RemoveThreadList( pThread ) ;
		}
	}
	if ( m_pThreadList != NULL )
	{
		m_pThreadList->m_pPrevThread = pThread ;
	}
	pThread->m_pNextThread = m_pThreadList ;
	pThread->m_pPrevThread = NULL ;
	m_pThreadList = pThread ;
	ECotophaScript::Unlock( ) ;
}

// スレッドリストから削除する
//////////////////////////////////////////////////////////////////////////////
void ECSContext::RemoveThreadList( ECSThread * pThread )
{
	ECotophaScript::Lock( ) ;
	if ( pThread->m_pNextThread != NULL )
	{
		pThread->m_pNextThread->m_pPrevThread = pThread->m_pPrevThread ;
	}
	if ( pThread->m_pPrevThread != NULL )
	{
		pThread->m_pPrevThread->m_pNextThread = pThread->m_pNextThread ;
	}
	else
	{
		m_pThreadList = pThread->m_pNextThread ;
	}
	pThread->m_pPrevThread = NULL ;
	pThread->m_pNextThread = NULL ;
	ECotophaScript::Unlock( ) ;
}

// 全てのサブスレッドを一時停止する
//////////////////////////////////////////////////////////////////////////////
void ECSContext::SuspendAllThread( void )
{
	ECSThread *	pNextThread ;
	for ( ; ; )
	{
		// assert lock 同期中の場合は、解除されるまで待つ
		ECSSakura2Processor::signalUnlocked->Wait( 10 ) ;
		ECotophaScript::Lock( ) ;
		if ( !(ECSSakura2Processor::maskGlobalInterrupt
					& ECSSakura2Processor::interruptAssertLock) )
		{
			break ;
		}
		ECotophaScript::Unlock( ) ;
	}
	pNextThread = m_pThreadList ;
	while( pNextThread != NULL )
	{
		if ( pNextThread->IsThreadRunning() )
		{
			pNextThread->SuspendThread( ) ;
		}
		pNextThread = pNextThread->m_pNextThread ;
	}
	ECotophaScript::Unlock( ) ;
}

// 全てのサブスレッドの実行を再開する
//////////////////////////////////////////////////////////////////////////////
void ECSContext::ResumeAllThread( void )
{
	ECSThread *	pNextThread ;
	ECotophaScript::Lock( ) ;
	pNextThread = m_pThreadList ;
	while( pNextThread != NULL )
	{
		if ( pNextThread->IsThreadRunning() )
		{
			pNextThread->ResumeThread( ) ;
		}
		pNextThread = pNextThread->m_pNextThread ;
	}
	ECotophaScript::Unlock( ) ;
}

// 関数の引数の数をチェックする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::VerifyArgumentCount
	( ECSObjArray<ECSObject> & lstArg, int nArgMin, int nArgMax )
{
	if ( nArgMax == 0 )
	{
		nArgMax = nArgMin ;
	}
	if ( ((int) lstArg.GetSize() < nArgMin)
			|| (nArgMax < (int) lstArg.GetSize()) )
	{
		return	ESLErrorMsg( "関数の引数の数が一致しません。" ) ;
	}
	return	eslErrSuccess ;
}

// 関数の引数を型を特定して取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::GetArgumentAsInt
	( int & nValue,
		ECSObjArray<ECSObject> & lstArg, int iArg, int nDefValue )
{
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(iArg) ) ;
	if ( pObj == NULL )
	{
		nValue = nDefValue ;
		return	eslErrSuccess ;
	}
	if ( pObj->m_vtType == csvtInteger )
	{
		nValue = ((ECSInteger*)pObj)->GetInt() ;
		return	eslErrSuccess ;
	}
	INT64	nValue64 ;
	if ( !pObj->OperateInteger( nValue64 ) )
	{
		nValue = (int) nValue64 ;
		return	eslErrSuccess ;
	}
	/*
	else if ( pObj->m_vtType == csvtReal )
	{
		nValue = (long int)
			::eriRoundR64ToLInt( ((ECSReal*)pObj)->m_varReal ) ;
		return	eslErrSuccess ;
	}
	pObj = GetArgumentObjectAs( lstArg, iArg, L"Integer" ) ;
	if ( (pObj != NULL) && (pObj->m_vtType == csvtInteger) )
	{
		nValue = ((ECSInteger*)pObj)->GetInt() ;
		return	eslErrSuccess ;
	}
	*/
	return	ESLErrorMsg
		( "整数型引数に不正な型のオブジェクトが渡されました。" ) ;
}

ESLError ECSContext::GetArgumentAsInt64
	( INT64 & nValue,
		ECSObjArray<ECSObject> & lstArg, int iArg, INT64 nDefValue )
{
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(iArg) ) ;
	if ( pObj == NULL )
	{
		nValue = nDefValue ;
		return	eslErrSuccess ;
	}
	if ( pObj->m_vtType == csvtInteger )
	{
		nValue = ((ECSInteger*)pObj)->GetValue() ;
		return	eslErrSuccess ;
	}
	if ( !pObj->OperateInteger( nValue ) )
	{
		return	eslErrSuccess ;
	}
	/*
	else if ( pObj->m_vtType == csvtReal )
	{
		nValue =
			::eriRoundR64ToLInt( ((ECSReal*)pObj)->m_varReal ) ;
		return	eslErrSuccess ;
	}
	pObj = GetArgumentObjectAs( lstArg, iArg, L"Integer" ) ;
	if ( (pObj != NULL) && (pObj->m_vtType == csvtInteger) )
	{
		nValue = ((ECSInteger*)pObj)->GetValue() ;
		return	eslErrSuccess ;
	}
	*/
	return	ESLErrorMsg
		( "整数型引数に不正な型のオブジェクトが渡されました。" ) ;
}

ESLError ECSContext::GetArgumentAsReal
	( double & rValue,
		ECSObjArray<ECSObject> & lstArg, int iArg, double rDefValue )
{
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(iArg) ) ;
	if ( pObj == NULL )
	{
		rValue = rDefValue ;
		return	eslErrSuccess ;
	}
	if ( pObj->m_vtType == csvtReal )
	{
		rValue = ((ECSReal*)pObj)->m_varReal ;
		return	eslErrSuccess ;
	}
	if ( !pObj->OperateReal( rValue ) )
	{
		return	eslErrSuccess ;
	}
	/*
	else if ( pObj->m_vtType == csvtInteger )
	{
		rValue = (double) ((ECSInteger*)pObj)->m_varInt ;
		return	eslErrSuccess ;
	}
	pObj = GetArgumentObjectAs( lstArg, iArg, L"Real" ) ;
	if ( (pObj != NULL) && (pObj->m_vtType == csvtReal) )
	{
		rValue = ((ECSReal*)pObj)->m_varReal ;
		return	eslErrSuccess ;
	}
	*/
	return	ESLErrorMsg
		( "実数型引数に不正な型のオブジェクトが渡されました。" ) ;
}

ESLError ECSContext::GetArgumentAsStr
	( ECSWideString & wstrValue,
		ECSObjArray<ECSObject> & lstArg,
		int iArg, const wchar_t * pwszDefValue )
{
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(iArg) ) ;
	if ( pObj == NULL )
	{
		wstrValue = pwszDefValue ;
		return	eslErrSuccess ;
	}
	if ( pObj->m_vtType == csvtString )
	{
		wstrValue = ((ECSString*)pObj)->m_varStr ;
		return	eslErrSuccess ;
	}
	if ( !pObj->OperateString( wstrValue ) )
	{
		return	eslErrSuccess ;
	}
	/*
	pObj = GetArgumentObjectAs( lstArg, iArg, L"String" ) ;
	if ( (pObj != NULL) && (pObj->m_vtType == csvtString) )
	{
		wstrValue = ((ECSString*)pObj)->m_varStr ;
		return	eslErrSuccess ;
	}
	*/
	return	ESLErrorMsg
		( "文字列型引数に不正な型のオブジェクトが渡されました。" ) ;
}

ECSObject * ECSContext::GetArgumentObjectAs
	( ECSObjArray<ECSObject> & lstArg,
			int iArg, const wchar_t * pwszTypeName )
{
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(iArg) ) ;
	if ( pObj != NULL )
	{
		return	pObj->GetTypeOf( pwszTypeName ) ;
	}
	return	NULL ;
}

// ユーザー定義構造体・クラスを生成する
//////////////////////////////////////////////////////////////////////////////
ECSStructure *
	ECSContext::CreateUserStructureObject( const wchar_t * pwszName )
{
	ECSStructure *	pStruct = NULL ;
	ECSClassInfo *	pClassInf = GetClassInfoAs( pwszName ) ;
	if ( pClassInf != NULL )
	{
		if ( pClassInf->GetAttribute()
			& (ECSTypeInfo::flagNativeObject | ECSTypeInfo::flagAbstract) )
		{
			return	NULL ;
		}
		pStruct = new ECSStructure( pClassInf ) ;
		if ( BuildUserClassObject( *pStruct, *pClassInf ) )
		{
			delete	pStruct ;
			return	NULL ;
		}
		return	pStruct ;
	}
	else
	{
		pStruct = new ECSStructure ;
		pStruct->m_pwszTag = pwszName ;
	}
	return	pStruct ;
}

ECSStructureInterface *
	ECSContext::CreateUserStructure( const wchar_t * pwszName )
{
	ECSStructure *	pStruct = NULL ;
	ECSClassInfo *	pClassInf = GetClassInfoAs( pwszName ) ;
	if ( pClassInf != NULL )
	{
		return	CreateUserClassObject( *pClassInf ) ;
	}
	else
	{
		pStruct = new ECSStructure ;
		pStruct->m_pwszTag = pwszName ;
	}
	return	pStruct ;
}

// 実行関数 : New 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteNew( void )
{
	//
	// 命令解析
	//
	ESLError		err ;
	BYTE *			pImage = m_pcsxi->m_pImage ;
	CSObjectMode	csomType = (CSObjectMode) pImage[m_ip ++] ;
	CSVariableType	csvtType = (CSVariableType) pImage[m_ip ++] ;
	ECSWideString *	pwstrType = NULL ;
	const wchar_t *	pwszType = NULL ;
	ECSWideString *	pwstrName ;
	ECSObject *		pNewObj = NULL ;
	DWORD			dwFuncAddr = (DWORD) -1 ;
	if ( csvtType == csvtClassObject )
	{
		DWORD	dwClassIndex = *((DWORD*)(pImage + m_ip)) ;
		m_ip += sizeof(DWORD) ;
		//
		const ECSClassInfo *
				pClassInf = m_pcsxi->GetClassInfoAt( dwClassIndex ) ;
		if ( pClassInf != NULL )
		{
			pNewObj = CreateClassObject( *pClassInf ) ;
			if ( pNewObj == NULL )
			{
				return	ESLErrorMsg( "クラスの作成に失敗しました。" ) ;
			}
		}
		else
		{
			return	ESLErrorMsg( "不正なクラスを指定しています。" ) ;
		}
		pwstrName = GetStringLiteral() ;
	}
	else
	{
		if ( csvtType == csvtObject )
		{
			pwstrType = GetStringLiteral() ;
			pwszType = *pwstrType ;
		}
		pwstrName = GetStringLiteral() ;
		//
		// 変数作成
		//
		pNewObj = CreateObject( csvtType, pwszType, &dwFuncAddr ) ;
		if ( pNewObj == NULL )
		{
			return	ESLErrorMsg( "変数の作成に失敗しました。" ) ;
		}
	}
	if ( csomType == csomStack )
	{
		//
		// スタック上に変数作成
		//
		err = m_stack.CreateNewVariable( *pwstrName, pNewObj ) ;
		if ( err )
		{
			return	err ;
		}
		if ( pNewObj->m_vtType == csvtReference )
		{
			((ECSReference*)pNewObj)->m_fNontemp = true ;
		}
	}
	else if ( csomType == csomThis )
	{
		//
		// this オブジェクトに変数作成
		//
		ECSStructure *	pThisStruct =
			ESLTypeCast<ECSStructure>
				( ECSObject::GetEntity( m_stack.GetCurrentThisObject() ) ) ;
		if ( pThisStruct == NULL )
		{
			return	ESLErrorMsg( "this オブジェクトが見つかりません。" ) ;
		}
		err = pThisStruct->AddNewVariable( *pwstrName, pNewObj ) ;
		if ( err )
		{
			return	err ;
		}
		if ( pNewObj->m_vtType == csvtReference )
		{
			((ECSReference*)pNewObj)->m_fNontemp = true ;
		}
	}
	else
	{
		return	ESLErrorMsg( "変数作成命令の記憶クラスが不正です。" ) ;
	}
	//
	// 構造体の場合には構築関数呼び出し
	//
	if ( dwFuncAddr != (DWORD) -1 )
	{
		PushObject( new_CSInteger( m_ip ) ) ;
		m_ip = dwFuncAddr ;
		MarkCallStackFlag( ) ;
		m_arg.m_varArray.RemoveAll( ) ;
		m_arg.m_varArray.Add( new_CSReference( pNewObj ) ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : Free 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteFree( void )
{
	delete_CSObject( PopObject( ) ) ;
	return	eslErrSuccess ;
}

// 実行関数 : Load 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteLoad( void )
{
	ECSObject *		pObj ;
	BYTE *			pImage = m_pcsxi->m_pImage ;
	CSObjectMode	csomType = (CSObjectMode) pImage[m_ip ++] ;
	CSVariableType	csvtType = (CSVariableType) pImage[m_ip ++] ;
	if ( csomType == csomImmediate )
	{
		//
		// 即値読み込み
		//
		switch ( csvtType )
		{
		case	csvtInteger64:
			pObj = new_CSInteger( *((INT64*)(pImage + m_ip)) ) ;
			m_ip += sizeof(INT64) ;
			break ;
		case	csvtInteger:
			pObj = new_CSInteger( *((long int*)(pImage + m_ip)) ) ;
			m_ip += sizeof(long int) ;
			break ;
		case	csvtBoolean:
			pObj = new_CSInteger
				( *((SBYTE*)(pImage + m_ip)) ) ;
			m_ip += sizeof(SBYTE) ;
			break ;
		case	csvtInt8:
			pObj = new_CSInteger
				( *((SBYTE*)(pImage + m_ip)) ) ;
			m_ip += sizeof(SBYTE) ;
			break ;
		case	csvtUint8:
			pObj = new_CSInteger
				( *((BYTE*)(pImage + m_ip)) ) ;
			m_ip += sizeof(BYTE) ;
			break ;
		case	csvtInt16:
			pObj = new_CSInteger
				( *((SWORD*)(pImage + m_ip)) ) ;
			m_ip += sizeof(SWORD) ;
			break ;
		case	csvtUint16:
			pObj = new_CSInteger
				( *((WORD*)(pImage + m_ip)) ) ;
			m_ip += sizeof(WORD) ;
			break ;
		case	csvtInt32:
			pObj = new_CSInteger
				( *((SDWORD*)(pImage + m_ip)) ) ;
			m_ip += sizeof(SDWORD) ;
			break ;
		case	csvtUint32:
			pObj = new_CSInteger
				( *((DWORD*)(pImage + m_ip)) ) ;
			m_ip += sizeof(DWORD) ;
			break ;
		case	csvtReal:
			pObj = new_CSReal( *((double*)(pImage + m_ip)) ) ;
			m_ip += sizeof(double) ;
			break ;
		case	csvtString:
			pObj = new_CSString( ) ;
			LoadStringLiteral( ((ECSString*)pObj)->m_varStr ) ;
			break ;
		case	csvtReference:
			pObj = new_CSReference( ) ;
			break ;
		case	csvtArray:
			pObj = new_CSArray( ) ;
			break ;
		case	csvtHash:
			pObj = new_CSHash( ) ;
			break ;
		case	csvtPointer:
			pObj = new_CSPointer( ) ;
			break ;
		/*
		case	csvtFunction:
			pObj = new_CSFunction
				( *((DWORD*)(pImage + m_ip + 1)),
					(ECSFunction::FunctionType) pImage[m_ip] ) ;
			m_ip += sizeof(BYTE) + sizeof(DWORD) ;
			break ;
		*/
		case	csvtClassObject:
			{
				DWORD	dwClassIndex = *((DWORD*)(pImage + m_ip)) ;
				m_ip += sizeof(DWORD) ;
				//
				const ECSClassInfo *
					pClassInf = m_pcsxi->GetClassInfoAt( dwClassIndex ) ;
				if ( pClassInf == NULL )
				{
					return	ESLErrorMsg( "クラス情報が見つかりません。" ) ;
				}
				pObj = CreateClassObject( *pClassInf ) ;
				if ( pObj == NULL )
				{
					return	ESLErrorMsg( "クラスの生成に失敗しました。" ) ;
				}
			}
			break ;
		case	csvtObject:
			{
				DWORD	dwFuncAddr = (DWORD) -1 ;
				ECSWideString *	pwstrType = GetStringLiteral() ;
				pObj = CreateObject( csvtObject, *pwstrType, &dwFuncAddr ) ;
				if ( pObj == NULL )
				{
					return	ESLErrorMsg( "即値をロードできませんでした。" ) ;
				}
				PushObject( pObj ) ;
				if ( dwFuncAddr != (DWORD) -1 )
				{
					PushObject( new_CSInteger( m_ip ) ) ;
					m_ip = dwFuncAddr ;
					MarkCallStackFlag( ) ;
					m_arg.m_varArray.RemoveAll( ) ;
					m_arg.m_varArray.Add( new_CSReference( pObj ) ) ;
				}
			}
			return	eslErrSuccess ;
		default:
			return	ESLErrorMsg( "不正な型情報です。" ) ;
		}
		PushObject( pObj ) ;
	}
	else
	{
		//
		// 変数参照
		//
		int			nIndex ;
		ESLError	err = eslErrSuccess ;
		//
		switch ( csomType )
		{
		case	csomAuto:
			pObj = NULL ;
			return	ESLErrorMsg( "不正な要素参照です。" ) ;
		case	csomStack:
			pObj = &m_stack ;
			break ;
		case	csomThis:
			pObj = m_stack.GetCurrentThisObject() ;
			if ( pObj == NULL )
			{
				return	ESLErrorMsg( "this オブジェクトが見つかりません。" ) ;
			}
			break ;
		case	csomGlobal:
			pObj = &(m_pcsxi->m_csgGlobal) ;
			break ;
		case	csomData:
			pObj = &(m_pcsxi->m_csgData) ;
			break ;
		case	csomImmediate:
		default:
			return	ESLErrorMsg( "不正な記憶クラスです。" ) ;
		}
		ESLAssert( pObj != NULL ) ;
		if ( csvtType != csvtReference )
		{
			if ( csvtType == csvtInteger )
			{
				int	iElement = *((long int*)(pImage + m_ip)) ;
				m_ip += sizeof(long int) ;
				err = pObj->GetVariableIndex( nIndex, iElement ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else if ( csvtType == csvtString )
			{
				ECSString *	pstrElement = GetStringLiteralObject() ;
				err = pObj->GetVariableIndex( nIndex, pstrElement->m_varStr ) ;
				if ( err )
				{
					EString	strErrMsg = "変数参照：" ;
					strErrMsg += EString(pstrElement->m_varStr) ;
					strErrMsg += "\n" ;
					strErrMsg += GetESLErrorMsg( err ) ;
					m_strErrMsg = strErrMsg ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
			else // if ( csvtType != csvtReference )
			{
				return	ESLErrorMsg( "不正な要素参照です。" ) ;
			}
			pObj = pObj->GetVariableAt( nIndex ) ;
			if ( pObj == NULL )
			{
				return	ESLErrorMsg( "変数が見つかりません。" ) ;
			}
		}
		PushObject( new_CSReference( pObj ) ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : Store 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteStore( void )
{
	ESLError	err ;
	CSOperatorType	csotType = (CSOperatorType) m_pcsxi->m_pImage[m_ip ++] ;
	ECSObject *	pObjSrc = PopObject( ) ;
	if ( pObjSrc != NULL )
	{
		ECSObject *	pObjDst = m_stack.m_varArray.GetLastAt( 0 ) ;
		if ( pObjDst != NULL )
		{
			if ( csotType == (BYTE) csotNop )
			{
				err = pObjDst->Move( *this, pObjSrc ) ;
			}
			else
			{
				err = pObjDst->Operate( *this, csotType, pObjSrc ) ;
				if ( !err && pObjDst->m_pResult )
				{
					pObjSrc = pObjDst->m_pResult ;
					pObjDst->m_pResult = NULL ;
					err = pObjDst->Move( *this, pObjSrc ) ;
				}
			}
		}
		else
		{
			err = ESLErrorMsg
				( "ストア命令にデスティネーションオペランドが見つかりません。" ) ;
		}
		if ( err )
		{
			delete	pObjSrc ;
		}
	}
	else
	{
		err = ESLErrorMsg
			( "ストア命令にソースオペランドが見つかりません。" ) ;
	}
	return	err ;
}

// 実行関数 : Enter 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteEnter( void )
{
	//
	// 名前空間名を取得
	//
	DWORD			i, dwArgCount ;
	ECSWideString *	pwstrFuncName ;
	BYTE *			pImage = m_pcsxi->m_pImage ;
	pwstrFuncName = GetStringLiteral() ;
	dwArgCount = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	//
	// 名前空間を作成し、引数を設定する
	//
	ESLError	err ;
	err = m_stack.CreateNameBlock( pwstrFuncName ) ;
	if ( err )
	{
		return	err ;
	}
	if ( dwArgCount != -1 )
	{
		//
		// 通常のブロック
		//
		ECSWideString *	pwstrType ;
		ECSWideString *	pwstrName ;
		const wchar_t *	pwszType ;
		for ( i = 0; i < dwArgCount; i ++ )
		{
			CSVariableType	csvtType = (CSVariableType) pImage[m_ip ++] ;
			ECSObject *	pObjArg = NULL ;
			pwszType = NULL ;
			if ( csvtType == csvtClassObject )
			{
				DWORD	dwClassIndex = *((DWORD*)(pImage + m_ip)) ;
				m_ip += sizeof(DWORD) ;
				const ECSClassInfo *
					pClassInf = m_pcsxi->GetClassInfoAt( dwClassIndex ) ;
				if ( pClassInf != NULL )
				{
					pObjArg = CreateClassObject( *pClassInf ) ;
				}
				else
				{
					return	ESLErrorMsg( "不正なクラスを指定しています。" ) ;
				}
				pwstrName = GetStringLiteral() ;
			}
			else
			{
				if ( csvtType == csvtObject )
				{
					pwstrType = GetStringLiteral() ;
					pwszType = *pwstrType ;
				}
				pwstrName = GetStringLiteral() ;
				pObjArg = CreateObject( csvtType, pwszType ) ;
			}
			if ( pObjArg == NULL )
			{
				m_strErrMsg = EString(*pwstrFuncName)
					+ "関数の呼び出しで、引数の型が不正です。" ;
				return	ESLErrorMsg(m_strErrMsg) ;
			}
			ECSObject *	pObjSrc = m_arg.m_varArray.GetAt( 0 ) ;
			m_arg.m_varArray.DetachAt( 0 ) ;
			if ( pObjSrc != NULL )
			{
				if ( (i == 0) && (*pwstrName == L"this")
					&& (pObjArg->m_vtType == csvtReference)
						&& (pObjSrc->m_vtType != csvtReference) )
				{
					delete_CSObject( pObjArg ) ;
					pObjArg = pObjSrc ;
				}
				else
				{
					err = MoveObject( pObjArg, pObjSrc ) ;
					if ( err )
					{
						delete	pObjSrc ;
						return	err ;
					}
				}
			}
			err = m_stack.CreateNewVariable( *pwstrName, pObjArg ) ;
			if ( err )
			{
				delete	pObjArg ;
				return	err ;
			}
		}
	}
	else
	{
		if ( pImage[m_ip ++] != 0 )
		{
			return	ESLErrorMsg
				( "定義されていない拡張名前空間命令を実行しようとしました。" ) ;
		}
		//
		// TRY ブロック
		//
		DWORD	dwCatchAddr = *((DWORD*)(pImage + m_ip)) ;
		m_ip += sizeof(DWORD) ;
		dwCatchAddr += m_ip ;
		//
		ECSStack::EStackBlock *	pBlock = m_stack.m_block.GetLastAt( 0 ) ;
		ESLAssert( pBlock != NULL ) ;
		if ( pBlock != NULL )
		{
			pBlock->m_dwFlags |= ECSStack::sfTryBlock ;
			pBlock->m_dwCatchAddr = dwCatchAddr ;
		}
	}
	if ( !(pwstrFuncName->IsEmpty())
		&& (*pwstrFuncName != L"@TRY") && (m_arg.m_varArray.GetSize() > 0) )
	{
		ESLTrace( "関数の引数が全て受け取られませんでした。\n" ) ;
		m_arg.m_varArray.SetSize( 0 ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : Leave 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteLeave( void )
{
	ECSWideString *	pwstrName ;
	ESLError	err =
		m_stack.ReleaseNameBlock( pwstrName, *this ) ;
	m_stack.UpdateCurrentFrame() ;
	return	err ;
}

// 実行関数 : Jump 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteJump( void )
{
	DWORD	dwJumpAddr = *((DWORD*)(m_pcsxi->m_pImage + m_ip)) ;
	m_ip = m_ip + sizeof(DWORD) + dwJumpAddr ;
	return	eslErrSuccess ;
}

// 実行関数 : CJump 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCJump( void )
{
	BYTE	bytConditional = m_pcsxi->m_pImage[m_ip ++] ;
	DWORD	dwJumpAddr = *((DWORD*)(m_pcsxi->m_pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	ECSObject *	pObj = PopObject( ) ;
	ECSObject *	pFlag = ECSObject::GetEntity( pObj ) ;
	if ( pFlag == NULL )
	{
		return	ESLErrorMsg
			( "条件判定命令に判定フラグが指定されていません。" ) ;
	}
	int			nBoolean ;
	ESLError	err = pFlag->OperateBoolean( nBoolean ) ;
	if ( err )
	{
		delete_CSObject( pObj ) ;
		return	err ;
	}
	BYTE	bytPushCondition = bytConditional & 0x02 ;
	bytConditional &= 0x01 ;
	if ( (bytConditional && nBoolean) || (!bytConditional && !nBoolean) )
	{
		m_ip += dwJumpAddr ;
		//
		if ( bytPushCondition )
		{
			PushObject( new_CSInteger( nBoolean ) ) ;
		}
	}
	delete_CSObject( pObj ) ;
	//
	return	eslErrSuccess ;
}

// 実行関数 : Call 命令（バイナリ互換用）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCall( void )
{
	//
	// 関数の情報取得
	//
	DWORD			i, dwArgCount ;
	const wchar_t *	pwszName ;
	BYTE *			pImage = m_pcsxi->m_pImage ;
	CSObjectMode	csomType = (CSObjectMode) pImage[m_ip ++] ;
	dwArgCount = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	//
	pwszName = GetStringLiteral()->CharPtr() ;
	//
	// Global Goto / Throw 命令判定
	//
	bool	fGlobalGoto = false ;
	bool	fThrow = false ;
	if ( pImage[m_ip] == csicReturn )
	{
		if ( pImage[m_ip + 1] == 2 )
		{
			fGlobalGoto = true ;
		}
		else if ( pImage[m_ip + 1] == 3 )
		{
			fThrow = true ;
		}
	}
	//
	// 関数引数設定
	//
	m_arg.m_varArray.SetSize( 0 ) ;
	for ( i = 0; i < dwArgCount; i ++ )
	{
		ECSObject *	pObjArg = PopObject( ) ;
		if ( pObjArg == NULL )
		{
			return	ESLErrorMsg
				( "関数呼出し命令で引数が取得できませんでした。" ) ;
		}
		if ( fGlobalGoto && (pObjArg->m_vtType == csvtReference) )
		{
			ECSObject *	pRef = ((ECSReference*)pObjArg)->m_pRef ;
			if ( pRef->IsValidObject() && IsLocalObject( pRef ) )
			{
				pRef = pRef->Duplicate() ;
				delete_CSObject( pObjArg ) ;
				pObjArg = pRef ;
			}
		}
		m_arg.m_varArray.SetAt( dwArgCount - i - 1, pObjArg ) ;
	}
	//
	// Global Goto 実行
	//
	if ( fGlobalGoto )
	{
		//
		// スタック解放
		//
		ReleaseLocalStackBlock( ) ;
		//
		// 帰りアドレス取得
		//
		ECSObject *	pObjRetAddr = PopObject( ) ;
		if ( pObjRetAddr == NULL )
		{
			delete_CSObject( m_pRetObj ) ;
			m_pRetObj = new_CSInteger() ;
			SetStatus( xsHalt ) ;
			return	eslErrSuccess ;
		}
		if ( pObjRetAddr->m_vtType != csvtInteger )
		{
			delete	pObjRetAddr ;
			return	ESLErrorMsg( "関数の復帰アドレスが不正です。" ) ;
		}
		m_ip = ((ECSInteger*)pObjRetAddr)->GetInt() ;
		delete_CSObject( pObjRetAddr ) ;
	}
	else if ( fThrow )
	{
		//
		// Throw 文処理
		//
		return	ThrowExpression( ) ;
	}
	//
	// this オブジェクトを取得
	//
	ECSObject *	pObjThis = NULL ;
	if ( csomType == csomThis )
	{
		pObjThis = m_arg.m_varArray.GetAt( 0 ) ;
	}
	else if ( csomType == csomAuto )
	{
		pObjThis = m_stack.GetCurrentThisObject() ;
	}
	//
	// メンバ関数を検索
	//
	int			nIndex ;
	if ( pObjThis->IsValidObject() )
	{
		if ( csomType != csomThis )
		{
			m_arg.m_varArray.InsertAt( 0, new_CSReference( pObjThis ) ) ;
		}
		if ( !pObjThis->GetFunction( *this, nIndex, pwszName ) )
		{
			DWORD	dwNextIP = m_ip ;
			if ( csomType == csomThis )
			{
				pObjThis = m_arg.m_varArray.GetAt( 0 ) ;
			}
			ESLError	err =
				pObjThis->CallFunction( *this, nIndex, m_arg.m_varArray ) ;
			if ( err )
			{
				if ( ECSObject::GetEntity( pObjThis ) != NULL )
				{
					pObjThis = ECSObject::GetEntity( pObjThis ) ;
				}
				EString	strErrMsg =
					EString(pObjThis->GetTypeName())
						+ "::" + EString(pwszName)
						+ " の呼び出し\n" + GetESLErrorMsg(err) ;
				m_strErrMsg = strErrMsg ;
				err = ESLErrorMsg(m_strErrMsg) ;
			}
			if ( dwNextIP == m_ip )
			{
				m_arg.m_varArray.RemoveAll( ) ;
			}
			return	err ;
		}
		else if ( csomType == csomThis )
		{
			if ( ECSObject::GetEntity( pObjThis ) != NULL )
			{
				pObjThis = ECSObject::GetEntity( pObjThis ) ;
			}
			try
			{
				m_strErrMsg = pObjThis->GetTypeName() ;
			}
			catch ( ... )
			{
				m_strErrMsg = "<unknow>" ;
			}
			m_strErrMsg	+=
				"::" + EString(pwszName) + " 関数が見つかりません。" ;
			return	ESLErrorMsg(m_strErrMsg) ;
		}
		else
		{
			m_arg.m_varArray.RemoveAt( 0 ) ;
		}
	}
	else if ( csomType == csomThis )
	{
		m_strErrMsg =
			"メンバ関数 " + EString(pwszName)
				+ " の呼び出しで、this オブジェクトが不正です。" ;
		return	ESLErrorMsg(m_strErrMsg) ;
	}
	//
	// グローバル関数を呼び出す
	//
	return	CallGlobalFunction( pwszName ) ;
}

// 実行関数 : GlobalCall 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteExCall( void )
{
	//
	// 関数引数設定
	//
	const wchar_t *	pwszName ;
	BYTE *			pImage = m_pcsxi->m_pImage ;
	//
	DWORD	dwArgCount = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	//
	CSObjectMode	csomType = (CSObjectMode) pImage[m_ip ++] ;
	CSVariableType	csvtType = (CSVariableType) pImage[m_ip ++] ;
	//
	ESLError	err = ExecuteExCallArguments( dwArgCount ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 呼び出し関数取得
	//
	if ( csomType == csomImmediate )
	{
		if ( csvtType == csvtString )
		{
			pwszName = GetStringLiteral()->CharPtr() ;
			return	CallGlobalFunction( pwszName ) ;
		}
		else if ( csvtType == csvtInteger )
		{
			DWORD	dwFuncAddr = *((DWORD*)(pImage + m_ip)) ;
			m_ip += sizeof(DWORD) ;
			//
			PushObject( new_CSInteger( m_ip ) ) ;
			m_ip = dwFuncAddr ;
			MarkCallStackFlag( ) ;
			return	eslErrSuccess ;
		}
		else
		{
			return	ESLErrorMsg( "サポートされない呼び出し関数型です。" ) ;
		}
	}
	return	ESLErrorMsg( "未サポートの Call 命令です。" ) ;
}

// 実行関数 : Return 命令（バイナリ互換用）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteReturn( void )
{
	//
	// 返り値取得
	//
	BYTE	bytFreeStack = m_pcsxi->m_pImage[m_ip ++] ;
	ECSObject *	pReturnObj = NULL ;
	if ( bytFreeStack != 1 )
	{
		pReturnObj = PopObject( ) ;
		if ( pReturnObj == NULL )
		{
			return	ESLErrorMsg( "関数の返り値を取得できませんでした。" ) ;
		}
		if ( pReturnObj->m_vtType == csvtReference )
		{
			if ( ((ECSReference*)pReturnObj)->m_pRef->IsValidObject() )
			{
				ECSObject *	pTemp = pReturnObj ;
				ECSObject *	pRef = ((ECSReference*)pReturnObj)->m_pRef ;
				if ( IsLocalObject( pRef ) )
				{
					pReturnObj = pRef->Duplicate() ;
					delete_CSObject( pTemp ) ;
				}
			}
		}
	}
	//
	// スタック解放
	//
	ReleaseLocalStackBlock( ) ;
	//
	// 命令ポインタ復帰
	//
	ECSObject *	pObjRetAddr = PopObject( ) ;
	if ( pObjRetAddr == NULL )
	{
		delete_CSObject( m_pRetObj ) ;
		m_pRetObj = pReturnObj ;
		SetStatus( xsHalt ) ;
		return	eslErrSuccess ;
	}
	if ( pObjRetAddr->m_vtType != csvtInteger )
	{
		delete	pObjRetAddr ;
		return	ESLErrorMsg( "関数の復帰アドレスが不正です。" ) ;
	}
	if ( ((ECSInteger*)pObjRetAddr)->m_varInt < 0 )
	{
		m_ip = - ((ECSInteger*)pObjRetAddr)->GetInt() ;
		SetStatus( xsHalt ) ;
	}
	else
	{
		m_ip = ((ECSInteger*)pObjRetAddr)->GetInt() ;
	}
	delete_CSObject( pObjRetAddr ) ;
	//
	// 返り値設定
	//
	if ( pReturnObj != NULL )
	{
		PushObject( pReturnObj ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : Return 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteExReturn( void )
{
	BYTE	bytFreeStack = m_pcsxi->m_pImage[m_ip ++] ;
	ECSObject *	pReturnObj = NULL ;
	if ( bytFreeStack != 0 )
	{
		pReturnObj = PopObject( ) ;
		if ( pReturnObj == NULL )
		{
			return	ESLErrorMsg( "関数の返り値を取得できませんでした。" ) ;
		}
	}
	//
	// スタック解放
	//
	ReleaseLocalStackBlock( ) ;
	//
	// 命令ポインタ復帰
	//
	ECSObject *	pObjRetAddr = PopObject( ) ;
	if ( pObjRetAddr == NULL )
	{
		delete_CSObject( m_pRetObj ) ;
		m_pRetObj = pReturnObj ;
		SetStatus( xsHalt ) ;
		return	eslErrSuccess ;
	}
	if ( pObjRetAddr->m_vtType != csvtInteger )
	{
		delete	pObjRetAddr ;
		return	ESLErrorMsg( "関数の復帰アドレスが不正です。" ) ;
	}
	SDWORD	ipRet = (SDWORD) ((ECSInteger*)pObjRetAddr)->GetInt() ;
	if ( ipRet < 0 )
	{
		m_ip = - ipRet ;
		SetStatus( xsHalt ) ;
	}
	else
	{
		m_ip = ipRet ;
	}
	delete_CSObject( pObjRetAddr ) ;
	//
	// 返り値設定
	//
	if ( pReturnObj != NULL )
	{
		PushObject( pReturnObj ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : Element 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteElement( void )
{
	ECSObject *		pObj = PopObject( ) ;
	ECSObject *		pElement = NULL ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg
			( "要素参照命令にオペランドが指定されていません。" ) ;
	}
	ESLError		err ;
	int				nIndex ;
	BYTE *			pImage = m_pcsxi->m_pImage ;
	const wchar_t *	pwszElementName = NULL ;
	CSVariableType	csvtType = (CSVariableType) pImage[m_ip ++] ;
	if ( csvtType == csvtInteger )
	{
		nIndex = *((long int*)(pImage + m_ip)) ;
		m_ip += sizeof(long int) ;
		err = pObj->GetVariableIndex( nIndex, nIndex ) ;
	}
	else if ( csvtType == csvtString )
	{
		ECSString *	pstrElement = GetStringLiteralObject() ;
		err = pObj->GetVariableIndex( nIndex, pstrElement->m_varStr ) ;
	}
	else
	{
		err = ESLErrorMsg( "不正な型による要素参照です。" ) ;
	}
	if ( err )
	{
		if ( csvtType == csvtString )
		{
			EString	strErrMsg = "メンバ参照：" ;
			strErrMsg += EString(pwszElementName) ;
			strErrMsg += "\n" ;
			strErrMsg += GetESLErrorMsg(err) ;
			m_strErrMsg = strErrMsg ;
			err = ESLErrorMsg( m_strErrMsg ) ;
		}
		delete	pObj ;
		return	err ;
	}
	pElement = pObj->GetVariableAt( nIndex ) ;
	if ( pElement == NULL )
	{
		ECSObject *	pEntity = ECSObject::GetEntity( pObj ) ;
		if ( pEntity == NULL )
		{
			m_strErrMsg =
				EString(pObj->GetTypeName()) + " 型オブジェクトの要素 " ;
			if ( pwszElementName == NULL )
			{
				m_strErrMsg += EString(nIndex) ;
			}
			else
			{
				m_strErrMsg += EString(pwszElementName) ;
			}
			m_strErrMsg += " が見つかりませんでした。" ;
			delete	pObj ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		pElement = new_CSReference( ) ;
		((ECSReference*)pElement)->m_pRefParent = pEntity ;
		((ECSReference*)pElement)->m_iParentRef = nIndex ;
	}
	else
	{
		if ( pObj->m_vtType == csvtReference )
		{
			pElement = new_CSReference( pElement ) ;
		}
		else
		{
			pElement = pElement->Duplicate() ;
		}
	}
	delete_CSObject( pObj ) ;
	PushObject( pElement ) ;
	return	eslErrSuccess ;
}

// 実行関数 : ElementIndirect 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteElementIndirect( void )
{
	ECSObject *	pObjIndex = PopObject( ) ;
	ECSObject *	pIndexEntity = ECSObject::GetEntity( pObjIndex ) ;
	if ( pIndexEntity == NULL )
	{
		return	ESLErrorMsg
			( "間接要素参照命令で指標オペランドが指定されていません。" ) ;
	}
	ECSObject *	pObj = PopObject( ) ;
	if ( pObj == NULL )
	{
		delete	pObjIndex ;
		return	ESLErrorMsg
			( "間接要素参照命令にオペランドが指定されていません。" ) ;
	}
	int			nIndex ;
	ESLError	err ;
	if ( pIndexEntity->m_vtType == csvtInteger )
	{
		err = pObj->GetVariableIndex
			( nIndex, ((ECSInteger*)pIndexEntity)->GetInt() ) ;
	}
	else if ( pIndexEntity->m_vtType == csvtString )
	{
		err = pObj->GetVariableIndex
			( nIndex, ((ECSString*)pIndexEntity)->m_varStr ) ;
	}
	else
	{
		INT64	nValue ;
		if ( !pIndexEntity->OperateInteger( nValue ) )
		{
			err = pObj->GetVariableIndex( nIndex, (int) nValue ) ;
		}
		else
		{
			err = ESLErrorMsg
				( "整数でも文字列でもないオブジェクトが指標に使用されています。" ) ;
		}
	}
	if ( err )
	{
		if ( pIndexEntity->m_vtType == csvtString )
		{
			EString	strErrMsg = "間接メンバ参照：" ;
			strErrMsg += EString(((ECSString*)pIndexEntity)->m_varStr) ;
			strErrMsg += "\n" ;
			strErrMsg += GetESLErrorMsg(err) ;
			m_strErrMsg = strErrMsg ;
			err = ESLErrorMsg( m_strErrMsg ) ;
		}
		delete	pObjIndex ;
		delete	pObj ;
		return	err ;
	}
	delete_CSObject( pObjIndex ) ;
	ECSObject *	pElement = pObj->GetVariableAt( nIndex ) ;
	if ( pElement == NULL )
	{
		ECSObject *	pEntity = ECSObject::GetEntity( pObj ) ;
		if ( pEntity == NULL )
		{
			m_strErrMsg =
				EString(pObj->GetTypeName())
					+ " 型オブジェクトに指定された要素が見つかりませんでした。" ;
			delete	pObj ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		pElement = new_CSReference( ) ;
		((ECSReference*)pElement)->m_pRefParent = pEntity ;
		((ECSReference*)pElement)->m_iParentRef = nIndex ;
	}
	else
	{
		if ( pObj->m_vtType == csvtReference )
		{
			pElement = new_CSReference( pElement ) ;
		}
		else
		{
			pElement = pElement->Duplicate() ;
		}
	}
	delete_CSObject( pObj ) ;
	PushObject( pElement ) ;
	return	eslErrSuccess ;
}

// 実行関数 : Operate 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteOperate( void )
{
	CSOperatorType	csotType = (CSOperatorType) m_pcsxi->m_pImage[m_ip ++] ;
	//
	ECSObject *	pObjSrc = PopObject( ) ;
	if ( pObjSrc == NULL )
	{
		return	ESLErrorMsg
			( "演算命令にソースオペランドが指定されていません。" ) ;
	}
	ECSObject *	pObjDst = PopObject( ) ;
	if ( pObjDst == NULL )
	{
		delete	pObjSrc ;
		return	ESLErrorMsg
			( "演算命令にディステーネーションオペランドが指定されていません。" ) ;
	}
	if ( pObjDst->m_vtType == csvtReference )
	{
		ECSObject *	pObjTemp = pObjDst ;
		do
		{
			pObjTemp = ((ECSReference*)pObjTemp)->m_pRef ;
			if ( pObjTemp == NULL )
			{
				delete_CSObject( pObjDst ) ;
				delete_CSObject( pObjSrc ) ;
				return	ESLErrorMsg( "演算命令でオペランドの複製に失敗しました。" ) ;
			}
		}
		while ( pObjTemp->m_vtType == csvtReference ) ;
		//
		switch ( pObjTemp->m_vtType )
		{
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
		default:
			pObjTemp = pObjTemp->Duplicate() ;
			break ;
		case	csvtInteger:
			pObjTemp = new_CSInteger
				( ((ECSInteger*)pObjTemp)->m_varInt ) ;
//				( ((ECSInteger*)pObjTemp)->m_varInt,
//					((ECSInteger*)pObjTemp)->m_varMask ) ;
			break ;
		case	csvtReal:
			pObjTemp = new_CSReal( ((ECSReal*)pObjTemp)->m_varReal ) ;
			break ;
		case	csvtString:
			{
				ECSString *	pTempStr = new_CSString( ) ;
				pTempStr->m_varStr = ((ECSString*)pObjTemp)->m_varStr ;
				pObjTemp = pTempStr ;
			}
			break ;
		}
		delete_CSObject( pObjDst ) ;
		pObjDst = pObjTemp ;
	}
	else if ( pObjDst->m_vtType == csvtPointerReference )
	{
		ECSObject *	pObjTemp = pObjDst->Duplicate() ;
		delete_CSObject( pObjDst ) ;
		pObjDst = pObjTemp ;
	}
	ESLError	err = pObjDst->Operate( *this, csotType, pObjSrc ) ;
	if ( err )
	{
		delete	pObjSrc ;
		delete	pObjDst ;
		return	err ;
	}
	ECSObject *	pObjResult = pObjDst ;
	if ( pObjDst->m_pResult != NULL )
	{
		pObjResult = pObjDst->m_pResult ;
		pObjDst->m_pResult = NULL ;
		delete_CSObject( pObjDst ) ;
	}
	PushObject( pObjResult ) ;
	return	eslErrSuccess ;
}

// 実行関数 : UniOperate 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteUniOperate( void )
{
	CSUnaryOperatorType	csuotType =
		(CSUnaryOperatorType) m_pcsxi->m_pImage[m_ip ++] ;
	//
	ECSObject *	pObjDst = PopObject( ) ;
	if ( pObjDst == NULL )
	{
		return	ESLErrorMsg
			( "演算命令にディステーネーションオペランドが指定されていません。" ) ;
	}
	if ( pObjDst->m_vtType == csvtReference )
	{
		ECSObject *	pObjTemp = pObjDst->Duplicate() ;
		delete_CSObject( pObjDst ) ;
		if ( pObjTemp == NULL )
		{
			return	ESLErrorMsg( "演算命令でオペランドの複製に失敗しました。" ) ;
		}
		pObjDst = pObjTemp ;
	}
	ESLError	err = pObjDst->UnaryOperate( *this, csuotType ) ;
	if ( err )
	{
		delete	pObjDst ;
		return	err ;
	}
	ECSObject *	pObjResult = pObjDst ;
	if ( pObjDst->m_pResult != NULL )
	{
		pObjResult = pObjDst->m_pResult ;
		pObjDst->m_pResult = NULL ;
		delete_CSObject( pObjDst ) ;
	}
	PushObject( pObjResult ) ;
	return	eslErrSuccess ;
}

// 実行関数 : Compare 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCompare( void )
{
	CSCompareType	csctType = (CSCompareType) m_pcsxi->m_pImage[m_ip ++] ;
	//
	ECSObject *	pObjSrc = PopObject( ) ;
	ECSObject *	pObjDst = PopObject( ) ;
	if ( pObjDst == NULL )
	{
		delete_CSObject( pObjSrc ) ;
		return	ESLErrorMsg
			( "比較命令にディステーネーションオペランドが指定されていません。" ) ;
	}
	ECSObject *	pSrcEntity = pObjSrc ;
	if ( csctType < csctPointerComparatorFirst )
	{
		pSrcEntity = ECSObject::GetEntity( pObjSrc ) ;
		if ( pSrcEntity == NULL )
		{
			delete_CSObject( pObjSrc ) ;
			delete_CSObject( pObjDst ) ;
			return	ESLErrorMsg
				( "比較命令にソースオペランドが指定されていません。" ) ;
		}
	}
	else if ( csctType == csctEqualPointer )
	{
		int	fBoolean = 0 ;
		if ( ECSObject::GetEntity( pObjSrc ) == ECSObject::GetEntity( pObjDst ) )
		{
			fBoolean = -1 ;
		}
		delete_CSObject( pObjSrc ) ;
		delete_CSObject( pObjDst ) ;
		PushObject( new_CSInteger( fBoolean ) ) ;
		return	eslErrSuccess ;
	}
	else if ( csctType == csctNotEqualPointer )
	{
		int	fBoolean = 0 ;
		if ( ECSObject::GetEntity( pObjSrc ) != ECSObject::GetEntity( pObjDst ) )
		{
			fBoolean = -1 ;
		}
		delete_CSObject( pObjSrc ) ;
		delete_CSObject( pObjDst ) ;
		PushObject( new_CSInteger( fBoolean ) ) ;
		return	eslErrSuccess ;
	}
	else
	{
		delete_CSObject( pObjSrc ) ;
		delete_CSObject( pObjDst ) ;
		return	ESLErrorMsg( "定義されていない比較演算子です。" ) ;
	}
	int			nResult ;
	ESLError	err = pObjDst->Compare( *this, nResult, csctType, *pSrcEntity ) ;
	delete_CSObject( pObjSrc ) ;
	delete_CSObject( pObjDst ) ;
	if ( err )
	{
		return	err ;
	}
	PushObject( new_CSInteger( nResult ) ) ;
	return	eslErrSuccess ;
}

// 実行関数 : ExOperate 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteExOperate( void )
{
	CSExtraOperatorType	csxotType =
			(CSExtraOperatorType) m_pcsxi->m_pImage[m_ip ++] ;
	ECSObject *	pObjSrc = PopObject( ) ;
	if ( pObjSrc == NULL )
	{
//		delete_CSObject( pObjSrc ) ; 
		return	ESLErrorMsg
			( "特殊演算子にオペランドが指定されていません。" ) ;
	}
	ECSObject *	pObjDst = GetStackTop() ;
	switch ( csxotType )
	{
	case	csxotMoveReference:
		{
			if ( pObjDst->m_vtType == csvtReference )
			{
				ECSReference *	pRefVar = (ECSReference*) pObjDst ;
				if ( pRefVar->m_pRef->IsValidObject()
					&& (pRefVar->m_pRef->m_vtType == csvtReference) )
				{
					pRefVar = (ECSReference*) pRefVar->m_pRef ;
					//
					if ( pObjSrc->m_vtType == csvtReference )
					{
						ECSReference *	pSrcRef = (ECSReference*) pObjSrc ;
						if ( pSrcRef->m_pRef->IsValidObject()
							&& (pSrcRef->m_pRef->m_vtType == csvtReference) )
						{
							pRefVar->SetReferenceCastInterface
								( ((ECSReference*)pSrcRef->m_pRef)->m_pRef,
									this, *((ECSReference*)(pSrcRef->m_pRef)) ) ;
						}
						else
						{
							pRefVar->SetReferenceCastInterface
								( pSrcRef->m_pRef, this, *pSrcRef ) ;
						}
						delete_CSObject( pObjSrc ) ;
					}
					else
					{
						pRefVar->SetOwnObject( pObjSrc, this ) ;
					}
				}
				else
				{
					delete_CSObject( pObjSrc ) ; 
				}
			}
			else
			{
				delete_CSObject( pObjSrc ) ; 
			}
		}
		break ;
	case	csxotArrayDim:
		{
			pObjDst = ECSObject::GetEntity( pObjDst ) ;
			if ( pObjDst == NULL )
			{
				delete_CSObject( pObjSrc ) ; 
				return	ESLErrorMsg
					( "配列を生成するオブジェクトが見つかりませんでした。" ) ;
			}
			if ( pObjDst->m_vtType != csvtArray )
			{
				delete_CSObject( pObjSrc ) ; 
				return	ESLErrorMsg( "配列を生成できませんでした。" ) ;
			}
			ECSArray *	pArray = (ECSArray*) pObjDst ;
			DWORD	dwDimension = *((DWORD*)(m_pcsxi->m_pImage + m_ip)) ;
			m_ip += sizeof(DWORD) ;
			pArray->MakeDimension
				( (const unsigned int *) (m_pcsxi->m_pImage + m_ip),
											(int) dwDimension, pObjSrc ) ;
			m_ip += sizeof(unsigned int) * dwDimension ;
		}
		break ;
	case	csxotHashContainer:
		{
			pObjDst = ECSObject::GetEntity( pObjDst ) ;
			if ( pObjDst == NULL )
			{
				delete_CSObject( pObjSrc ) ; 
				return	ESLErrorMsg
					( "配列を生成するオブジェクトが見つかりませんでした。" ) ;
			}
			if ( pObjDst->m_vtType != csvtHash )
			{
				delete_CSObject( pObjSrc ) ; 
				return	ESLErrorMsg( "Hash コンテナを設定できませんでした。" ) ;
			}
			ECSHash *	pHash = (ECSHash*) pObjDst ;
			pHash->SetDefaultElement( pObjSrc ) ;
		}
		break ;
	default:
		delete_CSObject( pObjSrc ) ; 
		return	ESLErrorMsg
			( "定義されていない特殊演算子です。" ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : ExUniOperate 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteExUniOperate( void )
{
	CSExtraUniOperatorType	csxuotType =
			(CSExtraUniOperatorType) m_pcsxi->m_pImage[m_ip ++] ;
	ECSObject *	pObjSrc = PopObject( ) ;
	if ( pObjSrc == NULL )
	{
		return	ESLErrorMsg
			( "特殊演算子にオペランドが指定されていません。" ) ;
	}
	ESLError	err ;
	switch ( csxuotType )
	{
	case	csxuotDeselect:
		{
			ECSObject *	pObj = NULL ;
			if ( pObjSrc->m_vtType == csvtReference )
			{
				ECSReference *	pRefVar = (ECSReference*) pObjSrc ;
				if ( pRefVar->m_pRef->IsValidObject() )
				{
					switch ( pRefVar->m_pRef->m_vtType )
					{
					case	csvtReference:
						pRefVar = (ECSReference*) pRefVar->m_pRef ;
						pObj = pRefVar->DetachObject( *this ) ;
						break ;
					}
				}
			}
			delete_CSObject( pObjSrc ) ; 
			//
			if ( pObj != NULL )
			{
				PushObject( pObj ) ;
			}
			else
			{
				PushObject( new_CSReference() ) ;
			}
		}
		break ;
	case	csxuotDelete:
		{
			ECSObject *	pObj = ECSObject::GetEntity( pObjSrc ) ;
			if ( (pObj != NULL)
				&& (pObj->m_vtType == csvtPointer) )
			{
				ECotophaScript::LockReference( ) ;
				//
				ECSPointer *	pPtr = (ECSPointer*) pObj ;
				if ( pPtr->m_pOwnObj != NULL )
				{
					delete_CSObject( pPtr->DetachObject( *this ) ) ;
				}
				else
				{
					ECSReference *	pRefOwner = ECSReference::FindReferenceOwner( pPtr->m_pRef ) ;
					if ( pRefOwner != NULL )
					{
						delete_CSObject( pRefOwner->DetachObject( *this ) ) ;
					}
				}
				ECotophaScript::UnlockReference( ) ;
			}
			delete_CSObject( pObjSrc ) ; 
			//
			PushObject( new_CSPointer() ) ;
		}
		break ;
	case	csxuotBoolean:
		{
			int	nBoolean ;
			err = pObjSrc->OperateBoolean( nBoolean ) ;
			if ( err )
			{
				return	err ;
			}
			delete_CSObject( pObjSrc ) ; 
			PushObject( new_CSInteger( nBoolean ) ) ;
		}
		break ;
	case	csxuotSizeOf:
		{
			INT64	nSize ;
			err = pObjSrc->OperateSizeOf( nSize ) ;
			if ( err )
			{
				return	err ;
			}
			delete_CSObject( pObjSrc ) ; 
			PushObject( new_CSInteger( nSize ) ) ;
		}
		break ;
	case	csxuotTypeOf:
		{
			ECSString *	pTypeOf = new_CSString() ;
			pTypeOf->m_varStr = pObjSrc->OperateTypeOf() ;
			delete_CSObject( pObjSrc ) ; 
			PushObject( pTypeOf ) ;
		}
		break ;
	case	csxuotStaticCast:
		{
			ECSReference *	pCastRef ;
			if ( pObjSrc->m_vtType == csvtReference )
			{
				pCastRef = (ECSReference*) pObjSrc ;
			}
			else
			{
				pCastRef = new_CSReference() ;
				pCastRef->SetOwnObject( pObjSrc, this ) ;
			}
			const SDWORD *	pdwImage =
				(const SDWORD *) (m_pcsxi->m_pImage + m_ip) ;
			pCastRef->m_iVarOffset += pdwImage[0] ;
			pCastRef->m_nVarBounds = pdwImage[1] ;
			pCastRef->m_iFuncOffset += pdwImage[2] ;
			m_ip += sizeof(DWORD) * 3 ;
			//
			PushObject( pCastRef ) ;
		}
		break ;
	case	csxuotDynamicCast:
		{
			ECSReference *	pCastRef ;
			if ( pObjSrc->m_vtType == csvtReference )
			{
				pCastRef = (ECSReference*) pObjSrc ;
			}
			else
			{
				pCastRef = new_CSReference() ;
				pCastRef->SetOwnObject( pObjSrc, this ) ;
			}
			ECSWideString *	pstrCastType = GetStringLiteral() ;
			//
			if ( pCastRef->m_pRef != NULL )
			{
				ECS_CAST_INTERFACE	ci ;
				err = pCastRef->m_pRef->
						OperateCastInterface( ci, pstrCastType->CharPtr() ) ;
				if ( err )
				{
					pCastRef->SetReference( NULL, this ) ;
				}
				else if ( ci.pCastObject != pCastRef->m_pRef )
				{
					if ( pCastRef->m_pOwnObj != NULL )
					{
						ECSReference	refTemp( pCastRef->m_pRef ) ;
						pCastRef->SetReferenceCastInterface
							( ci.pCastObject, this, ci ) ;
					}
					else
					{
						pCastRef->SetReferenceCastInterface
							( ci.pCastObject, this, ci ) ;
					}
				}
				else
				{
					pCastRef->m_iVarOffset = ci.iVarOffset ;
					pCastRef->m_nVarBounds = ci.nVarBounds ;
					pCastRef->m_iFuncOffset = ci.iFuncOffset ;
				}
			}
			PushObject( pCastRef ) ;
		}
		break ;
	case	csxuotDuplicate:
		if ( pObjSrc->m_vtType == csvtReference )
		{
			ECSObject *	pEntity = ECSObject::GetEntity( pObjSrc ) ;
			if ( pEntity != NULL )
			{
				switch ( pEntity->m_vtType )
				{
				case	csvtObject:
				case	csvtReference:
				case	csvtArray:
				case	csvtHash:
				default:
					PushObject( pEntity->Duplicate() ) ;
					break ;
				case	csvtInteger64:
				case	csvtInteger:
					PushObject
						( new_CSInteger
							( ((ECSInteger*)pEntity)->m_varInt,
								((ECSInteger*)pEntity)->m_varMask ) ) ;
					break ;
				case	csvtReal:
					PushObject
						( new_CSReal
							( ((ECSReal*)pEntity)->m_varReal ) ) ;
					break ;
				case	csvtString:
					{
						ECSString *	pTempStr = new_CSString( ) ;
						pTempStr->m_varStr = ((ECSString*)pEntity)->m_varStr ;
						PushObject( pTempStr ) ;
					}
					break ;
				case	csvtPointer:
					PushObject( new_CSPointer( *((ECSPointer*)pEntity) ) ) ;
					break ;
				}
				delete_CSObject( pObjSrc ) ;
			}
			else
			{
				PushObject( pObjSrc ) ;
			}
		}
		else
		{
			PushObject( pObjSrc->Duplicate() ) ;
			delete_CSObject( pObjSrc ) ;
		}
		break ;
	default:
		delete_CSObject( pObjSrc ) ; 
		return	ESLErrorMsg
			( "定義されていない特殊演算子です。" ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : CallMember 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCallMember( void )
{
	BYTE *	pImage = m_pcsxi->m_pImage ;
	DWORD	dwArgCount = *((DWORD*)(pImage + m_ip)) ;
	DWORD	dwClassIndex = *((DWORD*)(pImage + m_ip + sizeof(DWORD))) ;
	DWORD	dwFuncIndex = *((DWORD*)(pImage + m_ip + sizeof(DWORD) * 2)) ;
	m_ip += sizeof(DWORD) * 3 ;
	//
	// 引数取得
	//
	m_arg.m_varArray.SetSize( 0 ) ;
	m_arg.m_varArray.SetLimit( dwArgCount ) ;
	//
	DWORD	i ;
	for ( i = 0; i < dwArgCount; i ++ )
	{
		ECSObject *	pObjArg = PopObject( ) ;
		if ( pObjArg == NULL )
		{
			return	ESLErrorMsg
				( "関数呼出し命令で引数が取得できませんでした。" ) ;
		}
		m_arg.m_varArray.SetAt( dwArgCount - i - 1, pObjArg ) ;
	}
	ECSObject *	pObjThis = m_arg.m_varArray.GetAt( 0 ) ;
	ESLAssert( pObjThis != NULL ) ;
	//
	// this ポインタ取得
	//
	ECS_FUNCTION_POINTER	fptr ;
	ESLError	err =
		pObjThis->GetFunctionPointer( *this, fptr, (int) dwFuncIndex ) ;
	if ( err )
	{
		return	err ;
	}
	ECSReference *	pRefThis = NULL ;
	if ( pObjThis->m_vtType == csvtReference )
	{
		pRefThis = (ECSReference*) pObjThis ;
		if ( pRefThis->m_pRef != fptr.m_castThis.pCastObject )
		{
			pRefThis->SetReference( fptr.m_castThis.pCastObject, this ) ;
		}
	}
	else
	{
		pRefThis = new_CSReference( fptr.m_castThis.pCastObject ) ;
		m_arg.m_varArray.SetAt( 0, pRefThis ) ;
	}
	pRefThis->m_iVarOffset = fptr.m_castThis.iVarOffset ;
	pRefThis->m_nVarBounds = fptr.m_castThis.nVarBounds ;
	pRefThis->m_iFuncOffset = fptr.m_castThis.iFuncOffset ;
	//
	// 関数呼び出し
	//
	if ( fptr.m_ftType == ECS_FUNCTION_POINTER::funcScriptCall )
	{
		PushObject( new_CSInteger( m_ip ) ) ;
		m_ip = fptr.m_varFunc.addrScript ;
		MarkCallStackFlag( ) ;
	}
	else
	{
		switch ( fptr.m_ftType )
		{
		case	ECS_FUNCTION_POINTER::funcIndexCall:
			return	fptr.m_castThis.pCastObject->CallFunction
					( *this, fptr.m_varFunc.nIndex, m_arg.m_varArray ) ;

		case	ECS_FUNCTION_POINTER::funcScriptCall:
			PushObject( new_CSInteger( m_ip ) ) ;
			m_ip = fptr.m_varFunc.addrScript ;
			MarkCallStackFlag( ) ;
			return	eslErrSuccess;

		case	ECS_FUNCTION_POINTER::funcNativeCall:
			return	((fptr.m_castThis.pCastObject)->*
				(fptr.m_varFunc.pfnNative))
						( *this, fptr.m_varFunc.nIndex, m_arg.m_varArray ) ;

		case	ECS_FUNCTION_POINTER::funcNakedCall:
			return	CallFunctionWin32x86API
						( fptr.m_varFunc.pfnNaked, m_arg.m_varArray ) ;
		}
	}
	return	eslErrSuccess ;
}

// 実行関数 : CallNativeMember 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCallNativeMember( void )
{
	BYTE *	pImage = m_pcsxi->m_pImage ;
	DWORD	dwArgCount = *((DWORD*)(pImage + m_ip)) ;
	DWORD	dwClassIndex = *((DWORD*)(pImage + m_ip + sizeof(DWORD))) ;
	DWORD	dwFuncIndex = *((DWORD*)(pImage + m_ip + sizeof(DWORD) * 2)) ;
	m_ip += sizeof(DWORD) * 3 ;
	//
	// 引数取得
	//
	m_arg.m_varArray.SetSize( 0 ) ;
	m_arg.m_varArray.SetLimit( dwArgCount ) ;
	//
	DWORD	i ;
	for ( i = 0; i < dwArgCount; i ++ )
	{
		ECSObject *	pObjArg = PopObject( ) ;
		if ( pObjArg == NULL )
		{
			return	ESLErrorMsg
				( "関数呼出し命令で引数が取得できませんでした。" ) ;
		}
		m_arg.m_varArray.SetAt( dwArgCount - i - 1, pObjArg ) ;
	}
	ECSObject *	pObjThis = m_arg.m_varArray.GetAt( 0 ) ;
	ESLAssert( pObjThis != NULL ) ;
	//
	ECSObject *	pEntity = ECSObject::GetEntity( pObjThis ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg
			( "メンバ関数の呼び出しに this オブジェクトが見つかりません。" ) ;
	}
	//
	// クラス情報取得
	//
	const ECSClassInfo *	pClassInf = pEntity->m_pClassInf ;
	if ( pClassInf == NULL )
	{
		pClassInf = m_pcsxi->GetClassInfoAt( dwClassIndex ) ;
		if ( pClassInf == NULL )
		{
			return	ESLErrorMsg
				( "メンバ関数の呼び出しにクラス情報が見つかりません。" ) ;
		}
		const wchar_t *	pwszThisTypeName = pEntity->GetTypeName() ;
		if ( pClassInf->GetGlobalName() != pwszThisTypeName )
		{
			const ECSClassInfo *	pObjClassInf
					= m_pcsxi->GetClassInfoAs( pwszThisTypeName ) ;
			if ( pObjClassInf == NULL )
			{
				return	ESLErrorMsg
					( "メンバ関数の呼び出しに"
						"派生クラス情報が見つかりません。" ) ;
			}
			if ( pObjClassInf->GetCastParentClassAs
							( pClassInf->GetGlobalName() ) == NULL )
			{
				return	ESLErrorMsg
					( "メンバ関数の呼び出しで"
						"オブジェクトが派生クラスではありません。" ) ;
			}
			pClassInf = pObjClassInf ;
		}
		pEntity->m_pClassInf = pClassInf ;
	}
	//
	// 関数情報取得
	//
	ECSClassInfo::MemberFunction *
		pFunc = pClassInf->GetFunctionAt( dwFuncIndex ) ;
	if ( pFunc == NULL )
	{
		return	ESLErrorMsg( "関数情報が見つかりませんでした。" ) ;
	}
	if ( pFunc->m_fpFuncPointer.m_ftType
				== ECS_FUNCTION_POINTER::funcIndexCall )
	{
		return	pEntity->CallFunction
			( *this, pFunc->m_fpFuncPointer.m_varFunc.nIndex,
												m_arg.m_varArray ) ;
	}
	else if ( pFunc->m_fpFuncPointer.m_ftType
						== ECS_FUNCTION_POINTER::funcNativeCall )
	{
		return	(pEntity->*(pFunc->m_fpFuncPointer.m_varFunc.pfnNative))
			( *this, pFunc->m_fpFuncPointer.m_varFunc.nIndex,
													m_arg.m_varArray ) ;
	}
	else
	{
		return	ESLErrorMsg( "不正なネイティブコールです。" ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : Swap 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteSwap( void )
{
	const BYTE *	pImage = m_pcsxi->m_pImage ;
	BYTE			bytSubCode = pImage[m_ip] ;
	m_ip ++ ;
	//
	DWORD	dwIndex1 = *((DWORD*)(pImage + m_ip)) ;
	DWORD	dwIndex2 = *((DWORD*)(pImage + m_ip + sizeof(DWORD))) ;
	m_ip += sizeof(DWORD) * 2 ;
	//
	ESLError	err = m_stack.SwapLast( (int) dwIndex1, (int) dwIndex2 ) ;
	if ( err )
	{
		return	ESLErrorMsg( "swap 命令が失敗しました。" ) ;
	}
	//
	return	eslErrSuccess ;
}

// 実行関数 : CreateBuffer 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCreateBuffer( void )
{
	const BYTE *	pImage = m_pcsxi->m_pImage ;
	DWORD	dwSize = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	//
	ECSBuffer *	pBuf = new ECSBuffer ;
	m_stack.PushObject( pBuf ) ;
	//
	if ( pBuf->CreateBuffer( dwSize ) )
	{
		return	ESLErrorMsg( "naked バッファの生成に失敗しました" ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : CreateBufferVSize 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCreateBufferVSize( void )
{
	ECSObject *	pObjSize = PopObject() ;
	if ( pObjSize == NULL )
	{
		return	ESLErrorMsg( "生成するバッファサイズが指定されていません" ) ;
	}
	INT64	nSize ;
	if ( pObjSize->OperateInteger( nSize ) )
	{
		return	ESLErrorMsg( "生成するバッファサイズを取得できません" ) ;
	}
	delete_CSObject( pObjSize ) ;
	//
	ECSBuffer *	pBuf = new ECSBuffer ;
	m_stack.PushObject( pBuf ) ;
	//
	if ( pBuf->CreateBuffer( (int) nSize ) )
	{
		return	ESLErrorMsg( "naked バッファの生成に失敗しました" ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : PointerToObject 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecutePointerToObject( void )
{
	const BYTE *	pImage = m_pcsxi->m_pImage ;
	DWORD	dwOffset = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	//
	ECSPointer *	pPtr = new_CSPointer() ;
	ECSObject *		pObj = PopObject() ;
	if ( pObj != NULL )
	{
		if ( pObj->m_vtType == csvtReference )
		{
			ECSReference *	pRef = (ECSReference*) pObj ;
			pPtr->SetReferenceCastInterface( pRef->m_pRef, this, *pRef ) ;
			pPtr->m_iOffset = dwOffset ;
			delete_CSObject( pObj ) ;
		}
		else if ( pObj->m_vtType == csvtPointerReference )
		{
			ECSPointerReference *	pRefPtr = (ECSPointerReference*) pObj ;
			pPtr->SetReferenceCastInterface( pRefPtr->m_pRef, this, *pRefPtr ) ;
			pPtr->m_iOffset = pRefPtr->m_iOffset + dwOffset ;
			delete_CSObject( pObj ) ;
		}
		else
		{
			pPtr->SetOwnObject( pObj, this ) ;
			pPtr->m_iOffset = dwOffset ;
		}
	}
	//
	m_stack.PushObject( pPtr ) ;
	//
	return	eslErrSuccess ;
}

// 実行関数 : PointerToAddress 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecutePointerToAddress( void )
{
	ECSObject *	pAddr = PopObject() ;
	INT64	nAddr ;
	if ( (pAddr == NULL) || pAddr->OperateInteger( nAddr ) )
	{
		if ( pAddr != NULL )
		{
			delete_CSObject( pAddr ) ;
		}
		return	ESLErrorMsg( "リニアアドレスが取得できません" ) ;
	}
	delete_CSObject( pAddr ) ;
	//
	int			iOffset ;
	ECSObject *	pRefObj ;
	ECotophaScript::Lock() ;
	pRefObj =
		ESLTypeCast<ECSObject>
			( GetObjectFromLinearAddress( nAddr, iOffset ) ) ;
	ECotophaScript::Unlock() ;
	//
	m_stack.PushObject( new_CSPointer( pRefObj, iOffset ) ) ;
	//
	return	eslErrSuccess ;
}

// 実行関数 : ReferenceForPointer 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteReferenceForPointer( void )
{
	const BYTE *	pImage = m_pcsxi->m_pImage ;
	CSVariableType	csvtRefType = (CSVariableType) *((BYTE*)(pImage + m_ip)) ;
	m_ip ++ ;
	//
	ECSObject *	pPtrObj = PopObject() ;
	ECSObject *	pEntity = ECSObject::GetEntity( pPtrObj ) ;
	//
	if ( pEntity == NULL )
	{
		delete_CSObject( pPtrObj ) ;
		m_stack.PushObject( new_CSReference() ) ;
		return	eslErrSuccess ;
	}
	if ( pEntity->m_vtType == csvtPointer )
	{
		ECSPointer *	pPtr = (ECSPointer*) pEntity ;
		ECSReference *	pRef ;
		if ( (pPtr->m_iOffset == 0) & (csvtRefType == csvtObject) )
		{
			pRef = new_CSReference( pPtr->m_pRef ) ;
			m_stack.PushObject( pRef ) ;
		}
		else
		{
			pRef = new_CSPointerReference
					( pPtr->m_pRef, pPtr->m_iOffset, csvtRefType ) ;
			m_stack.PushObject( pRef ) ;
		}
		delete_CSObject( pPtrObj ) ;
	}
	else
	{
		m_stack.PushObject( pPtrObj ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : ReferenceForObjPointer 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteReferenceForObjPointer( void )
{
	ECSObject *	pPtrObj = PopObject() ;
	ECSObject *	pEntity = ECSObject::GetEntity( pPtrObj ) ;
	if ( pEntity == NULL )
	{
		delete_CSObject( pPtrObj ) ;
		m_stack.PushObject( new_CSReference() ) ;
		return	eslErrSuccess ;
	}
	if ( pEntity->m_vtType == csvtPointer )
	{
		ECSPointer *	pPtr = (ECSPointer*) pEntity ;
		ECSReference *	pRef = new_CSReference() ;
		pRef->SetReferenceCastInterface( pPtr->m_pRef, this, *pPtr ) ;
		m_stack.PushObject( pRef ) ;
		delete_CSObject( pPtrObj ) ;
	}
	else
	{
		m_stack.PushObject( pPtrObj ) ;
	}
	return	eslErrSuccess ;
}

// 実行関数 : CallFunctionPointer 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCallFunctionPointer( void )
{
	//
	// 呼び出し関数取得
	//
	ECSObject *	pCallFunc = PopObject( ) ;
	ECSObject *	pEntity = ECSObject::GetEntity( pCallFunc ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "間接関数呼び出しで関数ポインタが見つかりません" ) ;
	}
	//
	// 関数引数設定
	//
	const BYTE *	pImage = m_pcsxi->m_pImage ;
	//
	DWORD	dwArgCount = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	//
	ESLError	err = ExecuteExCallArguments( dwArgCount ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 呼び出し関数取得
	//
	if ( pEntity->m_vtType == csvtString )
	{
		err = CallGlobalFunction( ((ECSString*)pEntity)->m_varStr ) ;
	}
	else
	{
		INT64	nValue ;
		err = pEntity->OperateInteger( nValue ) ;
		if ( err )
		{
			return	err ;
		}
		if ( (DWORD)(nValue >> 32) != (ECSExecutionImage::roasCode << 24) )
		{
			return	ESLErrorMsg( "呼び出し関数アドレスが不正です" ) ;
		}
		PushObject( new_CSInteger( m_ip ) ) ;
		m_ip = (DWORD) nValue ;
		MarkCallStackFlag( ) ;
	}
	delete_CSObject( pCallFunc ) ;
	//
	return	err ;
}

// 実行関数 : CallNativeFunction 命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteCallNativeFunction( void )
{
	//
	// 関数引数設定
	//
	const BYTE *	pImage = m_pcsxi->m_pImage ;
	DWORD	dwArgCount = *((DWORD*)(pImage + m_ip)) ;
	DWORD	dwFuncIndex = *((DWORD*)(pImage + m_ip + sizeof(DWORD))) ;
	m_ip += sizeof(DWORD) * 2 ;
	//
	ESLError	err = ExecuteExCallArguments( dwArgCount ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 呼び出しゲート取得
	//
	NativeCallGate *	pGate = m_lstNativeCallGate.GetAt( dwFuncIndex ) ;
	if ( pGate != NULL )
	{
		return	pGate->Call( *this, m_arg.m_varArray ) ;
	}
	//
	// 呼び出し関数取得
	//
	const wchar_t *	pwszFuncName =
			m_pcsxi->m_staNativeFuncName.GetAt( dwFuncIndex ) ;
	if ( pwszFuncName == NULL )
	{
		return	ESLErrorMsg( "ネイティブ関数の呼び出しが不正です" ) ;
	}
	int	iFunc = m_staFuncName->FindIndex( pwszFuncName ) ;
	if ( iFunc >= 0 )
	{
		pGate = new NativeCotophaCallGate( m_pfnCallFunc[iFunc] ) ;
		m_lstNativeCallGate.SetAt( dwFuncIndex, pGate ) ;
		return	pGate->Call( *this, m_arg.m_varArray ) ;
	}
	API_ECS_FUNC	apiCallFunc = GetImportNativeFunction( pwszFuncName ) ;
	if ( apiCallFunc != NULL )
	{
		pGate = new NativePluginCallGate( apiCallFunc ) ;
		m_lstNativeCallGate.SetAt( dwFuncIndex, pGate ) ;
		return	pGate->Call( *this, m_arg.m_varArray ) ;
	}
	FARPROC	pfnProc = FindPluginedFunction( EString( pwszFuncName ) ) ;
	if ( pfnProc != NULL )
	{
		pGate = new NativeStdcallCallGate( pfnProc ) ;
		m_lstNativeCallGate.SetAt( dwFuncIndex, pGate ) ;
		return	pGate->Call( *this, m_arg.m_varArray ) ;
	}
	return	ESLErrorMsg( "ネイティブ関数が見つかりません" ) ;
}

// 静的文字列バッファポインタを取得する
//////////////////////////////////////////////////////////////////////////////
ECSWideString * ECSContext::GetConstantString( const wchar_t * pwszString )
{
	ECSString *	pstrConst = m_wstaConstStrBuf.GetAs( pwszString ) ;
	if ( pstrConst == NULL )
	{
		pstrConst = new ECSString( pwszString ) ;
		m_wstaConstStrBuf.SetAs( pwszString, pstrConst ) ;
	}
	return	&(pstrConst->m_varStr) ;
}

// 現在の命令ポインタから文字列リテラルを取得してポインタを進める
// （解放する必要のない静的なバッファを返す）
//////////////////////////////////////////////////////////////////////////////
ECSString * ECSContext::GetStringLiteralObject( void )
{
	DWORD		dwLength ;
	BYTE *		pImage = m_pcsxi->m_pImage ;
	ECSString *	pConstStr ;
	dwLength = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	if ( dwLength != 0x80000000 )
	{
		ESLTrace( "Warning : occured temporary "
				"string literal on cotopha script.\n" ) ;
		ECSWideString	wstrBuf ;
		if ( dwLength != 0 )
		{
			::eslMoveMemory( wstrBuf.GetBuffer(dwLength),
					pImage + m_ip, dwLength * sizeof(wchar_t) ) ;
			wstrBuf.ReleaseBuffer( dwLength ) ;
			m_ip += dwLength * sizeof(wchar_t) ;
		}
		else
		{
			wstrBuf = L"" ;
		}
		pConstStr = m_wstaConstStrBuf.GetAs( wstrBuf ) ;
		if ( pConstStr != NULL )
		{
			return	pConstStr ;
		}
		pConstStr = new ECSString( wstrBuf ) ;
		m_wstaConstStrBuf.SetAs( wstrBuf, pConstStr ) ;
		return	pConstStr ;
	}
	else
	{
		DWORD	dwStrIndex = *((DWORD*)(pImage + m_ip)) ;
		m_ip += sizeof(DWORD) ;
		//
		pConstStr = m_pcsxi->m_lstConstStr.GetAt( dwStrIndex ) ;
		if ( pConstStr != NULL )
		{
			return	pConstStr ;
		}
		pConstStr = m_wstaConstStrBuf.GetAs( L"" ) ;
		if ( pConstStr != NULL )
		{
			return	pConstStr ;
		}
		pConstStr = new ECSString( L"" ) ;
		m_wstaConstStrBuf.SetAs( L"", pConstStr ) ;
		return	pConstStr ;
	}
}

ECSWideString * ECSContext::GetStringLiteral( void )
{
	ECSString *	pstrObj = GetStringLiteralObject() ;
	return	&(pstrObj->m_varStr) ;
}

// 現在の命令ポインタから文字列リテラルを取得してポインタを進める
//////////////////////////////////////////////////////////////////////////////
void ECSContext::LoadStringLiteral( ECSSourceStream & wstrBuf )
{
	DWORD	dwLength ;
	BYTE *	pImage = m_pcsxi->m_pImage ;
	dwLength = *((DWORD*)(pImage + m_ip)) ;
	m_ip += sizeof(DWORD) ;
	if ( dwLength != 0x80000000 )
	{
		if ( dwLength != 0 )
		{
			::eslMoveMemory( wstrBuf.GetBuffer(dwLength),
					pImage + m_ip, dwLength * sizeof(wchar_t) ) ;
			wstrBuf.ReleaseBuffer( dwLength ) ;
			m_ip += dwLength * sizeof(wchar_t) ;
		}
		else if ( !wstrBuf.IsEmpty() )
		{
			wstrBuf = L"" ;
		}
	}
	else
	{
		DWORD	dwStrIndex = *((DWORD*)(pImage + m_ip)) ;
		m_ip += sizeof(DWORD) ;
		ECSString *	pConstStr = m_pcsxi->m_lstConstStr.GetAt( dwStrIndex ) ;
		if ( pConstStr != NULL )
		{
			wstrBuf = pConstStr->m_varStr ;
		}
		else
		{
			wstrBuf = L"" ;
		}
	}
}

// 関数の引数を object スタックから引数配列へセット
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ExecuteExCallArguments( int nArgCount )
{
	m_arg.m_varArray.SetSize( 0 ) ;
	for ( int i = 0; i < nArgCount; i ++ )
	{
		ECSObject *	pObjArg = PopObject( ) ;
		if ( pObjArg == NULL )
		{
			return	ESLErrorMsg
				( "関数呼出し命令で引数が取得できませんでした。" ) ;
		}
		m_arg.m_varArray.SetAt( nArgCount - i - 1, pObjArg ) ;
	}
	return	eslErrSuccess ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSContext::GetThisObject( void )
{
	int	nIndex ;
	if ( m_stack.GetVariableIndex( nIndex, L"this" /*m_strThis*/ ) )
	{
		return	NULL ;
	}
	return	ECSObject::GetEntity( m_stack.GetVariableAt(nIndex) ) ;
}

// 関数呼び出し名前空間をマークする
//////////////////////////////////////////////////////////////////////////////
void ECSContext::MarkCallStackFlag( void )
{
	ECSStack::EStackBlock *
		pBlock = m_stack.m_block.GetLastAt( 0 ) ;
	if ( pBlock != NULL )
	{
		pBlock->m_dwFlags |= ECSStack::sfCallBlock ;
	}
}

// 現在のスタック上のローカルな無名名前空間を削除
//////////////////////////////////////////////////////////////////////////////
void ECSContext::ReleaseLocalStackBlock( void )
{
	ECSWideString *	pwstrName ;
	for ( ; ; )
	{
		ECSStack::EStackBlock *	pBlock = m_stack.m_block.GetLastAt( 0 ) ;
		if ( pBlock == NULL )
		{
			break ;
		}
		if ( pBlock->m_dwFlags & ECSStack::sfCallBlock )
		{
			pBlock->m_dwFlags &= ~ECSStack::sfCallBlock ;
			break ;
		}
		m_stack.ReleaseNameBlock( pwstrName, *this ) ;
	}
	m_stack.UpdateCurrentFrame() ;
}

// 指定オブジェクトが関数のローカルな変数か判定
//////////////////////////////////////////////////////////////////////////////
bool ECSContext::IsLocalObject( ECSObject * pObj )
{
	int	i, j = 0 ;
	for ( i = 0; i < (int) m_stack.m_block.GetSize(); i ++ )
	{
		ECSStack::EStackBlock *
			pBlock = m_stack.m_block.GetLastAt( i ) ;
		if ( pBlock && pBlock->m_pwstrName
				&& !pBlock->m_pwstrName->IsEmpty() )
		{
			j = pBlock->m_iBound ;
			break ;
		}
	}
	for ( i = j; i < (int) m_stack.m_varArray.GetSize(); i ++ )
	{
		if ( m_stack.m_varArray.GetAt(i) == pObj )
		{
			return	true ;
		}
	}
	return	false ;
}

// 例外を発行する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::ThrowExpression( void )
{
	unsigned int	n ;
	for ( n = 0; n < m_stack.m_block.GetSize(); n ++ )
	{
		ECSStack::EStackBlock *	pBlock = m_stack.m_block.GetLastAt( n ) ;
		if ( pBlock == NULL )
		{
			continue ;
		}
		if ( pBlock->m_dwFlags & ECSStack::sfTryBlock )
		{
			ECSWideString *	pwstrName ;
			for ( unsigned int i = 0; i < n; i ++ )
			{
				m_stack.ReleaseNameBlock( pwstrName, *this ) ;
			}
			m_stack.CleanupNameBlock( *this ) ;
			m_stack.UpdateCurrentFrame() ;
			//
			ESLAssert( pBlock == m_stack.m_block.GetLastAt(0) ) ;
			pBlock->m_dwFlags &= ~ECSStack::sfTryBlock ;
			if ( pBlock->m_dwFlags & ECSStack::sfCallBlock )
			{
				pBlock->m_dwFlags &= ~ECSStack::sfCallBlock ;
//				delete	m_stack.PopObject() ;
			}
			m_ip = pBlock->m_dwCatchAddr ;
			pBlock->m_dwCatchAddr = 0 ;
			return	eslErrSuccess ;
		}
	}
	return	ESLErrorMsg( "処理されない例外が投げられました。" ) ;
}

// オブジェクトを破棄する
//////////////////////////////////////////////////////////////////////////////
void ECSContext::delete_CSObject( ECSObject * pObj )
{
	if ( pObj == NULL )
	{
		return ;
	}
	CSVariableType csvtType = pObj->m_vtType ;
	pObj->OnDestruction( *this ) ;
	switch ( csvtType )
	{
	case	csvtReference:
		m_tsbufReference.Delete( (ECSReference*) pObj ) ;
		break ;
	case	csvtInteger:
		m_tsbufInteger.Delete( (ECSInteger*) pObj ) ;
		break ;
	case	csvtReal:
		m_tsbufReal.Delete( (ECSReal*) pObj ) ;
		break ;
	case	csvtString:
		m_tsbufString.Delete( (ECSString*) pObj ) ;
		break ;
	case	csvtPointer:
		m_tsbufPointer.Delete( (ECSPointer*) pObj ) ;
		break ;
	case	csvtPointerReference:
		m_tsbufPointerRef.Delete( (ECSPointerReference*) pObj ) ;
		break ;
	case	csvtInteger64:
//	case	csvtFunction:
	case	csvtObject:
	case	csvtArray:
	case	csvtHash:
	case	csvtClassObject:
	case	csvtBoolean:
	case	csvtInt8:
	case	csvtUint8:
	case	csvtInt16:
	case	csvtUint16:
	case	csvtInt32:
	case	csvtUint32:
	case	csvtArrayDimension:
	case	csvtHashContainer:
	case	csvtReal32:
	case	csvtReal64:
	default:
		pObj->CleanupAllReference( *this ) ;
		delete	pObj ;
		break ;
	}
}

// native 関数ゲート
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::NativeCotophaCallGate::Call
			( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	(context.*m_pfnCall)( lstArg ) ;
}

ESLError ECSContext::NativePluginCallGate::Call
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	m_pfnCall( context, lstArg ) ;
}

ESLError ECSContext::NativeStdcallCallGate::Call
			( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	context.CallFunctionWin32x86API( m_pfnCall, lstArg ) ;
}

// スクリプト関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSContext::m_staFuncName = NULL ;
const wchar_t *	ECSContext::m_pwszFuncName[14] =
{
	L"GetSystemPerformance", L"SetSystemPerformance",
	L"GetCurrentTime", L"GetLocalTime", L"GetMemoryStatus",
	L"AddModule", L"OpenToAddArchiveFile", L"EnableArchiveFilePath",
	L"Sleep", L"Suspend", L"Exit",
	L"Trace", L"memmove",
	NULL
} ;
const ECSContext::PFUNC_CALL	ECSContext::m_pfnCallFunc[13] =
{
	&ECSContext::Call_GetSystemPerformance,
	&ECSContext::Call_SetSystemPerformance,
	&ECSContext::Call_GetCurrentTime,
	&ECSContext::Call_GetLocalTime,
	&ECSContext::Call_GetMemoryStatus,
	&ECSContext::Call_AddModule,
	&ECSContext::Call_OpenToAddArchiveFile,
	&ECSContext::Call_EnableArchiveFilePath,
	&ECSContext::Call_Sleep,
	&ECSContext::Call_Suspend,
	&ECSContext::Call_Exit,
	&ECSContext::Call_Trace,
	&ECSContext::Call_memmove,
} ;

// 関数 : Integer GetSystemPerformance( Integer nType := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_GetSystemPerformance( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = VerifyArgumentCount( lstArg, 0, 1 ) ;
	if ( err )
		return	err ;
	//
	int	nType ;
	err = GetArgumentAsInt( nType, lstArg, 0, 0 ) ;
	if ( err )
		return	err ;
	//
	long int	nValue = 0 ;
	ECSEnvironment *	pEnv ;
	switch ( nType )
	{
	case	0:	// CPU 種別
		nValue = ::glsGetEnabledProcessorType( ) ;
		break ;
	case	1:	// フォントアンチエイリアシング
		nValue = ERealFontImage::IsEnabledFontSmoothing() ? -1 : 0 ;
		break ;
	case	2:	// ファイルパス制限
		pEnv = GetEnvironment() ;
		nValue = pEnv ? pEnv->m_fAcceptOtherSaveDir : false ;
		break ;
	case	3:	// 論理プロセッサ数
		nValue = ESLThread::GetLogicalProcessorCount() ;
		break ;
	}
	return	PushObject( new_CSInteger( nValue ) ) ;
}

// 関数 : Integer SetSystemPerformance( Integer nType, Integer nFlags )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_SetSystemPerformance( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nType, nFlags ;
	err = GetArgumentAsInt( nType, lstArg, 0, 0 ) ;
	if ( err )
		return	err ;
	err = GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ECSEnvironment *	pEnv ;
	switch ( nType )
	{
	case	0:	// CPU 種別
		::glsEnableProcessorType( 0 ) ;
		nFlags &= ::glsGetEnabledProcessorType( ) ;
		if ( nFlags != 0 )
		{
			::glsEnableProcessorType( nFlags ) ;
		}
		else
		{
			::glsDisableProcessorType( -1 ) ;
		}
		break ;
	case	1:	// フォントアンチエイリアシング
		ERealFontImage::EnableFontSmoothing( nFlags != 0 ) ;
		break ;
	case	2:	// ファイルパス制限
		pEnv = GetEnvironment() ;
		if ( pEnv != NULL )
		{
			pEnv->m_fAcceptOtherSaveDir = (nFlags != 0) ;
		}
		break ;
	}
	return	PushObject( new_CSInteger( nFlags ) ) ;
}

// 関数 : Integer GetCurrentTime()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_GetCurrentTime( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = VerifyArgumentCount( lstArg, 0 ) ;
	if ( err )
		return	err ;
	//
	DWORD	dwCurrentTime = ::timeGetTime() ;
	m_nLastTickTime += (DWORD) (dwCurrentTime - m_dwLastBaseTime) ;
	m_dwLastBaseTime = dwCurrentTime ;
	return	PushObject( new_CSInteger( m_nLastTickTime ) ) ;
}

// 関数 : Time GetLocalTime()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_GetLocalTime( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = VerifyArgumentCount( lstArg, 0 ) ;
	if ( err )
		return	err ;
	//
	SYSTEMTIME	stLocal ;
	::GetLocalTime( &stLocal ) ;
	//
	ECSStructureInterface *	pTime = CreateUserStructure( L"Time" ) ;
	pTime->SetMemberAsInt( L"nYear", stLocal.wYear ) ;
	pTime->SetMemberAsInt( L"nMonth", stLocal.wMonth ) ;
	pTime->SetMemberAsInt( L"nDay", stLocal.wDay ) ;
	pTime->SetMemberAsInt( L"nWeek", stLocal.wDayOfWeek ) ;
	pTime->SetMemberAsInt( L"nHour", stLocal.wHour ) ;
	pTime->SetMemberAsInt( L"nMinute", stLocal.wMinute ) ;
	pTime->SetMemberAsInt( L"nSecond", stLocal.wSecond ) ;
	//
	return	PushObject( *pTime ) ;
}

// 関数 : MemoryStatus GetMemoryStatus()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_GetMemoryStatus( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = VerifyArgumentCount( lstArg, 0 ) ;
	if ( err )
		return	err ;
	//
	MEMORYSTATUS	memst ;
	::eslFillMemory( &memst, 0, sizeof(memst) ) ;
	memst.dwLength = sizeof(memst) ;
	::GlobalMemoryStatus( &memst ) ;
	//
	ECSStructureInterface *	pMemStatus = CreateUserStructure( L"MemoryStatus" ) ;
	pMemStatus->SetMemberAsInt( L"nTotalPhys", memst.dwTotalPhys ) ;
	pMemStatus->SetMemberAsInt( L"nAvailPhys", memst.dwAvailPhys ) ;
	pMemStatus->SetMemberAsInt( L"nTotalVirtual", memst.dwTotalVirtual ) ;
	pMemStatus->SetMemberAsInt( L"nAvailVirtual", memst.dwAvailVirtual ) ;
	//
	return	PushObject( *pMemStatus ) ;
}

// 関数 : Integer AddModule( String sModuleName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_AddModule( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrModuleName ;
	err = GetArgumentAsStr( wstrModuleName, lstArg, 0, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSEnvironment *	pEnv = GetEnvironment() ;
	if ( pEnv != NULL )
	{
		err = pEnv->AddModule( wstrModuleName, this ) ;
	}
	return	PushObject( new_CSInteger( err ) ) ;
}

// 関数 : Integer OpenToAddArchiveFile( String sFilePath[, String sPassword[, String sID]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_OpenToAddArchiveFile( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFilePath, wstrPassword, wstrID ;
	err = GetArgumentAsStr( wstrFilePath, lstArg, 0, NULL ) ;
	if ( err )
		return	err ;
	err = GetArgumentAsStr( wstrPassword, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSEnvironment *	pEnv = GetEnvironment() ;
	err = eslErrGeneral ;
	if ( pEnv != NULL )
	{
		err = pEnv->AddFileArchive
				( wstrFilePath, EString( wstrPassword ), wstrID ) ;
	}
	return	PushObject( new_CSInteger( err ) ) ;
}

// 関数 : Integer EnableArchiveFilePath( String sID, Boolean fEnable )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_EnableArchiveFilePath( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	int				fEnable ;
	err = GetArgumentAsStr( wstrID, lstArg, 0, NULL ) ;
	if ( err )
		return	err ;
	err = GetArgumentAsInt( fEnable, lstArg, 1, -1 ) ;
	if ( err )
		return	err ;
	//
	ECSEnvironment *	pEnv = GetEnvironment() ;
	err = eslErrGeneral ;
	if ( pEnv != NULL )
	{
		err = pEnv->EnableFilePath( wstrID, (fEnable != 0) ) ;
	}
	return	PushObject( new_CSInteger( err ) ) ;
}

// 関数 : Sleep( Integer nMilliSec )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_Sleep( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	int	nMilliSec ;
	err = GetArgumentAsInt( nMilliSec, lstArg, 0, 0 ) ;
	if ( err )
		return	err ;
	//
	if ( nMilliSec >= 1000 )
	{
		DWORD	dwBeginTime = ::timeGetTime( ) ;
		while ( GetStatus() != xsHalt )
		{
			DWORD	dwCurrentTime = ::timeGetTime() - dwBeginTime ;
			if ( dwCurrentTime >= (unsigned int) nMilliSec )
			{
				break ;
			}
			DWORD	dwWaitTime = nMilliSec - dwCurrentTime ;
			if ( dwWaitTime > 100 )
			{
				dwWaitTime = 100 ;
			}
			WaitUntilEvent( NULL, dwWaitTime ) ;
//			::Sleep( dwWaitTime ) ;
		}
	}
	else if ( nMilliSec > 0 )
	{
		WaitUntilEvent( NULL, nMilliSec ) ;
//		::Sleep( nMilliSec ) ;
	}
	return	PushObject( new_CSInteger() ) ;
}

// 関数 : Suspend( String sContextFileName := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_Suspend( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	SetStatus( ECSContext::xsInterrupt ) ;
	//
	return	PushObject( new_CSInteger() ) ;
}

// 関数 : Exit()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_Exit( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 0 ) ;
	if ( err )
		return	err ;
	//
	SetStatus( ECSContext::xsHalt ) ;
	//
	return	PushObject( new_CSInteger() ) ;
}

// 関数 : Error Trace( String sText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_Trace( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrText ;
	err = GetArgumentAsStr( wstrText, lstArg, 0, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	OutputDebugString( EString( wstrText ) ) ;
	if ( wstrText.Right(1) != L"\n" )
	{
		OutputDebugString( "\n" ) ;
	}
	return	PushObject( new_CSInteger() ) ;
}

// 関数 : void * memmove( void * dst, const void * src, int bytes )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSContext::Call_memmove( ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSObject *	pDstObj = lstArg.GetAt( 0 ) ;
	ECSObject *	pSrcObj = lstArg.GetAt( 1 ) ;
	int			nBytes ;
	err = GetArgumentAsInt( nBytes, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	if ( (pDstObj == NULL) || (pSrcObj == NULL) )
	{
		return	ESLErrorMsg( "memmove で引数がヌルです" ) ;
	}
	void *	ptrDst = pDstObj->GetBuffer( 0, nBytes, true ) ;
	void *	ptrSrc = pSrcObj->GetBuffer( 0, nBytes, false ) ;
	if ( (ptrDst == NULL) || (ptrSrc == NULL ) )
	{
		return	ESLErrorMsg( "memmove で引数がヌルです" ) ;
	}
	eslMoveMemory( ptrDst, ptrSrc, nBytes ) ;
	pDstObj->FlushBuffer( 0, nBytes, ptrDst, true ) ;
	pSrcObj->FlushBuffer( 0, nBytes, ptrSrc, false ) ;
	//
	lstArg.DetachAt( 0 ) ;
	//
	return	PushObject( pDstObj ) ;
}


//////////////////////////////////////////////////////////////////////////////
// プラグインインターフェース
//////////////////////////////////////////////////////////////////////////////
ECSContext::PLUGIN_CONTEXT * ECSContext::GetContextInterface( void )
{
	if ( m_ppic == NULL )
	{
		m_ppic = (PLUGIN_CONTEXT*) ::eslHeapAllocate
			( NULL, sizeof(PLUGIN_CONTEXT), ESL_HEAP_ZERO_INIT ) ;
		m_ppic->pBackLink = this ;
		m_ppic->pfnGetStatus = PIC_GetStatus ;
		m_ppic->pfnSetStatus = PIC_SetStatus ;
		m_ppic->pfnGetInstructionPointer = PIC_GetInstructionPointer ;
		m_ppic->pfnSetInstructionPointer = PIC_SetInstructionPointer ;
		m_ppic->pfnLock = PIC_Lock ;
		m_ppic->pfnUnlock = PIC_Unlock ;
		m_ppic->pfnLockExecution = PIC_LockExecution ;
		m_ppic->pfnUnlockExecution = PIC_UnlockExecution ;
		m_ppic->pfnCallFunction = PIC_CallFunction ;
		m_ppic->pfnPushObject = PIC_PushObject ;
		m_ppic->pfnPopObject = PIC_PopObject ;
		m_ppic->pfnOpenFile = PIC_OpenFile ;
		m_ppic->pfnSave = PIC_Save ;
		m_ppic->pfnLoad = PIC_Load ;
		m_ppic->pfnSaveObject = PIC_SaveObject ;
		m_ppic->pfnLoadObject = PIC_LoadObject ;
		m_ppic->pfnCreateObject = PIC_CreateObject ;
		m_ppic->pfnGetStack = PIC_GetStack ;
		m_ppic->pfnGetGlobal = PIC_GetGlobal ;
		m_ppic->pfnGetStatic = PIC_GetStatic ;
		m_ppic->pfnCreateReference = PIC_CreateReference ;
		m_ppic->pfnCreateInteger = PIC_CreateInteger ;
		m_ppic->pfnCreateReal = PIC_CreateReal ;
		m_ppic->pfnCreateString = PIC_CreateString ;
		m_ppic->pfnCreateAbstractObject = PIC_CreateAbstractObject ;
		m_ppic->pfnGetWaveOutputDevice = PIC_GetWaveOutputDevice ;
		m_ppic->pfnGetDrawImageObject = PIC_GetDrawImageObject ;
		m_ppic->pfnGetHeapHandle = PIC_GetHeapHandle ;
	}
	return	m_ppic ;
}

ECS_CONTEXT::ExecutionStatus
	__stdcall ECSContext::PIC_GetStatus( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	(ECS_CONTEXT::ExecutionStatus) ppic->pBackLink->GetStatus( ) ;
}

ECS_CONTEXT::ExecutionStatus
	 __stdcall ECSContext::PIC_SetStatus
		( ECS_CONTEXT * context, ECS_CONTEXT::ExecutionStatus status )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	(ECS_CONTEXT::ExecutionStatus)
		ppic->pBackLink->SetStatus( (ExecutionStatus) status ) ;
}

unsigned long int
	__stdcall ECSContext::PIC_GetInstructionPointer( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->m_ip ;
}

void __stdcall ECSContext::PIC_SetInstructionPointer
	( ECS_CONTEXT * context, unsigned long int ip )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ppic->pBackLink->m_ip = ip ;
}

ESLError __stdcall ECSContext::PIC_Lock
	( ECS_CONTEXT * context, DWORD dwTimeout )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->Lock( dwTimeout ) ;
}

ESLError __stdcall ECSContext::PIC_Unlock( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->Unlock( ) ;
}

ESLError __stdcall ECSContext::PIC_LockExecution
	( ECS_CONTEXT * context, DWORD dwTimeout )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->LockExecution( dwTimeout ) ;
}

ESLError __stdcall ECSContext::PIC_UnlockExecution( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->UnlockExecution( ) ;
}

ESLError __stdcall ECSContext::PIC_CallFunction
	( ECS_CONTEXT * context, DWORD dwFuncAddr,
		ECS_OBJECT * const* pArg, int nArgCount )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ECSObjArray<ECSObject>	lstArg ;
	lstArg.SetSize( nArgCount ) ;
	for ( int i = 0; i < nArgCount; i ++ )
	{
		if ( pArg[i] != NULL )
		{
			ECSObject::PLUGIN_OBJECT *	ppio =
				(ECSObject::PLUGIN_OBJECT*) pArg[i] ;
			ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
			lstArg.SetAt( i, ppio->pBackLink ) ;
		}
	}
	ESLError	err = ppic->pBackLink->CallFunction( dwFuncAddr, lstArg ) ;
	lstArg.DetachAll( ) ;
	return	err ;
}

ESLError __stdcall ECSContext::PIC_PushObject
	( ECS_CONTEXT * context, ECS_OBJECT * pObj )
{
	PLUGIN_CONTEXT *			ppic = (PLUGIN_CONTEXT*) context ;
	ECSObject::PLUGIN_OBJECT *	ppio = (ECSObject::PLUGIN_OBJECT*) pObj ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	return	ppic->pBackLink->PushObject( ppio->pBackLink ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_PopObject( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ECSObject *	pPopObj = ppic->pBackLink->PopObject( ) ;
	ECS_OBJECT *	pObj = NULL ;
	if ( pPopObj != NULL )
	{
		pObj = pPopObj->CreateInterface( ) ;
	}
	return	pObj ;
}

ECS_FILE * __stdcall ECSContext::PIC_OpenFile
	( ECS_CONTEXT * context,
		const wchar_t * pwszFileName, long int nOpenFlags )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ESLFileObject *	pfile =
		ppic->pBackLink->OpenFileOnScript( pwszFileName, nOpenFlags ) ;
	if ( pfile == NULL )
	{
		return	NULL ;
	}
	return	new ECSFilePIInterface( pfile, true ) ;
}

ESLError __stdcall ECSContext::PIC_Save
	( ECS_CONTEXT * context, ECS_FILE * pfile )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ECSPIFileInterface	file( pfile ) ;
	return	ppic->pBackLink->Save( file ) ;
}

ESLError __stdcall ECSContext::PIC_Load
	( ECS_CONTEXT * context, ECS_FILE * pfile )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ECSPIFileInterface	file( pfile ) ;
	return	ppic->pBackLink->Load( file ) ;
}

ESLError __stdcall ECSContext::PIC_SaveObject
	( ECS_CONTEXT * context, ECS_FILE * pfile, ECS_OBJECT * pObj )
{
	ECSObject *			pObjTarget = NULL ;
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	if ( pObj != NULL )
	{
		ECSObject::PLUGIN_OBJECT *	ppio = (ECSObject::PLUGIN_OBJECT*) pObj ;
		ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
		pObjTarget = ppio->pBackLink ;
	}
	ECSPIFileInterface	file( pfile ) ;
	return	ppic->pBackLink->SaveObject( file, pObjTarget ) ;
}

ESLError __stdcall ECSContext::PIC_LoadObject
	( ECS_CONTEXT * context, ECS_FILE * pfile, ECS_OBJECT ** pObj )
{
	PLUGIN_CONTEXT *			ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ECSPIFileInterface	file( pfile ) ;
	ECSObject *	pLoadedObj = NULL ;
	ESLError	err = ppic->pBackLink->LoadObject( file, pLoadedObj ) ;
	if ( !err )
	{
		if ( pLoadedObj != NULL )
		{
			*pObj = pLoadedObj->CreateInterface( ) ;
		}
		else
		{
			*pObj = NULL ;
		}
	}
	return	err ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_CreateObject
	( ECS_CONTEXT * context, CSVariableType csvtType,
		const wchar_t * pwszType, DWORD * pdwFuncAddr )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ECSObject *	pObj =
		ppic->pBackLink->CreateObject( csvtType, pwszType, pdwFuncAddr ) ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	return	pObj->CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_GetStack( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->m_stack.CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_GetGlobal( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->m_pcsxi->m_csgGlobal.CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_GetStatic( ECS_CONTEXT * context )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->m_pcsxi->m_csgData.CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_CreateReference
	( ECS_CONTEXT * context, ECS_OBJECT * pRef )
{
	ECSObject *					pRefObj = NULL ;
	PLUGIN_CONTEXT *			ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	if ( pRef != NULL )
	{
		ECSObject::PLUGIN_OBJECT *	ppio = (ECSObject::PLUGIN_OBJECT*) pRef ;
		ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
		pRefObj = ppio->pBackLink ;
	}
	ECSObject *	pObj = ppic->pBackLink->new_CSReference( pRefObj ) ;
	return	pObj->CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_CreateInteger
	( ECS_CONTEXT * context, long int nInitVal )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->new_CSInteger( nInitVal )->CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_CreateReal
	( ECS_CONTEXT * context, double rInitVal )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppic->pBackLink->new_CSReal( rInitVal )->CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSContext::PIC_CreateString
	( ECS_CONTEXT * context, const wchar_t * pwszInitVal )
{
	PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ECSString *	pstrNew = ppic->pBackLink->new_CSString( ) ;
	pstrNew->m_varStr = pwszInitVal ;
	return	pstrNew->CreateInterface( ) ;
}

ECS_OBJECT *
	__stdcall ECSContext::PIC_CreateAbstractObject( ECS_CONTEXT * context )
{
	ECSPIObject *	pObj = new ECSPIObject ;
	return	pObj->CreateInterface( ) ;
}

ESLObject *
	__stdcall ECSContext::PIC_GetWaveOutputDevice( ECS_CONTEXT * context )
{
	EWaveMixingServer *	pwmxs = ECotophaScript::OpenDirectSound( ) ;
	if ( pwmxs == NULL )
	{
		pwmxs = ECotophaScript::OpenWaveDevice( ) ;
	}
	return	pwmxs ;
}

ESLObject *
	__stdcall ECSContext::PIC_GetDrawImageObject( ECS_CONTEXT * context )
{
	return	ECotophaScript::CreateDrawImage( ) ;
}

HESLHEAP __stdcall ECSContext::PIC_GetHeapHandle( ECS_CONTEXT * context )
{
	return	eslGetGlobalHeap( ) ;
}


//////////////////////////////////////////////////////////////////////////////
// プラグイン用ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

void __stdcall ECSFilePIInterface::PIC_Release( ECS_FILE * pfile )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	delete	pfpi ;
}

unsigned long int __stdcall ECSFilePIInterface::PIC_Read
	( ECS_FILE * pfile, void * ptrBuffer, unsigned long int nBytes )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	if ( pfpi->m_pfile == NULL )
	{
		return	0 ;
	}
	return	pfpi->m_pfile->Read( ptrBuffer, nBytes ) ;
}

unsigned long int __stdcall ECSFilePIInterface::PIC_Write
	( ECS_FILE * pfile, const void * ptrBuffer, unsigned long int nBytes )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	if ( pfpi->m_pfile == NULL )
	{
		return	0 ;
	}
	return	pfpi->m_pfile->Write( ptrBuffer, nBytes ) ;
}

unsigned long int
	__stdcall ECSFilePIInterface::PIC_GetLength( ECS_FILE * pfile )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	if ( pfpi->m_pfile == NULL )
	{
		return	0 ;
	}
	return	pfpi->m_pfile->GetLength( ) ;
}

unsigned long int __stdcall ECSFilePIInterface::PIC_Seek
	( ECS_FILE * pfile, long int nOffsetPos, ECS_FILE::SeekOrigin fSeekFrom )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	if ( pfpi->m_pfile == NULL )
	{
		return	0 ;
	}
	return	pfpi->m_pfile->Seek
		( nOffsetPos, (ESLFileObject::SeekOrigin) fSeekFrom ) ;
}

unsigned long int
	__stdcall ECSFilePIInterface::PIC_GetPosition( ECS_FILE * pfile )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	if ( pfpi->m_pfile == NULL )
	{
		return	0 ;
	}
	return	pfpi->m_pfile->GetPosition( ) ;
}

ESLError __stdcall ECSFilePIInterface::PIC_SetEndOfFile( ECS_FILE * pfile )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	if ( pfpi->m_pfile == NULL )
	{
		return	eslErrSuccess ;
	}
	return	pfpi->m_pfile->SetEndOfFile( ) ;
}

const char * __stdcall ECSFilePIInterface::PIC_GetFilePath( ECS_FILE * pfile )
{
	ECSFilePIInterface *	pfpi = (ECSFilePIInterface*) pfile ;
	if ( pfpi->m_pfile == NULL )
	{
		return	NULL ;
	}
	ERawFile *	prf = ESLTypeCast<ERawFile>( pfpi->m_pfile ) ;
	if ( prf == NULL )
	{
		return	NULL ;
	}
	return	prf->GetFilePath( ) ;
}
