
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <rosetta/rosetta.h>
#include <glscs/glscs_sakura2_module_maker.h>
#include <rosetta/rosetta_compiler.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_sound_recorder.h>
#include <sakuraglx/sakuraglx.h>
#include <glscs/glscs_sakura2_obj_environment.h>
#include <glscs/glscs_sakura2_obj_synchronism.h>
#include <glscs/glscs_sakura2_obj_file.h>
#include <glscs/glscs_sakura2_obj_socket.h>
#include <glscs/glscs_sakura2_obj_module.h>
#include <glscs/glscs_sakura2_obj_std_ui.h>
#include <glscs/glscs_sakura2_obj_image.h>
#include <glscs/glscs_sakura2_obj_paint.h>
#include <glscs/glscs_sakura2_obj_window.h>
#include <glscs/glscs_sakura2_obj_window_menu.h>
#include <glscs/glscs_sakura2_obj_render.h>
#include <glscs/glscs_sakura2_obj_font.h>
#include <glscs/glscs_sakura2_obj_sound.h>
#include <glscs/glscs_sakura2_obj_audio_decoder.h>
#include <glscs/glscs_sakura2_obj_audio_player.h>
#include <glscs/glscs_sakura2_obj_media_player.h>
#include <glscs/glscs_sakura2_obj_joy_stick.h>
#include <glscs/glscs_sakura2_obj_vibrator.h>

#if	defined(__PLATFORM_ANDROID__)
#include <sakura/ssys_android_file.h>
#endif

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;

//#define	_CSVM_DEBUG_TRACE_SYSCALL	1
//#define	_CSVM_DEBUG_TRACE_NEW_OBJ	1


//////////////////////////////////////////////////////////////////////////////
// 標準クラス情報テーブル
//////////////////////////////////////////////////////////////////////////////

struct	NEW_OBJECT_ENTRY
{
	typedef	VirtualMachine::PROC_NEW_OBJECT	F ;
	const wchar_t *	pwszSymbol ;
	F				pfnEntry ;
} ;

struct	SYSTEN_CALL_ENTRY
{
	typedef	ECSSakura2Processor::PROC_SYSCALL	F ;
	const wchar_t *	pwszSymbol ;
	F				pfnEntry ;
} ;

#define	ENTRY_NEW_OBJECT( name )	{ L###name, ecs_new_object_##name }
#define	ENTRY_SYS_CALL( name )		{ L###name, ecs_nakedcall_##name }

template <class T>
	typename T::F binarySearchForVector
			( const T * pVector, size_t nCount, const wchar_t * pwszName )
{
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = (ssize_t) nCount - 1 ;
	while ( iFirst <= iEnd )
	{
		ssize_t	iMiddle = (iFirst + iEnd) >> 1 ;
		int	nCompare = SString::Compare
							( pVector[iMiddle].pwszSymbol, pwszName ) ;
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
			return	pVector[iMiddle].pfnEntry ;
		}
	}
	return	NULL ;
}

static NEW_OBJECT_ENTRY	g_NewObjectVector[] =
{
#include <glscs/glscs_sakura2_new_object.h>
} ;

static SYSTEN_CALL_ENTRY	g_SysCallVector[] =
{
#include <glscs/glscs_sakura2_sys_call.h>
} ;


