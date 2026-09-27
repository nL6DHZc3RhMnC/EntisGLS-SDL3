
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 配列オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSArray, ECSObject )

// 要素を複製する
//////////////////////////////////////////////////////////////////////////////
void ECSArray::CopyFrom( ECSArray & obj, int iFirst, int nCount )
{
	m_varArray.SetLimit
		( m_varArray.GetSize() + obj.m_varArray.GetSize() ) ;
	//
	if ( nCount == -1 )
	{
		nCount = obj.m_varArray.GetSize() - iFirst ;
	}
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( i + iFirst >= (int) obj.m_varArray.GetSize() )
		{
			break ;
		}
		ECSObject *	pElement = obj.m_varArray.GetAt( i + iFirst ) ;
		if ( pElement != NULL )
		{
			if ( pElement->m_vtType == csvtReference )
			{
				pElement =
					new ECSReference
						( ((ECSReference*)pElement)->m_pRef ) ;
			}
			else
			{
				pElement = pElement->Duplicate( ) ;
			}
		}
		m_varArray.Add( pElement ) ;
	}
}

// 要素を代入する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::MoveFrom( ECSContext & context, ECSArray & obj )
{
	int	i, nCount ;
	nCount = obj.m_varArray.GetSize() ;
	//
	m_varArray.SetSize( nCount ) ;
	//
	for ( i = 0; i < nCount; i ++ )
	{
		ECSObject *	pSrc = obj.m_varArray.GetAt( i ) ;
		if ( pSrc != NULL )
		{
			ECSObject *	pDst = m_varArray.GetAt( i ) ;
			bool		fNewDst = (pDst == NULL) ;
			ESLError	err =
				MoveElement( context, pDst, *pSrc, m_pDefObj ) ;
			if ( err )
			{
				return	err ;
			}
			if ( fNewDst )
			{
				m_varArray.SetAt( i, pDst ) ;
			}
		}
		else
		{
			m_varArray.SetAt( i, NULL ) ;
		}
	}
	ESLAssert( VerifyAllElementValidation() ) ;
	return	eslErrSuccess ;
}

// 要素代入処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::MoveElement
	( ECSContext & context, ECSObject *& pDst,
			ECSObject & objSrc, ECSObject * pDefValue )
{
	if ( pDst == NULL )
	{
		if ( pDefValue != NULL )
		{
			pDst = ECSTypeInfo::DuplicateType( pDefValue ) ;
		}
		else if ( objSrc.m_vtType == csvtReference )
		{
			pDst = context.new_CSReference
						( ((ECSReference*)&objSrc)->m_pRef ) ;
			return	eslErrSuccess ;
		}
		else
		{
			pDst = objSrc.Duplicate() ;
			return	eslErrSuccess ;
		}
	}
	if ( pDst->m_vtType == objSrc.m_vtType )
	{
		switch ( objSrc.m_vtType )
		{
		case	csvtObject:
		default:
			break ;
		case	csvtReference:
			((ECSReference*)pDst)->SetReferenceCastInterface
				( ((ECSReference*)&objSrc)->m_pRef,
						&context, *((ECSReference*)&objSrc) ) ;
			return	eslErrSuccess ;
		case	csvtArray:
			return	((ECSArray*)pDst)->
						MoveFrom( context, *((ECSArray*)&objSrc) ) ;
		case	csvtHash:
			return	((ECSHash*)pDst)->
						MoveFrom( context, *((ECSHash*)&objSrc) ) ;
		case	csvtInteger:
			((ECSInteger*)pDst)->
						SetValue( ((ECSInteger*)&objSrc)->GetValue() ) ;
			return	eslErrSuccess ;
		case	csvtReal:
			((ECSReal*)pDst)->m_varReal
							= ((ECSReal*)&objSrc)->m_varReal ;
			return	eslErrSuccess ;
		case	csvtString:
			((ECSString*)pDst)->m_varStr
							= ((ECSString*)&objSrc)->m_varStr ;
			return	eslErrSuccess ;
		}
	}
	return	pDst->Move( context, context.new_CSReference( &objSrc ) ) ;
}

