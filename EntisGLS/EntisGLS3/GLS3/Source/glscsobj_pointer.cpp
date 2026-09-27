
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// ポインタ参照オブジェクト
//////////////////////////////////////////////////////////////////////////////

#define	STORE_ROUND_INT	0

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSPointerReference, ECSReference )

// ロード関数
//////////////////////////////////////////////////////////////////////////////
static INT64 LoadBufferIntError( const void * ptrBuf )
{
	return	0 ;
}

static INT64 LoadBufferBoolean( const void * ptrBuf )
{
	return	- (((int) *((const unsigned __int8*) ptrBuf) + 0xFF) >> 8) ;
}

static INT64 LoadBufferInt8( const void * ptrBuf )
{
	return	*((const __int8*) ptrBuf) ;
}

static INT64 LoadBufferUInt8( const void * ptrBuf )
{
	return	*((const unsigned __int8*) ptrBuf) ;
}

static INT64 LoadBufferInt16( const void * ptrBuf )
{
	return	*((const __int16*) ptrBuf) ;
}

static INT64 LoadBufferUInt16( const void * ptrBuf )
{
	return	*((const unsigned __int16*) ptrBuf) ;
}

static INT64 LoadBufferInt32( const void * ptrBuf )
{
	return	*((const __int32*) ptrBuf) ;
}

static INT64 LoadBufferUInt32( const void * ptrBuf )
{
	return	*((const unsigned __int32*) ptrBuf) ;
}

static INT64 LoadBufferInt64( const void * ptrBuf )
{
	return	*((const __int64*) ptrBuf) ;
}

static INT64 LoadBufferIntFromFloat( const void * ptrBuf )
{
	return	(INT64) *((const REAL32*) ptrBuf) ;
}

static INT64 LoadBufferIntFromDouble( const void * ptrBuf )
{
	return	(INT64) *((const double*) ptrBuf) ;
}

static double LoadBufferRealError( const void * ptrBuf )
{
	return	0.0 ;
}

static double LoadBufferRealFromInt8( const void * ptrBuf )
{
	return	*((const __int8*) ptrBuf) ;
}

static double LoadBufferRealFromUInt8( const void * ptrBuf )
{
	return	*((const unsigned __int8*) ptrBuf) ;
}

static double LoadBufferRealFromInt16( const void * ptrBuf )
{
	return	*((const __int16*) ptrBuf) ;
}

static double LoadBufferRealFromUInt16( const void * ptrBuf )
{
	return	*((const unsigned __int16*) ptrBuf) ;
}

static double LoadBufferRealFromInt32( const void * ptrBuf )
{
	return	*((const __int32*) ptrBuf) ;
}

static double LoadBufferRealFromUInt32( const void * ptrBuf )
{
	return	*((const unsigned __int32*) ptrBuf) ;
}

static double LoadBufferRealFromInt64( const void * ptrBuf )
{
	return	(double) *((const __int64*) ptrBuf) ;
}

static double LoadBufferFloat( const void * ptrBuf )
{
	return	*((const float*) ptrBuf) ;
}

static double LoadBufferDouble( const void * ptrBuf )
{
	return	*((const double*) ptrBuf) ;
}

// ストア関数
//////////////////////////////////////////////////////////////////////////////
static ESLError StoreBufferIntError( void * ptrBuf, INT64 nValue )
{
	return	eslErrFailed ;
}

