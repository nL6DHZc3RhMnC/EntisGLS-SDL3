
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakura/sakura.h>
#include <sakura/ssys_heap_memory.h>
#include <sakura/ssys_fragment_file.h>
#include <sakura/ssys_socket.h>
#include <sakura/ssys_http_file.h>
#include <sakura/ssys_win_resource_file.h>
#include <sakuragl/sgl_window.h>
#include <stdio.h>

#if	_MSC_VER >= 1800
#include <VersionHelpers.h>
#include <processthreadsapi.h>
#endif

using namespace SSystem ;
using namespace	SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 定数
//////////////////////////////////////////////////////////////////////////////

const double	SSystem::PI = 3.1415926535897932384626433832795 ;



//////////////////////////////////////////////////////////////////////////////
// ライブラリ初期化
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( PLATFORM_INFORMATION	SSystem::g_infoPlatform ) ;
ESL_DLL_DECL( SSystem::CPU_Family	SSystem::g_cpuFamily ) ;
ESL_DLL_DECL( uint64_t				SSystem::g_cpuFeatures ) ;
ESL_DLL_DECL( unsigned int			SSystem::g_cpuLogicalCount ) ;
ESL_DLL_DECL( SSharableMutex *		SSystem::g_mutexGlobal = NULL ) ;
ESL_DLL_DECL( SCriticalSection *	SSystem::g_csmutexGlobal = NULL ) ;
ESL_DLL_DECL( DWORD					SSystem::g_tlsThread = (DWORD) -1 ) ;
ESL_DLL_DECL( DWORD					SSystem::g_tlsStorage = (DWORD) -1 ) ;

static atomic_int_t		g_countRefSystem = 0 ;
static uint64_t			g_nCurrentTime = 0 ;
static DWORD			g_dwLastTime = 0 ;


void SSystem::Initialize( void )
{
#if	!defined(ENTISGLS4_DLL_IMPORT)
	if ( AtomicAdd( &g_countRefSystem, 1 ) == 1 )
	{
		Trace( "SSystem::Initialize (%s)\n", entisgls4_version_name ) ;
		::CoInitialize( NULL ) ;
		//
		SSystem::eslHeapInitialize() ;
		//
		WORD	wReqVer = MAKEWORD( 1, 1 );
		WSADATA	wsaData ;
		::WSAStartup( wReqVer, &wsaData ) ;
		//
		DATE_TIME	dtCurrent ;
		wchar_t		wszTimeZone[32] ;
		int32_t		nDiffTime =
				SSystem::DifferenceInLocalTime( &wszTimeZone[0], 32 ) / -60 ;
		CurrentLocalDate( dtCurrent ) ;
		Trace( "current time: %04d/%02d/%02d %02d:%02d:%02d %s %+03d:%02d\n",
				dtCurrent.nYear, dtCurrent.nMonth, dtCurrent.nDay,
				dtCurrent.nHour, dtCurrent.nMinute, dtCurrent.nSecond,
				SString(&wszTimeZone[0]).ToCharArray().GetConstArray(),
				(nDiffTime / 60), (nDiffTime % 60) ) ;
		//
		LANGID	langID = GetUserDefaultLangID() ;
		Trace( "Language ID: %04X\n", langID ) ;
		if ( (langID & LANG_INVARIANT) == LANG_JAPANESE )
		{
			Trace( " Japanese mode\n" ) ;
			g_languageTarget = languageJapanese ;
		}
		else
		{
			Trace( " English mode\n" ) ;
			g_languageTarget = languageEnglish ;
		}
		//
		GetPlatformInformation( g_infoPlatform ) ;
		g_cpuFamily = GetCPUFamily() ;
		g_cpuFeatures = GetCPUFeatures() ;
		g_cpuLogicalCount = GetLogicalProcessorCount() ;
		Trace( "CPU family: %d\n", g_cpuFamily ) ;
		Trace( "CPU features: 0x%08x\n", g_cpuFeatures ) ;
		Trace( "logical processors: %d\n", g_cpuLogicalCount ) ;
		//
		MEMORY_STATUS	mstatus ;
		GetMemoryStatus( mstatus ) ;
		Trace( "system memory info;\n" ) ;
		Trace( "  total physical: %d [MB]\n", (int) (mstatus.nTotalPhys >> 20) ) ;
		Trace( "  avail physical: %d [MB]\n", (int) (mstatus.nAvailPhys >> 20) ) ;
		Trace( "  total virtual: %d [MB]\n", (int) (mstatus.nTotalVirtual >> 20) ) ;
		Trace( "  avail virtual: %d [MB]\n", (int) (mstatus.nAvailVirtual >> 20) ) ;
		//
		g_mutexGlobal = new SSharableMutex() ;
		g_mutexGlobal->Initialize() ;
		//
		g_csmutexGlobal = new SSystem::SCriticalSection ;
		//
		g_tlsThread = ::TlsAlloc() ;
		g_tlsStorage = ::TlsAlloc() ;
		ResetCurrentMilliSec( 0 ) ;
		::timeBeginPeriod( 1 ) ;
//		SThread::TestSwitchingPerformance() ;
		//
		SInternetSession *	pSession = new SInternetSession ;
		pSession->Open( L"Sakura2VM" ) ;
		SInternetSession::AttachInstance( pSession ) ;
		//
		SFileOpener::SetDefaultOpener( &g_defURLOpener ) ;
		g_defURLOpener.RegisterScheme
				( L"file://", new SStandardFileOpener ) ;
		g_defURLOpener.RegisterScheme
				( L"http://", new SOffsetFileOpener
					( L"http://", L'/', new SHttpFileOpener, true ),
					SVirtualURLOpener::schemeOverNetwork ) ;
		g_defURLOpener.RegisterScheme
				( L"https://", new SOffsetFileOpener
					( L"https://", L'/', new SHttpFileOpener, true ),
					SVirtualURLOpener::schemeOverNetwork ) ;
		//
		SFileOpener *	pAssetsOpener =
			new SWin32PEBinResourceOpener( ::GetModuleHandle( NULL ) ) ;
		g_defURLOpener.RegisterScheme( L"assets://", pAssetsOpener ) ;
		g_defURLOpener.RegisterScheme
			( L"fragments://",
				new SFragmentFileOpener
					( L"", L'/', new SStandardFileOpener, true ) ) ;
	}
#endif
}

