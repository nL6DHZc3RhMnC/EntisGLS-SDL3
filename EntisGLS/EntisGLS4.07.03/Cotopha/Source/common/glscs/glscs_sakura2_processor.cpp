
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_window.h>

using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// グローバルコンテキスト
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( DWORD	ECSSakura2Processor::maskGlobalInterrupt = 0 ) ;	// 大域割り込みマスク
ESL_DLL_DECL( DWORD	ECSSakura2Processor::countRuningContext = 0 ) ;	// 実行中コンテキスト数

// 排他同期処理のためのオブジェクト
ESL_DLL_DECL( SSystem::SCriticalSection *	ECSSakura2Processor::mutexGlobalAtomic = NULL ) ;

// スクリプト上の QuickLock 用
ESL_DLL_DECL( SSystem::SCriticalSection *	ECSSakura2Processor::mutexQuickLock = NULL ) ;

// assert lock の為に実行を中断したことを知らせる
ESL_DLL_DECL( SSystem::SignalEvent *	ECSSakura2Processor::signalLeave = NULL ) ;

// assert lock 中は非シグナル状態
ESL_DLL_DECL( SSystem::SignalEvent *	ECSSakura2Processor::signalUnlocked = NULL ) ;

// assert lock された回数
ESL_DLL_DECL( DWORD	ECSSakura2Processor::countAssertLocked = 0 ) ;

// システム関数のエントリ数
static int	g_countSystemCallFunc = 0 ;


//////////////////////////////////////////////////////////////////////////////
// 初期化関数
//////////////////////////////////////////////////////////////////////////////

// ライブラリ初期化
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2Processor::Initialize( void )
{
#if	!defined(ENTISGLS4_DLL_IMPORT)
	SSystem::Initialize() ;
	//
	// 同期オブジェクト生成
	//
	mutexGlobalAtomic = new SSystem::SCriticalSection() ;
	mutexQuickLock = new SSystem::SCriticalSection() ;
	signalLeave = new SSystem::SignalEvent ;
	signalUnlocked = new SSystem::SignalEvent ;
	signalLeave ->Initialize( false ) ;
	signalUnlocked->Initialize( false ) ;
	//
	// システム関数ソート
	//
	int	nSysCount = 0 ;
	while ( entrySysCall[nSysCount].pszFuncName != NULL )
	{
		nSysCount ++ ;
	}
	g_countSystemCallFunc = nSysCount ;
	//
	for ( int i = 0; i < nSysCount; i ++ )
	{
		int				iMin = i ;
		const char *	pszMinName = entrySysCall[i].pszFuncName ;
		//
		for ( int j = i + 1; j < nSysCount; j ++ )
		{
			const char *	pszFuncName = entrySysCall[j].pszFuncName ;
			for ( int k = 0; true; k ++ )
			{
				int	c = (int) pszFuncName[k] - (int) pszMinName[k] ;
				if ( c < 0 )
				{
					iMin = j ;
					pszMinName = pszFuncName ;
					break ;
				}
				else if ( (c > 0) || (pszFuncName[k] == 0) )
				{
					break ;
				}
			}
		}
		//
		if ( i != iMin )
		{
			SYSCALL_ENTRY	sceTemp = entrySysCall[i] ;
			entrySysCall[i] = entrySysCall[iMin] ;
			entrySysCall[iMin] = sceTemp ;
		}
	}
#endif
}

// ライブラリ終了
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2Processor::Close( void )
{
#if	!defined(ENTISGLS4_DLL_IMPORT)
	//
	// 同期オブジェクト消去
	//
	delete	mutexGlobalAtomic ;
	delete	mutexQuickLock ;
	delete	signalLeave ;
	delete	signalUnlocked ;
	mutexGlobalAtomic = NULL ;
	mutexQuickLock = NULL ;
	signalLeave = NULL ;
	signalUnlocked = NULL ;
	//
	SSystem::Finalize() ;
#endif
}


//////////////////////////////////////////////////////////////////////////////
// 実行コンテキスト
//////////////////////////////////////////////////////////////////////////////

// プロセッサ初期設定
//////////////////////////////////////////////////////////////////////////////
void Context::InitializeProcessor( INT64 initSP )
{
	//
	// レジスタ初期化
	//
	InitializeRegister() ;
	//
	m_regset[regSP].i = initSP ;
	//
	m_ipSegment = ECSSakura2::VirtualMachine::roasCode << 24 ;
	m_maskException = 0 ;
	m_idSystemCall = 0 ;
	m_idInteruption = 0 ;
	m_status = xsHalt ;
	//
	m_ptrCode = NULL ;
	m_ptrTrick = NULL ;
	m_pfnTranslate = default_translate_address ;
	m_pfnAtomicTranslate = default_atomic_translate_address ;
	//
	eslFillMemory( &m_segStack, 0, sizeof(m_segStack) ) ;
	eslFillMemory( &m_segLoadCache[0], 0, sizeof(m_segLoadCache) ) ;
	eslFillMemory( &m_segStoreCache[0], 0, sizeof(m_segStoreCache) ) ;
	eslFillMemory( &m_mask2ndTLB[0], 0, sizeof(m_mask2ndTLB) ) ;
	eslFillMemory( &m_seg2ndCache[0], 0, sizeof(m_seg2ndCache) ) ;
	//
	// メモリアクセスインターフェース設定
	//
	eslMoveMemory( m_pfnLoad, pfnSakuraLoad, sizeof(m_pfnLoad) ) ;
	eslMoveMemory( m_pfnStore, pfnSakuraStore, sizeof(m_pfnStore) ) ;
	//
	// 命令実行インターフェース設定
	//
	eslMoveMemory( m_pfnInstruction, pfnInstruction, sizeof(m_pfnInstruction) ) ;
}

// レジスタ初期化
//////////////////////////////////////////////////////////////////////////////
void Context::InitializeRegister( void )
{
	eslFillMemory( m_regset, 0, sizeof(m_regset) ) ;
	//
	m_regset[regIntOne].i = 1 ;
	m_regset[regFillBit].i = -1 ;
	m_regset[regMaskLow32].i = 0xFFFFFFFFUL ;
	m_regset[regMaskLow16].i = 0xFFFFU ;
	m_regset[regMaskLow8].i = 0xFFU ;
	//
	m_regset[regFloatOne].f = 1.0 ;
	m_regset[regFloatPI].f = 3.1415926535897932384626433832795 ;
}

// TLB リセット
//////////////////////////////////////////////////////////////////////////////
void Context::ResetAddressTranslationCache( void )
{
	m_segLoadCache[0].highAddress = 0 ;
	m_segLoadCache[0].limitSegment = 0 ;
	m_segLoadCache[1].highAddress = 0 ;
	m_segLoadCache[1].limitSegment = 0 ;
	m_segLoadCache[2].highAddress = 0 ;
	m_segLoadCache[2].limitSegment = 0 ;
	m_segLoadCache[3].highAddress = 0 ;
	m_segLoadCache[3].limitSegment = 0 ;
	//
	m_segStoreCache[0].highAddress = 0 ;
	m_segStoreCache[0].limitSegment = 0 ;
	m_segStoreCache[1].highAddress = 0 ;
	m_segStoreCache[1].limitSegment = 0 ;
	m_segStoreCache[2].highAddress = 0 ;
	m_segStoreCache[2].limitSegment = 0 ;
	m_segStoreCache[3].highAddress = 0 ;
	m_segStoreCache[3].limitSegment = 0 ;
	//
	m_mask2ndTLB[0] = 0 ;
}

// アドレス変換
//////////////////////////////////////////////////////////////////////////////
BYTE * Context::AsyncTranslateAddress( INT64 nAddress, size_t nRange )
{
	LinearAddressCache	lac ;
	if ( (*m_pfnTranslate)( this, &lac, nAddress ) != NULL )
	{
		SDWORD	dwOffset = (DWORD) nAddress ;
		dwOffset -= lac.baseOffset ;
		if ( (DWORD) ((dwOffset >> 31) | (dwOffset + nRange))
											<= lac.limitSegment )
		{
			return	lac.pbytBuffer + dwOffset ;
		}
	}
	return	NULL ;
}

BYTE * Context::AtomicTranslateAddress( INT64 nAddress, size_t nRange )
{
	LinearAddressCache	lac ;
	if ( (*m_pfnAtomicTranslate)( this, &lac, nAddress ) != NULL )
	{
		SDWORD	dwOffset = (DWORD) nAddress ;
		dwOffset -= lac.baseOffset ;
		if ( (DWORD) ((dwOffset >> 31) | (dwOffset + nRange))
											<= lac.limitSegment )
		{
			return	lac.pbytBuffer + dwOffset ;
		}
	}
	return	NULL ;
}

// 命令実行
//////////////////////////////////////////////////////////////////////////////
DWORD Context::ExecuteCore( void )
{
	#if	!defined(__DISABLED_EXCEPTION__) && !defined(__DEBUG__)
	try
	#endif
	{
		DWORD	maskExecution ;
		AtomicInc( &countRuningContext ) ;
		maskExecution = m_maskException | maskGlobalInterrupt ;
		while ( !maskExecution )
		{
			(m_pfnInstruction[m_ptrCode[m_ip]])( this ) ;
			maskExecution = m_maskException | maskGlobalInterrupt ;
		}
		AtomicDec( &countRuningContext ) ;
		return	maskExecution ;
	}
	#if	!defined(__DISABLED_EXCEPTION__) && !defined(__DEBUG__)
	catch ( ... )
	{
		SSystem::Trace( "exception in Context::ExecuteCore\n" ) ;
		AtomicOr( &m_maskException, exceptionBadInstruction ) ;
		AtomicDec( &countRuningContext ) ;
		return	m_maskException ;
	}
	#endif
}

// 実行ステータス変更
//////////////////////////////////////////////////////////////////////////////
Context::ExecutionStatus
	Context::ChangeExecutionStatus( Context::ExecutionStatus status )
{
	Context::ExecutionStatus	statusLast ;
	SSystem::QuickLock() ;
	statusLast = m_status ;
	m_status = status ;
	SSystem::QuickUnlock() ;
	AtomicOr( &m_maskException, interruptChangeStatus ) ;
	return	statusLast ;
}

// ペンディング・ステータス設定
//////////////////////////////////////////////////////////////////////////////
void Context::SetPendingStatus( int64_t countPending )
{
	m_countPending = countPending ;
	ChangeExecutionStatus( xsPending ) ;
}

// エラーメッセージを設定
//////////////////////////////////////////////////////////////////////////////
void Context::SetContextErrorMessage( const wchar_t * pszErrMsg )
{
	if ( pszErrMsg != NULL )
	{
		int	lenMsg = 0 ;
		while ( pszErrMsg[lenMsg] != 0 )
		{
			lenMsg ++ ;
		}
		wchar_t *	pszNewError = new wchar_t[lenMsg + 1] ;
		for ( int i = 0; i <= lenMsg; i ++ )
		{
			pszNewError[i] = pszErrMsg[i] ;
		}
		delete []	m_pszError ;
		m_pszError = pszNewError ;
		m_maskException |= exceptionSystemError ;
	}
	else
	{
		delete []	m_pszError ;
		m_pszError = NULL ;
		m_maskException &= ~exceptionSystemError ;
	}
}

// レジスタの値をデバッグ出力
//////////////////////////////////////////////////////////////////////////////
void Context::TraceDumpRegister( void )
{
	SSystem::Trace( "ip=%08X:%08X\n", m_ipSegment, m_ip ) ;
	//
	for ( int i = 0; i < 0x100; i += 4 )
	{
		SSystem::Trace
			( "r%d=%08X:%08X r%d=%08X:%08X r%d=%08X:%08X r%d=%08X:%08X\n",
				i, m_regset[i].h32, m_regset[i].l32,
				i+1, m_regset[i+1].h32, m_regset[i+1].l32,
				i+2, m_regset[i+2].h32, m_regset[i+2].l32,
				i+3, m_regset[i+3].h32, m_regset[i+3].l32 ) ;
	}
}

