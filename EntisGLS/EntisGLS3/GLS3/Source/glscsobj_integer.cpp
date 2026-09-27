
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 整数オブジェクト
//////////////////////////////////////////////////////////////////////////////

const INT64	ECSInteger::m_maskBoolean = 0x8000000000000000 ;
const INT64	ECSInteger::m_maskInt8 = 0x800000000000007F ;
const INT64	ECSInteger::m_maskInt16 = 0x8000000000007FFF ;
const INT64	ECSInteger::m_maskInt32 = 0x800000007FFFFFFF ;
const INT64	ECSInteger::m_maskUint8 = 0xFF ;
const INT64	ECSInteger::m_maskUint16 = 0xFFFF ;
const INT64	ECSInteger::m_maskUint32 = 0xFFFFFFFF ;
const INT64	ECSInteger::m_maskInt64 = -1 ;
const INT64	ECSInteger::m_maskUint64 = 0x7FFFFFFFFFFFFFFF ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSInteger, ECSObject )

// 整数型取得
//////////////////////////////////////////////////////////////////////////////
CSVariableType ECSInteger::GetIntegerType( void ) const
{
	if ( m_varMask & 0x7FFFFFFF00000000 )
	{
		return	csvtInteger ;
	}
	else if ( m_varMask & 0xFFFF0000 )
	{
		if ( IsSign() )
		{
			return	csvtInt32 ;
		}
		else
		{
			return	csvtUint32 ;
		}
	}
	else if ( m_varMask & 0xFF00 )
	{
		if ( IsSign() )
		{
			return	csvtInt16 ;
		}
		else
		{
			return	csvtUint16 ;
		}
	}
	else if ( m_varMask & 0xFF )
	{
		if ( IsSign() )
		{
			return	csvtInt8 ;
		}
		else
		{
			return	csvtUint8 ;
		}
	}
	return	csvtBoolean ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSInteger::GetTypeName( void ) const
{
	return	L"Integer" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSInteger::Duplicate( void )
{
	return	new ECSInteger( m_varInt, m_varMask ) ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	INT64		nValue ;
	ESLError	err = pEntity->OperateInteger( nValue ) ;
	if ( err )
	{
		return	err ;
	}
	SetValue( nValue ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	switch ( csuopType )
	{
	case	csuotPlus:
		break ;
	case	csuotNegate:
		SetValue( - m_varInt ) ;
		break ;
	case	csuotBitNot:
		SetValue( ~ m_varInt ) ;
		break ;
	case	csuotLogicalNot:
		SetValue( - (INT64) ! m_varInt ) ;
		m_varMask =  ECSInteger::m_maskBoolean ;
		break ;
	case	csuotIncrement:
		SetValue( m_varInt + 1 ) ;
		break ;
	case	csuotDecrement:
		SetValue( m_varInt - 1 ) ;
		break ;
	case	csuotIncrementAfter:
		m_pResult = context.new_CSInteger( m_varInt ) ;
		SetValue( m_varInt + 1 ) ;
		break ;
	case	csuotDecrementAfter:
		m_pResult = context.new_CSInteger( m_varInt ) ;
		SetValue( m_varInt - 1 ) ;
		break ;
	default:
		return	ESLErrorMsg( "Integer 型に未定義の単項演算子です。" ) ;
	}
	return	eslErrSuccess ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元オブジェクトが存在しません。" ) ;
	}
	ESLError	err = Operate( context, csopType, *pEntity ) ;
	if ( err )
	{
		return	err ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

ESLError ECSInteger::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject & obj )
{
	ECSObject *	pEntity = &obj ;
	INT64	nVal ;
	if ( pEntity->IsRealNumberObject() )
	{
		ECSReal *	pResult = context.new_CSReal( (double) m_varInt ) ;
		m_pResult = pResult ;
		return	pResult->Operate( context, csopType, obj ) ;
	}
	ESLError	err = pEntity->OperateInteger( nVal ) ;
	if ( err )
	{
		return	err ;
	}
	switch ( csopType )
	{
	case	csotAdd:
		SetValue( m_varInt + nVal ) ;
		break ;
	case	csotSub:
		SetValue( m_varInt - nVal ) ;
		break ;
	case	csotMul:
		SetValue( m_varInt * nVal ) ;
		break ;
	case	csotDiv:
		if ( nVal == 0 )
		{
			return	ESLErrorMsg( "零除算エラー。\n" ) ;
		}
		SetValue( m_varInt / nVal ) ;
		break ;
	case	csotMod:
		if ( nVal == 0 )
		{
			return	ESLErrorMsg( "零除算エラー。\n" ) ;
		}
		SetValue( m_varInt % nVal ) ;
		break ;
	case	csotAnd:
		SetValue( m_varInt & nVal ) ;
		break ;
	case	csotOr:
		SetValue( m_varInt | nVal ) ;
		break ;
	case	csotXor:
		SetValue( m_varInt ^ nVal ) ;
		break ;
	case	csotLogicalAnd:
		if ( pEntity->m_vtType == csvtString )
		{
			SetValue
				( - (int) (m_varInt
					&& ((ECSString*)pEntity)->m_varStr.GetLength()) ) ;
			m_varMask =  ECSInteger::m_maskBoolean ;
		}
		else
		{
			SetValue( - (int) (m_varInt && nVal) ) ;
			m_varMask =  ECSInteger::m_maskBoolean ;
		}
		break ;
	case	csoutLogicalOr:
		if ( pEntity->m_vtType == csvtString )
		{
			SetValue
				( - (int) (m_varInt
					|| ((ECSString*)pEntity)->m_varStr.GetLength()) ) ;
			m_varMask =  ECSInteger::m_maskBoolean ;
		}
		else
		{
			SetValue( - (int) (m_varInt || nVal) ) ;
			m_varMask =  ECSInteger::m_maskBoolean ;
		}
		break ;
	case	csotShiftRight:
		SetValue( m_varInt >> nVal ) ;
		break ;
	case	csotShiftLeft:
		SetValue( m_varInt << nVal ) ;
		break ;
	default:
		return	ESLErrorMsg( "未定義の二項演算子です。" ) ;
	}
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	if ( obj.IsRealNumberObject() )
	{
		double		rVal ;
		ESLError	err = obj.OperateReal( rVal ) ;
		switch ( cscpType )
		{
		case	csctEqual:
			nResult = - (int) (m_varInt == rVal) ;
			break ;
		case	csctLessEqual:
			nResult = - (int) (m_varInt <= rVal) ;
			break ;
		case	csctGreaterEqual:
			nResult = - (int) (m_varInt >= rVal) ;
			break ;
		case	csctNotEqual:
			nResult = - (int) (m_varInt != rVal) ;
			break ;
		case	csctLessThan:
			nResult = - (int) (m_varInt < rVal) ;
			break ;
		case	csctGreaterThan:
			nResult = - (int) (m_varInt > rVal) ;
			break ;
		}
		return	eslErrSuccess ;
	}
	INT64		nVal ;
	ESLError	err = obj.OperateInteger( nVal ) ;
	if ( err )
	{
		return	err ;
	}
	switch ( cscpType )
	{
	case	csctEqual:
		nResult = - (int) (m_varInt == nVal) ;
		break ;
	case	csctLessEqual:
		nResult = - (int) (m_varInt <= nVal) ;
		break ;
	case	csctGreaterEqual:
		nResult = - (int) (m_varInt >= nVal) ;
		break ;
	case	csctNotEqual:
		nResult = - (int) (m_varInt != nVal) ;
		break ;
	case	csctLessThan:
		nResult = - (int) (m_varInt < nVal) ;
		break ;
	case	csctGreaterThan:
		nResult = - (int) (m_varInt > nVal) ;
		break ;
	}
	return	eslErrSuccess ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::GetFunction
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
ESLError ECSInteger::CallFunction
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
ESLError ECSInteger::OperateBoolean( int & nBoolean )
{
	nBoolean = - (int) (m_varInt != 0) ;
	return	eslErrSuccess ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::OperateSizeOf( INT64 & nSize )
{
	nSize = SizeOf() / 8 ;
	return	eslErrSuccess ;
}

// 特殊演算子 : typeof
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSInteger::OperateTypeOf( void ) const
{
	switch ( m_varMask )
	{
	case	-1:
	default:
		return	L"Integer" ;
	case	0x7FFFFFFFFFFFFFFF:
		return	L"uint64" ;
	case	0x800000000000007F:
		return	L"int8" ;
	case	0x8000000000007FFF:
		return	L"int16" ;
	case	0x800000007FFFFFFF:
		return	L"int32" ;
	case	0xFF:
		return	L"uint8" ;
	case	0xFFFF:
		return	L"uint16" ;
	case	0xFFFFFFFF:
		return	L"uint32" ;
	case	0x8000000000000000:
		return	L"boolean" ;
	}
	return	NULL ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::OperateInteger( INT64 & nValue )
{
	nValue = m_varInt ;
	return	eslErrSuccess ;
}

// 実数取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::OperateReal( REAL64 & nValue )
{
	nValue = (REAL64) m_varInt ;
	return	eslErrSuccess ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::OperateString( EWideString & wstrValue )
{
	wstrValue.FromInteger( m_varInt ) ;
	return	eslErrSuccess ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSInteger::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	if ( iOffset + nSize <= sizeof(m_varInt) )
	{
		return	((BYTE*)&m_varInt) + iOffset ;
	}
	return	NULL ;
}

ECSSakura2Processor::LinearAddressCache *
	ECSInteger::GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg )
{
	seg.baseOffset = 0 ;
	seg.limitSegment = sizeof(INT64) ;
	seg.pbytBuffer = (BYTE*) &m_varInt ;
	return	&seg ;
}


// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Save( ESLFileObject & file, ECSContext & context )
{
	if ( file.Write( &m_varInt, sizeof(m_varInt) ) < sizeof(m_varInt) )
	{
		return	ESLErrorMsg( "整数値の保存に失敗しました。" ) ;
	}
	if ( file.Write( &m_varMask, sizeof(m_varMask) ) < sizeof(m_varMask) )
	{
		return	ESLErrorMsg( "整数値の保存に失敗しました。" ) ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Load( ESLFileObject & file, ECSContext & context )
{
	if ( file.Read( &m_varInt, sizeof(m_varInt) ) < sizeof(m_varInt) )
	{
		return	ESLErrorMsg( "整数値の読み込みに失敗しました。" ) ;
	}
	if ( file.Read( &m_varMask, sizeof(m_varMask) ) < sizeof(m_varMask) )
	{
		return	ESLErrorMsg( "整数値の読み込みに失敗しました。" ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	if ( (-0x10000 <= m_varInt) && (m_varInt <= 0x10000) )
	{
		strDump.FromInteger( m_varInt ) ;
	}
	else
	{
		EString	strHex ;
		strHex.HexFromInteger( m_varInt ) ;
		strDump = "0" + strHex + "H" ;
	}
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSInteger::m_staFuncName = NULL ;
const wchar_t *		ECSInteger::m_pwszFuncName[11] =
{
	L"Char", L"Format", L"Abs",
	L"TestBit", L"SetBit", L"ResetBit",
	L"RotateLeft", L"RotateRight",
	L"ShiftLeft", L"ShiftRight",
	NULL
} ;
const ECSInteger::PFUNC_CALL	ECSInteger::m_pfnCallFunc[10] =
{
	&ECSInteger::Call_Char, &ECSInteger::Call_Format,
	&ECSInteger::Call_Abs, &ECSInteger::Call_TestBit,
	&ECSInteger::Call_SetBit, &ECSInteger::Call_ResetBit,
	&ECSInteger::Call_RotateLeft, &ECSInteger::Call_RotateRight,
	&ECSInteger::Call_ShiftLeft, &ECSInteger::Call_ShiftRight
} ;

// メンバ関数 : String Char()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_Char
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSString *	pStr = new ECSString ;
	pStr->m_varStr.GetBuffer(1)[0] = (wchar_t) m_varInt ;
	pStr->m_varStr.ReleaseBuffer(1) ;
	context.PushObject( pStr ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : String Format( Integer nRadix := 0, Integer nCol := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_Format
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	//
	// 引数取得
	//
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nRadix, nCol ;
	err = context.GetArgumentAsInt( nRadix, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nCol, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	// パラメータ正規化
	//
	INT64		fFlag = 0 ;
	UINT64		nVal = m_varInt ;
	if ( nRadix <= 1 )
	{
		if ( (INT64) nVal < 0 )
		{
			fFlag = -1 ;
			nVal = - (INT64) nVal ;
		}
		nRadix = - nRadix ;
		if ( nRadix <= 1 )
		{
			nRadix = 10 ;
		}
	}
	if ( (nCol <= 0) || (nCol > 256) )
	{
		nCol = (int) (1 - fFlag) ;
		while ( nVal >= (unsigned int) nRadix )
		{
			nVal /= nRadix ;
			nCol ++ ;
		}
		nVal = (m_varInt ^ fFlag) - fFlag ;
	}
	//
	// 文字列へ変換
	//
	wchar_t *	pwszBuf ;
	ECSString *	pStr = new ECSString ;
	pwszBuf = pStr->m_varStr.GetBuffer( nCol ) ;
	for ( int i = 0; i < nCol; i ++ )
	{
		wchar_t	wch ;
		int		n = (int) (nVal % nRadix) ;
		nVal = (nVal - n) / nRadix ;
		//
		if ( n < 10 )
			wch = (wchar_t) (L'0' + n) ;
		else if ( n < 16 )
			wch = (wchar_t) (L'A' + n - 10) ;
		else
			wch = L'?' ;
		//
		pwszBuf[nCol - i - 1] = wch ;
	}
	if ( fFlag )
	{
		pwszBuf[0] = L'-' ;
	}
	pStr->m_varStr.ReleaseBuffer( nCol ) ;
	//
	return	context.PushObject( pStr ) ;
}

// メンバ関数 : Integer Abs()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_Abs
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSInteger *	pInt = new ECSInteger ;
	if ( m_varInt < 0 )
	{
		pInt->m_varInt = - m_varInt ;
	}
	else
	{
		pInt->m_varInt = m_varInt ;
	}
	return	context.PushObject( pInt ) ;
}

// メンバ関数 : Integer TestBit( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_TestBit
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	iBit ;
	err = context.GetArgumentAsInt( iBit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	if ( (iBit < 0) || (iBit >= 64) )
	{
		return	context.PushObject( new ECSInteger ) ;
	}
	context.PushObject( new ECSInteger( - ((m_varInt >> iBit) & 0x01) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Inetegr SetBit( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_SetBit
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	iBit ;
	err = context.GetArgumentAsInt( iBit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetValue( m_varInt | ((INT64) 1 << iBit) ) ;
	context.PushObject( new ECSInteger( m_varInt ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer ResetBit( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_ResetBit
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	iBit ;
	err = context.GetArgumentAsInt( iBit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetValue( m_varInt & ~((INT64) 1 << iBit) ) ;
	context.PushObject( new ECSInteger( m_varInt ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer RotateLeft( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_RotateLeft
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	iBit ;
	err = context.GetArgumentAsInt( iBit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	int	nBitSize = SizeOf() ;
	iBit %= nBitSize ;
	SetValue( (m_varInt << iBit) | (m_varInt >> (nBitSize - iBit)) ) ;
	context.PushObject( new ECSInteger( m_varInt ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer RotateRight( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_RotateRight
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	iBit ;
	err = context.GetArgumentAsInt( iBit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	int	nBitSize = SizeOf() ;
	iBit %= nBitSize ;
	SetValue( (m_varInt >> iBit) | (m_varInt << (nBitSize - iBit)) ) ;
	context.PushObject( new ECSInteger( m_varInt ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer ShiftLeft( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_ShiftLeft
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	iBit ;
	err = context.GetArgumentAsInt( iBit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetValue( m_varInt << iBit ) ;
	context.PushObject( new ECSInteger( m_varInt ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Integer ShiftRight( Integer i )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSInteger::Call_ShiftRight
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	iBit ;
	err = context.GetArgumentAsInt( iBit, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetValue( m_varInt >> iBit ) ;
	context.PushObject( new ECSInteger( m_varInt ) ) ;
	return	eslErrSuccess ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSInteger::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_INTEGER_INTERFACE" ) )
	{
		m_pii.pBackLink = this ;
		m_pii.pfnGetInteger = PIC_GetInteger ;
		m_pii.pfnSetInteger = PIC_SetInteger ;
		m_pii.pfnGetInteger64 = PIC_GetInteger64 ;
		m_pii.pfnSetInteger64 = PIC_SetInteger64 ;
		return	(ECS_INTEGER_INTERFACE*) &m_pii ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

long int __stdcall ECSInteger::PIC_GetInteger
	( ECS_INTEGER_INTERFACE * instance )
{
	PLUGIN_INTEGER *	ppii = (PLUGIN_INTEGER*) instance ;
	ESLAssert( &(ppii->pBackLink->m_pii) == ppii ) ;
	return	ppii->pBackLink->GetInt() ;
}

void __stdcall ECSInteger::PIC_SetInteger
	( ECS_INTEGER_INTERFACE * instance, long int nVal )
{
	PLUGIN_INTEGER *	ppii = (PLUGIN_INTEGER*) instance ;
	ESLAssert( &(ppii->pBackLink->m_pii) == ppii ) ;
	ppii->pBackLink->SetValue( nVal ) ;
}

INT64 __stdcall ECSInteger::PIC_GetInteger64
	( ECS_INTEGER_INTERFACE * instance )
{
	PLUGIN_INTEGER *	ppii = (PLUGIN_INTEGER*) instance ;
	ESLAssert( &(ppii->pBackLink->m_pii) == ppii ) ;
	return	ppii->pBackLink->GetValue() ;
}

void __stdcall ECSInteger::PIC_SetInteger64
	( ECS_INTEGER_INTERFACE * instance, INT64 nVal )
{
	PLUGIN_INTEGER *	ppii = (PLUGIN_INTEGER*) instance ;
	ESLAssert( &(ppii->pBackLink->m_pii) == ppii ) ;
	ppii->pBackLink->SetValue( nVal ) ;
}
