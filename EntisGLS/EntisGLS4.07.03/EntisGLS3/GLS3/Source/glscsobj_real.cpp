
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 実数オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSReal, ECSObject )

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSReal::GetTypeName( void ) const
{
	return	L"Real" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSReal::Duplicate( void )
{
	ECSReal *	pReal = new ECSReal( m_varReal ) ;
	pReal->m_vtRealType = m_vtRealType ;
	return	pReal ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元のオブジェクトが存在しません。" ) ;
	}
	REAL64		nValue ;
	ESLError	err = pEntity->OperateReal( nValue ) ;
	if ( err )
	{
		return	err ;
	}
	m_varReal = nValue ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	switch ( csuopType )
	{
	case	csuotNegate:
		m_varReal = - m_varReal ;
	case	csuotPlus:
		break ;
	default:
		return	ESLErrorMsg( "定義されていない Real 型の単項演算子です。" ) ;
	}
	return	eslErrSuccess ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元のオブジェクトが存在しません。" ) ;
	}
	ESLError	err = Operate( context, csopType, *pEntity ) ;
	if ( err )
	{
		return	err ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

ESLError ECSReal::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject & obj )
{
	ECSObject *	pEntity = &obj ;
	double		rVal ;
	ESLError	err = pEntity->OperateReal( rVal ) ;
	if ( err )
	{
		return	err ;
	}
	switch ( csopType )
	{
	case	csotAdd:
		m_varReal += rVal ;
		break ;
	case	csotSub:
		m_varReal -= rVal ;
		break ;
	case	csotMul:
		m_varReal *= rVal ;
		break ;
	case	csotDiv:
		m_varReal /= rVal ;
		break ;
	default:
		return	ESLErrorMsg( "定義されていない Real 型の演算です。" ) ;
	}
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	double		rVal ;
	ESLError	err = obj.OperateReal( rVal ) ;
	if ( err )
	{
		return	err ;
	}
	if ( cscpType & csctEqual )
	{
		switch ( cscpType )
		{
		case	csctEqual:
			nResult = - (int) (m_varReal == rVal) ;
			break ;
		case	csctLessEqual:
			nResult = - (int) (m_varReal <= rVal) ;
			break ;
		case	csctGreaterEqual:
			nResult = - (int) (m_varReal >= rVal) ;
			break ;
		}
	}
	else
	{
		switch ( cscpType )
		{
		case	csctNotEqual:
			nResult = - (int) (m_varReal != rVal) ;
			break ;
		case	csctLessThan:
			nResult = - (int) (m_varReal < rVal) ;
			break ;
		case	csctGreaterThan:
			nResult = - (int) (m_varReal > rVal) ;
			break ;
		}
	}
	return	eslErrSuccess ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::GetFunction
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
ESLError ECSReal::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg( "不正なメンバ関数を呼び出そうとしています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::OperateSizeOf( INT64 & nSize )
{
	nSize = SizeOf() / 8 ;
	return	eslErrSuccess ;
}

// 特殊演算子 : typeof
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSReal::OperateTypeOf( void ) const
{
	if ( m_vtRealType == csvtReal32 )
	{
		return	L"float" ;
	}
	else if ( m_vtRealType == csvtReal64 )
	{
		return	L"double" ;
	}
	return	L"Real" ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::OperateInteger( INT64 & nValue )
{
	nValue = ::eriRoundR64ToLInt( m_varReal ) ;
	return	eslErrSuccess ;
}

// 実数取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::OperateReal( REAL64 & nValue )
{
	nValue = m_varReal ;
	return	eslErrSuccess ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::OperateString( EWideString & wstrValue )
{
	wstrValue = EWideString( m_varReal ) ;
	return	eslErrSuccess ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSReal::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	if ( iOffset + nSize <= sizeof(m_varReal) )
	{
		return	((BYTE*)&m_varReal) + iOffset ;
	}
	return	NULL ;
}

ECSSakura2Processor::LinearAddressCache *
	ECSReal::GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg )
{
	seg.baseOffset = 0 ;
	seg.limitSegment = sizeof(m_varReal) ;
	seg.pbytBuffer = (BYTE*) &m_varReal ;
	return	&seg ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Save( ESLFileObject & file, ECSContext & context )
{
	if ( file.Write( &m_varReal, sizeof(m_varReal) ) < sizeof(m_varReal) )
	{
		return	ESLErrorMsg( "実数値の書き出しに失敗しました。" ) ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Load( ESLFileObject & file, ECSContext & context )
{
	if ( file.Read( &m_varReal, sizeof(m_varReal) ) < sizeof(m_varReal) )
	{
		return	ESLErrorMsg( "実数値の読み込みに失敗しました。" ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump( m_varReal ) ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSReal::m_staFuncName = NULL ;
const wchar_t *		ECSReal::m_pwszFuncName[14] =
{
	L"Pi", L"Abs", L"Log", L"Power",
	L"Sqrt", L"Sin", L"Cos", L"Tan",
	L"ASin", L"ACos", L"ATan",
	L"Round", L"Floor",
	NULL
} ;
const ECSReal::PFUNC_CALL	ECSReal::m_pfnCallFunc[13] =
{
	&ECSReal::Call_Pi, &ECSReal::Call_Abs,
	&ECSReal::Call_Log, &ECSReal::Call_Power,
	&ECSReal::Call_Sqrt, &ECSReal::Call_Sin,
	&ECSReal::Call_Cos, &ECSReal::Call_Tan,
	&ECSReal::Call_ASin, &ECSReal::Call_ACos,
	&ECSReal::Call_ATan,
	&ECSReal::Call_Round, &ECSReal::Call_Floor,
} ;

// メンバ関数 : Real Pi()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Pi
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( context.new_CSReal( m_varReal * 3.14159265359 ) ) ;
}

// メンバ関数 : Real Abs()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Abs
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( context.new_CSReal( fabs( m_varReal ) ) ) ;
}

// メンバ関数 : Real Log( [Real r] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Log
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	if ( lstArg.GetSize() == 1 )
	{
		return	context.PushObject( new ECSReal( log( m_varReal ) ) ) ;
	}
	double	r ;
	err = context.GetArgumentAsReal( r, lstArg, 1, 10.0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( context.new_CSReal( log( m_varReal ) / log( r ) ) ) ;
}

// メンバ関数 : Real Power( Real r )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Power
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	double	rVal ;
	err = context.GetArgumentAsReal( rVal, lstArg, 1, 1.0 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( context.new_CSReal( pow( m_varReal, rVal ) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Real Sqrt()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Sqrt
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( context.new_CSReal( sqrt( m_varReal ) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Real Sin()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Sin
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( context.new_CSReal( sin( m_varReal ) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Real Cos()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Cos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( context.new_CSReal( cos( m_varReal ) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Real Tan()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Tan
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	context.PushObject( context.new_CSReal( tan( m_varReal ) ) ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Real ASin()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_ASin
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( context.new_CSReal( asin( m_varReal ) ) ) ;
}

// メンバ関数 : Real ACos()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_ACos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( context.new_CSReal( acos( m_varReal ) ) ) ;
}

// メンバ関数 : Real ATan( [Real r] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_ATan
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	if ( lstArg.GetSize() == 1 )
	{
		return	context.PushObject( context.new_CSReal( atan( m_varReal ) ) ) ;
	}
	//
	double	r ;
	err = context.GetArgumentAsReal( r, lstArg, 1, 1.0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( context.new_CSReal( atan2( m_varReal, r ) ) ) ;
}

// メンバ関数 : Integer Round()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Round
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
			( context.new_CSInteger( eriRoundR64ToLInt( m_varReal ) ) ) ;
}

// メンバ関数 : Integer Floor()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSReal::Call_Floor
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
			( context.new_CSInteger
				( eriRoundR64ToLInt( floor( m_varReal ) ) ) ) ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSReal::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_REAL_INTERFACE" ) )
	{
		m_pir.pBackLink = this ;
		m_pir.pfnGetReal = PIC_GetReal ;
		m_pir.pfnSetReal = PIC_SetReal ;
		return	(ECS_REAL_INTERFACE*) &m_pir ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

double __stdcall ECSReal::PIC_GetReal( ECS_REAL_INTERFACE * instance )
{
	PLUGIN_REAL *	ppir = (PLUGIN_REAL*) instance ;
	ESLAssert( &(ppir->pBackLink->m_pir) == ppir ) ;
	return	ppir->pBackLink->m_varReal ;
}

void __stdcall ECSReal::PIC_SetReal
	( ECS_REAL_INTERFACE * instance, double rVal )
{
	PLUGIN_REAL *	ppir = (PLUGIN_REAL*) instance ;
	ESLAssert( &(ppir->pBackLink->m_pir) == ppir ) ;
	ppir->pBackLink->m_varReal = rVal ;
}
