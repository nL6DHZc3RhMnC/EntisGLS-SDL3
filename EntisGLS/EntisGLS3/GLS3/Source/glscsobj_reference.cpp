
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 参照オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSReference, ECSObject )

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSReference::GetTypeName( void ) const
{
	return	L"Reference" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSReference::Duplicate( void )
{
	if ( !m_pRef->IsValidObject() )
	{
		ECSReference *	pRef = new ECSReference ;
		pRef->m_pRef = NULL ;
		pRef->m_pRefParent = m_pRefParent ;
		pRef->m_iParentRef = m_iParentRef ;
		return	pRef ;
	}
	if ( m_fNontemp )
	{
		return	new ECSReference( m_pRef ) ;
	}
	return	m_pRef->Duplicate( ) ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Move( ECSContext & context, ECSObject * obj )
{
	if ( m_pRef->IsValidObject() )
	{
		//
		// 参照先への代入操作
		//
		return	m_pRef->Move( context, obj ) ;
	}
	ESLAssert( m_pRef == NULL ) ;
	//
	if ( !m_pRefParent->IsValidObject() )
	{
		ESLAssert( m_pRefParent == NULL ) ;
		//
		if ( obj->m_vtType != csvtReference )
		{
			//
			// Reference 所有オブジェクト設定
			//
			ESLAssert( m_pOwnObj == NULL ) ;
			ESLAssert( m_pRefParent == NULL ) ;
			m_pRef = m_pOwnObj = obj ;
			AddReferenceBackLinkChain( ) ;
			return	eslErrSuccess ;
		}
		//
		// Reference -> Reference 代入
		//
		ESLAssert( obj->m_vtType == csvtReference ) ;
		ECSReference *	pObj = (ECSReference*) obj ;
		ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
		if ( pEntity != NULL )
		{
			SetReferenceCastInterface( pEntity, &context, *pObj ) ;
		}
		else
		{
			SetReference
				( pObj->m_pRef, &context,
					pObj->m_iVarOffset,
						pObj->m_nVarBounds, pObj->m_iFuncOffset,
					pObj->m_pRefParent, pObj->m_iParentRef ) ;
		}
		context.delete_CSObject( obj ) ;
		return	eslErrSuccess ;
	}
	//
	// 配列要素へのアクセス
	//
	ECSObject *	pRefParent = m_pRefParent ;
	int			iParentRef = m_iParentRef ;
	SetReference( pRefParent->GetVariableAt( iParentRef ), &context ) ;
	if ( m_pRef != NULL )
	{
		return	m_pRef->Move( context, obj ) ;
	}
	//
	// 配列要素への設定
	//
	if ( obj->m_vtType == csvtReference )
	{
		ECSReference *	prefSrc = (ECSReference*) obj ;
		if ( prefSrc->m_pRef->IsValidObject() )
		{
			//
			// アイテムの所有 Reference オブジェクトを検索
			//
			ECSReference *	prefNext = prefSrc ;
			ECSReference *	prefOwner = NULL ;
			for ( ; ; )
			{
				if ( prefNext->m_pOwnObj != NULL )
				{
					prefOwner = prefNext ;
					break ;
				}
				ECSObject *	pNextRef = prefNext->m_pRef ;
				if ( !pNextRef->IsValidObject() )
				{
					break ;
				}
				if ( pNextRef->m_vtType != csvtReference )
				{
					ECSReference *	pBackRef = pNextRef->m_pBackRef ;
					while ( pBackRef != NULL )
					{
						if ( pBackRef->m_pOwnObj != NULL )
						{
							prefOwner = pBackRef ;
							break ;
						}
						pBackRef = pBackRef->m_pNextBackRef ;
					}
					break ;
				}
				prefNext = (ECSReference*) pNextRef ;
			}
			if ( prefOwner != NULL )
			{
				//
				// 参照元オブジェクトの所有アイテムを配列に移動
				//
				ESLAssert( prefOwner->m_pOwnObj != NULL ) ;
				obj = prefOwner->DetachObject( context ) ;
				prefOwner->SetReference( obj ) ;
				context.delete_CSObject( prefSrc ) ;
			}
			else // if ( prefSrc->m_pRef->IsValidObject() )
			{
				//
				// 参照オブジェクトの複製を配列に設定
				//
				ESLAssert( prefSrc->m_pRef->IsValidObject() ) ;
				obj = prefSrc->m_pRef->Duplicate( ) ;
				context.delete_CSObject( prefSrc ) ;
			}
		}
	}
	SetReference( pRefParent->SetVariableAt( iParentRef, obj ), &context ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"単項演算子を実行しようとしています。" ) ;
	}
	ESLError	err = m_pRef->UnaryOperate( context, csuopType ) ;
	if ( !err )
	{
		if ( m_pRef->m_pResult != NULL )
		{
			m_pResult = m_pRef->m_pResult ;
			m_pRef->m_pResult = NULL ;
		}
	}
	return	err ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"二項演算子を実行しようとしています。" ) ;
	}
	ESLError	err = m_pRef->Operate( context, csopType, obj ) ;
	if ( !err )
	{
		if ( m_pRef->m_pResult != NULL )
		{
			m_pResult = m_pRef->m_pResult ;
			m_pRef->m_pResult = NULL ;
		}
	}
	return	err ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"比較演算子を実行しようとしています。" ) ;
	}
	if ( cscpType == csctNotEqualPointer )
	{
		nResult = - (int) (ECSObject::GetEntity(this) != ECSObject::GetEntity(&obj)) ;
		return	eslErrSuccess ;
	}
	else if ( cscpType == csctEqualPointer )
	{
		nResult = - (int) (ECSObject::GetEntity(this) == ECSObject::GetEntity(&obj)) ;
		return	eslErrSuccess ;
	}
	return	m_pRef->Compare( context, nResult, cscpType, obj ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::GetVariableIndex( int & nIndex, int iMember )
{
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"メンバ変数を参照しようとしています。" ) ;
	}
	return	m_pRef->GetVariableIndex( nIndex, iMember + m_iVarOffset ) ;
}

