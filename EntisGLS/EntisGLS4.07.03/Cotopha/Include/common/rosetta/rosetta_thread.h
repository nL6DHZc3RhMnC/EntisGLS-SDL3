
#if	!defined(__ROSETTA_THREAD_H__)
#define	__ROSETTA_THREAD_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// Runnable クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSRunnableClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSRunnableClass, RSClass )
		// 構築関数
		RSRunnableClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Runnable" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Thread オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSThread	: public RSObject, public SSystem::SProcedure
	{
	public:
		RSThread *			m_pChainPrev ;
		RSThread *			m_pChainNext ;

		RSVirtualMachine *	m_pVM ;
		RSContext *			m_pContext ;
		RSObject *			m_pRunnable ;
		SSystem::SThread	m_thread ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( RSThread, RSObject, SProcedure )
		// 構築関数
		RSThread
			( RSVirtualMachine * pVM,
				RSObject * pRunnable, RSClass * pClass ) ;
		// 消滅関数
		virtual ~RSThread( void ) ;

	public:
		// オブジェクト解放処理
		virtual void Finalize( RSContext& context ) ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// Runnable オブジェクト関連付け
		void AttachRunnable( RSObject * pRunnable ) ;
		// スレッド開始
		void StartThread( void ) ;
		// スレッド強制終了
		bool AbortThread( void ) ;

	public:
		// スレッド関数
		virtual void Run( void ) ;
		// 開始前の処理
		virtual void Prepare( void ) ;
		// 完了後の処理
		virtual void Finalize( void ) ;

	public:
		// SSystem::SProcedure -> Runnable
		class	RunnableProcedure	: public SSystem::SProcedure
		{
		protected:
			RSVirtualMachine *	m_pVM ;
			RSThread *			m_pThread ;
			RSObject *			m_pRunnable ;
			bool				m_fAutoDelete ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RunnableProcedure, SProcedure )
			// 構築関数
			RunnableProcedure
				( RSVirtualMachine * pVM,
					RSThread * pThread,
					RSObject * pRunnable, bool fAutoDeleteProc ) ;
			// 実行関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Thread クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSThreadClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSThreadClass, RSClass )
		// 構築関数
		RSThreadClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Thread" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// void <init>( Runnable target )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void start()
		static RSObject * method_start
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void run()
		static RSObject * method_run
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void join( long millis )
		static RSObject * method_join
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Thread currentThread()
		static RSObject * method_currentThread
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static void sleep( long millis )
		static RSObject * method_sleep
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static void dumpStack()
		static RSObject * method_dumpStack
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;

}

#endif

