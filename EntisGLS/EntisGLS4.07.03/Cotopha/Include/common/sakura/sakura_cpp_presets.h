
#if	!defined(__SAKURA_CPP_PRESETS_H__)
#define	__SAKURA_CPP_PRESETS_H__	1

#if	defined(__COTOPHA__)
	// 実行プラットフォーム・スイッチ
	#if	!defined(PLATFORM_ANDROID)
	constant	PLATFORM_ANDROID = 0 ;
	#endif
	#if	!defined(PLATFORM_WINDOWS)
	constant	PLATFORM_WINDOWS = !PLATFORM_ANDROID ;
	#endif

	// 詞葉の基本型宣言
	#if	!defined(__GLS_H__)
	constant	__GLS_H__ = 1 ;
	#endif

	#mode basic style
	include "basictype.ch"
	#mode c compatible

	// 詞葉コンパイラでは
	// wchar_t と uint16_t は型の区別されない
	constant __WCHAR_EQU_UINT16__ = 1 ;

	// ポインタ型は64ビット
	constant	__POINTER64__	= 1 ;

	#if	!defined(__native)
		#define	__native	native
	#endif

#if	defined(DEBUG)
		#if	DEBUG
			constant	__DEBUG__ = 1 ;
		#endif
	#endif

#else
	#if	!defined(__native)
		#define	__native
	#endif

	#if	defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
		// Windows 環境 (base API => Win32 or Win64)
		#define	__PLATFORM_WINDOWS__	1
		#define	PLATFORM_WINDOWS	1
		#define	PLATFORM_ANDROID	0

		// 生成コードタイプ
		#if	defined(_M_X64) || defined(_M_AMD64)
			#define	DISABLE_ENTIS_GLS3	1
			#define	__PROCESSOR_INTEL_X86_64__		1
			#define	__PROCESSOR_INTEL_X86_SSE__		2
			#define	__POINTER64__	1
		#elif	defined(_M_IX86)
			#define	__PROCESSOR_INTEL_X86__	_M_IX86
			#if	defined(_M_IX86_FP) && !defined(_M_CEE_PURE)
				#define	__PROCESSOR_INTEL_X86_SSE__		_M_IX86_FP
			#endif
		#endif

	#else
		// Unix 系 (base API => POSIX)
		#define	__PLATFORM_UNIX_LIKE__		1
		#define	PLATFORM_WINDOWS	0
		#if	defined(PLATFORM_ANDROID)
			#if	PLATFORM_ANDROID
				// Android
				#define	__PLATFORM_ANDROID__	1
				#if	!defined(__ANDROID_BILLING_LIBRARY__)
					#define	__ANDROID_BILLING_LIBRARY__	5
				#endif
				#define	__API_OPEN_GL_ES__	1
				#if	!defined(ANDROID_API_LEVEL)
					#define	ANDROID_API_LEVEL	4
				#else
					#if	ANDROID_API_LEVEL >= 8
						#undef	__API_OPEN_GL_ES__
						#define	__API_OPEN_GL_ES__	2
					#endif
				#endif
				#if	!defined(__DISABLED_EXCEPTION__)
					#define	__DISABLED_EXCEPTION__	1
				#endif
			#endif
		#endif
		#if	!defined(ANDROID_NDK_VER)
			#define	ANDROID_NDK_VER	13
		#endif
		#if	!defined(PLATFORM_ANDROID)
			#define	PLATFORM_ANDROID	0
		#endif

		// 生成コードタイプ
		#if		defined(__x86_64__)
			#define	__PROCESSOR_INTEL_X86_64__		1
			#define	__PROCESSOR_INTEL_X86_SSE__		2
			#define	__POINTER64__	1
		#elif	defined(__i386__)
			#define	__PROCESSOR_INTEL_X86__	_M_IX86
		#elif	defined(__arm__)
			#if	defined(__aarch64__) || defined(__ARM_64BIT_STATE)
				#define	__POINTER64__	1
				#define	__PROCESSOR_ARM__	7
				#define	__PROCESSOR_ARM64__	8
			#elif	defined(__TARGET_ARCH_ARM)
				#define	__PROCESSOR_ARM__	__TARGET_ARCH_ARM
			#else
				#define	__PROCESSOR_ARM__	4
			#endif
			#if	defined(__thumb__)
				#if	defined(__TARGET_ARCH_THUMB)
					#define	__PROCESSOR_ARM_THUMB__	__TARGET_ARCH_THUMB
				#else
					#define	__PROCESSOR_ARM_THUMB__	1
				#endif
			#endif
		#elif	defined(__mips64__)
			#define	__POINTER64__	1
		#endif

	#endif

	#if	defined(_MSC_VER)
		#if	_MSC_VER < 1400
			// MS-C++ の古いバージョンでは
			// wchar_t と uint16_t は型の区別されない
			#define	__WCHAR_EQU_UINT16__	1
		#endif
	#endif

	#if	!defined(__DEBUG__)
		#if	defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
			#define	__DEBUG__	1
		#endif
	#endif

#endif

#endif