// レジスタの値をファイルに出力
//////////////////////////////////////////////////////////////////////////////
void Context::WriteDumpRegister( SSystem::SBufferedFile& file )
{
	SSystem::SString	strHigh, strLow ;
	strHigh.HexFromInteger( m_ipSegment, 8 ) ;
	strLow.HexFromInteger( m_ip, 8 ) ;
	file << L"ip=" << strHigh << L":" << strLow << "\n" ;
	//
	for ( int i = 0; i < 0x100; i ++ )
	{
		strHigh.HexFromInteger( m_regset[i].h32, 8 ) ;
		strLow.HexFromInteger( m_regset[i].l32, 8 ) ;
		if ( (i & 0x03) != 0 )
		{
			file << L" " ;
		}
		file << L"r" << (int64_t) i << L"=" << strHigh << L":" << strLow ;
		if ( (i & 0x03) != 3 )
		{
			file << L"\r\n" ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// 実行コンテキスト・抽象シェル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2Processor::ContextShell, ESLObject )

// ExecuteCore 実行前処理
//////////////////////////////////////////////////////////////////////////////
void ContextShell::PrepareCoreExecution( void )
{
	ResetAddressTranslationCache() ;
	//
	m_ptrCode = NULL ;
	m_ptrTrick = NULL ;
	m_maskException = exceptionFarJump ;
	//
	if ( m_pSakura2VM != NULL )
	{
		ECSSakura2::Object *	
			pObjCode = m_pSakura2VM->AtomicObjectFromAddress( m_ipSegment ) ;
		if ( pObjCode != NULL )
		{
			LinearAddressCache	seg ;
			if ( pObjCode->GetSegmentBuffer( seg ) != NULL )
			{
				m_ptrCode = seg.pbytBuffer ;
				m_ptrTrick = pObjCode->GetSegmentShadowBuffer( 0 ) ;
				m_maskException = 0 ;
			}
		}
	}
	(*m_pfnTranslate)( this, &m_segStack, m_regset[regSP].i ) ;
}

// 命令実行（例外処理含む）
//////////////////////////////////////////////////////////////////////////////
DWORD ContextShell::ExecuteKernel( void )
{
	DWORD	maskException ;
	if ( m_status == xsPending )
	{
		if ( OnPendingContext() == SSystem::errPending )
		{
			return	0 ;
		}
	}
	m_status = xsExecution ;
	do
	{
		if ( !(maskGlobalInterrupt & interruptAssertLock) )
		{
			// 実行
			PrepareCoreExecution() ;
			maskException = ExecuteCore() ;
		}
		else
		{
			maskException = maskGlobalInterrupt ;
		}
		if ( maskException & (interruptMask | exceptionContinueMask) )
		{
			// 実行継続可能な例外を処理
			if ( maskException & interruptAssertLock )
			{
				// assert lock 同期
				maskException = HandleExceptionAssertLock( maskException ) ;
				if ( maskException == 0 )
				{
					continue ;
				}
			}
			if ( maskException & exceptionSystemCall )
			{
				// システムコール
				maskException = HandleExceptionSystemCall( maskException ) ;
				if ( maskException == 0 )
				{
					continue ;
				}
			}
			if ( maskException & exceptionExtendStack )
			{
				// スタック拡張
				maskException = HandleExceptionExtendStack( maskException ) ;
			}
			if ( maskException & exceptionFarJump )
			{
				// far jump
				maskException = HandleExceptionFarJump( maskException ) ;
			}
			if ( maskException & interruptChangeStatus )
			{
				// 実行ステータスの変更割り込み
				AtomicAnd( &m_maskException, ~interruptChangeStatus ) ;
				maskException &= ~interruptChangeStatus ;
			}
			if ( maskException & interruptEscape )
			{
				// デバッグ用例外処理
				maskException = HandleExceptionEscape( maskException ) ;
			}
			if ( m_status != xsExecution )
			{
				break ;
			}
		}
	}
	while ( maskException == 0 ) ;
	return	maskException ;
}

// 命令実行（例外処理含む）
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::ExecuteShell( void )
{
	do
	{
		DWORD	maskException = ExecuteKernel() ;
		const wchar_t *	pszErr = GetExceptionErrorMessage( maskException ) ;
		if ( pszErr != NULL )
		{
			// 例外エラー発生
			pszErr = ThrowException( pszErr ) ;
			if ( pszErr != NULL )
			{
				return	pszErr ;
			}
			continue ;
		}
		if ( maskException & (exceptionContinueMask | interruptMask) )
		{
			// object mode へ切り替え／その他割り込み
			break ;
		}
		if ( m_status == xsSuspend )
		{
			// 一時停止処理
			if ( OnSuspendContext() == SSystem::errAbort )
			{
				return	NULL ;
			}
		}
	}
	while ( m_status == xsExecution ) ;
	return	NULL ;
}

// 例外エラーをスクリプトにスロー
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::ThrowException( const wchar_t * pszErrMsg )
{
	// XP レジスタのポイントするメモリ取得
	Register *	pXP =
		(Register*) AtomicTranslateAddress
					( m_regset[regXP].i, sizeof(Register) * 2 ) ;
	if ( pXP == NULL )
	{
		return	pszErrMsg ;
	}

	// エラーメッセージサイズ
	size_t	lenErrMsg = SSystem::SString::GetLength( pszErrMsg ) ;

	// バッファをアロケーション
	INT64	addrBuf = 0 ;
	if ( pszErrMsg != NULL )
	{
		ECSSakura2::BufferObject *	pBufErrMsg = new ECSSakura2::BufferObject ;
		pBufErrMsg->CreateBuffer( (DWORD) (lenErrMsg + 1) * sizeof(WORD) ) ;
		//
		// バッファにエラーメッセージ複製
		WORD *	pwBuf = (WORD*) pBufErrMsg->GetBuffer() ;
		for ( size_t i = 0; i <= lenErrMsg; i ++ )
		{
			pwBuf[i] = (WORD) pszErrMsg[i] ;
		}

		ECSSakura2Processor::AssertLock() ;
		addrBuf = m_pSakura2VM->AllocateHeapObjectAddress
					( pBufErrMsg, SSystem::mallocModeGlobal ) ;
		ECSSakura2Processor::AssertUnlock() ;
	}

	// レジスタ設定
	m_regset[regException0].l32 = 0x80000000 ;
	m_regset[regException0].h32 = 0xFFFFFFFF ;
	m_regset[regException1].i = addrBuf ;
	m_regset[regException2].i = 0 ;
	m_regset[regException3].i = 0 ;

	m_ip = pXP->l32 ;
	if ( m_ipSegment != pXP->h32 )
	{
		m_ipSegment = pXP->h32 ;
		AtomicOr( &m_maskException, exceptionFarJump ) ;
	}
	return	NULL ;
}

// 一時停止処理（ExecuteShell の外側で処理する場合には errAbort を返却）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ContextShell::OnSuspendContext( void )
{
	while ( m_status == xsSuspend )
	{
		SSystem::SleepMilliSec( 10 ) ;
	}
	return	SSystem::errSuccess ;
}

// 一時停止処理（イベント待ちのとき errPending を返却）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ContextShell::OnPendingContext( void )
{
	if ( m_countPending < 0 )
	{
		return	SSystem::errPending ;
	}
	if ( m_countPending > 0 )
	{
		m_countPending -- ;
	}
	if ( m_countPending > 0 )
	{
		return	SSystem::errPending ;
	}
	return	SSystem::errSuccess ;
}

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	ContextShell::BeginFunction
		( INT64 addrFunc, const Register *pArg, int nArgCount )
{
	// 引数
	const wchar_t *	pwszErr ;
	pwszErr = PushStack( pArg, nArgCount ) ;
	if ( pwszErr != NULL )
	{
		return	pwszErr ;
	}

	// 帰りアドレス
	Register	regRetIP ;
	regRetIP.i = -1 ;
	pwszErr = PushStack( &regRetIP, 1 ) ;
	if ( pwszErr != NULL )
	{
		return	pwszErr ;
	}

	// 次に実行する命令ポインタ
	m_ip = (DWORD) addrFunc ;
	m_ipSegment = (DWORD) (addrFunc >> 32) ;

	// 実行
	pwszErr = ExecuteShell() ;
	if ( pwszErr != NULL )
	{
		// 例外エラー
		m_pSakura2VM->HandleExceptionError( this, pwszErr ) ;
		return	pwszErr ;
	}
	if ( m_status == xsHalt )
	{
		// 関数終了後は引数の分だけスタックを解放
		m_regset[regSP].l32 += nArgCount * sizeof(Register) ;
	}
	return	NULL ;
}

// 仮想関数開始
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::BeginVirtualFunction
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
			return	BeginFunction( *pVector, pArg, nArgCount ) ;
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

// スタックへプッシュ
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::PushStack( const Register *pData, int nCount )
{
	Register *	pStack = NULL ;
	const wchar_t *	pwszErr = AllocateStack( pStack, nCount ) ;
	if ( pwszErr != NULL )
	{
		return	pwszErr ;
	}
	for ( int i = 0; i < nCount; i ++ )
	{
		pStack[i] = pData[i] ;
	}
	return	NULL ;
}

// スタックへ一時領域を確保しデータを転送
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::PushBinaryOnStack
	( int& nPushedCount, const void * ptrBuf, size_t nBytes )
{
	const int		nArgCount = (int) (nBytes + 0x07) >> 3 ;
	Register*		pStack = NULL ;
	const wchar_t *	pwszErr ;
	nPushedCount = 0 ;
	pwszErr = AllocateStack( pStack, nArgCount ) ;
	if ( pwszErr != NULL )
	{
		return	pwszErr ;
	}
	eslFillMemory( pStack, 0, nArgCount * sizeof(Register) ) ;
	eslMoveMemory( pStack, ptrBuf, nBytes ) ;
	nPushedCount = nArgCount ;
	return	NULL ;
}

// スタックへ一時領域を確保し文字列を転送
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::PushStringOnStack
	( int& nPushedCount, const wchar_t * pwszString, int nLength )
{
	if ( nLength < 0 )
	{
		nLength = 0 ;
		if ( pwszString != NULL )
		{
			while ( pwszString[nLength] != 0 )
			{
				nLength ++ ;
			}
		}
	}
	if ( nLength == 0 )
	{
		uint16_t	wchNull = 0 ;
		return	PushBinaryOnStack
			( nPushedCount, &wchNull, sizeof(uint16_t) ) ;
	}
	else if ( sizeof(uint16_t) == sizeof(pwszString) )
	{
		return	PushBinaryOnStack
			( nPushedCount, pwszString, (nLength + 1) * sizeof(uint16_t) ) ;
	}
	else
	{
		SSystem::SString	strString = pwszString ;
		return	PushBinaryOnStack
			( nPushedCount, strString.GetConstArray(),
				(strString.GetLength() + 1) * sizeof(uint16_t) ) ;
	}
}

// スタック確保
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::AllocateStack( Register*& pStack, int nCount )
{
	if ( nCount > 0 )
	{
		m_regset[regSP].l32 -= nCount * sizeof(Register) ;
		//
		pStack = (Register*) AtomicTranslateAddress
					( m_regset[regSP].i, nCount * sizeof(Register) ) ;
		if ( pStack == NULL )
		{
			if ( HandleExceptionExtendStack
						( exceptionExtendStack ) & exceptionStackOverflow )
			{
				return	L"スタックオーバーフローが発生しました" ;
			}
			pStack = (Register*) AtomicTranslateAddress
						( m_regset[regSP].i, nCount * sizeof(Register) ) ;
			if ( pStack == NULL )
			{
				return	L"スタックオーバーフローが発生しました" ;
			}
		}
	}
	return	NULL ;
}

// スタック解放
//////////////////////////////////////////////////////////////////////////////
void ContextShell::FreeStack( int nCount )
{
	m_regset[regSP].l32 += nCount * sizeof(Register) ;
}

// assert lock 同期例外処理
//////////////////////////////////////////////////////////////////////////////
DWORD ContextShell::HandleExceptionAssertLock( DWORD maskException )
{
	signalLeave->SetSignal() ;
	if ( m_pSakura2VM != NULL )
	{
		m_pSakura2VM->WaitSynchronism
			( this, signalUnlocked, SSystem::Synchronism::Infinite ) ;
	}
	else
	{
		while ( m_status == xsExecution )
		{
			if ( signalUnlocked->Wait( 10 ) == SSystem::errSuccess )
			{
				break ;
			}
		}
	}
	return	(maskException | m_maskException) & ~interruptAssertLock ;
}

// システムコール例外処理
//////////////////////////////////////////////////////////////////////////////
DWORD ContextShell::HandleExceptionSystemCall( DWORD maskException )
{
	AtomicAnd( &m_maskException, ~exceptionSystemCall ) ;
	maskException &= ~exceptionSystemCall ;
	//
	if ( m_pSakura2VM != NULL )
	{
		BYTE *	pbytBuf = m_segStack.pbytBuffer ;
		int		nSP = m_regset[regSP].l32 - m_segStack.baseOffset ;
		//
		const wchar_t *	pszErr =
			m_pSakura2VM->SystemCallByIdentity
				( this, m_idSystemCall, (Register*) (pbytBuf + nSP) ) ;
		if ( pszErr != NULL )
		{
			maskException |= exceptionSystemError ;
			SetContextErrorMessage( pszErr ) ;
		}
		else
		{
			maskException |= m_maskException ;
		}
	}
	else
	{
		maskException |= exceptionSystemError ;
		SetContextErrorMessage( L"仮想マシンが見つかりません" ) ;
	}
	return	maskException ;
}

// far jump 例外処理
//////////////////////////////////////////////////////////////////////////////
DWORD ContextShell::HandleExceptionFarJump( DWORD maskException )
{
	if ( (m_ipSegment == 0xFFFFFFFF) && (m_ip == 0xFFFFFFFF) )
	{
		m_status = xsHalt ;
		AtomicAnd( &m_maskException, ~exceptionFarJump ) ;
	}
	else if ( (m_pSakura2VM == NULL)
			|| (m_pSakura2VM->AtomicObjectFromAddress( m_ipSegment ) == NULL) )
	{
		maskException |= exceptionSystemError ;
		SetContextErrorMessage( L"命令ポインタが不正です" ) ;
	}
	else
	{
		AtomicAnd( &m_maskException, ~exceptionFarJump ) ;
	}
	return	maskException & ~exceptionFarJump ;
}

// デバッグ用例外処理
//////////////////////////////////////////////////////////////////////////////
DWORD ContextShell::HandleExceptionEscape( DWORD maskException )
{
	if ( m_pSakura2VM != NULL )
	{
		return	m_pSakura2VM->HandleExceptionEscape( this, maskException ) ;
	}
	AtomicAnd( &m_maskException, ~interruptEscape ) ;
	maskException &= ~interruptEscape ;
	return	maskException ;
}

// 例外エラーメッセージを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ContextShell::GetExceptionErrorMessage( DWORD maskException )
{
	if ( maskException & ~(exceptionContinueMask | interruptMask) )
	{
		if ( maskException & exceptionReadMemory )
		{
			return	L"メモリ読み込み例外が発生しました" ;
		}
		if ( maskException & exceptionWriteMemory )
		{
			return	L"メモリ書き出し例外が発生しました" ;
		}
		if ( maskException & exceptionStackOverflow )
		{
			return	L"スタックオーバーフロー例外が発生しました" ;
		}
		if ( maskException & exceptionBadInstruction )
		{
			return	L"不正な命令が実行されました" ;
		}
		if ( maskException & exceptionZeroDivision )
		{
			return	L"ゼロ除算例外が発生しました" ;
		}
		if ( maskException & exceptionSystemError )
		{
			return	m_pszError ;
		}
	}
	else if ( maskException & exceptionContinueMask )
	{
		if ( maskException & exceptionExtendStack )
		{
			return	L"処理されないスタック拡張例外が発生しました" ;
		}
		if ( maskException & exceptionFarJump )
		{
			return	L"処理されない far jump 例外が発生しました" ;
		}
		if ( maskException & exceptionObjectMode )
		{
			return	L"object mode 例外が発生しました" ;
		}
		if ( maskException & exceptionSystemCall )
		{
			return	L"処理されないシステムコール例外が発生しました" ;
		}
	}
	return	NULL ;
}

// 128 bit アライメント new
//////////////////////////////////////////////////////////////////////////////
void * ContextShell::AllocateContext( size_t nBytes, size_t offsetContext )
{
	BYTE *	pbytBuf = (BYTE*) esl_malloc( nBytes + 0x20 + offsetContext ) ;
	size_t	modContext = ((DWORD_PTR) (pbytBuf + offsetContext)) & 0x0F ;
	if ( modContext != 0 )
	{
		modContext = 0x10 - modContext ;
	}
	BYTE *	pbytObj = pbytBuf + (0x10 + modContext) ;
	((void**)pbytObj)[-1] = pbytBuf ;
	return	pbytObj ;
}

void ContextShell::FreeContext( void * pObj )
{
	esl_free( ((void**)pObj)[-1] ) ;
}

void * ContextShell::operator new ( size_t nBytes )
{
	size_t	offsetContext =
		offsetof(ContextShell,m_regset) - offsetof(Context,m_regset) ;
	//
	return	AllocateContext( nBytes, offsetContext ) ;
}

void ContextShell::operator delete ( void * pObj )
{
	FreeContext( pObj ) ;
}


//////////////////////////////////////////////////////////////////////////////
// assert lock 同期
//////////////////////////////////////////////////////////////////////////////

void ECSSakura2Processor::AssertLock( void )
{
	signalUnlocked->ResetSignal() ;
	AtomicOr( &maskGlobalInterrupt, interruptAssertLock ) ;
	//
	while ( countRuningContext != 0 )
	{
		if ( signalLeave->Wait( 10 ) == SSystem::errSuccess )
		{
			signalLeave->ResetSignal() ;
		}
		signalUnlocked->ResetSignal() ;
		AtomicOr( &maskGlobalInterrupt, interruptAssertLock ) ;
	}
	AtomicInc( &countAssertLocked ) ;
}

void ECSSakura2Processor::AssertUnlock( void )
{
	AtomicDec( &countAssertLocked ) ;
	if ( countAssertLocked == 0 )
	{
		AtomicAnd( &maskGlobalInterrupt, ~interruptAssertLock ) ;
		signalUnlocked->SetSignal() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// メモリアクセス関数
//////////////////////////////////////////////////////////////////////////////

const int	ECSSakura2Processor::sizeof_prim_data[dataTypeMax] =
{
	8, 4, 2, 1,
	4, 4, 2, 1,
} ;

const LOAD_MEM_PROC	ECSSakura2Processor::pfnLoadMemory[dataTypeMax] =
{
	mem_load_int64,
	mem_load_int32,
	mem_load_int16,
	mem_load_int8,
	mem_load_float32,
	mem_load_uint32,
	mem_load_uint16,
	mem_load_uint8,
} ;

const LOAD_PROC	ECSSakura2Processor::pfnSakuraLoad[dataTypeMax] =
{
	smem_load_int64,
	smem_load_int32,
	smem_load_int16,
	smem_load_int8,
	smem_load_float32,
	smem_load_uint32,
	smem_load_uint16,
	smem_load_uint8,
} ;

const STORE_MEM_PROC	ECSSakura2Processor::pfnStoreMemory[dataTypeMax] =
{
	mem_store_int64,
	mem_store_int32,
	mem_store_int16,
	mem_store_int8,
	mem_store_float32,
	mem_store_uint32,
	mem_store_uint16,
	mem_store_uint8,
} ;

const STORE_PROC	ECSSakura2Processor::pfnSakuraStore[dataTypeMax] =
{
	smem_store_int64,
	smem_store_int32,
	smem_store_int16,
	smem_store_int8,
	smem_store_float32,
	smem_store_int32,
	smem_store_int16,
	smem_store_int8,
} ;


// ロード関数
//////////////////////////////////////////////////////////////////////////////
INT64 __fastcall ECSSakura2Processor::mem_load_int64( const void * ptrMem )
{
	#if	defined(__PLATFORM_ANDROID__)
		return	((INT64) *((DWORD*)((BYTE*)ptrMem + 4)) << 32) | *((DWORD*)ptrMem) ;
	#else
		return	*((INT64*)ptrMem) ;
	#endif
}

INT64 __fastcall ECSSakura2Processor::mem_load_int32( const void * ptrMem )
{
	return	*((SDWORD*)ptrMem) ;
}

INT64 __fastcall ECSSakura2Processor::mem_load_int16( const void * ptrMem )
{
	return	*((SWORD*)ptrMem) ;
}

INT64 __fastcall ECSSakura2Processor::mem_load_int8( const void * ptrMem )
{
	return	*((SBYTE*)ptrMem) ;
}

INT64 __fastcall ECSSakura2Processor::mem_load_float32( const void * ptrMem )
{
	REAL64	v = *((REAL32*)ptrMem) ;
	return	*((INT64*)&v) ;
}

INT64 __fastcall ECSSakura2Processor::mem_load_uint32( const void * ptrMem )
{
	return	*((DWORD*)ptrMem) ;
}

INT64 __fastcall ECSSakura2Processor::mem_load_uint16( const void * ptrMem )
{
	return	*((WORD*)ptrMem) ;
}

INT64 __fastcall ECSSakura2Processor::mem_load_uint8( const void * ptrMem )
{
	return	*((BYTE*)ptrMem) ;
}

// ストア関数
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::mem_store_int64( void * ptrMem, INT64 nData )
{
	#if	defined(__PLATFORM_ANDROID__)
		*((DWORD*)ptrMem) = (DWORD) nData ;
		*((DWORD*)((BYTE*) ptrMem + 4)) = (DWORD) (nData >> 32) ;
	#else
		*((INT64*)ptrMem) = nData ;
	#endif
}

void __fastcall ECSSakura2Processor::mem_store_int32( void * ptrMem, INT64 nData )
{
	*((SDWORD*)ptrMem) = (SDWORD) nData ;
}

void __fastcall ECSSakura2Processor::mem_store_int16( void * ptrMem, INT64 nData )
{
	*((SWORD*)ptrMem) = (SWORD) nData ;
}

void __fastcall ECSSakura2Processor::mem_store_int8( void * ptrMem, INT64 nData )
{
	*((SBYTE*)ptrMem) = (SBYTE) nData ;
}

void __fastcall ECSSakura2Processor::mem_store_float32( void * ptrMem, INT64 nData )
{
	*((REAL32*)ptrMem) = (REAL32) *((REAL64*)&nData) ;
}

void __fastcall ECSSakura2Processor::mem_store_uint32( void * ptrMem, INT64 nData )
{
	*((DWORD*)ptrMem) = (DWORD) nData ;
}

void __fastcall ECSSakura2Processor::mem_store_uint16( void * ptrMem, INT64 nData )
{
	*((WORD*)ptrMem) = (WORD) nData ;
}

void __fastcall ECSSakura2Processor::mem_store_uint8( void * ptrMem, INT64 nData )
{
	*((BYTE*)ptrMem) = (BYTE) nData ;
}


#define	SAKURA_LOAD_IMPLEMENT(type,perform)	\
	INT64		nValue ;	\
	const DWORD	dwHighAddress = (DWORD) (nAddress >> 32) ;	\
	SDWORD		dwOffset = (SDWORD) nAddress ;	\
	LinearAddressCache &	segCache = context->m_segLoadCache[dwHighAddress & 0x03] ;	\
	if ( segCache.highAddress == dwHighAddress ) { \
		dwOffset -= segCache.baseOffset ;	\
		if ( (DWORD) ((dwOffset >> 31) | (dwOffset + (sizeof(type)))) \
						<= segCache.limitSegment) { \
			const void *	ptrBuf = segCache.pbytBuffer + dwOffset ;	\
			perform ;			\
			return	nValue ;	} \
	} else { \
		if ( (context->m_pfnTranslate)( context, &segCache, nAddress ) != NULL ) { \
			dwOffset -= segCache.baseOffset ;	\
			if ( (DWORD) ((dwOffset >> 31) | (dwOffset + (sizeof(type)))) \
										<= segCache.limitSegment) { \
				const void *	ptrBuf = segCache.pbytBuffer + dwOffset ;	\
				perform ;	\
				return	nValue ;	} \
		} \
	} \
	ECSSakura2Processor::AtomicOr	\
		( &(context->m_maskException), ECSSakura2Processor::exceptionReadMemory ) ;	\
	return	0

#define	SAKURA_STORE_IMPLEMENT64(type,perform)	\
	const DWORD	dwHighAddress = (DWORD) (nAddress >> 32) ;	\
	SDWORD		dwOffset = (SDWORD) nAddress ;	\
	ECSSakura2Processor::LinearAddressCache &	\
				segCache = context->m_segStoreCache[dwHighAddress & 0x03] ;	\
	if ( segCache.highAddress == dwHighAddress ) { \
		dwOffset -= segCache.baseOffset ;	\
		if ( (DWORD) ((dwOffset >> 31) | (dwOffset + (sizeof(type)))) \
									<= segCache.limitSegment) { \
			const void *	ptrBuf = segCache.pbytBuffer + dwOffset ;	\
			perform ;	\
			return ;	} \
	} else { \
		if ( (context->m_pfnTranslate)( context, &segCache, nAddress ) != NULL ) { \
			dwOffset -= segCache.baseOffset ;	\
			if ( (DWORD) ((dwOffset >> 31) | (dwOffset + (sizeof(type)))) \
											<= segCache.limitSegment) { \
				const void *	ptrBuf = segCache.pbytBuffer + dwOffset ;	\
				perform ;	\
				return ;	} \
		} \
	} \
	ECSSakura2Processor::AtomicOr	\
		( &(context->m_maskException), ECSSakura2Processor::exceptionWriteMemory ) ;	\
	return

#define	SAKURA_STORE_IMPLEMENT(type,perform)	\
	const DWORD	dwHighAddress = (DWORD) (nAddress >> 32) ;	\
	SDWORD		dwOffset = (SDWORD) nAddress ;	\
	ECSSakura2Processor::LinearAddressCache &	\
				segCache = context->m_segStoreCache[dwHighAddress & 0x03] ;	\
	if ( segCache.highAddress == dwHighAddress ) { \
		dwOffset -= segCache.baseOffset ;	\
		if ( (DWORD) ((dwOffset >> 31) | (dwOffset + (sizeof(type)))) \
									<= segCache.limitSegment) { \
			const void *	ptrBuf = segCache.pbytBuffer + dwOffset ;	\
			*((type*)ptrBuf) = (type) perform ;	\
			return ;	} \
	} else { \
		if ( (context->m_pfnTranslate)( context, &segCache, nAddress ) != NULL ) { \
			dwOffset -= segCache.baseOffset ;	\
			if ( (DWORD) ((dwOffset >> 31) | (dwOffset + (sizeof(type)))) \
											<= segCache.limitSegment) { \
				const void *	ptrBuf = segCache.pbytBuffer + dwOffset ;	\
				*((type*)ptrBuf) = (type) perform ;	\
				return ;	} \
		} \
	} \
	ECSSakura2Processor::AtomicOr	\
		( &(context->m_maskException), ECSSakura2Processor::exceptionWriteMemory ) ;	\
	return

// ロード関数
//////////////////////////////////////////////////////////////////////////////
INT64 __fastcall ECSSakura2Processor::smem_load_int64
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	#if	defined(__PLATFORM_ANDROID__)
		SAKURA_LOAD_IMPLEMENT(INT64,nValue = ((INT64) *((DWORD*)((BYTE*) ptrBuf + 4)) << 32) | *((DWORD*)ptrBuf) ) ;
	#else
		SAKURA_LOAD_IMPLEMENT(INT64,nValue = *((INT64*)ptrBuf)) ;
	#endif
}

INT64 __fastcall ECSSakura2Processor::smem_load_int32
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	SAKURA_LOAD_IMPLEMENT(SDWORD,nValue = *((SDWORD*)ptrBuf)) ;
}

INT64 __fastcall ECSSakura2Processor::smem_load_int16
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	SAKURA_LOAD_IMPLEMENT(SWORD,nValue = *((SWORD*)ptrBuf)) ;
}

INT64 __fastcall ECSSakura2Processor::smem_load_int8
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	SAKURA_LOAD_IMPLEMENT(SBYTE,nValue = *((SBYTE*)ptrBuf)) ;
}

INT64 __fastcall ECSSakura2Processor::smem_load_float32
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	SAKURA_LOAD_IMPLEMENT(REAL32,REAL64 r = *((REAL32*)ptrBuf) ; nValue = *((INT64*)&r)) ;
}

