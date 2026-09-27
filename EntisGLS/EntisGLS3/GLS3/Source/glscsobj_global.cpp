
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 大域変数名前空間
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSGlobal, ECSArray )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSGlobal::ECSGlobal( void )
{
	m_pElementType = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSGlobal::~ECSGlobal( void )
{
	delete	m_pElementType ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSGlobal::GetTypeName( void ) const
{
	return	L"Global" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSGlobal::Duplicate( void )
{
	ECSGlobal *	pGlobal = new ECSGlobal ;
	pGlobal->CopyFrom( *this ) ;
	//
	unsigned int	i, nCount ;
	nCount = m_staObjName.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszName = m_staObjName.GetAt( i ) ;
		if ( pwszName != NULL )
		{
			pGlobal->m_staObjName.Add( pwszName ) ;
		}
		else
		{
			pGlobal->m_staObjName.Add( NULL ) ;
		}
	}
	//
	return	pGlobal ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::Move( ECSContext & context, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない代入操作です。" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない演算操作です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない比較操作です。" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::GetVariableIndex( int & nIndex, int iMember )
{
	nIndex = iMember ;
	return	eslErrSuccess ;
}

ESLError ECSGlobal::GetVariableIndex
				( int & nIndex, const wchar_t * pwszMember )
{
	nIndex = m_staObjName.FindIndex( pwszMember ) ;
	if ( nIndex >= 0 )
	{
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "指標が範囲外です。" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	ECSWideString	wstrName = pwszName ;
	if ( wstrName == L"GetLength" )
	{
		nIndex = 0 ;
		return	eslErrSuccess ;
	}
	else if ( wstrName == L"GetTagName" )
	{
		nIndex = 1 ;
		return	eslErrSuccess ;
	}
	else if ( wstrName == L"IsEmpty" )
	{
		nIndex = 2 ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "定義されていない関数の呼び出しです。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( nIndex == 0 )
	{
		return	Call_GetLength( context, lstArg ) ;
	}
	else if ( nIndex == 1 )
	{
		return	Call_GetTagName( context, lstArg ) ;
	}
	else if ( nIndex == 2 )
	{
		return	Call_IsEmpty( context, lstArg ) ;
	}
	return	ESLErrorMsg( "定義されていない関数の呼び出しです。" ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = m_staObjName.SaveArray( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	ECSArray::Save( file, context ) ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = m_staObjName.LoadArray( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	return	ECSArray::Load( file, context ) ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = "要素数 = " ;
	strDump += EString( (int) m_varArray.GetSize() ) ;
	strDump += "\r\n" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		//
		// 要素名
		//
		strDump = EString("\t") * nIndent + "\t[" + EString(i) + "]" ;
		const wchar_t *	pwszName = m_staObjName.GetAt( i ) ;
		if ( pwszName != NULL )
		{
			strDump += " \"" ;
			strDump += EString(pwszName) ;
			strDump += '\"' ;
		}
		strDump += " = " ;
		//
		// 内容
		//
		ECSObject *	pObj = m_varArray.GetAt( i ) ;
		if ( pObj != NULL )
		{
			strDump += EString( pObj->GetTypeName() ) ;
			strDump += " : " ;
			buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			strDump = "" ;
			pObj->DumpObject( buf, nIndent + 1, context ) ;
		}
		else
		{
			strDump += "<nothing>" ;
		}
		strDump += "\r\n" ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	}
	return	eslErrSuccess ;
}

// 変数追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::AddVariable( const wchar_t * pwszName, ECSObject * pObj )
{
	ESLAssert( m_varArray.GetSize() == m_staObjName.GetSize() ) ;
	m_varArray.Add( pObj ) ;
	m_staObjName.Add( pwszName ) ;
	return	eslErrSuccess ;
}

// 変数全て削除
//////////////////////////////////////////////////////////////////////////////
void ECSGlobal::RemoveAllVariable( void )
{
	for ( int i = m_varArray.GetSize() - 1; i >= 0; i -- )
	{
		m_varArray.SetAt( i, NULL ) ;
	}
	m_staObjName.RemoveAll( ) ;
	m_varArray.RemoveAll( ) ;
}

// 変数名取得 : String GetTagName( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::Call_GetTagName
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nIndex ;
	err = context.GetArgumentAsInt( nIndex, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	const wchar_t *	pwszName = m_staObjName.GetAt( nIndex ) ;
	if ( pwszName == NULL )
	{
		return	context.PushObject( new ECSString ) ;
	}
	return	context.PushObject( new ECSString( pwszName ) ) ;
}

// 要素有無判定 : Boolean IsEmpty( String sTag ) const
//////////////////////////////////////////////////////////////////////////////
ESLError ECSGlobal::Call_IsEmpty
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrTag ;
	err = context.GetArgumentAsStr( wstrTag, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( (m_staObjName.FindIndex( wstrTag ) < 0) ? -1 : 0 ) ) ;
}
