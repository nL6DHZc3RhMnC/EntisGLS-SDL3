
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 変数抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSObject, Object )

// 破棄処理
//////////////////////////////////////////////////////////////////////////////
void ECSObject::OnDestruction
	( ECSSakura2::VirtualMachine * vm,
		ECSSakura2Processor::Context * context )
{
	if ( context != NULL )
	{
		OnDestruction( *((ECSContext*)context) ) ;
	}
}

// 保存準備処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSObject::PrepareSave
	( ECSSakura2::VirtualMachine * vm,
		ECSSakura2Processor::Context * context )
{
	IndexAllMember() ;
	return	SSystem::errSuccess ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSObject::SaveStatic
	( SSystem::SFileInterface * file,
		ECSSakura2::VirtualMachine * vm,
		ECSSakura2Processor::Context * context )
{
	ESLSFileInterface	sfile( file ) ;
	return	(SSystem::SError) Save( sfile, *((ECSContext*)context) ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSObject::LoadStatic
	( SSystem::SFileInterface * file,
		ECSSakura2::VirtualMachine * vm,
		ECSSakura2Processor::Context * context )
{
	ESLSFileInterface	sfile( file ) ;
	return	(SSystem::SError) Load( sfile, *((ECSContext*)context) ) ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSObject::CommitAfterLoad
	( ECSSakura2::VirtualMachine * vm,
		ECSSakura2Processor::Context * context )
{
	return	(SSystem::SError) CommitAllReference( *((ECSContext*)context) ) ;
}

// 型判定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSObject::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( EWideString::Compare( GetTypeName(), pwszTypeName ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
/*ESLError ECSObject::GetVariableIndex( int & nIndex, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない要素参照です。" ) ;
}*/

ESLError ECSObject::GetVariableIndex( int & nIndex, int iMember )
{
	return	ESLErrorMsg( "定義されていない要素参照です。" ) ;
}

ESLError ECSObject::GetVariableIndex( int & nIndex, const wchar_t * pwszMember )
{
	return	ESLErrorMsg( "定義されていない要素参照です。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSObject::GetVariableAt( int nIndex )
{
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSObject::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	return	ESLErrorMsg( "定義されていないメンバ関数の呼び出しです。" ) ;
}

// メンバ関数ポインタ取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::GetFunctionPointer
	( ECSContext & context,
		ECS_FUNCTION_POINTER & fptr, const wchar_t * pwszName )
{
	ESLError	err =
		GetFunction( context, fptr.m_varFunc.nIndex, pwszName ) ;
	if ( err )
	{
		return	err ;
	}
	fptr.m_ftType = ECS_FUNCTION_POINTER::funcIndexCall ;
	fptr.m_castThis = ECS_CAST_INTERFACE( this ) ;
	return	eslErrSuccess ;
}

ESLError ECSObject::GetFunctionPointer
	( ECSContext & context, ECS_FUNCTION_POINTER & fptr, int nIndex )

{
	fptr.m_ftType = ECS_FUNCTION_POINTER::funcIndexCall ;
	fptr.m_castThis = ECS_CAST_INTERFACE( this ) ;
	fptr.m_varFunc.nIndex = nIndex ;
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	context.PushObject( new ECSInteger( 0 ) ) ;
	return	ESLErrorMsg( "定義されていないメンバ関数の呼び出しです。" ) ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::OperateBoolean( int & nBoolean )
{
	INT64		nValue ;
	ESLError	err = OperateInteger( nValue ) ;
	if ( !err )
	{
		nBoolean = (nValue != 0) ? -1 : 0 ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "定義されていない boolean への変換です。" ) ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::OperateSizeOf( INT64 & nSize )
{
	return	ESLErrorMsg( "定義されていない sizeof 演算子です。" ) ;
}

// 特殊演算子 : typeof
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSObject::OperateTypeOf( void ) const
{
	return	GetTypeName() ;
}

// 特殊演算子 : interface 型変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::OperateCastInterface
		( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName )
{
	ci.pCastObject = GetTypeOf( pwszTypeName ) ;
	ci.iVarOffset = 0 ;
	ci.nVarBounds = -1 ;
	ci.iFuncOffset = 0 ;
	return	eslErrSuccess ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::OperateInteger( INT64 & nValue )
{
	return	ESLErrorMsg( "定義されていない整数への型変換です" ) ;
}

// 実数取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::OperateReal( REAL64 & nValue )
{
	return	ESLErrorMsg( "定義されていない実数への型変換です" ) ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::OperateString( EWideString & wstrValue )
{
	return	ESLErrorMsg( "定義されていない文字列への型変換です" ) ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSObject::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	return	NULL ;
}

void ECSObject::FlushBuffer
	( int iOffset, int nSize, void * ptrBuf, bool fModified )
{
}

ECSSakura2Processor::LinearAddressCache *
	ECSObject::GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg )
{
	return	NULL ;
}

BYTE * ECSObject::GetSegmentShadowBuffer( int iShadow )
{
	return	NULL ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSObject::IndexAllMember( void )
{
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSObject::CleanupAllReference( ECSContext & context )
{
	ECSObject::OnObjectDestory( this ) ;
	m_pBackRef = NULL ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::CommitAllReference( ECSContext & context )
{
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::Save( ESLFileObject & file, ECSContext & context )
{
	return	ESLErrorMsg
		( "オブジェクトのシリアル化処理が定義されていません。" ) ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::Load( ESLFileObject & file, ECSContext & context )
{
	return	ESLErrorMsg
		( "オブジェクトの再構成処理が定義されていません。" ) ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSObject::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	eslErrSuccess ;
}

// メモリ確保
//////////////////////////////////////////////////////////////////////////////
void * ECSObject::operator new ( size_t stObj )
{
	return	::eslHeapAllocate( NULL, stObj, 0 ) ;
}

void * ECSObject::operator new ( size_t stObj, void * ptrObj )
{
	return	ptrObj ;
}

void * ECSObject::operator new
	( size_t stObj, const char * pszFileName, int nLine )
{
	return	::eslHeapAllocate( NULL, stObj, 0 ) ;
}

// メモリ解放
//////////////////////////////////////////////////////////////////////////////
void ECSObject::operator delete( void * ptrObj )
{
	::eslHeapFree( NULL, ptrObj ) ;
}

// メモリの有効性検証
//////////////////////////////////////////////////////////////////////////////
bool ECSObject::IsValidObjectType( void ) const
{
	__try
	{
		if ( (m_vtType >= 0) & (m_vtType < csvtValidMax) )
		{
#if	defined(_DEBUG)
			return	(GetESLClassName() != NULL) ;
#else
			return	true ;
#endif
		}
		else
		{
			ESLTrace( "不正な詞葉オブジェクトへのポインタです。\n" ) ;
		}
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ESLTrace( "不正な詞葉オブジェクトへのポインタです。\n" ) ;
	}
	return	false ;
}

// オブジェクトの実体を取得
//////////////////////////////////////////////////////////////////////////////
#if	defined(_DEBUG)
const ECSObject * ECSObject::GetEntity( const ECSObject * pObj )
{
	if ( pObj != NULL )
	{
		return	pObj->GetObjectEntity() ;
	}
	return	NULL ;
}

ECSObject * ECSObject::GetEntity( ECSObject * pObj )
{
	if ( pObj != NULL )
	{
		return	pObj->GetObjectEntity() ;
	}
	return	NULL ;
}

ECSObject * ECSObject::GetObjectEntity( void )
{
	__try
	{
		ESLAssert( this != NULL ) ;
		/*
		if ( this == NULL )
		{
			return	NULL ;
		}
		*/
		if ( (m_vtType == csvtReference)
			/*| (m_vtType == csvtPointerReference)*/ )
		{
			if ( (((ECSReference*)this)->m_pRef != NULL)
				&& ((ECSReference*)this)->m_pRef->IsValidObject() )
			{
				return	((ECSReference*) this)->m_pRef->GetObjectEntity( ) ;
			}
			((ECSReference*)this)->m_pRef = NULL ;
			return	NULL ;
		}
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		return	NULL ;
	}
	return	this ;
}

const ECSObject * ECSObject::GetObjectEntity( void ) const
{
	__try
	{
		ESLAssert( this != NULL ) ;
		/*
		if ( this == NULL )
		{
			return	NULL ;
		}
		*/
		if ( (m_vtType == csvtReference)
			/*| (m_vtType == csvtPointerReference)*/ )
		{
			if ( (((const ECSReference*)this)->m_pRef != NULL)
				&& ((const ECSReference*)this)->m_pRef->IsValidObject() )
			{
				return	((const ECSReference*) this)->m_pRef->GetObjectEntity( ) ;
			}
			return	NULL ;
		}
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		return	NULL ;
	}
	return	this ;
}
#endif


// オブジェクト破棄の通知
//////////////////////////////////////////////////////////////////////////////
void ECSObject::OnObjectDestory( ECSObject * pObj )
{
	if ( m_pBackRef->IsValidObject() )
	{
		m_pBackRef->OnObjectDestory( pObj ) ;
	}
	m_pBackRef = NULL ;
}

// 実数データか判定
//////////////////////////////////////////////////////////////////////////////
bool ECSObject::IsRealNumberObject( void ) const
{
	const ECSObject *	pEntity = GetObjectEntity() ;
	if ( pEntity->IsValidObject() )
	{
		if ( pEntity->m_vtType == csvtReal )
		{
			return	true ;
		}
		else if ( pEntity->m_vtType == csvtPointerReference )
		{
			return	(((ECSPointerReference*)pEntity)->
							GetMemoryObjectType() == csvtReal) ;
		}
	}
	return	false ;
}

// プラグインインターフェースを作成する
//////////////////////////////////////////////////////////////////////////////
ECSObject::PLUGIN_OBJECT * ECSObject::CreateInterface( void )
{
	if ( m_ppio == NULL )
	{
		m_ppio = (PLUGIN_OBJECT*) ::eslHeapAllocate
					( NULL, sizeof(PLUGIN_OBJECT), ESL_HEAP_ZERO_INIT ) ;
		m_ppio->pBackLink = this ;
		InitializeInterface( m_ppio ) ;
	}
	return	m_ppio ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSObject::GetObjectInterface( const wchar_t * pwszType )
{
	if ( EWideString::CompareNoCase( pwszType, L"ECS_OBJECT" ) )
	{
		if ( !EWideString::CompareNoCase( pwszType, L"ECSObject" ) )
		{
			return	this ;
		}
		return	NULL ;
	}
	if ( m_ppio == NULL )
	{
		CreateInterface( ) ;
	}
	ESLAssert( m_ppio != NULL ) ;
	return	(ECS_OBJECT*) m_ppio ;
}

// プラグインインターフェースを初期化する
//////////////////////////////////////////////////////////////////////////////
void ECSObject::InitializeInterface( ECS_OBJECT * pObj )
{
	::eslFillMemory( pObj, 0, sizeof(ECS_OBJECT) ) ;
	pObj->pfnRelease = PIC_Release ;
	pObj->pfnQueryInterface = PIC_QueryInterface ;
	pObj->pfnGetTypeName = PIC_GetTypeName ;
	pObj->pfnGetTypeOf = PIC_GetTypeOf ;
	pObj->pfnDuplicate = PIC_Duplicate ;
	pObj->pfnMove = PIC_Move ;
	pObj->pfnUnaryOperate = PIC_UnaryOperate ;
	pObj->pfnOperate = PIC_Operate ;
	pObj->pfnCompare = PIC_Compare ;
	pObj->pfnGetVariableIndexInt = PIC_GetVariableIndexInt ;
	pObj->pfnGetVariableIndexStr = PIC_GetVariableIndexStr ;
	pObj->pfnGetVariableAt = PIC_GetVariableAt ;
	pObj->pfnSetVariableAt = PIC_SetVariableAt ;
	pObj->pfnGetFunction = PIC_GetFunction ;
	pObj->pfnCallFunction = PIC_CallFunction ;
	pObj->pfnIndexAllMember = PIC_IndexAllMember ;
	pObj->pfnCleanupAllReference = PIC_CleanupAllReference ;
	pObj->pfnCommitAllReference = PIC_CommitAllReference ;
	pObj->pfnSave = PIC_Save ;
	pObj->pfnLoad = PIC_Load ;
	pObj->pfnDumpObject = PIC_DumpObject ;
}

void __stdcall ECSObject::PIC_Release( ECS_OBJECT * instance )
{
	PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
	if ( ppio->pBackLink != NULL )
	{
		ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
		ECSObject *	pObj = ppio->pBackLink ;
		::eslHeapFree( NULL, pObj->m_ppio ) ;
		pObj->m_ppio = NULL ;
		delete	pObj ;
	}
}

void * __stdcall ECSObject::PIC_QueryInterface
	( ECS_OBJECT * instance, const wchar_t * pwszType )
{
	PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	return	ppio->pBackLink->GetObjectInterface( pwszType ) ;
}

const wchar_t * __stdcall ECSObject::PIC_GetTypeName( ECS_OBJECT * instance )
{
	PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	return	ppio->pBackLink->GetTypeName( ) ;
}

ECS_OBJECT * __stdcall ECSObject::PIC_GetTypeOf
	( ECS_OBJECT * instance, const wchar_t * pwszTypeName )
{
	PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ECSObject *	pTypeObj = ppio->pBackLink->GetTypeOf( pwszTypeName ) ;
	if ( pTypeObj == NULL )
	{
		return	NULL ;
	}
	return	pTypeObj->CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSObject::PIC_Duplicate( ECS_OBJECT * instance )
{
	PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ECSObject *	pDupObj = ppio->pBackLink->Duplicate( ) ;
	if ( pDupObj == NULL )
	{
		return	NULL ;
	}
	return	pDupObj->CreateInterface( ) ;
}

ESLError __stdcall ECSObject::PIC_Move
	( ECS_OBJECT * instance,
		ECS_CONTEXT * context, const ECS_OBJECT * obj )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	PLUGIN_OBJECT *		ppioObj = (PLUGIN_OBJECT*) obj ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ESLAssert( ppioObj->pBackLink->m_ppio == ppioObj ) ;
	return	ppio->pBackLink->Move
			( *(ppic->pBackLink), ppioObj->pBackLink ) ;
}

ESLError __stdcall ECSObject::PIC_UnaryOperate
	( ECS_OBJECT * instance,
		ECS_CONTEXT * context, CSUnaryOperatorType csuopType )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppio->pBackLink->UnaryOperate( *(ppic->pBackLink), csuopType ) ;
}

ESLError __stdcall ECSObject::PIC_Operate
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		CSOperatorType csopType, ECS_OBJECT * obj )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	PLUGIN_OBJECT *		ppioObj = (PLUGIN_OBJECT*) obj ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ESLAssert( ppioObj->pBackLink->m_ppio == ppioObj ) ;
	return	ppio->pBackLink->Operate
			( *(ppic->pBackLink), csopType, ppioObj->pBackLink ) ;
}

ESLError __stdcall ECSObject::PIC_Compare
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int * pResult, CSCompareType cscpType, ECS_OBJECT * obj )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	PLUGIN_OBJECT *		ppioObj = (PLUGIN_OBJECT*) obj ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ESLAssert( ppioObj->pBackLink->m_ppio == ppioObj ) ;
	return	ppio->pBackLink->Compare
		( *(ppic->pBackLink), *pResult, cscpType, *(ppioObj->pBackLink) ) ;
}

ESLError __stdcall ECSObject::PIC_GetVariableIndexInt
	( ECS_OBJECT * instance, int * pIndex, int iElement )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	return	ppio->pBackLink->GetVariableIndex( *pIndex, iElement ) ;
}

ESLError __stdcall ECSObject::PIC_GetVariableIndexStr
	( ECS_OBJECT * instance, int * pIndex, const wchar_t * pwszElement )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	return	ppio->pBackLink->GetVariableIndex( *pIndex, pwszElement ) ;
}

ECS_OBJECT * __stdcall ECSObject::PIC_GetVariableAt
	( ECS_OBJECT * instance, int nIndex )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ECSObject *	pElement = ppio->pBackLink->GetVariableAt( nIndex ) ;
	if ( pElement == NULL )
	{
		return	NULL ;
	}
	return	pElement->CreateInterface( ) ;
}

ECS_OBJECT * __stdcall ECSObject::PIC_SetVariableAt
	( ECS_OBJECT * instance, int nIndex, ECS_OBJECT * obj )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	PLUGIN_OBJECT *		ppioObj = (PLUGIN_OBJECT*) obj ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppioObj->pBackLink->m_ppio == ppioObj ) ;
	ECSObject *	pElement =
		ppio->pBackLink->SetVariableAt( nIndex, ppioObj->pBackLink ) ;
	if ( pElement == NULL )
	{
		return	NULL ;
	}
	return	pElement->CreateInterface( ) ;
}

ESLError __stdcall ECSObject::PIC_GetFunction
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int * pIndex, const wchar_t * pwszName )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppio->pBackLink->GetFunction
				( *(ppic->pBackLink), *pIndex, pwszName ) ;
}

ESLError __stdcall ECSObject::PIC_CallFunction
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int nIndex, ECS_OBJECT * const* pArg, int nArgCount )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	//
	ECSObjArray<ECSObject>	lstArg ;
	lstArg.SetSize( nArgCount ) ;
	for ( int i = 0; i < nArgCount; i ++ )
	{
		if ( pArg[i] != NULL )
		{
			PLUGIN_OBJECT *	ppioArg = (PLUGIN_OBJECT*) pArg[i] ;
			ESLAssert( ppioArg->pBackLink->m_ppio == ppioArg ) ;
			lstArg.SetAt( i, ppioArg->pBackLink ) ;
		}
	}
	ESLError	err = ppio->pBackLink->CallFunction
						( *(ppic->pBackLink), nIndex, lstArg ) ;
	lstArg.DetachAll( ) ;
	return	err ;
}

void __stdcall ECSObject::PIC_IndexAllMember( ECS_OBJECT * instance )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ppio->pBackLink->IndexAllMember( ) ;
}

