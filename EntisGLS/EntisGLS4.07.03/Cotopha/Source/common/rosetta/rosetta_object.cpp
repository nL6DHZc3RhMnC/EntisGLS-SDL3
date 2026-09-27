
#include <sakura/sakura.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// 基底オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSObject, SObject )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSObject::Monitor, SSignalEvent )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSObject::~RSObject( void )
{
	ESLAssert( m_pSynchronized == NULL ) ;
	ESLAssert( m_pFirstMonitor == NULL ) ;
}

// 排他同期取得
//////////////////////////////////////////////////////////////////////////////
SError RSObject::LockSynchronized
		( RSObject::Synchronized * sync, RSContext * context )
{
	for ( ; ; )
	{
		QuickLock() ;
		if ( m_pSynchronized != NULL )
		{
			if ( m_pSynchronized->pContext == context )
			{
				sync = m_pSynchronized ;
				sync->countSync ++ ;
				QuickUnlock() ;
				break ;
			}
			else
			{
				Monitor	monitor( Monitor::typeSynchronized ) ;
				monitor.Initialize( false ) ;
				AddMonitor( &monitor ) ;
				QuickUnlock() ;
				//
				SError	err = monitor.Wait() ;
				if ( err )
				{
					DetachMonitor( &monitor ) ;
					return	err ;
				}
			}
		}
		else
		{
			m_pSynchronized = sync ;
			sync->pContext = context ;
			sync->countSync = 1 ;
			QuickUnlock() ;
			break ;
		}
	}
	return	errSuccess ;
}

// 排他同期解放
//////////////////////////////////////////////////////////////////////////////
bool RSObject::UnlockSynchronized( RSContext * context )
{
	bool	fUnlocked = false ;
	QuickLock() ;
	Synchronized *	sync = m_pSynchronized ;
	if ( (sync != NULL) && (sync->pContext == context) )
	{
		if ( (-- (sync->countSync)) <= 0 )
		{
			m_pSynchronized = NULL ;
			fUnlocked = true ;
			NotifyAllMonitor( Monitor::typeSynchronized ) ;
		}
	}
	QuickUnlock() ;
	return	fUnlocked ;
}

// モニタ待機
//////////////////////////////////////////////////////////////////////////////
SError RSObject::WaitNotification( RSContext& context, int64_t timeout )
{
	QuickLock() ;
	if ( (m_pSynchronized != NULL)
		&& (m_pSynchronized->pContext == &context) )
	{
		if ( timeout >= 0 )
		{
			//
			// 一時的に排他同期解放
			//
			Synchronized *	sync = m_pSynchronized ;
			m_pSynchronized = NULL ;
			NotifyAllMonitor( Monitor::typeSynchronized ) ;
			//
			// 待機
			//
			Monitor	monNotify( Monitor::typeNotification ) ;
			monNotify.Initialize( false ) ;
			AddMonitor( &monNotify ) ;
			QuickUnlock() ;
			//
			SError	err ;
			if ( timeout == 0 )
			{
				err = context.WaitSynchronism
							( monNotify, SSynchronism::Infinite ) ;
			}
			else
			{
				err = context.WaitSynchronism( monNotify, timeout ) ;
			}
			DetachMonitor( &monNotify ) ;
			//
			// 排他同期を再確保
			//
			for ( ; ; )
			{
				QuickLock() ;
				if ( m_pSynchronized == NULL )
				{
					m_pSynchronized = sync ;
					QuickUnlock() ;
					break ;
				}
				else
				{
					Monitor	monSync( Monitor::typeSynchronized ) ;
					monSync.Initialize( false ) ;
					AddMonitor( &monSync ) ;
					QuickUnlock() ;
					//
					context.WaitSynchronism
							( monSync, SSynchronism::Infinite ) ;
					//
					DetachMonitor( &monSync ) ;
				}
			}
			return	err ;
		}
		else
		{
			QuickUnlock() ;
			context.ThrowExceptionError
				( L"タイムアウト時間が負数です",
							L"IllegalArgumentException" ) ;
			return	errFailed ;
		}
	}
	else
	{
		QuickUnlock() ;
		context.ThrowExceptionError
			( L"オブジェクトモニタを所有していません",
							L"IllegalMonitorStateException" ) ;
		return	errFailed ;
	}
}