ESLError ECSReference::GetVariableIndex( int & nIndex, const wchar_t * pwszMember )
{
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"メンバ変数を参照しようとしています。" ) ;
	}
	return	m_pRef->GetVariableIndex( nIndex, pwszMember ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSReference::GetVariableAt( int nIndex )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->GetVariableAt( nIndex ) ;
	}
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSReference::SetVariableAt( int nIndex, ECSObject * obj )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->SetVariableAt( nIndex, obj ) ;
	}
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"メンバ関数を呼び出そうとしています。" ) ;
	}
	ESLError	err = m_pRef->GetFunction( context, nIndex, pwszName ) ;
	nIndex += m_staFuncName->GetSize( ) ;
	return	err ;
}

// メンバ関数ポインタ取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::GetFunctionPointer
	( ECSContext & context,
		ECS_FUNCTION_POINTER & fptr, const wchar_t * pwszName )
{
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"メンバ関数を呼び出そうとしています。" ) ;
	}
	return	m_pRef->GetFunctionPointer( context, fptr, pwszName ) ;
}

ESLError ECSReference::GetFunctionPointer
	( ECSContext & context, ECS_FUNCTION_POINTER & fptr, int nIndex )
{
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"メンバ関数を呼び出そうとしています。" ) ;
	}
	return	m_pRef->GetFunctionPointer
					( context, fptr, nIndex + m_iFuncOffset ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (nIndex >= 0) && (nIndex < (int) m_staFuncName->GetSize()) )
	{
		return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
	}
	if ( !m_pRef->IsValidObject() )
	{
		return	ESLErrorMsg
			( "参照先のないオブジェクトに対して"
				"メンバ関数を呼び出そうとしています。" ) ;
	}
	nIndex -= m_staFuncName->GetSize( ) ;
	return	m_pRef->CallFunction( context, nIndex, lstArg ) ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::OperateBoolean( int & nBoolean )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->OperateBoolean( nBoolean ) ;
	}
	return	ECSObject::OperateBoolean( nBoolean ) ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::OperateSizeOf( INT64 & nSize )
{
	if ( m_pRef->IsValidObject() )
	{
		ESLError	err = m_pRef->OperateSizeOf( nSize ) ;
		nSize -= m_iVarOffset ;
		if ( (m_nVarBounds >= 0) && (m_nVarBounds < nSize) )
		{
			nSize = m_nVarBounds ;
		}
		return	err ;
	}
	return	ECSObject::OperateSizeOf( nSize ) ;
}

