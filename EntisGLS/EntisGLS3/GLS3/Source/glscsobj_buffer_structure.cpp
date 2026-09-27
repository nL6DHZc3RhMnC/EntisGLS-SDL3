
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// naked 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSBufferStructure, ECSBuffer, ECSStructureInterface )

// ECSObject インスタンス
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSBufferStructure::GetInstanceObject( void )
{
	return	this ;
}

// 整数のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
int ECSBufferStructure::GetMemberAsInt
	( const wchar_t * pwszName, int nDefValue )
{
	int	nValue = nDefValue ;
	if ( m_pClassInf != NULL )
	do
	{
		int	iVar = m_pClassInf->GetVariableIndex( pwszName ) ;
		if ( iVar < 0 )
		{
			break ;
		}
		ECSTypeInfo *	pVarType = m_pClassInf->GetVariableAt( iVar ) ;
		if ( pVarType == NULL )
		{
			break ;
		}
		int	iOffset =
			m_pClassInf->GetVariableNakedOffsetAt( iVar ) ;
		//
		ECSObject *	pTypeObj = pVarType->m_pValue ;
		if ( pTypeObj == NULL )
		{
			break ;
		}
		void *	ptrBuf ;
		int		nSize ;
		switch ( pTypeObj->m_vtType )
		{
		case	csvtInteger:
			nSize = ((ECSInteger*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, false ) ;
			if ( ptrBuf != NULL )
			{
				nValue =
					(int) ECSPointerReference::LoadBufferInteger
						( ptrBuf, ((ECSInteger*)pTypeObj)->GetIntegerType() ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, false ) ;
			}
			break ;
		case	csvtReal:
			nSize = ((ECSReal*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, false ) ;
			if ( ptrBuf != NULL )
			{
				nValue =
					(int) ECSPointerReference::LoadBufferInteger
							( ptrBuf, ((ECSReal*)pTypeObj)->m_vtRealType ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, false ) ;
			}
			break ;
		}
	}
	while ( false ) ;
	return	nValue ;
}

// 実数のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
double ECSBufferStructure::GetMemberAsReal
	( const wchar_t * pwszName, double rDefValue )
{
	double	rValue = rDefValue ;
	if ( m_pClassInf != NULL )
	do
	{
		int	iVar = m_pClassInf->GetVariableIndex( pwszName ) ;
		if ( iVar < 0 )
		{
			break ;
		}
		ECSTypeInfo *	pVarType = m_pClassInf->GetVariableAt( iVar ) ;
		if ( pVarType == NULL )
		{
			break ;
		}
		int	iOffset =
			m_pClassInf->GetVariableNakedOffsetAt( iVar ) ;
		//
		ECSObject *	pTypeObj = pVarType->m_pValue ;
		if ( pTypeObj == NULL )
		{
			break ;
		}
		void *	ptrBuf ;
		int		nSize ;
		switch ( pTypeObj->m_vtType )
		{
		case	csvtInteger:
			nSize = ((ECSInteger*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, false ) ;
			if ( ptrBuf != NULL )
			{
				rValue = ECSPointerReference::LoadBufferReal
					( ptrBuf, ((ECSInteger*)pTypeObj)->GetIntegerType() ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, false ) ;
			}
			break ;
		case	csvtReal:
			nSize = ((ECSReal*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, false ) ;
			if ( ptrBuf != NULL )
			{
				rValue = ECSPointerReference::LoadBufferReal
					( ptrBuf, ((ECSReal*)pTypeObj)->m_vtRealType ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, false ) ;
			}
			break ;
		}
	}
	while ( false ) ;
	return	rValue ;
}

// 整数のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSBufferStructure::SetMemberAsInt
	( const wchar_t * pwszName, int nValue )
{
	if ( m_pClassInf != NULL )
	do
	{
		int	iVar = m_pClassInf->GetVariableIndex( pwszName ) ;
		if ( iVar < 0 )
		{
			break ;
		}
		ECSTypeInfo *	pVarType = m_pClassInf->GetVariableAt( iVar ) ;
		if ( pVarType == NULL )
		{
			break ;
		}
		int	iOffset =
			m_pClassInf->GetVariableNakedOffsetAt( iVar ) ;
		//
		ECSObject *	pTypeObj = pVarType->m_pValue ;
		if ( pTypeObj == NULL )
		{
			break ;
		}
		void *	ptrBuf ;
		int		nSize ;
		switch ( pTypeObj->m_vtType )
		{
		case	csvtInteger:
			nSize = ((ECSInteger*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, true ) ;
			if ( ptrBuf != NULL )
			{
				ECSPointerReference::StoreBufferInteger
					( ptrBuf, nValue,
						((ECSInteger*)pTypeObj)->GetIntegerType() ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, true ) ;
			}
			break ;
		case	csvtReal:
			nSize = ((ECSReal*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, true ) ;
			if ( ptrBuf != NULL )
			{
				ECSPointerReference::StoreBufferInteger
					( ptrBuf, nValue,
						((ECSReal*)pTypeObj)->m_vtRealType ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, true ) ;
			}
			break ;
		}
	}
	while ( false ) ;
}

// 実数のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSBufferStructure::SetMemberAsReal
	( const wchar_t * pwszName, double rValue )
{
	if ( m_pClassInf != NULL )
	do
	{
		int	iVar = m_pClassInf->GetVariableIndex( pwszName ) ;
		if ( iVar < 0 )
		{
			break ;
		}
		ECSTypeInfo *	pVarType = m_pClassInf->GetVariableAt( iVar ) ;
		if ( pVarType == NULL )
		{
			break ;
		}
		int	iOffset =
			m_pClassInf->GetVariableNakedOffsetAt( iVar ) ;
		//
		ECSObject *	pTypeObj = pVarType->m_pValue ;
		if ( pTypeObj == NULL )
		{
			break ;
		}
		void *	ptrBuf ;
		int		nSize ;
		switch ( pTypeObj->m_vtType )
		{
		case	csvtInteger:
			nSize = ((ECSInteger*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, true ) ;
			if ( ptrBuf != NULL )
			{
				ECSPointerReference::StoreBufferReal
					( ptrBuf, rValue,
						((ECSInteger*)pTypeObj)->GetIntegerType() ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, true ) ;
			}
			break ;
		case	csvtReal:
			nSize = ((ECSReal*)pTypeObj)->SizeOf() / 8 ;
			ptrBuf = GetBuffer( iOffset, nSize, true ) ;
			if ( ptrBuf != NULL )
			{
				ECSPointerReference::StoreBufferReal
					( ptrBuf, rValue,
						((ECSReal*)pTypeObj)->m_vtRealType ) ;
				FlushBuffer( iOffset, nSize, ptrBuf, true ) ;
			}
			break ;
		}
	}
	while ( false ) ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSBufferStructure::GetTypeName( void ) const
{
	if ( m_pClassInf != NULL )
	{
		return	m_pClassInf->GetGlobalName() ;
	}
	return	ECSBuffer::GetTypeName() ;
}

ECSObject * ECSBufferStructure::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( m_pClassInf != NULL )
	{
		if ( m_pClassInf->GetGlobalName().Compare( pwszTypeName ) == 0 )
		{
			return	this ;
		}
	}
	return	NULL ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSBufferStructure::Duplicate( void )
{
	return	new ECSBufferStructure( *this ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBufferStructure::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	if ( m_pClassInf == NULL )
	{
		return	ESLErrorMsg
			( "定義されていない構造体の関数を呼び出そうとしています" ) ;
	}
	ECSWideString	wstrFuncName = m_pClassInf->GetGlobalName() ;
	wstrFuncName += L"::" ;
	wstrFuncName += pwszName ;
	//
	DWORD *	pdwFuncAddr =
		context.m_pcsxi->GetFunctionAddress( wstrFuncName ) ;
	if ( pdwFuncAddr == NULL )
	{
		return	ESLErrorMsg( "構造体の定義されていない関数を呼び出しています。" ) ;
	}
	nIndex = (int) *pdwFuncAddr ;
	return	eslErrSuccess ;
}

// メンバ関数ポインタ取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBufferStructure::GetFunctionPointer
	( ECSContext & context,
		ECS_FUNCTION_POINTER & fptr, const wchar_t * pwszName )
{
	if ( m_pClassInf != NULL )
	{
		int	iFunc = m_pClassInf->FindFunctionAs( pwszName ) ;
		if ( iFunc < 0 )
		{
			return	ESLErrorMsg
				( "定義されていないクラスのメンバ関数を呼び出しています。" ) ;
		}
		ECSClassInfo::MemberFunction *
			pFunc = m_pClassInf->GetFunctionAt( iFunc ) ;
		ESLAssert( pFunc != NULL ) ;
		if ( !(pFunc->m_pClassCast->pClassInf->GetAttribute()
								& ECSTypeInfo::flagNativeObject) )
		{
			if ( pFunc->m_pClassCast->nNakedOffset == 0 )
			{
				fptr = pFunc->m_fpFuncPointer ;
				fptr.m_castThis.pCastObject = this ;
				return	eslErrSuccess ;
			}
			return	ESLErrorMsg( "呼び出せないメンバ関数です" ) ;
		}
	}
	return	ESLErrorMsg( "定義されていないメンバ関数の呼び出しです" ) ;
}

ESLError ECSBufferStructure::GetFunctionPointer
	( ECSContext & context, ECS_FUNCTION_POINTER & fptr, int nIndex )
{
	if ( m_pClassInf == NULL )
	{
		return	ESLErrorMsg
			( "定義されていないクラスのメンバ関数を呼び出しています。" ) ;
	}
	ECSClassInfo::MemberFunction *
		pFunc = m_pClassInf->GetFunctionAt( nIndex ) ;
	if ( pFunc == NULL )
	{
		return	ESLErrorMsg
			( "定義されていないクラスのメンバ関数を呼び出しています。" ) ;
	}
	if ( !(pFunc->m_pClassCast->pClassInf->GetAttribute()
							& ECSTypeInfo::flagNativeObject) )
	{
		if ( pFunc->m_pClassCast->nNakedOffset == 0 )
		{
			fptr = pFunc->m_fpFuncPointer ;
			fptr.m_castThis.pCastObject = this ;
			ESLAssert( fptr.m_ftType == ECS_FUNCTION_POINTER::funcScriptCall ) ;
			return	eslErrSuccess ;
		}
		return	ESLErrorMsg( "呼び出せないメンバ関数です" ) ;
	}
	return	ESLErrorMsg( "定義されていないメンバ関数の呼び出しです" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBufferStructure::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	context.PushObject( context.new_CSInteger( context.m_ip ) ) ;
	context.m_ip = nIndex ;
	if ( (context.m_pcsxi->m_dwImageSize <= context.m_ip)
		|| (context.m_pcsxi->m_pImage[context.m_ip] != csicEnter) )
	{
		return	ESLErrorMsg( "不正な関数エントリです。" ) ;
	}
	context.MarkCallStackFlag( ) ;
	return	eslErrSuccess ;
}

// 特殊演算子 : interface 型変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSBufferStructure::OperateCastInterface
		( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName )
{
	if ( m_pClassInf != NULL )
	{
		ECSClassInfo::CastInfo *
			pCast = m_pClassInf->GetCastClassInfoAs( pwszTypeName ) ;
		if ( pCast != NULL )
		{
			if ( pCast->nNakedOffset == 0 )
			{
				ci = *pCast ;
				ci.pCastObject = this ;
				return	eslErrSuccess ;
			}
		}
	}
	return	ESLErrorMsg( "未定義の型変換です" ) ;
}

// スクリプトのデストラクタ
//////////////////////////////////////////////////////////////////////////////
void ECSBufferStructure::OnDestruction( ECSContext & context )
{
	if ( m_pClassInf != NULL )
	{
		ECSContext *	pSysContext = context.GetSystemContext() ;
		int	nCount = m_pClassInf->GetDestructorCount() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ECSClassInfo::MemberFunction *
				pDestructor = m_pClassInf->GetDestructorAt( i ) ;
			ECS_FUNCTION_POINTER &
					fptr = pDestructor->m_fpFuncPointer ;
			//
			ECSPointerReference *
				pRefThis = context.new_CSPointerReference() ;
			pRefThis->SetReferenceCastInterface
							( this, &context, fptr.m_castThis ) ;
			if ( pDestructor->m_pClassCast != NULL )
			{
				pRefThis->SetOffset( pDestructor->m_pClassCast->nNakedOffset ) ;
			}
			DWORD	ipSaved = pSysContext->m_ip ;
			if ( pDestructor->IsNakedCall() )
			{
				INT64	addrObj ;
				ECSSakura2Processor::AssertLock() ;
				addrObj = context.m_pcsxi->AllocateHeapObjectAddress( pRefThis ) ;
				ECSSakura2Processor::AssertUnlock() ;
				//
				ESLError	err =
					pSysContext->CallNakedFunction
						( fptr.m_varFunc.addrScript, &addrObj, 1 ) ;
				//
				context.m_pcsxi->FreeHeapObjectAddress( addrObj, &context ) ;
			}
			else
			{
				ECSArray	arg ;
				arg.m_varArray.Add( pRefThis ) ;
				//
				ESLError	err =
					pSysContext->CallFunction
						( fptr.m_varFunc.addrScript, arg.m_varArray ) ;
				//
				arg.RemoveBetween( context ) ;
			}
			pSysContext->m_ip = ipSaved ;
		}
	}
	ECSBuffer::OnDestruction( context ) ;
}