void SSystem::Finalize( void )
{
#if	!defined(ENTISGLS4_DLL_IMPORT)
	if ( AtomicSub( &g_countRefSystem, 1 ) == 0 )
	{
		Trace( "SSystem::Finalize\n" ) ;
		g_dnsLibServ.NotifyAllOnFinalize() ;
		//
		SThread::ExitAllStockedThread() ;
		SSystem::RemoveAllMemoryTightnessNotification() ;
		//
		delete	SInternetSession::GetInstance() ;
		SInternetSession::AttachInstance( NULL ) ;
		SEnvironmentInterface::AttachInstance( NULL ) ;
		//
		g_defURLOpener.UnregisterAllScheme() ;
		//
		SSystem::RemoveAllMemoryTightnessNotification() ;
		SSystem::g_eslMemoryTightness = false ;
		//
		delete	g_mutexGlobal ;
		delete	g_csmutexGlobal ;
		g_mutexGlobal = nullptr ;
		g_csmutexGlobal = nullptr ;
		//
		SThread::ReleaseLocalStorage() ;
		::TlsFree( g_tlsThread ) ;
		::TlsFree( g_tlsStorage ) ;
		g_tlsThread = (DWORD) -1 ;
		g_tlsStorage = (DWORD) -1 ;
		//
		SSystem::eslHeapUninitialize() ;
		//
//		::CoUninitialize() ;
	}
#endif
}


//////////////////////////////////////////////////////////////////////////////
// システム（メモリ）
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( ESL_FUNCPTR_MALLOC	esl_stub_malloc = ::malloc ) ;
ESL_DLL_DECL( ESL_FUNCPTR_REALLOC	esl_stub_relloc = ::realloc ) ;
ESL_DLL_DECL( ESL_FUNCPTR_FREE		esl_stub_free = ::free ) ;

