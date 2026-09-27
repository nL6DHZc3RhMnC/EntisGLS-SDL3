
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_THREAD_H__)
#define	__GLSCS_SAKURA2_OBJECT_THREAD_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// スレッド・オブジェクト (SSystem::Thread)
	//////////////////////////////////////////////////////////////////////////

	class	ThreadObject
				: public BufferObject,
					public ECSSakura2Processor::ContextShell,
					public SSystem::SSynchronismInterface,
					public SSystem::SProcedure
	{
	protected:
		// 実行パラメータ
		enum	ThreadExecutionStatus
		{
			statusNoExecution,
			statusPrepare,
			statusRun,
			statusFinalize,
		} ;
		SSystem::SThread *		m_thread ;
		INT64					m_addrProc ;
		ThreadExecutionStatus	m_statusThread ;
		size_t					m_countPostArgStack ;
		volatile bool			m_fAbort ;
		SSystem::SSignalEvent	m_signalSuspending ;
		ExecutionStatus			m_statusPrevSave ;

		// 保存用ステータス構造体
		struct	SAVE_STATUS
		{
			// Context
			DWORD	lowIP ;
			DWORD	highIP ;
			DWORD	maskException ;
			DWORD	idSystemCall ;
			DWORD	idInteruption ;
			DWORD	dwContextStatus ;
			// Thread
			INT64	addrProc ;
			DWORD	dwThreadStatus ;
		} ;

	public:
		// ローカルストレージ
		SSystem::SStrSortArray<int64_t>	m_ssaLocalStorage ;

	public:
		enum	DefaultValue
		{
			// デフォルトのスタック最大サイズ
			Sakura2StackLimit	= 0x1000000,		// 16MB
		} ;
		enum	ProcedureVector
		{
			procVectorRun,
			procVectorPrepare,
			procVectorFinalize,
			procVectorCount,
		} ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO3_NONEW
			( ThreadObject, BufferObject,
				ECSSakura2Processor::ContextShell,
				SSystem::SSynchronismInterface )

	protected:
		// 構築関数
		ThreadObject( void ) ;
	public:
		// 生成
		static ThreadObject * NewContext( void )
		{
			return	new ThreadObject ;
		}
		// 消滅関数
		virtual ~ThreadObject( void ) ;
		// 実行中のスレッドを強制的に終了させる
		void AbortThread( void ) ;
		// スレッドは強制終了中か？
		bool IsThreadAborting( void ) const ;
		// スレッドをサスペンド状態にする（以前の実行ステータスを返す）
		ExecutionStatus SuspendThread( void ) ;

	public:
		// コンテキスト初期化
		virtual void InitializeContext( VirtualMachine* pVM ) ;
		// 関数呼び出し
		virtual const wchar_t *
			CallFunction
				( INT64 addrFunc, const Register *pArg, int nArgCount ) ;
		// 仮想関数呼び出し
		virtual const wchar_t *
			CallVirtualFunction
				( INT64 addrObj, int iVirtual,
						const Register *pArg, int nArgCount ) ;
		// フレーム駆動スレッド実行継続処理
		virtual const wchar_t * ContinueFrameThread( void ) ;
		// フレーム駆動スレッド実行完了処理
		virtual const wchar_t * FinalizeFrameThread( void ) ;
		// スレッド開始（スクリプトインターフェース）
		virtual const wchar_t *
			syscallBeginThread
				( ECSSakura2Processor::Context * context, INT64 addrProc ) ;
		// フレーム駆動スレッド開始（スクリプトインターフェース）
		virtual const wchar_t *
			syscallBeginFrameThread
				( ECSSakura2Processor::Context * context,
							INT64 addrProc, DWORD dwInitStack ) ;
		// フレーム駆動スレッド継続（スクリプトインターフェース）
		virtual const wchar_t *
			syscallContinueFrameThread( ECSSakura2Processor::Context * context ) ;
		// フレーム駆動スレッドへ例外スロー（スクリプトインターフェース）
		virtual const wchar_t *
			syscallThrowException
				( ECSSakura2Processor::Context * context, INT64 addrErr ) ;
		// スクリプト・インスタンス取得
		INT64 GetThreadProcedureAddress( void ) const
			{
				return	m_addrProc ;
			}
		// 実行中か？
		virtual bool IsRunning( void ) const ;
		// 待機
		virtual SError Wait( int64_t msecTimeout = Infinite ) ;

	public:	// ECSSakura2::BufferObject オーバーライド
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存準備処理
		virtual SError PrepareSave
			( VirtualMachine * vm, Context * context ) ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後の後のスクリプト処理
		virtual SError OnLoadedDynamic
			( VirtualMachine * vm, Context * context ) ;

	public:	// ECSSakura2Processor::ContextShell オーバーライド
		// 一時停止処理（ループの外側で処理する場合には errAbort を返却）
		virtual SSystem::SError OnSuspendContext( void ) ;
		// スタック拡張例外処理
		virtual DWORD HandleExceptionExtendStack( DWORD maskException ) ;

	protected:	// SSystem::SProcedure オーバーライド
		// スレッド関数
		virtual void Run( void ) ;

	public:
		// 128 bit アライメント new
		void * operator new ( size_t nBytes ) ;
		void operator delete ( void * pObj ) ;
	} ;

}

// new SSystem::Thread
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Thread) ;

// bool SSystem::Thread::IsRunning( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_IsRunning) ;

// SSystem::SError SSystem::Thread::Wait( int64_t msecTimeout = Infinite ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_Wait) ;

// SSystem::SError SSystem::Thread::BeginThread( SProcedure * pProc ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_BeginThread) ;

// SSystem::SError SSystem::Thread::BeginFrameThread
//				( SProcedure * pProc, size_t nInitStack ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_BeginFrameThread) ;

// SSystem::SError SSystem::Thread::ContinueFrameThread( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_ContinueFrameThread) ;

// SSystem::SError SSystem::Thread::ThrowException( const wchar_t * pszErrMsg = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_ThrowException) ;

// static SSystem::Thread * SSystem::Thread::GetCurrentThread( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_GetCurrentThread) ;

// static SProcedure::Thread * SSystem::Thread::GetCurrentThreadProc( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_GetCurrentThreadProc) ;

// static ESLObject * GetLocalStorageAs( const wchar_t * pwszID ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_GetLocalStorageAs) ;

// static ESLObject * SetLocalStorageAs( const wchar_t * pwszID, ESLObject * pObj ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Thread_SetLocalStorageAs) ;


#endif
