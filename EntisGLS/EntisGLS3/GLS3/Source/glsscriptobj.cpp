
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 関数ポインタオブジェクト
//////////////////////////////////////////////////////////////////////////////
/*
// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSFunction, ECSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSFunction::ECSFunction( void )
{
	m_vtType = csvtFunction ;
	m_ftType = funcIndexCall ;
	m_castThis = ECS_CAST_INTERFACE( NULL ) ;
	m_varFunc.nIndex = -1 ;
}

ECSFunction::ECSFunction
	( FunctionType ftType, DWORD_PTR dwFuncAddr, ECSObject * pThis )
{
	m_vtType = csvtFunction ;
	m_ftType = ftType ;
	m_castThis = ECS_CAST_INTERFACE( pThis ) ;
	m_varFunc.addrScript = dwFuncAddr ;
}

ECSFunction::ECSFunction( DWORD dwFuncAddr )
{
	m_vtType = csvtFunction ;
	m_ftType = funcScriptCall ;
	m_castThis = ECS_CAST_INTERFACE( NULL ) ;
	m_varFunc.addrScript = dwFuncAddr ;
}

ECSFunction::ECSFunction( const ECS_FUNCTION_POINTER & func )
{
	m_vtType = csvtFunction ;
	m_ftType = func.m_ftType ;
	m_castThis = func.m_castThis ;
	m_varFunc = func.m_varFunc ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const ECSFunction & ECSFunction::operator = ( const ECS_FUNCTION_POINTER & func )
{
	m_ftType = func.m_ftType ;
	m_castThis = func.m_castThis ;
	m_varFunc = func.m_varFunc ;
	return	*this ;
}

// スクリプト関数設定（ランタイム）
//////////////////////////////////////////////////////////////////////////////
void ECSFunction::SetFunction
		( ECSContext & context, const wchar_t * pwszFuncName )
{
	m_ftType = funcScriptCall ;
	m_castThis = ECS_CAST_INTERFACE( NULL ) ;
	m_varFunc.addrScript = -1 ;
	//
	DWORD *	pdwFuncAddr =
		context.m_pcsxi->GetFunctionAddress( pwszFuncName ) ;
	if ( pdwFuncAddr != NULL )
	{
		m_varFunc.addrScript = *pdwFuncAddr ;
	}
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSFunction::GetTypeName( void ) const
{
	return	L"Function" ;
}

ECSObject * ECSFunction::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"Function" ) )
	{
		return	this ;
	}
	return	ECSObject::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSFunction::Duplicate( void )
{
	ECSFunction *	pDup = new ECSFunction( *this ) ;
	return	pDup ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = obj->GetObjectEntity( ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	if ( pEntity->m_vtType == csvtInteger )
	{
		m_ftType = funcScriptCall ;
		m_castThis = ECS_CAST_INTERFACE( NULL ) ;
		m_varFunc.addrScript = (DWORD) ((ECSInteger*)pEntity)->GetInt() ;
	}
	else if ( pEntity->m_vtType == csvtString )
	{
		SetFunction
			( context, (const wchar_t *) ((ECSString*)pEntity)->m_varStr ) ;
	}
	else if ( pEntity->m_vtType == csvtFunction )
	{
		operator = ( *((ECSFunction*)pEntity) ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない Function 型の単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない Function 型の演算です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない Function 型の比較です。" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	return	ESLErrorMsg
		( "定義されていないメンバ関数を呼び出しています。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::CallFunction
	( ECSContext & context,
		int nIndex, EObjArray<ECSObject> & lstArg )
{
	return	ESLErrorMsg( "不正なメンバ関数を呼び出そうとしました。" ) ;
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSFunction::CleanupAllReference( ECSContext & context )
{
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::CommitAllReference( ECSContext & context )
{
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Save( ESLFileObject & file, ECSContext & context )
{
	DWORD		dwType = m_ftType ;
	DWORD_PTR	dwAddr = (DWORD_PTR) m_varFunc.pfnNaked ;
	//
	file.Write( &dwType, sizeof(dwType) ) ;
	file.Write( &dwAddr, sizeof(dwAddr) ) ;
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Load( ESLFileObject & file, ECSContext & context )
{
	DWORD		dwType ;
	DWORD_PTR	dwAddr ;
	//
	file.Read( &dwType, sizeof(dwType) ) ;
	file.Read( &dwAddr, sizeof(dwAddr) ) ;
	//
	m_ftType = (FunctionType) dwType ;
	m_varFunc.pfnNaked = (FARPROC) dwAddr ;
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strBuf ;
	switch ( m_ftType )
	{
	case	funcIndexCall:
		strBuf = "indexed call " ;
		strBuf += EString( m_varFunc.nIndex ) ;
		break ;
	case	funcScriptCall:
		strBuf = "script address " ;
		strBuf += EString( m_varFunc.addrScript, 8 ) ;
		break ;
	case	funcNativeCall:
		strBuf = "native call " ;
		strBuf += EString( (DWORD_PTR) m_varFunc.pfnNaked, 8 ) ;
		break ;
	case	funcNakedCall:
		strBuf = "naked call " ;
		strBuf += EString( (DWORD_PTR) m_varFunc.pfnNaked, 8 ) ;
		break ;
	}
	buf.Write( strBuf.CharPtr(), strBuf.GetLength() ) ;
	return	eslErrSuccess ;
}
*/