void SSystem::GetMemoryStatus( MEMORY_STATUS& mstatus )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	MEMORYSTATUSEX	memst ;
	::eslFillMemory( &memst, 0, sizeof(MEMORYSTATUSEX) ) ;
	memst.dwLength = sizeof(MEMORYSTATUSEX) ;
	::GlobalMemoryStatusEx( &memst ) ;
	//
	mstatus.nTotalPhys = memst.ullTotalPhys ;
	mstatus.nAvailPhys = memst.ullAvailPhys ;
	mstatus.nTotalVirtual = memst.ullTotalVirtual ;
	mstatus.nAvailVirtual = memst.ullAvailVirtual ;
#else
	typedef	BOOL (WINAPI *API_GlobalMemoryStatusEx)( LPMEMORYSTATUSEX lpBuffer ) ;
	static HMODULE					hModuleKernel32 = NULL ;
	static API_GlobalMemoryStatusEx	apiGlobalMemoryStatusEx = NULL ;
	if ( hModuleKernel32 == NULL )
	{
		hModuleKernel32 = ::GetModuleHandle( "kernel32.dll" ) ;
		if ( hModuleKernel32 != NULL )
		{
			apiGlobalMemoryStatusEx =
				(API_GlobalMemoryStatusEx)
					::GetProcAddress( hModuleKernel32, "GlobalMemoryStatusEx" ) ;
		}
	}
	if ( apiGlobalMemoryStatusEx != NULL )
	{
		MEMORYSTATUSEX	memst ;
		::eslFillMemory( &memst, 0, sizeof(MEMORYSTATUSEX) ) ;
		memst.dwLength = sizeof(MEMORYSTATUSEX) ;
		if ( apiGlobalMemoryStatusEx( &memst ) )
		{
			mstatus.nTotalPhys = memst.ullTotalPhys ;
			mstatus.nAvailPhys = memst.ullAvailPhys ;
			mstatus.nTotalVirtual = memst.ullTotalVirtual ;
			mstatus.nAvailVirtual = memst.ullAvailVirtual ;
			return ;
		}
	}
	MEMORYSTATUS	memst ;
	::eslFillMemory( &memst, 0, sizeof(MEMORYSTATUS) ) ;
	memst.dwLength = sizeof(MEMORYSTATUS) ;
	::GlobalMemoryStatus( &memst ) ;
	//
	mstatus.nTotalPhys = memst.dwTotalPhys ;
	mstatus.nAvailPhys = memst.dwAvailPhys ;
	mstatus.nTotalVirtual = memst.dwTotalVirtual ;
	mstatus.nAvailVirtual = memst.dwAvailVirtual ;
#endif
}

void SSystem::SetMemoryAllocationMode( MemoryAllocationMode mode )
{
}


//////////////////////////////////////////////////////////////////////////////
// システム（タイマ・時刻）
//////////////////////////////////////////////////////////////////////////////

void SSystem::ResetCurrentMilliSec( uint64_t timeStart )
{
	g_csmutexGlobal->Lock() ;
	{
		g_dwLastTime = ::timeGetTime() ;
		g_nCurrentTime = timeStart ;
	}
	g_csmutexGlobal->Unlock() ;
}

uint64_t SSystem::CurrentMilliSec( void )
{
	uint64_t	timeCurrent ;
	g_csmutexGlobal->Lock() ;
	{
		DWORD	dwCurrent = ::timeGetTime() ;
		g_nCurrentTime += dwCurrent - g_dwLastTime ;
		g_dwLastTime = dwCurrent ;
		timeCurrent = g_nCurrentTime ;
	}
	g_csmutexGlobal->Unlock() ;
	return	timeCurrent ;
}

// パフォーマンスカウンタ
int64_t SSystem::GetPerformanceCounter( void )
{
	LARGE_INTEGER	timeCurrent ;
	if ( ::QueryPerformanceCounter( &timeCurrent ) )
	{
		return	timeCurrent.QuadPart ;
	}
	return	::timeGetTime() ;
}

int64_t SSystem::GetPerformanceFrequency( void )
{
	LARGE_INTEGER	timeFreq ;
	if ( ::QueryPerformanceFrequency( &timeFreq ) )
	{
		return	timeFreq.QuadPart ;
	}
	return	0 ;
}

