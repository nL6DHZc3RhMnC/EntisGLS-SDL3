
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 連想配列オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSHash, ECSObject )

// 要素を複製する
//////////////////////////////////////////////////////////////////////////////
void ECSHash::CopyFrom( ECSHash & obj )
{
	for ( int i = 0; i < (int) obj.m_varArray.GetSize(); i ++ )
	{
		ETaggedElement<ECSWideString,ECSObject> *	pElement ;
		pElement = obj.m_varArray.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		ECSObject *	pObj = pElement->GetObject( ) ;
		if ( pObj != NULL )
		{
			if ( pObj->m_vtType == csvtReference )
			{
				m_varArray.SetAs
					( pElement->Tag(),
						new ECSReference
							( ((ECSReference*)pObj)->m_pRef ) ) ;
			}
			else
			{
				m_varArray.SetAs( pElement->Tag(), pObj->Duplicate() ) ;
			}
		}
	}
}

// 要素を代入する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::MoveFrom( ECSContext & context, ECSHash & obj )
{
	int	i, j, nCount ;
	nCount = obj.m_varArray.GetSize() ;
	//
	m_varArray.SetSize( nCount ) ;
	//
	for ( i = 0, j = 0; i < nCount; i ++ )
	{
		ETaggedElement<ECSWideString,ECSObject> *	pSrcElement ;
		ETaggedElement<ECSWideString,ECSObject> *	pDstElement ;
		pSrcElement = obj.m_varArray.GetAt( i ) ;
		ESLAssert( pSrcElement != NULL ) ;
		if ( pSrcElement == NULL )
		{
			continue ;
		}
		//
		for ( ; ; )
		{
			pDstElement = m_varArray.GetAt( j ) ;
			if ( pDstElement == NULL )
			{
				break ;
			}
			int	nCompareTag =
				pDstElement->Tag().Compare( pSrcElement->Tag() ) ;
			if ( nCompareTag > 0 )
			{
				pDstElement = NULL ;
				break ;
			}
			else if ( nCompareTag == 0 )
			{
				break ;
			}
			else
			{
				m_varArray.RemoveAt( j ) ;
			}
		}
		ECSObject *	pDst = NULL ;
		if ( pDstElement == NULL )
		{
			pDstElement =
				new ETaggedElement<ECSWideString,ECSObject>
									( pSrcElement->Tag(), NULL ) ;
			m_varArray.InsertAt( j, pDstElement ) ;
		}
		else
		{
			pDst = pDstElement->GetObject() ;
		}
		j ++ ;
		ECSObject *	pSrc = pSrcElement->GetObject() ;
		if ( pSrc != NULL )
		{
			ESLError	err =
				ECSArray::MoveElement
						( context, pDst, *pSrc, m_pDefObj ) ;
			if ( err )
			{
				return	err ;
			}
			if ( pDst != pDstElement->GetObject() )
			{
				pDstElement->SetObject( pDst ) ;
			}
		}
		else
		{
			pDstElement->SetObject( NULL ) ;
		}
	}
	ESLAssert( (unsigned int) j <= m_varArray.GetSize() ) ;
	m_varArray.RemoveBetween( j, m_varArray.GetSize() - j ) ;
	ESLAssert( m_varArray.GetSize() == obj.m_varArray.GetSize() ) ;
	return	eslErrSuccess ;
}

// デフォルト要素を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSHash::SetDefaultElement( ECSObject * pDefObj )
{
	delete	m_pDefObj ;
	m_pDefObj = pDefObj ;
}