INT64 __fastcall ECSSakura2Processor::smem_load_uint32
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	SAKURA_LOAD_IMPLEMENT(DWORD,nValue = *((DWORD*)ptrBuf)) ;
}

INT64 __fastcall ECSSakura2Processor::smem_load_uint16
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	SAKURA_LOAD_IMPLEMENT(WORD,nValue = *((WORD*)ptrBuf)) ;
}

INT64 __fastcall ECSSakura2Processor::smem_load_uint8
	( ECSSakura2Processor::Context * context, INT64 nAddress )
{
	SAKURA_LOAD_IMPLEMENT(BYTE,nValue = *((BYTE*)ptrBuf)) ;
}


// ストア関数
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::smem_store_int64
	( ECSSakura2Processor::Context * context, INT64 nAddress, INT64 nData )
{
	#if	defined(__PLATFORM_ANDROID__)
		SAKURA_STORE_IMPLEMENT64(INT64,*((DWORD*)ptrBuf) = (DWORD) nData ; *((DWORD*)((BYTE*)ptrBuf + 4)) = (DWORD) (nData >> 32)) ;
	#else
		SAKURA_STORE_IMPLEMENT(INT64,nData) ;
	#endif
}

void __fastcall ECSSakura2Processor::smem_store_int32
	( ECSSakura2Processor::Context * context, INT64 nAddress, INT64 nData )
{
	SAKURA_STORE_IMPLEMENT(DWORD,nData) ;
}

void __fastcall ECSSakura2Processor::smem_store_int16
	( ECSSakura2Processor::Context * context, INT64 nAddress, INT64 nData )
{
	SAKURA_STORE_IMPLEMENT(WORD,nData) ;
}

void __fastcall ECSSakura2Processor::smem_store_int8
	( ECSSakura2Processor::Context * context, INT64 nAddress, INT64 nData )
{
	SAKURA_STORE_IMPLEMENT(BYTE,nData) ;
}

void __fastcall ECSSakura2Processor::smem_store_float32
	( ECSSakura2Processor::Context * context, INT64 nAddress, INT64 nData )
{
	SAKURA_STORE_IMPLEMENT(REAL32,*((REAL64*)&nData)) ;
}



//////////////////////////////////////////////////////////////////////////////
// アドレス変換関数
//////////////////////////////////////////////////////////////////////////////

LinearAddressCache *
	ECSSakura2Processor::default_translate_address
		( Context * context,
			LinearAddressCache * plac, INT64 nAddress )
{
	const DWORD				dwHighAddr = (DWORD) (nAddress >> 32) ;
	const DWORD				iTLB = ((dwHighAddr >> 21) ^ dwHighAddr) & 0x1F ;
	const DWORD				maskTLB = 1 << iTLB ;
	LinearAddressCache &	tlb2ndCache = context->m_seg2ndCache[iTLB] ;
	if ( ((context->m_mask2ndTLB[0] & maskTLB) != 0)
		& (tlb2ndCache.highAddress == dwHighAddr) )
	{
		*plac = tlb2ndCache ;
		return	plac ;
	}
	if ( context->m_pSakura2VM->SegmentFromAddress
							( &tlb2ndCache, dwHighAddr ) != NULL )
	{
		context->m_mask2ndTLB[0] |= maskTLB ;
		*plac = tlb2ndCache ;
		return	plac ;
	}
	plac->limitSegment = 0 ;
	return	NULL ;
}

LinearAddressCache *
	ECSSakura2Processor::default_atomic_translate_address
		( Context * context,
			LinearAddressCache * plac, INT64 nAddress )
{
	return	context->m_pSakura2VM->
				AtomicSegmentFromAddress( plac, (DWORD) (nAddress >> 32) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 命令実行関数（C++ コードベース）
//////////////////////////////////////////////////////////////////////////////

#define	I_NULL	bad_instruction
#define	I_OBJ	object_mode_instruction

const INSTRUCTION_PROC	ECSSakura2Processor::pfnInstruction[0x100] =
{
	// 0x00
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x10
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x20
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x30
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x40
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x50
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x60
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x70
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ, I_OBJ,
	// 0x80
	load_base, load_base_imm32, load_base_index, load_base_index_imm32,
	// 0x84
	store_base, store_base_imm32, store_base_index, store_base_index_imm32,
	// 0x88
	load_local_imm32, load_local_index_imm32,
	store_local_imm32, store_local_index_imm32,
	I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x90
	move_reg_reg, I_NULL, cvt_float2int, cvt_int2float,
	// 0x94
	srl_reg_reg_imm8, sra_reg_reg_imm8, sll_reg_reg_imm8, maskmove_reg_reg_reg,
	// 0x98
	add_reg_reg_imm32, mul_reg_reg_imm32, add_sp_imm32, move_reg_imm64,
	// 0x9C
	neg_int, not_int, neg_float, I_NULL,
	// 0xA0
	add_reg_reg, sub_reg_reg, mul_reg_reg, div_reg_reg,
	mod_reg_reg, and_reg_reg, or_reg_reg, xor_reg_reg,
	srl_reg_reg, sra_reg_reg, sll_reg_reg, move_sx32_reg_reg,
	move_sx16_reg_reg, move_sx8_reg_reg, I_NULL, I_NULL,
	// 0xB0
	fadd_reg_reg, fsub_reg_reg, fmul_reg_reg, fdiv_reg_reg,
	I_NULL, I_NULL, I_NULL, I_NULL,
	mul32_reg_reg, imul32_reg_reg, div32_reg_reg, idiv32_reg_reg,
	mod32_reg_reg, imod32_reg_reg, I_NULL, I_NULL,
	// 0xC0
	cmp_ne, cmp_eq, cmp_lt, cmp_le,
	cmp_gt, cmp_ge, cmp_c, cmp_cz,
	// 0xC8
	fcmp_ne, fcmp_eq, fcmp_lt, fcmp_le,
	fcmp_gt, fcmp_ge, I_NULL, I_NULL,
	// 0xD0
	jump_offset32, jump_reg, cnjump_reg_offset32, cjump_reg_offset32,
	call_imm32, call_reg, syscall_imm32, syscall_reg,
	// 0xD8
	ret_nop, I_NULL, I_NULL, I_NULL,
	// 0xDC
	push_reg, pop_reg, pushs_reg_imm8, pops_reg_imm8,
	// 0xE0
	memory_hint, float_extensition, simd64_extensition_2op, simd64_extensition_3op,
	simd128_extensition_2op, simd128_extensition_3op, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	// 0xF0
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, escape_interruption, no_operation, trap_native_code,
} ;


#define	PREPARE(context)	\
	const BYTE * code = context->m_ptrCode + context->m_ip

#define	PREPARE_MEM(context)	\
	PREPARE(context) ; \
	const int	xbbbbddd = code[1] ; \
	const int	data_type = xbbbbddd & 0x07 ; \
	const int	base = (xbbbbddd >> 3) & 0x0F

#define	PREPARE_MEM_INDEX(context)	\
	PREPARE_MEM(context) ;	\
	const int	yiiiiiii = code[2] ;	\
	const int	index = yiiiiiii & 0x7F ;	\
	const int	scale = ((xbbbbddd >> 7) << 1) + (yiiiiiii >> 7)

#define	PREPARE_2OP(context)	\
	const BYTE * code = context->m_ptrCode + context->m_ip ;	\
	const int	dstreg = code[1] ;	\
	const int	srcreg = code[2] ;	\
	context->m_ip += 3


// ロード命令 0x80～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::load_base( Context * context )
{
	PREPARE_MEM(context) ;
	//
	context->m_regset[code[2]].i =
		(context->m_pfnLoad[data_type])( context, context->m_regset[base].i ) ;
	//
	context->m_ip += 3 ;
}

void __fastcall ECSSakura2Processor::load_base_imm32( Context * context )
{
	PREPARE_MEM(context) ;
	//
	context->m_regset[code[2]].i =
		(context->m_pfnLoad[data_type])
			( context, context->m_regset[base].i
							+ (INT64) *((SDWORD*)(code + 3)) ) ;
	//
	context->m_ip += 7 ;
}

void __fastcall ECSSakura2Processor::load_base_index( Context * context )
{
	PREPARE_MEM_INDEX(context) ;
	//
	context->m_regset[code[3]].i =
		(context->m_pfnLoad[data_type])
			( context, context->m_regset[base].i
					+ (context->m_regset[index].i << scale) ) ;
	//
	context->m_ip += 4 ;
}

void __fastcall ECSSakura2Processor::load_base_index_imm32( Context * context )
{
	PREPARE_MEM_INDEX(context) ;
	//
	context->m_regset[code[3]].i =
		(context->m_pfnLoad[data_type])
			( context, context->m_regset[base].i
					+ (context->m_regset[index].i << scale)
					+ (INT64) *((SDWORD*)(code + 4)) ) ;
	//
	context->m_ip += 8 ;
}

// ストア命令 0x84～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::store_base( Context * context )
{
	PREPARE_MEM(context) ;
	//
	(context->m_pfnStore[data_type])
		( context, context->m_regset[base].i, context->m_regset[code[2]].i ) ;
	//
	context->m_ip += 3 ;
}

void __fastcall ECSSakura2Processor::store_base_imm32( Context * context )
{
	PREPARE_MEM(context) ;
	//
	(context->m_pfnStore[data_type])
		( context,
			context->m_regset[base].i
			+ (INT64) *((SDWORD*)(code + 3)),
					context->m_regset[code[2]].i ) ;
	//
	context->m_ip += 7 ;
}

void __fastcall ECSSakura2Processor::store_base_index( Context * context )
{
	PREPARE_MEM_INDEX(context) ;
	//
	(context->m_pfnStore[data_type])
		( context, context->m_regset[base].i
				+ (context->m_regset[index].i << scale),
					context->m_regset[code[3]].i ) ;
	//
	context->m_ip += 4 ;
}

void __fastcall ECSSakura2Processor::store_base_index_imm32( Context * context )
{
	PREPARE_MEM_INDEX(context) ;
	//
	(context->m_pfnStore[data_type])
		( context, context->m_regset[base].i
				+ (context->m_regset[index].i << scale)
				+ (INT64) *((SDWORD*)(code + 4)), context->m_regset[code[3]].i ) ;
	//
	context->m_ip += 8 ;
}

// ローカルメモリ・ロード命令 0x88～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::load_local_imm32( Context * context )
{
	PREPARE(context) ;
	const int	data_type = code[1] & 0x07 ;
	const int	base_ptr = context->m_regset[regBP].l32
					+ *((SDWORD*)(code + 3)) - context->m_segStack.baseOffset ;
	if ( (base_ptr >= 0)
		& ((unsigned int) (base_ptr + sizeof_prim_data[data_type])
											<= context->m_segStack.limitSegment) )
	{
		context->m_regset[code[2]].i =
			(pfnLoadMemory[data_type])( context->m_segStack.pbytBuffer + base_ptr ) ;
		//
		context->m_ip += 7 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionReadMemory ) ;
	}
}

void __fastcall ECSSakura2Processor::load_local_index_imm32( Context * context )
{
	PREPARE(context) ;
	const int	xx000ddd = code[1] ;
	const int	data_type = xx000ddd & 0x07 ;
	const int	base_ptr = context->m_regset[regBP].l32
						+ (context->m_regset[code[2]].l32 << (xx000ddd >> 5))
						+ *((SDWORD*)(code + 4)) - context->m_segStack.baseOffset ;
	if ( (base_ptr >= 0)
		& ((unsigned int) (base_ptr + sizeof_prim_data[data_type])
											<= context->m_segStack.limitSegment) )
	{
		context->m_regset[code[3]].i =
			(pfnLoadMemory[data_type])( context->m_segStack.pbytBuffer + base_ptr ) ;
		//
		context->m_ip += 8 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionReadMemory ) ;
	}
}

// ローカルメモリ・ストア命令 0x8A～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::store_local_imm32( Context * context )
{
	PREPARE(context) ;
	const int	data_type = code[1] & 0x07 ;
	const int	base_ptr = context->m_regset[regBP].l32
						+ *((SDWORD*)(code + 3)) - context->m_segStack.baseOffset ;
	if ( (base_ptr >= 0)
		& ((unsigned int) (base_ptr + sizeof_prim_data[data_type])
											<= context->m_segStack.limitSegment) )
	{
		(pfnStoreMemory[data_type])
			( context->m_segStack.pbytBuffer + base_ptr, context->m_regset[code[2]].i ) ;
		//
		context->m_ip += 7 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionWriteMemory ) ;
	}
}

void __fastcall ECSSakura2Processor::store_local_index_imm32( Context * context )
{
	PREPARE(context) ;
	const int	xx000ddd = code[1] ;
	const int	data_type = xx000ddd & 0x07 ;
	const int	base_ptr = context->m_regset[regBP].l32
						+ (context->m_regset[code[2]].l32 << (xx000ddd >> 5))
						+ *((SDWORD*)(code + 4)) - context->m_segStack.baseOffset ;
	if ( (base_ptr >= 0)
		& ((unsigned int) (base_ptr + sizeof_prim_data[data_type])
											<= context->m_segStack.limitSegment) )
	{
		(pfnStoreMemory[data_type])
			( context->m_segStack.pbytBuffer + base_ptr, context->m_regset[code[3]].i ) ;
		//
		context->m_ip += 8 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionWriteMemory ) ;
	}
}

// ムーブ命令 0x90
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::move_reg_reg( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]] = context->m_regset[code[2]] ;
	//
	context->m_ip += 3 ;
}

// 整数・実数変換命令 0x92～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::cvt_float2int( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]].i =
		eslRoundR64ToLInt( context->m_regset[code[2]].f ) ;
	//
	context->m_ip += 3 ;
}

void __fastcall ECSSakura2Processor::cvt_int2float( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]].f =
					(REAL64) context->m_regset[code[2]].i ;
	//
	context->m_ip += 3 ;
}

// シフト命令 0x94～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::srl_reg_reg_imm8( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]].i =
			((UINT64) context->m_regset[code[2]].ui) >> code[3] ;
	//
	context->m_ip += 4 ;
}

void __fastcall ECSSakura2Processor::sra_reg_reg_imm8( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]].i =
			((INT64) context->m_regset[code[2]].i) >> code[3] ;
	//
	context->m_ip += 4 ;
}

void __fastcall ECSSakura2Processor::sll_reg_reg_imm8( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]].i =
			context->m_regset[code[2]].i << code[3] ;
	//
	context->m_ip += 4 ;
}

// マスク合成命令 0x97
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::maskmove_reg_reg_reg( Context * context )
{
	PREPARE(context) ;
	//
	INT64	mask = context->m_regset[code[3]].i ;
	//
	context->m_regset[code[1]].i =
		(context->m_regset[code[1]].i & ~mask)
				| (context->m_regset[code[2]].i & mask) ;
	//
	context->m_ip += 4 ;
}

// 32ビット即値演算命令 0x98～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::add_reg_reg_imm32( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]].i =
			context->m_regset[code[2]].i + *((SDWORD*)(code + 3)) ;
	//
	context->m_ip += 7 ;
}

void __fastcall ECSSakura2Processor::mul_reg_reg_imm32( Context * context )
{
	PREPARE(context) ;
	//
	context->m_regset[code[1]].i =
			context->m_regset[code[2]].i * *((SDWORD*)(code + 3)) ;
	//
	context->m_ip += 7 ;
}

// スタックレジスタ加算操作 0x9A
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::add_sp_imm32( Context * context )
{
	PREPARE(context) ;
	DWORD *	ptrSP = &(context->m_regset[regSP].l32) ;
	*ptrSP += *((SDWORD*)(code + 1)) ;
	context->m_ip += 5 ;
	//
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset ;
	if ( (unsigned int) stack_ptr > context->m_segStack.limitSegment )
	{
		AtomicOr( &(context->m_maskException), exceptionExtendStack ) ;
	}
	if ( stack_ptr & 0x07 )
	{
		AtomicOr( &(context->m_maskException), exceptionMisalignment ) ;
	}
}