// 特殊演算子 : typeof
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSReference::OperateTypeOf( void ) const
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->OperateTypeOf() ;
	}
	return	NULL ;
}

// 特殊演算子 : interface 型変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::OperateCastInterface
		( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->OperateCastInterface( ci, pwszTypeName ) ;
	}
	return	ECSObject::OperateCastInterface( ci, pwszTypeName ) ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::OperateInteger( INT64 & nValue )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->OperateInteger( nValue ) ;
	}
	else
	{
		return	ESLErrorMsg( "参照先の無い整数値ロードです" ) ;
	}
}

// 実数取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::OperateReal( REAL64 & nValue )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->OperateReal( nValue ) ;
	}
	else
	{
		return	ESLErrorMsg( "参照先の無い実数値ロードです" ) ;
	}
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::OperateString( EWideString & wstrValue )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->OperateString( wstrValue ) ;
	}
	else
	{
		return	ESLErrorMsg( "参照先の無い文字列への変換です" ) ;
	}
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSReference::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->GetBuffer( iOffset, nSize, fWritable ) ;
	}
	else
	{
		return	ECSObject::GetBuffer( iOffset, nSize, fWritable ) ;
	}
}

void ECSReference::FlushBuffer
	( int iOffset, int nSize, void * ptrBuf, bool fModified )
{
	if ( m_pRef->IsValidObject() )
	{
		m_pRef->FlushBuffer( iOffset, nSize, ptrBuf, fModified ) ;
	}
	else
	{
		ECSObject::FlushBuffer( iOffset, nSize, ptrBuf, fModified ) ;
	}
}