// 配列要素有効性チェック
//////////////////////////////////////////////////////////////////////////////
bool ECSHash::VerifyAllElementValidation( void ) const
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ECSObject *	pElement = m_varArray.GetObjectAt( i ) ;
		if ( pElement != NULL )
		{
			if ( ((int) pElement->m_vtType < 0)
				|| ((int) pElement->m_vtType >= (int) csvtMax) )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// 配列要素削除
//////////////////////////////////////////////////////////////////////////////
void ECSHash::RemoveElementAt( ECSContext & context, int nIndex )
{
	ETaggedElement<ECSWideString,ECSObject> *
					pElement = m_varArray.GetAt( nIndex ) ;
	m_varArray.DetachAt( nIndex ) ;
	if ( pElement != NULL )
	{
		ECSObject *	pObj = pElement->DetachObject() ;
		if ( pObj != NULL )
		{
			context.delete_CSObject( pObj ) ;
		}
		delete	pElement ;
	}
}

void ECSHash::RemoveElementAs
	( ECSContext & context, const wchar_t * pwszTag )
{
	ECSObject *	pObj = m_varArray.DetachAs( pwszTag ) ;
	if ( pObj != NULL )
	{
		context.delete_CSObject( pObj ) ;
	}
}

void ECSHash::RemoveAll( ECSContext & context )
{
	int	nCount = m_varArray.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		ETaggedElement<ECSWideString,ECSObject> *
						pElement = m_varArray.GetAt( i ) ;
		if ( pElement != NULL )
		{
			ECSObject *	pObj = pElement->DetachObject() ;
			if ( pObj != NULL )
			{
				context.delete_CSObject( pObj ) ;
			}
		}
	}
	m_varArray.RemoveAll() ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSHash::GetTypeName( void ) const
{
	return	L"Hash" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSHash::Duplicate( void )
{
	ECSHash *	pHash = new ECSHash ;
	pHash->CopyFrom( *this ) ;
	//
	if ( m_pDefObj != NULL )
	{
		pHash->m_pDefObj = ECSTypeInfo::DuplicateType( m_pDefObj ) ;
	}
	//
	return	pHash ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	if ( pEntity->m_vtType != csvtHash )
	{
		return	ESLErrorMsg( "定義されていない Hash 型への代入です。" ) ;
	}
	if ( m_pDefObj != NULL )
	{
		ESLError	err = MoveFrom( context, *((ECSHash*)pEntity) ) ;
		if ( err )
		{
			return	err ;
		}
		ESLAssert( VerifyAllElementValidation() ) ;
	}
	else
	{
		m_varArray.RemoveAll( ) ;
		CopyFrom( *((ECSHash*)pEntity) ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない Hash 型の単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	if ( pEntity->m_vtType != csvtHash )
	{
		return	ESLErrorMsg( "定義されていない Hash 型への演算です。" ) ;
	}
	if ( csopType != csotAdd )
	{
		return	ESLErrorMsg( "定義されていない Hash 型への演算です。" ) ;
	}
	CopyFrom( *((ECSHash*)pEntity) ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない Hash 型の比較です。" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::GetVariableIndex( int & nIndex, int iMember )
{
	if ( (unsigned int) iMember < m_varArray.GetSize() )
	{
		nIndex = iMember ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "Hash 型の指標が範囲外です。" ) ;
}

ESLError ECSHash::GetVariableIndex
				( int & nIndex, const wchar_t * pwszMember )
{
	nIndex = -1 ;
	if ( m_varArray.GetAs( pwszMember, (unsigned int*) &nIndex ) == NULL )
	{
		if ( nIndex == -1 )
		{
			nIndex = m_varArray.Add( pwszMember, NULL ) ;
		}
	}
	return	eslErrSuccess ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSHash::GetVariableAt( int nIndex )
{
	ECSObject *	pObj = NULL ;
	ETaggedElement<ECSWideString,ECSObject> *	pElement ;
	pElement = m_varArray.GetAt( nIndex ) ;
	if ( pElement != NULL )
	{
		pObj = pElement->GetObject( ) ;
		if ( (pObj == NULL) && (m_pDefObj != NULL) )
		{
			pElement->SetObject( ECSTypeInfo::DuplicateType( m_pDefObj ) ) ;
			pObj = pElement->GetObject( ) ;
		}
	}
	return	pObj ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSHash::SetVariableAt( int nIndex, ECSObject * obj )
{
	ETaggedElement<ECSWideString,ECSObject> *	pElement ;
	pElement = m_varArray.GetAt( nIndex ) ;
	if ( pElement != NULL )
	{
		pElement->SetObject( obj ) ;
		return	pElement->GetObject( ) ;
	}
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg( "定義されていない関数を呼び出しています。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg( "不正な関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::OperateSizeOf( INT64 & nSize )
{
	nSize = m_varArray.GetSize() ;
	return	eslErrSuccess ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSHash::IndexAllMember( void )
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ETaggedElement<ECSWideString,ECSObject> *	pElement ;
		pElement = m_varArray.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		ECSObject *	pObj = pElement->GetObject( ) ;
		if ( pObj != NULL )
		{
			pObj->m_pParent = this ;
			pObj->m_nIndex = i ;
			pObj->IndexAllMember( ) ;
		}
	}
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSHash::CleanupAllReference( ECSContext & context )
{
	CleanupAllElementRef( context ) ;
	ECSObject::CleanupAllReference( context ) ;
}

// 配列要素の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSHash::CleanupAllElementRef( ECSContext & context )
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ETaggedElement<ECSWideString,ECSObject> *	pElement ;
		pElement = m_varArray.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		ECSObject *	pObj = pElement->GetObject( ) ;
		if ( pObj != NULL )
		{
			pObj->CleanupAllReference( context ) ;
		}
	}
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::CommitAllReference( ECSContext & context )
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ECSObject *	pObj = m_varArray.GetObjectAt( i ) ;
		if ( pObj != NULL )
		{
			ESLError	err = pObj->CommitAllReference( context ) ;
			if ( err )
			{
//				return	err ;
			}
		}
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Save( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwCount ;
	dwCount = m_varArray.GetSize( ) ;
	file.Write( &dwCount, sizeof(dwCount) ) ;
	//
	for ( int i = 0; i < (int) dwCount; i ++ )
	{
		DWORD	dwLength ;
		ETaggedElement<ECSWideString,ECSObject> *	pElement ;
		pElement = m_varArray.GetAt( i ) ;
		if ( pElement != NULL )
		{
			dwLength = pElement->Tag().GetLength() ;
			file.Write( &dwLength, sizeof(dwLength) ) ;
			file.Write
				( pElement->Tag().CharPtr(), dwLength * sizeof(wchar_t) ) ;
			//
			ECSObject *	pObj = pElement->GetObject( ) ;
			ESLError	err = context.SaveObject( file, pObj ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			dwLength = (DWORD) -1 ;
			file.Write( &dwLength, sizeof(dwLength) ) ;
		}
	}
	return	context.SaveObject( file, m_pDefObj ) ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Load( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	ESLErrorMsg( "連想配列の読み込みに失敗しました。" ) ;
	}
	m_varArray.RemoveAll( ) ;
	m_varArray.SetLimit( dwCount ) ;
	//
	for ( int i = 0; i < (int) dwCount; i ++ )
	{
		DWORD	dwLength ;
		ETaggedElement<ECSWideString,ECSObject> *	pElement = NULL ;
		if ( file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
		{
			return	ESLErrorMsg( "連想配列の読み込みに失敗しました。" ) ;
		}
		if ( dwLength != (DWORD) -1 )
		{
			pElement = new ETaggedElement<ECSWideString,ECSObject>( ) ;
			file.Read
				( pElement->Tag().GetBuffer(dwLength),
							dwLength * sizeof(wchar_t) ) ;
			pElement->Tag().ReleaseBuffer( dwLength ) ;
			//
			ECSObject *	pObj ;
			ESLError	err = context.LoadObject( file, pObj ) ;
			if ( err )
			{
				return	err ;
			}
			pElement->SetObject( pObj ) ;
		}
		m_varArray.SetAt( i, pElement ) ;
	}
	delete	m_pDefObj ;
	m_pDefObj = NULL ;
	return	context.LoadObject( file, m_pDefObj ) ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::DumpObject
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
		ETaggedElement<ECSWideString,ECSObject> *	pElement ;
		pElement = m_varArray.GetAt( i ) ;
		strDump = EString("\t") * nIndent + "\t[" + EString(i) + "] " ;
		if ( pElement != NULL )
		{
			strDump += "\"" + EString(pElement->Tag()) + "\" : " ;
			buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			//
			ECSObject *	pObj = pElement->GetObject( ) ;
			if ( pObj != NULL )
			{
				strDump = EString( pObj->GetTypeName() ) ;
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
				strDump = "<nothing>" ;
			}
		}
		else
		{
			strDump += ": <nothing>" ;
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
void ECSHash::OnDestruction( ECSContext & context )
{
	RemoveAll( context ) ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSHash::m_staFuncName = NULL ;
const wchar_t *		ECSHash::m_pwszFuncName[10] =
{
	L"GetLength", L"IsEmpty", L"GetTagName",
	L"FindTagIndex", L"OrderIndex",
	L"Detach", L"Remove", L"RemoveAll", L"SetDefaultElement", NULL
} ;
const ECSHash::PFUNC_CALL	ECSHash::m_pfnCallFunc[9] =
{
	&ECSHash::Call_GetLength, &ECSHash::Call_IsEmpty,
	&ECSHash::Call_GetTagName, &ECSHash::Call_FindTagIndex,
	&ECSHash::Call_OrderIndex,
	&ECSHash::Call_Detach, &ECSHash::Call_Remove, &ECSHash::Call_RemoveAll,
	&ECSHash::Call_SetDefaultElement,
} ;

// メンバ関数 : Integer GetLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_GetLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( new ECSInteger( (int) m_varArray.GetSize() ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer IsEmpty( index )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_IsEmpty
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObjIndex = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObjIndex == NULL )
	{
		return	ESLErrorMsg( "指標が指定されていません。" ) ;
	}
	long int	nResult = 0 ;
	if ( pObjIndex->m_vtType == csvtInteger )
	{
		nResult =
			- (long int) (m_varArray.GetAt
				( ((ECSInteger*) pObjIndex)->GetInt() ) == NULL) ;
	}
	else if ( pObjIndex->m_vtType == csvtString )
	{
		nResult =
			- (long int) (m_varArray.GetAs
				( ((ECSString*) pObjIndex)->m_varStr.CharPtr() ) == NULL) ;
	}
	else
	{
		return	ESLErrorMsg
			( "指標は整数、または文字列でなければなりません。" ) ;
	}
	return	context.PushObject( new ECSInteger( nResult ) ) ;
}

// メンバ関数 : String GetTagName( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_GetTagName
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int		i ;
	err = context.GetArgumentAsInt( i, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ETaggedElement<ECSWideString,ECSObject> *	pElement ;
	ECSString *	pStr = new ECSString ;
	pElement = m_varArray.GetAt( i ) ;
	if ( pElement != NULL )
	{
		pStr->m_varStr = pElement->Tag() ;
	}
	context.PushObject( pStr ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer FindTagIndex( String tag )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_FindTagIndex
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrTag ;
	err = context.GetArgumentAsStr( wstrTag, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	unsigned int	nIndex ;
	if ( m_varArray.GetAs( wstrTag, &nIndex ) == NULL )
	{
		nIndex = -1 ;
	}
	return	context.PushObject( new ECSInteger( nIndex ) ) ;
}

// メンバ関数 : Integer OrderIndex( String tag )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_OrderIndex
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrTag ;
	err = context.GetArgumentAsStr( wstrTag, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	unsigned int	nIndex = m_varArray.OrderIndex( wstrTag ) ;
	return	context.PushObject( context.new_CSInteger( nIndex ) ) ;
}

// メンバ関数 : Reference Detach( String tag )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_Detach
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrTag ;
	err = context.GetArgumentAsStr( wstrTag, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = m_varArray.DetachAs( wstrTag ) ;
	if ( pObj != NULL )
	{
		return	context.PushObject( pObj ) ;
	}
	else
	{
		return	context.PushObject( context.new_CSReference() ) ;
	}
}

// メンバ関数 : Remove( String tag )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_Remove
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrTag ;
	err = context.GetArgumentAsStr( wstrTag, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	RemoveElementAs( context, wstrTag ) ;
	//
	context.PushObject( context.new_CSInteger() ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : RemoveAll()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_RemoveAll
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	RemoveAll( context ) ;
	//
	context.PushObject( context.new_CSInteger() ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : SetDefaultElement( [object] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSHash::Call_SetDefaultElement
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	context.delete_CSObject( m_pDefObj ) ;
	m_pDefObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( m_pDefObj != NULL )
	{
		m_pDefObj = ECSTypeInfo::DuplicateType( m_pDefObj ) ;
	}
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// 整数のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
int ECSHash::GetMemberAsInt( const wchar_t * pwszName, int nDefValue )
{
	ECSObject *	pObj = m_varArray.GetAs( pwszName ) ;
	if ( pObj != NULL )
	{
		ECSSourceStream	css ;
		switch ( pObj->m_vtType )
		{
		case	csvtInteger:
			return	((ECSInteger*)pObj)->GetInt() ;
		case	csvtReal:
			return	(int) ::eriRoundR64ToLInt( ((ECSReal*)pObj)->m_varReal ) ;
		case	csvtString:
			css = ((ECSString*)pObj)->m_varStr ;
			return	css.GetInteger( ) ;
		}
	}
	return	nDefValue ;
}

// 実数のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
double ECSHash::GetMemberAsReal( const wchar_t * pwszName, double rDefValue )
{
	ECSObject *	pObj = m_varArray.GetAs( pwszName ) ;
	if ( pObj != NULL )
	{
		ECSSourceStream	css ;
		switch ( pObj->m_vtType )
		{
		case	csvtReal:
			return	((ECSReal*)pObj)->m_varReal ;
		case	csvtInteger:
			return	(double) ((ECSInteger*)pObj)->GetValue( ) ;
		case	csvtString:
			css = ((ECSString*)pObj)->m_varStr ;
			return	css.GetRealNumber( ) ;
		}
	}
	return	rDefValue ;
}

// 文字列のメンバ変数を取得する
//////////////////////////////////////////////////////////////////////////////
ECSWideString ECSHash::GetMemberAsStr
	( const wchar_t * pwszName, const wchar_t * pwszDefValue )
{
	ECSObject *	pObj = m_varArray.GetAs( pwszName ) ;
	if ( pObj != NULL )
	{
		switch ( pObj->m_vtType )
		{
		case	csvtString:
			return	((ECSString*)pObj)->m_varStr ;
		case	csvtInteger:
			return	((ECSInteger*)pObj)->GetInt() ;
		case	csvtReal:
			return	(double) ((ECSReal*)pObj)->m_varReal ;
		}
	}
	return	pwszDefValue ;
}

// 整数のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSHash::SetMemberAsInt( const wchar_t * pwszName, int nValue )
{
	ECSInteger *	pObj = (ECSInteger*) m_varArray.GetAs( pwszName ) ;
	if ( (pObj == NULL) || (pObj->m_vtType != csvtInteger) )
	{
		m_varArray.SetAs( pwszName, new ECSInteger( nValue ) ) ;
	}
	else
	{
		pObj->SetValue( nValue ) ;
	}
}

// 実数のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSHash::SetMemberAsReal( const wchar_t * pwszName, double rValue )
{
	ECSReal *	pObj = (ECSReal*) m_varArray.GetAs( pwszName ) ;
	if ( (pObj == NULL) || (pObj->m_vtType != csvtReal) )
	{
		m_varArray.SetAs( pwszName, new ECSReal( rValue ) ) ;
	}
	else
	{
		pObj->m_varReal = rValue ;
	}
}

// 文字列のメンバ変数を設定する
//////////////////////////////////////////////////////////////////////////////
void ECSHash::SetMemberAsStr
	( const wchar_t * pwszName, const wchar_t * pwszValue )
{
	ECSString *	pObj = (ECSString*) m_varArray.GetAs( pwszName ) ;
	if ( (pObj == NULL) || (pObj->m_vtType != csvtString) )
	{
		m_varArray.SetAs( pwszName, new ECSString( pwszValue ) ) ;
	}
	else
	{
		pObj->m_varStr = pwszValue ;
	}
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSHash::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::Compare( pwszType, L"ECS_HASH_INTERFACE" ) )
	{
		m_pih.pBackLink = this ;
		m_pih.pfnGetLength = PIC_GetLength ;
		m_pih.pfnGetElement = PIC_GetElement ;
		m_pih.pfnGetTagName = PIC_GetTagName ;
		m_pih.pfnRemove = PIC_Remove ;
		m_pih.pfnRemoveAll = PIC_RemoveAll ;
		m_pih.pfnSetDefaultElement = PIC_SetDefaultElement ;
		return	(ECS_HASH_INTERFACE*) &m_pih ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

unsigned int __stdcall ECSHash::PIC_GetLength
	( ECS_HASH_INTERFACE * instance )
{
	PLUGIN_HASH *	ppih = (PLUGIN_HASH*) instance ;
	ESLAssert( &(ppih->pBackLink->m_pih) == ppih ) ;
	return	ppih->pBackLink->m_varArray.GetSize( ) ;
}

ECS_OBJECT * __stdcall ECSHash::PIC_GetElement
	( ECS_HASH_INTERFACE * instance, const wchar_t * pwszTag )
{
	PLUGIN_HASH *	ppih = (PLUGIN_HASH*) instance ;
	ESLAssert( &(ppih->pBackLink->m_pih) == ppih ) ;
	ECSObject *	pObj = ppih->pBackLink->m_varArray.GetAs( pwszTag ) ;
	if ( pObj != NULL )
	{
		return	pObj->CreateInterface( ) ;
	}
	return	NULL ;
}

ECS_OBJECT * __stdcall ECSHash::PIC_GetTagName
	( ECS_HASH_INTERFACE * instance, int nIndex )
{
	PLUGIN_HASH *	ppih = (PLUGIN_HASH*) instance ;
	ESLAssert( &(ppih->pBackLink->m_pih) == ppih ) ;
	ETaggedElement<ECSWideString,ECSObject> *	pElement ;
	pElement = ppih->pBackLink->m_varArray.GetAt( nIndex ) ;
	if ( pElement != NULL )
	{
		ECSString *	pstrTagName = new ECSString( pElement->Tag() ) ;
		return	pstrTagName->CreateInterface( ) ;
	}
	return	NULL ;
}

void __stdcall ECSHash::PIC_Remove
	( ECS_HASH_INTERFACE * instance, const wchar_t * pwszTag )
{
	PLUGIN_HASH *	ppih = (PLUGIN_HASH*) instance ;
	ESLAssert( &(ppih->pBackLink->m_pih) == ppih ) ;
	ppih->pBackLink->m_varArray.RemoveAs( pwszTag ) ;
}

void __stdcall ECSHash::PIC_RemoveAll( ECS_HASH_INTERFACE * instance )
{
	PLUGIN_HASH *	ppih = (PLUGIN_HASH*) instance ;
	ESLAssert( &(ppih->pBackLink->m_pih) == ppih ) ;
	ppih->pBackLink->m_varArray.RemoveAll( ) ;
}

void __stdcall ECSHash::PIC_SetDefaultElement
	( ECS_HASH_INTERFACE * instance, ECS_OBJECT * pDefault )
{
	PLUGIN_HASH *	ppih = (PLUGIN_HASH*) instance ;
	ESLAssert( &(ppih->pBackLink->m_pih) == ppih ) ;
	ECSObject::PLUGIN_OBJECT *	ppio = (ECSObject::PLUGIN_OBJECT*) pDefault ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	delete	ppih->pBackLink->m_pDefObj ;
	ppih->pBackLink->m_pDefObj = ppio->pBackLink ;
}