// 一定時間処理を解放（ミリ秒）
void SSystem::SleepMilliSec( int msec )
{
	::Sleep( msec ) ;
}

// 一定フレーム処理を解放（フレーム駆動スレッド限定）
void SSystem::SleepFrame( int64_t frames )
{
	SThread *	pThread = SThread::GetCurrentThread() ;
	if ( pThread != NULL )
	{
		AtomicAdd( &(pThread->m_countPending), (atomic_int_t) frames ) ;
		pThread->m_signalMaster.SetSignal() ;
		//
		while ( pThread->m_countPending > 0 )
		{
			if ( pThread->m_signalFrame.Wait( 10 ) == errSuccess )
			{
				pThread->m_signalFrame.ResetSignal() ;
			}
		}
		if ( pThread->m_flagThrow )
		{
			throw	SException( 0, pThread->m_strException ) ;
		}
	}
}

// 現在の日付
void SSystem::CurrentLocalDate( DATE_TIME& date )
{
	SYSTEMTIME	stLocal ;
	::GetLocalTime( &stLocal ) ;
	//
	date.nYear = stLocal.wYear ;
	date.nMonth = stLocal.wMonth ;
	date.nDay = stLocal.wDay ;
	date.nWeek = stLocal.wDayOfWeek ;
	date.nHour = stLocal.wHour ;
	date.nMinute = stLocal.wMinute ;
	date.nSecond = stLocal.wSecond ;
	date.nMilliSec = stLocal.wMilliseconds ;
}

// 標準時との時差（秒単位）
int32_t SSystem::DifferenceInLocalTime
		( wchar_t * pwszName, size_t nNameCapacity )
{
	TIME_ZONE_INFORMATION	tzi ;
	eslFillMemory( &tzi, 0, sizeof(TIME_ZONE_INFORMATION) ) ;
	DWORD	dwResult = GetTimeZoneInformation( &tzi ) ;
	if ( (dwResult == TIME_ZONE_ID_UNKNOWN)
		|| (dwResult == TIME_ZONE_ID_STANDARD) )
	{
		for ( size_t i = 0; i < nNameCapacity; i ++ )
		{
			if ( (pwszName[i] = tzi.StandardName[i]) == 0 )
			{
				break ;
			}
		}
		return	tzi.Bias * 60 ;
	}
	else if ( dwResult == TIME_ZONE_ID_DAYLIGHT )
	{
		for ( size_t i = 0; i < nNameCapacity; i ++ )
		{
			if ( (pwszName[i] = tzi.DaylightName[i]) == 0 )
			{
				break ;
			}
		}
		return	(tzi.Bias + tzi.DaylightBias) * 60 ;
	}
	if ( nNameCapacity >= 1 )
	{
		pwszName[0] = 0 ;
	}
	return	0 ;
}


//////////////////////////////////////////////////////////////////////////////
// プラットフォーム情報
//////////////////////////////////////////////////////////////////////////////

