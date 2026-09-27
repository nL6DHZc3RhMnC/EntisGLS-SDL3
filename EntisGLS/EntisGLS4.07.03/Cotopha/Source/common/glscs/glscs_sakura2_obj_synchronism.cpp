
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_obj_synchronism.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// スレッド同期抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::SynchronismObject, Object )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SynchronismObject::SynchronismObject( void )
{
	m_signal.Initialize( false ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SynchronismObject::~SynchronismObject( void )
{
}

// シグナル値取得
//////////////////////////////////////////////////////////////////////////////
int64_t SynchronismObject::Value( void )
{
	int64_t	value ;
	m_mutex.Lock() ;
	value = m_value ;
	m_mutex.Unlock() ;
	return	value ;
}

// 待機
//////////////////////////////////////////////////////////////////////////////
SError SynchronismObject::Wait( Context * context, int64_t msecTimeout )
{
	if ( msecTimeout != SSynchronism::Infinite )
	{
		int64_t	msecStart = CurrentMilliSec() ;
		for ( ; ; )
		{
			if ( context->m_status != Context::xsExecution )
			{
				return	errAbort ;
			}
			m_mutex.Lock() ;
			if ( m_value > 0 )
			{
				OnSignal( context ) ;
				m_mutex.Unlock() ;
				return	errSuccess ;
			}
			m_signal.ResetSignal() ;
			m_mutex.Unlock() ;
			//
			int64_t	msecPast = CurrentMilliSec() - msecStart ;
			if ( msecPast >= msecTimeout )
			{
				break ;
			}
			int64_t	msecLeft = msecTimeout - msecPast ;
			ESLAssert( msecLeft > 0 ) ;
			if ( msecLeft > 10 )
			{
				msecLeft = 10 ;
			}
			m_signal.Wait( msecLeft ) ;
		}
	}
	else
	{
		for ( ; ; )
		{
			if ( context->m_status != Context::xsExecution )
			{
				return	errAbort ;
			}
			m_mutex.Lock() ;
			if ( m_value > 0 )
			{
				OnSignal( context ) ;
				m_mutex.Unlock() ;
				return	errSuccess ;
			}
			m_signal.ResetSignal() ;
			m_mutex.Unlock() ;
			m_signal.Wait( 10 ) ;
		}
	}
	return	errTimeout ;
}

// Wait 関数でシグナル状態を取得した時の処理
// ※m_mutex の Lock セクション内で呼び出される
//////////////////////////////////////////////////////////////////////////////
void SynchronismObject::OnSignal( Context * context )
{
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError SynchronismObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	file->Write( &m_value, sizeof(int64_t) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError SynchronismObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	file->Read( &m_value, sizeof(int64_t) ) ;
	if ( m_value > 0 )
	{
		m_signal.SetSignal() ;
	}
	else
	{
		m_signal.ResetSignal() ;
	}
	return	errSuccess ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// atomic_int_t SSystem::Synchronism::Value( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Synchronism_Value,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SynchronismObject, pSync, arg, Synchronism::Value ) ;
	//
	context->m_regset[regAcc].i = pSync->Value() ;
	return	NULL ;
}

// SError SSystem::Synchronism::Wait( int64_t msecTimeout ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Synchronism_Wait,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SynchronismObject, pSync, arg, Synchronism::Wait ) ;
	//
	context->m_regset[regAcc].i = pSync->Wait( context, arg[1].i ) ;
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// シグナルイベント同期クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::SignalEventObject, SynchronismObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SignalEventObject::SignalEventObject( bool fInitSignal )
{
	if ( fInitSignal )
	{
		m_value = 1 ;
		m_signal.SetSignal() ;
	}
	else
	{
		m_value = 0 ;
		m_signal.ResetSignal() ;
	}
}

// シグナル値設定
//////////////////////////////////////////////////////////////////////////////
void SignalEventObject::SetSignal( void )
{
	m_mutex.Lock() ;
	m_value = 1 ;
	m_signal.SetSignal() ;
	m_mutex.Unlock() ;
}

void SignalEventObject::ResetSignal( void )
{
	m_mutex.Lock() ;
	m_value = 0 ;
	m_signal.ResetSignal() ;
	m_mutex.Unlock() ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SignalEventObject::GetTypeName( void ) const
{
	return	L"SSystem::SignalEvent" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError SignalEventObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	SynchronismObject::SaveStatic( file, vm, context ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError SignalEventObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	SynchronismObject::LoadStatic( file, vm, context ) ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::SignalEvent
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_SignalEvent, context, cls_id )
{
	return	new SignalEventObject ;
}

// static SSystem::SignalEvent*
//		SSystem::SignalEvent::Create( bool fInitSignal ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_SignalEvent_Create, context, arg )
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	AssertLock() ;
	context->m_regset[regAcc].i =
		vm->AllocateHeapObjectAddress
				( new SignalEventObject( (arg[0].i != 0) ) ) ;
	AssertUnlock() ;
	return	NULL ;
}

// void SSystem::SignalEvent::SetSignal( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_SignalEvent_SetSignal, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SignalEventObject, pSync, arg, SignalEvent::SetSignal ) ;
	//
	pSync->SetSignal() ;
	return	NULL ;
}

// void SSystem::SignalEvent::ResetSignal( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_SignalEvent_ResetSignal, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SignalEventObject, pSync, arg, SignalEvent::ResetSignal ) ;
	//
	pSync->ResetSignal() ;
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// セマフォ同期クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::SemaphoreObject, SynchronismObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SemaphoreObject::SemaphoreObject( int64_t nInitCount, int64_t nMaxCount )
{
	if ( nInitCount > nMaxCount )
	{
		nInitCount = nMaxCount ;
	}
	m_value = nInitCount ;
	m_countMax = nMaxCount ;
	//
	if ( m_value > 0 )
	{
		m_signal.SetSignal() ;
	}
}

// セマフォ解放
//////////////////////////////////////////////////////////////////////////////
void SemaphoreObject::Release( void )
{
	m_mutex.Lock() ;
	if ( m_value < m_countMax )
	{
		++ m_value ;
		m_signal.SetSignal() ;
	}
	m_mutex.Unlock() ;
}

// Wait 関数でシグナル状態を取得した時の処理
// ※m_mutex の Lock セクション内で呼び出される
//////////////////////////////////////////////////////////////////////////////
void SemaphoreObject::OnSignal( Context * context )
{
	if ( m_value > 0 )
	{
		-- m_value ;
	}
	if ( m_value == 0 )
	{
		m_signal.ResetSignal() ;
	}
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SemaphoreObject::GetTypeName( void ) const
{
	return	L"SSystem::Semaphore" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError SemaphoreObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SynchronismObject::SaveStatic( file, vm, context ) ;
	file->Write( &m_countMax, sizeof(int64_t) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError SemaphoreObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SynchronismObject::LoadStatic( file, vm, context ) ;
	file->Read( &m_countMax, sizeof(int64_t) ) ;
	return	errSuccess ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Semaphore
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_Semaphore, context, cls_id )
{
	return	new SemaphoreObject ;
}

// static SSystem::Semaphore*
//		SSystem::Semaphore::Create
//		( atomic_int_t nInitCount = 1, atomic_int_t nMaxCount = 1 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Semaphore_Create, context, arg )
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	AssertLock() ;
	context->m_regset[regAcc].i =
		vm->AllocateHeapObjectAddress
				( new SemaphoreObject( arg[0].i, arg[1].i ) ) ;
	AssertUnlock() ;
	return	NULL ;
}

// void SSystem::Semaphore::Release( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Semaphore_Release, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SemaphoreObject, pSync, arg, SSemaphore::Release ) ;
	//
	pSync->Release() ;
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// ミューテックス同期クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::MutexObject, SynchronismObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
MutexObject::MutexObject( void )
{
	m_value = 1 ;
	m_signal.SetSignal() ;
	//
	m_dwOwnerThread = 0 ;
	m_dwLockedCount = 0 ;
}

// 待機
//////////////////////////////////////////////////////////////////////////////
SError MutexObject::Wait( Context * context, int64_t msecTimeout )
{
	DWORD	dwThread = 0 ;
	if ( context != NULL )
	{
		Object *	pThread = context->m_pThread ;
		if ( pThread != NULL )
		{
			dwThread = pThread->m_dwHighAddr ;
		}
	}
	m_mutex.Lock() ;
	if ( (m_dwLockedCount > 0) & (m_dwOwnerThread == dwThread) )
	{
		++ m_dwLockedCount ;
		m_mutex.Unlock() ;
		return	errSuccess ;
	}
	m_mutex.Unlock() ;
	//
	return	SynchronismObject::Wait( context, msecTimeout ) ;
}

// ミューテックス解放
//////////////////////////////////////////////////////////////////////////////
void MutexObject::Release( Context * context )
{
	DWORD	dwThread = 0 ;
	if ( context != NULL )
	{
		Object *	pThread = context->m_pThread ;
		if ( pThread != NULL )
		{
			dwThread = pThread->m_dwHighAddr ;
		}
	}
	m_mutex.Lock() ;
	if ( m_dwOwnerThread == dwThread )
	{
		if ( m_dwLockedCount > 0 )
		{
			m_dwLockedCount -- ;
		}
		if ( m_dwLockedCount == 0 )
		{
			m_dwOwnerThread = 0 ;
			m_value = 1 ;
			m_signal.SetSignal() ;
		}
	}
	m_mutex.Unlock() ;
}

// Wait 関数でシグナル状態を取得した時の処理
// ※m_mutex の Lock セクション内で呼び出される
//////////////////////////////////////////////////////////////////////////////
void MutexObject::OnSignal( Context * context )
{
	DWORD	dwThread = 0 ;
	if ( context != NULL )
	{
		Object *	pThread = context->m_pThread ;
		if ( pThread != NULL )
		{
			dwThread = pThread->m_dwHighAddr ;
		}
	}
	m_dwOwnerThread = dwThread ;
	m_dwLockedCount = 1 ;
	m_value = 0 ;
	m_signal.ResetSignal() ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * MutexObject::GetTypeName( void ) const
{
	return	L"SSystem::Mutex" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError MutexObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SynchronismObject::SaveStatic( file, vm, context ) ;
	file->Write( &m_dwOwnerThread, sizeof(DWORD) ) ;
	file->Write( &m_dwLockedCount, sizeof(DWORD) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError MutexObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	SynchronismObject::LoadStatic( file, vm, context ) ;
	file->Read( &m_dwOwnerThread, sizeof(DWORD) ) ;
	file->Read( &m_dwLockedCount, sizeof(DWORD) ) ;
	return	errSuccess ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Mutex
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_Mutex, context, cls_id )
{
	return	new MutexObject ;
}

// static SSystem::Mutex * SSystem::Mutex::Create( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Mutex_Create, context, arg )
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	AssertLock() ;
	context->m_regset[regAcc].i =
		vm->AllocateHeapObjectAddress( new MutexObject ) ;
	AssertUnlock() ;
	return	NULL ;
}

// void SSystem::Mutex::Release( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SSystem_Mutex_Release, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, MutexObject, pSync, arg, Mutex::Release ) ;
	//
	pSync->Release( context ) ;
	return	NULL ;
}

#endif
