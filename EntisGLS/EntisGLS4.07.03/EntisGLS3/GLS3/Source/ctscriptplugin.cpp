
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 外部モジュールインターフェース
//////////////////////////////////////////////////////////////////////////////

#include <ctscriptplugin.h>


//////////////////////////////////////////////////////////////////////////////
// プラグイン固有オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSPISubClass, ESLObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSPISubClass::~ECSPISubClass( void )
{
	if ( m_baseobj != NULL )
	{
		m_baseobj->Release( ) ;
	}
}

// オブジェクトインスタンスを設定（オーバーライド）
//////////////////////////////////////////////////////////////////////////////
void ECSPISubClass::SetObjectInstance
	( ECS_OBJECT * absobj, ECS_OBJECT * baseobj )
{
	if ( m_instance != NULL )
	{
		m_absobj.pfnRelease( m_instance ) ;
	}
	if ( m_baseobj != NULL )
	{
		m_baseobj->Release( ) ;
		m_baseobj = NULL ;
	}
	if ( absobj != NULL )
	{
		m_absobj = *absobj ;
		absobj->ptrInstance = this ;
		absobj->pfnRelease = PI_Release ;
		absobj->pfnQueryInterface = PI_QueryInterface ;
		absobj->pfnGetTypeName = PI_GetTypeName ;
		absobj->pfnGetTypeOf = PI_GetTypeOf ;
		absobj->pfnDuplicate = PI_Duplicate ;
		absobj->pfnMove = PI_Move ;
		absobj->pfnUnaryOperate = PI_UnaryOperate ;
		absobj->pfnOperate = PI_Operate ;
		absobj->pfnCompare = PI_Compare ;
		absobj->pfnGetVariableIndexInt = PI_GetVariableIndex ;
		absobj->pfnGetVariableIndexStr = PI_GetVariableIndex ;
		absobj->pfnGetVariableAt = PI_GetVariableAt ;
		absobj->pfnSetVariableAt = PI_SetVariableAt ;
		absobj->pfnGetFunction = PI_GetFunction ;
		absobj->pfnCallFunction = PI_CallFunction ;
		absobj->pfnIndexAllMember = PI_IndexAllMember ;
		absobj->pfnCleanupAllReference = PI_CleanupAllReference ;
		absobj->pfnCommitAllReference = PI_CommitAllReference ;
		absobj->pfnSave = PI_Save ;
		absobj->pfnLoad = PI_Load ;
		absobj->pfnDumpObject = PI_DumpObject ;
	}
	m_instance = absobj ;
	m_baseobj = baseobj ;
}

// オーバーライド可能な関数
//////////////////////////////////////////////////////////////////////////////
void ECSPISubClass::Release( void )
{
	m_absobj.pfnRelease( m_instance ) ;
	delete	this ;
}

void * ECSPISubClass::QueryInterface( const wchar_t * pwszType )
{
	if ( m_baseobj != NULL )
	{
		void *	pInterface = m_baseobj->QueryInterface( pwszType ) ;
		if ( pInterface != NULL )
		{
			return	pInterface ;
		}
	}
	return	m_absobj.pfnQueryInterface( m_instance, pwszType ) ;
}

const wchar_t * ECSPISubClass::GetTypeName( void )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->GetTypeName( ) ;
	}
	return	m_absobj.pfnGetTypeName( m_instance ) ;
}

ECS_OBJECT * ECSPISubClass::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( m_baseobj != NULL )
	{
		ECS_OBJECT *	ptrObj = m_baseobj->GetTypeOf( pwszTypeName ) ;
		if ( ptrObj != NULL )
		{
			return	ptrObj ;
		}
	}
	return	m_absobj.pfnGetTypeOf( m_instance, pwszTypeName ) ;
}

ECS_OBJECT * ECSPISubClass::Duplicate( void )
{
	if ( m_baseobj != NULL )
	{
		ECS_OBJECT *	ptrObj = m_baseobj->Duplicate( ) ;
		if ( ptrObj != NULL )
		{
			return	ptrObj ;
		}
	}
	return	m_absobj.pfnDuplicate( m_instance ) ;
}

ESLError ECSPISubClass::Move( ECS_CONTEXT * context, const ECS_OBJECT * obj )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->Move( context, obj ) ;
	}
	return	m_absobj.pfnMove( m_instance, context, obj ) ;
}