// 配列要素削除
//////////////////////////////////////////////////////////////////////////////
void ECSArray::RemoveBetween( ECSContext & context, int nIndex, int nCount )
{
	if ( nIndex < 0 )
	{
		ESLAssert( nIndex >= 0 ) ;
		nIndex = 0 ;
	}
	if ( nCount == -1 )
	{
		nCount = m_varArray.GetSize() - nIndex ;
	}
	for ( int i = 0; i < nCount; i ++ )
	{
		ECSObject *	pObj = m_varArray.GetAt( nIndex + i ) ;
		if ( pObj != NULL )
		{
			context.delete_CSObject( pObj ) ;
		}
	}
	m_varArray.DetachBetween( nIndex, nCount ) ;
}

// メンバ有効性チェック
//////////////////////////////////////////////////////////////////////////////
bool ECSArray::VerifyAllElementValidation( void ) const
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ECSObject *	pElement = m_varArray.GetAt( i ) ;
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

// デフォルト要素
//////////////////////////////////////////////////////////////////////////////
void ECSArray::SetDefaultElement( ECSObject * pDefObj )
{
	delete	m_pDefObj ;
	m_pDefObj = pDefObj ;
}

// 多次元配列生成
//////////////////////////////////////////////////////////////////////////////
void ECSArray::MakeDimension
	( const unsigned int nBounds[], int nDim, ECSObject * pDefObj )
{
	if ( nDim == 1 )
	{
		SetBounds( nBounds[0] ) ;
		SetDefaultElement( pDefObj ) ;
	}
	else if ( nDim > 1 )
	{
		ECSArray *	pSubArray = new ECSArray ;
		pSubArray->MakeDimension( &(nBounds[1]), nDim - 1, pDefObj ) ;
		//
		SetBounds( nBounds[0] ) ;
		SetDefaultElement( pSubArray ) ;
	}
}

// （デフォルト要素の設定から）次元取得
//////////////////////////////////////////////////////////////////////////////
int ECSArray::GetDimension( void ) const
{
	const ECSArray *	pArray = this ;
	int	nDim = 1 ;
	while ( (pArray->m_pDefObj != NULL)
		&& (pArray->m_pDefObj->m_vtType == csvtArray) )
	{
		nDim ++ ;
		pArray = (const ECSArray *) pArray->m_pDefObj ;
	}
	return	nDim ;
}

// （デフォルト要素の設定から）次元サイズ取得
//////////////////////////////////////////////////////////////////////////////
int ECSArray::GetDimensionSize( unsigned int nBounds[], int nDim ) const
{
	const ECSArray *	pArray = this ;
	int	i = 0 ;
	for ( ; ; )
	{
		if ( i < nDim )
		{
			nBounds[i] = pArray->m_nBounds ;
		}
		i ++ ;
		if ( (pArray->m_pDefObj == NULL)
			|| (pArray->m_pDefObj->m_vtType != csvtArray) )
		{
			break ;
		}
		pArray = (const ECSArray *) pArray->m_pDefObj ;
	}
	return	i ;
}