// 64ビット即値読み込み命令 0x9B
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::move_reg_imm64( Context * context )
{
	PREPARE(context) ;
	//
	#if defined(__PLATFORM_ANDROID__)
		const int	reg = code[1] ;
		Register *	pImm = (Register*) (code + 2) ;
		context->m_regset[reg].l32 = pImm->l32 ;
		context->m_regset[reg].h32 = pImm->h32 ;
	#else
		context->m_regset[code[1]].i = *((INT64*)(code + 2)) ;
	#endif
	//
	context->m_ip += 10 ;
}

// 演算命令 0x9C～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::neg_int( Context * context )
{
	PREPARE(context) ;
	const int	dstreg = code[1] ;
	//
	context->m_regset[dstreg].i = - context->m_regset[dstreg].i ;
	//
	context->m_ip += 2 ;
}

void __fastcall ECSSakura2Processor::not_int( Context * context )
{
	PREPARE(context) ;
	const int	dstreg = code[1] ;
	//
	context->m_regset[dstreg].i = ~context->m_regset[dstreg].i ;
	//
	context->m_ip += 2 ;
}

void __fastcall ECSSakura2Processor::neg_float( Context * context )
{
	PREPARE(context) ;
	const int	dstreg = code[1] ;
	//
	context->m_regset[dstreg].f
			= - context->m_regset[dstreg].f ;
	//
	context->m_ip += 2 ;
}

// 整数演算命令 0xA0～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::add_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i += context->m_regset[srcreg].i ;
}

void __fastcall ECSSakura2Processor::sub_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i -= context->m_regset[srcreg].i ;
}

void __fastcall ECSSakura2Processor::mul_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i *= context->m_regset[srcreg].i ;
}

void __fastcall ECSSakura2Processor::div_reg_reg( Context * context )
{
	#if	!defined(__DISABLED_EXCEPTION__)
	try
	#endif
	{
		PREPARE_2OP(context) ;
		#if	defined(__DISABLED_EXCEPTION__)
		if ( context->m_regset[srcreg].i != 0 )
		#endif
		{
			context->m_regset[dstreg].i /= context->m_regset[srcreg].i ;
		}
		#if	defined(__DISABLED_EXCEPTION__)
		else
		{
			AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
		}
		#endif
	}
	#if	!defined(__DISABLED_EXCEPTION__)
	catch ( ... )
	{
		AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
	}
	#endif
}

void __fastcall ECSSakura2Processor::mod_reg_reg( Context * context )
{
	#if	!defined(__DISABLED_EXCEPTION__)
	try
	#endif
	{
		PREPARE_2OP(context) ;
		#if	defined(__DISABLED_EXCEPTION__)
		if ( context->m_regset[srcreg].i != 0 )
		#endif
		{
			context->m_regset[dstreg].i %= context->m_regset[srcreg].i ;
		}
		#if	defined(__DISABLED_EXCEPTION__)
		else
		{
			AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
		}
		#endif
	}
	#if	!defined(__DISABLED_EXCEPTION__)
	catch ( ... )
	{
		AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
	}
	#endif
}

void __fastcall ECSSakura2Processor::and_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i &= context->m_regset[srcreg].i ;
}

void __fastcall ECSSakura2Processor::or_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i |= context->m_regset[srcreg].i ;
}

void __fastcall ECSSakura2Processor::xor_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i ^= context->m_regset[srcreg].i ;
}

void __fastcall ECSSakura2Processor::srl_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i =
		(UINT64) context->m_regset[dstreg].ui
					>> (BYTE) (context->m_regset[srcreg].l32 & 0x3F) ;
}

void __fastcall ECSSakura2Processor::sra_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i =
		(INT64) context->m_regset[dstreg].i
					>> (BYTE) (context->m_regset[srcreg].l32 & 0x3F) ;
}

void __fastcall ECSSakura2Processor::sll_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i
					<<= (BYTE) (context->m_regset[srcreg].l32 & 0x3F) ;
}

// 整数符号拡張命令 0xAB～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::move_sx32_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i = (SDWORD) context->m_regset[srcreg].li32 ;
}

void __fastcall ECSSakura2Processor::move_sx16_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i = (SWORD) context->m_regset[srcreg].li32 ;
}

void __fastcall ECSSakura2Processor::move_sx8_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i = (SBYTE) context->m_regset[srcreg].li32 ;
}

// 実数演算命令 0xB0～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::fadd_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].f += context->m_regset[srcreg].f ;
}

void __fastcall ECSSakura2Processor::fsub_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].f -= context->m_regset[srcreg].f ;
}

void __fastcall ECSSakura2Processor::fmul_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].f *= context->m_regset[srcreg].f ;
}

void __fastcall ECSSakura2Processor::fdiv_reg_reg( Context * context )
{
	#if	!defined(__DISABLED_EXCEPTION__)
	try
	#endif
	{
		PREPARE_2OP(context) ;
		context->m_regset[dstreg].f /= context->m_regset[srcreg].f ;
	}
	#if	!defined(__DISABLED_EXCEPTION__)
	catch ( ... )
	{
		AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
	}
	#endif
}

// 特殊精度整数演算命令　0xB8～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::mul32_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i
			= (INT64) (DWORD) context->m_regset[dstreg].l32
						* (DWORD) context->m_regset[srcreg].l32 ;
}

void __fastcall ECSSakura2Processor::imul32_reg_reg( Context * context )
{
	PREPARE_2OP(context) ;
	context->m_regset[dstreg].i
			= (INT64) (SDWORD) context->m_regset[dstreg].li32
						* (SDWORD) context->m_regset[srcreg].li32 ;
}

void __fastcall ECSSakura2Processor::div32_reg_reg( Context * context )
{
	#if	!defined(__DISABLED_EXCEPTION__)
	try
	#endif
	{
		PREPARE_2OP(context) ;
		#if	defined(__DISABLED_EXCEPTION__)
		if ( (DWORD) context->m_regset[srcreg].l32
				> (DWORD) context->m_regset[dstreg].h32 )
		#endif
		{
			context->m_regset[dstreg].i
					= (UINT64) context->m_regset[dstreg].ui
								/ (DWORD) context->m_regset[srcreg].l32 ;
		}
		#if	defined(__DISABLED_EXCEPTION__)
		else
		{
			AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
		}
		#endif
	}
	#if	!defined(__DISABLED_EXCEPTION__)
	catch ( ... )
	{
		AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
	}
	#endif
}

void __fastcall ECSSakura2Processor::idiv32_reg_reg( Context * context )
{
	#if	!defined(__DISABLED_EXCEPTION__)
	try
	#endif
	{
		PREPARE_2OP(context) ;
		#if	defined(__DISABLED_EXCEPTION__)
		if ( (SDWORD) context->m_regset[srcreg].li32 != 0 )
		#endif
		{
			context->m_regset[dstreg].i
					/= (SDWORD) context->m_regset[srcreg].li32 ;
		}
		#if	defined(__DISABLED_EXCEPTION__)
		else
		{
			AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
		}
		#endif
	}
	#if	!defined(__DISABLED_EXCEPTION__)
	catch ( ... )
	{
		AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
	}
	#endif
}

void __fastcall ECSSakura2Processor::mod32_reg_reg( Context * context )
{
	#if	!defined(__DISABLED_EXCEPTION__)
	try
	#endif
	{
		PREPARE_2OP(context) ;
		#if	defined(__DISABLED_EXCEPTION__)
		if ( (DWORD) context->m_regset[srcreg].l32
				> (DWORD) context->m_regset[dstreg].h32 )
		#endif
		{
			context->m_regset[dstreg].i
					= (UINT64) context->m_regset[dstreg].ui
								% (DWORD) context->m_regset[srcreg].l32 ;
		}
		#if	defined(__DISABLED_EXCEPTION__)
		else
		{
			AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
		}
		#endif
	}
	#if	!defined(__DISABLED_EXCEPTION__)
	catch ( ... )
	{
		AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
	}
	#endif
}

void __fastcall ECSSakura2Processor::imod32_reg_reg( Context * context )
{
	#if	!defined(__DISABLED_EXCEPTION__)
	try
	#endif
	{
		PREPARE_2OP(context) ;
		#if	defined(__DISABLED_EXCEPTION__)
		if ( (SDWORD) context->m_regset[srcreg].li32 != 0 )
		#endif
		{
			context->m_regset[dstreg].i
					%= (SDWORD) context->m_regset[srcreg].li32 ;
		}
		#if	defined(__DISABLED_EXCEPTION__)
		else
		{
			AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
		}
		#endif
	}
	#if	!defined(__DISABLED_EXCEPTION__)
	catch ( ... )
	{
		AtomicOr( &(context->m_maskException), exceptionZeroDivision ) ;
	}
	#endif
}

// 整数比較命令 0xC0～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::cmp_ne( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].i
								!= context->m_regset[srcreg].i) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::cmp_eq( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].i
								== context->m_regset[srcreg].i) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::cmp_lt( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].i
								< context->m_regset[srcreg].i) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::cmp_le( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].i
								<= context->m_regset[srcreg].i) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::cmp_gt( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].i
								> context->m_regset[srcreg].i) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::cmp_ge( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].i
								>= context->m_regset[srcreg].i) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::cmp_c( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) ((UINT64) context->m_regset[dstreg].ui
								< (UINT64) context->m_regset[srcreg].ui) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::cmp_cz( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) ((UINT64) context->m_regset[dstreg].ui
								<= (UINT64) context->m_regset[srcreg].ui) ;
	context->m_regset[dstreg].i = - s ;
}

// 実数比較命令 0xC8～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::fcmp_ne( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].f
								!= context->m_regset[srcreg].f) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::fcmp_eq( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].f
								== context->m_regset[srcreg].f) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::fcmp_lt( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].f
								< context->m_regset[srcreg].f) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::fcmp_le( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].f
								<= context->m_regset[srcreg].f) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::fcmp_gt( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].f
								> context->m_regset[srcreg].f) ;
	context->m_regset[dstreg].i = - s ;
}

void __fastcall ECSSakura2Processor::fcmp_ge( Context * context )
{
	PREPARE_2OP(context) ;
	const int	s = (int) (context->m_regset[dstreg].f
								>= context->m_regset[srcreg].f) ;
	context->m_regset[dstreg].i = - s ;
}

// 相対無条件ジャンプ命令 0xD0
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::jump_offset32( Context * context )
{
	PREPARE(context) ;
	context->m_ip += *((SDWORD*)(code + 1)) + 5 ;
}

// 間接無条件ジャンプ命令 0xD1
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::jump_reg( Context * context )
{
	PREPARE(context) ;
	const int	reg = code[1] ;
	const DWORD	ipLastSegment = context->m_ipSegment ;
	context->m_ip = (DWORD) context->m_regset[reg].l32 ;
	context->m_ipSegment = (DWORD) context->m_regset[reg].h32 ;
	//
	if ( ipLastSegment != context->m_ipSegment )
	{
		AtomicOr( &(context->m_maskException), exceptionFarJump ) ;
	}
}

// 相対条件ジャンプ命令 0xD2
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::cnjump_reg_offset32( Context * context )
{
	PREPARE(context) ;
	if ( !((int) context->m_regset[code[1]].l32 & 0x01) )
	{
		context->m_ip += *((SDWORD*)(code + 2)) + 6 ;
	}
	else
	{
		context->m_ip += 6 ;
	}
}

void __fastcall ECSSakura2Processor::cjump_reg_offset32( Context * context )
{
	PREPARE(context) ;
	if ( (int) context->m_regset[code[1]].l32 & 0x01 )
	{
		context->m_ip += *((SDWORD*)(code + 2)) + 6 ;
	}
	else
	{
		context->m_ip += 6 ;
	}
}

// コール命令 0xD4
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::call_imm32( Context * context )
{
	PREPARE(context) ;
	DWORD *		ptrSP = (DWORD*) &(context->m_regset[regSP]) ;
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset - 8 ;
	if ( (unsigned int) stack_ptr < context->m_segStack.limitSegment )
	{
		// PUSH IP
		*ptrSP -= 8 ;
		*((INT64*)(context->m_segStack.pbytBuffer + stack_ptr)) =
			(context->m_ip + 5) | ((INT64) context->m_ipSegment << 32) ;
		//
		// JUMP offset
		context->m_ip = *((DWORD*)(code + 1)) ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionExtendStack ) ;
	}
}

// 間接コール命令 0xD5
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::call_reg( Context * context )
{
	PREPARE(context) ;
	DWORD *		ptrSP = (DWORD*) &(context->m_regset[regSP]) ;
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset - 8 ;
	if ( (unsigned int) stack_ptr < context->m_segStack.limitSegment )
	{
		// PUSH IP
		*ptrSP -= 8 ;
		*((INT64*)(context->m_segStack.pbytBuffer + stack_ptr)) =
			(context->m_ip + 2) | ((INT64) context->m_ipSegment << 32) ;
		//
		// JUMP far reg
		const int	reg = code[1] ;
		const DWORD	ipLastSegment = context->m_ipSegment ;
		context->m_ip = (DWORD) context->m_regset[reg].l32 ;
		context->m_ipSegment = (DWORD) context->m_regset[reg].h32 ;
		//
		if ( ipLastSegment != context->m_ipSegment )
		{
			AtomicOr( &(context->m_maskException), exceptionFarJump ) ;
		}
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionExtendStack ) ;
	}
}

// システムコール命令 0xD6
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::syscall_imm32( Context * context )
{
	PREPARE(context) ;
	context->m_idSystemCall = *((DWORD*)(code + 1)) ;
	context->m_ip += 5 ;
	//
	AtomicOr( &(context->m_maskException), exceptionSystemCall ) ;
}

// 間接システムコール命令 0xD7
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::syscall_reg( Context * context )
{
	PREPARE(context) ;
	context->m_idSystemCall = (DWORD) context->m_regset[code[1]].l32 ;
	context->m_ip += 2 ;
	//
	AtomicOr( &(context->m_maskException), exceptionSystemCall ) ;
}

// リターン命令 0xD8
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::ret_nop( Context * context )
{
	PREPARE(context) ;
	DWORD *		ptrSP = (DWORD*) &(context->m_regset[regSP]) ;
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset ;
	if ( (unsigned int) stack_ptr < context->m_segStack.limitSegment )
	{
		const INT64	ipReturn = *((INT64*)(context->m_segStack.pbytBuffer + stack_ptr)) ;
		*ptrSP += 8 ;
		//
		const DWORD	ipLastSegment = context->m_ipSegment ;
		context->m_ip = (DWORD) ipReturn ;
		context->m_ipSegment = (DWORD) (ipReturn >> 32) ;
		//
		if ( ipLastSegment != context->m_ipSegment )
		{
			AtomicOr( &(context->m_maskException), exceptionFarJump ) ;
		}
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionReadMemory ) ;
	}
}

// PUSH/POP 命令 0xDC～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::push_reg( Context * context )
{
	PREPARE(context) ;
	DWORD *		ptrSP = &(context->m_regset[regSP].l32) ;
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset - 8 ;
	if ( (unsigned int) stack_ptr < context->m_segStack.limitSegment )
	{
		*ptrSP -= 8 ;
		*((INT64*)(context->m_segStack.pbytBuffer + stack_ptr)) =
								context->m_regset[code[1]].i ;
		//
		context->m_ip += 2 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionExtendStack ) ;
	}
}

void __fastcall ECSSakura2Processor::pop_reg( Context * context )
{
	PREPARE(context) ;
	DWORD *		ptrSP = &(context->m_regset[regSP].l32) ;
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset ;
	if ( (unsigned int) stack_ptr < context->m_segStack.limitSegment )
	{
		context->m_regset[code[1]].i =
				*((INT64*)(context->m_segStack.pbytBuffer + stack_ptr)) ;
		*ptrSP += 8 ;
		//
		context->m_ip += 2 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionReadMemory ) ;
	}
}

void __fastcall ECSSakura2Processor::pushs_reg_imm8( Context * context )
{
	PREPARE(context) ;
	DWORD *		ptrSP = &(context->m_regset[regSP].l32) ;
	const int	reg_first = code[1] ;
	const int	count = code[2] ;
	const int	bytes_push = (count << 3) ;
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset - bytes_push ;
	if ( (unsigned int) stack_ptr < context->m_segStack.limitSegment )
	{
		INT64*	ptrStackTop = (INT64*) (context->m_segStack.pbytBuffer + stack_ptr) ;
		*ptrSP -= bytes_push ;
		for ( int i = 0; i < count; i ++ )
		{
			ptrStackTop[i] = context->m_regset[reg_first + i].i ;
		}
		context->m_ip += 3 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionExtendStack ) ;
	}
}

void __fastcall ECSSakura2Processor::pops_reg_imm8( Context * context )
{
	PREPARE(context) ;
	DWORD *		ptrSP = &(context->m_regset[regSP].l32) ;
	const int	reg_first = code[1] ;
	const int	count = code[2] ;
	const int	bytes_pop = (count << 3) ;
	const int	stack_ptr = *ptrSP - context->m_segStack.baseOffset ;
	if ( (unsigned int) stack_ptr < context->m_segStack.limitSegment )
	{
		const INT64*	ptrStackTop =
				(const INT64*) (context->m_segStack.pbytBuffer + stack_ptr) ;
		for ( int i = 0; i < count; i ++ )
		{
			context->m_regset[reg_first + i].i = ptrStackTop[i] ;
		}
		*ptrSP += bytes_pop ;
		context->m_ip += 3 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionReadMemory ) ;
	}
}

// MEMORY HINT 命令 0xE0
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::memory_hint( Context * context )
{
	context->m_ip += 3 ;
}

// FLOAT EXTENSION 命令 0xE1
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::float_extensition( Context * context )
{
	PREPARE(context) ;
	//
	OPERATION_DST_SRC_PROC	pfnOperation = pfnFloatOperationDstSrc[code[1]] ;
	if ( pfnOperation != NULL )
	{
		(*pfnOperation)
			( &(context->m_regset[code[2]]), &(context->m_regset[code[3]]) ) ;
		context->m_ip += 4 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionBadInstruction ) ;
	}
}

// 64bit SIMD EXTENSION 2OP 命令 0xE2
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd64_extensition_2op( Context * context )
{
	PREPARE(context) ;
	//
	OPERATION_DST_SRC_PROC	pfnOperation = pfnSIMD64_OperationDstSrc[code[1]] ;
	if ( pfnOperation != NULL )
	{
		(*pfnOperation)
			( &(context->m_regset[code[2]]), &(context->m_regset[code[3]]) ) ;
		context->m_ip += 4 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionBadInstruction ) ;
	}
}

// 64bit SIMD EXTENSION 3OP 命令 0xE3
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd64_extensition_3op( Context * context )
{
	PREPARE(context) ;
	//
	OPERATION_DST_SRC_IMM_PROC
			pfnOperation = pfnSIMD64_OperationDstSrcImm8[code[1]] ;
	if ( pfnOperation != NULL )
	{
		(*pfnOperation)
			( &(context->m_regset[code[2]]),
				&(context->m_regset[code[3]]), code[4] ) ;
		context->m_ip += 5 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionBadInstruction ) ;
	}
}

