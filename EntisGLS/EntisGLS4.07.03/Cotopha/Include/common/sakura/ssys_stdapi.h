
#if	!defined(__SAKURA2_STDAPI_H__)
#define	__SAKURA2_STDAPI_H__

//////////////////////////////////////////////////////////////////////////////
// 標準メモリアロケーションのスタブ
//////////////////////////////////////////////////////////////////////////////

namespace	SakuraGL
{
	class	SGLAbstractWindow ;

	#if	defined(__COTOPHA__)
	class	__native Window ;
	#else
	typedef	SGLAbstractWindow	Window ;
	#endif
}

#if	defined(__COTOPHA__)

#define	esl_malloc		malloc
#define	esl_realloc		realloc
#define	esl_free		free

#else

typedef	void * (*ESL_FUNCPTR_MALLOC)( size_t size ) ;
typedef	void * (*ESL_FUNCPTR_REALLOC)( void * memblock, size_t size ) ;
typedef void (*ESL_FUNCPTR_FREE)( void * memblock ) ;

extern ESL_DLL_EXPORT	ESL_FUNCPTR_MALLOC	esl_stub_malloc ;
extern ESL_DLL_EXPORT	ESL_FUNCPTR_REALLOC	esl_stub_relloc ;
extern ESL_DLL_EXPORT	ESL_FUNCPTR_FREE	esl_stub_free ;

#endif


namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// エラーコード
	//////////////////////////////////////////////////////////////////////////

	enum	SError
	{
		errSuccess			= eslErrSuccess,
		errNotSupported		= eslErrNotSupported,
		errFailed			= eslErrFailed,
		errAbort			= eslErrAbort,
		errInvalidParam		= eslErrInvalidParam,
		errTimeout			= eslErrTimeout,
		errPending			= eslErrPending,
		errContinue			= eslErrContinue,
		errDummy			= eslErrDummy,
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 定数
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
		constant	PI = 3.1415926535897932384626433832795 ;
	#else
		extern	const double	PI ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// ライブラリ初期化
	//////////////////////////////////////////////////////////////////////////

	void Initialize( void ) ;
	void Finalize( void ) ;

	enum	EntisGLS4_Version
	{
		entisglsVersionMajor	= 4,
		entisglsVersionMinor 	= 7,
	} ;
	extern ESL_DLL_EXPORT const uint32_t	entisgls4_version ;
	extern ESL_DLL_EXPORT const char *		entisgls4_version_name ;


	//////////////////////////////////////////////////////////////////////////
	// システム（メモリ）
	//////////////////////////////////////////////////////////////////////////

	// メモリ情報
	struct	MEMORY_STATUS
	{
		int64_t	nTotalPhys ;
		int64_t	nAvailPhys ;
		int64_t	nTotalVirtual ;
		int64_t	nAvailVirtual ;
	} ;
	__native void GetMemoryStatus( MEMORY_STATUS& mstatus ) ;

	// メモリ・アロケーション・モード
	enum	MemoryAllocationMode
	{
		mallocModeAuto,
		mallocModeGlobal,
		mallocModeShared,
		mallocModeThread,
	} ;
	__native void SetMemoryAllocationMode( MemoryAllocationMode mode ) ;


	//////////////////////////////////////////////////////////////////////////
	// システム（タイマ・時刻）
	//////////////////////////////////////////////////////////////////////////

	enum	DayOfWeek
	{
		weekSunday,
		weekMonday,
		weekTuesday,
		weekWednesday,
		weekThursday,
		weekFriday,
		weekSaturday,
	} ;
	struct	DATE_TIME
	{
		int16_t		nYear ;			// 1～
		uint16_t	nMonth ;		// 1～12
		uint16_t	nDay ;			// 1～31
		uint16_t	nWeek ;			// enum DayOfWeek
		uint16_t	nHour ;			// 0～23
		uint16_t	nMinute ;		// 0～59
		uint16_t	nSecond ;		// 0～59
		uint16_t	nMilliSec ;		// 0～999

		// 比較
		int Compare( const DATE_TIME& date ) const ;
		bool operator == ( const DATE_TIME& date ) const
			{ return (Compare( date ) == 0) ; }
		bool operator != ( const DATE_TIME& date ) const
			{ return (Compare( date ) != 0) ; }
		bool operator > ( const DATE_TIME& date ) const
			{ return (Compare( date ) > 0) ; }
		bool operator >= ( const DATE_TIME& date ) const
			{ return (Compare( date ) >= 0) ; }
		bool operator < ( const DATE_TIME& date ) const
			{ return (Compare( date ) < 0) ; }
		bool operator <= ( const DATE_TIME& date ) const
			{ return (Compare( date ) <= 0) ; }

		// 累積日数変換（紀元元年～）
		static uint64_t GetAccumulatedDayCount
					( int nYear, int nMonth, int nDay ) ;
		uint64_t GetAccumulatedDayCount( void ) const
			{	return	GetAccumulatedDayCount( nYear, nMonth, nDay ) ;	}
		void SetAccumulatedDayCount( uint64_t countDay ) ;

		// 累積ミリ秒数（紀元元年～）
		uint64_t GetAccumulatedMilliSec( void ) const ;
		void SetAccumulatedMilliSec( uint64_t nAccMilliSec ) ;

		// カレンダー（グレゴリオ暦）
		// 閏年判定
		static bool IsLeapYear( int nYear )
		{
			return	((nYear % 4) == 0)
						&& (((nYear % 100) != 0)
							|| ((nYear % 400) == 0)) ;
		}
		bool IsLeapYear( void ) const
			{	return	IsLeapYear( nYear ) ;	}
		// 月の日数取得
		static unsigned int GetDayOfMonth( int nYear, int nMonth ) ;
		unsigned int GetDayOfMonth( void ) const
			{	return	GetDayOfMonth( nYear, nMonth ) ;	}
		// 曜日計算
		static DayOfWeek ComputeDayOfWeek( int nYear, int nMonth, int nDay ) ;
		DayOfWeek ComputeDayOfWeek( void ) const
			{	return	ComputeDayOfWeek( nYear, nMonth, nDay ) ;	}
	} ;

	#if	!defined(__COTOPHA__)
		void ResetCurrentMilliSec( uint64_t timeStart ) ;
	#endif

	// 起動からの時間（ミリ秒）
	__native uint64_t CurrentMilliSec( void ) ;
	// パフォーマンスカウンタ
	__native int64_t GetPerformanceCounter( void ) ;
	__native int64_t GetPerformanceFrequency( void ) ;
	// 一定時間処理を解放（ミリ秒）
	__native void SleepMilliSec( int msec ) ;
	// 一定フレーム処理を解放（フレーム駆動スレッド限定）
	__native void SleepFrame( int64_t frames ) ;
	// 現在の日付
	__native void CurrentLocalDate( DATE_TIME& date ) ;
	// 標準時との時差（秒単位）
	__native int32_t DifferenceInLocalTime
		( wchar_t * pwszName = NULL, size_t nNameCapacity = 0 ) ;

	// パフォーマンスカウンタ
	class	STimeCounter
	{
	protected:
		int64_t			m_timeStart ;
		int64_t			m_timeFreq ;
		double			m_timeFreeze ;
		atomic_int_t	m_nVirtStart ;
		bool			m_flagPerformance ;
		bool			m_flagFreeze ;

		static bool			m_flagVirtual ;
		static atomic_int_t	m_timeVirtual ;

	public:
		// 構築関数
		STimeCounter( void ) ;
		// 現在の経過時間 [ms]
		int64_t GetTime( void ) const ;
		// 現在の経過時間（実数）[ms]
		double GetRealTime( void ) const ;
		// 基準時間リセット
		void Reset( int64_t timeInit = 0 ) ;
		// 一時停止
		void Freeze( void ) ;
		// 再開
		void Restart( void ) ;

	public:
		// 仮想タイマーモード設定
		static void SetVirtualTimerMode( bool flagVirtual ) ;
		// 仮想タイマーを進める
		static void AdvanceVirtualTimer( atomic_int_t msecAdv ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// プラットフォーム情報
	//////////////////////////////////////////////////////////////////////////

	enum	PlatformAPIFamily
	{
		platformFamilyCotopha	= 0x00000000,
		platformFamilyWin32		= 0x01000000,
		platformFamilyPosix		= 0x02000000,
		platformFamilyMask		= 0xFF000000,
	} ;
	enum	PlatformOS
	{
		platformOS_Cotopha			= 0x00000000,
		platformOS_Windows			= 0x01000001,
		platformOS_WindowsNT		= 0x01000002,
		platformOS_WindowsNT_Server	= 0x01000003,
		platformOS_LinuxAndroid		= 0x02000010,
	} ;
	enum	WindowsVersion
	{
		versionWindows95			= 0x00040000,
		versionWindows98			= 0x00040001,
		versionWindowsME			= 0x00040009,
	} ;
	enum	WindowsNTVersion
	{
		versionWindowsNT4			= 0x00040000,
		versionWindows2000			= 0x00050000,
		versionWindowsXP			= 0x00050001,
		versionWindowsServer2003	= 0x00050002,
		versionWindowsVista			= 0x00060000,
		versionWindows7				= 0x00060001,
		versionWindows8				= 0x00060002,
		versionWindows8_1			= 0x00060003,
		versionWindows10			= 0x000A0000,
	} ;
	enum	WindowsNTServerVersion
	{
		versionWindowsServer2008	= 0x00060000,
		versionWindowsServer2008R2	= 0x00060001,
		versionWindowsServer2012	= 0x00060002,
		versionWindowsServer2012R2	= 0x00060003,
		versionWindowsServer2016	= 0x000A0000,
	} ;
	enum	JITCompilerFeature
	{
		jitFeature_Compiler			= 0x00000001,
		jitFeature_Saturation		= 0x00000002,
		jitFeature_Float			= 0x00000010,
		jitFeature_SIMD64			= 0x00000020,
		jitFeature_SIMD128			= 0x00000040,
	} ;
	struct	PLATFORM_INFORMATION
	{
		uint32_t	platformFamily ;		// 仮想マシン／ライブラリ
		uint32_t	runtimeOS ;				// 実行 OS
		uint32_t	versionOS ;				// 実行 OS バージョン
		uint32_t	runtimeArchitecture ;	// アーキテクチャ (=32)
		uint32_t	featuresJIT ;			// JIT コンパイラ有効機能
		uint32_t	reserved[3] ;
	} ;

	__native void GetPlatformInformation( PLATFORM_INFORMATION& pi ) ;

	extern ESL_DLL_EXPORT PLATFORM_INFORMATION	g_infoPlatform ;


	//////////////////////////////////////////////////////////////////////////
	// プロセッサ情報
	//////////////////////////////////////////////////////////////////////////

	enum	CPU_Family
	{
		cpuFamily_Unknown	= -1,
		cpuFamily_X86		= 0,
		cpuFamily_ARM		= 1,
		cpuFamily_MIPS		= 2,
		cpuFamily_X86_64	= 3,
		cpuFamily_ARM64		= 4,
		cpuFamily_MIPS64	= 5,
	} ;
	enum	CPU_X86_Features
	{
		cpuX86_Feature_MMX	= 0x0002,
		cpuX86_Feature_SSE	= 0x0008,
		cpuX86_Feature_SSE2	= 0x0010,
		cpuX86_Feature_SSE3	= 0x0020,
	} ;
	enum	CPU_ARM_Features
	{
		cpuARM_Feature_ARMv7	= 0x0001,
		cpuARM_Feature_VFPv3	= 0x0010,
		cpuARM_Feature_NEON		= 0x0100,
	} ;

	__native CPU_Family GetCPUFamily( void ) ;
	__native uint64_t GetCPUFeatures( void ) ;
	__native unsigned int GetLogicalProcessorCount( void ) ;

	extern ESL_DLL_EXPORT CPU_Family	g_cpuFamily ;
	extern ESL_DLL_EXPORT uint64_t		g_cpuFeatures ;
	extern ESL_DLL_EXPORT unsigned int	g_cpuLogicalCount ;


	//////////////////////////////////////////////////////////////////////////
	// システム（モジュール・エクスポート関数）
	//////////////////////////////////////////////////////////////////////////

	__native ulong_ptr_t GetModuleExportFunction
		( const wchar_t * pszFuncName, const wchar_t * pszReserved = NULL ) ;


	//////////////////////////////////////////////////////////////////////////
	// デバッグ用関数
	//////////////////////////////////////////////////////////////////////////

	__native void Trace( const char * pszTrace, ... ) ;
	__native void Assert
		( const char * pszExpr, const char * pszFile, int nLineNum ) ;

	enum	MessageBoxStyle
	{
		msgboxStyleOk,
		msgboxStyleOkCancel,
		msgboxStyleYesNo,
		msgboxStyleYesNoCancel,
		msgboxStyleRetryCancel,
		msgboxStyleAbortRetryIgnore,
	} ;
	enum	MessageBoxResult
	{
		msgboxResultOk,
		msgboxResultCancel,
		msgboxResultYes,
		msgboxResultNo,
		msgboxResultRetry,
		msgboxResultAbort,
		msgboxResultIgnore,
		msgboxResultUser,
	} ;
	__native int MessageBox
		( const wchar_t * pwszMsg,
			const wchar_t * pwszCaption = NULL,
			int nStyles = msgboxStyleOk,
			SakuraGL::Window * pPaintWnd = NULL ) ;
	#if	defined(__PLATFORM_WINDOWS__)
	int DoMessageBox
		( const wchar_t * pwszMsg,
			const wchar_t * pwszCaption = NULL,
			int nStyles = msgboxStyleOk, HWND hwndParent = NULL ) ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// メモリアトミック操作
	//////////////////////////////////////////////////////////////////////////

	__native atomic_int_t AtomicXchg( volatile atomic_int_t* p, atomic_int_t v ) ;
	__native atomic_int_t AtomicAdd( volatile atomic_int_t* p, atomic_int_t v ) ;
	__native atomic_int_t AtomicSub( volatile atomic_int_t* p, atomic_int_t v ) ;
	__native atomic_int_t AtomicAnd( volatile atomic_int_t* p, atomic_int_t v ) ;
	__native atomic_int_t AtomicOr( volatile atomic_int_t* p, atomic_int_t v ) ;
	__native atomic_int_t AtomicXor( volatile atomic_int_t* p, atomic_int_t v ) ;

}


//////////////////////////////////////////////////////////////////////////////
// 標準的なメモリアロケーター
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

namespace	SSystem
{
	extern	bool			g_eslMemoryTightness ;

	// メモリ残量が逼迫しているか？
	bool IsMemoryTightness( void ) ;
	// メモリ残量が逼迫しているかテスト
	bool TestMemoryTightness( void ) ;

	// メモリ逼迫通知チェイン
	typedef void (*PTR_FUNC_NOTIFY_TIGHTNESS)
						( void * pInstance, int nReserved ) ;
	struct	NOTIFY_MEMORY_TIGHTNESS_CHAIN
	{
		NOTIFY_MEMORY_TIGHTNESS_CHAIN *	pNextChain ;
		PTR_FUNC_NOTIFY_TIGHTNESS		pfnNotify ;
		void *							pInstance ;
	} ;
	extern	NOTIFY_MEMORY_TIGHTNESS_CHAIN *	m_pnmtcMemTightnessChain ;

	void AddMemoryTightnessNotification
			( PTR_FUNC_NOTIFY_TIGHTNESS pfnNotify, void * pInstance ) ;
	void RemoveMemoryTightnessNotification
			( PTR_FUNC_NOTIFY_TIGHTNESS pfnNotify, void * pInstance ) ;
	void RemoveAllMemoryTightnessNotification( void ) ;
	void NotifyMemoryTightness( int nReserved = 0 ) ;
}

inline void * esl_malloc( size_t size )
{
	return	esl_stub_malloc( size ) ;
}

inline void * esl_realloc( void * memblock, size_t size )
{
	return	esl_stub_relloc( memblock, size ) ;
}

inline void esl_free( void * memblock )
{
	esl_stub_free( memblock ) ;
}

#endif


#endif