void SSystem::GetPlatformInformation( PLATFORM_INFORMATION& pi )
{
#if	_MSC_VER >= 1800
	if ( IsWindowsXPOrGreater() )
	{
		if ( IsWindowsServer() )
		{
			pi.runtimeOS = platformOS_WindowsNT_Server ;
			//
			if ( IsWindows10OrGreater() )
			{
				pi.versionOS = versionWindowsServer2016 ;
			}
			else if ( IsWindows8Point1OrGreater() )
			{
				pi.versionOS = versionWindowsServer2012R2 ;
			}
			else if ( IsWindows8OrGreater() )
			{
				pi.versionOS = versionWindowsServer2012 ;
			}
			else if ( IsWindows7OrGreater() )
			{
				pi.versionOS = versionWindowsServer2008R2 ;
			}
			else
			{
				pi.versionOS = versionWindowsServer2008 ;
			}
		}
		else
		{
			pi.runtimeOS = platformOS_WindowsNT ;
			//
			if ( IsWindows10OrGreater() )
			{
				pi.versionOS = versionWindows10 ;
			}
			else if ( IsWindows8Point1OrGreater() )
			{
				pi.versionOS = versionWindows8_1 ;
			}
			else if ( IsWindows8OrGreater() )
			{
				pi.versionOS = versionWindows8 ;
			}
			else if ( IsWindows7OrGreater() )
			{
				pi.versionOS = versionWindows7 ;
			}
			else if ( IsWindowsVistaOrGreater() )
			{
				pi.versionOS = versionWindowsVista ;
			}
			else
			{
				pi.versionOS = versionWindowsXP ;
			}
		}
	}
	else
	{
		pi.runtimeOS = platformOS_Windows ;
		pi.versionOS = versionWindowsME ;
	}
#else
	OSVERSIONINFOEX	osvi ;
	bool	fInfoEx = true ;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX) ;
	if ( !::GetVersionEx( (OSVERSIONINFO*) &osvi ) )
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
		::GetVersionEx( (OSVERSIONINFO*) &osvi ) ;
		fInfoEx = false ;
	}
	//
	::eslFillMemory( &pi, 0, sizeof(PLATFORM_INFORMATION) ) ;
	pi.platformFamily = platformFamilyWin32 ;
	if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		if ( (osvi.dwMajorVersion < 6)
			|| !fInfoEx
			|| (osvi.wProductType == VER_NT_WORKSTATION) )
		{
			pi.runtimeOS = platformOS_WindowsNT ;
		}
		else
		{
			pi.runtimeOS = platformOS_WindowsNT_Server ;
		}
	}
	else
	{
		pi.runtimeOS = platformOS_Windows ;
	}
	pi.versionOS = (osvi.dwMajorVersion << 16)
					| (osvi.dwMinorVersion & 0xFFFF) ;
#endif
#if	defined(__PROCESSOR_INTEL_X86_64__)
	pi.runtimeArchitecture = 64 ;
#else
	pi.runtimeArchitecture = 32 ;
#endif
}


//////////////////////////////////////////////////////////////////////////////
// プロセッサ情報
//////////////////////////////////////////////////////////////////////////////

CPU_Family SSystem::GetCPUFamily( void )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	return	cpuFamily_X86_64 ;
#else
	return	cpuFamily_X86 ;
#endif
}

uint64_t SSystem::GetCPUFeatures( void )
{
#if	_MSC_VER >= 1800
	uint64_t	maskFeatures = 0 ;
	if ( IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) )
	{
		maskFeatures |= cpuX86_Feature_MMX ;
	}
	if ( IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) )
	{
		maskFeatures |= cpuX86_Feature_SSE ;
	}
	if ( IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) )
	{
		maskFeatures |= cpuX86_Feature_SSE2 ;
	}
	if ( IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) )
	{
		maskFeatures |= cpuX86_Feature_SSE3 ;
	}
	return	maskFeatures ;
#else
	try
	{
		DWORD	maskFeatures = 0 ;
		#if	defined(__PROCESSOR_INTEL_X86_64__)
			int	reg[4] ;
			__cpuid( reg, 0 ) ;
			__cpuid( reg, 1 ) ;
		#else
			DWORD	reg[4] ;
			__asm
			{
				xor	eax, eax
				cpuid
				mov	eax, 1
				cpuid
				mov	reg[0], eax
				mov	reg[4], ebx
				mov	reg[8], ecx
				mov	reg[12], edx
			}
		#endif
		if ( reg[3] & (1 << 23) )
		{
			maskFeatures |= cpuX86_Feature_MMX ;
		}
		if ( reg[3] & (1 << 25) )
		{
			maskFeatures |= cpuX86_Feature_SSE ;
			if ( reg[3] & (1 << 26) )
			{
				maskFeatures |= cpuX86_Feature_SSE2 ;
				if ( reg[2] & 1 )
				{
					maskFeatures |= cpuX86_Feature_SSE3 ;
				}
			}
		}
		return	maskFeatures ;
	}
	catch ( ... )
	{
	}
	return	0 ;
#endif
}