// 128bit SIMD EXTENSION 2OP 命令 0xE4
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd128_extensition_2op( Context * context )
{
	PREPARE(context) ;
	//
	OPERATION_SIMD128_DST_SRC_PROC
			pfnOperation = pfnSIMD128_OperationDstSrc[code[1]] ;
	if ( pfnOperation != NULL )
	{
		(*pfnOperation)
			( (Register128*) &(context->m_regset[code[2]]),
				(Register128*) &(context->m_regset[code[3]]) ) ;
		context->m_ip += 4 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionBadInstruction ) ;
	}
}

// 128bit SIMD EXTENSION 3OP 命令 0xE5
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd128_extensition_3op( Context * context )
{
	PREPARE(context) ;
	//
	OPERATION_SIMD128_DST_SRC_IMM_PROC
			pfnOperation = pfnSIMD128_OperationDstSrcImm8[code[1]] ;
	if ( pfnOperation != NULL )
	{
		(*pfnOperation)
			( context,
				(Register128*) &(context->m_regset[code[2]]),
				(Register128*) &(context->m_regset[code[3]]), code[4] ) ;
		context->m_ip += 5 ;
	}
	else
	{
		AtomicOr( &(context->m_maskException), exceptionBadInstruction ) ;
	}
}

// エスケープ命令 0xFD
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::escape_interruption( Context * context )
{
	AtomicOr( &(context->m_maskException), interruptEscape ) ;
}

// NOP 命令 0xFE
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::no_operation( Context * context )
{
	context->m_ip += 1 ;
}

// 不正命令
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::bad_instruction( Context * context )
{
	AtomicOr( &(context->m_maskException), exceptionBadInstruction ) ;
}

// ネイティブコード化トラップ
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::trap_native_code( Context * context )
{
	INSTRUCTION_PROC	pfnNative =
			*((INSTRUCTION_PROC*)(context->m_ptrTrick + context->m_ip)) ;
	pfnNative( context ) ;
}