void __stdcall ECSObject::PIC_CleanupAllReference
	( ECS_OBJECT * instance, ECS_CONTEXT * context )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ppio->pBackLink->CleanupAllReference( *(ppic->pBackLink) ) ;
}

ESLError __stdcall ECSObject::PIC_CommitAllReference
	( ECS_OBJECT * instance, ECS_CONTEXT * context )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	return	ppio->pBackLink->CommitAllReference( *(ppic->pBackLink) ) ;
}

ESLError __stdcall ECSObject::PIC_Save
	( ECS_OBJECT * instance,
		ECS_FILE * pfile, ECS_CONTEXT * context )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	//
	ECSPIFileInterface	file( pfile ) ;
	return	ppio->pBackLink->Save( file, *(ppic->pBackLink) ) ;
}

ESLError __stdcall ECSObject::PIC_Load
	( ECS_OBJECT * instance,
		ECS_FILE * pfile, ECS_CONTEXT * context )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	//
	ECSPIFileInterface	file( pfile ) ;
	return	ppio->pBackLink->Load( file, *(ppic->pBackLink) ) ;
}

ESLError __stdcall ECSObject::PIC_DumpObject
	( ECS_OBJECT * instance, ECS_FILE * pfile,
		int nIndent, ECS_CONTEXT * context )
{
	PLUGIN_OBJECT *		ppio = (PLUGIN_OBJECT*) instance ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	//
	EStreamBuffer	buf ;
	ESLError		err = ppio->pBackLink->DumpObject
							( buf, nIndent, *(ppic->pBackLink) ) ;
	EPtrBuffer		ptrbuf = buf.GetBuffer( ) ;
	pfile->Write( ptrbuf, ptrbuf.GetLength() ) ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// プラグイン拡張用抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

#if	defined(_DEBUG)
#define	PLUGIN_TRY	{
#define	PLUGIN_ENDTRY	}
#else
#define	PLUGIN_TRY	__try {
#define	PLUGIN_ENDTRY	} __except ( EXCEPTION_EXECUTE_HANDLER ) { }
#endif


// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSPIObject::ECSPIObject( void )
{
	CreateInterface( ) ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSPIObject::GetTypeName( void ) const
{
	PLUGIN_TRY
	{
		return	m_ppio->GetTypeName( ) ;
	}
	PLUGIN_ENDTRY
	return	NULL ;
}

ECSObject * ECSPIObject::GetTypeOf( const wchar_t * pwszTypeName )
{
	PLUGIN_TRY
	{
		ECS_OBJECT *	pObj = m_ppio->GetTypeOf( pwszTypeName ) ;
		if ( pObj == NULL )
		{
			return	NULL ;
		}
		PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) pObj ;
		return	ppio->pBackLink ;
	}
	PLUGIN_ENDTRY
	return	NULL ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPIObject::Duplicate( void )
{
	PLUGIN_TRY
	{
		ECS_OBJECT *	pObj = m_ppio->Duplicate( ) ;
		if ( pObj == NULL )
		{
			return	NULL ;
		}
		PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) pObj ;
		return	ppio->pBackLink ;
	}
	PLUGIN_ENDTRY
	return	NULL ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPIObject::Move( ECSContext & context, ECSObject * obj )
{
	PLUGIN_TRY
	{
		return	m_ppio->Move
			( context.GetContextInterface(), obj->CreateInterface() ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg( "代入操作中にプラグインでエラーが発生しました。" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPIObject::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	PLUGIN_TRY
	{
		return	m_ppio->UnaryOperate
			( context.GetContextInterface(), csuopType ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "単項演算子実行中にプラグインでエラーが発生しました。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPIObject::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	PLUGIN_TRY
	{
		return	m_ppio->Operate
			( context.GetContextInterface(),
					csopType, obj->CreateInterface() ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "二項演算子実行中にプラグインでエラーが発生しました。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPIObject::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	PLUGIN_TRY
	{
		return	m_ppio->Compare
			( context.GetContextInterface(), &nResult,
					cscpType, obj.CreateInterface() ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "比較演算子実行中にプラグインでエラーが発生しました。" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPIObject::GetVariableIndex( int & nIndex, int iMember )
{
	PLUGIN_TRY
	{
		return	m_ppio->GetVariableIndex( &nIndex, iMember ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "メンバ変数参照中にプラグインでエラーが発生しました。" ) ;
}

ESLError ECSPIObject::GetVariableIndex( int & nIndex, const wchar_t * pwszName )
{
	PLUGIN_TRY
	{
		return	m_ppio->GetVariableIndex( &nIndex, pwszName ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "メンバ変数参照中にプラグインでエラーが発生しました。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPIObject::GetVariableAt( int nIndex )
{
	PLUGIN_TRY
	{
		ECS_OBJECT *	pObj = m_ppio->GetVariableAt( nIndex ) ;
		if ( pObj == NULL )
		{
			return	NULL ;
		}
		PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) pObj ;
		return	ppio->pBackLink ;
	}
	PLUGIN_ENDTRY
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPIObject::SetVariableAt( int nIndex, ECSObject * obj )
{
	PLUGIN_TRY
	{
		ECS_OBJECT *	pObj =
			m_ppio->SetVariableAt( nIndex, obj->CreateInterface() ) ;
		if ( pObj == NULL )
		{
			return	NULL ;
		}
		PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) pObj ;
		return	ppio->pBackLink ;
	}
	PLUGIN_ENDTRY
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPIObject::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	PLUGIN_TRY
	{
		return	m_ppio->GetFunction
			( context.GetContextInterface(), &nIndex, pwszName ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "メンバ関数照会中にプラグインでエラーが発生しました。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
static ESLError ECSPIObject__CallFunction
	( ECS_OBJECT * pObj, ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	EPtrObjArray<ECS_OBJECT>	arg ;
	for ( unsigned int i = 0; i < lstArg.GetSize(); i ++ )
	{
		ECSObject *	pObj = lstArg.GetAt( i ) ;
		if ( pObj != NULL )
		{
			arg.SetAt( i, pObj->CreateInterface() ) ;
		}
		else
		{
			arg.SetAt( i, NULL ) ;
		}
	}
	return	pObj->CallFunction
		( context.GetContextInterface(),
			nIndex, arg.GetData(), arg.GetSize() ) ;
}
ESLError ECSPIObject::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	PLUGIN_TRY
	{
		return	ECSPIObject__CallFunction( m_ppio, context, nIndex, lstArg ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "メンバ関数呼び出し中にプラグインでエラーが発生しました。" ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSPIObject::IndexAllMember( void )
{
	PLUGIN_TRY
	{
		m_ppio->IndexAllMember( ) ;
	}
	PLUGIN_ENDTRY
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSPIObject::CleanupAllReference( ECSContext & context )
{
	PLUGIN_TRY
	{
		m_ppio->CleanupAllReference( context.GetContextInterface() ) ;
	}
	PLUGIN_ENDTRY
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPIObject::CommitAllReference( ECSContext & context )
{
	PLUGIN_TRY
	{
		return	m_ppio->CommitAllReference( context.GetContextInterface() ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "CommitAllReference 関数呼び出し中にプラグインでエラーが発生しました。" ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
static ESLError ECSPIObject__Save
	( ECS_OBJECT * pObj, ESLFileObject & file, ECSContext & context )
{
	ECSFilePIInterface	filepi( &file ) ;
	return	pObj->Save( &filepi, context.GetContextInterface() ) ;
}

ESLError ECSPIObject::Save( ESLFileObject & file, ECSContext & context )
{
	PLUGIN_TRY
	{
		return	ECSPIObject__Save( m_ppio, file, context ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "Save 関数呼び出し中にプラグインでエラーが発生しました。" ) ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
static ESLError ECSPIObject__Load
	( ECS_OBJECT * pObj, ESLFileObject & file, ECSContext & context )
{
	ECSFilePIInterface	filepi( &file ) ;
	return	pObj->Load( &filepi, context.GetContextInterface() ) ;
}

ESLError ECSPIObject::Load( ESLFileObject & file, ECSContext & context )
{
	PLUGIN_TRY
	{
		return	ECSPIObject__Load( m_ppio, file, context ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "Load 関数呼び出し中にプラグインでエラーが発生しました。" ) ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
static ESLError ECSPIObject__DumpObject
	( ECS_OBJECT * pObj, EStreamBuffer & buf,
			int nIndent, ECSContext & context )
{
	EMemoryFile			memfile ;
	ECSFilePIInterface	filepi( &memfile ) ;
	memfile.Create( 0x400 ) ;
	ESLError	err =
		pObj->DumpObject
			( &filepi, nIndent, context.GetContextInterface() ) ;
	buf.Write( memfile.GetBuffer(), memfile.GetLength() ) ;
	return	err ;
}

ESLError ECSPIObject::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	PLUGIN_TRY
	{
		return	ECSPIObject__DumpObject( m_ppio, buf, nIndent, context ) ;
	}
	PLUGIN_ENDTRY
	return	ESLErrorMsg
		( "DumpObject 関数呼び出し中にプラグインでエラーが発生しました。" ) ;
}

// プラグインインターフェースを作成する
//////////////////////////////////////////////////////////////////////////////
ECSObject::PLUGIN_OBJECT * ECSPIObject::CreateInterface( void )
{
	if ( m_ppio == NULL )
	{
		m_ppio = (PLUGIN_OBJECT*) ::eslHeapAllocate
			( NULL, sizeof(PLUGIN_OBJECT), ESL_HEAP_ZERO_INIT ) ;
		m_ppio->pBackLink = this ;
		m_ppio->pfnRelease = PIC_Release ;
		m_ppio->pfnQueryInterface = PIC_QueryInterface ;
		m_ppio->pfnGetTypeName = PIC_GetTypeName ;
		m_ppio->pfnGetTypeOf = PIC_GetTypeOf ;
		m_ppio->pfnDuplicate = PIC_Duplicate ;
		m_ppio->pfnMove = PIC_Move ;
		m_ppio->pfnUnaryOperate = PIC_UnaryOperate ;
		m_ppio->pfnOperate = PIC_Operate ;
		m_ppio->pfnCompare = PIC_Compare ;
		m_ppio->pfnGetVariableIndexInt = PIC_GetVariableIndex ;
		m_ppio->pfnGetVariableIndexStr = PIC_GetVariableIndex ;
		m_ppio->pfnGetVariableAt = PIC_GetVariableAt ;
		m_ppio->pfnSetVariableAt = PIC_SetVariableAt ;
		m_ppio->pfnGetFunction = PIC_GetFunction ;
		m_ppio->pfnCallFunction = PIC_CallFunction ;
		m_ppio->pfnIndexAllMember = PIC_IndexAllMember ;
		m_ppio->pfnCleanupAllReference = PIC_CleanupAllReference ;
		m_ppio->pfnCommitAllReference = PIC_CommitAllReference ;
		m_ppio->pfnSave = PIC_Save ;
		m_ppio->pfnLoad = PIC_Load ;
		m_ppio->pfnDumpObject = PIC_DumpObject ;
	}
	return	m_ppio ;
}

// プラグインインターフェース
//////////////////////////////////////////////////////////////////////////////
void __stdcall ECSPIObject::PIC_Release( ECS_OBJECT * instance )
{
	PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
	if ( ppio->pBackLink != NULL )
	{
		ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
		ECSObject *	pObj = ppio->pBackLink ;
		::eslHeapFree( NULL, pObj->m_ppio ) ;
		pObj->m_ppio = NULL ;
		delete	pObj ;
	}
}

void * __stdcall ECSPIObject::PIC_QueryInterface
	( ECS_OBJECT * instance, const wchar_t * pwszType )
{
	if ( EWideString::CompareNoCase( pwszType, L"ECS_OBJECT" ) )
	{
		if ( !EWideString::CompareNoCase( pwszType, L"ECSObject" ) )
		{
			PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
			ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
			return	ppio->pBackLink ;
		}
		return	NULL ;
	}
	return	instance ;
}

const wchar_t *
	__stdcall ECSPIObject::PIC_GetTypeName( ECS_OBJECT * instance )
{
	return	NULL ;
}

ECS_OBJECT * __stdcall ECSPIObject::PIC_GetTypeOf
	( ECS_OBJECT * instance, const wchar_t * pwszTypeName )
{
	return	NULL ;
}

ECS_OBJECT * __stdcall ECSPIObject::PIC_Duplicate( ECS_OBJECT * instance )
{
	return	NULL ;
}

ESLError __stdcall ECSPIObject::PIC_Move
	( ECS_OBJECT * instance, ECS_CONTEXT * context, const ECS_OBJECT * obj )
{
	return	ESLErrorMsg( "定義されていない代入操作です。" ) ;
}

ESLError __stdcall ECSPIObject::PIC_UnaryOperate
	( ECS_OBJECT * instance,
		ECS_CONTEXT * context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない単項演算子です。" ) ;
}

ESLError __stdcall ECSPIObject::PIC_Operate
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		CSOperatorType csopType, ECS_OBJECT * obj )
{
	return	ESLErrorMsg( "定義されていない二項演算子です。" ) ;
}

ESLError __stdcall ECSPIObject::PIC_Compare
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int * pResult, CSCompareType cscpType, ECS_OBJECT * obj )
{
	return	ESLErrorMsg( "定義されていない比較演算子です。" ) ;
}

ESLError __stdcall ECSPIObject::PIC_GetVariableIndex
	( ECS_OBJECT * instance, int * pIndex, int iMember )
{
	return	ESLErrorMsg( "定義されていない要素参照です。" ) ;
}

ESLError __stdcall ECSPIObject::PIC_GetVariableIndex
	( ECS_OBJECT * instance, int * pIndex, const wchar_t * pwszName )
{
	return	ESLErrorMsg( "定義されていない要素参照です。" ) ;
}

ECS_OBJECT * __stdcall ECSPIObject::PIC_GetVariableAt
	( ECS_OBJECT * instance, int nIndex )
{
	return	NULL ;
}

ECS_OBJECT * __stdcall ECSPIObject::PIC_SetVariableAt
	( ECS_OBJECT * instance, int nIndex, ECS_OBJECT * obj )
{
	return	NULL ;
}

ESLError __stdcall ECSPIObject::PIC_GetFunction
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int * pIndex, const wchar_t * pwszName )
{
	return	ESLErrorMsg( "定義されていないメンバ関数の呼び出しです。" ) ;
}

ESLError __stdcall ECSPIObject::PIC_CallFunction
	( ECS_OBJECT * instance, ECS_CONTEXT * context,
		int nIndex, ECS_OBJECT * const* pArg, int nArgCount )
{
	return	ESLErrorMsg( "定義されていないメンバ関数の呼び出しです。" ) ;
}

void __stdcall ECSPIObject::PIC_IndexAllMember( ECS_OBJECT * instance )
{
}

void __stdcall ECSPIObject::PIC_CleanupAllReference
	( ECS_OBJECT * instance, ECS_CONTEXT * context )
{
}

ESLError __stdcall ECSPIObject::PIC_CommitAllReference
	( ECS_OBJECT * instance, ECS_CONTEXT * context )
{
	return	eslErrSuccess ;
}

ESLError __stdcall ECSPIObject::PIC_Save
	( ECS_OBJECT * instance,
		ECS_FILE * pfile, ECS_CONTEXT * context )
{
	return	eslErrSuccess ;
}

ESLError __stdcall ECSPIObject::PIC_Load
	( ECS_OBJECT * instance,
		ECS_FILE * pfile, ECS_CONTEXT * context )
{
	return	eslErrSuccess ;
}

ESLError __stdcall ECSPIObject::PIC_DumpObject
	( ECS_OBJECT * instance, ECS_FILE * pfile,
		int nIndent, ECS_CONTEXT * context )
{
	return	eslErrSuccess ;
}