ECSSakura2Processor::LinearAddressCache *
	ECSReference::GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->GetSegmentBuffer( seg ) ;
	}
	else
	{
		return	ECSObject::GetSegmentBuffer( seg ) ;
	}
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSReference::IndexAllMember( void )
{
	if ( m_pOwnObj != NULL )
	{
		m_pOwnObj->IndexAllMember( ) ;
		m_pOwnObj->m_pParent = this ;
		m_pOwnObj->m_nIndex = -1 ;
	}
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSReference::CleanupAllReference( ECSContext & context )
{
	CleanupAllBackReference( ) ;
	//
	if ( m_pOwnObj != NULL )
	{
		ReleaseOwnObject( &context ) ;
	}
	else
	{
		DetachBackLinkChain( ) ;
	}
	//
	m_pRefParent = NULL ;
	m_iParentRef = 0 ;
	//
	ECSObject::CleanupAllReference( context ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::CommitAllReference( ECSContext & context )
{
	if ( m_pOwnObj != NULL )
	{
		AddReferenceBackLinkChain( ) ;
		return	m_pOwnObj->CommitAllReference( context ) ;
	}
	SetReference
		( NULL, &context, m_iVarOffset, m_nVarBounds, m_iFuncOffset ) ;
	//
	int	iDimFirst = 0 ;
	switch ( m_omBaseMode )
	{
	case	csomStack:
		m_pRef = &(context.m_stack) ;
		break ;
	case	csomGlobal:
		m_pRef = &(context.m_pcsxi->m_csgGlobal) ;
		break ;
	case	csomData:
		m_pRef = &(context.m_pcsxi->m_csgData) ;
		break ;
	case	csomArgument:
		m_pRef = &(context.m_arg) ;
		break ;
	case	csomHeap:
		m_pRef = ESLTypeCast<ECSObject>
				( context.m_pcsxi->
					m_heapGlobal.GetAt(m_dimIndex.GetAt(iDimFirst ++)) ) ;
		break ;
	case	csomHeapShared:
		m_pRef = ESLTypeCast<ECSObject>
				( context.m_pcsxi->
					m_heapShared.GetAt(m_dimIndex.GetAt(iDimFirst ++)) ) ;
		break ;
	case	csomImmediate:
		return	eslErrSuccess ;
	default:
		return	ESLErrorMsg( "不正な記憶クラスです。" ) ;
	}
	//
	for ( int i = iDimFirst; i < (int) m_dimIndex.GetSize(); i ++ )
	{
		m_pRefParent = m_pRef ;
		m_iParentRef = m_dimIndex.GetAt(i) ;
		if ( (m_pRef->m_vtType == csvtReference) & (m_iParentRef == -1) )
		{
			m_pRef = ((ECSReference*)m_pRef)->m_pOwnObj ;
		}
		else
		{
			m_pRef = m_pRef->GetVariableAt( m_iParentRef ) ;
		}
		if ( m_pRef == NULL )
		{
			if ( (i + 1) == (int) m_dimIndex.GetSize() )
			{
				m_pRef = NULL ;
				return	eslErrSuccess ;
			}
			m_pRefParent = NULL ;
			return	ESLErrorMsg( "参照先オブジェクトが見つかりません。" ) ;
		}
	}
	m_dimIndex.RemoveAll( ) ;
	//
	AddReferenceBackLinkChain( ) ;
	//
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = CommitGlobalReference( context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	file.Write( &m_iVarOffset, sizeof(m_iVarOffset) ) ;
	file.Write( &m_nVarBounds, sizeof(m_nVarBounds) ) ;
	file.Write( &m_iFuncOffset, sizeof(m_iFuncOffset) ) ;
	//
	DWORD	dwDimCount = m_dimIndex.GetSize( ) ;
	file.Write( &m_omBaseMode, sizeof(m_omBaseMode) ) ;
	file.Write( &dwDimCount, sizeof(DWORD) ) ;
	for ( DWORD i = 0; i < dwDimCount; i ++ )
	{
		int	nIndex = m_dimIndex.GetAt( i ) ;
		file.Write( &nIndex, sizeof(nIndex) ) ;
	}
	m_dimIndex.RemoveAll( ) ;
	//
	return	context.SaveObject( file, m_pOwnObj ) ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Load( ESLFileObject & file, ECSContext & context )
{
	file.Read( &m_iVarOffset, sizeof(m_iVarOffset) ) ;
	file.Read( &m_nVarBounds, sizeof(m_nVarBounds) ) ;
	file.Read( &m_iFuncOffset, sizeof(m_iFuncOffset) ) ;
	//
	DWORD	dwDimCount = 0 ;
	file.Read( &m_omBaseMode, sizeof(m_omBaseMode) ) ;
	file.Read( &dwDimCount, sizeof(DWORD) ) ;
	m_dimIndex.RemoveAll( ) ;
	for ( DWORD i = 0; i < dwDimCount; i ++ )
	{
		int	nIndex ;
		if ( file.Read( &nIndex, sizeof(nIndex) ) < sizeof(nIndex) )
		{
			return	ESLErrorMsg( "データの読み込みに失敗しました。" ) ;
		}
		m_dimIndex.Add( nIndex ) ;
	}
	//
	ESLError	err ;
	SetReference
		( NULL, &context, m_iVarOffset, m_nVarBounds, m_iFuncOffset ) ;
	err = context.LoadObject( file, m_pOwnObj ) ;
	if ( err )
	{
		return	err ;
	}
	m_pRef = m_pOwnObj ;
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = FormatReferenceObjectIndex( context ) ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	if ( m_pOwnObj != NULL )
	{
		return	m_pOwnObj->DumpObject( buf, nIndent, context ) ;
	}
	return	eslErrSuccess ;
}

// 参照オブジェクトインデックス表記を取得する
//////////////////////////////////////////////////////////////////////////////
EString ECSReference::FormatReferenceObjectIndex( ECSContext & context )
{
	EString	strDump ;
	if ( m_pRef->IsValidObject() )
	{
		strDump += "Type = " ;
		try
		{
			const wchar_t *	pwszTypeName = m_pRef->GetTypeName() ;
			strDump += EString( pwszTypeName ) ;
		}
		catch ( ... )
		{
			return	"" ;
		}
		//
		if ( m_pOwnObj != NULL )
		{
			strDump += " : " ;
			return	strDump ;
		}
		else
		{
			ESLError	err = CommitGlobalReference( context ) ;
			if ( !err )
			{
				switch ( m_omBaseMode )
				{
				case	csomImmediate:
					strDump = "[nothing]" ;
					break ;
				case	csomStack:
					strDump = "[stack]" ;
					break ;
				case	csomGlobal:
					strDump = "[global]" ;
					break ;
				case	csomData:
					strDump = "[data]" ;
					break ;
				case	csomArgument:
					strDump = "[arg]" ;
					break ;
				case	csomHeap:
					strDump = "[heap]" ;
					break ;
				case	csomHeapShared:
					strDump = "[shared heap]" ;
					break ;
				}
				for ( int i = 0; i < (int) m_dimIndex.GetSize(); i ++ )
				{
					strDump += '[' ;
					strDump += EString( m_dimIndex.GetAt(i) ) ;
					strDump += ']' ;
				}
				m_dimIndex.RemoveAll( ) ;
			}
		}
	}
	else
	{
		strDump = "<null>" ;
	}
	return	strDump ;
}

// 大域的参照解決
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::CommitGlobalReference( ECSContext & context )
{
	ECSObject *	pRef = m_pRef ;
	m_dimIndex.RemoveAll( ) ;
	//
	if ( m_pOwnObj != NULL )
	{
		m_omBaseMode = csomImmediate ;
		return	eslErrSuccess ;
	}
	if ( !pRef->IsValidObject() )
	{
		pRef = m_pRefParent ;
		if ( pRef->IsValidObject() )
		{
			m_dimIndex.InsertAt( 0, m_iParentRef ) ;
		}
		else
		{
			pRef = NULL ;
		}
	}
	if ( pRef == NULL )
	{
		m_omBaseMode = csomImmediate ;
	}
	else
	{
		while ( pRef->m_pParent != NULL )
		{
			m_dimIndex.InsertAt( 0, pRef->m_nIndex ) ;
			pRef = pRef->m_pParent ;
		}
		if ( pRef == &(context.m_stack) )
		{
			m_omBaseMode = csomStack ;
		}
		else if ( pRef == &(context.m_pcsxi->m_csgGlobal) )
		{
			m_omBaseMode = csomGlobal ;
		}
		else if ( pRef == &(context.m_pcsxi->m_csgData) )
		{
			m_omBaseMode = csomData ;
		}
		else if ( pRef == &(context.m_arg) )
		{
			m_omBaseMode = csomArgument ;
		}
		else if ( pRef == &(context.m_pcsxi->m_csaHeap) )
		{
			m_omBaseMode = csomHeap ;
		}
		else if ( pRef == &(context.m_pcsxi->m_csaHeapShared) )
		{
			m_omBaseMode = csomHeapShared ;
		}
		else
		{
			return	ESLErrorMsg
				( "記憶クラスにないオブジェクトを参照しています。" ) ;
		}
	}
	//
	return	eslErrSuccess ;
}

// スクリプトのデストラクタ
//////////////////////////////////////////////////////////////////////////////
void ECSReference::OnDestruction( ECSContext & context )
{
	if ( m_pBackRef != NULL )
	{
		ECotophaScript::LockReference( ) ;
		CleanupAllBackReference( ) ;
		ECotophaScript::UnlockReference( ) ;
	}
	SetReference( NULL, &context ) ;
	m_fNontemp = false ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSReference::m_staFuncName = NULL ;
const wchar_t *		ECSReference::m_pwszFuncName[6] =
{
	L"GetType", L"SetReference", L"DetachReference",
	L"IsIdentity", L"Duplicate", NULL
} ;
const ECSReference::PFUNC_CALL	ECSReference::m_pfnCallFunc[5] =
{
	&ECSReference::Call_GetType,
	&ECSReference::Call_SetReference,
	&ECSReference::Call_DetachReference,
	&ECSReference::Call_IsIdentity,
	&ECSReference::Call_Duplicate,
} ;

// メンバ関数 : String GetType( )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Call_GetType
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !m_pRef->IsValidObject() )
	{
		context.PushObject( new ECSString( L"" ) ) ;
		return	eslErrSuccess ;
	}
	if ( m_pRef->m_vtType == csvtReference )
	{
		return	((ECSReference*)m_pRef)->Call_GetType( context, lstArg ) ;
	}
	context.PushObject( new ECSString( m_pRef->GetTypeName() ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : SetReference( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Call_SetReference
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSReference *	prefObj = this ;
	while ( prefObj->m_pRef->IsValidObject()
			&& (prefObj->m_pRef->m_vtType == csvtReference) )
	{
		if ( prefObj->m_fNontemp )
		{
			break ;
		}
		prefObj = (ECSReference*) (prefObj->m_pRef) ;
	}
	ECSObject *	pObj = lstArg.GetAt( 1 ) ;
	prefObj->SetReference( pObj, &context ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ変数 : object DetachReference()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Call_DetachReference
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSReference *	prefObj = this ;
	while ( prefObj->m_pRef->IsValidObject()
			&& (prefObj->m_pRef->m_vtType == csvtReference) )
	{
		if ( prefObj->m_fNontemp )
		{
			break ;
		}
		prefObj = (ECSReference*) (prefObj->m_pRef) ;
	}
	ECSObject *	pOwnObj = prefObj->DetachObject( context ) ;
	if ( pOwnObj == NULL )
	{
		return	context.PushObject( context.new_CSReference() ) ;
	}
	else
	{
		return	context.PushObject( pOwnObj ) ;
	}
}

// メンバ関数 : Integer IsIdentity( Reference object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Call_IsIdentity
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	return	context.PushObject
		( context.new_CSInteger( (ECSObject::GetEntity(this) == pObj) ? -1 : 0 ) ) ;
}

// メンバ関数 : object Duplicate()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReference::Call_Duplicate
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSObject *	pEntity = ECSObject::GetEntity( this ) ;
	if ( pEntity == NULL )
	{
		return	context.PushObject( context.new_CSReference() ) ;
	}
	return	context.PushObject( pEntity->Duplicate() ) ;
}

// 参照や所有オブジェクトを分離する
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSReference::DetachObject( ECSContext & context )
{
	ECSObject *	pOwnObj = m_pOwnObj ;
	m_pOwnObj = NULL ;
	//
	DetachBackLinkChain( ) ;
	//
	m_pRef = NULL ;
	m_pRefParent = NULL ;
	m_iParentRef = 0 ;
	//
	return	pOwnObj ;
}

// Reference オブジェクトへの参照を、参照先への参照へ正規化する
//////////////////////////////////////////////////////////////////////////////
void ECSReference::CleanupAllBackReference( void )
{
	ECSReference *	pNextRef = m_pBackRef ;
	ECSObject *		pRef = m_pRef ;
	m_pBackRef = NULL ;
	//
	while ( pNextRef != NULL )
	{
		ESLAssert( pNextRef->m_pOwnObj == NULL ) ;
		ESLAssert( pNextRef->m_pRef == this ) ;
		ECSReference *	pBackRef = pNextRef ;
		pNextRef = pNextRef->m_pNextBackRef ;
		pBackRef->m_pRef = pRef ;
		pBackRef->m_pRefParent = NULL ;
		pBackRef->m_iParentRef = 0 ;
		pBackRef->m_pNextBackRef = NULL ;
		pBackRef->m_pPrevBackRef = NULL ;
		if ( pRef != NULL )
		{
			pBackRef->AddReferenceBackLinkChain( ) ;
		}
	}
}

// 参照先からのバックリンクを更新する
//////////////////////////////////////////////////////////////////////////////
void ECSReference::UpdateReferenceBackLink( void )
{
	ESLAssert( m_pRef->IsValidObject() ) ;
	if ( m_pRef->m_vtType != csvtReference )
	{
		AddReferenceBackLinkChain( ) ;
	}
	else
	{
		ECSReference *	prefRef = (ECSReference*) m_pRef ;
		while ( prefRef->m_pRef->IsValidObject()
				&& (prefRef->m_pRef->m_vtType == csvtReference) )
		{
			if ( prefRef->m_fNontemp )
			{
				break ;
			}
			prefRef = (ECSReference*) (prefRef->m_pRef) ;
		}
		m_pRef = prefRef ;
		ESLAssert( m_pRef != this ) ;
		if ( m_pRef != NULL )
		{
			AddReferenceBackLinkChain( ) ;
		}
	}
}

// オブジェクトを所有する ECSReference を検索する
//////////////////////////////////////////////////////////////////////////////
ECSReference * ECSReference::FindReferenceOwner( ECSObject * pObj )
{
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	ECSReference *	pRefOwner = NULL ;
	ECotophaScript::LockReference( ) ;
	//
	ECSReference *	pRefOrgNext = pObj->m_pBackRef ;
	while ( pRefOrgNext != NULL )
	{
		if ( pRefOrgNext->m_pOwnObj == pObj )
		{
			pRefOwner = pRefOrgNext ;
			break ;
		}
		pRefOrgNext = pRefOrgNext->m_pNextBackRef ;
	}
	ECotophaScript::UnlockReference( ) ;
	//
	return	pRefOwner ;
}

// 参照先オブジェクトから破棄通知
//////////////////////////////////////////////////////////////////////////////
void ECSReference::OnObjectDestory( ECSObject * pObj )
{
	if ( m_pRef == pObj )
	{
		ECSReference *	pNext = m_pNextBackRef ;
		DetachBackLinkChain( ) ;
		m_pRefParent = NULL ;
		m_iParentRef = 0 ;
		//
		if ( pNext != NULL )
		{
			ESLAssert( pNext->IsValidObject() ) ;
			pNext->OnObjectDestory( pObj ) ;
		}
	}
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSReference::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_REFERENCE_INTERFACE" ) )
	{
		m_pir.pBackLink = this ;
		m_pir.pfnGetObjectEntity = PIC_GetObjectEntity ;
		m_pir.pfnSetReference = PIC_SetReference ;
		return	(ECS_REFERENCE_INTERFACE*) &m_pir ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

ECS_OBJECT * __stdcall ECSReference::PIC_GetObjectEntity
	( ECS_REFERENCE_INTERFACE * instance )
{
	PLUGIN_REFERENCE *	ppir = (PLUGIN_REFERENCE*) instance ;
	ESLAssert( &(ppir->pBackLink->m_pir) == ppir ) ;
	ECSObject *	pObj = ECSObject::GetEntity( ppir->pBackLink ) ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	return	pObj->CreateInterface( ) ;
}

void __stdcall ECSReference::PIC_SetReference
	( ECS_REFERENCE_INTERFACE * instance,
		ECS_OBJECT * pRef, ECS_CONTEXT * context )
{
	PLUGIN_REFERENCE *	ppir = (PLUGIN_REFERENCE*) instance ;
	PLUGIN_OBJECT *	ppioRef = (PLUGIN_OBJECT*) pRef ;
	ECSContext::PLUGIN_CONTEXT *
						ppic = (ECSContext::PLUGIN_CONTEXT*) context ;
	ESLAssert( &(ppir->pBackLink->m_pir) == ppir ) ;
	ESLAssert( ppioRef->pBackLink->m_ppio == pRef ) ;
	ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
	ppir->pBackLink->SetReference( ppioRef->pBackLink, ppic->pBackLink ) ;
}
