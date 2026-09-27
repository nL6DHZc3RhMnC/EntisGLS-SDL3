
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_SYNCHRONISM_H__)
#define	__GLSCS_SAKURA2_OBJECT_SYNCHRONISM_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// スレッド同期抽象オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SynchronismObject	: public Object
	{
	protected:
		SSystem::SSignalEvent		m_signal ;
		SSystem::SCriticalSection	m_mutex ;
		int64_t						m_value ;	// シグナル値（0が非シグナル）

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ECSSakura2::SynchronismObject, Object )
		// 構築関数
		SynchronismObject( void ) ;
		// 消滅関数
		virtual ~SynchronismObject( void ) ;

	public:
		// シグナル値取得
		virtual int64_t Value( void ) ;
		// 待機
		virtual SError Wait( Context * context, int64_t msecTimeout ) ;

	protected:
		// Wait 関数でシグナル状態を取得した時の処理
		// ※m_mutex の Lock セクション内で呼び出される
		virtual void OnSignal( Context * context ) ;

	public:
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// シグナルイベント同期クラス
	//////////////////////////////////////////////////////////////////////////

	class	SignalEventObject	: public SynchronismObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( ECSSakura2::SignalEventObject, SynchronismObject )
		// 構築関数
		SignalEventObject( bool fInitSignal = false ) ;
		// シグナル値設定
		virtual void SetSignal( void ) ;
		virtual void ResetSignal( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// セマフォ同期クラス
	//////////////////////////////////////////////////////////////////////////

	class	SemaphoreObject	: public SynchronismObject
	{
	protected:
		int64_t	m_countMax ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( ECSSakura2::SemaphoreObject, SynchronismObject )
		// 構築関数
		SemaphoreObject
			( int64_t nInitCount = 1, int64_t nMaxCount = 1 ) ;
		// セマフォ解放
		virtual void Release( void ) ;

	protected:
		// Wait 関数でシグナル状態を取得した時の処理
		// ※m_mutex の Lock セクション内で呼び出される
		virtual void OnSignal( Context * context ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ミューテックス同期クラス
	//////////////////////////////////////////////////////////////////////////

	class	MutexObject	: public SynchronismObject
	{
	protected:
		DWORD	m_dwOwnerThread ;
		DWORD	m_dwLockedCount ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( ECSSakura2::MutexObject, SynchronismObject )
		// 構築関数
		MutexObject( void ) ;
		// 待機
		virtual SError Wait( Context * context, int64_t msecTimeout ) ;
		// ミューテックス解放
		virtual void Release( Context * context ) ;

	protected:
		// Wait 関数でシグナル状態を取得した時の処理
		// ※m_mutex の Lock セクション内で呼び出される
		virtual void OnSignal( Context * context ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
	} ;

}

// atomic_int_t SSystem::Synchronism::Value( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Synchronism_Value) ;
// SError SSystem::Synchronism::Wait( int64_t msecTimeout ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Synchronism_Wait) ;

// new SSystem::SignalEvent
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_SignalEvent) ;
// static SSystem::SignalEvent*
//		SSystem::SignalEvent::Create( bool fInitSignal ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_SignalEvent_Create) ;
// void SSystem::SignalEvent::SetSignal( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_SignalEvent_SetSignal) ;
// void SSystem::SignalEvent::ResetSignal( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_SignalEvent_ResetSignal) ;

// new SSystem::Semaphore
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Semaphore) ;
// static SSystem::Semaphore*
//		SSystem::Semaphore::Create
//		( atomic_int_t nInitCount = 1, atomic_int_t nMaxCount = 1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Semaphore_Create) ;
// void SSystem::Semaphore::Release( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Semaphore_Release) ;

// new SSystem::Mutex
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Mutex) ;
// static SSystem::Mutex * SSystem::Mutex::Create( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Mutex_Create) ;
// void SSystem::Mutex::Release( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Mutex_Release) ;


#endif