// オブジェクト命令
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::object_mode_instruction( Context * context )
{
	AtomicOr( &(context->m_maskException), exceptionObjectMode ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 64bit 浮動小数点演算関数
//////////////////////////////////////////////////////////////////////////////

// 命令テーブル
const ECSSakura2Processor::OPERATION_DST_SRC_PROC
		ECSSakura2Processor::pfnFloatOperationDstSrc[0x100] =
{
	// 0x00
	float64_abs, float64_log, float64_pow, float64_sqrt,
	float64_sin, float64_cos, float64_tan, float64_asin,
	float64_acos, float64_atan, float64_round, float64_floor,
	NULL, NULL, NULL, NULL,
	// 0x10
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

void __fastcall ECSSakura2Processor::float64_abs( Register * dst, const Register * src )
{
	dst->f = fabs( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_log( Register * dst, const Register * src )
{
	dst->f = log( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_pow( Register * dst, const Register * src )
{
	dst->f = pow( dst->f, src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_sqrt( Register * dst, const Register * src )
{
	dst->f = sqrt( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_sin( Register * dst, const Register * src )
{
	dst->f = sin( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_cos( Register * dst, const Register * src )
{
	dst->f = cos( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_tan( Register * dst, const Register * src )
{
	dst->f = tan( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_asin( Register * dst, const Register * src )
{
	dst->f = asin( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_acos( Register * dst, const Register * src )
{
	dst->f = acos( src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_atan( Register * dst, const Register * src )
{
	dst->f = atan2( dst->f, src->f ) ;
}

void __fastcall ECSSakura2Processor::float64_round( Register * dst, const Register * src )
{
	dst->i = eslRoundR64ToLInt( src->f ) ; ;
}

void __fastcall ECSSakura2Processor::float64_floor( Register * dst, const Register * src )
{
	dst->i = eslRoundR64ToLInt( floor( src->f ) ) ; ;
}


//////////////////////////////////////////////////////////////////////////////
// 64bit SIMD 処理関数
//////////////////////////////////////////////////////////////////////////////

// 命令テーブル
const ECSSakura2Processor::OPERATION_DST_SRC_PROC
			ECSSakura2Processor::pfnSIMD64_OperationDstSrc[0x100] =
{
	// 0x00
	simd_paddub, simd_paddsb, simd_paddb, simd_padduw,
	simd_paddsw, simd_paddw, simd_paddd, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x10
	simd_psubub, simd_psubsb, simd_psubb, simd_psubuw,
	simd_psubsw, simd_psubw, simd_psubd, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	simd_psrlw, simd_psrld, NULL, NULL,
	NULL, NULL, NULL, NULL,
	simd_psraw, simd_psrad, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x30
	simd_psllw, simd_pslld, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	simd_pcmp_ne_sb, simd_pcmp_ne_sw, simd_pcmp_ne_sd, NULL,
	NULL, NULL, NULL, NULL,
	simd_pcmp_eq_sb, simd_pcmp_eq_sw, simd_pcmp_eq_sd, NULL,
	NULL, NULL, NULL, NULL,
	// 0x50
	simd_pcmp_lt_sb, simd_pcmp_lt_sw, simd_pcmp_lt_sd, NULL,
	NULL, NULL, NULL, NULL,
	simd_pcmp_le_sb, simd_pcmp_le_sw, simd_pcmp_le_sd, NULL,
	NULL, NULL, NULL, NULL,
	// 0x60
	simd_pcmp_gt_sb, simd_pcmp_gt_sw, simd_pcmp_gt_sd, NULL,
	NULL, NULL, NULL, NULL,
	simd_pcmp_ge_sb, simd_pcmp_ge_sw, simd_pcmp_ge_sd, NULL,
	NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	simd_pmullw, simd_pmulhsw, simd_pmulhusw, simd_pmaddwd,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x90
	simd_punpack_lbw, simd_punpack_lwd, simd_punpack_ldq, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0xA0
	simd_pcvt_swb, simd_pcvt_uswb, simd_pcvt_sdw, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

const ECSSakura2Processor::OPERATION_DST_SRC_IMM_PROC
			ECSSakura2Processor::pfnSIMD64_OperationDstSrcImm8[0x100] =
{
	// 0x00
	simd_psrlw_imm8, simd_psrld_imm8, NULL, NULL,
	NULL, NULL, NULL, NULL,
	simd_psraw_imm8, simd_psrad_imm8, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x10
	simd_psllw_imm8, simd_pslld_imm8, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x20
	simd_pshufw_imm8, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

// 加減算命令 0xE2 0x00～
//////////////////////////////////////////////////////////////////////////////
inline UINT64 unsigned_packed_add( UINT64 src1, UINT64 src2, UINT64 mask, int bs )
{
	UINT64	ld = (src1 & mask) + (src2 & mask) ;
	UINT64	hd = ((src1 >> bs) & mask) + ((src2 >> bs) & mask) ;
	UINT64	sat = (((mask - ld) >> bs) & mask) | ((mask - hd) & (mask << bs)) ;
	return	(ld & mask) | ((hd & mask) << bs) | sat ;
}

inline int signed_saturated_add( int src1, int src2, int bias, int mask )
{
	int	d = src1 + src2 + bias ;
	int	s = (d >> 31) ;
	d &= ~s ;
	s = (mask - d) >> 31 ;
	return	((d | s) & mask) - bias ;
}

inline UINT64 unsigned_packed_sub
	( UINT64 src1, UINT64 src2, UINT64 mask, UINT64 cr, int bs )
{
	UINT64	ld = ((src1 & mask) | cr) - (src2 & mask) ;
	UINT64	hd = (((src1 >> bs) & mask) | cr) - ((src2 >> bs) & mask) ;
	UINT64	ldm = ld & cr ;
	UINT64	hdm = hd & cr ;
	ldm -= (ldm >> bs) ;
	hdm -= (hdm >> bs) ;
	ld &= ldm ;
	hd &= hdm ;
	return	ld | (hd << bs) ;
}

inline int signed_saturated_sub( int src1, int src2, int bias, int mask )
{
	int	d = src1 - src2 + bias ;
	int	s = (d >> 31) ;
	d &= ~s ;
	s = (mask - d) >> 31 ;
	return	((d | s) & mask) - bias ;
}

void __fastcall ECSSakura2Processor::simd_paddub( Register * dst, const Register * src )
{
	dst->i = unsigned_packed_add( dst->i, src->i, 0x00FF00FF00FF00FF, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_paddsb( Register * dst, const Register * src )
{
	SBYTE *			pbd = (SBYTE*) dst ;
	const SBYTE *	pbs = (const SBYTE*) src ;
	pbd[0] = (SBYTE) signed_saturated_add( pbd[0], pbs[0], 0x80, 0xFF ) ;
	pbd[1] = (SBYTE) signed_saturated_add( pbd[1], pbs[1], 0x80, 0xFF ) ;
	pbd[2] = (SBYTE) signed_saturated_add( pbd[2], pbs[2], 0x80, 0xFF ) ;
	pbd[3] = (SBYTE) signed_saturated_add( pbd[3], pbs[3], 0x80, 0xFF ) ;
	pbd[4] = (SBYTE) signed_saturated_add( pbd[4], pbs[4], 0x80, 0xFF ) ;
	pbd[5] = (SBYTE) signed_saturated_add( pbd[5], pbs[5], 0x80, 0xFF ) ;
	pbd[6] = (SBYTE) signed_saturated_add( pbd[6], pbs[6], 0x80, 0xFF ) ;
	pbd[7] = (SBYTE) signed_saturated_add( pbd[7], pbs[7], 0x80, 0xFF ) ;
}

void __fastcall ECSSakura2Processor::simd_paddb( Register * dst, const Register * src )
{
	BYTE *			pbd = (BYTE*) dst ;
	const BYTE *	pbs = (const BYTE*) src ;
	pbd[0] += pbs[0] ;
	pbd[1] += pbs[1] ;
	pbd[2] += pbs[2] ;
	pbd[3] += pbs[3] ;
	pbd[4] += pbs[4] ;
	pbd[5] += pbs[5] ;
	pbd[6] += pbs[6] ;
	pbd[7] += pbs[7] ;
}

void __fastcall ECSSakura2Processor::simd_padduw( Register * dst, const Register * src )
{
	dst->i = unsigned_packed_add( dst->i, src->i, 0x0000FFFF0000FFFF, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_paddsw( Register * dst, const Register * src )
{
	SWORD *			pwd = (SWORD*) dst ;
	const SWORD *	pws = (const SWORD*) src ;
	pwd[0] = (SWORD) signed_saturated_add( pwd[0], pws[0], 0x8000, 0xFFFF ) ;
	pwd[1] = (SWORD) signed_saturated_add( pwd[1], pws[1], 0x8000, 0xFFFF ) ;
	pwd[2] = (SWORD) signed_saturated_add( pwd[2], pws[2], 0x8000, 0xFFFF ) ;
	pwd[3] = (SWORD) signed_saturated_add( pwd[3], pws[3], 0x8000, 0xFFFF ) ;
}

void __fastcall ECSSakura2Processor::simd_paddw( Register * dst, const Register * src )
{
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	pwd[0] += pws[0] ;
	pwd[1] += pws[1] ;
	pwd[2] += pws[2] ;
	pwd[3] += pws[3] ;
}

void __fastcall ECSSakura2Processor::simd_paddd( Register * dst, const Register * src )
{
	dst->l32 += src->l32 ;
	dst->h32 += src->h32 ;
}

void __fastcall ECSSakura2Processor::simd_psubub( Register * dst, const Register * src )
{
	dst->i = unsigned_packed_sub
				( dst->i, src->i, 0x00FF00FF00FF00FF, 0x0100010001000100, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_psubsb( Register * dst, const Register * src )
{
	SBYTE *			pbd = (SBYTE*) dst ;
	const SBYTE *	pbs = (const SBYTE*) src ;
	pbd[0] = (SBYTE) signed_saturated_sub( pbd[0], pbs[0], 0x80, 0xFF ) ;
	pbd[1] = (SBYTE) signed_saturated_sub( pbd[1], pbs[1], 0x80, 0xFF ) ;
	pbd[2] = (SBYTE) signed_saturated_sub( pbd[2], pbs[2], 0x80, 0xFF ) ;
	pbd[3] = (SBYTE) signed_saturated_sub( pbd[3], pbs[3], 0x80, 0xFF ) ;
	pbd[4] = (SBYTE) signed_saturated_sub( pbd[4], pbs[4], 0x80, 0xFF ) ;
	pbd[5] = (SBYTE) signed_saturated_sub( pbd[5], pbs[5], 0x80, 0xFF ) ;
	pbd[6] = (SBYTE) signed_saturated_sub( pbd[6], pbs[6], 0x80, 0xFF ) ;
	pbd[7] = (SBYTE) signed_saturated_sub( pbd[7], pbs[7], 0x80, 0xFF ) ;
}

void __fastcall ECSSakura2Processor::simd_psubb( Register * dst, const Register * src )
{
	BYTE *			pbd = (BYTE*) dst ;
	const BYTE *	pbs = (const BYTE*) src ;
	pbd[0] -= pbs[0] ;
	pbd[1] -= pbs[1] ;
	pbd[2] -= pbs[2] ;
	pbd[3] -= pbs[3] ;
	pbd[4] -= pbs[4] ;
	pbd[5] -= pbs[5] ;
	pbd[6] -= pbs[6] ;
	pbd[7] -= pbs[7] ;
}

void __fastcall ECSSakura2Processor::simd_psubuw( Register * dst, const Register * src )
{
	dst->i = unsigned_packed_sub
				( dst->i, src->i, 0x0000FFFF0000FFFF, 0x0001000000010000, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_psubsw( Register * dst, const Register * src )
{
	SWORD *			pwd = (SWORD*) dst ;
	const SWORD *	pws = (const SWORD*) src ;
	pwd[0] = (SWORD) signed_saturated_sub( pwd[0], pws[0], 0x8000, 0xFFFF ) ;
	pwd[1] = (SWORD) signed_saturated_sub( pwd[1], pws[1], 0x8000, 0xFFFF ) ;
	pwd[2] = (SWORD) signed_saturated_sub( pwd[2], pws[2], 0x8000, 0xFFFF ) ;
	pwd[3] = (SWORD) signed_saturated_sub( pwd[3], pws[3], 0x8000, 0xFFFF ) ;
}

void __fastcall ECSSakura2Processor::simd_psubw( Register * dst, const Register * src )
{
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	pwd[0] -= pws[0] ;
	pwd[1] -= pws[1] ;
	pwd[2] -= pws[2] ;
	pwd[3] -= pws[3] ;
}

void __fastcall ECSSakura2Processor::simd_psubd( Register * dst, const Register * src )
{
	dst->l32 -= src->l32 ;
	dst->h32 -= src->h32 ;
}

// シフト命令 0xE2 0x20～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_psrlw( Register * dst, const Register * src )
{
	const int	s = src->l32 & 0x0F ;
	WORD *		pwd = (WORD*) dst ;
	pwd[0] >>= s ;
	pwd[1] >>= s ;
	pwd[2] >>= s ;
	pwd[3] >>= s ;
}

void __fastcall ECSSakura2Processor::simd_psraw( Register * dst, const Register * src )
{
	const int	s = src->l32 & 0x0F ;
	SWORD *		pwd = (SWORD*) dst ;
	pwd[0] >>= s ;
	pwd[1] >>= s ;
	pwd[2] >>= s ;
	pwd[3] >>= s ;
}

void __fastcall ECSSakura2Processor::simd_psllw( Register * dst, const Register * src )
{
	const int	s = src->l32 & 0x0F ;
	WORD *		pwd = (WORD*) dst ;
	pwd[0] <<= s ;
	pwd[1] <<= s ;
	pwd[2] <<= s ;
	pwd[3] <<= s ;
}

void __fastcall ECSSakura2Processor::simd_psrld( Register * dst, const Register * src )
{
	const int	s = src->l32 & 0x1F ;
	dst->l32 >>= s ;
	dst->h32 >>= s ;
}

void __fastcall ECSSakura2Processor::simd_psrad( Register * dst, const Register * src )
{
	const int	s = src->l32 & 0x1F ;
	dst->li32 >>= s ;
	dst->hi32 >>= s ;
}

void __fastcall ECSSakura2Processor::simd_pslld( Register * dst, const Register * src )
{
	const int	s = src->l32 & 0x1F ;
	dst->l32 <<= s ;
	dst->h32 <<= s ;
}

// 比較命令 0xE2 0x40～
//////////////////////////////////////////////////////////////////////////////
inline UINT64 unsigned_packed_compare_ne
	( UINT64 src1, UINT64 src2, UINT64 mask, UINT64 cr, int bs )
{
	UINT64	d = src1 ^ src2 ;
	UINT64	ld = (d & mask) + mask ;
	UINT64	hd = ((d >> bs) & mask) + mask ;
	UINT64	ldm = ld & cr ;
	UINT64	hdm = hd & cr ;
	ldm -= (ldm >> bs) ;
	hdm -= (hdm >> bs) ;
	return	ldm | (hdm << bs) ;
}

inline UINT64 unsigned_packed_compare_ge
	( UINT64 src1, UINT64 src2, UINT64 mask, UINT64 cr, int bs )
{
	UINT64	ld = ((src1 & mask) | cr) - (src2 & mask) ;
	UINT64	hd = (((src1 >> bs) & mask) | cr) - ((src2 >> bs) & mask) ;
	UINT64	ldm = ld & cr ;
	UINT64	hdm = hd & cr ;
	ldm -= (ldm >> bs) ;
	hdm -= (hdm >> bs) ;
	return	ldm | (hdm << bs) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_ne_sb( Register * dst, const Register * src )
{
	dst->i = unsigned_packed_compare_ne
				( dst->i, src->i, 0x00FF00FF00FF00FF, 0x0100010001000100, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_eq_sb( Register * dst, const Register * src )
{
	dst->i = ~unsigned_packed_compare_ne
				( dst->i, src->i, 0x00FF00FF00FF00FF, 0x0100010001000100, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_lt_sb( Register * dst, const Register * src )
{
	const UINT64	s = 0x8080808080808080 ;
	dst->i = ~unsigned_packed_compare_ge
				( dst->i ^ s, src->i ^ s, 0x00FF00FF00FF00FF, 0x0100010001000100, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_le_sb( Register * dst, const Register * src )
{
	const UINT64	s = 0x8080808080808080 ;
	dst->i = unsigned_packed_compare_ge
				( src->i ^ s, dst->i ^ s, 0x00FF00FF00FF00FF, 0x0100010001000100, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_gt_sb( Register * dst, const Register * src )
{
	const UINT64	s = 0x8080808080808080 ;
	dst->i = ~unsigned_packed_compare_ge
				( src->i ^ s, dst->i ^ s, 0x00FF00FF00FF00FF, 0x0100010001000100, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_ge_sb( Register * dst, const Register * src )
{
	const UINT64	s = 0x8080808080808080 ;
	dst->i = unsigned_packed_compare_ge
				( dst->i ^ s, src->i ^ s, 0x00FF00FF00FF00FF, 0x0100010001000100, 8 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_ne_sw( Register * dst, const Register * src )
{
	dst->i = unsigned_packed_compare_ne
				( dst->i, src->i, 0x0000FFFF0000FFFF, 0x0001000000010000, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_eq_sw( Register * dst, const Register * src )
{
	dst->i = ~unsigned_packed_compare_ne
				( dst->i, src->i, 0x0000FFFF0000FFFF, 0x0001000000010000, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_lt_sw( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000800080008000 ;
	dst->i = ~unsigned_packed_compare_ge
				( dst->i ^ s, src->i ^ s,
					0x0000FFFF0000FFFF, 0x0001000000010000, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_le_sw( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000800080008000 ;
	dst->i = unsigned_packed_compare_ge
				( src->i ^ s, dst->i ^ s,
					0x0000FFFF0000FFFF, 0x0001000000010000, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_gt_sw( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000800080008000 ;
	dst->i = ~unsigned_packed_compare_ge
				( src->i ^ s, dst->i ^ s,
					0x0000FFFF0000FFFF, 0x0001000000010000, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_ge_sw( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000800080008000 ;
	dst->i = unsigned_packed_compare_ge
				( dst->i ^ s, src->i ^ s,
					0x0000FFFF0000FFFF, 0x0001000000010000, 16 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_ne_sd( Register * dst, const Register * src )
{
	dst->i = unsigned_packed_compare_ne
				( dst->i, src->i, 0x00000000FFFFFFFF, 0x0000000100000000, 32 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_eq_sd( Register * dst, const Register * src )
{
	dst->i = ~unsigned_packed_compare_ne
				( dst->i, src->i, 0x00000000FFFFFFFF, 0x0000000100000000, 32 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_lt_sd( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000000080000000 ;
	dst->i = ~unsigned_packed_compare_ge
				( dst->i ^ s, src->i ^ s, 0x00000000FFFFFFFF, 0x0000000100000000, 32 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_le_sd( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000000080000000 ;
	dst->i = unsigned_packed_compare_ge
				( src->i ^ s, dst->i ^ s, 0x00000000FFFFFFFF, 0x0000000100000000, 32 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_gt_sd( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000000080000000 ;
	dst->i = ~unsigned_packed_compare_ge
				( src->i ^ s, dst->i ^ s, 0x00000000FFFFFFFF, 0x0000000100000000, 32 ) ;
}

void __fastcall ECSSakura2Processor::simd_pcmp_ge_sd( Register * dst, const Register * src )
{
	const UINT64	s = 0x8000000080000000 ;
	dst->i = unsigned_packed_compare_ge
				( dst->i ^ s, src->i ^ s, 0x00000000FFFFFFFF, 0x0000000100000000, 32 ) ;
}

// 乗算命令 0xE2 0x80～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_pmullw( Register * dst, const Register * src )
{
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	pwd[0] *= pws[0] ;
	pwd[1] *= pws[1] ;
	pwd[2] *= pws[2] ;
	pwd[3] *= pws[3] ;
}

void __fastcall ECSSakura2Processor::simd_pmulhsw( Register * dst, const Register * src )
{
	SWORD *			pwd = (SWORD*) dst ;
	const SWORD *	pws = (const SWORD*) src ;
	pwd[0] = (SWORD) (((int) pwd[0] * pws[0]) >> 16) ;
	pwd[1] = (SWORD) (((int) pwd[1] * pws[1]) >> 16) ;
	pwd[2] = (SWORD) (((int) pwd[2] * pws[2]) >> 16) ;
	pwd[3] = (SWORD) (((int) pwd[3] * pws[3]) >> 16) ;
}

void __fastcall ECSSakura2Processor::simd_pmulhusw( Register * dst, const Register * src )
{
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	pwd[0] = (WORD) (((DWORD) pwd[0] * pws[0]) >> 16) ;
	pwd[1] = (WORD) (((DWORD) pwd[1] * pws[1]) >> 16) ;
	pwd[2] = (WORD) (((DWORD) pwd[2] * pws[2]) >> 16) ;
	pwd[3] = (WORD) (((DWORD) pwd[3] * pws[3]) >> 16) ;
}

void __fastcall ECSSakura2Processor::simd_pmaddwd( Register * dst, const Register * src )
{
	SWORD *			pwd = (SWORD*) dst ;
	const SWORD *	pws = (const SWORD*) src ;
	dst->l32 = ((int) pwd[0] * pws[0]) + ((int) pwd[1] * pws[1]) ;
	dst->h32 = ((int) pwd[2] * pws[2]) + ((int) pwd[3] * pws[3]) ;
}

// インターリーブ命令 0xE2 0x90～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_punpack_lbw( Register * dst, const Register * src )
{
	BYTE *			pbd = (BYTE*) dst ;
	const BYTE *	pbs = (const BYTE*) src ;
	pbd[7] = pbs[3] ;
	pbd[6] = pbd[3] ;
	pbd[5] = pbs[2] ;
	pbd[4] = pbd[2] ;
	pbd[3] = pbs[1] ;
	pbd[2] = pbd[1] ;
	pbd[1] = pbs[0] ;
//	pbd[0] = pbd[0] ;
}

void __fastcall ECSSakura2Processor::simd_punpack_lwd( Register * dst, const Register * src )
{
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	pwd[3] = pws[1] ;
	pwd[2] = pwd[1] ;
	pwd[1] = pws[0] ;
//	pwd[0] = pwd[0] ;
}

void __fastcall ECSSakura2Processor::simd_punpack_ldq( Register * dst, const Register * src )
{
	dst->h32 = src->l32 ;
}


// 飽和変換命令 0xE2 0xA0～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_pcvt_swb( Register * dst, const Register * src )
{
	const UINT64	s    = 0x8000800080008000 ;
	const UINT64	mask = 0x0000FFFF0000FFFF ;
	const UINT64	cr   = 0x0001000000010000 ;
	Register	ds, ss ;
	ds.i = unsigned_packed_sub
		( dst->i ^ s, 0x7F807F807F807F80, mask, cr, 16 ) ;
	ss.i = unsigned_packed_sub
		( src->i ^ s, 0x7F807F807F807F80, mask, cr, 16 ) ;
	ds.i |= unsigned_packed_compare_ge
				( ds.i, 0x00FF00FF00FF00FF, mask, cr, 16 ) ;
	ss.i |= unsigned_packed_compare_ge
				( ss.i, 0x00FF00FF00FF00FF, mask, cr, 16 ) ;
	//
	BYTE *	pbd = (BYTE*) dst ;
	pbd[0] = (BYTE) (ds.l32 & 0xFF) - 0x80 ;
	pbd[1] = (BYTE) ((ds.l32 >> 16) & 0xFF) - 0x80 ;
	pbd[2] = (BYTE) (ds.h32 & 0xFF) - 0x80 ;
	pbd[3] = (BYTE) ((ds.h32 >> 16) & 0xFF) - 0x80 ;
	pbd[4] = (BYTE) (ss.l32 & 0xFF) - 0x80 ;
	pbd[5] = (BYTE) ((ss.l32 >> 16) & 0xFF) - 0x80 ;
	pbd[6] = (BYTE) (ss.h32 & 0xFF) - 0x80 ;
	pbd[7] = (BYTE) ((ss.h32 >> 16) & 0xFF) - 0x80 ;
}

void __fastcall ECSSakura2Processor::simd_pcvt_uswb( Register * dst, const Register * src )
{
	const UINT64	mask = 0x0000FFFF0000FFFF ;
	const UINT64	cr   = 0x0001000000010000 ;
	const UINT64	sm   = 0x8000800080008000 ;
	Register	ds, ss ;
	UINT64	zmdst = dst->i & sm ;
	UINT64	zmsrc = src->i & sm ;
	zmdst -= (zmdst >> 15) ;
	zmsrc -= (zmsrc >> 15) ;
	ds.i = dst->i & ~(zmdst | sm) ;
	ss.i = src->i & ~(zmsrc | sm) ;
	ds.i |= unsigned_packed_compare_ge
				( ds.i, 0x00FF00FF00FF00FF, mask, cr, 16 ) ;
	ss.i |= unsigned_packed_compare_ge
				( ss.i, 0x00FF00FF00FF00FF, mask, cr, 16 ) ;
	//
	BYTE *	pbd = (BYTE*) dst ;
	pbd[0] = (BYTE) (ds.l32 & 0xFF) ;
	pbd[1] = (BYTE) ((ds.l32 >> 16) & 0xFF) ;
	pbd[2] = (BYTE) (ds.h32 & 0xFF) ;
	pbd[3] = (BYTE) ((ds.h32 >> 16) & 0xFF) ;
	pbd[4] = (BYTE) (ss.l32 & 0xFF) ;
	pbd[5] = (BYTE) ((ss.l32 >> 16) & 0xFF) ;
	pbd[6] = (BYTE) (ss.h32 & 0xFF) ;
	pbd[7] = (BYTE) ((ss.h32 >> 16) & 0xFF) ;
}

void __fastcall ECSSakura2Processor::simd_pcvt_sdw( Register * dst, const Register * src )
{
	const UINT64	s    = 0x8000000080000000 ;
	const UINT64	mask = 0x00000000FFFFFFFF ;
	const UINT64	cr   = 0x0000000100000000 ;
	Register	ds, ss ;
	ds.i = unsigned_packed_sub
		( dst->i ^ s, 0x7FFF80007FFF8000, mask, cr, 32 ) ;
	ss.i = unsigned_packed_sub
		( src->i ^ s, 0x7FFF80007FFF8000, mask, cr, 32 ) ;
	ds.i |= unsigned_packed_compare_ge
				( ds.i, 0x0000FFFF0000FFFF, mask, cr, 32 ) ;
	ss.i |= unsigned_packed_compare_ge
				( ss.i, 0x0000FFFF0000FFFF, mask, cr, 32 ) ;
	//
	WORD *	pwd = (WORD*) dst ;
	pwd[0] = (WORD) (ds.l32 - 0x8000) ;
	pwd[1] = (WORD) (ds.h32 - 0x8000) ;
	pwd[2] = (WORD) (ss.l32 - 0x8000) ;
	pwd[3] = (WORD) (ss.h32 - 0x8000) ;
}

// 即値シフト命令 0xE3 0x00～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_psrlw_imm8
		( Register * dst, const Register * src, int imm8 )
{
	const int		s = imm8 & 0x0F ;
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	pwd[0] = pws[0] >> s ;
	pwd[1] = pws[1] >> s ;
	pwd[2] = pws[2] >> s ;
	pwd[3] = pws[3] >> s ;
}

void __fastcall ECSSakura2Processor::simd_psraw_imm8
		( Register * dst, const Register * src, int imm8 )
{
	const int		s = imm8 & 0x0F ;
	SWORD *			pwd = (SWORD*) dst ;
	const SWORD *	pws = (const SWORD*) src ;
	pwd[0] = pws[0] >> s ;
	pwd[1] = pws[1] >> s ;
	pwd[2] = pws[2] >> s ;
	pwd[3] = pws[3] >> s ;
}

void __fastcall ECSSakura2Processor::simd_psllw_imm8
		( Register * dst, const Register * src, int imm8 )
{
	const int		s = imm8 & 0x0F ;
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	pwd[0] = pws[0] << s ;
	pwd[1] = pws[1] << s ;
	pwd[2] = pws[2] << s ;
	pwd[3] = pws[3] << s ;
}

void __fastcall ECSSakura2Processor::simd_psrld_imm8
		( Register * dst, const Register * src, int imm8 )
{
	const int	s = imm8 & 0x1F ;
	dst->l32 = src->l32 >> s ;
	dst->h32 = src->h32 >> s ;
}

void __fastcall ECSSakura2Processor::simd_psrad_imm8
		( Register * dst, const Register * src, int imm8 )
{
	const int	s = imm8 & 0x1F ;
	dst->li32 = src->li32 >> s ;
	dst->hi32 = src->hi32 >> s ;
}

void __fastcall ECSSakura2Processor::simd_pslld_imm8
		( Register * dst, const Register * src, int imm8 )
{
	const int	s = imm8 & 0x1F ;
	dst->l32 = src->l32 << s ;
	dst->h32 = src->h32 << s ;
}

// シャッフル命令 0xE3 0x20～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_pshufw_imm8( Register * dst, const Register * src, int imm8 )
{
	WORD *			pwd = (WORD*) dst ;
	const WORD *	pws = (const WORD*) src ;
	WORD	wbuf[4] ;
	wbuf[0] = pws[imm8 & 0x03] ;
	wbuf[1] = pws[(imm8 >> 2) & 0x03] ;
	wbuf[2] = pws[(imm8 >> 4) & 0x03] ;
	wbuf[3] = pws[(imm8 >> 6) & 0x03] ;
	pwd[0] = wbuf[0] ;
	pwd[1] = wbuf[1] ;
	pwd[2] = wbuf[2] ;
	pwd[3] = wbuf[3] ;
}


//////////////////////////////////////////////////////////////////////////////
// 128bit SIMD 処理関数
//////////////////////////////////////////////////////////////////////////////

// 命令テーブル
const ECSSakura2Processor::OPERATION_SIMD128_DST_SRC_PROC
				ECSSakura2Processor::pfnSIMD128_OperationDstSrc[0x100] =
{
	// 0x00
	simd_fadd_32, simd_fsub_32, simd_fmul_32, simd_fdiv_32,
	simd_fsqrt_32, simd_frcp_32, simd_frsqrt_32, simd_fabs_32,
	simd_fmax_32, simd_fmin_32, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x10
	simd_vadd_32, simd_vsub_32, simd_vmul_32, simd_vdiv_32,
	simd_vsqrt_32, simd_vrcp_32, simd_vrsqrt_32, simd_vabs_32,
	simd_vmax_32, simd_vmin_32, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x20
	simd_vcmp_ne_32, simd_vcmp_eq_32, simd_vcmp_lt_32, simd_vcmp_le_32,
	simd_vcmp_gt_32, simd_vcmp_ge_32, NULL, NULL,
	simd_vmove, simd_vand, simd_vor, simd_vxor,
	NULL, NULL, NULL, NULL,
	// 0x30
	simd_dcvt_f2i, simd_dcvt_i2f, simd_dcvt_d2f, simd_dcvt_f2d,
	simd_vcvt_f2w, simd_vcvt_w2f, simd_vcvt_f2i, simd_vcvt_i2f,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

const ECSSakura2Processor::OPERATION_SIMD128_DST_SRC_IMM_PROC
				ECSSakura2Processor::pfnSIMD128_OperationDstSrcImm8[0x100] =
{
	// 0x00
	simd_vmaskmove, simd_vshuf32, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x10
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;


// シングル演算命令 0xE4 0x00～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_fadd_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] += src->f32[0] ;
}

void __fastcall ECSSakura2Processor::simd_fsub_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] -= src->f32[0] ;
}

void __fastcall ECSSakura2Processor::simd_fmul_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] *= src->f32[0] ;
}

void __fastcall ECSSakura2Processor::simd_fdiv_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] /= src->f32[0] ;
}

void __fastcall ECSSakura2Processor::simd_fsqrt_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) sqrt( src->f32[0] ) ;
}

void __fastcall ECSSakura2Processor::simd_frcp_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = 1.0f / src->f32[0] ;
}

void __fastcall ECSSakura2Processor::simd_frsqrt_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) (1.0f / sqrt( src->f32[0] )) ;
}

void __fastcall ECSSakura2Processor::simd_fabs_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) fabs( src->f32[0] ) ;
}

void __fastcall ECSSakura2Processor::simd_fmax_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (dst->f32[0] > src->f32[0]) ? dst->f32[0] : src->f32[0] ;
}

void __fastcall ECSSakura2Processor::simd_fmin_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (dst->f32[0] < src->f32[0]) ? dst->f32[0] : src->f32[0] ;
}


// ベクタ演算命令 0xE4 0x10～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_vadd_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] += src->f32[0] ;
	dst->f32[1] += src->f32[1] ;
	dst->f32[2] += src->f32[2] ;
	dst->f32[3] += src->f32[3] ;
}

void __fastcall ECSSakura2Processor::simd_vsub_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] -= src->f32[0] ;
	dst->f32[1] -= src->f32[1] ;
	dst->f32[2] -= src->f32[2] ;
	dst->f32[3] -= src->f32[3] ;
}

void __fastcall ECSSakura2Processor::simd_vmul_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] *= src->f32[0] ;
	dst->f32[1] *= src->f32[1] ;
	dst->f32[2] *= src->f32[2] ;
	dst->f32[3] *= src->f32[3] ;
}

void __fastcall ECSSakura2Processor::simd_vdiv_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] /= src->f32[0] ;
	dst->f32[1] /= src->f32[1] ;
	dst->f32[2] /= src->f32[2] ;
	dst->f32[3] /= src->f32[3] ;
}

void __fastcall ECSSakura2Processor::simd_vsqrt_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) sqrt( src->f32[0] ) ;
	dst->f32[1] = (REAL32) sqrt( src->f32[1] ) ;
	dst->f32[2] = (REAL32) sqrt( src->f32[2] ) ;
	dst->f32[3] = (REAL32) sqrt( src->f32[3] ) ;
}

void __fastcall ECSSakura2Processor::simd_vrcp_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = 1.0f / src->f32[0] ;
	dst->f32[1] = 1.0f / src->f32[1] ;
	dst->f32[2] = 1.0f / src->f32[2] ;
	dst->f32[3] = 1.0f / src->f32[3] ;
}

void __fastcall ECSSakura2Processor::simd_vrsqrt_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) (1.0f / sqrt( src->f32[0] )) ;
	dst->f32[1] = (REAL32) (1.0f / sqrt( src->f32[1] )) ;
	dst->f32[2] = (REAL32) (1.0f / sqrt( src->f32[2] )) ;
	dst->f32[3] = (REAL32) (1.0f / sqrt( src->f32[3] )) ;
}

void __fastcall ECSSakura2Processor::simd_vabs_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) fabs( src->f32[0] ) ;
	dst->f32[1] = (REAL32) fabs( src->f32[1] ) ;
	dst->f32[2] = (REAL32) fabs( src->f32[2] ) ;
	dst->f32[3] = (REAL32) fabs( src->f32[3] ) ;
}

void __fastcall ECSSakura2Processor::simd_vmax_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (dst->f32[0] > src->f32[0]) ? dst->f32[0] : src->f32[0] ;
	dst->f32[1] = (dst->f32[1] > src->f32[1]) ? dst->f32[1] : src->f32[1] ;
	dst->f32[2] = (dst->f32[2] > src->f32[2]) ? dst->f32[2] : src->f32[2] ;
	dst->f32[3] = (dst->f32[3] > src->f32[3]) ? dst->f32[3] : src->f32[3] ;
}

void __fastcall ECSSakura2Processor::simd_vmin_32( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (dst->f32[0] < src->f32[0]) ? dst->f32[0] : src->f32[0] ;
	dst->f32[1] = (dst->f32[1] < src->f32[1]) ? dst->f32[1] : src->f32[1] ;
	dst->f32[2] = (dst->f32[2] < src->f32[2]) ? dst->f32[2] : src->f32[2] ;
	dst->f32[3] = (dst->f32[3] < src->f32[3]) ? dst->f32[3] : src->f32[3] ;
}


// ベクタ比較命令 0xE4 0x20～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_vcmp_ne_32( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = (dst->f32[0] != src->f32[0]) ? -1 : 0 ;
	dst->i32[1] = (dst->f32[1] != src->f32[1]) ? -1 : 0 ;
	dst->i32[2] = (dst->f32[2] != src->f32[2]) ? -1 : 0 ;
	dst->i32[3] = (dst->f32[3] != src->f32[3]) ? -1 : 0 ;
}

void __fastcall ECSSakura2Processor::simd_vcmp_eq_32( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = (dst->f32[0] == src->f32[0]) ? -1 : 0 ;
	dst->i32[1] = (dst->f32[1] == src->f32[1]) ? -1 : 0 ;
	dst->i32[2] = (dst->f32[2] == src->f32[2]) ? -1 : 0 ;
	dst->i32[3] = (dst->f32[3] == src->f32[3]) ? -1 : 0 ;
}

void __fastcall ECSSakura2Processor::simd_vcmp_lt_32( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = (dst->f32[0] < src->f32[0]) ? -1 : 0 ;
	dst->i32[1] = (dst->f32[1] < src->f32[1]) ? -1 : 0 ;
	dst->i32[2] = (dst->f32[2] < src->f32[2]) ? -1 : 0 ;
	dst->i32[3] = (dst->f32[3] < src->f32[3]) ? -1 : 0 ;
}

void __fastcall ECSSakura2Processor::simd_vcmp_le_32( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = (dst->f32[0] <= src->f32[0]) ? -1 : 0 ;
	dst->i32[1] = (dst->f32[1] <= src->f32[1]) ? -1 : 0 ;
	dst->i32[2] = (dst->f32[2] <= src->f32[2]) ? -1 : 0 ;
	dst->i32[3] = (dst->f32[3] <= src->f32[3]) ? -1 : 0 ;
}

void __fastcall ECSSakura2Processor::simd_vcmp_gt_32( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = (dst->f32[0] > src->f32[0]) ? -1 : 0 ;
	dst->i32[1] = (dst->f32[1] > src->f32[1]) ? -1 : 0 ;
	dst->i32[2] = (dst->f32[2] > src->f32[2]) ? -1 : 0 ;
	dst->i32[3] = (dst->f32[3] > src->f32[3]) ? -1 : 0 ;
}

void __fastcall ECSSakura2Processor::simd_vcmp_ge_32( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = (dst->f32[0] >= src->f32[0]) ? -1 : 0 ;
	dst->i32[1] = (dst->f32[1] >= src->f32[1]) ? -1 : 0 ;
	dst->i32[2] = (dst->f32[2] >= src->f32[2]) ? -1 : 0 ;
	dst->i32[3] = (dst->f32[3] >= src->f32[3]) ? -1 : 0 ;
}


// 移動・ビット演算命令 0xE4 0x28～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_vmove( Register128 * dst, const Register128 * src )
{
	dst->i[0] = src->i[0] ;
	dst->i[1] = src->i[1] ;
}

void __fastcall ECSSakura2Processor::simd_vand( Register128 * dst, const Register128 * src )
{
	dst->i[0] &= src->i[0] ;
	dst->i[1] &= src->i[1] ;
}

void __fastcall ECSSakura2Processor::simd_vor( Register128 * dst, const Register128 * src )
{
	dst->i[0] |= src->i[0] ;
	dst->i[1] |= src->i[1] ;
}

void __fastcall ECSSakura2Processor::simd_vxor( Register128 * dst, const Register128 * src )
{
	dst->i[0] ^= src->i[0] ;
	dst->i[1] ^= src->i[1] ;
}


// ベクタ変換命令 0xE4 0x30～
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_dcvt_f2i( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = eslRoundR32ToInt( src->f32[0] ) ;
	dst->i32[1] = eslRoundR32ToInt( src->f32[1] ) ;
	dst->i32[2] = 0 ;
	dst->i32[3] = 0 ;
}

void __fastcall ECSSakura2Processor::simd_dcvt_i2f( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) src->i32[0] ;
	dst->f32[1] = (REAL32) src->i32[1] ;
	dst->f32[2] = 0 ;
	dst->f32[3] = 0 ;
}

void __fastcall ECSSakura2Processor::simd_dcvt_d2f( Register128 * dst, const Register128 * src )
{
	REAL32	t[2] ;
	t[0] = (REAL32) src->f[0] ;
	t[1] = (REAL32) src->f[1] ;
	dst->f32[0] = t[0] ;
	dst->f32[1] = t[1] ;
	dst->f32[2] = 0 ;
	dst->f32[3] = 0 ;
}

void __fastcall ECSSakura2Processor::simd_dcvt_f2d( Register128 * dst, const Register128 * src )
{
	REAL64	t[2] ;
	t[0] = src->f32[0] ;
	t[1] = src->f32[1] ;
	dst->f[0] = t[0] ;
	dst->f[1] = t[1] ;
}

void __fastcall ECSSakura2Processor::simd_vcvt_f2w( Register128 * dst, const Register128 * src )
{
	Register	t[2] ;
	t[0].li32 = eslRoundR32ToInt( src->f32[0] ) ;
	t[0].hi32 = eslRoundR32ToInt( src->f32[1] ) ;
	t[1].li32 = eslRoundR32ToInt( src->f32[2] ) ;
	t[1].hi32 = eslRoundR32ToInt( src->f32[3] ) ;
	//
	simd_pcvt_sdw( &t[0], &t[1] ) ;
	//
	dst->i[0] = t[0].i ;
	dst->i[1] = 0 ;
}

void __fastcall ECSSakura2Processor::simd_vcvt_w2f( Register128 * dst, const Register128 * src )
{
	const SWORD *	pws = (const SWORD*) src ;
	SWORD	s[4] ;
	s[0] = pws[0] ;
	s[1] = pws[1] ;
	s[2] = pws[2] ;
	s[3] = pws[3] ;
	//
	dst->f32[0] = s[0] ;
	dst->f32[1] = s[1] ;
	dst->f32[2] = s[2] ;
	dst->f32[3] = s[3] ;
}

void __fastcall ECSSakura2Processor::simd_vcvt_f2i( Register128 * dst, const Register128 * src )
{
	dst->i32[0] = eslRoundR32ToInt( src->f32[0] ) ;
	dst->i32[1] = eslRoundR32ToInt( src->f32[1] ) ;
	dst->i32[2] = eslRoundR32ToInt( src->f32[2] ) ;
	dst->i32[3] = eslRoundR32ToInt( src->f32[3] ) ;
}

void __fastcall ECSSakura2Processor::simd_vcvt_i2f( Register128 * dst, const Register128 * src )
{
	dst->f32[0] = (REAL32) src->i32[0] ;
	dst->f32[1] = (REAL32) src->i32[1] ;
	dst->f32[2] = (REAL32) src->i32[2] ;
	dst->f32[3] = (REAL32) src->i32[3] ;
}


// ベクタ選択合成命令 0xE5 0x00
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_vmaskmove
	( Context * context, Register128 * dst, const Register128 * src, int imm8 )
{
	UINT64	mask[2] ;
	mask[0] = context->m_regset[imm8 & 0xFE].i ;
	mask[1] = context->m_regset[imm8 | 0x01].i ;
	//
	dst->i[0] = (dst->i[0] & ~mask[0]) | (src->i[0] & mask[0]) ;
	dst->i[1] = (dst->i[1] & ~mask[1]) | (src->i[1] & mask[1]) ;
}

// ベクタシャッフル命令 0xE5 0x01
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2Processor::simd_vshuf32
	( Context * context, Register128 * dst, const Register128 * src, int imm8 )
{
	REAL32	f[4] ;
	f[0] = src->f32[imm8 & 0x03] ;
	f[1] = src->f32[(imm8 >> 2) & 0x03] ;
	f[2] = src->f32[(imm8 >> 4) & 0x03] ;
	f[3] = src->f32[(imm8 >> 6) & 0x03] ;
	dst->f32[0] = f[0] ;
	dst->f32[1] = f[1] ;
	dst->f32[2] = f[2] ;
	dst->f32[3] = f[3] ;
}


//////////////////////////////////////////////////////////////////////////////
// 標準関数
//////////////////////////////////////////////////////////////////////////////

#define	SYSCALL_ENTRY_DESCRIPTION(x)	{ #x, syscall_##x }

#if	!defined(ENTISGLS4_DLL_IMPORT)
ECSSakura2Processor::SYSCALL_ENTRY	ECSSakura2Processor::entrySysCall[0x100] =
{
	SYSCALL_ENTRY_DESCRIPTION(memmove),
	SYSCALL_ENTRY_DESCRIPTION(memset),
	SYSCALL_ENTRY_DESCRIPTION(malloc),
	SYSCALL_ENTRY_DESCRIPTION(shared_malloc),
	SYSCALL_ENTRY_DESCRIPTION(realloc),
	SYSCALL_ENTRY_DESCRIPTION(free),
	SYSCALL_ENTRY_DESCRIPTION(sprintf_s),
	SYSCALL_ENTRY_DESCRIPTION(vsprintf_s),
	SYSCALL_ENTRY_DESCRIPTION(object_new),
	SYSCALL_ENTRY_DESCRIPTION(object_shared_new),
	SYSCALL_ENTRY_DESCRIPTION(object_delete),
	SYSCALL_ENTRY_DESCRIPTION(throw_exception),
	SYSCALL_ENTRY_DESCRIPTION(fabs),
	SYSCALL_ENTRY_DESCRIPTION(log),
	SYSCALL_ENTRY_DESCRIPTION(log10),
	SYSCALL_ENTRY_DESCRIPTION(pow),
	SYSCALL_ENTRY_DESCRIPTION(sqrt),
	SYSCALL_ENTRY_DESCRIPTION(sin),
	SYSCALL_ENTRY_DESCRIPTION(cos),
	SYSCALL_ENTRY_DESCRIPTION(tan),
	SYSCALL_ENTRY_DESCRIPTION(asin),
	SYSCALL_ENTRY_DESCRIPTION(acos),
	SYSCALL_ENTRY_DESCRIPTION(atan),
	SYSCALL_ENTRY_DESCRIPTION(atan2),
	SYSCALL_ENTRY_DESCRIPTION(round),
	SYSCALL_ENTRY_DESCRIPTION(floor),
	{ "SSystem::AtomicXchg" , syscall_SSystem_AtomicXchg },
	{ "SSystem::AtomicAdd" , syscall_SSystem_AtomicAdd },
	{ "SSystem::AtomicSub" , syscall_SSystem_AtomicSub },
	{ "SSystem::AtomicOr" , syscall_SSystem_AtomicOr },
	{ "SSystem::AtomicAnd" , syscall_SSystem_AtomicAnd },
	{ "SSystem::AtomicXor" , syscall_SSystem_AtomicXor },
	{ "SSystem::LockSystem" , syscall_SSystem_LockSystem },
	{ "SSystem::UnlockSystem" , syscall_SSystem_UnlockSystem },
	{ "SSystem::UnlockAllSystem" , syscall_SSystem_UnlockAllSystem },
	{ "SSystem::QuickLock" , syscall_SSystem_QuickLock },
	{ "SSystem::QuickUnlock" , syscall_SSystem_QuickUnlock },
	{ "SSystem::SleepFrame" , syscall_SSystem_SleepFrame },
	{ "SSystem::GetMemoryStatus" , syscall_SSystem_GetMemoryStatus },
	{ "SSystem::SetMemoryAllocationMode" , syscall_SSystem_SetMemoryAllocationMode },
	{ "SSystem::CurrentMilliSec" , syscall_SSystem_CurrentMilliSec },
	{ "SSystem::GetPerformanceCounter" , syscall_SSystem_GetPerformanceCounter },
	{ "SSystem::GetPerformanceFrequency" , syscall_SSystem_GetPerformanceFrequency },
	{ "SSystem::SleepMilliSec" , syscall_SSystem_SleepMilliSec },
	{ "SSystem::CurrentLocalDate" , syscall_SSystem_CurrentLocalDate },
	{ "SSystem::DifferenceInLocalTime" , syscall_SSystem_DifferenceInLocalTime },
	{ "SSystem::GetPlatformInformation" , syscall_SSystem_GetPlatformInformation },
	{ "SSystem::GetCPUFamily" , syscall_SSystem_GetCPUFamily },
	{ "SSystem::GetCPUFeatures" , syscall_SSystem_GetCPUFeatures },
	{ "SSystem::GetLogicalProcessorCount" , syscall_SSystem_GetLogicalProcessorCount },
	{ "SSystem::GetModuleExportFunction" , syscall_SSystem_GetModuleExportFunction },
	{ "SSystem::Assert" , syscall_SSystem_Assert },
	{ "SSystem::Trace" , syscall_SSystem_Trace },
	{ "SSystem::MessageBox" , syscall_SSystem_MessageBox },
	{ NULL, NULL },
} ;
#endif

// システム関数検索
//////////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::PROC_SYSCALL
	ECSSakura2Processor::GetSystemCallProc( const wchar_t * pwszFuncName )
{
	int	iFirst = 0 ;
	int	iEnd = g_countSystemCallFunc - 1 ;
	while ( iFirst <= iEnd )
	{
		const int		iMiddle = ((iFirst + iEnd) >> 1) ;
		const char *	pszEntryName = entrySysCall[iMiddle].pszFuncName ;
		//
		int	nCompare = 0 ;
		for ( int i = 0; true; i ++ )
		{
			nCompare = (int) pszEntryName[i] - (int) pwszFuncName[i] ;
			if ( (nCompare != 0) || (pszEntryName[i] == 0) )
			{
				break ;
			}
		}
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
			return	entrySysCall[iMiddle].pfnSysCall ;
		}
	}
	return	NULL ;
}

// void * memmove( void * dst, const void * src, int bytes )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSakura2Processor::syscall_memmove
		( Context * context, const Register * pArg )
{
	unsigned int	nBytes = pArg[2].l32 ;
	void *			ptrDst =
			context->AtomicTranslateAddress( pArg[0].i, nBytes ) ;
	const void *	ptrSrc =
			context->AtomicTranslateAddress( pArg[1].i, nBytes ) ;
	//
	context->m_regset[regAcc].i = 0 ;
	if ( (ptrDst != NULL) && (ptrSrc != NULL) )
	{
		eslMoveMemory( ptrDst, ptrSrc, nBytes ) ;
		context->m_regset[regAcc] = pArg[0] ;
	}
	return	NULL ;
}

// void * memset( void * dst, int c, int bytes )
const wchar_t * ECSSakura2Processor::syscall_memset
		( Context * context, const Register * pArg )
{
	int				nFillCode = pArg[1].l32 ;
	unsigned int	nBytes = pArg[2].l32 ;
	void *			ptrDst =
			context->AtomicTranslateAddress( pArg[0].i, nBytes ) ;
	//
	context->m_regset[regAcc].i = 0 ;
	if ( ptrDst != NULL )
	{
		eslFillMemory( ptrDst, nFillCode, nBytes ) ;
		context->m_regset[regAcc] = pArg[0] ;
	}
	return	NULL ;
}

// void * malloc( int bytes )
const wchar_t * ECSSakura2Processor::syscall_malloc
		( Context * context, const Register * pArg )
{
	context->m_regset[0].i =
		context->m_pSakura2VM->AllocateHeapMemory( pArg[0].l32 ) ;
	return	NULL ;
}

// void * shared_malloc( int bytes )
const wchar_t * ECSSakura2Processor::syscall_shared_malloc
		( Context * context, const Register * pArg )
{
	context->m_regset[0].i =
		context->m_pSakura2VM->AllocateHeapMemory
					( pArg[0].l32, SSystem::mallocModeShared ) ;
	return	NULL ;
}

// void * realloc( void * buf, int bytes )
const wchar_t * ECSSakura2Processor::syscall_realloc
		( Context * context, const Register * pArg )
{
	context->m_regset[0].i =
		context->m_pSakura2VM->ReallocateHeapMemory
					( pArg[0].i, pArg[1].l32 ) ;
	return	NULL ;
}

// void free( void * buf )
const wchar_t * ECSSakura2Processor::syscall_free
		( Context * context, const Register * pArg )
{
	if ( pArg[0].i != 0 )
	{
		context->m_pSakura2VM->FreeHeapMemory( pArg[0].i, context ) ;
	}
	return	NULL ;
}

// int sprintf_s( char * buf, size_t size, const char * format, ... )
const wchar_t * ECSSakura2Processor::syscall_sprintf_s
		( Context * context, const Register * pArg )
{
	uint16_t *	pwszFormat =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[2].i, sizeof(uint16_t) ) ;
	if ( pwszFormat == NULL )
	{
		context->m_regset[0].i = -1 ;
		return	NULL ;
	}
	SSystem::SString	strFormat = pwszFormat ;
	SSystem::SString	strBuffer ;
	context->m_pSakura2VM->
			FormatStringVlist( strBuffer, strFormat, pArg + 3 ) ;
	//
	uint16_t *	pwszBuffer =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[0].i, sizeof(uint16_t) ) ;
	if ( pwszBuffer == NULL )
	{
		// 出力先バッファが NULL の場合、ANSI では -1 を返すが
		// 詞葉では文字列長を返す
		context->m_regset[0].i = strBuffer.GetLength() ;
		return	NULL ;
	}
	size_t	nLength = strBuffer.GetLength() + 1 ;
	context->m_regset[0].i = nLength - 1 ;
	if ( nLength > (size_t) pArg[1].i )
	{
		nLength = (size_t) pArg[1].i ;
		context->m_regset[0].i = nLength ;
	}
	eslMoveMemory
		( pwszBuffer, strBuffer.GetConstArray(),
						nLength * sizeof(uint16_t) ) ;
	return	NULL ;
}

// int vsprintf_s( char * buf, size_t size, const char * format, va_list argptr )
const wchar_t * ECSSakura2Processor::syscall_vsprintf_s
		( Context * context, const Register * pArg )
{
	uint16_t *	pwszFormat =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[2].i, sizeof(uint16_t) ) ;
	Register *	pVarArgList =
			(Register*) context->AtomicTranslateAddress
								( pArg[3].i, sizeof(Register) ) ;
	if ( (pwszFormat == NULL) || (pVarArgList == NULL) )
	{
		context->m_regset[0].i = -1 ;
		return	NULL ;
	}
	SSystem::SString	strFormat = pwszFormat ;
	SSystem::SString	strBuffer ;
	context->m_pSakura2VM->
			FormatStringVlist( strBuffer, strFormat, pVarArgList ) ;
	//
	uint16_t *	pwszBuffer =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[0].i, sizeof(uint16_t) ) ;
	if ( pwszBuffer == NULL )
	{
		// 出力先バッファが NULL の場合、ANSI では -1 を返すが
		// 詞葉では文字列長を返す
		context->m_regset[0].i = strBuffer.GetLength() ;
		return	NULL ;
	}
	size_t	nLength = strBuffer.GetLength() + 1 ;
	context->m_regset[0].i = nLength - 1 ;
	if ( nLength > (size_t) pArg[1].i )
	{
		nLength = (size_t) pArg[1].i ;
		context->m_regset[0].i = nLength ;
	}
	eslMoveMemory
		( pwszBuffer, strBuffer.GetConstArray(),
						nLength * sizeof(uint16_t) ) ;
	return	NULL ;
}

// オブジェクト生成・消滅
//////////////////////////////////////////////////////////////////////////////

// void * object_new( int cls_id )
const wchar_t * ECSSakura2Processor::syscall_object_new
	( Context * context, const Register * pArg )
{
	ECSSakura2::Object *	pObj =
			context->m_pSakura2VM->
				NewObjectByIdentity( context, pArg[0].l32 ) ;
	if ( pObj != NULL )
	{
		ECSSakura2Processor::AssertLock() ;
		context->m_regset[0].i =
			context->m_pSakura2VM->AllocateHeapObjectAddress( pObj ) ;
		ECSSakura2Processor::AssertUnlock() ;
		return	NULL ;
	}
	else
	{
		return	L"object の生成に失敗しました" ;
	}
}

// void * object_shared_new( int cls_id )
const wchar_t * ECSSakura2Processor::syscall_object_shared_new
		( Context * context, const Register * pArg )
{
	ECSSakura2::Object *	pObj =
			context->m_pSakura2VM->
				NewObjectByIdentity( context, pArg[0].l32 ) ;
	if ( pObj != NULL )
	{
		ECSSakura2Processor::AssertLock() ;
		context->m_regset[0].i =
			context->m_pSakura2VM->
				AllocateHeapObjectAddress( pObj, SSystem::mallocModeShared ) ;
		ECSSakura2Processor::AssertUnlock() ;
		return	NULL ;
	}
	else
	{
		return	L"object の生成に失敗しました" ;
	}
}

// void object_delete( void * obj )
const wchar_t * ECSSakura2Processor::syscall_object_delete
	( Context * context, const Register * pArg )
{
	ECSSakura2::Object *	pObj =
		context->m_pSakura2VM->AtomicObjectFromAddress( pArg[0].h32 ) ;
	if ( pObj != NULL )
	{
//		ECSSakura2Processor::AssertLock() ;
		context->m_pSakura2VM->FreeHeapObjectAddress( pArg[0].i, context ) ;
//		ECSSakura2Processor::AssertUnlock() ;
	}
	return	NULL ;
}


// 例外
// void throw_exception( int code, void * obj, void * dstr, void * rtcctbl )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSakura2Processor::syscall_throw_exception
		( Context * context, const Register * pArg )
{
	context->m_regset[regException0] = pArg[0] ;
	context->m_regset[regException1] = pArg[1] ;
	context->m_regset[regException2] = pArg[2] ;
	context->m_regset[regException3] = pArg[3] ;
	//
	Register *	pXP =
		(Register*) context->AtomicTranslateAddress
					( context->m_regset[regXP].i, sizeof(Register) ) ;
	if ( pXP == NULL )
	{
		if ( (context->m_regset[regException0].l32 == 0x80000000)
			&& (context->m_regset[regException0].h32 = 0xFFFFFFFF) )
		{
			uint16_t *	pwszErrMsg =
				(uint16_t*) context->AtomicTranslateAddress
								( context->m_regset[regException1].i ) ;
			if ( pwszErrMsg != NULL )
			{
				SSystem::SString	strErrMsg = pwszErrMsg ;
				context->SetContextErrorMessage( strErrMsg ) ;
				return	context->m_pszError ;
			}
		}
		return	L"処理されない例外がスローされました" ;
	}
	context->m_ip = pXP->l32 ;
	if ( context->m_ipSegment != pXP->h32 )
	{
		context->m_ipSegment = pXP->h32 ;
		AtomicOr( &(context->m_maskException), exceptionFarJump ) ;
	}
	return	NULL ;
}


// アトミック処理
//////////////////////////////////////////////////////////////////////////////

// int64 SSystem::AtomicXchg( int64* p, long int v ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_AtomicXchg
		( Context * context, const Register * pArg )
{
	AssertLock() ;
	mutexGlobalAtomic->Lock() ;
	//
	INT64 *	p = (INT64*) context->AsyncTranslateAddress( pArg[0].i, 8 ) ;
	if ( p != NULL )
	{
		INT64	nTemp = *p ;
		*p = pArg[1].i ;
		context->m_regset[regAcc].i = nTemp ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	//
	mutexGlobalAtomic->Unlock() ;
	AssertUnlock() ;
	return	NULL ;
}

// int64 SSystem::AtomicAdd( int64* p, long int v ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_AtomicAdd
		( Context * context, const Register * pArg )
{
	AssertLock() ;
	mutexGlobalAtomic->Lock() ;
	//
	INT64 *	p = (INT64*) context->AsyncTranslateAddress( pArg[0].i, 8 ) ;
	if ( p != NULL )
	{
		context->m_regset[regAcc].i = (*p += pArg[1].i) ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	//
	mutexGlobalAtomic->Unlock() ;
	AssertUnlock() ;
	return	NULL ;
}

// int64 SSystem::AtomicSub( int64* p, long int v ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_AtomicSub
		( Context * context, const Register * pArg )
{
	AssertLock() ;
	mutexGlobalAtomic->Lock() ;
	//
	INT64 *	p = (INT64*) context->AsyncTranslateAddress( pArg[0].i, 8 ) ;
	if ( p != NULL )
	{
		context->m_regset[regAcc].i = (*p -= pArg[1].i) ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	//
	mutexGlobalAtomic->Unlock() ;
	AssertUnlock() ;
	return	NULL ;
}

// int64 SSystem::AtomicOr( int64* p, long int v ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_AtomicOr
		( Context * context, const Register * pArg )
{
	AssertLock() ;
	mutexGlobalAtomic->Lock() ;
	//
	INT64 *	p = (INT64*) context->AsyncTranslateAddress( pArg[0].i, 8 ) ;
	if ( p != NULL )
	{
		context->m_regset[regAcc].i = (*p |= pArg[1].i) ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	//
	mutexGlobalAtomic->Unlock() ;
	AssertUnlock() ;
	return	NULL ;
}

// int64 SSystem::AtomicAnd( int64* p, long int v ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_AtomicAnd
		( Context * context, const Register * pArg )
{
	AssertLock() ;
	mutexGlobalAtomic->Lock() ;
	//
	INT64 *	p = (INT64*) context->AsyncTranslateAddress( pArg[0].i, 8 ) ;
	if ( p != NULL )
	{
		context->m_regset[regAcc].i = (*p &= pArg[1].i) ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	//
	mutexGlobalAtomic->Unlock() ;
	AssertUnlock() ;
	return	NULL ;
}

// int64 SSystem::AtomicXor( int64* p, long int v ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_AtomicXor
		( Context * context, const Register * pArg )
{
	AssertLock() ;
	mutexGlobalAtomic->Lock() ;
	//
	INT64 *	p = (INT64*) context->AsyncTranslateAddress( pArg[0].i, 8 ) ;
	if ( p != NULL )
	{
		context->m_regset[regAcc].i = (*p ^= pArg[1].i) ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	//
	mutexGlobalAtomic->Unlock() ;
	AssertUnlock() ;
	return	NULL ;
}

// SError SSystem::LockSystem( int64_t msecTimeout = Synchronism::Infinite ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_LockSystem
		( Context * context, const Register * pArg )
{
	int64_t			msecTimeout = pArg[0].i ;
	SSystem::SError	err = SSystem::errAbort ;
	while ( context->m_status == Context::xsExecution )
	{
		if ( msecTimeout == SSystem::Synchronism::Infinite )
		{
			err = SSystem::LockTrace( __FILE__, __LINE__, 30 ) ;
			if ( err == SSystem::errSuccess )
			{
				break ;
			}
		}
		else
		{
			int64_t	msecNext = 10 ;
			if ( msecNext > msecTimeout )
			{
				msecNext = msecTimeout ;
			}
			err = SSystem::LockTrace( __FILE__, __LINE__, msecNext ) ;
			if ( err == SSystem::errSuccess )
			{
				break ;
			}
			msecTimeout -= msecNext ;
			if ( msecTimeout <= 0 )
			{
				err = SSystem::errTimeout ;
				break ;
			}
		}
	}
	context->m_regset[regAcc].i = err ;
	return	NULL ;
}

// SError SSystem::UnlockSystem( void ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_UnlockSystem
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::Unlock() ;
	return	NULL ;
}

// atomic_int_t SSystem::UnlockAllSystem( void ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_UnlockAllSystem
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::UnlockAll() ;
	return	NULL ;
}

// void SSystem::QuickLock( void ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_QuickLock
		( Context * context, const Register * pArg )
{
	mutexQuickLock->Lock() ;
	return	NULL ;
}

// void SSystem::QuickUnlock( void ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_QuickUnlock
		( Context * context, const Register * pArg )
{
	mutexQuickLock->Unlock() ;
	return	NULL ;
}


// 算術関数
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSakura2Processor::syscall_fabs
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = fabs( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_log
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = log( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_log10
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = log10( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_pow
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = pow( pArg[0].f, pArg[1].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_sqrt
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = sqrt( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_sin
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = sin( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_cos
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = cos( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_tan
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = tan( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_asin
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = asin( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_acos
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = acos( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_atan
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = atan( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_atan2
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].f = atan2( pArg[0].f, pArg[1].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_round
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = eslRoundR64ToLInt( pArg[0].f ) ;
	return	NULL ;
}

const wchar_t * ECSSakura2Processor::syscall_floor
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i
			= eslRoundR64ToLInt( floor( pArg[0].f ) ) ;
	return	NULL ;
}


// ペンディング処理
// void SSystem::SleepFrame( int64_t frames ) ;
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSakura2Processor::syscall_SSystem_SleepFrame
		( Context * context, const Register * pArg )
{
	context->SetPendingStatus( pArg[0].i ) ;
	return	NULL ;
}


// メモリ情報
// void SSystem::GetMemoryStatus( SSystem::MEMORY_STATUS mstatus ) ;
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetMemoryStatus
		( Context * context, const Register * pArg )
{
	SSystem::MEMORY_STATUS *	pmStatus =
			(SSystem::MEMORY_STATUS*)
				context->AtomicTranslateAddress
					( pArg[0].i, sizeof(SSystem::MEMORY_STATUS) ) ;
	if ( pmStatus == NULL )
	{
		return	L"invalid pointer (SSystem::GetMemoryStatus)" ;
	}
	SSystem::GetMemoryStatus( *pmStatus ) ;
	return	NULL ;
}

// メモリ・アロケーション・モード
// void SSystem::SetMemoryAllocationMode( SSystem::MemoryAllocationMode mode ) ;
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSakura2Processor::syscall_SSystem_SetMemoryAllocationMode
		( Context * context, const Register * pArg )
{
	SSystem::SetMemoryAllocationMode
		( (SSystem::MemoryAllocationMode) pArg[0].i ) ;
	return	NULL ;
}


// タイマ・時刻
//////////////////////////////////////////////////////////////////////////////

// uint64_t SSystem::CurrentMilliSec( void )
const wchar_t * ECSSakura2Processor::syscall_SSystem_CurrentMilliSec
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::CurrentMilliSec() ;
	return	NULL ;
}

// int64_t GetPerformanceCounter( void ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetPerformanceCounter
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::GetPerformanceCounter() ;
	return	NULL ;
}

// int64_t GetPerformanceFrequency( void ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetPerformanceFrequency
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::GetPerformanceFrequency() ;
	return	NULL ;
}

// void SSystem::SleepMilliSec( int msec )
const wchar_t * ECSSakura2Processor::syscall_SSystem_SleepMilliSec
		( Context * context, const Register * pArg )
{
	SSystem::SleepMilliSec( (int) pArg[0].i ) ;
	return	NULL ;
}

// void SSystem::CurrentLocalDate( SSystem::DATE_TIME& date )
const wchar_t * ECSSakura2Processor::syscall_SSystem_CurrentLocalDate
		( Context * context, const Register * pArg )
{
	SSystem::DATE_TIME *	pDate =
		(SSystem::DATE_TIME*)
			context->AtomicTranslateAddress
				( pArg[0].i, sizeof(SSystem::DATE_TIME) ) ;
	if ( pDate == NULL )
	{
		return	L"invalid pointer (SSystem::CurrentLocalDate)" ;
	}
	SSystem::CurrentLocalDate( *pDate ) ;
	return	NULL ;
}

// int32_t DifferenceInLocalTime
//		( wchar_t * pwszName = NULL, size_t nNameCapacity = 0 ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_DifferenceInLocalTime
		( Context * context, const Register * pArg )
{
	const size_t	nNameCapacity = (size_t) pArg[1].i ;
	uint16_t *		pwName =
		(uint16_t*)
			context->AtomicTranslateAddress
				( pArg[0].i, nNameCapacity * sizeof(uint16_t) ) ;
	//
	SSystem::SArray<wchar_t>	aZoneName ;
	wchar_t *	pwszName = aZoneName.GetArray(nNameCapacity) ;
	context->m_regset[regAcc].i =
		SSystem::DifferenceInLocalTime( pwszName, nNameCapacity ) ;
	//
	for ( size_t i = 0; i < nNameCapacity; i ++ )
	{
		pwName[i] = (uint16_t) pwszName[i] ;
	}
	aZoneName.FinishArray() ;
	return	NULL ;
}


// プラットフォーム情報
//////////////////////////////////////////////////////////////////////////////

// void SSystem::GetPlatformInformation( SSystem::PLATFORM_INFORMATION& pi )
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetPlatformInformation
		( Context * context, const Register * pArg )
{
	SSystem::PLATFORM_INFORMATION *	pPlatform =
		(SSystem::PLATFORM_INFORMATION*)
			context->AtomicTranslateAddress
				( pArg[0].i, sizeof(SSystem::PLATFORM_INFORMATION) ) ;
	if ( pPlatform == NULL )
	{
		return	L"invalid pointer (SSystem::GetPlatformInformation)" ;
	}
	SSystem::GetPlatformInformation( *pPlatform ) ;
	pPlatform->platformFamily = SSystem::platformFamilyCotopha ;
	//
	SSystem::SEnvironmentInterface *
				pEnv = context->m_pSakura2VM->GetEnvironment() ;
	if ( pEnv != NULL )
	{
		if ( pEnv->IsEnabledSakura2JITCompiler() )
		{
			pPlatform->featuresJIT = SSystem::jitFeature_Compiler ;
			pPlatform->featuresJIT |=
				ECSSakura2::ExecutableModule::GetJITCompilerFeatures() ;
		}
	}
	return	NULL ;
}


// 実行プロセッサ情報
//////////////////////////////////////////////////////////////////////////////

// SSystem::CPU_Family SSystem::GetCPUFamily( void )
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetCPUFamily
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::GetCPUFamily() ;
	return	NULL ;
}

// uint64_t SSystem::GetCPUFeatures( void )
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetCPUFeatures
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::GetCPUFeatures() ;
	return	NULL ;
}

// unsigned int SSystem::GetLogicalProcessorCount( void )
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetLogicalProcessorCount
		( Context * context, const Register * pArg )
{
	context->m_regset[regAcc].i = SSystem::GetLogicalProcessorCount() ;
	return	NULL ;
}


// システム（モジュール・エクスポート関数）
//////////////////////////////////////////////////////////////////////////////

// ulong_ptr_t GetModuleExportFunction
//	( const wchar_t * pszFuncName, const wchar_t * pszReserved = NULL ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_GetModuleExportFunction
			( Context * context, const Register * pArg )
{
	uint16_t *	pwszFuncName =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[0].i, sizeof(uint16_t) ) ;
	uint16_t *	pwszReserved = NULL ;
	if ( pArg[1].i != 0 )
	{
		pwszReserved =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[1].i, sizeof(uint16_t) ) ;
	}
	context->m_regset[regAcc].i = 0 ;
	if ( pwszFuncName == NULL )
	{
		return	NULL ;
	}
	SSystem::SString	strFuncName = pwszFuncName ;
	SSystem::SString	strReserved = pwszReserved ;
	context->m_regset[regAcc].i =
		context->m_pSakura2VM->
			GetFunctionAddress( strFuncName, strReserved ) ;
	return	NULL ;
}


// デバッグ用関数
//////////////////////////////////////////////////////////////////////////////

// void SSystem::Trace( const char * pszTrace, ... )
const wchar_t * ECSSakura2Processor::syscall_SSystem_Trace
		( Context * context, const Register * pArg )
{
	uint16_t *	pwszTrace =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[0].i, sizeof(uint16_t) ) ;
	if ( pwszTrace == NULL )
	{
		return	NULL ;
	}
	SSystem::SString		strFormat = pwszTrace ;
	SSystem::SString		strTrace ;
	SSystem::SArray<char>	bufTrace ;
	context->m_pSakura2VM->FormatStringVlist
						( strTrace, strFormat, pArg + 1 ) ;
	SSystem::Trace( "%s", strTrace.EncodeDefaultTo( bufTrace ) ) ;
	return	NULL ;
}

// void SSystem::Assert
//	( const char * pszExpr, const char * pszFile, int nLineNum )
const wchar_t * ECSSakura2Processor::syscall_SSystem_Assert
		( Context * context, const Register * pArg )
{
	return	NULL ;
}

// int SSystem::MessageBox
//	( const char * pszMsg,
//		const char * pszCaption = NULL,
//		int nStyles = msgboxStyleOk,
//		SakuraGl::Window * pParentWnd = NULL ) ;
const wchar_t * ECSSakura2Processor::syscall_SSystem_MessageBox
		( Context * context, const Register * pArg )
{
	uint16_t *	pszMsg =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[0].i, sizeof(uint16_t) ) ;
	uint16_t *	pszCaption =
			(uint16_t*) context->AtomicTranslateAddress
								( pArg[1].i, sizeof(uint16_t) ) ;
	SakuraGL::SGLAbstractWindow *
		pParentWnd = ESLTypeCast<SakuraGL::SGLAbstractWindow>
			( context->m_pSakura2VM->AtomicObjectFromAddress( pArg[3].h32 ) ) ;
	SSystem::SString	strMsg = pszMsg ;
	SSystem::SString	strCaption = pszCaption ;
	//
	context->m_regset[regAcc].i =
		SSystem::MessageBox
			( strMsg, strCaption, (int) pArg[2].i, pParentWnd ) ;
	//
	return	NULL ;
}

