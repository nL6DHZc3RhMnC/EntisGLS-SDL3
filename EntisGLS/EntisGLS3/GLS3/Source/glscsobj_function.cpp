
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 関数ポインタオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSFunction, ECSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSFunction::ECSFunction( void )
{
	m_vtType = csvtFunction ;
	m_fpAddress = 0 ;
	m_pThisCall = NULL ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const ECSFunction & ECSFunction::operator = ( const ECSFunction & func )
{
	m_fpAddress = func.m_fpAddress ;
	m_pThisCall = func.m_pThisCall ;
	m_prototype = func.m_prototype ;
	return	*this ;
}

// スクリプト関数設定（ランタイム）
//////////////////////////////////////////////////////////////////////////////
void ECSFunction::SetFunction
	( ECSContext & context, const wchar_t * pwszFuncName )
{
	m_fpAddress = 0 ;
	//
	ECSExecutionImage::FUNC_ENTRY *
		pFuncEntry = context.m_pcsxi->GetFunctionEntry( pwszFuncName ) ;
	if ( pFuncEntry != NULL )
	{
		m_fpAddress =
			((INT64)ECSExecutionImage::roasCode << 56)
									| pFuncEntry->dwAddress ;
	}
	else
	{
		ESLTrace
			( "not found function %s\n",
				EString(pwszFuncName).CharPtr() ) ;
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
	if ( EWideString::Compare( L"Function", pwszTypeName ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSFunction::Duplicate( void )
{
	ECSFunction *	pFunc = new ECSFunction ;
	*pFunc = *this ;
	return	pFunc ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	if ( pEntity->m_vtType == csvtFunction )
	{
		*this = *((ECSFunction*)pEntity) ;
	}
	else
	{
		INT64		nValue ;
		ESLError	err = pEntity->OperateInteger( nValue ) ;
		if ( err )
		{
			return	err ;
		}
		m_fpAddress = nValue ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない単項演算子です" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない二項演算子です" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない比較演算子です" ) ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::OperateBoolean( int & nBoolean )
{
	nBoolean = - (int) (m_fpAddress != 0) ;
	return	eslErrSuccess ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::OperateInteger( INT64 & nValue )
{
	nValue = m_fpAddress ;
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Save( ESLFileObject & file, ECSContext & context )
{
	file.Write( &m_fpAddress, sizeof(UINT64) ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::Load( ESLFileObject & file, ECSContext & context )
{
	file.Read( &m_fpAddress, sizeof(UINT64) ) ;
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFunction::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	strDump.HexFromInteger( m_fpAddress, 16 ) ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