unsigned int SSystem::GetLogicalProcessorCount( void )
{
	unsigned int	nProcessorCount = 0 ;
	DWORD_PTR		dwProcessMask, dwSystemMask ;
	if ( ::GetProcessAffinityMask
			( ::GetCurrentProcess(), &dwProcessMask, &dwSystemMask ) )
	{
		while ( dwSystemMask != 0 )
		{
			if ( dwSystemMask & 0x01 )
			{
				nProcessorCount ++ ;
			}
			dwSystemMask >>= 1 ;
		}
	}
	return	nProcessorCount ;
}


//////////////////////////////////////////////////////////////////////////////
// システム（モジュール・エクスポート関数）
//////////////////////////////////////////////////////////////////////////////

ulong_ptr_t SSystem::GetModuleExportFunction
	( const wchar_t * pszFuncName, const wchar_t * pszReserved )
{
	SString			strFuncName = pszFuncName ;
	SArray<char>	bufFuncName ;
	return	(ulong_ptr_t) ::GetProcAddress
			( ::GetModuleHandle( NULL ),
					strFuncName.EncodeDefaultTo( bufFuncName ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// デバッグ用関数
//////////////////////////////////////////////////////////////////////////////

void SSystem::Trace( const char * pszTrace, ... )
{
	char	szBuf[0x1000] ;
	va_list	vl ;
	va_start( vl, pszTrace ) ;

#if	_MSC_VER >= 1400
	int	nSize = ::_vsnprintf_s( szBuf, 0x1000, 0xFFF, pszTrace, vl ) ;
#else
	int	nSize = ::_vsnprintf( szBuf, 0xFFF, pszTrace, vl ) ;
#endif

	::OutputDebugString( szBuf ) ;
}

void SSystem::Assert
	( const char * pszExpr, const char * pszFile, int nLineNum )
{
#if	defined(__DEBUG__)
	static bool	s_flagIgnoreAssertion = false ;

	if ( !s_flagIgnoreAssertion )
	{
		char	szBuf[0x1000] ;
		int		iFileName = 0 ;
		for ( int i = 0; pszFile[i] != 0; i ++ )
		{
			if ( (pszFile[i] == '\\') || (pszFile[i] == '/') )
			{
				iFileName = i + 1 ;
			}
		}
		#if	_MSC_VER >= 1400
			::sprintf_s
				( szBuf, 0x1000, "%s\n\n%s (%d line)\n\nCancel to ignore assertion.",
					pszExpr, pszFile + iFileName, nLineNum ) ;
		#else
			::sprintf
				( szBuf, "%s\n\n%s (%d line)\n\nCancel to ignore assertion.",
					pszExpr, pszFile + iFileName, nLineNum ) ;
		#endif

		Trace( "assert:%s(%d): %s\n",
					pszFile + iFileName, nLineNum, pszExpr ) ;

		int	nMBResult =
			::MessageBox
				( NULL, szBuf, "SSystem Assertion!!", MB_OKCANCEL | MB_ICONSTOP ) ;

		if ( nMBResult == IDOK )
		{
		#if	defined(__PROCESSOR_INTEL_X86_64__)
			__int2c() ;
		#else
			__asm	int	3 ;
		#endif
		}
		else
		{
			s_flagIgnoreAssertion = true ;
		}
	}
#endif
}

int SSystem::MessageBox
	( const wchar_t * pwszMsg, const wchar_t * pwszCaption,
				int nStyles, SakuraGL::Window * pParentWnd )
{
	HWND	hwndParent = NULL ;
	if ( pParentWnd == NULL )
	{
		QuickLock() ;
		SGLAbstractWindow *	pDefWindow = SGLAbstractWindow::GetDefaultWindow() ;
		if ( pDefWindow != NULL )
		{
			hwndParent = pDefWindow->GetWindowHandle() ;
		}
		QuickUnlock() ;
	}
	else
	{
		hwndParent = pParentWnd->GetWindowHandle() ;
	}
	return	DoMessageBox( pwszMsg, pwszCaption, nStyles, hwndParent ) ;
}

int SSystem::DoMessageBox
	( const wchar_t * pwszMsg,
		const wchar_t * pwszCaption, int nStyles, HWND hwndParent )
{
	UINT	uType = 0 ;
	switch ( nStyles )
	{
	case	msgboxStyleOk:
		uType = MB_OK | MB_ICONINFORMATION ;
		break ;
	case	msgboxStyleOkCancel:
		uType = MB_OKCANCEL | MB_ICONQUESTION ;
		break ;
	case	msgboxStyleRetryCancel:
		uType = MB_RETRYCANCEL | MB_ICONQUESTION ;
		break ;
	case	msgboxStyleAbortRetryIgnore:
		uType = MB_ABORTRETRYIGNORE | MB_ICONQUESTION ;
		break ;
	case	msgboxStyleYesNo:
		uType = MB_YESNO | MB_ICONQUESTION ;
		break ;
	case	msgboxStyleYesNoCancel:
		uType = MB_YESNOCANCEL | MB_ICONQUESTION ;
		break ;
	}
	SString	strMsg = pwszMsg ;
	SString	strCaption = pwszCaption ;
	int	nMBResult =
		::MessageBox
			( hwndParent, strMsg.ToCharArray(),
					strCaption.ToCharArray(), uType ) ;
	switch ( nMBResult )
	{
	case	IDOK:
		nMBResult = msgboxResultOk ;
		break ;
	case	IDCANCEL:
		nMBResult = msgboxResultCancel ;
		break ;
	case	IDABORT:
		nMBResult = msgboxResultAbort ;
		break ;
	case	IDRETRY:
		nMBResult = msgboxResultRetry ;
		break ;
	case	IDIGNORE:
		nMBResult = msgboxResultIgnore ;
		break ;
	case	IDYES:
		nMBResult = msgboxResultYes ;
		break ;
	case	IDNO:
		nMBResult = msgboxResultNo ;
		break ;
	}
	return	nMBResult ;
}


//////////////////////////////////////////////////////////////////////////////
// メモリアトミック操作
//////////////////////////////////////////////////////////////////////////////

atomic_int_t SSystem::AtomicXchg( volatile atomic_int_t* p, atomic_int_t v )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	return	_InterlockedExchange( p, v ) ;
#else
	__asm
	{
		mov	edx, p
		mov	eax, v
		lock xchg [edx], eax
		mov	v, eax
	}
	return	v ;
#endif
}

atomic_int_t SSystem::AtomicAdd( volatile atomic_int_t* p, atomic_int_t v )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	#if	_MSC_VER >= 1910
		return	InterlockedAdd( p, v ) ;
	#else
		return	_InterlockedAdd( p, v ) ;
	#endif
#else
	__asm
	{
		mov	edx, p
		mov	eax, v
		lock xadd [edx], eax
		add	v, eax
	}
	return	v ;
#endif
}

atomic_int_t SSystem::AtomicSub( volatile atomic_int_t* p, atomic_int_t v )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	#if	_MSC_VER >= 1910
		return	InterlockedAdd( p, -v ) ;
	#else
		return	_InterlockedAdd( p, -v ) ;
	#endif
#else
	__asm
	{
		mov	edx, p
		mov	eax, v
		neg	eax
		lock xadd [edx], eax
		sub	eax, v
		mov	v, eax
	}
	return	v ;
#endif
}

atomic_int_t SSystem::AtomicAnd( volatile atomic_int_t* p, atomic_int_t v )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	return	_InterlockedAnd( p, v ) & v ;
#else
	__asm
	{
		mov	edx, p
		mov	eax, v
		lock and [edx], eax
	}
	return	*p ;
#endif
}

atomic_int_t SSystem::AtomicOr( volatile atomic_int_t* p, atomic_int_t v )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	return	_InterlockedOr( p, v ) | v ;
#else
	__asm
	{
		mov	edx, p
		mov	eax, v
		lock or [edx], eax
	}
	return	*p ;
#endif
}

atomic_int_t SSystem::AtomicXor( volatile atomic_int_t* p, atomic_int_t v )
{
#if	defined(__PROCESSOR_INTEL_X86_64__)
	return	_InterlockedXor( p, v ) ^ v ;
#else
	__asm
	{
		mov	edx, p
		mov	eax, v
		lock xor [edx], eax
	}
	return	*p ;
#endif
}