// モニタ追加
//////////////////////////////////////////////////////////////////////////////
void RSObject::AddMonitor( RSObject::Monitor * monitor )
{
	QuickLock() ;
	monitor->m_pNextChain = m_pFirstMonitor ;
	m_pFirstMonitor = monitor ;
	QuickUnlock() ;
}

// モニタ分離
//////////////////////////////////////////////////////////////////////////////
void RSObject::DetachMonitor( Monitor * monitor )
{
	QuickLock() ;
	Monitor *	pLast = m_pFirstMonitor ;
	Monitor *	pPrev = NULL ;
	while ( pLast != NULL )
	{
		Monitor *	pNext = pLast->m_pNextChain ;
		if ( pLast == monitor )
		{
			pLast->m_pNextChain = NULL ;
			pLast->SetSignal() ;
			break ;
		}
		else
		{
			if ( pPrev != NULL )
			{
				pPrev->m_pNextChain = pLast ;
			}
			else
			{
				m_pFirstMonitor = pLast ;
			}
			pPrev = pLast ;
		}
		pLast = pNext ;
	}
	if ( pPrev != NULL )
	{
		pPrev->m_pNextChain = NULL ;
	}
	else
	{
		m_pFirstMonitor = NULL ;
	}
	QuickUnlock() ;
}

// モニタ通知と分離
//////////////////////////////////////////////////////////////////////////////
void RSObject::NotifyMonitor( RSObject::Monitor::Type typeNotify )
{
	QuickLock() ;
	Monitor *	pLast = m_pFirstMonitor ;
	Monitor *	pPrev = NULL ;
	while ( pLast != NULL )
	{
		Monitor *	pNext = pLast->m_pNextChain ;
		if ( pLast->m_type == typeNotify )
		{
			pLast->m_pNextChain = NULL ;
			pLast->SetSignal() ;
			break ;
		}
		else
		{
			if ( pPrev != NULL )
			{
				pPrev->m_pNextChain = pLast ;
			}
			else
			{
				m_pFirstMonitor = pLast ;
			}
			pPrev = pLast ;
		}
		pLast = pNext ;
	}
	if ( pPrev != NULL )
	{
		pPrev->m_pNextChain = NULL ;
	}
	else
	{
		m_pFirstMonitor = NULL ;
	}
	QuickUnlock() ;
}

