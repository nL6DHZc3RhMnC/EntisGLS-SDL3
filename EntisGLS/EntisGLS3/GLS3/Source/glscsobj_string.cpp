
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <rosetta/rosetta.h>
#include <glscs_rosetta.h>


//////////////////////////////////////////////////////////////////////////////
// 文字列オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSString, ECSObject )

// 書き込み用内部バッファ取得
//////////////////////////////////////////////////////////////////////////////
wchar_t * ECSString::LockBuffer( int nLength )
{
	m_nLockBufSize = nLength ;
	return	m_varStr.GetBuffer( nLength ) ;
}

// 書き込み用内部バッファ確定
//////////////////////////////////////////////////////////////////////////////
void ECSString::UnlockBuffer( int nStrLen )
{
	m_nLockBufSize = -1 ;
	m_varStr.ReleaseBuffer( nStrLen ) ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSString::GetTypeName( void ) const
{
	return	L"String" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSString::Duplicate( void )
{
	ECSString *	pStr = new ECSString ;
	pStr->m_varStr = m_varStr ;
	return	pStr ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	if ( pEntity->m_vtType == csvtString )
	{
		m_varStr = ((ECSString*)pEntity)->m_varStr ;
	}
	else if ( pEntity->m_vtType == csvtPointer )
	{
		context.AtomicLoadString( m_varStr, pEntity, 0 ) ;
		m_varStr.MoveIndex( 0 ) ;
	}
	else
	{
		ESLError	err = pEntity->OperateString( m_varStr ) ;
		if ( err )
		{
			return	err ;
		}
		m_varStr.MoveIndex( 0 ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	switch ( csuopType )
	{
	case	csuotNegate:
		m_varStr.MakeReverse( ) ;
	case	csuotPlus:
		break ;
	case	csuotLogicalNot:
		m_pResult = context.new_CSInteger
			( - (long int) (m_varStr.GetLength() == 0),
									ECSInteger::m_maskBoolean ) ;
		break ;
	default:
		return	ESLErrorMsg( "定義されていない String 型の単項演算子です。" ) ;
	}
	return	eslErrSuccess ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	ESLError	err = ECSString::Operate( context, csopType, *pEntity ) ;
	if ( err )
	{
		return	err ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

ESLError ECSString::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject & obj )
{
	ECSObject *	pEntity = &obj ;
	if ( pEntity->m_vtType == csvtString )
	{
		switch ( csopType )
		{
		case	csotAdd:
			m_varStr += ((ECSString*)pEntity)->m_varStr ;
			break ;
		case	csotLogicalAnd:
			m_pResult = context.new_CSInteger
				( - (long int) (m_varStr.GetLength()
						&& ((ECSString*)pEntity)->m_varStr.GetLength()),
											ECSInteger::m_maskBoolean ) ;
			break ;
		case	csoutLogicalOr:
			m_pResult = context.new_CSInteger
				( - (long int) (m_varStr.GetLength()
						|| ((ECSString*)pEntity)->m_varStr.GetLength()),
											ECSInteger::m_maskBoolean ) ;
			break ;
		default:
			return	ESLErrorMsg( "定義されていない String 型の演算です。" ) ;
		}
	}
	else if ( pEntity->m_vtType == csvtInteger )
	{
		switch ( csopType )
		{
		case	csotMul:
			m_varStr = m_varStr * ((ECSInteger*)pEntity)->GetInt() ;
			break ;
		case	csotLogicalAnd:
			m_pResult = context.new_CSInteger
				( - (long int) (m_varStr.GetLength()
						&& ((ECSInteger*)pEntity)->GetValue()),
									ECSInteger::m_maskBoolean ) ;
			break ;
		case	csoutLogicalOr:
			m_pResult = context.new_CSInteger
				( - (long int) (m_varStr.GetLength()
					|| ((ECSInteger*)pEntity)->GetValue()),
									ECSInteger::m_maskBoolean ) ;
			break ;
		default:
			return	ESLErrorMsg( "定義されていない String 型の演算です。" ) ;
		}
	}
	else
	{
		ESLError	err ;
		switch ( csopType )
		{
		case	csotAdd:
			{
				EWideString	wstrText ;
				err = pEntity->OperateString( wstrText ) ;
				if ( err )
				{
					return	err ;
				}
				m_varStr += wstrText ;
			}
			break ;
		case	csotMul:
			{
				INT64	nValue ;
				err = pEntity->OperateInteger( nValue ) ;
				if ( err )
				{
					return	err ;
				}
				m_varStr = m_varStr * (int) nValue ;
			}
			break ;
		default:
			return	ESLErrorMsg( "定義されていない String 型の演算です。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( &obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "比較元オブジェクトが存在しません。" ) ;
	}
	if ( pEntity->m_vtType != csvtString )
	{
		return	ESLErrorMsg( "定義されていない String 型の比較です。" ) ;
	}
	ECSString *	pStr = (ECSString*) pEntity ;
	if ( cscpType & csctEqual )
	{
		switch ( cscpType )
		{
		case	csctEqual:
			nResult = - (int) (m_varStr == pStr->m_varStr) ;
			break ;
		case	csctLessEqual:
			nResult = - (int) (m_varStr <= pStr->m_varStr) ;
			break ;
		case	csctGreaterEqual:
			nResult = - (int) (m_varStr >= pStr->m_varStr) ;
			break ;
		}
	}
	else
	{
		switch ( cscpType )
		{
		case	csctNotEqual:
			nResult = - (int) (m_varStr != pStr->m_varStr) ;
			break ;
		case	csctLessThan:
			nResult = - (int) (m_varStr < pStr->m_varStr) ;
			break ;
		case	csctGreaterThan:
			nResult = - (int) (m_varStr > pStr->m_varStr) ;
			break ;
		}
	}
	return	eslErrSuccess ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "定義されていないメンバ関数を呼び出しています。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg( "不正なメンバ関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::OperateBoolean( int & nBoolean )
{
	nBoolean = - (int) (m_varStr.GetLength() != 0) ;
	return	eslErrSuccess ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::OperateSizeOf( INT64 & nSize )
{
	nSize = m_varStr.GetLength() ;
	return	eslErrSuccess ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::OperateInteger( INT64 & nValue )
{
	int	nIndex = m_varStr.GetIndex() ;
	nValue = m_varStr.GetLargeInteger( ) ;
	m_varStr.MoveIndex( nIndex ) ;
	return	eslErrSuccess ;
}

// 実数取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::OperateReal( REAL64 & nValue )
{
	int	nIndex = m_varStr.GetIndex() ;
	nValue = m_varStr.GetRealNumber( ) ;
	m_varStr.MoveIndex( nIndex ) ;
	return	eslErrSuccess ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::OperateString( EWideString & wstrValue )
{
	wstrValue = m_varStr ;
	return	eslErrSuccess ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSString::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	if ( (unsigned int) (iOffset + nSize)
					<= m_varStr.GetLength() * sizeof(wchar_t) )
	{
		return	((BYTE*)m_varStr.CharPtr()) + iOffset ;
	}
	return	NULL ;
}

ECSSakura2Processor::LinearAddressCache *
	ECSString::GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg )
{
	seg.baseOffset = 0 ;
	seg.limitSegment = m_varStr.GetBufferLength() * 2 ;
	seg.pbytBuffer = (BYTE*) m_varStr.CharPtr() ;
	return	&seg ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Save( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength = m_varStr.GetLength( ) ;
	DWORD	dwIndex = m_varStr.GetIndex() ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	if ( file.Write( m_varStr.CharPtr(),
			dwLength * sizeof(wchar_t) ) < dwLength * sizeof(wchar_t) )
	{
		return	ESLErrorMsg( "文字列の書き出しに失敗しました。" ) ;
	}
	file.Write( &dwIndex, sizeof(dwIndex) ) ;
	file.Write( &m_nLockBufSize, sizeof(m_nLockBufSize) ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Load( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	ESLErrorMsg( "文字列の読み込みに失敗しました。" ) ;
	}
	wchar_t *	pwszBuf = m_varStr.GetBuffer( dwLength ) ;
	if ( pwszBuf == NULL )
	{
		return	ESLErrorMsg( "文字列の読み込みに失敗しました。" ) ;
	}
	if ( file.Read( pwszBuf,
			dwLength * sizeof(wchar_t) ) < dwLength * sizeof(wchar_t) )
	{
		m_varStr.ReleaseBuffer( 0 ) ;
		return	ESLErrorMsg( "文字列の読み込みに失敗しました。" ) ;
	}
	m_varStr.ReleaseBuffer( dwLength ) ;
	//
	DWORD	dwIndex ;
	if ( file.Read( &dwIndex, sizeof(dwIndex) ) < sizeof(dwIndex) )
	{
		return	ESLErrorMsg( "文字列の読み込みに失敗しました。" ) ;
	}
	m_varStr.MoveIndex( dwIndex ) ;
	//
	file.Read( &m_nLockBufSize, sizeof(m_nLockBufSize) ) ;
	if ( m_nLockBufSize > 0 )
	{
		m_varStr.GetBuffer( m_nLockBufSize ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = "length = " ;
	strDump += EString( (int) m_varStr.GetLength() ) ;
	strDump += ", \"" ;
	strDump += EString( m_varStr.Left( 32 ) ) ;
	if ( m_varStr.GetLength() > 32 )
		strDump += "…" ;
	strDump += '\"' ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// スクリプトのデストラクタ
//////////////////////////////////////////////////////////////////////////////
void ECSString::OnDestruction( ECSContext & context )
{
	m_varStr.FreeString() ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSString::m_staFuncName = NULL ;
const wchar_t *		ECSString::m_pwszFuncName[43] =
{
	L"GetLength", L"Char", L"SetChar",
	L"Left", L"Right", L"Middle",
	L"MakeUpper", L"MakeLower",
	L"TrimLeft", L"TrimRight",
	L"Compare", L"CompareNoCase", L"CompareLeft", L"CompareLeftNoCase",
	L"OffsetFilePath", L"ParseFileDrive",
	L"ParseFileDirectory", L"ParseFileName",
	L"ParseFileTitle", L"ParseFileExtension",
	L"GetCEncoded", L"GetCDecoded", 
	L"GetXMLEncoded", L"GetXMLDecoded", 
	L"Find", L"Replace", L"Separate",
	L"Calculate", L"Execute", L"IsMatchUsage",
	L"GetIndex", L"SeekIndex", L"SeekNext",
	L"NextChar", L"NextInteger", L"NextRealNumber",
	L"NextNeedChar", L"NextString", L"NextToken",
	L"NextEnclosedString", L"NextMatchUsage",
	L"Call",
	NULL
} ;
const ECSString::PFUNC_CALL	ECSString::m_pfnCallFunc[42] =
{
	&ECSString::Call_GetLength, &ECSString::Call_Char,
	&ECSString::Call_SetChar, &ECSString::Call_Left,
	&ECSString::Call_Right, &ECSString::Call_Middle,
	&ECSString::Call_MakeUpper, &ECSString::Call_MakeLower,
	&ECSString::Call_TrimLeft, &ECSString::Call_TrimRight,
	&ECSString::Call_Compare, &ECSString::Call_CompareNoCase,
	&ECSString::Call_CompareLeft, &ECSString::Call_CompareLeftNoCase,
	&ECSString::Call_OffsetFilePath, &ECSString::Call_ParseFileDrive,
	&ECSString::Call_ParseFileDirectory, &ECSString::Call_ParseFileName,
	&ECSString::Call_ParseFileTitle, &ECSString::Call_ParseFileExtension,
	&ECSString::Call_GetCEncoded, &ECSString::Call_GetCDecoded,
	&ECSString::Call_GetXMLEncoded, &ECSString::Call_GetXMLDecoded,
	&ECSString::Call_Find, &ECSString::Call_Replace,
	&ECSString::Call_Separate, &ECSString::Call_Calculate,
	&ECSString::Call_Execute, &ECSString::Call_IsMatchUsage,
	&ECSString::Call_GetIndex,
	&ECSString::Call_SeekIndex,
	&ECSString::Call_SeekNext,
	&ECSString::Call_NextChar,
	&ECSString::Call_NextInteger,
	&ECSString::Call_NextRealNumber,
	&ECSString::Call_NextNeedChar,
	&ECSString::Call_NextString,
	&ECSString::Call_NextToken,
	&ECSString::Call_NextEnclosedString,
	&ECSString::Call_NextMatchUsage,
	&ECSString::Call_Call
} ;

// メンバ関数 : Integer GetLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_GetLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( new ECSInteger( m_varStr.GetLength() ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer Char( Integer i := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Char
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	index ;
	err = context.GetArgumentAsInt( index, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( new ECSInteger( (int) m_varStr.GetAt( index ) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : SetChar( Integer i, {Integer|String} c )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_SetChar
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int	index, code ;
	err = context.GetArgumentAsInt( index, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt( 2 ) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "引数が指定されていません。" ) ;
	}
	if ( pObj->m_vtType == csvtInteger )
	{
		code = ((ECSInteger*)pObj)->GetInt() ;
	}
	else if ( pObj->m_vtType == csvtString )
	{
		code = ((ECSString*)pObj)->m_varStr.GetAt( 0 ) ;
	}
	else
	{
		return	ESLErrorMsg
			( "2 番目の引数が整数型でも文字列型でもありません。" ) ;
	}
	if ( (unsigned int) index < m_varStr.GetLength() )
	{
		m_varStr.SetAt( index, (wchar_t) code ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : String Left( Integer count )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Left
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nCount ;
	err = context.GetArgumentAsInt( nCount, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = new ECSString ;
	pStr->m_varStr = m_varStr.Left( nCount ) ;
	context.PushObject( pStr ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : String Right( Integer count )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Right
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nCount ;
	err = context.GetArgumentAsInt( nCount, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = new ECSString ;
	pStr->m_varStr = m_varStr.Right( nCount ) ;
	context.PushObject( pStr ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : String Middle( Integer first, Integer count := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Middle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int	iFirst, nCount ;
	err = context.GetArgumentAsInt( iFirst, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nCount, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = new ECSString ;
	pStr->m_varStr = m_varStr.Middle( iFirst, nCount ) ;
	context.PushObject( pStr ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : MakeUpper()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_MakeUpper
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	m_varStr.MakeUpper( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : MakeLower()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_MakeLower
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	m_varStr.MakeLower( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : TrimLeft()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_TrimLeft
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	m_varStr.TrimLeft( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : TrimRight()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_TrimRight
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	m_varStr.TrimRight( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer Compare( String sText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Compare
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStrText =
		ESLTypeCast<ECSString>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pStrText == NULL )
	{
		return	ESLErrorMsg( "引数に String オブジェクトが指定されていません" ) ;
	}
	return	context.PushObject
		( context.new_CSInteger( m_varStr.Compare( pStrText->m_varStr ) ) ) ;
}

// メンバ関数 : Integer CompareNoCase( String sText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_CompareNoCase
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStrText =
		ESLTypeCast<ECSString>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pStrText == NULL )
	{
		return	ESLErrorMsg( "引数に String オブジェクトが指定されていません" ) ;
	}
	return	context.PushObject
		( context.new_CSInteger( m_varStr.CompareNoCase( pStrText->m_varStr ) ) ) ;
}

// メンバ関数 : Integer CompareLeft( String sText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_CompareLeft
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStrText =
		ESLTypeCast<ECSString>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pStrText == NULL )
	{
		return	ESLErrorMsg( "引数に String オブジェクトが指定されていません" ) ;
	}
	return	context.PushObject
		( context.new_CSInteger( m_varStr.CompareLeft( pStrText->m_varStr ) ) ) ;
}

// メンバ関数 : Integer CompareLeftNoCase( String sText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_CompareLeftNoCase
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStrText =
		ESLTypeCast<ECSString>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pStrText == NULL )
	{
		return	ESLErrorMsg( "引数に String オブジェクトが指定されていません" ) ;
	}
	return	context.PushObject
		( context.new_CSInteger( m_varStr.CompareLeftNoCase( pStrText->m_varStr ) ) ) ;
}

// メンバ関数 : String OffsetFilePath( String sOffsetPath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_OffsetFilePath
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	EWideString	wstrOffsetPath ;
	err = context.GetArgumentAsStr( wstrOffsetPath, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSString( m_varStr.OffsetFilePath( wstrOffsetPath ) ) ) ;
}

// メンバ関数 : String ParseFileDrive()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_ParseFileDrive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSString( m_varStr.GetFileDrivePart() ) ) ;
}

// メンバ関数 : String ParseFileDirectory()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_ParseFileDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSString( m_varStr.GetFileDirectoryPart() ) ) ;
}

// メンバ関数 : String ParseFileName()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_ParseFileName
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSString( m_varStr.GetFileNamePart() ) ) ;
}

// メンバ関数 : String ParseFileTitle()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_ParseFileTitle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSString( m_varStr.GetFileTitlePart() ) ) ;
}

// メンバ関数 : String ParseFileExtension()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_ParseFileExtension
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSString( m_varStr.GetFileExtensionPart() ) ) ;
}

// メンバ関数 : String GetCEncoded() const
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_GetCEncoded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = context.new_CSString() ;
	pStr->m_varStr = m_varStr ;
	EDescription::EncodeTextCEscSequence( pStr->m_varStr ) ;
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : String GetCDecoded() const
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_GetCDecoded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = context.new_CSString() ;
	pStr->m_varStr = m_varStr ;
	EDescription::DecodeTextCEscSequence( pStr->m_varStr ) ;
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : String GetXMLEncoded() const
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_GetXMLEncoded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = context.new_CSString() ;
	EDescription::EncodeTextContents( pStr->m_varStr, m_varStr ) ;
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : String GetXMLDecoded() const
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_GetXMLDecoded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = context.new_CSString() ;
	pStr->m_varStr = m_varStr ;
	EDescription::DecodeTextContents( pStr->m_varStr ) ;
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : Integer Find( String s, Integer i := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Find
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int				iFind ;
	ECSWideString	wstrFind ;
	err = context.GetArgumentAsStr( wstrFind, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( iFind, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( new ECSInteger( m_varStr.Find( wstrFind, iFind ) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : String Replace
//		( String strOld, String strNew, Integer nFlag := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Replace
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrOld, wstrNew ;
	int				nFlag ;
	err = context.GetArgumentAsStr( wstrOld, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrNew, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFlag, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	if ( nFlag )
	{
		ECSSourceStream	swsText = m_varStr ;
		ECSSourceStream	swsUsage = wstrOld ;
		EWideString		wstrReplaced ;
		if ( !swsText.ReplaceUsage( wstrReplaced, swsUsage, wstrNew ) )
		{
			return	context.PushObject( new ECSString( wstrReplaced ) ) ;
		}
	}
	return	context.PushObject
		( new ECSString( m_varStr.Replace( wstrOld, wstrNew ) ) ) ;
}

// メンバ関数 : Separate( Reference rArray, String strSep := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Separate
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pArray =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	if ( pArray == NULL )
	{
		return	ESLErrorMsg
			( "引数に Array オブジェクトが指定されていません" ) ;
	}
	ECSWideString	wstrSep ;
	err = context.GetArgumentAsStr( wstrSep, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	if ( wstrSep.IsEmpty() )
	{
		ECSSourceStream	cssSrc ;
		cssSrc = m_varStr ;
		while ( !cssSrc.DisregardSpace() )
		{
			pArray->m_varArray.Add
				( new ECSString( cssSrc.GetAToken() ) ) ;
		}
	}
	else
	{
		int	iLast = 0, iFind ;
		for ( ; ; )
		{
			iFind = m_varStr.Find( wstrSep, iLast ) ;
			if ( iFind < 0 )
			{
				break ;
			}
			pArray->m_varArray.Add
				( new ECSString( m_varStr.Middle( iLast, iFind - iLast ) ) ) ;
			iLast = iFind + wstrSep.GetLength() ;
		}
		pArray->m_varArray.Add
			( new ECSString( m_varStr.Middle( iLast ) ) ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : object Calculate( [Reference rErrMsg] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Calculate
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pErrMsg =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 1, L"String" ) ) ;
	//
	ECSCompiler	compiler ;
	ECSObject *	pValue ;
	ECSSourceStream	cssExpr ;
	cssExpr = m_varStr ;
	compiler.AttachExternalMacroVariable( &(context.m_pcsxi->m_csgGlobal) ) ;
	compiler.InitConstExprContext( context.m_pcsxi ) ;
	err = compiler.CalculateExpression( pValue, cssExpr, 0, NULL, false ) ;
	if ( err )
	{
		if ( pErrMsg != NULL )
		{
			pErrMsg->m_varStr = GetESLErrorMsg(err) ;
		}
		pValue = new ECSReference ;
	}
	else
	{
		if ( pErrMsg != NULL )
		{
			pErrMsg->m_varStr = L"" ;
		}
	}
	compiler.ReleaseConstExprContext() ;
	return	context.PushObject( pValue ) ;
}

// メンバ関数 : object Execute( [String& strErr] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Execute
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pErrMsg =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 1, L"String" ) ) ;
	//
	Rosetta::RSVirtualMachine	vm ;
	vm.Initialize() ;
	//
	Rosetta::RSClass *	pClass =
		new Rosetta::RSClass( vm.GetClassClass(), L"CotophaObject" ) ;
	vm.RegisterNewClass( pClass ) ;
	vm.ImplementNewClasses() ;
	//
	Rosetta::RSContext	rsContext( &vm ) ;
	//
	Rosetta::RSCotophaObject *	prsObjGlobal =
		new Rosetta::RSCotophaObject
			( pClass, &context,
				context.new_CSReference( &(context.m_pcsxi->m_csgGlobal) ) ) ;
	Rosetta::RSCotophaObject *	prsObjShare =
		new Rosetta::RSCotophaObject
			( pClass, &context,
				context.new_CSReference( &(context.m_pcsxi->m_csgData) ) ) ;
	//
	vm.AddRef() ;
	rsContext.PushNamespace
		( NULL, &vm, Rosetta::RSObject::modifierPublic, true ) ;
	rsContext.PushNamespace
		( NULL, prsObjShare, Rosetta::RSObject::modifierPublic, false ) ;
	rsContext.PushNamespace
		( NULL, prsObjGlobal, Rosetta::RSObject::modifierPublic, false ) ;
	//
	Rosetta::RSObject *
		prsRetObj = rsContext.EvaluateExpression( m_varStr ) ;
	ECSObject *	pRetObj = NULL ;
	if ( prsRetObj != NULL )
	{
		Rosetta::RSCotophaObject *	pcoObj =
				ESLTypeCast<Rosetta::RSCotophaObject>
							( prsRetObj->GetEntityObject() ) ;
		if ( pcoObj != NULL )
		{
			pRetObj = pcoObj->DetachObject() ;
		}
		else
		{
			switch ( prsRetObj->GetBasicType() )
			{
			case	Rosetta::RSObject::typeNumber:
				{
					int64_t	num ;
					if ( prsRetObj->AsInteger( num ) )
					{
						pRetObj = context.new_CSInteger( num ) ;
					}
				}
				break;
			case	Rosetta::RSObject::typeInteger:
				{
					double	num ;
					if ( prsRetObj->AsRealNumber( num ) )
					{
						pRetObj = context.new_CSReal( num ) ;
					}
				}
				break ;
			case	Rosetta::RSObject::typeBoolean:
				pRetObj = context.new_CSInteger
								( prsRetObj->AsBoolean() ? -1 : 0 ) ;
				break ;
			case	Rosetta::RSObject::typeString:
				{
					SSystem::SString	str ;
					if ( prsRetObj->AsString( str ) )
					{
						pRetObj = context.new_CSString( str ) ;
					}
				}
				break ;
			}
		}
		delete	prsRetObj ;
	}
	if ( pRetObj == NULL )
	{
		pRetObj = new ECSReference ;
	}
	if ( pErrMsg != NULL )
	{
		pErrMsg->m_varStr = L"" ;
	}
	Rosetta::RSObject *	pException = rsContext.PopException() ;
	if ( pException != NULL )
	{
		SSystem::SString	strError ;
		if ( pException->AsString( strError ) )
		{
			if ( pErrMsg != NULL )
			{
				pErrMsg->m_varStr = strError ;
			}
		}
	}
	//
	rsContext.PopNamespace() ;
	rsContext.PopNamespace() ;
	rsContext.PopNamespace() ;
	//
	vm.Release() ;
	//
	return	context.PushObject( pRetObj ) ;
}

// メンバ関数 : String IsMatchUsage
//				( String strUsage[, Reference aParam[, Reference nIndex]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_IsMatchUsage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrUsage ;
	err = context.GetArgumentAsStr( wstrUsage, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pParam =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 2, L"Array" ) ) ;
	ECSInteger *	pIndex =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 3, L"Integer" ) ) ;
	//
	EObjArray<EWideString>	lstParam ;
	EString	strErrMsg ;
	ECSSourceStream	cssData, cssUsage ;
	cssData = m_varStr ;
	cssUsage = wstrUsage ;
	err = cssData.IsMatchUsage( cssUsage, strErrMsg, &lstParam ) ;
	if ( !err )
	{
		if ( pParam != NULL )
		{
			pParam->m_varArray.RemoveAll( ) ;
			for ( int i = 0; i < (int) lstParam.GetSize(); i ++ )
			{
				pParam->m_varArray.Add( new ECSString( lstParam[i] ) ) ;
			}
		}
		if ( pIndex != NULL )
		{
			pIndex->SetValue( cssData.GetIndex( ) ) ;
		}
	}
	//
	return	context.PushObject
		( new ECSString( ECSWideString( strErrMsg ) ) ) ;
}

// メンバ関数 : Integer GetIndex()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_GetIndex
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( context.new_CSInteger( m_varStr.GetIndex() ) ) ;
}

// メンバ関数 : Integer SeekIndex( Integer nIndex )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_SeekIndex
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nIndex ;
	err = context.GetArgumentAsInt( nIndex, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	m_varStr.MoveIndex( nIndex ) ;
	//
	return	context.PushObject
		( context.new_CSInteger( m_varStr.GetIndex() ) ) ;
}

// メンバ関数 : Integer SeekNext()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_SeekNext
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( context.new_CSInteger( m_varStr.DisregardSpace() ? -1 : 0 ) ) ;
}

// メンバ関数 : String NextChar()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextChar
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = context.new_CSString() ;
	wchar_t	wchNext = m_varStr.GetCharacter() ;
	if ( wchNext )
	{
		pStr->m_varStr.GetBuffer(1)[0] = wchNext ;
		pStr->m_varStr.ReleaseBuffer(1) ;
	}
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : Integer NextInteger( [Integer fError] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextInteger
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSInteger *	pError =
		ESLTypeCast<ECSInteger>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	//
	int		fError = -1 ;
	int		nRadix = m_varStr.GetNumberRadix( ) ;
	INT64	nNum = 0 ;
	if ( nRadix >= 0 )
	{
		nNum = m_varStr.GetLargeInteger( nRadix ) ;
		fError = 0 ;
	}
	if ( pError != NULL )
	{
		pError->SetValue( fError ) ;
	}
	return	context.PushObject( context.new_CSInteger( nNum ) ) ;
}

// メンバ関数 : Real NextRealNumber( [Integer fError] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextRealNumber
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSInteger *	pError =
		ESLTypeCast<ECSInteger>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	//
	int		fError = -1 ;
	int		nRadix = m_varStr.GetNumberRadix( ) ;
	double	rNum = 0 ;
	if ( nRadix >= 0 )
	{
		rNum = m_varStr.GetRealNumber( nRadix ) ;
		fError = 0 ;
	}
	if ( pError != NULL )
	{
		pError->SetValue( fError ) ;
	}
	return	context.PushObject( context.new_CSReal( rNum ) ) ;
}

// メンバ関数 : String NextNeedChar( String sNext )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextNeedChar
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrNext ;
	err = context.GetArgumentAsStr( wstrNext, lstArg, 1, L"" ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = context.new_CSString() ;
	wchar_t	wchNext = m_varStr.HasToComeChar( wstrNext ) ;
	if ( wchNext )
	{
		pStr->m_varStr.GetBuffer(1)[0] = wchNext ;
		pStr->m_varStr.ReleaseBuffer(1) ;
	}
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : String NextString()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextString
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStr = context.new_CSString() ;
	pStr->m_varStr = m_varStr.GetString() ;
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : String NextToken( [Integer fTokenType] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextToken
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSInteger *	pError =
		ESLTypeCast<ECSInteger>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	//
	int	nTokenType ;
	ECSString *	pStr = context.new_CSString() ;
	pStr->m_varStr = m_varStr.GetAToken( &nTokenType ) ;
	//
	if ( pError != NULL )
	{
		pError->SetValue( nTokenType ) ;
	}
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : String NextEnclosedString( String sClose )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextEnclosedString
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrClose ;
	err = context.GetArgumentAsStr( wstrClose, lstArg, 1, L"" ) ;
	if ( err )
		return	err ;
	//
	wchar_t	wchClose = wstrClose.GetAt( 0 ) ;
	//
	ECSString *	pStr = context.new_CSString() ;
	if ( wchClose != L'\0' )
	{
		pStr->m_varStr = m_varStr.GetEnclosedString( wchClose ) ;
	}
	else
	{
		pStr->m_varStr = m_varStr.GetString() ;
	}
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : String NextMatchUsage
//				( String strUsage[, Reference aParam] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_NextMatchUsage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSSourceStream	cssUsage ;
	err = context.GetArgumentAsStr( cssUsage, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pParam =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 2, L"Array" ) ) ;
	//
	EObjArray<EWideString>	lstParam ;
	EString	strErrMsg ;
	err = m_varStr.IsMatchUsage( cssUsage, strErrMsg, &lstParam ) ;
	if ( !err )
	{
		if ( pParam != NULL )
		{
			pParam->m_varArray.RemoveAll( ) ;
			for ( int i = 0; i < (int) lstParam.GetSize(); i ++ )
			{
				pParam->m_varArray.Add( new ECSString( lstParam[i] ) ) ;
			}
		}
	}
	return	context.PushObject
		( new ECSString( ECSWideString( strErrMsg ) ) ) ;
}

// メンバ関数 : Call( ... )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSString::Call_Call
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	if ( m_varStr.IsEmpty() )
	{
		return	ESLErrorMsg( "不正な関数の呼び出しです。" ) ;
	}
	ECSObject *	pObjThis = lstArg.GetAt(0) ;
	lstArg.DetachAt( 0 ) ;
	lstArg.Add( pObjThis ) ;
	return	context.CallGlobalFunction( m_varStr ) ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSString::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_STRING_INTERFACE" ) )
	{
		m_pis.pBackLink = this ;
		m_pis.pfnGetString = PIC_GetString ;
		m_pis.pfnGetBuffer = PIC_GetBuffer ;
		m_pis.pfnReleaseBuffer = PIC_ReleaseBuffer ;
		return	(ECS_STRING_INTERFACE*) &m_pis ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

const wchar_t * __stdcall ECSString::PIC_GetString
	( ECS_STRING_INTERFACE * instance )
{
	PLUGIN_STRING *	ppis = (PLUGIN_STRING*) instance ;
	ESLAssert( &(ppis->pBackLink->m_pis) == ppis ) ;
	return	ppis->pBackLink->m_varStr ;
}

wchar_t * __stdcall ECSString::PIC_GetBuffer
	( ECS_STRING_INTERFACE * instance, unsigned int nLength )
{
	PLUGIN_STRING *	ppis = (PLUGIN_STRING*) instance ;
	ESLAssert( &(ppis->pBackLink->m_pis) == ppis ) ;
	return	ppis->pBackLink->m_varStr.GetBuffer( nLength ) ;
}

void __stdcall ECSString::PIC_ReleaseBuffer
	( ECS_STRING_INTERFACE * instance, int nLength )
{
	PLUGIN_STRING *	ppis = (PLUGIN_STRING*) instance ;
	ESLAssert( &(ppis->pBackLink->m_pis) == ppis ) ;
	ppis->pBackLink->m_varStr.ReleaseBuffer( nLength ) ;
}


//////////////////////////////////////////////////////////////////////////////
// naked call 関数
//////////////////////////////////////////////////////////////////////////////

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_GetLength
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis == NULL )
	{
		context->m_regset[ECSSakura2Processor::regAcc].i = 0 ;
		return	NULL ;
	}
	context->m_regset[ECSSakura2Processor::regAcc].i = pThis->m_varStr.GetLength() ;
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_GetBuffer
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	context->m_regset[ECSSakura2Processor::regAcc] = pArg[0] ;
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_SetString
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis != NULL )
	{
		wchar_t *	pwszStr =
			(wchar_t*) context->AtomicTranslateAddress( pArg[1].i ) ;
		if ( pArg[2].i < 0 )
		{
			pThis->m_varStr = pwszStr ;
		}
		else
		{
			pThis->m_varStr = EWideString( pwszStr, pArg[2].l32 ) ;
		}
	}
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_AppendString
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis != NULL )
	{
		wchar_t *	pwszStr =
			(wchar_t*) context->AtomicTranslateAddress( pArg[1].i ) ;
		if ( pArg[2].i < 0 )
		{
			pThis->m_varStr += pwszStr ;
		}
		else
		{
			pThis->m_varStr += EWideString( pwszStr, pArg[2].l32 ) ;
		}
	}
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_AppendInteger
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis != NULL )
	{
		EWideString	wstrValue ;
		wstrValue.FromInteger( pArg[1].i, pArg[2].l32 ) ;
		pThis->m_varStr += wstrValue ;
	}
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_AppendHexInteger
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis != NULL )
	{
		EWideString	wstrValue ;
		wstrValue.HexFromInteger( pArg[1].i, pArg[2].l32 ) ;
		pThis->m_varStr += wstrValue ;
	}
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_AppendReal
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis != NULL )
	{
		EWideString	wstrValue ;
		wstrValue.FromReal( pArg[1].f, pArg[2].l32 ) ;
		pThis->m_varStr += wstrValue ;
	}
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_LockBuffer
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis != NULL )
	{
		pThis->LockBuffer( pArg[1].l32 ) ;
	}
	context->m_regset[ECSSakura2Processor::regAcc] = pArg[0] ;
	return	NULL ;
}

ECS_EXPORT const wchar_t *
	ecs_nakedcall_String_UnlockBuffer
		( ECSSakura2Processor::Context * pcontext,
			const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ECSString *	pThis =
		ESLTypeCast<ECSString>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	if ( pThis != NULL )
	{
		pThis->UnlockBuffer( pArg[1].l32 ) ;
	}
	return	NULL ;
}