//////////////////////////////////////////////////////////////////////////////
// 標準的な仮想マシンの実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::StandardVM, VirtualMachine )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
StandardVM::StandardVM( void )
	: m_heapGlobal( roasNakedHeap ),
		m_heapShared( roasNakedSharedHeap ), m_heapThread( roasNakedThread )
{
	m_modeDefHeap = SSystem::mallocModeGlobal ;
	m_pEnv = NULL ;
	m_pSysContext = NULL ;
	m_pMainThread = NULL ;
	m_pExceptionHandler = NULL ;
	InitializeDirectoryTable() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
StandardVM::~StandardVM( void )
{
	StandardVM::ReleaseVM() ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
void StandardVM::InitializeVM( void )
{
	InitializeDirectoryTable() ;
	//
	CreateMainThread() ;
	//
	ThreadObject *	pSysThread = LockSystemThread() ;
	UnlockSystemThread( pSysThread ) ;
}

// 仮想マシンの解放
//////////////////////////////////////////////////////////////////////////////
void StandardVM::ReleaseVM( void )
{
	StandardVM::Lock() ;
	for ( int i = roasNakedHeap; i < roasNakedThread; i ++ )
	{
		SPointerArray<Object> *	ptblObj = m_pAddressRootDirectory[i] ;
		if ( ptblObj != NULL )
		{
			for ( size_t j = 0; j < ptblObj->GetLength(); j ++ )
			{
				ThreadObject *	pThread =
					ESLTypeCast<ThreadObject>( ptblObj->GetAt(j) ) ;
				if ( pThread != NULL )
				{
					StandardVM::Unlock() ;
					pThread->AbortThread() ;
					StandardVM::Lock() ;
				}
			}
		}
	}
	m_pSysContext = NULL ;
	m_pMainThread = NULL ;
	m_lstTempContext.RemoveAll() ;
	StandardVM::Unlock() ;
	//
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	StandardVM::Lock() ;
	InitializeDirectoryTable() ;
	m_allocModules.RemoveAll() ;
	StandardVM::Unlock() ;
	SSystem::Unlock() ;
}

// 環境設定を関連付け
//////////////////////////////////////////////////////////////////////////////
void StandardVM::AttachEnvironment( SEnvironmentInterface * pEnv )
{
	m_pEnv = pEnv ;
}

// 環境設定を取得
//////////////////////////////////////////////////////////////////////////////
SEnvironmentInterface * StandardVM::GetEnvironment( void ) const
{
	if ( m_pEnv != NULL )
	{
		return	m_pEnv ;
	}
	return	VirtualMachine::GetEnvironment() ;
}

// モジュールを仮想マシンにロード
//////////////////////////////////////////////////////////////////////////////
const wchar_t * StandardVM::LoadModule
	( ThreadObject * pThread, ExecutableModule * pModule, int iModule )
{
	if ( iModule < 0 )
	{
		AllocateModule( pModule ) ;
	}
	else
	{
		AttachModuleAt( iModule, pModule ) ;
		AllocateModuleAt( iModule, pModule ) ;
	}
	SArray<DWORD>&	vectorPrologue = pModule->m_vectorPrologue ;
	const size_t	countPrologue = vectorPrologue.GetLength() ;
	const DWORD		dwHighIP = (roasCode << 24)
								| (pModule->m_iModule & 0x00FFFFFF) ;
	for ( size_t i = 0; i < countPrologue; i ++ )
	{
		INT64	addrFunc = (((INT64)dwHighIP) << 32) | vectorPrologue.At(i) ;
		const wchar_t *	pwszException =
				pThread->CallFunction( addrFunc, NULL, 0 ) ;
		if ( pwszException != NULL )
		{
			ESLTrace( "exception at prologue %d\n", (int) i ) ;
			return	pwszException ;
		}
	}
	return	NULL ;
}

const wchar_t *
	StandardVM::LoadModuleByPrologueOnSysThread
				( ExecutableModule * pModule, int iModule )
{
	ThreadObject *	pThread = LockSystemThread() ;
	if ( pThread == NULL )
	{
		ESLTrace( "failed to LockSystemThread.\n" ) ;
		if ( iModule < 0 )
		{
			AllocateModule( pModule ) ;
		}
		else
		{
			AttachModuleAt( iModule, pModule ) ;
			AllocateModuleAt( iModule, pModule ) ;
		}
		return	NULL ;
	}
	const wchar_t *	pwszErr = LoadModule( pThread, pModule, iModule ) ;
	UnlockSystemThread( pThread ) ;
	return	pwszErr ;
}

// モジュールを仮想マシンから解放
//////////////////////////////////////////////////////////////////////////////
const wchar_t * StandardVM::UnloadModule
	( ThreadObject * pThread, ExecutableModule * pModule )
{
	SArray<DWORD>&	vectorEpilogue = pModule->m_vectorEpilogue ;
	const size_t	countEpilogue = vectorEpilogue.GetLength() ;
	const DWORD		dwHighIP = (roasCode << 24)
								| (pModule->m_iModule & 0x00FFFFFF) ;
	for ( size_t i = 0; i < countEpilogue; i ++ )
	{
		INT64	addrFunc = (((INT64)dwHighIP) << 32) | vectorEpilogue.At(i) ;
		const wchar_t *	pwszException =
				pThread->CallFunction( addrFunc, NULL, 0 ) ;
		if ( pwszException != NULL )
		{
			ESLTrace( "exception at epilogue %d\n", (int) i ) ;
			return	pwszException ;
		}
	}
	FreeModuleAllocation( pModule ) ;
	return	NULL ;
}

const wchar_t *
	StandardVM::UnloadModuleByEpilogueOnSysThread( ExecutableModule * pModule )
{
	ThreadObject *	pThread = LockSystemThread() ;
	if ( pThread == NULL )
	{
		ESLTrace( "failed to LockSystemThread.\n" ) ;
		FreeModuleAllocation( pModule ) ;
		return	NULL ;
	}
	const wchar_t *	pwszErr = UnloadModule( pThread, pModule ) ;
	UnlockSystemThread( pThread ) ;
	return	pwszErr ;
}

// メインスレッドを生成
//////////////////////////////////////////////////////////////////////////////
ThreadObject * StandardVM::CreateMainThread( void )
{
	ThreadObject *	pThread ;
	Lock() ;
	if ( m_pMainThread == NULL )
	{
		m_pMainThread = ThreadObject::NewContext() ;
		m_pMainThread->InitializeContext( this ) ;
	}
	pThread = m_pMainThread ;
	Unlock() ;
	return	pThread ;
}

// メインスレッドを取得
//////////////////////////////////////////////////////////////////////////////
ThreadObject * StandardVM::GetMainThread( void ) const
{
	return	m_pMainThread ;
}

// システムスレッドを取得／生成
//////////////////////////////////////////////////////////////////////////////
ThreadObject * StandardVM::LockSystemThread( void )
{
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pSysContext == NULL )
	{
		m_pSysContext = ThreadObject::NewContext() ;
		m_pSysContext->InitializeContext( this ) ;
	}
	return	m_pSysContext ;
}

// システムスレッドを解放
//////////////////////////////////////////////////////////////////////////////
void StandardVM::UnlockSystemThread( ThreadObject * pSysThread )
{
	SSystem::Unlock() ;
}

// システムスレッド（非同期・一時）を取得／生成
//////////////////////////////////////////////////////////////////////////////
ThreadObject * StandardVM::CreateSystemAsyncThread( void )
{
	ThreadObject *	pThread = NULL ;
	SSystem::QuickLock() ;
	size_t	nCount = m_lstTempContext.GetLength() ;
	if ( nCount > 0 )
	{
		pThread = m_lstTempContext.GetAt( nCount - 1 ) ;
		m_lstTempContext.RemoveAt( nCount - 1 ) ;
		ESLAssert( pThread != NULL ) ;
	}
	SSystem::QuickUnlock() ;
	if ( pThread == NULL )
	{
		pThread = ThreadObject::NewContext() ;
		pThread->InitializeContext( this ) ;
	}
	return	pThread ;
}

// システムスレッド（非同期・一時）を解放
//////////////////////////////////////////////////////////////////////////////
void StandardVM::ReleaseSystemAsyncThread( ThreadObject * pSysThread )
{
	SSystem::QuickLock() ;
	ESLAssert( m_lstTempContext.FindPtr( pSysThread ) < 0 ) ;
	m_lstTempContext.Add( pSysThread ) ;
	if ( m_lstTempContext.GetLength() > 0x100 )
	{
		ESLTrace
			( "too many temporary async thread. (%d)\n",
							m_lstTempContext.GetLength() ) ;
	}
	SSystem::QuickUnlock() ;
}

// システムスレッド上で関数を呼び出し
//////////////////////////////////////////////////////////////////////////////
INT64 StandardVM::CallFunctionOnSysThread
	( INT64 addrFunc, const Register *pArg, int nArgCount )
{
	ThreadObject *	pThread = LockSystemThread() ;
	ESLAssert( pThread != NULL ) ;
	INT64	nValue = 0 ;
	if ( pThread != NULL )
	{
		const wchar_t *	pwszErr =
			pThread->CallFunction( addrFunc, pArg, nArgCount ) ;
		if ( pwszErr != NULL )
		{
			HandleExceptionError( pThread, pwszErr ) ;
		}
		else
		{
			nValue = pThread->m_regset[regAcc].i ;
		}
		UnlockSystemThread( pThread ) ;
	}
	return	nValue ;
}

INT64 StandardVM::CallAsyncFunctionOnSysThread
	( INT64 addrFunc, const Register *pArg, int nArgCount )
{
	ThreadObject *	pThread = CreateSystemAsyncThread() ;
	ESLAssert( pThread != NULL ) ;
	INT64	nValue = 0 ;
	if ( pThread != NULL )
	{
		const wchar_t *	pwszErr =
			pThread->CallFunction( addrFunc, pArg, nArgCount ) ;
		if ( pwszErr != NULL )
		{
			HandleExceptionError( pThread, pwszErr ) ;
		}
		else
		{
			nValue = pThread->m_regset[regAcc].i ;
		}
		ReleaseSystemAsyncThread( pThread ) ;
	}
	return	nValue ;
}

// システムスレッド上で仮想関数を呼び出し
//////////////////////////////////////////////////////////////////////////////
INT64 StandardVM::CallVirtualOnSysThread
	( INT64 addrObj, int iVirtual, const Register *pArg, int nArgCount )
{
	INT64 *	pObj = (INT64*) TranslateAddress( addrObj, sizeof(INT64) ) ;
	if ( pObj != NULL )
	{
		INT64 *	pVector =
			(INT64*) TranslateAddress
				( *pObj + iVirtual * sizeof(INT64), sizeof(INT64) ) ;
		if ( pVector != NULL )
		{
			return	CallFunctionOnSysThread( *pVector, pArg, nArgCount ) ;
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
	return	0 ;
}

INT64 StandardVM::CallAsyncVirtualOnSysThread
	( INT64 addrObj, int iVirtual, const Register *pArg, int nArgCount )
{
	INT64 *	pObj = (INT64*) TranslateAddress( addrObj, sizeof(INT64) ) ;
	if ( pObj != NULL )
	{
		INT64 *	pVector =
			(INT64*) TranslateAddress
				( *pObj + iVirtual * sizeof(INT64), sizeof(INT64) ) ;
		if ( pVector != NULL )
		{
			return	CallAsyncFunctionOnSysThread( *pVector, pArg, nArgCount ) ;
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
	return	0 ;
}

// アドレスからオブジェクトへ変換
//////////////////////////////////////////////////////////////////////////////
Object * StandardVM::ObjectFromAddress( DWORD dwHighAddr ) const
{
	const int	sel = (int) (dwHighAddr >> 24) ;
	return	m_pAddressRootDirectory[sel]->GetAt( dwHighAddr & 0x00FFFFFF ) ;
}

// アドレスからセグメント情報へ変換
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
	StandardVM::SegmentFromAddress
		( LinearAddressCache * plac, DWORD dwHighAddr ) const
{
	const int	sel = (int) (dwHighAddr >> 24) ;
	ECSSakura2::Object *	pObj =
		m_pAddressRootDirectory[sel]->GetAt( dwHighAddr & 0x00FFFFFF ) ;
	if ( pObj != NULL )
	{
		plac->highAddress = dwHighAddr ;
		return	pObj->GetSegmentBuffer( *plac ) ;
	}
	return	NULL ;
}

// クラス名からクラス ID を取得
//////////////////////////////////////////////////////////////////////////////
int StandardVM::GetClassIdentity( const wchar_t * pwszClassName ) const
{
	return	m_vectorNewObject.FindEntry( pwszClassName ) ;
}

// クラス ID を追加
//////////////////////////////////////////////////////////////////////////////
int StandardVM::AddClassIdentity( const wchar_t * pwszClassName )
{
	int	idClass = m_vectorNewObject.FindEntry( pwszClassName ) ;
	if ( idClass >= 0 )
	{
		return	idClass ;
	}
	return	m_vectorNewObject.AddEntry( pwszClassName ) ;
}

// クラス ID からオブジェクトを生成
//////////////////////////////////////////////////////////////////////////////
Object * StandardVM::NewObjectByIdentity( Context * context, int cls_id )
{
	PROC_NEW_OBJECT *	ppfnNewObject ;
	PROC_NEW_OBJECT		pfnNewObject = NULL ;
	Lock() ;
	ppfnNewObject = m_vectorNewObject.GetAt(cls_id) ;
	if ( ppfnNewObject != NULL )
	{
		pfnNewObject = *ppfnNewObject ;
	}
	if ( pfnNewObject == NULL )
	{
		SString *	pstrClass =
			m_vectorNewObject.GetEntryIndex().GetAt( cls_id ) ;
		ESLAssert( pstrClass != NULL ) ;
		if ( pstrClass != NULL )
		{
			pfnNewObject = GetNewObjectProc( *pstrClass ) ;
			m_vectorNewObject.SetAt( cls_id, pfnNewObject ) ;
		}
	}
	#if	defined(_CSVM_DEBUG_TRACE_NEW_OBJ)
	{
		SString *	pstrClass =
			m_vectorNewObject.GetEntryIndex().GetAt( cls_id ) ;
		if ( pstrClass != NULL )
		{
			SSystem::Trace
				( "SakuraVM: new object \'%s\' at ip=%08X:%08X\n",
					pstrClass->ToCharArray().GetConstArray(),
					context->m_ipSegment, context->m_ip ) ;
		}
	}
	#endif
	Unlock() ;
	if ( pfnNewObject != NULL )
	{
		return	pfnNewObject( context, cls_id ) ;
	}
	return	NULL ;
}

// システム関数 ID から syscall 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
const wchar_t * StandardVM::SystemCallByIdentity
	( Context * context, int syscall_id, const Register * pArg )
{
	ECSSakura2Processor::PROC_SYSCALL *	ppfnSysCall ;
	ECSSakura2Processor::PROC_SYSCALL	pfnSysCall = NULL ;
	Lock() ;
	ppfnSysCall = m_vectorSysCall.GetAt( syscall_id ) ;
	if ( ppfnSysCall != NULL )
	{
		pfnSysCall = *ppfnSysCall ;
	}
	if ( pfnSysCall == NULL )
	{
		SString *	pstrSysCall =
			m_vectorSysCall.GetEntryIndex().GetAt( syscall_id ) ;
		ESLAssert( pstrSysCall != NULL ) ;
		if ( pstrSysCall != NULL )
		{
			pfnSysCall = GetSystemCallProc( *pstrSysCall ) ;
			m_vectorSysCall.SetAt( syscall_id, pfnSysCall ) ;
		}
	}
	#if	defined(_CSVM_DEBUG_TRACE_SYSCALL)
	{
		SString *	pstrSysCall =
			m_vectorSysCall.GetEntryIndex().GetAt( syscall_id ) ;
		if ( pstrSysCall != NULL )
		{
			SSystem::Trace
				( "SakuraVM: syscall \'%s\' at ip=%08X:%08X, "
					"arg(%08X:%08X,%08X:%08X,%08X:%08X,%08X:%08X)\n",
					pstrSysCall->ToCharArray().GetConstArray(),
					context->m_ipSegment, context->m_ip,
					pArg[0].h32, pArg[0].l32, pArg[1].h32, pArg[1].l32,
					pArg[2].h32, pArg[2].l32, pArg[3].h32, pArg[3].l32 ) ;
		}
	}
	#endif
	Unlock() ;
	if ( pfnSysCall != NULL )
	{
		return	pfnSysCall( context, pArg ) ;
	}
	SString *	pstrSysCall =
		m_vectorSysCall.GetEntryIndex().GetAt( syscall_id ) ;
	m_strException = L"system call empty vector (" ;
	m_strException += *pstrSysCall ;
	m_strException += L")" ;
	return	m_strException ;
}

// new object 関数取得
//////////////////////////////////////////////////////////////////////////////
VirtualMachine::PROC_NEW_OBJECT
	StandardVM::GetNewObjectProc( const wchar_t * pwszClassName )
{
	//
	// エクスポート関数検索
	//
	SString	strSymbol = L"ecs_new_object_" ;
	AppendImportCotophaSymbol( strSymbol, pwszClassName ) ;
	//
	VirtualMachine::PROC_NEW_OBJECT
		pfnNewObject = (VirtualMachine::PROC_NEW_OBJECT)
							GetModuleExportFunction( strSymbol ) ;
	if ( pfnNewObject == NULL )
	{
		//
		// 標準関数検索
		//
		strSymbol = L"" ;
		AppendImportCotophaSymbol( strSymbol, pwszClassName ) ;
		//
		pfnNewObject =
			binarySearchForVector<NEW_OBJECT_ENTRY>
				( g_NewObjectVector,
					sizeof(g_NewObjectVector)/sizeof(NEW_OBJECT_ENTRY),
					strSymbol ) ;
	}
	return	pfnNewObject ;
}

// syscall 関数取得
//////////////////////////////////////////////////////////////////////////////
PROC_SYSCALL StandardVM::GetSystemCallProc( const wchar_t * pwszSysCall )
{
	//
	// エクスポート関数検索
	//
	SString	strSymbol = L"ecs_nakedcall_" ;
	AppendImportCotophaSymbol( strSymbol, pwszSysCall ) ;
	//
	PROC_SYSCALL	pfnSysCall =
		(ECSSakura2Processor::PROC_SYSCALL)
						GetModuleExportFunction( strSymbol ) ;
	if ( pfnSysCall != NULL )
	{
		return	pfnSysCall ;
	}
	//
	// 標準関数検索
	//
	strSymbol = L"" ;
	AppendImportCotophaSymbol( strSymbol, pwszSysCall ) ;
	//
	pfnSysCall =
		binarySearchForVector<SYSTEN_CALL_ENTRY>
			( g_SysCallVector,
				sizeof(g_SysCallVector)/sizeof(SYSTEN_CALL_ENTRY),
				strSymbol ) ;
	if ( pfnSysCall != NULL )
	{
		return	pfnSysCall ;
	}
	return	ECSSakura2Processor::GetSystemCallProc( pwszSysCall ) ;
}

// エクスポート関数取得
//////////////////////////////////////////////////////////////////////////////
void * StandardVM::GetModuleExportFunction( const wchar_t * pszFuncName )
{
	void**	ppFuncAddr = m_ssaExFuncAddr.GetAs( pszFuncName ) ;
	if ( ppFuncAddr != NULL )
	{
		return	*ppFuncAddr ;
	}
	return	(void*) SSystem::GetModuleExportFunction( pszFuncName ) ;
}

// GetModuleExportFunction で取得できる関数アドレスの登録
//////////////////////////////////////////////////////////////////////////////
void StandardVM::RegisterExportFunction
	( const wchar_t * pszFuncName, void * pFuncAddr )
{
	m_ssaExFuncAddr.SetAs( pszFuncName, pFuncAddr ) ;
}

void StandardVM::RegisterExportFunctions
	( const StandardVM::EX_FUNC_ENTRY * pFuncEntries, ssize_t nCount )
{
	if ( nCount < 0 )
	{
		nCount = 0 ;
		while ( pFuncEntries[nCount].pszFuncName != NULL )
		{
			nCount ++ ;
		}
	}
	for ( size_t i = 0; i < (size_t) nCount; i ++ )
	{
		m_ssaExFuncAddr.SetAs
			( pFuncEntries[i].pszFuncName, pFuncEntries[i].pFuncAddr ) ;
	}
}

// クラスや関数名をインポートする名前を結合する
//////////////////////////////////////////////////////////////////////////////
void StandardVM::AppendImportCotophaSymbol
	( SSystem::SString & strSymbol, const wchar_t * pwszSymbol )
{
	const size_t	lenBaseSym = strSymbol.GetLength() ;
	size_t			lenSymbol = 0 ;
	while ( pwszSymbol[lenSymbol] != 0 )
	{
		lenSymbol ++ ;
	}
	//
	uint16_t *	pszDst = strSymbol.LockBuffer( lenBaseSym + lenSymbol ) ;
	size_t		iDst = lenBaseSym, iSrc = 0 ;
	while ( iSrc < lenSymbol )
	{
		wchar_t	wch = pwszSymbol[iSrc ++] ;
		if ( wch == L':' )
		{
			if ( pwszSymbol[iSrc] == L':' )
			{
				iSrc ++ ;
			}
			pszDst[iDst ++] = '_' ;
		}
		else
		{
			pszDst[iDst ++] = (uint16_t) wch ;
		}
	}
	pszDst[iDst] = 0 ;
	//
	strSymbol.UnlockBuffer( (ssize_t) iDst ) ;
}

// ディレクトリ・テーブルを初期化
//////////////////////////////////////////////////////////////////////////////
void StandardVM::InitializeDirectoryTable( void )
{
	for ( int i = 0; i < 0x100; i ++ )
	{
		m_pAddressRootDirectory[i] = &m_ptblNull ;
	}
	m_pAddressRootDirectory[roasCode] = &m_ptblCodeImages ;
	m_pAddressRootDirectory[roasNakedGlobal] = &m_ptblNakedGlobals ;
	m_pAddressRootDirectory[roasNakedConst] = &m_ptblNakedConsts ;
	m_pAddressRootDirectory[roasNakedShared] = &m_ptblNakedShareds ;
	m_pAddressRootDirectory[roasNakedHeap] = &m_heapGlobal ;
	m_pAddressRootDirectory[roasNakedSharedHeap] = &m_heapShared ;
	m_pAddressRootDirectory[roasNakedThread] = &m_heapThread ;
	//
	m_modeDefHeap = SSystem::mallocModeGlobal ;
	m_heapGlobal.RemoveAll( this, NULL ) ;
	m_heapShared.RemoveAll( this, NULL ) ;
	m_heapThread.RemoveAll( this, NULL ) ;
	//
	m_pSysContext = NULL ;
	m_pMainThread = NULL ;
}

// デフォルトスタックサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t StandardVM::GetDefaultStackSize( void ) const
{
	ExecutableModule *	pModule = m_allocModules.GetAt( 0 ) ;
	if ( pModule != NULL )
	{
		if ( pModule->m_exmHeader.nStackSize > 0 )
		{
			return	pModule->m_exmHeader.nStackSize ;
		}
	}
	return	VirtualMachine::GetDefaultStackSize() ;
}

// デフォルトヒープサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t StandardVM::GetDefaultHeapSize( void ) const
{
	ExecutableModule *	pModule = m_allocModules.GetAt( 0 ) ;
	if ( pModule != NULL )
	{
		if ( pModule->m_exmHeader.nHeapSize > 0 )
		{
			return	pModule->m_exmHeader.nHeapSize ;
		}
	}
	return	VirtualMachine::GetDefaultHeapSize() ;
}

// メモリブロック確保
//////////////////////////////////////////////////////////////////////////////
INT64 StandardVM::AllocateHeapMemory
	( DWORD dwBytes, SSystem::MemoryAllocationMode mode )
{
	SEnvironmentInterface *	pEnv = GetEnvironment() ;
	size_t	nMaxHeapBlock = 0x400 ;
	if ( pEnv != NULL )
	{
		nMaxHeapBlock = pEnv->GetHeapBlockMaxSize() ;
	}
	if ( dwBytes <= nMaxHeapBlock )
	{
		return	AllocateHeapBlockMemory( dwBytes, mode ) ;
	}
	INT64	addrAlloc ;
	ECSSakura2::BufferObject *	pBuf = new BufferObject ;
	pBuf->CreateBuffer( dwBytes ) ;
	AssertLock() ;
	addrAlloc = AllocateHeapObjectAddress( pBuf, mode ) ;
	AssertUnlock() ;
	return	addrAlloc ;
}

// メモリブロック再確保
//////////////////////////////////////////////////////////////////////////////
INT64 StandardVM::ReallocateHeapMemory( INT64 addrBlock, DWORD dwBytes )
{
	if ( addrBlock == 0 )
	{
		return	AllocateHeapMemory( dwBytes ) ;
	}
	SEnvironmentInterface *	pEnv = GetEnvironment() ;
	size_t	nMaxHeapBlock = 0x400 ;
	if ( pEnv != NULL )
	{
		nMaxHeapBlock = pEnv->GetHeapBlockMaxSize() ;
	}
	INT64		addrAlloc = 0 ;
	const DWORD	dwHighAddr = (DWORD) (addrBlock >> 32) ;
	const DWORD	dwLowAddr = (DWORD) addrBlock ;
	Lock() ;
	Object *	pObj = ObjectFromAddress( dwHighAddr ) ;
	HeapBufferObject *	pHeap = ESLTypeCast<HeapBufferObject>( pObj ) ;
	if ( pHeap != NULL )
	{
		if ( dwBytes <= nMaxHeapBlock )
		{
			DWORD	dwAddr = dwLowAddr ;
			if ( pHeap->ReallocateHeapBlock( dwAddr, dwBytes ) )
			{
				Unlock() ;
				return	(((INT64) dwHighAddr) << 32) | dwAddr ;
			}
		}
		DWORD	dwCopyBytes =
					pHeap->GetHeapBlockLength( (DWORD) addrBlock ) ;
		if ( dwCopyBytes > dwBytes )
		{
			dwCopyBytes = dwBytes ;
		}
		ECSSakura2::BufferObject *	pBuf = new BufferObject ;
		pBuf->CreateBuffer( dwBytes ) ;
		::eslMoveMemory
			( pBuf->GetBuffer(),
				pHeap->GetBuffer() + dwLowAddr, dwCopyBytes ) ;
		pHeap->FreeHeapBlock( dwLowAddr ) ;
		Unlock() ;
		//
		SSystem::MemoryAllocationMode mode = mallocModeGlobal ;
		if ( (dwHighAddr >> 24) == roasNakedSharedHeap )
		{
			mode = mallocModeShared ;
		}
		AssertLock() ;
		addrAlloc = AllocateHeapObjectAddress( pBuf, mode ) ;
		AssertUnlock() ;
	}
	else
	{
		Unlock() ;
		//
		BufferObject *	pBuf = ESLTypeCast<BufferObject>( pObj ) ;
		if ( pBuf != NULL )
		{
			AssertLock() ;
			addrAlloc = addrBlock ;
			pBuf->ResizeBuffer( dwBytes ) ;
			AssertUnlock() ;
		}
		else
		{
			ESLTrace( "failed to ReallocateHeapMemory.\n" ) ;
		}
	}
	return	addrAlloc ;
}

// メモリブロック解放
//////////////////////////////////////////////////////////////////////////////
void StandardVM::FreeHeapMemory( INT64 addrBlock, Context * context )
{
	Object *	pObj =
		AtomicObjectFromAddress( (DWORD) (addrBlock >> 32) ) ;
	HeapBufferObject *	pHeap = ESLTypeCast<HeapBufferObject>( pObj ) ;
	if ( pHeap != NULL )
	{
		HeapIndex&	indexHeap =
			((addrBlock >> 56) == roasNakedSharedHeap)
							? m_indexSharedHeap : m_indexHeap ;
		HeapBufferObject *	pCurHeap =
			ESLTypeCast<HeapBufferObject>
				( ObjectFromAddress( indexHeap.m_dwCurrentHeap ) ) ;
		pHeap->FreeHeapBlock( (DWORD) addrBlock ) ;
		if ( pHeap->IsEmptyHeap()
			&& ((pCurHeap == NULL) || pCurHeap->IsEmptyHeap()) )
		{
//			AssertLock() ;
//			Lock() ;
			if ( pHeap->IsEmptyHeap()
				&& ((pCurHeap == NULL) || pCurHeap->IsEmptyHeap()) )
			{
				ESLTrace( "free heap object #%08X\n",
							(unsigned int) ((DWORD) (addrBlock >> 32)) ) ;
				FreeHeapObjectAddress( addrBlock, context ) ;
			}
//			Unlock() ;
//			AssertUnlock() ;
		}
	}
	else
	{
//		AssertLock() ;
		FreeHeapObjectAddress( addrBlock, context ) ;
//		AssertUnlock() ;
	}
}

// ヒープ索引を更新
//////////////////////////////////////////////////////////////////////////////
void StandardVM::UpdateHeapIndexTable( HeapIndex& indexHeap, DWORD sel )
{
	SSystem::SPointerArray<Object> *	pDir ;
	Lock() ;
	indexHeap.m_indexHeapObj.RemoveAll() ;
	indexHeap.m_dwCurrentHeap = 0 ;
	//
	pDir = m_pAddressRootDirectory[sel] ;
	if ( pDir != NULL )
	{
		size_t	nCount = pDir->GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			if ( ESLTypeCast<HeapBufferObject>( pDir->GetAt(i) ) != NULL )
			{
				indexHeap.m_indexHeapObj.Add( (sel << 24) | (DWORD) i ) ;
			}
		}
	}
	Unlock() ;
}

// ヒープ上にメモリブロック確保
//////////////////////////////////////////////////////////////////////////////
INT64 StandardVM::AllocateHeapBlockMemory
	( DWORD dwBytes, SSystem::MemoryAllocationMode mode )
{
	if ( mode == mallocModeAuto )
	{
		mode = m_modeDefHeap ;
	}
	HeapIndex&	indexHeap =
			(mode == mallocModeShared) ? m_indexSharedHeap : m_indexHeap ;
	Lock() ;
	HeapBufferObject *	pHeap =
		ESLTypeCast<HeapBufferObject>
			( ObjectFromAddress( indexHeap.m_dwCurrentHeap ) ) ;
	if ( pHeap != NULL )
	{
		//
		// 最後に割り当てられたヒープへのアロケーション
		//
		DWORD	dwAddr = 0 ;
		if ( pHeap->AllocateHeapBlock( dwAddr, dwBytes ) )
		{
			Unlock() ;
			return	(((INT64)indexHeap.m_dwCurrentHeap) << 32) | dwAddr ;
		}
		indexHeap.m_dwCurrentHeap = 0 ;
	}
	//
	// アロケーション可能なヒープが見つかるまで順次試行
	//
	for ( size_t i = 0; i < indexHeap.m_indexHeapObj.GetLength(); i ++ )
	{
		DWORD	dwHighAddr = indexHeap.m_indexHeapObj.At(i) ;
		pHeap = ESLTypeCast<HeapBufferObject>
						( ObjectFromAddress( dwHighAddr ) ) ;
		if ( pHeap != NULL )
		{
			DWORD	dwAddr = 0 ;
			if ( pHeap->AllocateHeapBlock( dwAddr, dwBytes ) )
			{
				indexHeap.m_dwCurrentHeap = dwHighAddr ;
				Unlock() ;
				return	(((INT64)dwHighAddr) << 32) | dwAddr ;
			}
		}
		else
		{
			indexHeap.m_indexHeapObj.RemoveAt( i -- ) ;
		}
	}
	Unlock() ;
	//
	// 新規ヒープ
	//
	DWORD	dwHeapSize = (DWORD) GetDefaultHeapSize() ;
	if ( dwHeapSize < dwBytes + 0x100 )
	{
		dwHeapSize = (dwBytes + 0x200) & ~0xFF ;
	}
	pHeap = new HeapBufferObject ;
	pHeap->CreateBuffer( dwHeapSize ) ;
	//
	INT64	addrAlloc ;
	AssertLock() ;
	addrAlloc = AllocateHeapObjectAddress( pHeap, mode ) ;
	AssertUnlock() ;
	//
	const DWORD	dwHighAddr = (DWORD) (addrAlloc >> 32) ;
	Lock() ;
	indexHeap.m_indexHeapObj.Add( dwHighAddr ) ;
	//
	DWORD	dwAddr = 0 ;
	if ( pHeap->AllocateHeapBlock( dwAddr, dwBytes ) )
	{
		ESLTrace( "new heap object #%08X\n", (unsigned int) dwHighAddr ) ;
		indexHeap.m_dwCurrentHeap = dwHighAddr ;
		Unlock() ;
		return	(((INT64)dwHighAddr) << 32) | dwAddr ;
	}
	Unlock() ;
	//
	ESLTrace( "failed to AllocateHeapBlockMemory.\n" ) ;
	return	0 ;
}

// オブジェクトヒープにアドレスを確保
//////////////////////////////////////////////////////////////////////////////
INT64 StandardVM::AllocateHeapObjectAddress
	( Object * pObj, MemoryAllocationMode mode )
{
	INT64	nAddress = 0 ;
//	ESLAssert( GetRuningContextCount() == 0 ) ;
	Lock() ;
	if ( mode == mallocModeAuto )
	{
		mode = m_modeDefHeap ;
	}
	if ( mode == mallocModeShared )
	{
		int	iHeap = m_heapShared.AllocateObject( pObj ) ;
		nAddress = ((INT64) ((roasNakedSharedHeap << 24) | iHeap)) << 32 ;
	}
	else if ( mode == mallocModeThread )
	{
		int	iHeap = m_heapThread.AllocateObject( pObj ) ;
		nAddress = ((INT64) ((roasNakedThread << 24) | iHeap)) << 32 ;
	}
	else
	{
		int	iHeap = m_heapGlobal.AllocateObject( pObj ) ;
		nAddress = ((INT64) ((roasNakedHeap << 24) | iHeap)) << 32 ;
	}
	Unlock() ;
	return	nAddress ;
}

INT64 StandardVM::AllocateHeapObjectAddress( INT64 nAddr, Object * pObj )
{
	const int	sel = (int) (nAddr >> 56) ;
	const int	iObj = (int) ((nAddr >> 32) & 0x00FFFFFF) ;
	ESLAssert( (nAddr >> 56) == roasNakedHeap ) ;
//	ESLAssert( GetRuningContextCount() == 0 ) ;
	if ( sel == roasNakedHeap )
	{
		Lock() ;
		int	iHeap = m_heapGlobal.AllocateObjectAt( iObj, pObj ) ;
		Unlock() ;
		return	((INT64) ((sel << 24) | iHeap)) << 32 ;
	}
	else if ( sel == roasNakedSharedHeap )
	{
		Lock() ;
		int	iHeap = m_heapShared.AllocateObjectAt( iObj, pObj ) ;
		Unlock() ;
		return	((INT64) ((sel << 24) | iHeap)) << 32 ;
	}
	else if ( sel == roasNakedThread )
	{
		Lock() ;
		int	iHeap = m_heapThread.AllocateObjectAt( iObj, pObj ) ;
		Unlock() ;
		return	((INT64) ((sel << 24) | iHeap)) << 32 ;
	}
	ESLTrace
		( "virtual address %08X:%08X is not heap.\n",
						(unsigned int) ((DWORD) (nAddr >> 32)),
						(unsigned int) ((DWORD) nAddr) ) ;
	return	0 ;
}

// オブジェクトヒープを解放
//////////////////////////////////////////////////////////////////////////////
void StandardVM::FreeHeapObjectAddress( INT64 nAddress, Context * context )
{
	const int	sel = (int) (nAddress >> 56) ;
	const int	iObj = (int) ((nAddress >> 32) & 0x00FFFFFF) ;
	Object *	pObj ;
	if ( sel == roasNakedHeap )
	{
		AssertLock() ;
		Lock() ;
		pObj = m_heapGlobal.DetachObjectAt( iObj ) ;
		Unlock() ;
		AssertUnlock() ;
		pObj->OnDestruction( this, context ) ;
		delete	pObj ;
		return ;
	}
	else if ( sel == roasNakedSharedHeap )
	{
		AssertLock() ;
		Lock() ;
		pObj = m_heapShared.DetachObjectAt( iObj ) ;
		Unlock() ;
		AssertUnlock() ;
		pObj->OnDestruction( this, context ) ;
		delete	pObj ;
		return ;
	}
	else if ( sel == roasNakedThread )
	{
		AssertLock() ;
		Lock() ;
		pObj = m_heapThread.DetachObjectAt( iObj ) ;
		Unlock() ;
		AssertUnlock() ;
		pObj->OnDestruction( this, context ) ;
		delete	pObj ;
		return ;
	}
	ESLTrace
		( "virtual address %08X:%08X is not heap.\n",
						(unsigned int) ((DWORD) (nAddress >> 32)),
						(unsigned int) ((DWORD) nAddress) ) ;
	ESLTrace( "free is called from %08X:%08X (thread #%08X)\n",
		(unsigned int) context->m_ipSegment,
		(unsigned int) context->m_ip,
		(unsigned int) (context->m_pSakura2VM
							? context->m_pSakura2VM->m_dwHighAddr : -1) ) ;
	return ;
}

// 同期オブジェクト待機処理
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::WaitSynchronism
	( Context * context,
		SSystem::SSynchronismInterface * pSync, int64_t msecTimeout )
{
	uint64_t	msecStart = SSystem::CurrentMilliSec() ;
	for ( ; ; )
	{
		if ( context->m_status != Context::xsExecution )
		{
			return	errAbort ;
		}
		SError	errWait ;
		if ( msecTimeout == SSystem::SSynchronism::Infinite )
		{
			if ( pSync != NULL )
			{
				errWait = pSync->Wait( 10 ) ;
				if ( errWait == errSuccess )
				{
					return	errSuccess ;
				}
			}
			else
			{
				SSystem::SleepMilliSec( 10 ) ;
			}
		}
		else
		{
			int64_t	msecNext = SSystem::CurrentMilliSec() - msecStart ;
			if ( msecNext > 10 )
			{
				msecNext = 10 ;
			}
			else if ( msecNext <= 0 )
			{
				msecNext = 1 ;
			}
			if ( pSync != NULL )
			{
				errWait = pSync->Wait( msecNext ) ;
				if ( errWait == errSuccess )
				{
					return	errSuccess ;
				}
			}
			else
			{
				SSystem::SleepMilliSec( (int) msecNext ) ;
			}
			if ( SSystem::CurrentMilliSec()
								- msecStart >= (uint64_t) msecTimeout )
			{
				return	errTimeout ;
			}
		}
	}
}

// 処理されない例外エラー処理
//////////////////////////////////////////////////////////////////////////////
void StandardVM::HandleExceptionError
	( Context * context, const wchar_t * pwszErr )
{
	if ( m_pExceptionHandler != NULL )
	{
		if ( m_pExceptionHandler->
				HandleExceptionError( this, context, pwszErr ) )
		{
			return ;
		}
	}
	context->m_status = Context::xsHalt ;
	AtomicOr( &(context->m_maskException), interruptChangeStatus ) ;
	//
	SString			strErrMsg = pwszErr ;
	SArray<char>	bufErrMsg ;
	SSystem::Trace( "unhandle exception error\n" ) ;
	SSystem::Trace( "%s\n\n", strErrMsg.EncodeDefaultTo(bufErrMsg) ) ;
	context->TraceDumpRegister() ;
	//
	SString		strHighIP, strLowIP ;
	strHighIP.HexFromInteger( context->m_ipSegment, 8 ) ;
	strLowIP.HexFromInteger( context->m_ip, 8 ) ;
	//
	strErrMsg = L"例外エラーが発生しました (" ;
	strErrMsg += strHighIP ;
	strErrMsg += L":" ;
	strErrMsg += strLowIP ;
	strErrMsg += L")\n" ;
	strErrMsg += pwszErr ;
	strErrMsg += L"\n\n" ;
	//
	STACK_FRAME_INFO	sfi, sfiCurrent ;
	Register			ipTrace, bpTrace ;
	ipTrace.l32 = context->m_ip ;
	ipTrace.h32 = context->m_ipSegment ;
	bpTrace = context->m_regset[regBP] ;
	sfiCurrent.pFuncArg = NULL ;
	sfiCurrent.countArg = 0 ;
	//
	StandardVM::Lock() ;
	size_t	i ;
	SSystem::Trace( "\ncall trace;\n" ) ;
	strErrMsg += L"call trace;\n" ;
	for ( i = 0; i < 0x100; i ++ )
	{
		if ( TraceStackFrameInfo( sfi, context, ipTrace, bpTrace ) )
		{
			if ( i == 0 )
			{
				Register *	pStack =
					(Register*) TranslateAddress
						( context->m_regset[regSP].i, sizeof(Register) ) ;
				if ( pStack != NULL )
				{
					ipTrace = *pStack ;
					bpTrace = context->m_regset[regBP] ;
					continue ;
				}
			}
			break ;
		}
		if ( i == 0 )
		{
			sfiCurrent = sfi ;
		}
		if ( sfi.pwszFuncName != NULL )
		{
			SString	strFuncInfo = sfi.pwszFuncName ;
			strFuncInfo += L"( " ;
			strFuncInfo += FormatStackFrameArguments( sfi ) ;
			strFuncInfo += L" ) at " ;
			strFuncInfo += SString( ipTrace.h32, 8, 16 ) ;
			strFuncInfo += L":" ;
			strFuncInfo += SString( ipTrace.l32, 8, 16 ) ;
			//
			if ( i < 8 )
			{
				strErrMsg += strFuncInfo ;
				strErrMsg += L"\n" ;
			}
			else if ( i == 8 )
			{
				strErrMsg += L"..." ;
			}
			SArray<char>	bufFuncInfo ;
			SSystem::Trace
				( "%s\n", strFuncInfo.EncodeDefaultTo(bufFuncInfo) ) ;
		}
		ipTrace = sfi.ipReturn ;
		bpTrace = sfi.bpReturn ;
	}
	SSystem::Trace( "\n" ) ;
	//
	SString	strDumpTP ;
	if ( FormatMemoryDump( strDumpTP, L"tp", context->m_regset[regTP].i ) )
	{
		strErrMsg += L"\n" ;
		strErrMsg += strDumpTP ;
		strErrMsg += L"\n" ;
		//
		SArray<char>	bufDump ;
		SSystem::Trace
			( "%s\n", strDumpTP.EncodeDefaultTo(bufDump) ) ;
	}
	for ( i = 0; i < sfiCurrent.countArg; i ++ )
	{
		if ( sfiCurrent.pFuncArg[i].i != context->m_regset[regTP].i )
		{
			SString	strDump ;
			SString	strArgName = L"arg[" ;
			strArgName += SString( i ) ;
			strArgName += L"]" ;
			if ( FormatMemoryDump
				( strDump, strArgName, sfiCurrent.pFuncArg[i].i ) )
			{
				SArray<char>	bufDump ;
				SSystem::Trace
					( "\n%s\n", strDump.EncodeDefaultTo(bufDump) ) ;
			}
		}
	}
	StandardVM::Unlock() ;
	//
	SSystem::MessageBox( strErrMsg, L"EntisGLS4 - Sakura2VM" ) ;
}

// 例外エラーハンドラ設定
//////////////////////////////////////////////////////////////////////////////
void StandardVM::AttachExceptionHandler( ExceptionHandler * pHandler )
{
	m_pExceptionHandler = pHandler ;
}

// スタックフレームのトレース
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::TraceStackFrameInfo
	( STACK_FRAME_INFO& sfi, Context * context,
			const Register& ip, const Register& bp )
{
	//
	// モジュール取得
	//
	if ( ((ip.h32 >> 24) != roasCode) || (ip.l32 == 0xFFFFFFFF) )
	{
		return	errFailed ;
	}
	ExecutableModule *	pModule =
		m_allocModules.GetAt( ip.h32 & 0x00FFFFFF ) ;
	if ( pModule == NULL )
	{
		return	errFailed ;
	}
	//
	// コードバッファ取得
	//
	DWORD	dwCodeLength = pModule->m_bufCode.GetLength() ;
	DWORD	dwCodeBase = pModule->m_bufCode.GetBufferBase() ;
	BYTE *	pbytCode = pModule->m_bufCode.GetCodeShadowBuffer() ;
	if ( pbytCode == NULL )
	{
		pbytCode = pModule->m_bufCode.GetBuffer() ;
	}
	if ( pbytCode == NULL )
	{
		return	errFailed ;
	}
	//
	// 関数情報取得
	//
	sfi.pFuncEntry = NULL ;
	sfi.pwszFuncName =
		pModule->SearchFunctionAtAddress( ip.l32, &(sfi.pFuncEntry) ) ;
	if ( (sfi.pFuncEntry == NULL)
		|| (sfi.pFuncEntry->dwAddress >= (dwCodeLength + dwCodeBase)) )
	{
		return	errFailed ;
	}
	BYTE *	pbytFuncCode = pbytCode + sfi.pFuncEntry->dwAddress ;
	//
	// 関数の呼び出し元取得
	//
	Register *	pStackBP =
		(Register*) TranslateAddress( bp.i, sizeof(Register) * 2 ) ;
	if ( pStackBP == NULL )
	{
		return	errFailed ;
	}
	sfi.bpReturn = pStackBP[0] ;
	//
	Register *	pStackCall = pStackBP + 1 ;
	if ( (pbytFuncCode[0] == codePushRegs) && (pbytFuncCode[1] == regBP) )
	{
		pStackCall = pStackBP + pbytFuncCode[2] ;
	}
	sfi.ipReturn = pStackCall[0] ;
	//
	// 呼び出し元 IP の add.sp imm32 を判定
	//
	if ( ((sfi.ipReturn.h32 >> 24) != roasCode)
				|| (sfi.ipReturn.l32 == 0xFFFFFFFF) )
	{
		return	errFailed ;
	}
	ExecutableModule *	pCallModule =
		m_allocModules.GetAt( sfi.ipReturn.h32 & 0x00FFFFFF ) ;
	if ( pCallModule == NULL )
	{
		return	errFailed ;
	}
	DWORD	dwCallCodeLength = pCallModule->m_bufCode.GetLength() ;
	DWORD	dwCallCodeBase = pCallModule->m_bufCode.GetBufferBase() ;
	BYTE *	pbytCallCode = pCallModule->m_bufCode.GetCodeShadowBuffer() ;
	if ( pbytCallCode == NULL )
	{
		pbytCallCode = pCallModule->m_bufCode.GetBuffer() ;
	}
	if ( (pbytCallCode == NULL)
		|| (sfi.ipReturn.l32 + 5 > (dwCodeLength + dwCodeBase)) )
	{
		return	errFailed ;
	}
	BYTE *	pbytRetCode = pbytCallCode + sfi.ipReturn.l32 ;
	if ( pbytRetCode[0] == codeAddSPImm32 )
	{
		sfi.pFuncArg = pStackCall + 1 ;
		sfi.countArg = *((DWORD*)(pbytRetCode + 1)) / sizeof(Register) ;
	}
	else
	{
		sfi.pFuncArg = NULL ;
		sfi.countArg = 0 ;
	}
	return	errSuccess ;
}

// スタックフレーム情報の表示形式フォーマット
//////////////////////////////////////////////////////////////////////////////
SSystem::SString
	StandardVM::FormatStackFrameArguments
				( const StandardVM::STACK_FRAME_INFO& sfi )
{
	if ( sfi.pFuncArg == NULL )
	{
		return	L"" ;
	}
	SString	strFormat ;
	for ( size_t i = 0; i < sfi.countArg; i ++ )
	{
		if ( i != 0 )
		{
			strFormat += L", " ;
		}
		strFormat += FormatStackFrameArgumentValue( sfi.pFuncArg[i] ) ;
	}
	return	strFormat ;
}

SSystem::SString
	StandardVM::FormatStackFrameArgumentValue( const Register& value )
{
	if ( (-1000000 <= value.i) && (value.i <= 1000000) )
	{
		return	SString( value.i ) ;
	}
	else if ( (value.h32 & 0xFFF00000)
		&& (-1000000.0 <= value.f) && (value.f <= 1000000)
		&& ((value.f <= -0.01) || (0.01 <= value.f))
		&& (ObjectFromAddress( value.h32 ) == NULL) )
	{
		return	SString( value.f, 0 ) ;
	}
	else if ( value.h32 == 0 )
	{
		return	SString( L"0x" ) + SString( value.i, 0, 16 ) ;
	}
	else if ( (value.h32 == 0xFFFFFFFF) && (value.l32 & 0x80000000) )
	{
		return	SString( L"-0x" ) + SString( -value.i, 0, 16 ) ;
	}
	else
	{
		return	SString( L"0x" ) + SString( value.h32, 8, 16 )
								+ L":" + SString( value.l32, 8, 16 ) ;
	}
}

// メモリダンプのトレース
//////////////////////////////////////////////////////////////////////////////
bool StandardVM::FormatMemoryDump
	( SSystem::SString& strDump,
		const wchar_t * pwszPointer, INT64 nAddress )
{
	Object *	pObjPtr = ObjectFromAddress( (DWORD) (nAddress >> 32) ) ;
	if ( pObjPtr == NULL )
	{
		return	false ;
	}
	strDump += pwszPointer ;
	strDump += L"=" ;
	strDump += SString( (DWORD) (nAddress >> 32), 8, 16 ) ;
	strDump += L":" ;
	strDump += SString( (DWORD) nAddress, 8, 16 ) ;
	strDump += L"\n" ;
	//
	strDump += L"Object #" ;
	strDump += SString( (DWORD) (nAddress >> 32), 8, 16 ) ;
	strDump += L" : " ;
	strDump += pObjPtr->GetTypeName() ;
	//
	LinearAddressCache	lacPtr ;
	if ( pObjPtr->GetSegmentBuffer( lacPtr ) != NULL )
	{
		strDump += L"; " ;
		strDump += SString( lacPtr.baseOffset, 8, 16 ) ;
		strDump += L"-" ;
		strDump +=
			SString( lacPtr.baseOffset + lacPtr.limitSegment, 8, 16 ) ;
		//
		for ( size_t i = 0; i < 4; i ++ )
		{
			SString	strTemp = L"\n" ;
			strTemp += pwszPointer ;
			strTemp += L"+" ;
			strTemp += SString( i, 2, 16 ) ;
			strTemp += L" : " ;
			strTemp += FormatMemoryDumpLine( nAddress + i * 0x10, 16 ) ;
			strDump += strTemp ;
		}
	}
	strDump += L"\n" ;
	return	true ;
}

// 16進メモリダンプ表示形式フォーマット
//////////////////////////////////////////////////////////////////////////////
SSystem::SString StandardVM::FormatMemoryDumpLine( INT64 nAddress, size_t nCount )
{
	SString	strDump ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( i != 0 )
		{
			strDump += L" " ;
		}
		BYTE *	pbytMem = TranslateAddress( nAddress + i, 1 ) ;
		if ( pbytMem != NULL )
		{
			strDump += SString( pbytMem[0], 2, 16 ) ;
		}
		else
		{
			strDump += L"??" ;
		}
	}
	return	strDump ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface * StandardVM::NewOpenFile
		( const wchar_t * pwszFilePath, long int nOpenFlags ) const
{
	if ( (pwszFilePath == NULL) || (pwszFilePath[0] == 0)
		|| (SString::Compare(pwszFilePath,SFile::DefaultName::StandardOutput) == 0)
		|| (SString::Compare(pwszFilePath,SFile::DefaultName::StandardInput) == 0) )
	{
		#if	defined(__PLATFORM_ANDROID__)
			return	new SConsoleFile ;
		#else
			SFile *	pFile = new SFile ;
			if ( pFile->Open( pwszFilePath, nOpenFlags ) )
			{
				delete	pFile ;
				return	NULL ;
			}
			return	pFile ;
		#endif
	}
	if ( !(nOpenFlags & SFileInterface::modeWrite) )
	{
		Lock() ;
		size_t	nCount = m_allocModules.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			ExecutableModule *	pModule = m_allocModules.GetAt( i ) ;
			if ( pModule != NULL )
			{
				SFileOpener *	pOpener = pModule->m_pOpener ;
				if ( pOpener != NULL )
				{
					SFileInterface *	pFile =
						pOpener->NewOpenFile( pwszFilePath, nOpenFlags ) ;
					if ( pFile != NULL )
					{
						Unlock() ;
						return	pFile ;
					}
				}
			}
		}
		Unlock() ;
	}
	return	VirtualMachine::NewOpenFile( pwszFilePath, nOpenFlags ) ;
}

// ファイルは存在するか？
//////////////////////////////////////////////////////////////////////////////
bool StandardVM::IsExistingFile( const wchar_t * pwszFilePath ) const
{
	Lock() ;
	size_t	nCount = m_allocModules.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ExecutableModule *	pModule = m_allocModules.GetAt( i ) ;
		if ( pModule != NULL )
		{
			SFileOpener *	pOpener = pModule->m_pOpener ;
			if ( pOpener != NULL )
			{
				if ( pOpener->IsExisting( pwszFilePath ) )
				{
					return	true ;
				}
			}
		}
	}
	Unlock() ;
	return	VirtualMachine::IsExistingFile( pwszFilePath ) ;
}

// モジュールをアロケーション
//////////////////////////////////////////////////////////////////////////////
int StandardVM::AllocateModule( ExecutableModule * pModule )
{
	return	AllocateModuleAt( AttachModule( pModule ), pModule ) ;
}

int StandardVM::AllocateModuleAt( int iModule, ExecutableModule * pModule )
{
	Lock() ;
	ESLAssert( m_allocModules.GetAt(iModule) == pModule ) ;
	pModule->ReallocateModule( iModule, m_vectorNewObject, m_vectorSysCall ) ;
	int	nUnsolved = pModule->ImportSymbols( m_allocModules ) ;
	if ( nUnsolved > 0 )
	{
		ESLTrace( "there are %d unsolved symbols"
					" in AllocateModuleAt.\n", nUnsolved ) ;
	}
	m_ptblCodeImages.SetAt( iModule, &(pModule->m_bufCode) ) ;
	pModule->m_bufCode.m_dwHighAddr = (roasCode << 24) | iModule ;
	//
	m_ptblNakedGlobals.SetAt( iModule, &(pModule->m_bufGlobal) ) ;
	pModule->m_bufGlobal.m_dwHighAddr = (roasNakedGlobal << 24) | iModule ;
	//
	m_ptblNakedConsts.SetAt( iModule, &(pModule->m_bufConst) ) ;
	pModule->m_bufConst.m_dwHighAddr = (roasNakedConst << 24) | iModule ;
	//
	m_ptblNakedShareds.SetAt( iModule, &(pModule->m_bufShared) ) ;
	pModule->m_bufShared.m_dwHighAddr = (roasNakedShared << 24) | iModule ;
	//
	pModule->m_iModule = iModule ;
	OnModuleAttached( iModule, pModule ) ;
	Unlock() ;
	return	iModule ;
}

// モジュールのアロケーションを解放
//////////////////////////////////////////////////////////////////////////////
void StandardVM::FreeModuleAllocation( ExecutableModule * pModule )
{
	Lock() ;
	ssize_t	iModule = m_allocModules.FindPtr( pModule ) ;
	if ( iModule >= 0 )
	{
		OnModuleDetached( (int) iModule, pModule ) ;
		m_allocModules.SetAt( iModule, NULL ) ;
		m_ptblCodeImages.SetAt( iModule, NULL ) ;
		m_ptblNakedGlobals.SetAt( iModule, NULL ) ;
		m_ptblNakedConsts.SetAt( iModule, NULL ) ;
		m_ptblNakedShareds.SetAt( iModule, NULL ) ;
		pModule->m_bufCode.m_dwHighAddr = 0 ;
		pModule->m_bufGlobal.m_dwHighAddr = 0 ;
		pModule->m_bufConst.m_dwHighAddr = 0 ;
		pModule->m_bufShared.m_dwHighAddr = 0 ;
		pModule->m_iModule = -1 ;
	}
	Unlock() ;
}

// モジュール番号をアロケーション
//////////////////////////////////////////////////////////////////////////////
int StandardVM::AttachModule( ExecutableModule * pModule )
{
	ssize_t	iModule ;
	Lock() ;
	iModule = m_allocModules.FindPtr( NULL ) ;
	if ( iModule >= 0 )
	{
		m_allocModules.SetAt( iModule, pModule ) ;
	}
	else
	{
		iModule = (int) m_allocModules.Add( pModule ) ;
	}
	Unlock() ;
	return	(int) iModule ;
}

// 特定のモジュール番号にアロケーション
//////////////////////////////////////////////////////////////////////////////
int StandardVM::AttachModuleAt( int iModule, ExecutableModule * pModule )
{
	Lock() ;
	ESLAssert( m_allocModules.GetAt( iModule ) == NULL ) ;
	m_allocModules.SetAt( iModule, pModule ) ;
	Unlock() ;
	return	iModule ;
}

// モジュール番号を検索
//////////////////////////////////////////////////////////////////////////////
int StandardVM::FindModule( ExecutableModule * pModule )
{
	ssize_t	iModule ;
	Lock() ;
	iModule = m_allocModules.FindPtr( pModule ) ;
	Unlock() ;
	return	(int) iModule ;
}

// モジュール番号を解放
//////////////////////////////////////////////////////////////////////////////
void StandardVM::DetachModule( ExecutableModule * pModule )
{
	ssize_t	iModule ;
	Lock() ;
	iModule = m_allocModules.FindPtr( pModule ) ;
	if ( iModule >= 0 )
	{
		m_allocModules.SetAt( iModule, NULL ) ;
	}
	Unlock() ;
}

void StandardVM::DetachModuleAt( int iModule )
{
	Lock() ;
	ESLAssert( m_allocModules.GetAt( iModule ) != NULL ) ;
	m_allocModules.SetAt( iModule, NULL ) ;
	Unlock() ;
}

// モジュール総数取得
//////////////////////////////////////////////////////////////////////////////
size_t StandardVM::GetModuleCount( void ) const
{
	return	m_allocModules.GetLength() ;
}

// モジュール取得
//////////////////////////////////////////////////////////////////////////////
ExecutableModule * StandardVM::GetModuleAt( int iModule )
{
	ExecutableModule *	pModule ;
	Lock() ;
	pModule = m_allocModules.GetAt( iModule ) ;
	Unlock() ;
	return	pModule ;
}

// 全モジュールの関数を検索
//////////////////////////////////////////////////////////////////////////////
uint64_t StandardVM::GetFunctionAddress
	( const wchar_t * pwszFuncName, const wchar_t * pszReserved )
{
	uint64_t	addrFunc = 0 ;
	Lock() ;
	const size_t	nCount = m_allocModules.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ExecutableModule *	pModule = m_allocModules.GetAt( i ) ;
		if ( pModule != NULL )
		{
			ExecutableModule::FUNC_ENTRY *
					pFunc = pModule->GetFunctionEntry( pwszFuncName ) ;
			if ( pFunc != NULL )
			{
				const DWORD	dwHighIP =
								(roasCode << 24)
									| (pModule->m_iModule & 0x00FFFFFF) ;
				addrFunc = (((uint64_t)dwHighIP) << 32) | pFunc->dwAddress ;
				break ;
			}
		}
	}
	Unlock() ;
	return	addrFunc ;
}

// コードアドレスから関数情報を検索
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::SearchFunctionAtAddress
	( StandardVM::SearchFunctionInfo& sfi, INT64 nAddress )
{
	SError	err = errFailed ;
	Lock() ;
	const size_t	nCount = m_allocModules.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ExecutableModule *	pModule = m_allocModules.GetAt( i ) ;
		if ( (pModule != NULL)
			&& (pModule->m_bufCode.m_dwHighAddr == (DWORD) (nAddress >> 32)) )
		{
			sfi.pModule = pModule ;
			sfi.pwszFuncName =
				pModule->SearchFunctionAtAddress
					( (DWORD) nAddress, &(sfi.pFuncEntry) ) ;
			if ( sfi.pwszFuncName != NULL )
			{
				err = errSuccess ;
			}
			break ;
		}
	}
	Unlock() ;
	return	err ;
}