void RSObject::NotifyAllMonitor( RSObject::Monitor::Type typeNotify )
{
	QuickLock() ;
	Monitor *	pLast = m_pFirstMonitor ;
	Monitor *	pPrev = NULL ;
	m_pFirstMonitor = NULL ;
	while ( pLast != NULL )
	{
		Monitor *	pNext = pLast->m_pNextChain ;
		if ( pLast->m_type == typeNotify )
		{
			pLast->m_pNextChain = NULL ;
			pLast->SetSignal() ;
		}
		else
		{
			if ( pPrev != NULL )
			{
				pPrev->m_pNextChain = pLast ;
			}
			else
			{
				m_pFirstMonitor = pLast ;
			}
			pPrev = pLast ;
		}
		pLast = pNext ;
	}
	if ( pPrev != NULL )
	{
		pPrev->m_pNextChain = NULL ;
	}
	QuickUnlock() ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSObject::GetTypeName( void ) const
{
	if ( m_pClass != NULL )
	{
		return	m_pClass->GetRSClassName() ;
	}
	return	NULL ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::InstanceOf( const wchar_t * pwszType )
{
	if ( m_pClass != NULL )
	{
		if ( m_pClass->IsInstanceOf( pwszType ) )
		{
			return	this ;
		}
	}
	return	NULL ;
}

RSObject * RSObject::InstanceOf( RSClass * pClass )
{
	if ( m_pClass != NULL )
	{
		if ( pClass == m_pClass )
		{
			return	this ;
		}
		if ( m_pClass->IsInstanceOf( pClass ) )
		{
			return	this ;
		}
	}
	return	NULL ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSObject::IsIntegerType( void ) const
{
	return	false ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSObject::IsFloatType( void ) const
{
	return	false ;
}

// 文字列型か？
//////////////////////////////////////////////////////////////////////////////
bool RSObject::IsStringType( void ) const
{
	return	false ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSObject::IsObjectType( void ) const
{
	return	true ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSObject::AsInteger( int64_t& number ) const
{
	number = 0 ;
	return	false ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSObject::AsRealNumber( double& number ) const
{
	number = 0 ;
	return	false ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSObject::AsBoolean( void ) const
{
	return	true ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSObject::AsString( SSystem::SString& strValue ) const
{
	strValue = L"<abstract object>" ;
	return	false ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSObject::IsEqualObject( RSObject * pObj ) const
{
	if ( pObj != NULL )
	{
		pObj = pObj->GetEntityObject() ;
	}
	return	(this == pObj) ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSObject::ToDebugDump
	( SSystem::SFileInterface& dump,
		size_t nPtrNest, const wchar_t * pwszIndent )
{
	SString	str ;
	if ( AsString( str ) )
	{
		dump.WriteEncodedString( str ) ;
	}
}

// 値設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSObject::SetIntegerAs( int64_t nValue )
{
	return	errFailed ;
}

SSystem::SError RSObject::SetNumberAs( double nValue )
{
	return	errFailed ;
}

SSystem::SError RSObject::SetStringAs( const wchar_t * pwszValue )
{
	return	errFailed ;
}

// 配列要素取得
//////////////////////////////////////////////////////////////////////////////
int64_t RSObject::GetElementIntegerAt
	( RSContext& context, int nIndex, int64_t nDefault, bool * pError )
{
	int64_t		num = nDefault ;
	bool		fError = true ;
	RSObject *	pObj = GetElementAt( context, nIndex ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetEntityObject() != NULL )
		{
			fError = !(pObj->AsInteger( num )) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	if ( pError != NULL )
	{
		*pError = fError ;
	}
	return	num ;
}

double RSObject::GetElementNumberAt
	( RSContext& context, int nIndex, double nDefault, bool * pError )
{
	double		num = nDefault ;
	bool		fError = true ;
	RSObject *	pObj = GetElementAt( context, nIndex ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetEntityObject() != NULL )
		{
			fError = !(pObj->AsRealNumber( num )) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	if ( pError != NULL )
	{
		*pError = fError ;
	}
	return	num ;
}

SSystem::SString RSObject::GetElementStringAt
	( RSContext& context, int nIndex,
			const wchar_t * pwszDefault, bool * pError )
{
	SString		str = pwszDefault ;
	bool		fError = true ;
	RSObject *	pObj = GetElementAt( context, nIndex ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetEntityObject() != NULL )
		{
			fError = !(pObj->AsString( str )) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	if ( pError != NULL )
	{
		*pError = fError ;
	}
	return	str ;
}

// 配列要素設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSObject::SetElementIntegerAt
	( RSContext& context, int nIndex, int64_t nValue )
{
	context.ReleaseObjectRef
		( SetElementAt
			( context, nIndex, context.new_Integer( nValue ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSObject::SetElementNumberAt
	( RSContext& context, int nIndex, double nValue )
{
	context.ReleaseObjectRef
		( SetElementAt
		( context, nIndex, context.new_Number( nValue ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSObject::SetElementStringAt
	( RSContext& context, int nIndex, const wchar_t * pwszValue )
{
	context.ReleaseObjectRef
		( SetElementAt
		( context, nIndex, context.new_String( pwszValue ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// メンバ要素取得
//////////////////////////////////////////////////////////////////////////////
int64_t RSObject::GetMemberIntegerAs
	( RSContext& context,
		const wchar_t * pwszMember, int64_t nDefault, bool * pError )
{
	int64_t		num = nDefault ;
	bool		fError = true ;
	RSObject *	pObj = GetMemberAs( context, pwszMember ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetEntityObject() != NULL )
		{
			fError = !(pObj->AsInteger( num )) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	if ( pError != NULL )
	{
		*pError = fError ;
	}
	return	num ;
}

double RSObject::GetMemberNumberAs
	( RSContext& context,
		const wchar_t * pwszMember, double nDefault, bool * pError )
{
	double		num = nDefault ;
	bool		fError = true ;
	RSObject *	pObj = GetMemberAs( context, pwszMember ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetEntityObject() != NULL )
		{
			fError = !(pObj->AsRealNumber( num )) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	if ( pError != NULL )
	{
		*pError = fError ;
	}
	return	num ;
}

SSystem::SString RSObject::GetMemberStringAs
	( RSContext& context,
		const wchar_t * pwszMember,
		const wchar_t * pwszDefault, bool * pError )
{
	SString		str = pwszDefault ;
	bool		fError = true ;
	RSObject *	pObj = GetMemberAs( context, pwszMember ) ;
	if ( pObj != NULL )
	{
		if ( pObj->GetEntityObject() != NULL )
		{
			fError = !(pObj->AsString( str )) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	if ( pError != NULL )
	{
		*pError = fError ;
	}
	return	str ;
}

SSystem::SObject * RSObject::GetMemberNativeObjAs
	( RSContext& context, const wchar_t * pwszMember, bool * pError )
{
	SSystem::SObject *	pSObj = NULL ;
	bool				fError = true ;
	RSObject *			pObj = GetMemberAs( context, pwszMember ) ;
	if ( pObj != NULL )
	{
		RSNativeObject *	pNativeObj =
			ESLTypeCast<RSNativeObject>( pObj->GetEntityObject() ) ;
		if ( pNativeObj != NULL )
		{
			pSObj = pNativeObj->GetObject() ;
			fError = false ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	if ( pError != NULL )
	{
		*pError = fError ;
	}
	return	pSObj ;
}

uint8_t * RSObject::GetMemberNativePtrAs
	( RSContext& context, const wchar_t * pwszMember, size_t nReqBytes )
{
	uint8_t *	ptrBuf = NULL ;
	RSObject *	pObj = GetMemberAs( context, pwszMember ) ;
	if ( pObj != NULL )
	{
		RSTypedArrayPointer *	pTypedPtr =
			ESLTypeCast<RSTypedArrayPointer>( pObj->GetEntityObject() ) ;
		if ( pTypedPtr != NULL )
		{
			ptrBuf = pTypedPtr->GetPointer( nReqBytes ) ;
		}
		context.ReleaseObjectRef( pObj ) ;
	}
	return	ptrBuf ;
}

// メンバ要素設定
//////////////////////////////////////////////////////////////////////////////
SError RSObject::SetMemberIntegerAs
	( RSContext& context, const wchar_t * pwszMember, int64_t nValue )
{
	context.ReleaseObjectRef
		( SetMemberAs
			( context, pwszMember, context.new_Integer( nValue ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SError RSObject::SetMemberNumberAs
	( RSContext& context, const wchar_t * pwszMember, double nValue )
{
	context.ReleaseObjectRef
		( SetMemberAs
			( context, pwszMember, context.new_Number( nValue ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SError RSObject::SetMemberStringAs
	( RSContext& context,
		const wchar_t * pwszMember, const wchar_t * pwszValue )
{
	context.ReleaseObjectRef
		( SetMemberAs
			( context, pwszMember, context.new_String( pwszValue ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSObject::SetMemberNativeObjRefAs
	( RSContext& context,
		const wchar_t * pwszMember,
		SSystem::SObject * pRef, RSClass * pClass )
{
	context.ReleaseObjectRef
		( SetMemberAs
			( context, pwszMember, new RSNativeObject( pRef, pClass ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSObject::SetMemberNativePtrRefAs
	( RSContext& context,
		const wchar_t * pwszMember,
		uint8_t * ptrBuf, size_t nBytes,
		RSPrimitiveNumberType type )
{
	RSArrayBuffer *
		pBuf = new RSArrayBuffer( context.GetArrayBufferClass() ) ;
	pBuf->AttachBuffer( ptrBuf, nBytes ) ;
	//
	context.ReleaseObjectRef
		( SetMemberAs
			( context, pwszMember,
				context.new_PointerNumber
					( pBuf, RSReferenceNumber::FromPrimitiveType(type) ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSObject::SetMemberStructPtrRefAs
	( RSContext& context,
		const wchar_t * pwszMember,
		uint8_t * ptrBuf, size_t nBytes,
		RSStructuredPointerClass * pType )
{
	RSArrayBuffer *
		pBuf = new RSArrayBuffer( context.GetArrayBufferClass() ) ;
	pBuf->AttachBuffer( ptrBuf, nBytes ) ;
	//
	context.ReleaseObjectRef
		( SetMemberAs
			( context, pwszMember, context.new_StructuredPointer( pType, pBuf ) ) ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// メンバ要素作成
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSObject::CreateMemberIntegerAs
	( RSContext& context,
		const wchar_t * pwszMember, int64_t nValue, uint32_t accMod )
{
	RSObject *	pObj =
		CreateMemberAs
			( context, pwszMember, context.new_Integer( nValue ) ) ;
	pObj->SetModifiers( accMod ) ;
	context.ReleaseObjectRef( pObj ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSObject::CreateMemberNumberAs
	( RSContext& context,
		const wchar_t * pwszMember, double nValue, uint32_t accMod )
{
	RSObject *	pObj =
		CreateMemberAs
			( context, pwszMember, context.new_Number( nValue ) ) ;
	pObj->SetModifiers( accMod ) ;
	context.ReleaseObjectRef( pObj ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSObject::CreateMemberStringAs
	( RSContext& context,
		const wchar_t * pwszMember, const wchar_t * pwszValue, uint32_t accMod )
{
	RSObject *	pObj =
		CreateMemberAs
			( context, pwszMember,
				context.new_Pointer
					( context.new_String( pwszValue ),
							context.GetStringClass() ) ) ;
	pObj->SetModifiers( accMod ) ;
	context.ReleaseObjectRef( pObj ) ;
	if ( context.IsException() )
	{
		context.ClearException() ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// メンバ全削除
//////////////////////////////////////////////////////////////////////////////
void RSObject::RemoveAllMembers( RSContext& context )
{
	size_t	nCount = GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		context.ReleaseObjectRef
			( RemoveMemberAs
				( context, GetElementNameAt( (int) (nCount - i - 1) ) ) ) ;
		if ( context.IsException() )
		{
			context.ClearException() ;
			break ;
		}
	}
}

// オブジェクト解放処理
//////////////////////////////////////////////////////////////////////////////
void RSObject::Finalize( RSContext& context )
{
	// ※Finalize を呼ぶ前には必ず AddRef を呼び出しておく
	// 　なぜなら、finalize 関数内部や DisposeObject 関数内部、
	// 　あるいはネイティブ側が SReference でこのオブジェクトを参照している場合
	// 　これから削除する（不可避）にも拘わらず、AddRef することができる。
	// 　m_countRef==0 で不用意に AddRef された場合、その処理終了時の
	// 　ReleaseRef では再びこのオブジェクトを削除しようとしてしまう。
	ESLAssert( m_countRef == 1 ) ;

	if ( m_pClass != nullptr )
	{
		RSFunctionObject *	pDestructor = m_pClass->GetDestructor() ;
		if ( (pDestructor != nullptr)
			&& (pDestructor->m_pPrototype != nullptr)
			&& (pDestructor->m_pPrototype->m_methodNative.pfnMethod
								!= RSGenricObjectClass::method_finalize) )
		{
			AddRef() ;
			context.ReleaseObjectRef
				( context.CallFunction( *pDestructor, this, nullptr, 0, false ) ) ;
		}
	}
	DisposeObject( context ) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSObject::DisposeObject( RSContext& context )
{
	m_accModifier = modifierPublic ;
	m_pComment = NULL ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::DuplicateObject( RSContext& context ) const
{
	return	CloneObject( context ) ;
}

// 実体
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::GetEntityObject( void ) const
{
	return	(RSObject*) this ;
}

// 実体型
//////////////////////////////////////////////////////////////////////////////
RSClass * RSObject::GetEntityClass( void ) const
{
	RSObject *	pObj = GetEntityObject() ;
	if ( pObj != NULL )
	{
		return	pObj->m_pClass ;
	}
	return	m_pClass ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::GetElementAt( RSContext& context, int nIndex ) const
{
	return	NULL ;
}

// 要素名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSObject::GetElementNameAt( int nIndex ) const
{
	return	NULL ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::SetElementAt
	( RSContext& context, int nIndex, RSObject * pObj )
{
	context.ReleaseObjectRef( pObj ) ;
	return	NULL ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSObject::GetElementCount( void ) const
{
	return	0 ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSObject::GetElementLimit( void ) const
{
	return	0 ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	return	NULL ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	context.ReleaseObjectRef( pObj ) ;
	context.ThrowExceptionError( L"メンバの変更は禁止されています" ) ;
	return	NULL ;
}

// メンバ新規作成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::CreateMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	context.ReleaseObjectRef( pObj ) ;
	context.ThrowExceptionError( L"メンバの作成は禁止されています" ) ;
	return	NULL ;
}

// メンバ削除
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::RemoveMemberAs
	( RSContext& context, const wchar_t * pwszName )
{
	context.ThrowExceptionError( L"メンバの削除は禁止されています" ) ;
	return	NULL ;
}

// メソッド取得
//////////////////////////////////////////////////////////////////////////////
RSObject::METHOD_ENTRY *
	RSObject::GetMethodAs
		( RSContext& context,
			const wchar_t * pwszName, RSObject::METHOD_ENTRY& method ) const
{
	if ( m_pClass != NULL )
	{
		RSFunctionObject *	pFuncObj =
			m_pClass->GetVirtualMemberAs( context, pwszName ) ;
		if ( pFuncObj != NULL )
		{
			method.pfnMethod = &RSObject::methodRuntimeFunction ;
			method.pInstance = pFuncObj ;
			return	&method ;
		}
	}
	return	NULL ;
}

// メソッド呼び出し
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::CallMethodAs
	( RSContext& context,
		const wchar_t * pwszFuncName,
		RSObject*const* ppArgs, size_t countArg,
		bool fStructCast, bool* pArgMatchResult )
{
	return	context.CallMethod
		( this, pwszFuncName, ppArgs, countArg, fStructCast, pArgMatchResult ) ;
}

// スクリプト上のメソッド呼び出し関数
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::methodRuntimeFunction
	( RSContext& context, void * pInstance,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSFunctionObject *	pFuncObj = (RSFunctionObject*) pInstance ;
	ESLAssert( pFuncObj != NULL ) ;
	return	context.CallFunction( *pFuncObj, pThis, ppArg, count, false ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::OperatorPlus( RSContext& context ) const
{
	context.ThrowExceptionError( L"未定義の前置 + 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorNegate( RSContext& context ) const
{
	context.ThrowExceptionError( L"未定義の前置 - 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorBitNot( RSContext& context ) const
{
	context.ThrowExceptionError( L"未定義の前置 ~ 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorIncrement( RSContext& context )
{
	context.ThrowExceptionError( L"未定義のインクリメント演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorDecrement( RSContext& context )
{
	context.ThrowExceptionError( L"未定義のデクリメント演算子です" ) ;
	return	NULL ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::OperatorMul( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の * 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorDiv( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の / 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorMod( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の % 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の + 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の - 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorShiftLeft( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の << 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorShiftRight( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の >> 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の >>> 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorBitAnd( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の & 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorBitOr( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の | 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorBitXor( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の ^ 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	RSObject *	pEntity = (pObj != NULL) ? pObj->GetEntityObject() : NULL ;
	return	context.new_Boolean( this == pEntity ) ;
}

RSObject * RSObject::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	RSObject *	pEntity = (pObj != NULL) ? pObj->GetEntityObject() : NULL ;
	return	context.new_Boolean( this != pEntity ) ;
}

RSObject * RSObject::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の >= 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の > 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の <= 演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	context.ThrowExceptionError( L"未定義の < 演算子です" ) ;
	return	NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::OperatorMove( RSContext& context, RSObject * pObj )
{
	context.ThrowExceptionError( L"未定義の代入演算子です" ) ;
	return	NULL ;
}

RSObject * RSObject::OperatorMoveMul( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorMul( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveDiv( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorDiv( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveMod( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorMod( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveAdd( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorAdd( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveSub( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorSub( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveShiftLeft( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorShiftLeft( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveShiftRight( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorShiftRight( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveBitShiftRight( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorBitShiftRight( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveBitAnd( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorBitAnd( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveBitOr( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorBitOr( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

RSObject * RSObject::OperatorMoveBitXor( RSContext& context, RSObject * pObj )
{
	RSObject *	pRObj = OperatorBitXor( context, pObj ) ;
	RSObject *	pMoved = NULL ;
	if ( pRObj != NULL )
	{
		pMoved = OperatorMove( context, pRObj ) ;
		context.ReleaseObjectRef( pRObj ) ;
	}
	return	pMoved ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
RSObject * RSObject::SerializeObject( RSContext& context )
{
	return	NULL ;
}

SSystem::SError RSObject::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	errNotSupported ;
}

SSystem::SError RSObject::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	return	errNotSupported ;
}

SSystem::SError RSObject::SaveObjectBinary
	( RSObject * pObj, RSContext& context, SSystem::SFileInterface& file )
{
	int64_t		fposBytes = file.GetPosition() ;
	uint32_t	nBytes = 0 ;
	file.Write( &nBytes, sizeof(uint32_t) ) ;
	//
	int64_t		fposStart = file.GetPosition() ;
	SError		err = SaveObjectDirectBinary( pObj, context, file ) ;
	if ( err )
	{
		file.Seek( fposStart ) ;
		return	err ;
	}
	int64_t		fposEnd = file.GetPosition() ;
	nBytes = (uint32_t) (fposEnd - fposStart) ;
	file.Seek( fposBytes ) ;
	file.Write( &nBytes, sizeof(uint32_t) ) ;
	file.Seek( fposEnd ) ;
	return	err ;
}

SSystem::SError RSObject::SaveObjectDirectBinary
	( RSObject * pObj, RSContext& context, SSystem::SFileInterface& file )
{
	int8_t	typeObj = typeInvalid ;
	if ( pObj == NULL )
	{
		if ( file.Write( &typeObj, sizeof(int8_t) ) < sizeof(int8_t) )
		{
			return	errFailed ;
		}
		return	errSuccess ;
	}
	SString	strType ;
	typeObj = (int8_t) (pObj->m_typeObj) ;
	switch ( typeObj )
	{
	case	typeNumber:
	case	typeInteger:
	case	typeBoolean:
	case	typeString:
	case	typeArray:
	case	typeReference:
	case	typePointer:
		break ;
	default:
		typeObj = typeObject ;
		if ( context.GetDynamicObjectClass() != pObj->m_pClass )
		{
			if ( pObj->m_pClass != NULL )
			{
				strType = pObj->m_pClass->GetRSClassName() ;
				typeObj = typeGenericObject ;
			}
		}
		break ;
	}
	if ( file.Write( &typeObj, sizeof(int8_t) ) < sizeof(int8_t) )
	{
		return	errFailed ;
	}
	if ( typeObj == typeGenericObject )
	{
		if ( file.WriteString( strType ) )
		{
			return	errFailed ;
		}
	}
	return	pObj->SerializeBinary( context, file ) ;
}

SSystem::SError RSObject::MakeXMLDocumentOfObject
	( RSObject * pObj, RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	if ( pObj == NULL )
	{
		xmlDoc.SetTag( L"null" ) ;
		return	errSuccess ;
	}
	xmlDoc.SetTag( pObj->GetTypeName() ) ;
	return	pObj->MakeXMLDocument( context, xmlDoc ) ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSObject::RestoreObject( RSContext& context, RSObject * pObj )
{
	return	errSuccess ;
}

SSystem::SError RSObject::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	errNotSupported ;
}

SSystem::SError RSObject::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	return	errNotSupported ;
}

RSObject * RSObject::LoadObjectBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	uint32_t	nBytes = 0 ;
	if ( file.Read( &nBytes, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	NULL ;
	}
	int64_t		fpos = file.GetPosition() ;
	RSObject *	pObj = LoadObjectDirectBinary( context, file ) ;
	file.Seek( fpos + nBytes ) ;
	return	pObj ;
}

RSObject * RSObject::LoadObjectDirectBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	int8_t	typeObj = typeInvalid ;
	if ( file.Read( &typeObj, sizeof(int8_t) ) < sizeof(int8_t) )
	{
		return	NULL ;
	}
	RSObject *	pObj = NULL ;
	RSClass *	pClass = NULL ;
	SString		strType ;
	RSObject *	pArg = NULL ;
	switch ( typeObj )
	{
	case	typeNumber:
		pObj = context.new_Number( 0.0 ) ;
		break ;
	case	typeInteger:
		pObj = context.new_Integer( 0 ) ;
		break ;
	case	typeBoolean:
		pObj = context.new_Boolean( false ) ;
		break ;
	case	typeString:
		pObj = context.new_String( NULL ) ;
		break ;
	case	typeArray:
		pObj = context.new_Array() ;
		break ;
	case	typeReference:
		pObj = context.new_Reference( NULL ) ;
		break ;
	case	typePointer:
		pObj = context.new_Pointer( NULL ) ;
		break ;
	case	typeObject:
		pObj = new RSDynamicObject( context.GetDynamicObjectClass() ) ;
		break ;
	default:
		if ( file.ReadString( strType ) )
		{
			return	NULL ;
		}
		pClass = context.GetClassAs( strType ) ;
		if ( pClass == NULL )
		{
			return	NULL ;
		}
		pArg = context.new_Array() ;
		pObj = pClass->NewInstance( context, pArg ) ;
		context.ReleaseObjectRef( pArg ) ;
		break ;
	}
	if ( pObj != NULL )
	{
		pObj->RestoreBinary( context, file ) ;
	}
	return	pObj ;
}

RSObject * RSObject::RestoreObjectOfXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	if ( xmlDoc.GetTag() == L"null" )
	{
		return	NULL ;
	}
	RSClass *	pClass = context.GetClassAs( xmlDoc.GetTag() ) ;
	RSObject *	pObj = NULL ;
	if ( pClass != NULL )
	{
		RSObject *	pArg = context.new_Array() ;
		pObj = pClass->NewInstance( context, pArg ) ;
		context.ReleaseObjectRef( pArg ) ;
	}
	else
	{
		if ( xmlDoc.GetTag() == L"Pointer" )
		{
			pObj = context.new_Pointer( NULL ) ;
		}
		else if ( xmlDoc.GetTag() == L"Reference" )
		{
			pObj = context.new_Reference( NULL ) ;
		}
	}
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	SError	err = pObj->RestoreXMLDocument( context, xmlDoc ) ;
	if ( err )
	{
		context.ReleaseObjectRef( pObj ) ;
		return	NULL ;
	}
	return	pObj ;
}

