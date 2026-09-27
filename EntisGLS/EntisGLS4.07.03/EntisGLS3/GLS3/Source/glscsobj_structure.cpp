
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 構造体
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSStructureInterface, ESLObject )
IMPLEMENT_CLASS_INFO2( ECSStructure, ECSArray, ECSStructureInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSStructure::ECSStructure( void )
{
	m_vtType = csvtObject ;
	m_pwszTag = NULL ;
}

ECSStructure::ECSStructure( const ECSClassInfo * pClassInf )
{
	m_vtType = csvtObject ;
	m_pwszTag = pClassInf->GetGlobalName() ;
	m_pClassInf = pClassInf ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSStructure::~ECSStructure( void )
{
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSStructure::GetTypeName( void ) const
{
	return	m_pwszTag ;
}

ECSObject * ECSStructure::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( EWideString::Compare( m_pwszTag, pwszTypeName ) == 0 )
	{
		return	this ;
	}
	if ( m_pClassInf != NULL )
	{
		ECSClassInfo::CastInfo *
			pCastInf = m_pClassInf->GetCastClassInfoAs( pwszTypeName ) ;
		if ( pCastInf != NULL )
		{
			if ( (pCastInf->pClassInf != NULL)
				&& (pCastInf->pClassInf->GetAttribute()
							& ECSTypeInfo::flagNativeObject) )
			{
				return	m_varArray.GetAt( pCastInf->iNativeParent ) ;
			}
			return	this ;
		}
	}
	else
	{
		ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
		if ( pParent != NULL )
		{
			return	pParent->GetTypeOf( pwszTypeName ) ;
		}
	}
	return	NULL ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSStructure::Duplicate( void )
{
	ECSStructure *	pObj = new ECSStructure ;
	pObj->CopyFrom( *this ) ;
	//
	pObj->m_pwszTag = m_pwszTag ;
	pObj->m_pClassInf = m_pClassInf ;
	//
	if ( m_pClassInf == NULL )
	{
		pObj->m_staMember = m_staMember ;
	}
	if ( m_pDefObj != NULL )
	{
		pObj->SetDefaultElement
			( ECSTypeInfo::DuplicateType( m_pDefObj ) ) ;
	}
	return	pObj ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::Move( ECSContext & context, ECSObject * obj )
{
	if ( m_pClassInf == NULL )
	{
		DWORD *	pdwFuncAddr =
			context.m_pcsxi->GetFunctionAddress
				( EWideString( m_pwszTag ) + L"::@Move" ) ;
		if ( pdwFuncAddr != NULL )
		{
			//
			// オーバーロードスクリプト関数呼び出し
			//
			ECSObjArray<ECSObject>	lstArg ;
			lstArg.Add( context.new_CSReference( this ) ) ;
			lstArg.Add( context.new_CSReference( obj ) ) ;
			context.PushObject
				( context.new_CSInteger( - (long int) context.m_ip ) ) ;
			ESLError	err = context.CallFunction( *pdwFuncAddr, lstArg ) ;
			if ( !err )
			{
				ECSObject *	pError = context.PopObject( ) ;
				if ( (pError->m_vtType == csvtInteger)
					&& (((ECSInteger*)pError)->GetValue() != 0) )
				{
					err = ESLErrorMsg
						( "@Move オーバーロード関数がエラーを返しました。" ) ;
				}
				else if ( (pError->m_vtType == csvtString)
						&& !((ECSString*)pError)->m_varStr.IsEmpty() )
				{
					context.m_strErrMsg = ((ECSString*)pError)->m_varStr ;
					err = ESLErrorMsg( context.m_strErrMsg ) ;
				}
				if ( !err )
				{
					context.SetStatus( context.xsExecution ) ;
					context.delete_CSObject( obj ) ;
				}
			}
			return	err ;
		}
	}
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元のオブジェクトが存在しません。" ) ;
	}
	if ( EWideString::Compare( m_pwszTag, pEntity->GetTypeName() ) == 0 )
	{
		ECSStructure *	pStruct = ESLTypeCast<ECSStructure>( pEntity ) ;
		if ( pStruct != NULL )
		{
			ESLError	err = MoveFromStructure( context, *pStruct ) ;
			ESLAssert( VerifyAllElementValidation() ) ;
			if ( !err )
			{
				context.delete_CSObject( obj ) ;
				return	eslErrSuccess ;
			}
			return	err ;
		}
	}
	ECSHash *	pHash = ESLTypeCast<ECSHash>( pEntity ) ;
	if ( pHash != NULL )
	{
		ESLError	err = MoveFromHash( context, *pHash ) ;
		ESLAssert( VerifyAllElementValidation() ) ;
		if ( !err )
		{
			context.delete_CSObject( obj ) ;
			return	eslErrSuccess ;
		}
		return	err ;
	}
	return	ESLErrorMsg( "構造体への定義されていない代入操作です。" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	if ( m_pClassInf == NULL )
	{
		DWORD *	pdwFuncAddr =
			context.m_pcsxi->GetFunctionAddress
				( EWideString( m_pwszTag ) + L"::@UnaryOperate" ) ;
		if ( pdwFuncAddr != NULL )
		{
			//
			// オーバーロードスクリプト関数呼び出し
			//
			ECSObjArray<ECSObject>	lstArg ;
			lstArg.Add( context.new_CSReference( this ) ) ;
			lstArg.Add( context.new_CSInteger( csuopType ) ) ;
			context.PushObject
				( context.new_CSInteger( - (long int) context.m_ip ) ) ;
			ESLError	err = context.CallFunction( *pdwFuncAddr, lstArg ) ;
			if ( !err )
			{
				ECSObject *	pError = context.PopObject( ) ;
				if ( (pError->m_vtType == csvtInteger)
					&& (((ECSInteger*)pError)->GetValue() != 0) )
				{
					err = ESLErrorMsg
						( "@UnaryOperate オーバーロード関数がエラーを返しました。" ) ;
				}
				else if ( (pError->m_vtType == csvtString)
						&& !((ECSString*)pError)->m_varStr.IsEmpty() )
				{
					context.m_strErrMsg = ((ECSString*)pError)->m_varStr ;
					err = ESLErrorMsg( context.m_strErrMsg ) ;
				}
				if ( !err )
				{
					context.SetStatus( context.xsExecution ) ;
				}
			}
			return	err ;
		}
		ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
		if ( pParent != NULL )
		{
			return	pParent->UnaryOperate( context, csuopType ) ;
		}
	}
	return	ESLErrorMsg( "構造体への定義されていない演算操作です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	if ( m_pClassInf == NULL )
	{
		DWORD *	pdwFuncAddr =
			context.m_pcsxi->GetFunctionAddress
				( EWideString( m_pwszTag ) + L"::@Operate" ) ;
		if ( pdwFuncAddr != NULL )
		{
			//
			// オーバーロードスクリプト関数呼び出し
			//
			ECSObjArray<ECSObject>	lstArg ;
			lstArg.Add( context.new_CSReference( this ) ) ;
			lstArg.Add( context.new_CSInteger( csopType ) ) ;
			lstArg.Add( context.new_CSReference( obj ) ) ;
			context.PushObject
				( context.new_CSInteger( - (long int) context.m_ip ) ) ;
			ESLError	err = context.CallFunction( *pdwFuncAddr, lstArg ) ;
			if ( !err )
			{
				ECSObject *	pError = context.PopObject( ) ;
				if ( (pError->m_vtType == csvtInteger)
					&& (((ECSInteger*)pError)->GetValue() != 0) )
				{
					delete	pError ;
					err = ESLErrorMsg
						( "@Operate オーバーロード関数がエラーを返しました。" ) ;
				}
				else if ( (pError->m_vtType == csvtString)
						&& !((ECSString*)pError)->m_varStr.IsEmpty() )
				{
					context.m_strErrMsg = ((ECSString*)pError)->m_varStr ;
					err = ESLErrorMsg( context.m_strErrMsg ) ;
					delete	pError ;
				}
				if ( !err )
				{
					context.SetStatus( context.xsExecution ) ;
					context.delete_CSObject( obj ) ;
				}
				delete	pError ;
			}
			return	err ;
		}
		ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
		if ( pParent != NULL )
		{
			return	pParent->Operate( context, csopType, obj ) ;
		}
	}
	return	ESLErrorMsg( "構造体への定義されていない演算操作です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	if ( m_pClassInf == NULL )
	{
		DWORD *	pdwFuncAddr =
			context.m_pcsxi->GetFunctionAddress
				( EWideString( m_pwszTag ) + L"::@Compare" ) ;
		if ( pdwFuncAddr != NULL )
		{
			//
			// オーバーロードスクリプト関数呼び出し
			//
			ECSObjArray<ECSObject>	lstArg ;
			ECSInteger *			pResult = context.new_CSInteger( ) ;
			lstArg.Add( context.new_CSReference( this ) ) ;
			lstArg.Add( context.new_CSReference( pResult ) ) ;
			lstArg.Add( context.new_CSInteger( cscpType ) ) ;
			lstArg.Add( context.new_CSReference( &obj ) ) ;
			context.PushObject( pResult ) ;
			context.PushObject
				( context.new_CSInteger( - (long int) context.m_ip ) ) ;
			ESLError	err = context.CallFunction( *pdwFuncAddr, lstArg ) ;
			if ( !err )
			{
				ECSObject *	pError = context.PopObject( ) ;
				ESLVerify( pResult == context.PopObject( ) ) ;
				nResult = pResult->GetInt() ;
				if ( (pError->m_vtType == csvtInteger)
					&& (((ECSInteger*)pError)->GetValue() != 0) )
				{
					err = ESLErrorMsg
						( "@Compare オーバーロード関数がエラーを返しました。" ) ;
				}
				else if ( (pError->m_vtType == csvtString)
						&& !((ECSString*)pError)->m_varStr.IsEmpty() )
				{
					context.m_strErrMsg = ((ECSString*)pError)->m_varStr ;
					err = ESLErrorMsg( context.m_strErrMsg ) ;
				}
				if ( !err )
				{
					context.SetStatus( context.xsExecution ) ;
				}
			}
			return	err ;
		}
		ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
		if ( pParent != NULL )
		{
			return	pParent->Compare( context, nResult, cscpType, obj ) ;
		}
	}
	return	ESLErrorMsg( "構造体への定義されていない比較操作です。" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::GetVariableIndex( int & nIndex, int iMember )
{
	if ( m_pClassInf != NULL )
	{
		nIndex = iMember ;
		return	eslErrSuccess ;
	}
	ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
	if ( pParent != NULL )
	{
		ESLError	err =
			pParent->GetVariableIndex( nIndex, iMember ) ;
		if ( !err )
		{
			nIndex += m_varArray.GetSize() ;
		}
		return	err ;
	}
	return	ESLErrorMsg( "未定義のメンバ変数を参照しています。" ) ;
}

ESLError ECSStructure::GetVariableIndex
				( int & nIndex, const wchar_t * pwszMember )
{
	nIndex = FindVariableIndex( pwszMember ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	if ( m_pClassInf == NULL )
	{
		ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
		if ( pParent != NULL )
		{
			ESLError	err =
					pParent->GetVariableIndex( nIndex, pwszMember ) ;
			if ( err )
			{
				return	err ;
			}
			nIndex += m_varArray.GetSize( ) ;
			return	eslErrSuccess ;
		}
	}
	return	ESLErrorMsg( "未定義のメンバ変数を参照しています。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSStructure::GetVariableAt( int nIndex )
{
	if ( (m_pClassInf == NULL) && (nIndex >= (int) m_varArray.GetSize()) )
	{
		ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
		if ( pParent == NULL )
		{
			return	NULL ;
		}
		return	pParent->GetVariableAt( nIndex - m_varArray.GetSize() ) ;
	}
	return	m_varArray.GetAt( nIndex ) ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSStructure::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	ECSWideString	wstrFuncName = m_pwszTag ;
	wstrFuncName += L"::" ;
	wstrFuncName += pwszName ;
	//
	if ( context.m_pcsxi == nullptr )
	{
		return	ESLErrorMsg( "実行イメージのない関数呼び出しです。" ) ;
	}
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
ESLError ECSStructure::GetFunctionPointer
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
		if ( pFunc->m_pClassCast->pClassInf->GetAttribute()
								& ECSTypeInfo::flagNativeObject )
		{
			fptr = pFunc->m_fpFuncPointer ;
			fptr.m_castThis =
				ECS_CAST_INTERFACE
					( m_varArray.GetAt( fptr.m_castThis.iNativeParent ) ) ;
		}
		else
		{
			fptr = pFunc->m_fpFuncPointer ;
			fptr.m_castThis.pCastObject = this ;
		}
	}
	else
	{
		ECSWideString	wstrFuncName = m_pwszTag ;
		wstrFuncName += L"::" ;
		wstrFuncName += pwszName ;
		//
		DWORD *	pdwFuncAddr =
			context.m_pcsxi->GetFunctionAddress( wstrFuncName ) ;
		if ( pdwFuncAddr == NULL )
		{
			ECSObject *	pParent = GetDynamicVariableAs( L"parent" ) ;
			if ( pParent != NULL )
			{
				return	pParent->GetFunctionPointer
								( context, fptr, pwszName ) ;
			}
			return	ESLErrorMsg
				( "構造体の定義されていない関数を呼び出しています。" ) ;
		}
		fptr.m_ftType = ECS_FUNCTION_POINTER::funcScriptCall ;
		fptr.m_castThis = ECS_CAST_INTERFACE( this ) ;
		fptr.m_varFunc.addrScript = *pdwFuncAddr ;
	}
	return	eslErrSuccess ;
}

ESLError ECSStructure::GetFunctionPointer
	( ECSContext & context, ECS_FUNCTION_POINTER & fptr, int nIndex )
{
	if ( m_pClassInf == NULL )
	{
		return	ECSArray::GetFunctionPointer( context, fptr, nIndex ) ;
	}
	ECSClassInfo::MemberFunction *
		pFunc = m_pClassInf->GetFunctionAt( nIndex ) ;
	if ( pFunc == NULL )
	{
		return	ESLErrorMsg
			( "定義されていないクラスのメンバ関数を呼び出しています。" ) ;
	}
	if ( pFunc->m_pClassCast->pClassInf->GetAttribute()
							& ECSTypeInfo::flagNativeObject )
	{
		fptr = pFunc->m_fpFuncPointer ;
		fptr.m_castThis =
			ECS_CAST_INTERFACE
				( m_varArray.GetAt( fptr.m_castThis.iNativeParent ) ) ;
	}
	else
	{
		fptr = pFunc->m_fpFuncPointer ;
		fptr.m_castThis.pCastObject = this ;
		ESLAssert( fptr.m_ftType == ECS_FUNCTION_POINTER::funcScriptCall ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::CallFunction
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
	if ( &lstArg != &(context.m_arg.m_varArray) )
	{
		context.m_arg.m_varArray.RemoveAll( ) ;
	//	context.m_arg.m_varArray.Add( context.new_CSReference( this ) ) ;
		for ( int i = 0; i < (int) lstArg.GetSize(); i ++ )
		{
			ECSObject *	pObj = lstArg.GetAt( i ) ;
			if ( pObj != NULL )
			{
				if ( pObj->m_vtType == csvtReference )
				{
					ECSReference *	pRef = context.new_CSReference() ;
					pRef->SetReferenceCastInterface
						( ((ECSReference*)pObj)->m_pRef,
								&context, *((ECSReference*)pObj) ) ;
					context.m_arg.m_varArray.Add( pRef ) ;
				}
				else if ( pObj->m_vtType == csvtPointerReference )
				{
					ECSPointerReference *	pPtrRef =
						context.new_CSPointerReference( *((ECSPointerReference*)pObj) ) ;
					context.m_arg.m_varArray.Add( pPtrRef ) ;
				}
				else
				{
					context.m_arg.m_varArray.Add( pObj->Duplicate() ) ;
				}
			}
		}
	}
	return	eslErrSuccess ;
}

// 特殊演算子 : interface 型変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::OperateCastInterface
		( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName )
{
	if ( m_pClassInf != NULL )
	{
		ECSClassInfo::CastInfo *
			pCast = m_pClassInf->GetCastClassInfoAs( pwszTypeName ) ;
		if ( pCast != NULL )
		{
			ci = *pCast ;
			ci.pCastObject = this ;
			return	eslErrSuccess ;
		}
	}
	return	ECSArray::OperateCastInterface( ci, pwszTypeName ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSArray::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_staMember.SaveArray( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSArray::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_staMember.LoadArray( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	int		i, nLength ;
	nLength = m_varArray.GetSize( ) ;
	//
	EString	strDump ;
	strDump = "要素数 = " ;
	strDump += EString( nLength ) ;
	strDump += "\r\n" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	for ( i = 0; i < nLength; i ++ )
	{
		const wchar_t *	pwszTagName = GetVariableName( i ) ;
		if ( pwszTagName == NULL )
		{
			continue ;
		}
		ECSObject *	pObj = m_varArray.GetAt( i ) ;
		//
		strDump = EString("\t") * nIndent + "\t[" + EString(i) + "] " ;
		strDump += "\"" + EString(pwszTagName) + "\" : " ;
		//
		if ( pObj != NULL )
		{
			strDump += EString( pObj->GetTypeName() ) ;
			strDump += " : " ;
			buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			ESLError	err =
				pObj->DumpObject( buf, nIndent + 1, context ) ;
			if ( err )
			{
				return	err ;
			}
			strDump = "" ;
		}
		else
		{
			strDump += "<nothing>" ;
		}
		if ( i + 1 < nLength )
		{
			strDump += "\r\n" ;
		}
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	}
	return	eslErrSuccess ;
}

// スクリプトのデストラクタ
//////////////////////////////////////////////////////////////////////////////
void ECSStructure::OnDestruction( ECSContext & context )
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
			ECSReference *	pRefThis = context.new_CSReference() ;
			pRefThis->SetReferenceCastInterface
							( this, &context, fptr.m_castThis ) ;
			//
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
				pSysContext->CallFunction
						( fptr.m_varFunc.addrScript, arg.m_varArray ) ;
				//
				arg.RemoveBetween( context ) ;
			}
			pSysContext->m_ip = ipSaved ;
		}
	}
	ECSArray::OnDestruction( context ) ;
}

// メンバを取得（動的構造体）
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSStructure::GetDynamicVariableAs( const wchar_t * pwszMember )
{
	ESLAssert( m_pClassInf == NULL ) ;
	int	iMember = m_staMember.FindIndex( pwszMember ) ;
	if ( iMember >= 0 )
	{
		return	m_varArray.GetAt( iMember ) ;
	}
	return	NULL ;
}

// メンバ変数の指標を取得
//////////////////////////////////////////////////////////////////////////////
int ECSStructure::FindVariableIndex( const wchar_t * pwszMember ) const
{
	if ( m_pClassInf != NULL )
	{
		return	m_pClassInf->GetVariableIndex( pwszMember ) ;
	}
	else
	{
		return	m_staMember.FindIndex( pwszMember ) ;
	}
	return	-1 ;
}

// メンバ変数名を取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSStructure::GetVariableName( int nIndex ) const
{
	if ( m_pClassInf != NULL )
	{
		return	m_pClassInf->GetVariableNameAt( nIndex ) ;
	}
	else
	{
		return	m_staMember.GetAt( nIndex ) ;
	}
	return	NULL ;
}

// オブジェクト代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::MoveFromStructure
			( ECSContext & context, const ECSStructure & obj )
{
	ESLError	err ;
	if ( obj.m_pClassInf == m_pClassInf )
	{
		for ( int i = 0; i < (int) obj.m_varArray.GetSize(); i ++ )
		{
			err = MoveMemberAt( context, i, obj.m_varArray.GetAt(i) ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	else
	{
		for ( int i = 0; i < (int) obj.m_varArray.GetSize(); i ++ )
		{
			const wchar_t *	pwszName = obj.GetVariableName( i ) ;
			if ( pwszName != NULL )
			{
				err = MoveMemberAs
					( context, pwszName, obj.m_varArray.GetAt(i) ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
	}
	return	eslErrSuccess ;
}

// オブジェクト代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::MoveFromHash
		( ECSContext & context, const ECSHash & obj )
{
	for ( int i = 0; i < (int) obj.m_varArray.GetSize(); i ++ )
	{
		ECSWideString *	pTagName = obj.m_varArray.GetTagAt( i ) ;
		if ( pTagName != NULL )
		{
			ESLError	err = MoveMemberAs
				( context, *pTagName, obj.m_varArray.GetObjectAt(i) ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	return	eslErrSuccess ;
}

// メンバへオブジェクト代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::MoveMemberAs
	( ECSContext & context, const wchar_t * pwszName, ECSObject * obj )
{
	int	iMember = FindVariableIndex( pwszName ) ;
	if ( iMember >= 0 )
	{
		return	MoveMemberAt( context, iMember, obj ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSStructure::MoveMemberAt
	( ECSContext & context, int iMember, ECSObject * obj )
{
	if ( obj == NULL )
	{
		return	eslErrSuccess ;
	}
	ECSObject *	pDstObj = m_varArray.GetAt( iMember ) ;
	if ( pDstObj == NULL )
	{
		return	eslErrSuccess ;
	}
	ESLError		err = eslErrSuccess ;
	ECSReference *	pRef ;
	if ( pDstObj->m_vtType == obj->m_vtType )
	{
		switch ( pDstObj->m_vtType )
		{
		case	csvtInteger:
			((ECSInteger*)pDstObj)->SetValue
						( ((ECSInteger*)obj)->GetValue() ) ;
			break ;
		case	csvtReal:
			((ECSReal*)pDstObj)->m_varReal = ((ECSReal*)obj)->m_varReal ;
			break ;
		case	csvtString:
			((ECSString*)pDstObj)->m_varStr = ((ECSString*)obj)->m_varStr ;
			break ;
		default:
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
			pRef = context.new_CSReference( obj ) ;
			err = pDstObj->Move( context, pRef ) ;
			break ;
		}
	}
	else
	{
		pRef = context.new_CSReference( obj ) ;
		err = pDstObj->Move( context, pRef ) ;
	}
	if ( err )
	{
		context.delete_CSObject( pRef ) ;
	}
	return	err ;
}

// メンバ変数追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStructure::AddNewVariable
	( const wchar_t * pwszName, ECSObject * pObj )
{
	if ( m_pClassInf != NULL )
	{
		return	ESLErrorMsg
			( "静的クラスに対して動的にメンバを構築しようとしています。" ) ;
	}
	int	i = FindVariableIndex( pwszName ) ;
	if ( i >= 0 )
	{
//		m_varArray.SetAt( i, pObj ) ;
		return	ESLErrorMsg( "二重にメンバ変数を定義しています。" ) ;
	}
	else
	{
		m_varArray.Add( pObj ) ;
		m_staMember.Add( pwszName ) ;
	}
	ESLAssert( m_varArray.GetSize() == m_staMember.GetSize() ) ;
	//
	return	eslErrSuccess ;
}

// ECSObject インスタンス
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSStructure::GetInstanceObject( void )
{
	return	this ;
}

// メンバを取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSStructure::GetMemberAs( const wchar_t * pwszMember )
{
	int	iMember = FindVariableIndex( pwszMember ) ;
	if ( iMember >= 0 )
	{
		return	m_varArray.GetAt( iMember ) ;
	}
	return	NULL ;
}

// 整数のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
int ECSStructure::GetMemberAsInt( const wchar_t * pwszName, int nDefValue )
{
	ECSObject *	pObj = GetMemberAs( pwszName ) ;
	if ( pObj != NULL )
	{
		switch ( pObj->m_vtType )
		{
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
			break ;
		case	csvtInteger:
			return	((ECSInteger*)pObj)->GetInt() ;
		case	csvtReal:
			return	(int) eriRoundR64ToLInt( ((ECSReal*)pObj)->m_varReal ) ;
		case	csvtString:
			return	((ECSString*)pObj)->m_varStr.AsInteger() ;
		}
	}
	return	nDefValue ;
}

// 実数のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
double ECSStructure::GetMemberAsReal
	( const wchar_t * pwszName, double rDefValue )
{
	ECSObject *	pObj = GetMemberAs( pwszName ) ;
	if ( pObj != NULL )
	{
		switch ( pObj->m_vtType )
		{
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
			break ;
		case	csvtInteger:
			return	(double) ((ECSInteger*)pObj)->GetValue() ;
		case	csvtReal:
			return	((ECSReal*)pObj)->m_varReal ;
		case	csvtString:
			return	((ECSString*)pObj)->m_varStr.AsReal() ;
		}
	}
	return	rDefValue ;
}

// 文字列のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
ECSWideString ECSStructure::GetMemberAsStr
	( const wchar_t * pwszName, const wchar_t * pwszDefValue )
{
	ECSObject *	pObj = GetMemberAs( pwszName ) ;
	ECSWideString	wstrResult ;
	if ( pObj != NULL )
	{
		switch ( pObj->m_vtType )
		{
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
			break ;
		case	csvtInteger:
			wstrResult.FromInteger( ((ECSInteger*)pObj)->GetValue() ) ;
			return	wstrResult ;
		case	csvtReal:
			wstrResult.FromReal( ((ECSReal*)pObj)->m_varReal ) ;
			return	wstrResult ;
		case	csvtString:
			return	((ECSString*)pObj)->m_varStr ;
		}
	}
	return	pwszDefValue ;
}

// 整数のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSStructure::SetMemberAsInt( const wchar_t * pwszName, int nValue )
{
	ECSObject *	pObj = GetMemberAs( pwszName ) ;
	if ( pObj != NULL )
	{
		switch ( pObj->m_vtType )
		{
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
			break ;
		case	csvtInteger:
			((ECSInteger*)pObj)->SetValue( nValue ) ;
			break ;
		case	csvtReal:
			((ECSReal*)pObj)->m_varReal = nValue ;
			break ;
		case	csvtString:
			((ECSString*)pObj)->m_varStr.FromInteger( nValue ) ;
			break ;
		}
	}
	else if ( m_pClassInf == NULL )
	{
		AddNewVariable( pwszName, new ECSInteger( nValue ) ) ;
	}
}

// 実数のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSStructure::SetMemberAsReal( const wchar_t * pwszName, double rValue )
{
	ECSObject *	pObj = GetMemberAs( pwszName ) ;
	if ( pObj != NULL )
	{
		switch ( pObj->m_vtType )
		{
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
			break ;
		case	csvtInteger:
			((ECSInteger*)pObj)->SetValue( eriRoundR64ToLInt( rValue ) ) ;
			break ;
		case	csvtReal:
			((ECSReal*)pObj)->m_varReal = rValue ;
			break ;
		case	csvtString:
			((ECSString*)pObj)->m_varStr.FromReal( rValue ) ;
			break ;
		}
	}
	else if ( m_pClassInf == NULL )
	{
		AddNewVariable( pwszName, new ECSReal( rValue ) ) ;
	}
}

// 文字列のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSStructure::SetMemberAsStr
	( const wchar_t * pwszName, const wchar_t * pwszValue )
{
	ECSObject *	pObj = GetMemberAs( pwszName ) ;
	if ( pObj != NULL )
	{
		ECSSourceStream	cssStr ;
		switch ( pObj->m_vtType )
		{
		case	csvtObject:
		case	csvtReference:
		case	csvtArray:
		case	csvtHash:
			break ;
		case	csvtInteger:
			cssStr = pwszValue ;
			((ECSInteger*)pObj)->SetValue( cssStr.GetLargeInteger() ) ;
			break ;
		case	csvtReal:
			cssStr = pwszValue ;
			((ECSReal*)pObj)->m_varReal = cssStr.GetRealNumber() ;
			break ;
		case	csvtString:
			((ECSString*)pObj)->m_varStr = pwszValue ;
			break ;
		}
	}
	else if ( m_pClassInf == NULL )
	{
		AddNewVariable( pwszName, new ECSString( pwszValue ) ) ;
	}
}