// メインスレッドと new object & syscall ベクタの保存
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::SaveMainThreadAndSysVector( SFileInterface * pfile )
{
	SError	err ;
	DWORD	dwHeader = 0 ;
	if ( m_pMainThread == NULL )
	{
		pfile->Write( &dwHeader, sizeof(DWORD) ) ;
		return	errSuccess ;
	}
	dwHeader = 0x07 ;
	pfile->Write( &dwHeader, sizeof(DWORD) ) ;
	//
	ThreadObject *	pSysThread = LockSystemThread() ;
	do
	{
		err = m_pMainThread->SaveStatic( pfile, this, pSysThread ) ;
		if ( err )
		{
			break ;
		}
		err = SaveNewObjectVector( pfile ) ;
		if ( err )
		{
			break ;
		}
		err = SaveSystemCallVector( pfile ) ;
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

// new object ベクタ保存
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::SaveNewObjectVector( SFileInterface * pfile )
{
	return	SaveStringIndexedArray
					( m_vectorNewObject.GetEntryIndex(), pfile ) ;
}

// syscall ベクタ保存
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::SaveSystemCallVector( SFileInterface * pfile )
{
	return	SaveStringIndexedArray
					( m_vectorSysCall.GetEntryIndex(), pfile ) ;
}

// メインスレッドと new object & syscall ベクタの復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::LoadMainThreadAndSysVector( SFileInterface * pfile )
{
	DWORD	dwHeader = 0 ;
	if ( pfile->Read( &dwHeader, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	ThreadObject *	pSysThread = LockSystemThread() ;
	SError			err = errSuccess ;
	do
	{
		if ( dwHeader & 0x01 )
		{
			ThreadObject *	pMainThread = CreateMainThread() ;
			err = pMainThread->LoadStatic( pfile, this, pSysThread ) ;
			if ( err )
			{
				break ;
			}
		}
		if ( dwHeader & 0x02 )
		{
			err = LoadNewObjectVector( pfile ) ;
			if ( err )
			{
				break ;
			}
		}
		if ( dwHeader & 0x04 )
		{
			err = LoadSystemCallVector( pfile ) ;
		}
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

// new object ベクタ復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::LoadNewObjectVector( SFileInterface * pfile )
{
	m_vectorNewObject.RemoveAll() ;
	return	LoadStringIndexedArray
					( m_vectorNewObject.GetEntryIndex(), pfile ) ;
}

// syscall ベクタ復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::LoadSystemCallVector( SFileInterface * pfile )
{
	m_vectorSysCall.RemoveAll() ;
	return	LoadStringIndexedArray
					( m_vectorSysCall.GetEntryIndex(), pfile ) ;
}

// ベクタ保存
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::SaveStringIndexedArray
	( const SSystem::SIndexedArray
			<SSystem::SString,const wchar_t*> & indexArray,
				SFileInterface * pfile )
{
	DWORD	dwCount = (DWORD) indexArray.GetLength() ;
	if ( pfile->Write( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		SString *	pstr = indexArray.GetAt( i ) ;
		if ( pstr != NULL )
		{
			SError	err = pfile->WriteString( *pstr ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			DWORD	dwLength = (DWORD) -1 ;
			pfile->Write( &dwLength, sizeof(DWORD) ) ;
		}
	}
	return	errSuccess ;
}

// ベクタ復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError StandardVM::LoadStringIndexedArray
	( SSystem::SIndexedArray
			<SSystem::SString,const wchar_t*> & indexArray,
				SFileInterface * pfile )
{
	indexArray.RemoveAll() ;
	//
	DWORD	dwCount = 0 ;
	if ( pfile->Read( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	for ( DWORD i = 0; i < dwCount; i ++ )
	{
		SString *	pstr = new SString ;
		SError	err = pfile->ReadString( *pstr ) ;
		if ( err )
		{
			delete	pstr ;
			return	err ;
		}
		indexArray.Add( pstr ) ;
	}
	return	errSuccess ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * StandardVM::GetTypeName( void ) const
{
	return	L"SSystem::Sakura2VM" ;
}

// 破棄処理
//////////////////////////////////////////////////////////////////////////////
void StandardVM::OnDestruction
	( VirtualMachine * vm, Context * context )
{
	ReleaseVM() ;
}

// 保存準備処理
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::PrepareSave
	( VirtualMachine * vm, Context * context )
{
	ThreadObject *	pSysThread = LockSystemThread() ;
	if ( m_pMainThread != NULL )
	{
		m_pMainThread->PrepareSave( this, pSysThread ) ;
	}
	SError	err = m_heapGlobal.PrepareSave( this, pSysThread ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ThreadObject *	pSysThread = LockSystemThread() ;
	SError	err ;
	do
	{
		err = SaveMainThreadAndSysVector( file ) ;
		if ( err )
		{
			break ;
		}
		err = m_heapGlobal.SaveHeapStatic( file, this, pSysThread ) ;
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

SError StandardVM::SaveDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ThreadObject *	pSysThread = LockSystemThread() ;
	SError	err ;
	do
	{
		err = SaveMainThreadAndSysVector( file ) ;
		if ( err )
		{
			break ;
		}
		err = m_heapGlobal.SaveHeapDynamic( file, this, pSysThread ) ;
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ThreadObject *	pSysThread = LockSystemThread() ;
	SError	err ;
	do
	{
		m_heapGlobal.RemoveAll( this, pSysThread ) ;
		//
		err = LoadMainThreadAndSysVector( file ) ;
		if ( err )
		{
			break ;
		}
		err = m_heapGlobal.LoadHeapStatic( file, this, pSysThread ) ;
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

SError StandardVM::LoadDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ThreadObject *	pSysThread = LockSystemThread() ;
	SError	err ;
	do
	{
		m_heapGlobal.RemoveAll( this, pSysThread ) ;
		//
		err = LoadMainThreadAndSysVector( file ) ;
		if ( err )
		{
			break ;
		}
		err = m_heapGlobal.LoadHeapDynamic( file, this, pSysThread ) ;
	}
	while ( false ) ;
	UnlockSystemThread( pSysThread ) ;
	return	err ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	return	m_heapGlobal.CommitAfterLoad( this, context ) ;
}

// 復元後の後のスクリプト処理
//////////////////////////////////////////////////////////////////////////////
SError StandardVM::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	return	m_heapGlobal.OnLoadedDynamic( this, context ) ;
}