static ESLError StoreBufferBoolean( void * ptrBuf, INT64 nValue )
{
	*((__int8*)ptrBuf) = (__int8) - (int) (nValue != 0) ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferInt8( void * ptrBuf, INT64 nValue )
{
#if	STORE_ROUND_INT
	nValue &= ECSInteger::m_maskInt8 ;
	*((__int8*)ptrBuf) =
		(__int8) (nValue | ((nValue >> 63) & ~ECSInteger::m_maskInt8)) ;
#else
	*((__int8*)ptrBuf) = (__int8) nValue ;
#endif
	return	eslErrSuccess ;
}

static ESLError StoreBufferUInt8( void * ptrBuf, INT64 nValue )
{
#if	STORE_ROUND_INT
	nValue &= ECSInteger::m_maskUint8 ;
	*((unsigned __int8*)ptrBuf) =
		(unsigned __int8) (nValue | ((nValue >> 63) & ~ECSInteger::m_maskUint8)) ;
#else
	*((unsigned __int8*)ptrBuf) = (unsigned __int8) nValue ;
#endif
	return	eslErrSuccess ;
}

static ESLError StoreBufferInt16( void * ptrBuf, INT64 nValue )
{
#if	STORE_ROUND_INT
	nValue &= ECSInteger::m_maskInt16 ;
	*((__int16*)ptrBuf) =
		(__int16) (nValue | ((nValue >> 63) & ~ECSInteger::m_maskInt16)) ;
#else
	*((__int16*)ptrBuf) = (__int16) nValue ;
#endif
	return	eslErrSuccess ;
}

static ESLError StoreBufferUInt16( void * ptrBuf, INT64 nValue )
{
#if	STORE_ROUND_INT
	nValue &= ECSInteger::m_maskUint16 ;
	*((unsigned __int16*)ptrBuf) =
		(unsigned __int16) (nValue | ((nValue >> 63) & ~ECSInteger::m_maskUint16)) ;
#else
	*((unsigned __int16*)ptrBuf) = (unsigned __int16) nValue ;
#endif
	return	eslErrSuccess ;
}

static ESLError StoreBufferInt32( void * ptrBuf, INT64 nValue )
{
#if	STORE_ROUND_INT
	nValue &= ECSInteger::m_maskInt32 ;
	*((__int32*)ptrBuf) =
		(__int32) (nValue | ((nValue >> 63) & ~ECSInteger::m_maskInt32)) ;
#else
	*((__int32*)ptrBuf) = (__int32) nValue ;
#endif
	return	eslErrSuccess ;
}

static ESLError StoreBufferUInt32( void * ptrBuf, INT64 nValue )
{
#if	STORE_ROUND_INT
	nValue &= ECSInteger::m_maskUint32 ;
	*((unsigned __int32*)ptrBuf) =
		(unsigned __int32) (nValue | ((nValue >> 63) & ~ECSInteger::m_maskUint32)) ;
#else
	*((unsigned __int32*)ptrBuf) = (unsigned __int32) nValue ;
#endif
	return	eslErrSuccess ;
}

static ESLError StoreBufferInt64( void * ptrBuf, INT64 nValue )
{
	*((__int64*)ptrBuf) = nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferIntToFloat( void * ptrBuf, INT64 nValue )
{
	*((REAL32*)ptrBuf) = (REAL32) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferIntToDouble( void * ptrBuf, INT64 nValue )
{
	*((REAL64*)ptrBuf) = (REAL64) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferRealError( void * ptrBuf, double nValue )
{
	return	eslErrFailed ;
}

static ESLError StoreBufferRealToInt8( void * ptrBuf, double nValue )
{
	*((__int8*)ptrBuf) = (__int8) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferRealToUInt8( void * ptrBuf, double nValue )
{
	*((unsigned __int8*)ptrBuf) = (unsigned __int8) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferRealToInt16( void * ptrBuf, double nValue )
{
	*((__int16*)ptrBuf) = (__int16) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferRealToUInt16( void * ptrBuf, double nValue )
{
	*((unsigned __int16*)ptrBuf) = (unsigned __int16) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferRealToInt32( void * ptrBuf, double nValue )
{
	*((__int32*)ptrBuf) = (__int32) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferRealToUInt32( void * ptrBuf, double nValue )
{
	*((unsigned __int32*)ptrBuf) = (unsigned __int32) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferRealToInt64( void * ptrBuf, double nValue )
{
	*((__int64*)ptrBuf) = (__int64) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferFloat( void * ptrBuf, double nValue )
{
	*((REAL32*)ptrBuf) = (REAL32) nValue ;
	return	eslErrSuccess ;
}

static ESLError StoreBufferDouble( void * ptrBuf, double nValue )
{
	*((REAL64*)ptrBuf) = nValue ;
	return	eslErrSuccess ;
}

const int	ECSPointerReference::m_nTypeBytes[csvtExTypeMax] =
{
	0, 0, 0, 0,
	8, 8, 0, 8,
	0, 0, 1, 1,
	1, 2, 2, 4,
	4, 0, 0, 4,
	8, 0
} ;

const CSVariableType	ECSPointerReference::m_csvtTypeIntFloat[csvtExTypeMax] =
{
	csvtObject, csvtObject, csvtObject, csvtObject,
	csvtInteger, csvtReal, csvtObject, csvtInteger,
	csvtObject, csvtObject, csvtInteger, csvtInteger,
	csvtInteger, csvtInteger, csvtInteger, csvtInteger,
	csvtInteger, csvtObject, csvtObject, csvtReal,
	csvtReal, csvtObject,
} ;

const ECSPointerReference::PFUNC_LOAD_INT
	ECSPointerReference::m_pfnLoadIntFunc[csvtExTypeMax] =
{
	&LoadBufferIntError,
	&LoadBufferIntError,
	&LoadBufferIntError,
	&LoadBufferIntError,
	&LoadBufferInt64,
	&LoadBufferIntFromDouble,
	&LoadBufferIntError,
	&LoadBufferInt64,
	&LoadBufferIntError,
	&LoadBufferIntError,
	&LoadBufferBoolean,
	&LoadBufferInt8,
	&LoadBufferUInt8,
	&LoadBufferInt16,
	&LoadBufferUInt16,
	&LoadBufferInt32,
	&LoadBufferUInt32,
	&LoadBufferIntError,
	&LoadBufferIntError,
	&LoadBufferIntFromFloat,
	&LoadBufferIntFromDouble,
	&LoadBufferIntError,
} ;

const ECSPointerReference::PFUNC_LOAD_REAL
	ECSPointerReference::m_pfnLoadRealFunc[csvtExTypeMax] =
{
	&LoadBufferRealError,
	&LoadBufferRealError,
	&LoadBufferRealError,
	&LoadBufferRealError,
	&LoadBufferRealFromInt64,
	&LoadBufferDouble,
	&LoadBufferRealError,
	&LoadBufferRealFromInt64,
	&LoadBufferRealError,
	&LoadBufferRealError,
	&LoadBufferRealError,
	&LoadBufferRealFromInt8,
	&LoadBufferRealFromUInt8,
	&LoadBufferRealFromInt16,
	&LoadBufferRealFromUInt16,
	&LoadBufferRealFromInt32,
	&LoadBufferRealFromUInt32,
	&LoadBufferRealError,
	&LoadBufferRealError,
	&LoadBufferFloat,
	&LoadBufferDouble,
	&LoadBufferRealError,
} ;

const ECSPointerReference::PFUNC_STORE_INT
	ECSPointerReference::m_pfnStoreIntFunc[csvtExTypeMax] =
{
	&StoreBufferIntError,
	&StoreBufferIntError,
	&StoreBufferIntError,
	&StoreBufferIntError,
	&StoreBufferInt64,
	&StoreBufferIntToDouble,
	&StoreBufferIntError,
	&StoreBufferInt64,
	&StoreBufferIntError,
	&StoreBufferIntError,
	&StoreBufferBoolean,
	&StoreBufferInt8,
	&StoreBufferUInt8,
	&StoreBufferInt16,
	&StoreBufferUInt16,
	&StoreBufferInt32,
	&StoreBufferUInt32,
	&StoreBufferIntError,
	&StoreBufferIntError,
	&StoreBufferIntToFloat,
	&StoreBufferIntToDouble,
	&StoreBufferIntError,
} ;

const ECSPointerReference::PFUNC_SOTRE_REAL
	ECSPointerReference::m_pfnStoreRealFunc[csvtExTypeMax] =
{
	&StoreBufferRealError,
	&StoreBufferRealError,
	&StoreBufferRealError,
	&StoreBufferRealError,
	&StoreBufferRealToInt64,
	&StoreBufferDouble,
	&StoreBufferRealError,
	&StoreBufferRealToInt64,
	&StoreBufferRealError,
	&StoreBufferRealError,
	&StoreBufferRealError,
	&StoreBufferRealToInt8,
	&StoreBufferRealToUInt8,
	&StoreBufferRealToInt16,
	&StoreBufferRealToUInt16,
	&StoreBufferRealToInt32,
	&StoreBufferRealToUInt32,
	&StoreBufferRealError,
	&StoreBufferRealError,
	&StoreBufferFloat,
	&StoreBufferDouble,
	&StoreBufferRealError,
} ;

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSPointerReference::GetTypeName( void ) const
{
	return	L"PointerReference" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPointerReference::Duplicate( void )
{
	CSVariableType	csvtMemType = GetMemoryObjectType() ;
	if ( csvtMemType == csvtInteger )
	{
		INT64	nValue ;
		if ( !LoadBufferInteger( nValue ) )
		{
			return	new ECSInteger( nValue ) ;
		}
	}
	else if ( csvtMemType == csvtReal )
	{
		double	nValue ;
		if ( !LoadBufferReal( nValue ) )
		{
			return	new ECSReal( nValue ) ;
		}
	}
	return	NULL ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::Move( ECSContext & context, ECSObject * obj )
{
	CSVariableType	csvtMemType = GetMemoryObjectType() ;
	ESLError		err ;
	if ( csvtMemType == csvtInteger )
	{
		INT64	nValue ;
		err = obj->OperateInteger( nValue ) ;
		if ( err )
		{
			return	err ;
		}
		if ( StoreBufferInteger( nValue ) )
		{
			return	ESLErrorMsg( "ストア先の整数ポインタが無効です" ) ;
		}
	}
	else if ( csvtMemType == csvtReal )
	{
		double	nValue ;
		err = obj->OperateReal( nValue ) ;
		if ( err )
		{
			return	err ;
		}
		if ( StoreBufferReal( nValue ) )
		{
			return	ESLErrorMsg( "ストア先の実数ポインタが無効です" ) ;
		}
	}
	else
	{
		return	ESLErrorMsg( "無効なポインタ型です" ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	CSVariableType	csvtMemType = GetMemoryObjectType() ;
	if ( csvtMemType == csvtInteger )
	{
		INT64	nValue ;
		if ( LoadBufferInteger( nValue ) )
		{
			return	ESLErrorMsg( "ロード先の整数ポインタが無効です" ) ;
		}
		switch ( csuopType )
		{
		case	csuotPlus:
			break ;
		case	csuotNegate:
			nValue = - nValue ;
			break ;
		case	csuotBitNot:
			nValue = ~ nValue ;
			break ;
		case	csuotLogicalNot:
			nValue = - (int) !nValue ;
			break ;
		case	csuotIncrement:
			nValue ++ ;
			break ;
		case	csuotDecrement:
			nValue -- ;
			break ;
		case	csuotIncrementAfter:
			m_pResult = context.new_CSInteger( nValue ) ;
			nValue ++ ;
			break ;
		case	csuotDecrementAfter:
			m_pResult = context.new_CSInteger( nValue ) ;
			nValue -- ;
			break ;
		default:
			return	ESLErrorMsg( "整数型に未定義の単項演算子です。" ) ;
		}
		if ( StoreBufferInteger( nValue ) )
		{
			return	ESLErrorMsg( "ストア先の整数ポインタが無効です" ) ;
		}
	}
	else if ( csvtMemType == csvtReal )
	{
		double	nValue ;
		if ( LoadBufferReal( nValue ) )
		{
			return	ESLErrorMsg( "ロード先の実数ポインタが無効です" ) ;
		}
		switch ( csuopType )
		{
		case	csuotPlus:
			break ;
		case	csuotNegate:
			nValue = - nValue ;
			break ;
		default:
			return	ESLErrorMsg( "整数型に未定義の単項演算子です。" ) ;
		}
		if ( StoreBufferReal( nValue ) )
		{
			return	ESLErrorMsg( "ストア先の実数ポインタが無効です" ) ;
		}
	}
	else
	{
		return	ESLErrorMsg( "無効なポインタ型です" ) ;
	}
	return	eslErrSuccess ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	CSVariableType	csvtMemType = GetMemoryObjectType() ;
	if ( csvtMemType == csvtInteger )
	{
		ESLError	err ;
		INT64		nSrcValue ;
		err = obj->OperateInteger( nSrcValue ) ;
		if ( err )
		{
			return	err ;
		}
		INT64	nDstValue ;
		if ( LoadBufferInteger( nDstValue ) )
		{
			return	ESLErrorMsg( "ロード先の整数ポインタが無効です" ) ;
		}
		switch ( csopType )
		{
		case	csotAdd:
			nDstValue += nSrcValue ;
			break ;
		case	csotSub:
			nDstValue -= nSrcValue ;
			break ;
		case	csotMul:
			nDstValue *= nSrcValue ;
			break ;
		case	csotDiv:
			if ( nSrcValue == 0 )
			{
				return	ESLErrorMsg( "零除算エラー" ) ;
			}
			nDstValue /= nSrcValue ;
			break ;
		case	csotMod:
			if ( nSrcValue == 0 )
			{
				return	ESLErrorMsg( "零除算エラー" ) ;
			}
			nDstValue %= nSrcValue ;
			break ;
		case	csotAnd:
			nDstValue &= nSrcValue ;
			break ;
		case	csotOr:
			nDstValue |= nSrcValue ;
			break ;
		case	csotXor:
			nDstValue ^= nSrcValue ;
			break ;
		case	csotLogicalAnd:
			nDstValue = - (int) (nDstValue && nSrcValue) ;
			break ;
		case	csoutLogicalOr:
			nDstValue = - (int) (nDstValue || nSrcValue) ;
			break ;
		case	csotShiftRight:
			nDstValue >>= nSrcValue ;
			break ;
		case	csotShiftLeft:
			nDstValue <<= nSrcValue ;
			break ;
		default:
			return	ESLErrorMsg( "整数型に未定義の演算子です。" ) ;
		}
		if ( StoreBufferInteger( nDstValue ) )
		{
			return	ESLErrorMsg( "ストア先の整数ポインタが無効です" ) ;
		}
	}
	else if ( csvtMemType == csvtReal )
	{
		ESLError	err ;
		double		nSrcValue ;
		err = obj->OperateReal( nSrcValue ) ;
		if ( err )
		{
			return	err ;
		}
		double	nDstValue ;
		if ( LoadBufferReal( nDstValue ) )
		{
			return	ESLErrorMsg( "ロード先の整数ポインタが無効です" ) ;
		}
		switch ( csopType )
		{
		case	csotAdd:
			nDstValue += nSrcValue ;
			break ;
		case	csotSub:
			nDstValue -= nSrcValue ;
			break ;
		case	csotMul:
			nDstValue *= nSrcValue ;
			break ;
		case	csotDiv:
			if ( nSrcValue == 0 )
			{
				return	ESLErrorMsg( "実数零除算エラー" ) ;
			}
			nDstValue /= nSrcValue ;
			break ;
		default:
			return	ESLErrorMsg( "実数型に未定義の演算子です。" ) ;
		}
		if ( StoreBufferReal( nDstValue ) )
		{
			return	ESLErrorMsg( "ストア先の実数ポインタが無効です" ) ;
		}
	}
	else
	{
		return	ESLErrorMsg( "無効なポインタ型です" ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	CSVariableType	csvtMemType = GetMemoryObjectType() ;
	if ( (csvtMemType == csvtInteger) && !obj.IsRealNumberObject() )
	{
		INT64	nValue0, nValue1 ;
		if ( LoadBufferInteger( nValue0 ) )
		{
			return	ESLErrorMsg( "ロード先の整数ポインタが無効です" ) ;
		}
		ESLError	err = obj.OperateInteger( nValue1 ) ;
		if ( err )
		{
			return	err ;
		}
		switch ( cscpType )
		{
		case	csctEqual:
			nResult = - (int) (nValue0 == nValue1) ;
			break ;
		case	csctLessEqual:
			nResult = - (int) (nValue0 <= nValue1) ;
			break ;
		case	csctGreaterEqual:
			nResult = - (int) (nValue0 >= nValue1) ;
			break ;
		case	csctNotEqual:
			nResult = - (int) (nValue0 != nValue1) ;
			break ;
		case	csctLessThan:
			nResult = - (int) (nValue0 < nValue1) ;
			break ;
		case	csctGreaterThan:
			nResult = - (int) (nValue0 > nValue1) ;
			break ;
		}
	}
	else
	{
		double	nValue0, nValue1 ;
		ESLError	err = obj.OperateReal( nValue1 ) ;
		if ( err )
		{
			return	err ;
		}
		if ( LoadBufferReal( nValue0 ) )
		{
			return	ESLErrorMsg( "ロード先の実数ポインタが無効です" ) ;
		}
		switch ( cscpType )
		{
		case	csctEqual:
			nResult = - (int) (nValue0 == nValue1) ;
			break ;
		case	csctLessEqual:
			nResult = - (int) (nValue0 <= nValue1) ;
			break ;
		case	csctGreaterEqual:
			nResult = - (int) (nValue0 >= nValue1) ;
			break ;
		case	csctNotEqual:
			nResult = - (int) (nValue0 != nValue1) ;
			break ;
		case	csctLessThan:
			nResult = - (int) (nValue0 < nValue1) ;
			break ;
		case	csctGreaterThan:
			nResult = - (int) (nValue0 > nValue1) ;
			break ;
		}
	}
	return	eslErrSuccess ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::GetVariableIndex( int & nIndex, int iMember )
{
	return	ECSObject::GetVariableIndex( nIndex, iMember ) ;
}

ESLError ECSPointerReference::GetVariableIndex
				( int & nIndex, const wchar_t * pwszMember )
{
	return	ECSObject::GetVariableIndex( nIndex, pwszMember ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPointerReference::GetVariableAt( int nIndex )
{
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPointerReference::SetVariableAt( int nIndex, ECSObject * obj )
{
	delete	obj ;
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	return	ECSReference::GetFunction( context, nIndex, pwszName ) ;
}

// メンバ関数ポインタ取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::GetFunctionPointer
	( ECSContext & context,
		ECS_FUNCTION_POINTER & fptr, const wchar_t * pwszName )
{
	return	ECSReference::GetFunctionPointer( context, fptr, pwszName ) ;
}

ESLError ECSPointerReference::GetFunctionPointer
	( ECSContext & context,
		ECS_FUNCTION_POINTER & fptr, int nIndex )
{
	return	ECSReference::GetFunctionPointer( context, fptr, nIndex ) ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	return	ECSReference::CallFunction( context, nIndex, lstArg ) ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::OperateBoolean( int & nBoolean )
{
	INT64	nValue ;
	if ( LoadBufferInteger( nValue ) )
	{
		return	ESLErrorMsg( "ロード先の整数ポインタが無効です" ) ;
	}
	nBoolean = - (int) (nValue != 0) ;
	return	eslErrSuccess ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::OperateSizeOf( INT64 & nSize )
{
	nSize = GetMemoryObjectBytes() ;
	if ( nSize == 0 )
	{
		return	ECSReference::OperateSizeOf( nSize ) ;
	}
	return	eslErrSuccess ;
}

// 特殊演算子 : interface 型変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::OperateCastInterface
		( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName )
{
	return	ECSReference::OperateCastInterface( ci, pwszTypeName ) ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::OperateInteger( INT64 & nValue )
{
	if ( LoadBufferInteger( nValue ) )
	{
		return	ESLErrorMsg( "ロード先の整数ポインタが無効です" ) ;
	}
	return	eslErrSuccess ;
}

// 実数取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::OperateReal( REAL64 & nValue )
{
	if ( LoadBufferReal( nValue ) )
	{
		return	ESLErrorMsg( "ロード先の実数ポインタが無効です" ) ;
	}
	return	eslErrSuccess ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::OperateString( EWideString & wstrValue )
{
	ESLError		err ;
	CSVariableType	csvtMemType = GetMemoryObjectType() ;
	if ( csvtMemType == csvtInteger )
	{
		INT64	nValue ;
		err = OperateInteger( nValue ) ;
		if ( !err )
		{
			wstrValue.FromInteger( nValue ) ;
			return	eslErrSuccess ;
		}
	}
	else if ( csvtMemType == csvtReal )
	{
		REAL64	nValue ;
		err = OperateReal( nValue ) ;
		if ( !err )
		{
			wstrValue = EWideString( nValue ) ;
			return	eslErrSuccess ;
		}
	}
	else
	{
		return	ECSReference::OperateString( wstrValue ) ;
	}
	return	err ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSPointerReference::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	if ( m_pRef->IsValidObject() )
	{
		return	m_pRef->GetBuffer( iOffset + m_iOffset, nSize, fWritable ) ;
	}
	return	NULL ;
}

void ECSPointerReference::FlushBuffer
	( int iOffset, int nSize, void * ptrBuf, bool fModified )
{
	if ( m_pRef->IsValidObject() )
	{
		m_pRef->FlushBuffer
			( iOffset + m_iOffset, nSize, ptrBuf, fModified ) ;
	}
}

ECSSakura2Processor::LinearAddressCache *
	ECSPointerReference::GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg )
{
	if ( m_pRef->IsValidObject() )
	{
		ECSSakura2Processor::LinearAddressCache *
					pSeg = m_pRef->GetSegmentBuffer( seg ) ;
		seg.pbytBuffer += m_iOffset ;
		//
		SDWORD	dwLastLimit = seg.limitSegment - m_iOffset ;
		DWORD	dwOutOfBounds = dwLastLimit >> 31 ;
		seg.limitSegment = dwLastLimit & ~dwOutOfBounds ;
		//
		return	pSeg ;
	}
	return	NULL ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSReference::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	file.Write( &m_iOffset, sizeof(m_iOffset) ) ;
	file.Write( &m_csvtRefType, sizeof(m_csvtRefType) ) ;
	file.Write( &m_fReadOnly, sizeof(m_fReadOnly) ) ;
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = ECSReference::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	file.Read( &m_iOffset, sizeof(m_iOffset) ) ;
	file.Read( &m_csvtRefType, sizeof(m_csvtRefType) ) ;
	file.Read( &m_fReadOnly, sizeof(m_fReadOnly) ) ;
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointerReference::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = FormatReferenceObjectIndex( context ) ;
	strDump += " : buffer[" ;
	strDump += EString( m_iOffset ) ;
	strDump += "] " ;
	//
	switch ( m_csvtRefType )
	{
	case	csvtInteger:
	case	csvtInteger64:
		strDump += "int64 " ;
		break ;
	case	csvtReal:
		strDump += "real64 " ;
		break ;
	case	csvtBoolean:
		strDump += "bool " ;
		break ;
	case	csvtInt8:
		strDump += "int8 " ;
		break ;
	case	csvtUint8:
		strDump += "uint8 " ;
		break ;
	case	csvtInt16:
		strDump += "int16 " ;
		break ;
	case	csvtUint16:
		strDump += "uint16 " ;
		break ;
	case	csvtInt32:
		strDump += "int32 " ;
		break ;
	case	csvtUint32:
		strDump += "uint32 " ;
		break ;
	}
	//
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	if ( m_pOwnObj != NULL )
	{
		return	m_pOwnObj->DumpObject( buf, nIndent, context ) ;
	}
	return	eslErrSuccess ;
}

// スクリプトのデストラクタ
//////////////////////////////////////////////////////////////////////////////
void ECSPointerReference::OnDestruction( ECSContext & context )
{
	ECSReference::OnDestruction( context ) ;
	m_csvtRefType = csvtObject ;
	m_fReadOnly = false ;
}


//////////////////////////////////////////////////////////////////////////////
// ポインターオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSPointer, ECSPointerReference )

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSPointer::GetTypeName( void ) const
{
	return	L"Pointer" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPointer::Duplicate( void )
{
	ECSPointer *	pRef = new ECSPointer ;
	pRef->CopyPointerFrom( NULL, *this ) ;
	return	pRef ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointer::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity != NULL )
	{
		INT64	nAddress ;
		if ( pEntity->m_vtType == csvtPointer )
		{
			ECSPointer *	pSrcPtr = (ECSPointer*) pEntity ;
			SetReferenceCastInterface( pSrcPtr->m_pRef, &context, *pSrcPtr ) ;
			m_iOffset = pSrcPtr->m_iOffset ;
			m_csvtRefType = pSrcPtr->m_csvtRefType ;
			//
			context.delete_CSObject( obj ) ;
			return	eslErrSuccess ;
		}
		else if ( !pEntity->OperateInteger( nAddress ) )
		{
			int			iOffset ;
			ECSObject *	pObj =
				ESLTypeCast<ECSObject>
					( context.GetObjectFromLinearAddress( nAddress, iOffset ) ) ;
			//
			SetReference( pObj, &context ) ;
			m_iOffset = iOffset ;
			//
			context.delete_CSObject( obj ) ;
			return	eslErrSuccess ;
		}
	}
	else
	{
		SetReference( NULL, &context ) ;
		m_iOffset = 0 ;
		//
		context.delete_CSObject( obj ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "不正なポインタの代入です" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointer::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	switch ( csuopType )
	{
	case	csuotLogicalNot:
		m_pResult = context.new_CSInteger
						( m_pRef->IsValidObject() ? 0 : -1 ) ;
		break ;
	case	csuotIncrement:
		m_iOffset ++ ;
		break ;
	case	csuotDecrement:
		m_iOffset -- ;
		break ;
	case	csuotIncrementAfter:
		m_pResult = context.new_CSPointer( *this ) ;
		m_iOffset ++ ;
		break ;
	case	csuotDecrementAfter:
		m_pResult = context.new_CSPointer( *this ) ;
		m_iOffset -- ;
		break ;
	default:
		return	ESLErrorMsg( "定義されていないポインタへの単項演算子です" ) ;
	}
	return	eslErrSuccess ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointer::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	ESLError	err ;
	INT64		nValue ;
	int			nBoolean ;
	switch ( csopType )
	{
	case	csotAdd:
		err = obj->OperateInteger( nValue ) ;
		if ( err )
		{
			return	err ;
		}
		m_iOffset += (int) nValue ;
		break ;
	case	csotSub:
		err = obj->OperateInteger( nValue ) ;
		if ( err )
		{
			return	err ;
		}
		m_iOffset -= (int) nValue ;
		break ;
	case	csotLogicalAnd:
		err = obj->OperateBoolean( nBoolean ) ;
		if ( err )
		{
			return	err ;
		}
		m_pResult = context.new_CSInteger
				( (m_pRef->IsValidObject() && nBoolean) ? -1 : 0 ) ;
		break ;
	case	csoutLogicalOr:
		err = obj->OperateBoolean( nBoolean ) ;
		if ( err )
		{
			return	err ;
		}
		m_pResult = context.new_CSInteger
				( (m_pRef->IsValidObject() || nBoolean) ? -1 : 0 ) ;
		break ;
	default:
		return	ESLErrorMsg( "定義されていないポインタへの演算子です" ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointer::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( &obj ) ;
	switch ( cscpType )
	{
	case	csctEqual:
		if ( pEntity->m_vtType == csvtPointer )
		{
			ECSPointer *	pSrcPtr = (ECSPointer*) pEntity ;
			nResult =
				- (int) ((m_pRef == pSrcPtr->m_pRef)
							&& ((m_pRef == NULL)
								|| (m_iOffset == pSrcPtr->m_iOffset))) ;
		}
		else
		{
			nResult =
				- (int) ((m_pRef == NULL) && (pEntity == NULL)) ;
		}
		break ;
	case	csctNotEqual:
		if ( pEntity->m_vtType == csvtPointer )
		{
			ECSPointer *	pSrcPtr = (ECSPointer*) pEntity ;
			nResult =
				- (int) ((m_pRef != pSrcPtr->m_pRef)
							|| ((m_pRef != NULL)
									&& (m_iOffset != pSrcPtr->m_iOffset))) ;
		}
		else
		{
			nResult =
				- (int) ((m_pRef != NULL) || (pEntity != NULL)) ;
		}
		break ;
	default:
		return	ESLErrorMsg( "ポインタの定義されていない比較演算子です" ) ;
	}
	return	eslErrSuccess ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointer::OperateBoolean( int & nBoolean )
{
	nBoolean = (m_pRef->IsValidObject() ? -1 : 0) ;
	return	eslErrSuccess ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPointer::OperateSizeOf( INT64 & nSize )
{
	nSize = 8 ;
	return	eslErrSuccess ;
}

