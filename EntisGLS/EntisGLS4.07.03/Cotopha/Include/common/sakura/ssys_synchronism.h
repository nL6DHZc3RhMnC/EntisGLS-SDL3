
#if	!defined(__SAKURA2_SYNCHRONISM_H__)
#define	__SAKURA2_SYNCHRONISM_H__

//////////////////////////////////////////////////////////////////////////////
// 同期オブジェクトクラス
//////////////////////////////////////////////////////////////////////////////

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// 同期基底クラス
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Synchronism
	{
	public:
		enum	Timeout
		{
			Infinite	= -1,
		} ;
		// シグナル値取得
		native atomic_int_t Value( void ) const ;
		// 待機
		native SError Wait( int64_t msecTimeout = Infinite ) ;
	} ;
	#endif

	class	SSynchronismInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSynchronismInterface, ESLObject )
		// 待機
		enum	Timeout
		{
			Infinite	= -1,
		} ;
		virtual SError Wait( int64_t msecTimeout = Infinite ) = 0 ;
	} ;

	class	SSynchronism	: public SSynchronismInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSynchronism, SSynchronismInterface )

	protected:
	#if	defined(__COTOPHA__)
		Synchronism *			m_pSync ;

	#elif	defined(__PLATFORM_WINDOWS__)
		HANDLE					m_hSync ;
		volatile atomic_int_t	m_value ;

		virtual void OnSignal( void ) ;

	#else
	public:
		enum	FutexValue
		{
			futexWaked	= 1,
			futexLocked	= 0,
			futexWait	= -1,
		} ;
	protected:
		volatile atomic_int_t	m_mutex ;
		volatile atomic_int_t	m_value ;
		volatile atomic_int_t	m_wait ;

		virtual void OnSignal( void ) ;

	public:
		static void LockSimpleMutex( volatile atomic_int_t * mutex ) ;
		static void UnlockSimpleMutex( volatile atomic_int_t * mutex ) ;
	#endif

	public:
		// 構築関数
		SSynchronism( void ) ;
		// 消滅関数
		virtual ~SSynchronism( void ) ;
		// 同期オブジェクト削除
		virtual void Delete( void ) ;
		// シグナル値取得
		virtual atomic_int_t Value( void ) const ;
		// 待機
		// ※シグナル状態になると errSuccess を返す
		// 　errSuccess を受け取った直後に SSynchronism を安全に削除できることは保証されていない
		// 　SignalEvent のみ、SetSignal を待つ Wait の直後に安全に削除することが保証される
		virtual SError Wait( int64_t msecTimeout = Infinite ) ;

	#if	defined(__PLATFORM_WINDOWS__)
		// ハンドル取得
		HANDLE GetHandle( void ) const
		{
			return	m_hSync ;
		}
	#endif
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SSynchronism	Synchronism ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// シグナルイベント同期クラス
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native SignalEvent	: public Synchronism
	{
	public:
		// 生成
		static native SignalEvent * Create( bool fInitSignal ) ;
		// シグナル値設定
		native void SetSignal( void ) ;
		native void ResetSignal( void ) ;
	} ;
	#endif

	class	SSignalEvent	: public SSynchronism
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSignalEvent, SSynchronism )

	public:
		// 初期化
		void Initialize( bool fSignal ) ;
		// シグナル値設定
		virtual void SetSignal( void ) ;
		virtual void ResetSignal( void ) ;

		#if	!defined(__COTOPHA__)
		// 生成
		static SSignalEvent * Create( bool fInitSignal ) ;
		#endif

		#if	defined(__PLATFORM_WINDOWS__)
		// シグナル値取得
		virtual atomic_int_t Value( void ) const ;
		#endif
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SSignalEvent	SignalEvent ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// セマフォ同期クラス
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Semaphore	: public Synchronism
	{
	public:
		// 生成
		static native Semaphore * Create
			( atomic_int_t nInitCount = 1, atomic_int_t nMaxCount = 1 ) ;
		// セマフォ解放
		native void Release( void ) ;
	} ;
	#endif

	class	SSemaphore	: public SSynchronism
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSemaphore, SSynchronism )

	protected:
	#if	!defined(__COTOPHA__) && !defined(__PLATFORM_WINDOWS__)
		atomic_int_t	m_countMax ;

		virtual void OnSignal( void ) ;
	#endif

	public:
		// 初期化
		void Initialize
			( atomic_int_t nInitCount = 1, atomic_int_t nMaxCount = 1 ) ;
		// セマフォ解放
		virtual void Release( void ) ;

		#if	!defined(__COTOPHA__)
		static SSemaphore * Create
			( atomic_int_t nInitCount = 1, atomic_int_t nMaxCount = 1 ) ;
		#endif
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SSemaphore	Semaphore ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// ミューテックス同期クラス
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Mutex	: public Synchronism
	{
	public:
		// 生成
		static native Mutex * Create( void ) ;
		// ミューテックス解放
		native void Release( void ) ;
	} ;
	#endif

	// 軽量ミューテックス
	class	SCriticalSection	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SCriticalSection, ESLObject )
		// 構築関数
		SCriticalSection( void ) ;
		// 消滅関数
		virtual ~SCriticalSection( void ) ;

	protected:
	#if	defined(__COTOPHA__)
		Mutex *				m_pMutex ;
	#elif	defined(__PLATFORM_WINDOWS__)
		CRITICAL_SECTION	m_csMutex ;
		DWORD			m_idOwnerThread ;
		atomic_int_t	m_countLocked ;
	#else
		volatile atomic_int_t	m_mutex ;
		volatile atomic_int_t	m_value ;
		volatile atomic_int_t	m_wait ;
		volatile pid_t			m_pidOwner ;
		volatile atomic_int_t	m_countLocked ;
		/*
		static pid_t gettid( void )
		{
			return syscall( SYS_gettid ) ;
		}
		*/
	#endif

	public:
		// 同期
		void Lock( void ) const ;
		// 解放
		void Unlock( void ) const ;

	#if	!defined(__COTOPHA__)
		// 現在のスレッドが排他処理権を有しているか？
		atomic_int_t TestLocked( void ) const ;
		// 現在のスレッドが排他処理権を有している場合、全て解放する
		atomic_int_t UnlockAll( void ) const ;
		// UnlockAll された排他処理を再度取得する
		void Relock( atomic_int_t nLock ) const ;

		// ロック回数
		atomic_int_t GetLockedCount( void ) const
		{
			return	m_countLocked ;
		}
		#if	defined(__PLATFORM_WINDOWS__)
			// ロックスレッド
			DWORD GetLockThreadId( void ) const
			{
				return	m_idOwnerThread ;
			}
		#else
			// ロックスレッド
			DWORD GetLockThreadId( void ) const
			{
				return	m_pidOwner ;
			}
		#endif

	#endif

	} ;

	// ミューテックス
	class	SMutex	: public SSynchronism
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SMutex, SSynchronism )

	protected:
	#if	defined(__COTOPHA__)

	#elif	defined(__PLATFORM_WINDOWS__)
		DWORD			m_idOwnerThread ;
		atomic_int_t	m_countLocked ;

		virtual void OnSignal( void ) ;

	#else
		volatile pid_t			m_pidOwner ;
		volatile atomic_int_t	m_countLocked ;
		/*
		static pid_t gettid( void )
		{
			return syscall( SYS_gettid ) ;
		}
		*/
		virtual void OnSignal( void ) ;

	public:
		virtual SError Wait( int64_t msecTimeout = Infinite ) ;
	#endif

	#if	defined(__DEBUG__)
		const char *	m_pszSource ;
		size_t			m_nLineNum ;
		uint64_t		m_nLockedTick ;
	#endif

	public:
		// 構築関数
		SMutex( void ) ;
		// 初期化
		void Initialize( void ) ;
		// ミューテックス解放
		virtual void Release( void ) ;

	public:
		// Wait() の別名
		SError LockTrace
				( const char * pszSource,
					size_t nLineNum, int64_t msecTimeout = Infinite ) const ;
		SError Lock( int64_t msecTimeout = Infinite ) const ;
		// Release() の別名
		void Unlock( void ) const ;
		// 排他的処理時間
		uint32_t UnlockLatency( void ) const ;

		#if	!defined(__COTOPHA__)

		// 生成
		static SMutex * Create( void ) ;

		// 現在のスレッドが排他処理権を有しているか？
		atomic_int_t TestLocked( void ) const ;
		// 排他的処理時間チェック付き Release
		uint32_t UnlockVerifyLatency( uint32_t msecMargin ) const ;
		// 現在のスレッドが排他処理権を有している場合、全て解放する
		atomic_int_t UnlockAll( void ) ;
		// UnlockAll された排他処理を再度取得する
		SError Relock( atomic_int_t nLock ) ;

		#endif
	} ;

	// 共有可能ミューテックス
	class	SSharableMutex	: public SMutex
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSharableMutex, SMutex )

	protected:
	#if	defined(__COTOPHA__)

	#elif	defined(__PLATFORM_WINDOWS__)
		typedef	DWORD	ThreadID ;
	#else
		typedef	pid_t	ThreadID ;
	#endif
		struct	ShareThread
		{
			ThreadID	idThread ;
			size_t		nLocked ;
		} ;
		enum	ConstantValue
		{
			maxThreadCount	= 64,
		} ;
		ShareThread	m_ShareThreads[maxThreadCount] ;
		size_t		m_nShareThreads ;

	public:
		// 構築関数
		SSharableMutex( void ) ;

	public:
		// 共有スレッド追加（既に他のスレッドが所有権を獲得している状態で呼び出す）
		SError SharedLock( void ) ;
		// 共有スレッド削除（所有権を獲得しているスレッドが解放する前に呼び出す）
		SError SharedUnlock( void ) ;

	protected:
		// 共有スレッド判定
		ssize_t FindSharedThread( void ) const ;
		// スレッド・ロックカウンタ減算
		void UnlockSharedThread( size_t iThread ) ;

	public:
		// ミューテックス解放
		virtual void Release( void ) ;
		// 所有権獲得
		virtual SError Wait( int64_t msecTimeout = Infinite ) ;
		// 所有権獲得時
		virtual void OnSignal( void ) ;

		// 現在のスレッドが排他処理権を有しているか？
		atomic_int_t TestLocked( void ) const ;
		// 排他的処理時間チェック付き Release
		uint32_t UnlockVerifyLatency( uint32_t msecMargin ) const ;
		// 現在のスレッドが排他処理権を有している場合、全て解放する
		atomic_int_t UnlockAll( void ) ;
		// UnlockAll された排他処理を再度取得する
		SError Relock( atomic_int_t nLock ) ;
	} ;

	// 状態別ミューテックス
	class	SStateMutex	: public SSynchronismInterface
	{
	public:
		enum	StateType
		{
			typeExclusive,
			typeSharable,
		} ;

	protected:
		int					m_state ;
		StateType			m_type ;
		atomic_int_t		m_countLocked ;
	#if	defined(__COTOPHA__)
		ulong_ptr_t			m_idThread ;
	#elif	defined(__PLATFORM_WINDOWS__)
		DWORD				m_idThread ;
	#else
		pid_t				m_idThread ;
	#endif
		SSignalEvent		m_signalFree ;
		SCriticalSection	m_csState ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SStateMutex, SSynchronismInterface )
		// 構築関数
		SStateMutex( void ) ;

	public:
		// 待機（デフォルトの状態でロックする）
		virtual SError Wait( int64_t msecTimeout = Infinite ) ;

	public:
		// 指定の状態でロックする
		SError LockState
			( int state, StateType type, int64_t msecTimeout = Infinite ) ;
		// ロックを解除する
		void Unlock( void ) ;
	} ;

	// Read/Write 状態別ミューテックス
	class	SReadWriteMutex	: public SStateMutex
	{
	public:
		enum	State
		{
			stateRead,
			stateWrite,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SReadWriteMutex, SStateMutex )
		// 構築関数
		SReadWriteMutex( void ) ;

	public:
		// Read 用にロックする（他のスレッドも Read 出来る）
		SError LockRead( int64_t msecTimeout = Infinite ) ;
		// Write 用にロックする（Unlock するまで他のスレッドはロックできない）
		SError LockWrite( int64_t msecTimeout = Infinite ) ;
	} ;

	// ミューテックス・テンプレート
	template <class T> class SSmartLock
	{
	protected:
		T *				m_pSync ;
		atomic_int_t	m_nRelock ;
	public:
		// 構築関数
		SSmartLock( T * pSync ) : m_pSync( pSync ), m_nRelock( 0 )
		{
			if ( pSync != NULL )
			{
				pSync->Lock() ;
			}
		}
		// 消滅関数
		~SSmartLock( void )
		{
			if ( m_pSync != NULL )
			{
				ESLAssert( m_nRelock == 0 ) ;
				m_pSync->Unlock() ;
			}
		}
		// 解放
		void Unlock( void )
		{
			if ( m_pSync != NULL )
			{
				m_pSync->Unlock() ;
				m_pSync = NULL ;
			}
		}
		void UnlockAll( void )
		{
			if ( m_pSync != NULL )
			{
				m_nRelock = m_pSync->UnlockAll() ;
			}
		}
		// 再度所有権獲得
		void Relock( void )
		{
			if ( m_pSync != NULL )
			{
				m_pSync->Relock( m_nRelock ) ;
				m_nRelock = 0 ;
			}
		}
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SMutex	Mutex ;
	#endif

	// 全体ミューテックス
	SError Lock( int64_t msecTimeout = Synchronism::Infinite ) ;
	SError LockTrace
		( const char * pszSource, size_t nLineNum,
			int64_t msecTimeout = Synchronism::Infinite ) ;
	SError Unlock( void ) ;
	atomic_int_t UnlockAll( void ) ;
	SError Relock( atomic_int_t nLock ) ;

	#if	defined(__COTOPHA__)
		native SError LockSystem
				( int64_t msecTimeout = Synchronism::Infinite ) ;
		native SError UnlockSystem( void ) ;
		native atomic_int_t UnlockAllSystem( void ) ;

		native void QuickLock( void ) ;
		native void QuickUnlock( void ) ;

	#else
		extern ESL_DLL_EXPORT SSharableMutex*		g_mutexGlobal ;
		extern ESL_DLL_EXPORT SCriticalSection *	g_csmutexGlobal ;

		inline atomic_int_t TestLocked( void )
		{
			ESLAssert( g_mutexGlobal != NULL ) ;
			return	g_mutexGlobal->TestLocked() ;
		}
		inline uint32_t UnlockLatency( void )
		{
			ESLAssert( g_mutexGlobal != NULL ) ;
			return	g_mutexGlobal->UnlockLatency() ;
		}
		inline uint32_t UnlockVerifyLatency( uint32_t msecMargin )
		{
			ESLAssert( g_mutexGlobal != NULL ) ;
			return	g_mutexGlobal->UnlockVerifyLatency( msecMargin ) ;
		}
		inline void QuickLock( void )
		{
			ESLAssert( g_csmutexGlobal != NULL ) ;
			g_csmutexGlobal->Lock() ;
		}
		inline void QuickUnlock( void )
		{
			ESLAssert( g_csmutexGlobal != NULL ) ;
			g_csmutexGlobal->Unlock() ;
		}

	#endif


	//////////////////////////////////////////////////////////////////////////
	// イベント通知
	//////////////////////////////////////////////////////////////////////////

	class	SEventSignalNotifier	: public ESLObject
	{
	protected:
		SCriticalSection			m_csSync ;
		SPointerArray<SSignalEvent>	m_aSignals ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SEventSignalNotifier, ESLObject )
		// 構築関数
		SEventSignalNotifier( void ) ;
		// 消滅関数
		virtual ~SEventSignalNotifier( void ) ;

	public:
		// 通知対象追加
		void AttachSignalEvent( SSignalEvent * pSignal ) ;
		// 通知対象解除
		void DetachSignalEvent( SSignalEvent * pSignal ) ;
		void DetachAllSignalEvents( void ) ;
		// 通知
		void NotifyEventSignal( bool afterDetachAll = true ) ;

	} ;

} ;


#endif

