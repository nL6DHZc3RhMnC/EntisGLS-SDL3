
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakura/sakura.h>
#include <sakura/ssys_heap_memory.h>
#include <sakura/ssys_fragment_file.h>
#include <sakura/ssys_socket.h>
#include <sakura/ssys_http_file.h>
#include <esl/esl_java_object.h>
#include <sakura/ssys_android_file.h>

#include <malloc.h>
#include <time.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <cpu-features.h>
#include <android/log.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 定数
//////////////////////////////////////////////////////////////////////////////

const double	SSystem::PI = 3.1415926535897932384626433832795 ;



//////////////////////////////////////////////////////////////////////////////
// ライブラリ初期化
//////////////////////////////////////////////////////////////////////////////

PLATFORM_INFORMATION	SSystem::g_infoPlatform ;
SSystem::CPU_Family		SSystem::g_cpuFamily ;
uint64_t				SSystem::g_cpuFeatures ;
unsigned int			SSystem::g_cpuLogicalCount ;
SSharableMutex *		SSystem::g_mutexGlobal = NULL ;
SCriticalSection *		SSystem::g_csmutexGlobal = NULL ;
pthread_key_t			SSystem::g_keyThread = -1 ;
pthread_key_t			SSystem::g_keyStorage = -1 ;

static volatile atomic_int_t	g_atomicMutex = SSystem::SSynchronism::futexWaked ;
static uint64_t					g_nCurrentTime = 0 ;
static uint64_t					g_nLastTime = 0 ;

static atomic_int_t				g_countRefSystem = 0 ;


void SSystem::Initialize( void )
{
	if ( AtomicAdd( &g_countRefSystem, 1 ) == 1 )
	{
		Trace( "SSystem::Initialize\n" ) ;
		//
		SSystem::eslHeapInitialize() ;
		//
		DATE_TIME	dtCurrent ;
		wchar_t		wszTimeZone[32] ;
		int32_t		nDiffTime =
				SSystem::DifferenceInLocalTime( &wszTimeZone[0], 32 ) / -60 ;
		CurrentLocalDate( dtCurrent ) ;
		Trace( "current time: %04d/%02d/%02d %02d:%02d:%02d %s %+03d:%02d\n",
				dtCurrent.nYear, dtCurrent.nMonth, dtCurrent.nDay,
				dtCurrent.nHour, dtCurrent.nMinute, dtCurrent.nSecond,
				SString(&wszTimeZone[0]).ToCharArray().GetArray(),
				(nDiffTime / 60), (nDiffTime % 60) ) ;
		//
		GetPlatformInformation( g_infoPlatform ) ;
		g_cpuFamily = GetCPUFamily() ;
		g_cpuFeatures = GetCPUFeatures() ;
		g_cpuLogicalCount = GetLogicalProcessorCount() ;
		Trace( "CPU family: %d\n", (int) g_cpuFamily ) ;
		Trace( "CPU features: 0x%08x\n", (int) g_cpuFeatures ) ;
		Trace( "logical processors: %d\n", (int) g_cpuLogicalCount ) ;
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
		pthread_key_create( &g_keyThread, NULL ) ;
		pthread_key_create( &g_keyStorage, NULL ) ;
		//
		ResetCurrentMilliSec( 0 ) ;
//		SThread::TestSwitchingPerformance() ;
		//
		SFileOpener::SetDefaultOpener( &g_defURLOpener ) ;
		g_defURLOpener.RegisterScheme
				( L"file://", new SStandardFileOpener ) ;
		g_defURLOpener.RegisterScheme
				( L"http://", new SOffsetFileOpener
					( L"http://",  L'/', new SHttpFileOpener, true ),
					SVirtualURLOpener::schemeOverNetwork ) ;
		g_defURLOpener.RegisterScheme
				( L"https://", new SOffsetFileOpener
					( L"https://",  L'/', new SHttpFileOpener, true ),
					SVirtualURLOpener::schemeOverNetwork ) ;
		g_defURLOpener.RegisterScheme
				( L"data://", new SOffsetFileOpener
					( L"/data/",  L'/', new SStandardFileOpener, true ) ) ;
		//
		SFileOpener *	pAssetsOpener = new SAssetFileOpener ;
		g_defURLOpener.RegisterScheme
				( L"assets://", pAssetsOpener ) ;
		g_defURLOpener.RegisterScheme
			( L"fragments://",
				new SFragmentFileOpener( L"", L'/', pAssetsOpener, false ) ) ;
		//
		SString	strStorageDir ;
		JNI::GetAndroidStorageDirectory( strStorageDir ) ;
		g_defURLOpener.RegisterScheme
				( L"sd://", new SOffsetFileOpener
					( strStorageDir,  L'/', new SStandardFileOpener, true ) ) ;
		//
		SString	strPrivateDir ;
		JNI::GetAndroidStoragePrivateDirectory( strPrivateDir ) ;
		g_defURLOpener.RegisterScheme
				( L"storage://", new SOffsetFileOpener
					( strPrivateDir,  L'/', new SStandardFileOpener, true ) ) ;
	}
	if ( g_defURLOpener.FindScheme( L"local://" ) < 0 )
	{
		SString	strLocalDir ;
		JNI::GetAndroidLocalFilesDirectory( strLocalDir ) ;
		if ( !strLocalDir.IsEmpty() )
		{
			g_defURLOpener.RegisterScheme
				( L"local://", new SOffsetFileOpener
					( strLocalDir,  L'/', new SStandardFileOpener, true ) ) ;
		}
	}
}

