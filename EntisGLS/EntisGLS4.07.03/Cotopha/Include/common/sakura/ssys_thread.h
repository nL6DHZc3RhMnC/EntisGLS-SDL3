
#if	!defined(__SAKURA2_THREAD_H__)
#define	__SAKURA2_THREAD_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// 関数抽象クラス
	//////////////////////////////////////////////////////////////////////////

	class	SProcedure
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SProcedure )
		// 消滅関数
		virtual ~SProcedure( void ) ;
		// スレッド関数
		virtual void Run( void ) = 0 ;
		// 開始前の処理
		virtual void Prepare( void ) ;
		// 完了後の処理
		virtual void Finalize( void ) ;
		// 処理の即時終了要求（主に別スレッドから呼び出し）
		enum	RequestQuitLevel
		{
			quitNotice = -1,	// 終了通知（終了までにUI等を含むことも可）
			quitNormally,		// 通常終了（長い時間の処理は出来るだけ含まず
								//          ／長い処理を含まないなら通常通り実行）
			quitAbort,			// 可能な限り即時終了
		} ;
		virtual void RequestQuit( RequestQuitLevel rql ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 関数呼び出しクラス
	//////////////////////////////////////////////////////////////////////////

	class	SProcedureCaller	: public ESLObject, public SProcedure
	{
	public:
		typedef void (*PFUNC_PTR)( void * pInstance ) ;

	protected:
		PFUNC_PTR	m_pfnFunc ;
		void *		m_pInstance ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SProcedureCaller, ESLObject, SProcedure )
		// 構築関数
		SProcedureCaller( PFUNC_PTR pfnFunc, void * pInstance ) ;
		// スレッド関数
		virtual void Run( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 同期関数
	//////////////////////////////////////////////////////////////////////////

	class	SSyncProcedure	: public ESLObject, public SProcedure
	{
	protected:
		bool				m_flagAutoDelete ;
		bool				m_flagFinished ;
		bool				m_flagProcOwner ;
		atomic_int_t		m_countWaitRef ;
		SProcedure *		m_pProc ;
		SSignalEvent		m_done ;
		SCriticalSection	m_csSync ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SSyncProcedure, ESLObject, SProcedure )
		// 構築関数
		SSyncProcedure( SProcedure * pProc, bool flagOwner = false ) ;
		// スレッド関数
		virtual void Run( void ) ;
		// 開始前の処理
		virtual void Prepare( void ) ;
		// 完了後の処理
		virtual void Finalize( void ) ;
		// 終了後に自身を消去するか設定する
		void SetAutoDelete( bool flagAutoDelete = true ) ;
		// 完了待ち
		SError WaitDone( int64_t timeout = Synchronism::Infinite ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 例外クラス
	//////////////////////////////////////////////////////////////////////////

	class	SException	: public ESLObject
	{
	protected:
		int64_t	m_nError ;
		SString	m_strMessage ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SException, ESLObject )
		// 構築関数
		SException( void ) ;
		SException( const SException& e ) ;
		SException( int64_t nError, const wchar_t * pwszMessage ) ;
		// 消滅関数
		virtual ~SException( void ) ;
		// エラーコード取得
		int64_t GetError( void ) const ;
		// メッセージ取得
		const SString& GetMessage( void ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// スレッドオブジェクト
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Thread
	{
	public:
		// スレッド実行中？
		native bool IsRunning( void ) const ;
		// 待機
		native SError Wait( int64_t msecTimeout = Synchronism::Infinite ) ;
		// スレッド起動
		native SError BeginThread( SProcedure * pProc ) ;
		// フレーム駆動スレッド起動
		native SError BeginFrameThread
					( SProcedure * pProc, size_t nInitStack ) ;
		// フレーム駆動
		native SError ContinueFrameThread( void ) ;
		// 例外をスロー
		native SError ThrowException( const wchar_t * pszErrMsg = NULL ) ;
		// 現在のスレッド取得
		static native Thread * GetCurrentThread( void ) ;
		static native SProcedure * GetCurrentThreadProc( void ) ;
		// 関連付けアイテム取得
		static ESLObject * GetLocalStorageAs( const wchar_t * pwszID ) ;
		// 関連付けアイテム設定
		static ESLObject *
			SetLocalStorageAs( const wchar_t * pwszID, ESLObject * pObj ) ;
	} ;
	#endif

	class	SThread	: public SSynchronism
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SThread, SSynchronism )
		// 構築関数
		SThread( void ) ;
		// 消滅関数
		virtual ~SThread( void ) ;

	protected:
		SProcedure *	m_pProc ;

	#if	defined(__COTOPHA__)
		// 関数リダイレクト
		class	SThreadProcecure : public SProcedure
		{
		public:
			SThread *	m_pThread ;
		public:
			// スレッド関数
			virtual void Run( void ) ;
		} ;
		Thread *			m_pThread ;
		SThreadProcecure	m_procRedirect ;

		friend class SThreadProcecure ;

	#else
		#if	defined(__PLATFORM_WINDOWS__)
			// スレッド関数
			static DWORD WINAPI ThreadProc( LPVOID lpParameter ) ;
			static DWORD WINAPI FrameThreadProc( LPVOID lpParameter ) ;

			DWORD			m_dwThreadID ;

		#else
			// スレッド関数
			static void * ThreadProc( void * lpParameter ) ;
			static void * FrameThreadProc( void * lpParameter ) ;

			bool			m_flagThread ;
			pthread_t		m_idThread ;
			SSignalEvent	m_signalDone ;

			#if	defined(__PLATFORM_ANDROID__)
			JNIEnv *		m_pJNIEnv ;
			#endif
		#endif

		bool			m_flagFinished ;
		bool			m_flagThrow ;
		atomic_int_t	m_countPending ;
		SSignalEvent	m_signalMaster ;	// マスタスレッド側モニタ
		SSignalEvent	m_signalFrame ;		// フレームスレッド側モニタ
		SString			m_strException ;

		friend void SSystem::SleepFrame( int64_t frames ) ;
	#endif

	public:
		// 同期オブジェクト削除
		virtual void Delete( void ) ;
		// シグナル値取得
		virtual atomic_int_t Value( void ) const ;

		#if	!defined(__PLATFORM_WINDOWS__)
		// 待機
		virtual SError Wait( int64_t msecTimeout = Infinite ) ;
		#endif

		// スレッド実行中？
		virtual bool IsRunning( void ) const ;

	public:
		// スレッド起動
		virtual SError BeginThread( SProcedure * pProc ) ;
		// フレーム駆動スレッド起動
		SError BeginFrameThread( SProcedure * pProc, size_t nInitStack ) ;
		// フレーム駆動
		SError ContinueFrameThread( void ) ;
		// フレーム同期
		SError SyncFrameThread( void ) ;
		// 例外をスロー
		SError ThrowException( const wchar_t * pszErrMsg = NULL ) ;
		// 現在のスレッド取得
		static SThread * GetCurrentThread( void ) ;
		static SProcedure * GetCurrentThreadProc( void ) ;
		// 現在のスレッド判定
		bool IsCurrentThread( void ) ;
		// スレッド識別子
		#if	defined(__COTOPHA__)
		typedef	ulong_ptr_t	IdType ;
		constant	InvalidId	= 0 ;
		#elif	defined(__PLATFORM_WINDOWS__)
		typedef	DWORD	IdType ;
		enum { InvalidId = 0 } ;
		#else
		typedef	pid_t	IdType ;
		enum { InvalidId = 0 } ;
		#endif
		static IdType GetCurrentId( void ) ;

		#if	!defined(__COTOPHA__)
		// スレッド関連付け
		void AttachCurrentThread( void ) ;
		// スレッド関連付け解除
		void DetachCurrentThread( void ) ;
		#endif

	public:
		// スレッド関数
		typedef	void (*THREAD_PROCEDURE)( void * pInstance ) ;

		// スレッド関数実行
		static SThread * BeginStockThread
				( THREAD_PROCEDURE pfnProc, void * pInstance ) ;
		// ストックされている全スレッドを終了させる
		static void ExitAllStockedThread( void ) ;

	public:
		// スレッド切り替え性能テスト（評価値は１回のスレッド同期切り替えのミリ秒）
		static double TestSwitchingPerformance( void ) ;

		static double		s_msecSwitchingPerformance ;

	protected:
		struct	TEST_SWITCHING_PARAM
		{
			atomic_int_t	nThreads ;
			atomic_int_t	nReady ;
			atomic_int_t	nProcessed ;
			bool			fQuitEvent ;
			SSignalEvent *	pSignalDone ;
			SSignalEvent *	pSignalReady ;
			SSignalEvent *	pSignalEvent ;
		} ;
		static	void TestSwitchingPerformanceProc( void * pInstance ) ;

	protected:
		// ストックスレッド
		class	StockThreadProcedure	: public SProcedure
		{
		public:
			StockThreadProcedure *	m_pNext ;
			SThread *				m_pThread ;
			SSignalEvent			m_evStart ;
			THREAD_PROCEDURE		m_procStart ;
			void *					m_ptrInstance ;
		public:
			// 構築関数
			StockThreadProcedure( void ) ;
			// 消滅関数
			~StockThreadProcedure( void ) ;
			// スレッド開始
			SError BeginThread( void ) ;
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		protected:
			// 遅延削除関数
			static void DelayDeleteThread( void * pInstance ) ;
		} ;
		static ESL_DLL_EXPORT StockThreadProcedure *	m_pStockThread ;
		static ESL_DLL_EXPORT atomic_int_t				m_countStockedThread ;
		static ESL_DLL_EXPORT atomic_int_t				m_countRunningStockThread ;
		friend class	StockThreadProcedure ;

	public:
		// 実行中ストックスレッド数取得
		static atomic_int_t GetRunningStockThread( void )
		{
			return	m_countRunningStockThread ;
		}

	public:	// スレッドローカルストレージ
		// 関連付けアイテム取得
		static ESLObject * GetLocalStorageAs( const wchar_t * pwszID ) ;
		// 関連付けアイテム設定
		static ESLObject *
			SetLocalStorageAs( const wchar_t * pwszID, ESLObject * pObj ) ;
		// スレッドローカルストレージ解放
		static void ReleaseLocalStorage( void ) ;
	} ;

	#if	!defined(__COTOPHA__)
		typedef	SThread	Thread ;

		#if	defined(__PLATFORM_WINDOWS__)
			extern ESL_DLL_EXPORT	DWORD	g_tlsThread ;
			extern ESL_DLL_EXPORT	DWORD	g_tlsStorage ;
		#else
			extern	pthread_key_t	g_keyThread ;
			extern	pthread_key_t	g_keyStorage ;
		#endif
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 並列処理用スレッド関数
	//////////////////////////////////////////////////////////////////////////

	class	SParallelProcedure
	{
	protected:
		SCriticalSection			m_csFastLock ;
		//volatile atomic_int_t		m_atomSpinLock ;
		volatile atomic_int_t		m_countRunning ;
		SSignalEvent				m_signalFinished ;

		struct	THREAD_PARAM
		{
			SParallelProcedure *	pThis ;
			void *					pInstance ;
		} ;

		// 全SParallelProcedure共通
		// ※SParallelProcedure::Start の呼び出し自体が
		// 　複数のスレッドで実行された場合の
		// 　並列スレッド総数を抑制するため
		static atomic_int_t	m_countReadyThread ;
		static unsigned int	m_countProcessor ;

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SParallelProcedure )
		// 構築関数
		SParallelProcedure( void ) ;
		// 指定数のスレッドで並列処理を実行する
		void Start( void** pInstance, size_t countThread ) ;
		// スピンロック
		void SpinLock( void ) ;
		void SpinUnlock( void ) ;

	protected:
		// スレッド関数
		static void ParallelThreadProc( void * pInstance ) ;

	public:
		// ループ処理／終了判定関数
		virtual bool Continue( void * pInstance ) = 0 ;
		// 並列処理関数
		virtual void RunParallel( void * pInstance ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 実行キュー
	//////////////////////////////////////////////////////////////////////////

	class	SProcedureQueue
	{
	public:
		struct	ProcContainer
		{
			SProcedure *	m_pProc ;
			SSignalEvent *	m_pDone ;
			bool			m_flagAutoDelete ;
			bool			m_flagFence ;

			ProcContainer( void )
				: m_pProc( nullptr ), m_pDone( nullptr ),
					m_flagAutoDelete( false ),
					m_flagFence( false ) { }
			ProcContainer
				( SProcedure * pProc,
					SSignalEvent * pDone = nullptr,
					bool flagAutoDelete = false, bool flagFence = false )
				: m_pProc( pProc ), m_pDone( pDone ),
					m_flagAutoDelete( flagAutoDelete ),
					m_flagFence( flagFence ) { }
			ProcContainer( const ProcContainer& pc )
				: m_pProc( pc.m_pProc ), m_pDone( pc.m_pDone ),
					m_flagAutoDelete( pc.m_flagAutoDelete ),
					m_flagFence( pc.m_flagFence ) { }
			const ProcContainer& operator = ( const ProcContainer& pc )
			{
				m_pProc = pc.m_pProc ;
				m_pDone = pc.m_pDone ;
				m_flagAutoDelete = pc.m_flagAutoDelete ;
				m_flagFence = pc.m_flagFence ;
				return	*this ;
			}
			bool IsEmpty( void ) const
			{
				return	(m_pProc == nullptr) && (m_pDone == nullptr) ;
			}
		} ;
		typedef	const ProcContainer *	ProcIdentity ;
		static constexpr const ProcIdentity	NullProcId = nullptr ;

		class	Dispatcher
		{
		protected:
			SProcedureQueue&	m_queue ;
			ProcContainer *		m_pcRunning ;
			SProcedure::RequestQuitLevel
								m_reqQuit ;
			bool				m_flagQuit ;
			bool				m_flagRunning ;
		public:
			Dispatcher( SProcedureQueue& que ) ;
			void Invoke( ProcContainer& pc ) ;
			void RequestQuit( SProcedure::RequestQuitLevel rql ) ;
		} ;

	protected:
		SCriticalSection				m_csLock ;
		SLinkedList<ProcContainer>		m_listProc ;
		SObjectArray<Dispatcher>		m_dispatchers ;
		SSignalEvent					m_signalDispatch ;
		SSignalEvent					m_signalProc ;
		SSignalEvent					m_signalEmpty ;
		SSignalEvent					m_signalAllDone ;
		volatile int					m_nPendingProc ;
		SPointerArray<SProcedure>		m_aFinallyProcs ;
		SProcedure::RequestQuitLevel	m_reqQuitLevel ;
		volatile bool					m_flagQuit ;
		volatile bool					m_flagFence ;

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SProcedureQueue )
		// 構築関数
		SProcedureQueue( void ) ;
		// 消滅関数
		~SProcedureQueue( void ) ;
		// 初期化（再使用）
		void Reset( void ) ;
		// キューに追加
		ProcIdentity AddProcedure
			( SProcedure * pProc,
				SSignalEvent * pDoneSignal = nullptr,
				bool flagAutoDelete = false, bool flagFence = false ) ;
		// 未実行ならキューから削除
		bool CancelProcedure( ProcIdentity procId ) ;
		// キューの実行順序をフェンスする
		void SetFence( void ) ;
		// キューから取り出す
		ProcContainer GetProcedure
			( int64_t msecTimeout = SSynchronismInterface::Infinite ) ;
		ProcContainer PeekProcedure( void ) ;
		// キュー同期
		void Lock( void ) const ;
		void Unlock( void ) const ;
		// キューを実行（キューが空でも終了まで待機）
		void Run( void ) ;
		void RunAll( void ) ;
		// 別スレッドでキュー（Run）を実行
		void AsyncRun( void ) ;
		// キューを実行（終了判定関係なくキューが空になるまで）
		size_t Flush( void ) ;
		// キューが空か？
		bool IsEmpty( void ) const ;
		// キューが空になるのを待つ
		SError WaitUntilEmpty( int64_t msecTimeout = SSynchronism::Infinite ) ;
		// 全ての実行ループが完了するのを待つ
		SError WaitAllRunLoops( int64_t msecTimeout = SSynchronism::Infinite ) ;
		// Dispatcher 取得
		Dispatcher * GetDispatcher( void ) ;
		// Dispatcher 解放
		void ReleaseDispatcher( Dispatcher * pDispatcher ) ;
		// 終了フラグ設定
		void RequestQuit( SProcedure::RequestQuitLevel rql = SProcedure::quitNormally ) ;
		// 終了フラグ判定
		bool GetQuitFlag( void ) const ;

	public:
		// FinallyProcedur;
		//   Run（AsyncRun）完了時に Flush を呼び出さなくても
		//   必ず実行すべき SProcedure
		// ※同一の SProcedure* がキュー上で実行されると
		//   自動的に DetachFinallyProcedur される
		SError AttachFinallyProcedur( SProcedure * pProc ) ;
		SError DetachFinallyProcedur( SProcedure * pProc ) ;

	protected:
		// 別スレッドでキューを実行する関数
		static void AsyncRunProc( void * pInstance ) ;
		// 実行キューシグナル更新
		void CheckPendingSignal( void ) ;
		// 実行キュー空シグナル更新
		void CheckEmptySignal( void ) ;
		// 実行ループ全完了シグナル更新
		void CheckAllDoneSignal( void ) ;

	} ;


} ;

#endif