// （デフォルト要素の設定から）末端デフォルト値取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSArray::GetEndDefaultElement( void ) const
{
	const ECSArray *	pArray = this ;
	while ( (pArray->m_pDefObj != NULL)
		&& (pArray->m_pDefObj->m_vtType == csvtArray) )
	{
		pArray = (const ECSArray *) pArray->m_pDefObj ;
	}
	ESLAssert( pArray != NULL ) ;
	return	pArray->m_pDefObj ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSArray::GetTypeName( void ) const
{
	return	L"Array" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSArray::Duplicate( void )
{
	ECSArray *	pArray = new ECSArray ;
	pArray->CopyFrom( *this ) ;
	//
	if ( m_pDefObj != NULL )
	{
		pArray->m_pDefObj = ECSTypeInfo::DuplicateType( m_pDefObj ) ;
	}
	pArray->m_nBounds = m_nBounds ;
	//
	return	pArray ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity(obj) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	if ( pEntity->m_vtType != csvtArray )
	{
		return	ESLErrorMsg( "定義されていない Array 型への変換です。" ) ;
	}
	if ( m_pDefObj != NULL )
	{
		ESLError	err = MoveFrom( context, *((ECSArray*)pEntity) ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		m_varArray.RemoveAll( ) ;
		CopyFrom( *((ECSArray*)pEntity) ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない Array 型の単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	if ( csopType != csotAdd )
	{
		return	ESLErrorMsg( "定義されていない Array 型の演算です。" ) ;
	}
	if ( obj->m_vtType == csvtReference )
	{
		ECSObject *	pEntity = ECSObject::GetEntity(obj) ;
		if ( pEntity->IsValidObject() )
		{
			m_varArray.Add( pEntity->Duplicate( ) ) ;
			context.delete_CSObject( obj ) ;
		}
		else
		{
			m_varArray.Add( obj ) ;
		}
	}
	else
	{
		m_varArray.Add( obj ) ;
	}
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない Array 型の比較演算です。" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::GetVariableIndex( int & nIndex, int iMember )
{
	if ( (unsigned int) iMember < m_nBounds )
	{
		nIndex = iMember ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "配列の境界を越えています。" ) ;
}

ESLError ECSArray::GetVariableIndex( int & nIndex, const wchar_t * pwszMember )
{
	return	ESLErrorMsg( "Array 型の指標は整数型でなければなりません。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSArray::GetVariableAt( int nIndex )
{
	ECSObject *	pObj = m_varArray.GetAt( nIndex ) ;
	if ( pObj == NULL )
	{
		if ( (m_pDefObj != NULL) && ((unsigned int) nIndex < m_nBounds) )
		{
			pObj = ECSTypeInfo::DuplicateType( m_pDefObj ) ;
			m_varArray.SetAt( nIndex, pObj ) ;
		}
	}
	return	pObj ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSArray::SetVariableAt( int nIndex, ECSObject * obj )
{
	if ( (unsigned int) nIndex < m_nBounds )
	{
		m_varArray.SetAt( nIndex, obj ) ;
		return	obj ;
	}
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::GetFunction
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
ESLError ECSArray::CallFunction
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
ESLError ECSArray::OperateSizeOf( INT64 & nSize )
{
	nSize = m_varArray.GetSize() ;
	return	eslErrSuccess ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSArray::IndexAllMember( void )
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ECSObject *	pElement = m_varArray.GetAt( i ) ;
		if ( pElement != NULL )
		{
			pElement->m_pParent = this ;
			pElement->m_nIndex = i ;
			pElement->IndexAllMember( ) ;
		}
	}
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSArray::CleanupAllReference( ECSContext & context )
{
	CleanupAllElementRef( context ) ;
	ECSObject::CleanupAllReference( context ) ;
}

// 配列要素の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSArray::CleanupAllElementRef( ECSContext & context )
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ECSObject *	pElement = m_varArray.GetAt( i ) ;
		if ( pElement != NULL )
		{
			pElement->CleanupAllReference( context ) ;
		}
	}
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::CommitAllReference( ECSContext & context )
{
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ECSObject *	pElement = m_varArray.GetAt( i ) ;
		if ( pElement != NULL )
		{
			ESLError	err = pElement->CommitAllReference( context ) ;
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
ESLError ECSArray::Save( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength = m_varArray.GetSize( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	//
	for ( int i = 0; i < (int) dwLength; i ++ )
	{
		ESLError	err = context.SaveObject( file, m_varArray.GetAt(i) ) ;
		if ( err )
		{
			return	err ;
		}
	}
	ESLError	err = context.SaveObject( file, m_pDefObj ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_nBounds, sizeof(m_nBounds) ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Load( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength ;
	if ( (file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength))
		|| (dwLength >= 0x40000000) )
	{
		return	ESLErrorMsg( "配列オブジェクトの読み込みに失敗しました。" ) ;
	}
	m_varArray.RemoveAll( ) ;
	m_varArray.SetSize( dwLength ) ;
	//
	for ( int i = 0; i < (int) dwLength; i ++ )
	{
		ECSObject *	pObj ;
		ESLError	err = context.LoadObject( file, pObj ) ;
		if ( err )
		{
			return	err ;
		}
		m_varArray.SetAt( i, pObj ) ;
	}
	delete	m_pDefObj ;
	m_pDefObj = NULL ;
	ESLError	err = context.LoadObject( file, m_pDefObj ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &m_nBounds, sizeof(m_nBounds) ) ;
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = "要素数 = " ;
	strDump += EString( (int) m_varArray.GetSize() ) ;
	strDump += "\r\n" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		ECSObject *	pObj = m_varArray.GetAt( i ) ;
		if ( pObj != NULL )
		{
			strDump = EString("\t") * nIndent + "\t[" + EString(i) + "] = " ;
			strDump += EString( pObj->GetTypeName() ) ;
			strDump += " : " ;
			buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			strDump = "" ;
			pObj->DumpObject( buf, nIndent + 1, context ) ;
			if ( i + 1 < (int) m_varArray.GetSize() )
			{
				strDump += "\r\n" ;
			}
			buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
		}
	}
	return	eslErrSuccess ;
}

// スクリプトのデストラクタ
//////////////////////////////////////////////////////////////////////////////
void ECSArray::OnDestruction( ECSContext & context )
{
	RemoveBetween( context ) ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSArray::m_staFuncName = NULL ;
const wchar_t *		ECSArray::m_pwszFuncName[11] =
{
	L"GetLength", L"IsEmpty", L"Find", L"Swap",
	L"Push", L"Pop", L"Insert", L"Remove", L"Detach", L"Merge", NULL
} ;
const ECSArray::PFUNC_CALL	ECSArray::m_pfnCallFunc[10] =
{
	&ECSArray::Call_GetLength, &ECSArray::Call_IsEmpty,
	&ECSArray::Call_Find, &ECSArray::Call_Swap,
	&ECSArray::Call_Push, &ECSArray::Call_Pop,
	&ECSArray::Call_Insert, &ECSArray::Call_Remove,
	&ECSArray::Call_Detach, &ECSArray::Call_Merge
} ;

// メンバ関数 : Integer GetLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_GetLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( new ECSInteger( (int) m_varArray.GetSize() ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer IsEmpty( Integer nIndex )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_IsEmpty
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
	return	context.PushObject( new ECSInteger
				( - (long int) (m_varArray.GetAt(nIndex) == NULL) ) ) ;
}

// メンバ関数 : Find( object, Integer nIndex := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Find
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt( 1 ) ) ;
	int	nFind = -1 ;
	if ( pObj != NULL )
	{
		int	nIndex ;
		err = context.GetArgumentAsInt( nIndex, lstArg, 2, 0 ) ;
		if ( err )
			return	err ;
		//
		while ( nIndex < (int) m_varArray.GetSize() )
		{
			ECSObject *	pElement = m_varArray.GetAt( nIndex ) ;
			if ( pElement != NULL )
			{
				int	nResult = 0 ;
				err = pElement->Compare
					( context, nResult, csctEqual, *pObj ) ;
				if ( !err && nResult )
				{
					nFind = nIndex ;
					break ;
				}
			}
			nIndex ++ ;
		}
	}
	return	context.PushObject( context.new_CSInteger( nFind ) ) ;
}

// メンバ関数 : Swap( Integer i, Integer j )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Swap
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		i, j ;
	err = context.GetArgumentAsInt( i, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( j, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	m_varArray.Swap( i, j ) ;
	context.PushObject( new ECSInteger ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Push( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Push
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj ;
	pObj = lstArg.GetAt( 1 ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "第1引数が指定されていません。" ) ;
	}
	if ( (m_pDefObj != NULL) && (m_pDefObj->m_vtType == csvtReference) )
	{
		ECSReference *	pRef = context.new_CSReference() ;
		if ( pObj->m_vtType == csvtReference )
		{
			pRef->SetReferenceCastInterface
				( ((ECSReference*)pObj)->m_pRef,
					&context, *((ECSReference*)pObj) ) ;
		}
		else
		{
			pRef->SetOwnObject( pObj ) ;
			lstArg.DetachAt( 1 ) ;
			lstArg.InsertAt( 1, NULL ) ;
		}
		pObj = pRef ;
	}
	else
	{
		if ( pObj->m_vtType == csvtReference )
		{
			pObj = pObj->Duplicate() ;
		}
		else
		{
			lstArg.DetachAt( 1 ) ;
			lstArg.InsertAt( 1, NULL ) ;
		}
	}
	if ( m_varArray.GetSize() >= GetBounds() )
	{
		return	ESLErrorMsg( "配列境界サイズを超えています。" ) ;
	}
	m_varArray.Push( pObj ) ;
	return	context.PushObject( new ECSInteger() ) ;
}

// メンバ関数 : object Pop()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Pop
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
//	ECSReference *	pRef ;
	ECSObject *	pObj = m_varArray.Pop() ;
	if ( pObj != NULL )
	{
		return	context.PushObject( pObj ) ;
/*		if ( pObj->m_vtType == csvtReference )
		{
			pRef = (ECSReference*) pObj ;
		}
		else
		{
			pRef = context.new_CSReference() ;
			pRef->SetOwnObject( pObj, &context ) ;
		}
*/	}
	else
	{
		return	context.PushObject( context.new_CSReference() ) ;
	}
}

// メンバ関数 : Insert( Integer i, object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Insert
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		i ;
	ECSObject *	pObj ;
	err = context.GetArgumentAsInt( i, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	pObj = lstArg.GetAt( 2 ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "第2引数が指定されていません。" ) ;
	}
	if ( (m_pDefObj != NULL) && (m_pDefObj->m_vtType == csvtReference) )
	{
		ECSReference *	pRef = context.new_CSReference() ;
		if ( pObj->m_vtType == csvtReference )
		{
			pRef->SetReferenceCastInterface
				( ((ECSReference*)pObj)->m_pRef,
					&context, *((ECSReference*)pObj) ) ;
		}
		else
		{
			lstArg.DetachAt( 2 ) ;
			lstArg.InsertAt( 2, NULL ) ;
			pRef->SetOwnObject( pObj ) ;
		}
		pObj = pRef ;
	}
	else
	{
		if ( pObj->m_vtType == csvtReference )
		{
			pObj = pObj->Duplicate() ;
		}
		else
		{
			lstArg.DetachAt( 2 ) ;
			lstArg.InsertAt( 2, NULL ) ;
		}
	}
	if ( i < 0 )
	{
		ESLAssert( i >= 0 ) ;
		i = 0 ;
	}
	if ( ((unsigned int) i >= GetBounds())
		|| (m_varArray.GetSize() >= GetBounds()) )
	{
		return	ESLErrorMsg( "配列境界サイズを超えています。" ) ;
	}
	m_varArray.InsertAt( i, pObj ) ;
	context.PushObject( new ECSInteger ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Remove(  Integer i := 0, Integer n := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Remove
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int		i, n ;
	err = context.GetArgumentAsInt( i, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( n, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	//
	RemoveBetween( context, i, n ) ;
	//
	context.PushObject( new ECSInteger ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Detach( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Detach
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
	ECSObject *	pObj = m_varArray.GetAt( i ) ;
	m_varArray.DetachAt( i ) ;
	if ( pObj != NULL )
	{
		while ( pObj->m_vtType == csvtReference )
		{
			ECSReference *	pRef = (ECSReference*) pObj ;
			if ( pRef->m_pOwnObj == NULL )
			{
				break ;
			}
			pObj = pRef->DetachObject( context ) ;
			context.delete_CSObject( pRef ) ;
		}
		context.PushObject( pObj ) ;
	}
	else
	{
		context.PushObject( new ECSReference ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数 ; Merge( Array arg )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSArray::Call_Merge
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pObj = (ECSArray*) ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( (pObj == NULL) || (pObj->m_vtType != csvtArray) )
	{
		return	ESLErrorMsg( "引数が指定されていません。" ) ;
	}
	if ( m_varArray.GetSize() + pObj->m_varArray.GetSize() >= GetBounds() )
	{
		return	ESLErrorMsg( "配列境界サイズを超えています。" ) ;
	}
	CopyFrom( *pObj ) ;
	context.PushObject( new ECSInteger ) ;
	return	eslErrSuccess ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSArray::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::Compare( pwszType, L"ECS_ARRAY_INTERFACE" ) )
	{
		m_pia.pBackLink = this ;
		m_pia.pfnGetLength = PIC_GetLength ;
		m_pia.pfnIsEmpty = PIC_IsEmpty ;
		m_pia.pfnSwap = PIC_Swap ;
		m_pia.pfnInsert = PIC_Insert ;
		m_pia.pfnRemove = PIC_Remove ;
		m_pia.pfnDetach = PIC_Detach ;
		return	(ECS_ARRAY_INTERFACE*) &m_pia ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

unsigned int __stdcall
	ECSArray::PIC_GetLength( ECS_ARRAY_INTERFACE * instance )
{
	PLUGIN_ARRAY *	ppia = (PLUGIN_ARRAY*) instance ;
	ESLAssert( &(ppia->pBackLink->m_pia) == ppia ) ;
	return	ppia->pBackLink->m_varArray.GetSize( ) ;
}

int __stdcall ECSArray::PIC_IsEmpty
	( ECS_ARRAY_INTERFACE * instance, int nIndex )
{
	PLUGIN_ARRAY *	ppia = (PLUGIN_ARRAY*) instance ;
	ESLAssert( &(ppia->pBackLink->m_pia) == ppia ) ;
	return	(ppia->pBackLink->m_varArray.GetAt(nIndex) == NULL) ;
}

void __stdcall ECSArray::PIC_Swap
	( ECS_ARRAY_INTERFACE * instance, int nIndex1, int nIndex2 )
{
	PLUGIN_ARRAY *	ppia = (PLUGIN_ARRAY*) instance ;
	ESLAssert( &(ppia->pBackLink->m_pia) == ppia ) ;
	ppia->pBackLink->m_varArray.Swap( nIndex1, nIndex2 ) ;
}

void __stdcall ECSArray::PIC_Insert
	( ECS_ARRAY_INTERFACE * instance, int nIndex, ECS_OBJECT * pObj )
{
	PLUGIN_ARRAY *	ppia = (PLUGIN_ARRAY*) instance ;
	ESLAssert( &(ppia->pBackLink->m_pia) == ppia ) ;
	ECSObject::PLUGIN_OBJECT *	ppio = (ECSObject::PLUGIN_OBJECT*) pObj ;
	ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
	ppia->pBackLink->m_varArray.InsertAt( nIndex, ppio->pBackLink ) ;
}

void __stdcall ECSArray::PIC_Remove
	( ECS_ARRAY_INTERFACE * instance, int nIndex, int nCount )
{
	PLUGIN_ARRAY *	ppia = (PLUGIN_ARRAY*) instance ;
	ESLAssert( &(ppia->pBackLink->m_pia) == ppia ) ;
	ppia->pBackLink->m_varArray.RemoveBetween( nIndex, nCount ) ;
}

ECS_OBJECT * __stdcall ECSArray::PIC_Detach
	( ECS_ARRAY_INTERFACE * instance, int nIndex )
{
	PLUGIN_ARRAY *	ppia = (PLUGIN_ARRAY*) instance ;
	ESLAssert( &(ppia->pBackLink->m_pia) == ppia ) ;
	ECSObject *	pObj = ppia->pBackLink->m_varArray.GetAt( nIndex ) ;
	ppia->pBackLink->m_varArray.DetachAt( nIndex ) ;
	return	pObj->CreateInterface( ) ;
}