void SSystem::Finalize( void )
{
	if ( AtomicSub( &g_countRefSystem, 1 ) == 0 )
	{
		Trace( "SSystem::Finalize\n" ) ;
		g_dnsLibServ.NotifyAllOnFinalize() ;
		//
		SThread::ExitAllStockedThread() ;
		SSystem::RemoveAllMemoryTightnessNotification() ;
		//
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
		pthread_key_delete( g_keyThread ) ;
		pthread_key_delete( g_keyStorage ) ;
		g_keyThread = -1 ;
		g_keyStorage = -1 ;
		//
		SSystem::eslHeapUninitialize() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// システム（メモリ）
//////////////////////////////////////////////////////////////////////////////

ESL_FUNCPTR_MALLOC	esl_stub_malloc = ::malloc ;
ESL_FUNCPTR_REALLOC	esl_stub_relloc = ::realloc ;
ESL_FUNCPTR_FREE	esl_stub_free = ::free ;

void SSystem::GetMemoryStatus( MEMORY_STATUS& mstatus )
{
	eslFillMemory( &mstatus, 0, sizeof(MEMORY_STATUS) ) ;
	/*
	struct sysinfo	inf ;
	if ( sysinfo( &inf ) == 0 )
	{
		mstatus.nTotalPhys = (int64_t) inf.totalram * inf.mem_unit ;
		mstatus.nAvailPhys = (int64_t) inf.freeram * inf.mem_unit ;
		mstatus.nTotalVirtual = (int64_t) inf.totalswap * inf.mem_unit ;
		mstatus.nAvailVirtual = (int64_t) inf.freeswap * inf.mem_unit ;
	}
	*/
	struct sysinfo	inf ;
	if ( syscall( __NR_sysinfo, &inf ) == 0 )
	{
		mstatus.nTotalPhys = (int64_t) inf.totalram * inf.mem_unit ;
		mstatus.nAvailPhys = (int64_t) inf.freeram * inf.mem_unit ;
		mstatus.nTotalVirtual = (int64_t) inf.totalswap * inf.mem_unit ;
		mstatus.nAvailVirtual = (int64_t) inf.freeswap * inf.mem_unit ;
	}
}

void SSystem::SetMemoryAllocationMode( MemoryAllocationMode mode )
{
}



//////////////////////////////////////////////////////////////////////////////
// システム（タイマ・時刻）
//////////////////////////////////////////////////////////////////////////////

void SSystem::ResetCurrentMilliSec( uint64_t timeStart )
{
	timeval	tv ;
	if ( gettimeofday( &tv, NULL ) == 0 )
	{
		g_nLastTime =
			((uint64_t) tv.tv_sec * 1000)
						+ (tv.tv_usec / 1000) - timeStart ;
	}
}

uint64_t SSystem::CurrentMilliSec( void )
{
	timeval	tv ;
	if ( gettimeofday( &tv, NULL ) == 0 )
	{
		return	((uint64_t) tv.tv_sec * 1000)
						+ (tv.tv_usec / 1000) - g_nLastTime ;
	}
	return	0 ;
}

// パフォーマンスカウンタ
int64_t SSystem::GetPerformanceCounter( void )
{
	timeval	tv ;
	if ( gettimeofday( &tv, NULL ) == 0 )
	{
		return	((int64_t) tv.tv_sec * 1000000) + tv.tv_usec ;
	}
	return	0 ;
}

int64_t SSystem::GetPerformanceFrequency( void )
{
	return	1000000 ;
}

// 一定時間処理を解放（ミリ秒）
void SSystem::SleepMilliSec( int msec )
{
	const time_t	nMilliSec = (msec % 1000) ;
	const long		nSec = (msec - nMilliSec) / 1000 ;
	timespec	req ;
	req.tv_sec = nSec ;
	req.tv_nsec = nMilliSec * 1000000 ;
	nanosleep( &req, NULL ) ;
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
	timeval	tv ;
	if ( gettimeofday( &tv, NULL ) == 0 )
	{
		tm		tmLocal ;
		tm *	ptmLocal = localtime_r( &(tv.tv_sec), &tmLocal ) ;
		//
		date.nYear = (uint16_t) (ptmLocal->tm_year + 1900) ;
		date.nMonth = (uint16_t) (ptmLocal->tm_mon + 1) ;
		date.nDay = (uint16_t) (ptmLocal->tm_mday) ;
		date.nWeek = (uint16_t) (ptmLocal->tm_wday) ;
		date.nHour = (uint16_t) (ptmLocal->tm_hour) ;
		date.nMinute = (uint16_t) (ptmLocal->tm_min) ;
		date.nSecond = (uint16_t) (ptmLocal->tm_sec) ;
		date.nMilliSec = (uint16_t) (tv.tv_usec / 1000) ;
	}
	else
	{
		eslFillMemory( &date, 0, sizeof(DATE_TIME) ) ;
	}
}

// 標準時との時差（秒単位）
int32_t SSystem::DifferenceInLocalTime
		( wchar_t * pwszName, size_t nNameCapacity )
{
	tzset() ;
	//
	for ( size_t i = 0; i < nNameCapacity; i ++ )
	{
		if ( (pwszName[i] = tzname[0][i]) == 0 )
		{
			break ;
		}
	}
	return	timezone ;
}


//////////////////////////////////////////////////////////////////////////////
// プラットフォーム情報
//////////////////////////////////////////////////////////////////////////////

void SSystem::GetPlatformInformation( PLATFORM_INFORMATION& pi )
{
	::eslFillMemory( &pi, 0, sizeof(PLATFORM_INFORMATION) ) ;
	pi.platformFamily = platformFamilyPosix ;
	pi.runtimeOS = platformOS_LinuxAndroid ;
	pi.runtimeArchitecture = 32 ;
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidGetAPILevel =
		jsclsEntisGLS.GetStaticMethodID( "getAPILevel", "()I" ) ;
	pi.versionOS = jsclsEntisGLS.CallStaticIntMethod( jmidGetAPILevel ) ;
}



//////////////////////////////////////////////////////////////////////////////
// プロセッサ情報
//////////////////////////////////////////////////////////////////////////////

CPU_Family SSystem::GetCPUFamily( void )
{
	AndroidCpuFamily	nCPUFamily = android_getCpuFamily() ;
	switch ( nCPUFamily )
	{
	case	ANDROID_CPU_FAMILY_ARM:
		return	cpuFamily_ARM ;

	case	ANDROID_CPU_FAMILY_X86:
		return	cpuFamily_X86 ;

	case	ANDROID_CPU_FAMILY_MIPS:
		return	cpuFamily_MIPS ;

	case	ANDROID_CPU_FAMILY_ARM64:
		return	cpuFamily_ARM64 ;

	case	ANDROID_CPU_FAMILY_X86_64:
		return	cpuFamily_X86_64 ;

	case	ANDROID_CPU_FAMILY_MIPS64:
		return	cpuFamily_MIPS64 ;

	default:
		break ;
	}
	return	cpuFamily_Unknown ;
}

uint64_t SSystem::GetCPUFeatures( void )
{
	AndroidCpuFamily	nCPUFamily = android_getCpuFamily() ;
	uint64_t			androidCPUFeatures = android_getCpuFeatures() ;
	uint64_t			sakuraCPUFeatures = 0 ;
	switch ( nCPUFamily )
	{
	case	ANDROID_CPU_FAMILY_ARM:
		if ( androidCPUFeatures & ANDROID_CPU_ARM_FEATURE_ARMv7 )
		{
			sakuraCPUFeatures |= cpuARM_Feature_ARMv7 ;
		}
		if ( androidCPUFeatures & ANDROID_CPU_ARM_FEATURE_VFPv3 )
		{
			sakuraCPUFeatures |= cpuARM_Feature_VFPv3 ;
		}
		if ( androidCPUFeatures & ANDROID_CPU_ARM_FEATURE_NEON )
		{
			sakuraCPUFeatures |= cpuARM_Feature_NEON ;
		}
		break ;

	case	ANDROID_CPU_FAMILY_ARM64:
		sakuraCPUFeatures |=
			cpuARM_Feature_ARMv7
				| cpuARM_Feature_VFPv3 | cpuARM_Feature_NEON ;
		break ;

	case	ANDROID_CPU_FAMILY_X86:
		sakuraCPUFeatures |=
			cpuX86_Feature_MMX | cpuX86_Feature_SSE
				| cpuX86_Feature_SSE2 | cpuX86_Feature_SSE3 ;
		if ( androidCPUFeatures & ANDROID_CPU_X86_FEATURE_SSSE3 )
		{
		}
		break ;

	case	ANDROID_CPU_FAMILY_X86_64:
		sakuraCPUFeatures |=
			cpuX86_Feature_MMX | cpuX86_Feature_SSE
				| cpuX86_Feature_SSE2 | cpuX86_Feature_SSE3 ;
		break ;

	case	ANDROID_CPU_FAMILY_MIPS:
		break ;

	default:
		break ;
	}
	return	sakuraCPUFeatures ;
}

unsigned int SSystem::GetLogicalProcessorCount( void )
{
	return	android_getCpuCount() ;
}


//////////////////////////////////////////////////////////////////////////////
// システム（モジュール・エクスポート関数）
//////////////////////////////////////////////////////////////////////////////

ulong_ptr_t SSystem::GetModuleExportFunction
	( const wchar_t * pszFuncName, const wchar_t * pszReserved )
{
	return	(ulong_ptr_t) NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// デバッグ用関数
//////////////////////////////////////////////////////////////////////////////

void SSystem::Trace( const char * pszTrace, ... )
{
	char	szBuf[0x1000] ;
	va_list	vl ;
	va_start( vl, pszTrace ) ;
	vsnprintf( szBuf, 0x1000, pszTrace, vl ) ;
	//
	__android_log_print
		( ANDROID_LOG_DEBUG, "EntisGLS", "%s", szBuf ) ;
}

void SSystem::Assert
	( const char * pszExpr, const char * pszFile, int nLineNum )
{
#if	defined(__DEBUG__)
	char	szBuf[0x1000] ;
	int		iFileName = 0 ;
	for ( int i = 0; pszFile[i] != 0; i ++ )
	{
		if ( (pszFile[i] == '\\') || (pszFile[i] == '/') )
		{
			iFileName = i + 1 ;
		}
	}
	__android_log_print
		( ANDROID_LOG_ERROR,
			"EntisGLS", "assert %s(%d): %s",
			(pszFile + iFileName), nLineNum, pszExpr ) ;
#endif
}

int SSystem::MessageBox
	( const wchar_t * pwszMsg, const wchar_t * pwszCaption,
			int nStyles, SakuraGL::SGLAbstractWindow * pPaintWnd )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidDoMessageBox =
		jsclsEntisGLS.GetStaticMethodID
			( "doMessageBox",
				"(L" JAVA_LANG_STRING ";L" JAVA_LANG_STRING ";I)I" ) ;
	JNI::JavaObject	jobjMsg ;
	JNI::JavaObject	jobjCaption ;
	return	jsclsEntisGLS.CallStaticIntMethod
		( jmidDoMessageBox,
			jobjCaption.CreateWideString(pwszCaption),
			jobjMsg.CreateWideString(pwszMsg), nStyles ) ;
}


//////////////////////////////////////////////////////////////////////////////
// メモリアトミック操作
//////////////////////////////////////////////////////////////////////////////

atomic_int_t SSystem::AtomicXchg( volatile atomic_int_t* p, atomic_int_t v )
{
	atomic_int_t	vTemp ;
	SSynchronism::LockSimpleMutex( &g_atomicMutex ) ;
	{
		vTemp = *p ;
		*p = v ;
		v = vTemp ;
	}
	SSynchronism::UnlockSimpleMutex( &g_atomicMutex ) ;
	return	v ;
}

atomic_int_t SSystem::AtomicAdd( volatile atomic_int_t* p, atomic_int_t v )
{
	SSynchronism::LockSimpleMutex( &g_atomicMutex ) ;
	{
		*p += v ;
		v = *p ;
	}
	SSynchronism::UnlockSimpleMutex( &g_atomicMutex ) ;
	return	v ;
}

atomic_int_t SSystem::AtomicSub( volatile atomic_int_t* p, atomic_int_t v )
{
	SSynchronism::LockSimpleMutex( &g_atomicMutex ) ;
	{
		*p -= v ;
		v = *p ;
	}
	SSynchronism::UnlockSimpleMutex( &g_atomicMutex ) ;
	return	v ;
}

atomic_int_t SSystem::AtomicAnd( volatile atomic_int_t* p, atomic_int_t v )
{
	SSynchronism::LockSimpleMutex( &g_atomicMutex ) ;
	{
		*p &= v ;
		v = *p ;
	}
	SSynchronism::UnlockSimpleMutex( &g_atomicMutex ) ;
	return	v ;
}

atomic_int_t SSystem::AtomicOr( volatile atomic_int_t* p, atomic_int_t v )
{
	SSynchronism::LockSimpleMutex( &g_atomicMutex ) ;
	{
		*p |= v ;
		v = *p ;
	}
	SSynchronism::UnlockSimpleMutex( &g_atomicMutex ) ;
	return	v ;
}

atomic_int_t SSystem::AtomicXor( volatile atomic_int_t* p, atomic_int_t v )
{
	SSynchronism::LockSimpleMutex( &g_atomicMutex ) ;
	{
		*p ^= v ;
		v = *p ;
	}
	SSynchronism::UnlockSimpleMutex( &g_atomicMutex ) ;
	return	v ;
}