ESLError ECSPISubClass::UnaryOperate
	( ECS_CONTEXT * context, CSUnaryOperatorType csuopType )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->UnaryOperate( context, csuopType ) ;
	}
	return	m_absobj.pfnUnaryOperate( m_instance, context, csuopType ) ;
}

ESLError ECSPISubClass::Operate
	( ECS_CONTEXT * context, CSOperatorType csopType, ECS_OBJECT * obj )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->Operate( context, csopType, obj ) ;
	}
	return	m_absobj.pfnOperate( m_instance, context, csopType, obj ) ;
}

ESLError ECSPISubClass::Compare
	( ECS_CONTEXT * context, int * pResult,
		CSCompareType cscpType, ECS_OBJECT * obj )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->Compare( context, pResult, cscpType, obj ) ;
	}
	return	m_absobj.pfnCompare( m_instance, context, pResult, cscpType, obj ) ;
}

ESLError ECSPISubClass::GetVariableIndex( int * pIndex, int iElement )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->GetVariableIndex( pIndex, iElement ) ;
	}
	return	m_absobj.pfnGetVariableIndexInt( m_instance, pIndex, iElement ) ;
}

ESLError ECSPISubClass::GetVariableIndex( int * pIndex, const wchar_t * pwszElement )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->GetVariableIndex( pIndex, pwszElement ) ;
	}
	return	m_absobj.pfnGetVariableIndexStr( m_instance, pIndex, pwszElement ) ;
}

ECS_OBJECT * ECSPISubClass::GetVariableAt( int nIndex )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->GetVariableAt( nIndex ) ;
	}
	return	m_absobj.pfnGetVariableAt( m_instance, nIndex ) ;
}

ECS_OBJECT * ECSPISubClass::SetVariableAt( int nIndex, ECS_OBJECT * obj )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->SetVariableAt( nIndex, obj ) ;
	}
	return	m_absobj.pfnSetVariableAt( m_instance, nIndex, obj ) ;
}

ESLError ECSPISubClass::GetFunction
	( ECS_CONTEXT * context, int * pIndex, const wchar_t * pwszName )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->GetFunction( context, pIndex, pwszName ) ;
	}
	return	m_absobj.pfnGetFunction
		( m_instance, context, pIndex, pwszName ) ;
}

ESLError ECSPISubClass::CallFunction
	( ECS_CONTEXT * context,
		int nIndex, ECS_OBJECT * const* pArg, int nArgCount )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->CallFunction( context, nIndex, pArg, nArgCount ) ;
	}
	return	m_absobj.pfnCallFunction
		( m_instance, context, nIndex, pArg, nArgCount ) ;
}

void ECSPISubClass::IndexAllMember( void )
{
	if ( m_baseobj != NULL )
	{
		m_baseobj->IndexAllMember( ) ;
	}
	m_absobj.pfnIndexAllMember( m_instance ) ;
}

void ECSPISubClass::CleanupAllReference( ECS_CONTEXT * context )
{
	if ( m_baseobj != NULL )
	{
		m_baseobj->CleanupAllReference( context ) ;
	}
	m_absobj.pfnCleanupAllReference( m_instance, context ) ;
}

ESLError ECSPISubClass::CommitAllReference( ECS_CONTEXT * context )
{
	if ( m_baseobj != NULL )
	{
		ESLError	err = m_baseobj->CommitAllReference( context ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	m_absobj.pfnCommitAllReference( m_instance, context ) ;
}

ESLError ECSPISubClass::Save( ECS_FILE * pfile, ECS_CONTEXT * context )
{
	if ( m_baseobj != NULL )
	{
		ESLError	err = m_baseobj->Save( pfile, context ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	m_absobj.pfnSave( m_instance, pfile, context ) ;
}

ESLError ECSPISubClass::Load( ECS_FILE * pfile, ECS_CONTEXT * context )
{
	if ( m_baseobj != NULL )
	{
		ESLError	err = m_baseobj->Load( pfile, context ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	m_absobj.pfnLoad( m_instance, pfile, context ) ;
}

ESLError ECSPISubClass::DumpObject
	( ECS_FILE * pfile, int nIndent, ECS_CONTEXT * context )
{
	if ( m_baseobj != NULL )
	{
		return	m_baseobj->DumpObject( pfile, nIndent, context ) ;
	}
	return	m_absobj.pfnDumpObject( m_instance, pfile, nIndent, context ) ;
}

// オーバーライド関数
//////////////////////////////////////////////////////////////////////////////
void __stdcall ECSPISubClass::PI_Release( ECS_OBJECT * instance )
{
	((ECSPISubClass*)(instance->ptrInstance))->Release( ) ;
}

void * __stdcall ECSPISubClass::PI_QueryInterface
	( ECS_OBJECT * instance, const wchar_t * pwszType )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->QueryInterface( pwszType ) ;
}

const wchar_t * __stdcall ECSPISubClass::PI_GetTypeName( ECS_OBJECT * instance )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->GetTypeName( ) ;
}

ECS_OBJECT * __stdcall ECSPISubClass::PI_GetTypeOf
	( ECS_OBJECT * instance, const wchar_t * pwszTypeName )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->GetTypeOf( pwszTypeName ) ;
}

ECS_OBJECT * __stdcall ECSPISubClass::PI_Duplicate( ECS_OBJECT * instance )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->Duplicate( ) ;
}

ESLError __stdcall ECSPISubClass::PI_Move
	( ECS_OBJECT * instance,
		ECS_CONTEXT * context, const ECS_OBJECT * obj )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->Move( context, obj ) ;
}

ESLError __stdcall ECSPISubClass::PI_UnaryOperate
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		CSUnaryOperatorType csuopType )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->
					UnaryOperate( context, csuopType ) ;
}

