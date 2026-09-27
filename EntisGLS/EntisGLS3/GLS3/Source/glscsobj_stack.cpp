
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 名前付きスタック
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSStack, ECSArray )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSStack::ECSStack( void )
{
	m_pCurrent = NULL ;
	m_nCurrent = 1 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSStack::~ECSStack( void )
{
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSStack::GetTypeName( void ) const
{
	return	L"Stack" ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::Move( ECSContext & context, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない代入操作です。" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "定義されていない単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない演算操作です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "定義されていない比較操作です。" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::GetVariableIndex( int & nIndex, int iMember )
{
	if ( iMember >= 0 )
	{
		nIndex = iMember ;
		if ( m_pCurrent != NULL )
		{
			nIndex += m_pCurrent->m_iBound ;
		}
	}
	else
	{
		nIndex = (int) m_varArray.GetSize() + iMember ;
	}
	return	eslErrSuccess ;
}

ESLError ECSStack::GetVariableIndex
				( int & nIndex, const wchar_t * pwszMember )
{
	for ( int j = 0; j < m_nCurrent; j ++ )
	{
		EStackBlock *	pBlock = m_block.GetLastAt( j ) ;
		if ( pBlock == NULL )
		{
			return	ESLErrorMsg( "スタックに名前空間が存在しません。" ) ;
		}
		nIndex = pBlock->FindIndex( pwszMember ) ;
		if ( nIndex >= 0 )
		{
			nIndex += pBlock->m_iBound ;
			return	eslErrSuccess ;
		}
	}
	return	ESLErrorMsg
		( "スタック上に変数が見つかりませんでした。" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	return	ESLErrorMsg( "定義されていない関数の呼び出しです。" ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	return	ESLErrorMsg( "定義されていない関数の呼び出しです。" ) ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::OperateSizeOf( INT64 & nSize )
{
	return	ESLErrorMsg( "stack への不正な sizeof 演算子です。" ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSArray::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwLength = m_block.GetSize() ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	for ( DWORD i = 0; i < dwLength; i ++ )
	{
		EStackBlock *	pBlock = m_block.GetAt( i ) ;
		ESLAssert( pBlock != NULL ) ;
		if ( pBlock == NULL )
		{
			pBlock = new EStackBlock ;
			m_block.SetAt( i, pBlock ) ;
		}
		file.Write( &(pBlock->m_dwFlags), sizeof(DWORD) ) ;
		DWORD	dwStrLen = pBlock->m_pwstrName->GetLength( ) ;
		file.Write( &dwStrLen, sizeof(DWORD) ) ;
		if ( dwStrLen > 0 )
		{
			file.Write
				( pBlock->m_pwstrName->CharPtr(),
						dwStrLen * sizeof(wchar_t) ) ;
		}
		file.Write( &(pBlock->m_iBound), sizeof(int) ) ;
		file.Write( &(pBlock->m_nVarCount), sizeof(int) ) ;
		file.Write( &(pBlock->m_dwCatchAddr), sizeof(DWORD) ) ;
		//
		err = pBlock->SaveArray( file ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSArray::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwLength ;
	m_block.RemoveAll( ) ;
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0; i < dwLength; i ++ )
	{
		EStackBlock *	pBlock = new EStackBlock ;
		m_block.SetAt( i, pBlock ) ;
		//
		file.Read( &(pBlock->m_dwFlags), sizeof(DWORD) ) ;
		DWORD	dwStrLen ;
		if ( file.Read( &dwStrLen, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	eslErrSuccess ;
		}
		ECSWideString	wstrTemp ;
		file.Read
			( wstrTemp.GetBuffer( dwStrLen ),
								dwStrLen * sizeof(wchar_t) ) ;
		wstrTemp.ReleaseBuffer( dwStrLen ) ;
		//
		pBlock->m_pwstrName = context.GetConstantString( wstrTemp ) ;
		//
		file.Read( &(pBlock->m_iBound), sizeof(int) ) ;
		file.Read( &(pBlock->m_nVarCount), sizeof(int) ) ;
		file.Read( &(pBlock->m_dwCatchAddr), sizeof(DWORD) ) ;
		//
		err = pBlock->LoadArray( file, context ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = "要素数 = " ;
	strDump += EString( (int) m_varArray.GetSize() ) ;
	strDump += "\r\n" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	int		iNextBlock = 0, iBlockNest = 0 ;
	EStackBlock *	pBlock = m_block.GetAt( 0 ) ;
	EStackBlock *	pNextBlock = pBlock ;
	for ( int i = 0; i < (int) m_varArray.GetSize(); i ++ )
	{
		//
		// 名前空間名
		//
		while ( iNextBlock <= i )
		{
			if ( pNextBlock != NULL )
			{
				strDump = "名前空間 : "
					+ EString(pNextBlock->m_pwstrName->CharPtr()) + "\r\n" ;
				buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
			}
			pBlock = pNextBlock ;
			pNextBlock = m_block.GetAt( ++ iBlockNest ) ;
			if ( pNextBlock != NULL )
			{
				iNextBlock = pNextBlock->m_iBound ;
			}
			else
			{
				iNextBlock = m_varArray.GetSize( ) ;
			}
		}
		//
		// 要素名
		//
		strDump = EString("\t") * nIndent + "\t[" + EString(i) + "]" ;
		if ( pBlock != NULL )
		{
			const wchar_t *	pwszName = pBlock->GetAt( i - pBlock->m_iBound ) ;
			if ( pwszName != NULL )
			{
				strDump += " \"" ;
				strDump += EString(pwszName) ;
				strDump += "\"" ;
			}
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

// 現在の関数フレーム情報を更新する
//////////////////////////////////////////////////////////////////////////////
void ECSStack::UpdateCurrentFrame( void )
{
	int		i = 1 ;
	EStackBlock *	pBlock ;
	for ( ; ; )
	{
		pBlock = m_block.GetLastAt( i ) ;
		if ( pBlock == NULL )
		{
			break ;
		}
		if ( pBlock->m_dwFlags & sfCallBlock )
		{
			pBlock = m_block.GetLastAt( i - 1 ) ;
			break ;
		}
		i ++ ;
	}
	if ( pBlock == NULL )
	{
		pBlock = m_block.GetAt( 0 ) ;
		i = m_block.GetSize() ;
	}
	m_nCurrent = i ;
	m_pCurrent = pBlock ;
}

// 名前空間を作成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::CreateNameBlock( ECSWideString * pstrName )
{
	EStackBlock *	pBlock = new EStackBlock ;
	pBlock->m_pwstrName = pstrName ;
	pBlock->m_iBound = m_varArray.GetSize( ) ;
	m_block.Push( pBlock ) ;
	//
	UpdateCurrentFrame() ;
	//
	return	eslErrSuccess ;
}

// 名前空間を削除する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::ReleaseNameBlock
	( ECSWideString *& pwstrName, ECSContext & context )
{
	EStackBlock *	pBlock = m_block.Pop( ) ;
	if ( pBlock != NULL )
	{
		for ( int i = m_varArray.GetSize() - 1;
					i >= (int) pBlock->m_iBound; i -- )
		{
			ECSObject *	pObj = m_varArray.GetAt( i ) ;
			if ( pObj != NULL )
			{
				context.delete_CSObject( pObj ) ;
			}
		}
		int	nReleaseCount = m_varArray.GetSize() - pBlock->m_iBound ;
		if ( nReleaseCount > 0 )
		{
			m_varArray.DetachBetween( pBlock->m_iBound, nReleaseCount ) ;
		}
		ESLAssert( m_varArray.GetSize() == pBlock->m_iBound ) ;
//		m_varArray.SetSize( pBlock->m_iBound ) ;
		pwstrName = pBlock->m_pwstrName ;
		delete	pBlock ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "解放する名前空間が存在しません。" ) ;
}

// 名前空間の一時領域をクリアする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::CleanupNameBlock( ECSContext & context )
{
	EStackBlock *	pBlock = m_block.GetLastAt( 0 ) ;
	if ( pBlock != NULL )
	{
		int	nLimit = pBlock->m_iBound + pBlock->m_nVarCount ;
		for ( int i = m_varArray.GetSize() - 1; i >= nLimit; i -- )
		{
			ECSObject *	pObj = m_varArray.GetAt( i ) ;
			if ( pObj != NULL )
			{
				pObj->OnDestruction( context ) ;
				pObj->CleanupAllReference( context ) ;
			}
		}
		m_varArray.SetSize( nLimit ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "名前空間が存在しません。" ) ;
}

// 変数を追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::CreateNewVariable
	( const wchar_t * pwszName, ECSObject * pObj )
{
	EStackBlock *	pBlock = m_block.GetLastAt( ) ;
	if ( pBlock != NULL )
	{
		while ( pBlock->GetSize() + pBlock->m_iBound < m_varArray.GetSize() )
		{
			pBlock->Add( NULL ) ;
		}
		m_varArray.Push( pObj ) ;
		pBlock->Add( pwszName ) ;
		pBlock->m_nVarCount = m_varArray.GetSize() - pBlock->m_iBound ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "スタック上に名前空間が存在しません。" ) ;
}

// スタックをスワップする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStack::SwapLast( int nIndex1, int nIndex2 )
{
	EStackBlock *	pBlock = m_block.GetLastAt( ) ;
	if ( pBlock != NULL )
	{
		int	nLength = (int) m_varArray.GetSize() ;
		int	nLimit = nLength - (pBlock->m_iBound + pBlock->m_nVarCount) ;
		if ( (nIndex1 < nLimit) && (nIndex2 < nLimit) )
		{
			m_varArray.Swap( nLength - nIndex1 - 1, nLength - nIndex2 - 1 ) ;
			return	eslErrSuccess ;
		}
	}
	return	eslErrFailed ;
}

// 全ての名前空間とスタックを削除する
//////////////////////////////////////////////////////////////////////////////
void ECSStack::RemoveAll( void )
{
	for ( int i = m_varArray.GetSize() - 1; i >= 0; i -- )
	{
		m_varArray.SetAt( i, NULL ) ;
	}
	m_varArray.RemoveAll( ) ;
	m_block.RemoveAll( ) ;
	//
	m_pCurrent = NULL ;
	m_nCurrent = 1 ;
}