ESLError __stdcall ECSPISubClass::PI_Operate
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		CSOperatorType csopType, ECS_OBJECT * obj )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->
					Operate( context, csopType, obj ) ;
}

ESLError __stdcall ECSPISubClass::PI_Compare
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int * pResult, CSCompareType cscpType, ECS_OBJECT * obj )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->
					Compare( context, pResult, cscpType, obj ) ;
}

ESLError __stdcall ECSPISubClass::PI_GetVariableIndex
	( ECS_OBJECT * instance, int * pIndex, const wchar_t * pwszElement )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->GetVariableIndex( pIndex, pwszElement ) ;
}

ESLError __stdcall ECSPISubClass::PI_GetVariableIndex
	( ECS_OBJECT * instance, int * pIndex, int iElement )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->GetVariableIndex( pIndex, iElement ) ;
}

ECS_OBJECT * __stdcall ECSPISubClass::PI_GetVariableAt
	( ECS_OBJECT * instance, int nIndex )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->GetVariableAt( nIndex ) ;
}

ECS_OBJECT * __stdcall ECSPISubClass::PI_SetVariableAt
	( ECS_OBJECT * instance, int nIndex, ECS_OBJECT * obj )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->SetVariableAt( nIndex, obj ) ;
}

ESLError __stdcall ECSPISubClass::PI_GetFunction
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int * pIndex, const wchar_t * pwszName )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->
					GetFunction( context, pIndex, pwszName ) ;
}

ESLError __stdcall ECSPISubClass::PI_CallFunction
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int nIndex, ECS_OBJECT * const* pArg, int nArgCount )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->
					CallFunction( context, nIndex, pArg, nArgCount ) ;
}

void __stdcall ECSPISubClass::PI_IndexAllMember( ECS_OBJECT * instance )
{
	((ECSPISubClass*)(instance->ptrInstance))->IndexAllMember( ) ;
}

void __stdcall ECSPISubClass::PI_CleanupAllReference
	( ECS_OBJECT * instance, ECS_CONTEXT * context )
{
	((ECSPISubClass*)
		(instance->ptrInstance))->CleanupAllReference( context ) ;
}

ESLError __stdcall ECSPISubClass::PI_CommitAllReference
	( ECS_OBJECT * instance, ECS_CONTEXT * context )
{
	return	((ECSPISubClass*)
				(instance->ptrInstance))->CommitAllReference( context ) ;
}

ESLError __stdcall ECSPISubClass::PI_Save
	( ECS_OBJECT * instance,
		ECS_FILE * pfile, ECS_CONTEXT * context )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->Save( pfile, context ) ;
}

ESLError __stdcall ECSPISubClass::PI_Load
	( ECS_OBJECT * instance,
		ECS_FILE * pfile, ECS_CONTEXT * context )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->Load( pfile, context ) ;
}

ESLError __stdcall ECSPISubClass::PI_DumpObject
	( ECS_OBJECT * instance, ECS_FILE * pfile,
		int nIndent, ECS_CONTEXT * context )
{
	return	((ECSPISubClass*)(instance->ptrInstance))->
						DumpObject( pfile, nIndent, context ) ;
}

