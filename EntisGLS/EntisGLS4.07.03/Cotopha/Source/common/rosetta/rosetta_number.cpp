
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_number.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// 数値
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSNumber, RSObject )

// 整数値設定
//////////////////////////////////////////////////////////////////////////////
void RSNumber::SetInteger( int32_t num )
{
	m_flagInt32 = true ;
	m_intValue = num ;
}

// 実数値設定
//////////////////////////////////////////////////////////////////////////////
void RSNumber::SetNumber( double num )
{
	m_flagInt32 = false ;
	m_fpValue = num ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSNumber::GetTypeName( void ) const
{
	return	L"Number" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumber::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"Number" ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::IsIntegerType( void ) const
{
	return	m_flagInt32 ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::IsFloatType( void ) const
{
	return	true ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::IsObjectType( void ) const
{
	return	false ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::AsInteger( int64_t& number ) const
{
	if ( m_flagInt32 )
	{
		number = m_intValue ;
	}
	else
	{
		number = (int32_t) eslRoundR64ToLInt( m_fpValue ) ;
	}
	return	true ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::AsRealNumber( double& number ) const
{
	if ( m_flagInt32 )
	{
		number = m_intValue ;
	}
	else
	{
		number = m_fpValue ;
	}
	return	true ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::AsBoolean( void ) const
{
	if ( m_flagInt32 )
	{
		return	m_intValue != 0 ;
	}
	else
	{
		return	m_fpValue != 0.0 ;
	}
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::AsString( SSystem::SString& strValue ) const
{
	if ( m_flagInt32 )
	{
		strValue.FromInteger( m_intValue ) ;
	}
	else
	{
		strValue.FromReal( m_fpValue ) ;
	}
	return	true ;
}

// 値設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSNumber::SetIntegerAs( int64_t nValue )
{
	if ( (nValue >= -0x7FFFFFFF) && (nValue <= 0x7FFFFFFF) )
	{
		SetInteger( (int32_t) nValue ) ;
	}
	else
	{
		SetNumber( (double) nValue ) ;
	}
	return	errSuccess ;
}

SSystem::SError RSNumber::SetNumberAs( double nValue )
{
	SetNumber( (double) nValue ) ;
	return	errSuccess ;
}

SSystem::SError RSNumber::SetStringAs( const wchar_t * pwszValue )
{
	SString	strNum( pwszValue ) ;
	bool	flagError = false ;
	SetNumber( strNum.AsReal( 10, &flagError ) ) ;
	return	flagError ? errFailed : errSuccess ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSNumber::IsEqualObject( RSObject * pObj ) const
{
	RSNumber *	pNum = ESLTypeCast<RSNumber>( pObj ) ;
	if ( pNum == NULL )
	{
		double	num ;
		if ( (pObj == NULL)
			|| !pObj->AsRealNumber( num ) )
		{
			return	false ;
		}
		if ( m_flagInt32 )
		{
			return	(m_intValue == num) ;
		}
		else
		{
			return	(m_fpValue == num) ;
		}
	}
	if ( m_flagInt32 )
	{
		return	pNum->m_flagInt32 && (m_intValue == pNum->m_intValue) ;
	}
	else
	{
		return	!(pNum->m_flagInt32) && (m_fpValue == pNum->m_fpValue) ;
	}
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumber::CloneObject( RSContext& context ) const
{
	if ( m_flagInt32 )
	{
		return	context.new_Number( m_flagInt32 ) ;
	}
	else
	{
		return	context.new_Number( m_fpValue ) ;
	}
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumber::OperatorPlus( RSContext& context ) const
{
	return	CloneObject( context ) ;
}

RSObject * RSNumber::OperatorNegate( RSContext& context ) const
{
	if ( m_flagInt32 )
	{
		return	context.new_Integer( - m_intValue ) ;
	}
	else
	{
		return	context.new_Number( - m_fpValue ) ;
	}
}

RSObject * RSNumber::OperatorBitNot( RSContext& context ) const
{
	int64_t	number ;
	AsInteger( number ) ;
	return	context.new_Integer( (int32_t) ~number ) ;
}

RSObject * RSNumber::OperatorIncrement( RSContext& context )
{
	int64_t	number ;
	AsInteger( number ) ;
	SetInteger( (int32_t) number + 1 ) ;
	AddRef() ;
	return	this ;
}

RSObject * RSNumber::OperatorDecrement( RSContext& context )
{
	int64_t	number ;
	AsInteger( number ) ;
	SetInteger( (int32_t) number - 1 ) ;
	AddRef() ;
	return	this ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumber::OperatorMul( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		number *= m_intValue ;
		return	context.new_Integer( number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Number( num1 * num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorDiv( RSContext& context, RSObject * pObj ) const
{
	double	num1, num2 = 0 ;
	AsRealNumber( num1 ) ;
	if ( pObj->AsRealNumber( num2 ) )
	{
		return	context.new_Number( num1 / num2 ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorMod( RSContext& context, RSObject * pObj ) const
{
	int64_t	num1, num2 = 0 ;
	AsInteger( num1 ) ;
	if ( pObj->AsInteger( num2 ) )
	{
		if ( num2 != 0 )
		{
			return	context.new_Integer( num1 % num2 ) ;
		}
		context.SetException( context.new_Exception( L"零除算エラー" ) ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		number += m_intValue ;
		return	context.new_Integer( number ) ;
	}
	else if ( pObj->IsStringType() )
	{
		SString	str1, str2 ;
		AsString( str1 ) ;
		pObj->AsString( str2 ) ;
		return	context.new_String( str1 + str2 ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Number( num1 + num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		number = m_intValue - number ;
		return	context.new_Integer( number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Number( num1 - num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorShiftLeft( RSContext& context, RSObject * pObj ) const
{
	int64_t	num1, num2 = 0 ;
	AsInteger( num1 ) ;
	if ( pObj->AsInteger( num2 ) )
	{
		return	context.new_Integer( num1 << num2 ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorShiftRight( RSContext& context, RSObject * pObj ) const
{
	int64_t	num1, num2 = 0 ;
	AsInteger( num1 ) ;
	if ( pObj->AsInteger( num2 ) )
	{
		return	context.new_Integer( num1 >> num2 ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const
{
	int64_t	num1, num2 = 0 ;
	AsInteger( num1 ) ;
	if ( pObj->AsInteger( num2 ) )
	{
		return	context.new_Integer( (uint64_t) num1 >> num2 ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorBitAnd( RSContext& context, RSObject * pObj ) const
{
	int64_t	num1, num2 = 0 ;
	AsInteger( num1 ) ;
	if ( pObj->AsInteger( num2 ) )
	{
		return	context.new_Integer( num1 & num2 ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorBitOr( RSContext& context, RSObject * pObj ) const
{
	int64_t	num1, num2 = 0 ;
	AsInteger( num1 ) ;
	if ( pObj->AsInteger( num2 ) )
	{
		return	context.new_Integer( num1 | num2 ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorBitXor( RSContext& context, RSObject * pObj ) const
{
	int64_t	num1, num2 = 0 ;
	AsInteger( num1 ) ;
	if ( pObj->AsInteger( num2 ) )
	{
		return	context.new_Integer( num1 ^ num2 ) ;
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue == number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Boolean( num1 == num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue != number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Boolean( num1 != num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue >= number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Boolean( num1 >= num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue > number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Boolean( num1 > num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue <= number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Boolean( num1 <= num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSNumber::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	if ( m_flagInt32 && pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue < number ) ;
	}
	else
	{
		double	num1, num2 = 0 ;
		AsRealNumber( num1 ) ;
		if ( pObj->AsRealNumber( num2 ) )
		{
			return	context.new_Boolean( num1 < num2 ) ;
		}
	}
	return	NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumber::OperatorMove( RSContext& context, RSObject * pObj )
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		if ( pObj->AsInteger( number ) )
		{
			m_flagInt32 = true ;
			m_intValue = (int32_t) number ;
			AddRef() ;
			return	this ;
		}
	}
	double	number ;
	if ( pObj->AsRealNumber( number ) )
	{
		m_flagInt32 = false ;
		m_fpValue = number ;
		AddRef() ;
		return	this ;
	}
	context.ThrowExceptionError( L"数値へ変換できません" ) ;
	return	NULL ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumber::SerializeObject( RSContext& context )
{
	return	CloneObject( context ) ;
}

SSystem::SError RSNumber::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	uint8_t	type = m_flagInt32 ;
	file.Write( &type, sizeof(uint8_t) ) ;
	if ( m_flagInt32 )
	{
		file.Write( &m_intValue, sizeof(int32_t) ) ;
	}
	else
	{
		file.Write( &m_fpValue, sizeof(double) ) ;
	}
	return	errSuccess ;
}

SSystem::SError RSNumber::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	if ( m_flagInt32 )
	{
		xmlDoc.SetAttributeAs( L"type", L"int32" ) ;
		xmlDoc.SetAttrIntegerAs( L"int32", m_intValue ) ;
	}
	else
	{
		xmlDoc.SetAttributeAs( L"type", L"double" ) ;
		xmlDoc.SetAttrRealAs( L"double", m_fpValue ) ;
	}
	return	errSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSNumber::RestoreObject( RSContext& context, RSObject * pObj )
{
	context.ReleaseObjectRef( OperatorMove( context, pObj ) ) ;
	return	errSuccess ;
}

SSystem::SError RSNumber::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	uint8_t	type ;
	if ( file.Read( &type, sizeof(uint8_t) ) < sizeof(uint8_t) )
	{
		return	errFailed ;
	}
	m_flagInt32 = (type != 0) ;
	if ( m_flagInt32 )
	{
		if ( file.Read( &m_intValue, sizeof(int32_t) ) < sizeof(int32_t) )
		{
			return	errFailed ;
		}
	}
	else
	{
		if ( file.Read( &m_fpValue, sizeof(double) ) < sizeof(double) )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

SSystem::SError RSNumber::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	const SString *	pstrType = xmlDoc.GetAttributeAs( L"type" ) ;
	if ( (pstrType != NULL) && (*pstrType == L"int32") )
	{
		m_flagInt32 = true ;
		m_intValue = (int32_t) xmlDoc.GetAttrIntegerAs( L"int32", 0 ) ;
	}
	else
	{
		m_flagInt32 = false ;
		m_fpValue = xmlDoc.GetAttrRealAs( L"double", 0.0 ) ;
	}
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 数値型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSNumberClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSNumberClass::RSNumberClass( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSNumberClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddFunctionDescriptiveAs
		( context, perr, L"doubleToLongBits", L"long", L"double a",
				NULL, &RSNumberClass::method_doubleToLongBits, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"floatToIntBits", L"int", L"float a",
				NULL, &RSNumberClass::method_floatToIntBits, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"longBitsToDouble", L"double", L"long bits",
				NULL, &RSNumberClass::method_longBitsToDouble, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"intBitsToFloat", L"float", L"int bits",
				NULL, &RSNumberClass::method_intBitsToFloat, NULL ) ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumberClass::NewInstance( RSContext& context, RSObject * pArg )
{
	if ( (pArg != NULL) && (pArg->GetElementCount() >= 1) )
	{
		if ( pArg->GetElementCount() >= 2 )
		{
			context.ThrowExceptionError
				( m_strClassName + L" の構築引数が多すぎます" ) ;
			return	NULL ;
		}
		RSObject *	pObjArg = pArg->GetElementAt( context, 0 ) ;
		if ( pObjArg != NULL )
		{
			RSObject *	pNumber = context.new_Number( 0.0 ) ;
			RSObject *	pObj = pNumber->OperatorMove( context, pObjArg ) ;
			context.ReleaseObjectRef( pObjArg ) ;
			context.ReleaseObjectRef( pObj ) ;
			if ( pObj == NULL )
			{
				context.ThrowExceptionError
					( m_strClassName + L" の初期値が不正です" ) ;
			}
			pNumber->SetRSClass( this ) ;
			return	pNumber ;
		}
		context.ThrowExceptionError
			( m_strClassName + L" の初期値が不正です" ) ;
		return	NULL ;
	}
	RSObject *	pNumber = context.new_Number( 0.0 ) ;
	pNumber->SetRSClass( this ) ;
	return	pNumber ;
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumberClass::NewVariable( RSContext& context )
{
	RSObject *	pNumber = context.new_Number( 0.0 ) ;
	pNumber->SetRSClass( this ) ;
	return	pNumber ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
bool RSNumberClass::TestCastInstance
	( RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj != NULL )
	{
		double	num ;
		return	pObj->IsIntegerType() || pObj->AsRealNumber( num ) ;
	}
	return	false ;
}

RSObject * RSNumberClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	if ( pObj->IsIntegerType() )
	{
		int64_t	num ;
		if ( pObj->AsInteger( num ) )
		{
			if ( (castMethod == RSClass::castForce)
				|| ((-0x7FFFFFFF <= num) && (num <= 0x7FFFFFFF)) )
			{
				return	context.new_NumberInt32( (int32_t) num ) ;
			}
		}
	}
	double	num ;
	if ( pObj->AsRealNumber( num ) )
	{
		return	context.new_Number( num ) ;
	}
	return	NULL ;
}

// static long doubleToLongBits( double value )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumberClass::method_doubleToLongBits
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	float64_t	value = arg.DoubleAt( 0 ) ;
	return		context.new_Integer( *((int64_t*)&value) ) ;
}

// static int floatToIntBits( float value )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumberClass::method_floatToIntBits
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	float32_t	value = (float32_t) arg.DoubleAt( 0 ) ;
	return		context.new_Integer( *((int32_t*)&value) ) ;
}

// static double longBitsToDouble( long bits )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumberClass::method_longBitsToDouble
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t		value = arg.LongAt( 0 ) ;
	return		context.new_Number( *((float64_t*)&value) ) ;
}

// static float intBitsToFloat( int bits )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNumberClass::method_intBitsToFloat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int32_t		value = (int32_t) arg.LongAt( 0 ) ;
	return		context.new_Number( *((float32_t*)&value) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 整数値
//////////////////////////////////////////////////////////////////////////////

const RSInteger::TypeParam	RSInteger::m_typeParam[RSInteger::typeCount] =
{
	{	0x00, 0x01, 1 },
	{	0x7F, 0x80, 8 },
	{	0xFF, 0x00, 8 },
	{	0x7FFF, 0x8000, 16 },
	{	0xFFFF, 0x0000, 16 },
	{	0x7FFFFFFF, 0x80000000, 32 },
	{	0xFFFFFFFF, 0x00000000, 32 },
	{	(int64_t) -1 /*0xFFFFFFFFFFFFFFFF*/, 0x0000000000000000, 64 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSInteger, RSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSInteger::RSInteger
		( RSClass * pClass, int64_t numInt, RSInteger::IntegerType intType )
	: RSObject( pClass, typeInteger )
{
	TypeParam	tp = m_typeParam[intType] ;
	m_intType = intType ;
	m_intValue = (numInt & tp.maskBits) | -(numInt & tp.maskSign) ;
}

// 数値設定
//////////////////////////////////////////////////////////////////////////////
void RSInteger::SetInteger( int64_t num )
{
	TypeParam	tp = m_typeParam[m_intType] ;
	m_intValue = (num & tp.maskBits) | -(num & tp.maskSign) ;
}

// 数値型設定
//////////////////////////////////////////////////////////////////////////////
void RSInteger::SetIntegerType( RSInteger::IntegerType intType )
{
	m_intType = intType ;
	SetInteger( m_intValue ) ;
}

// 型の符号有無
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::IsSign( void ) const
{
	return	(m_typeParam[m_intType].maskSign != 0) ;
}

// 型のビットサイズ
//////////////////////////////////////////////////////////////////////////////
size_t RSInteger::BitSizeOf( void ) const
{
	return	m_typeParam[m_intType].sizeBits ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSInteger::GetTypeName( void ) const
{
	return	L"Integer" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInteger::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"Integer" ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::IsIntegerType( void ) const
{
	return	true ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::IsFloatType( void ) const
{
	return	false ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::IsObjectType( void ) const
{
	return	false ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::AsInteger( int64_t& number ) const
{
	number = m_intValue ;
	return	true ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::AsRealNumber( double& number ) const
{
	number = (double) m_intValue ;
	return	true ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::AsBoolean( void ) const
{
	return	m_intValue != 0 ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::AsString( SSystem::SString& strValue ) const
{
	strValue.FromInteger( m_intValue ) ;
	return	true ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSInteger::IsEqualObject( RSObject * pObj ) const
{
	if ( (pObj != NULL) && pObj->IsIntegerType() )
	{
		int64_t	num ;
		if ( pObj->AsInteger( num ) )
		{
			return	(m_intValue == num) ;
		}
		return	false ;
	}
	double	num ;
	if ( (pObj != NULL) && pObj->AsRealNumber( num ) )
	{
		return	(m_intValue == num) ;
	}
	return	false ;
}

// 値設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInteger::SetIntegerAs( int64_t nValue )
{
	SetInteger( nValue ) ;
	return	errSuccess ;
}

SSystem::SError RSInteger::SetNumberAs( double nValue )
{
	int64_t	num = esl_lroundfi( nValue ) ;
	SetInteger( num ) ;
	return	(num == nValue) ? errSuccess : errFailed ;
}

SSystem::SError RSInteger::SetStringAs( const wchar_t * pwszValue )
{
	SString	strNum( pwszValue ) ;
	bool	flagError = false ;
	SetInteger( strNum.AsInteger( 10, true, &flagError ) ) ;
	return	flagError ? errFailed : errSuccess ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInteger::CloneObject( RSContext& context ) const
{
	return	context.new_Integer( m_intValue, m_intType ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInteger::OperatorPlus( RSContext& context ) const
{
	return	CloneObject( context ) ;
}

RSObject * RSInteger::OperatorNegate( RSContext& context ) const
{
	return	context.new_Integer( - m_intValue, m_intType ) ;
}

RSObject * RSInteger::OperatorBitNot( RSContext& context ) const
{
	return	context.new_Integer( ~m_intValue, m_intType ) ;
}

RSObject * RSInteger::OperatorIncrement( RSContext& context )
{
	SetInteger( m_intValue + 1 ) ;
	AddRef() ;
	return	this ;
}

RSObject * RSInteger::OperatorDecrement( RSContext& context )
{
	SetInteger( m_intValue - 1 ) ;
	AddRef() ;
	return	this ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInteger::OperatorMul( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Integer( m_intValue * number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Number( (double) m_intValue * number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorDiv( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		if ( number != 0 )
		{
			return	context.new_Integer( m_intValue / number ) ;
		}
		context.ThrowExceptionError( L"零除算エラー" ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Number( (double) m_intValue / number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorMod( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		if ( number != 0 )
		{
			return	context.new_Integer( m_intValue % number ) ;
		}
		context.ThrowExceptionError( L"零除算エラー" ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Number( (double) m_intValue / number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Integer( m_intValue + number ) ;
	}
	else if ( pObj->IsStringType() )
	{
		SString	str1, str2 ;
		AsString( str1 ) ;
		pObj->AsString( str2 ) ;
		return	context.new_String( str1 + str2 ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Number( (double) m_intValue + number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Integer( m_intValue - number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Number( (double) m_intValue - number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorShiftLeft( RSContext& context, RSObject * pObj ) const
{
	int64_t	number = 0 ;
	if ( pObj->AsInteger( number ) )
	{
		return	context.new_Integer( m_intValue << number ) ;
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorShiftRight( RSContext& context, RSObject * pObj ) const
{
	int64_t	number = 0 ;
	if ( pObj->AsInteger( number ) )
	{
		return	context.new_Integer( m_intValue >> number ) ;
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const
{
	int64_t	number = 0 ;
	if ( pObj->AsInteger( number ) )
	{
		return	context.new_Integer( (uint64_t) m_intValue >> number ) ;
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorBitAnd( RSContext& context, RSObject * pObj ) const
{
	int64_t	number = 0 ;
	if ( pObj->AsInteger( number ) )
	{
		return	context.new_Integer( m_intValue & number ) ;
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorBitOr( RSContext& context, RSObject * pObj ) const
{
	int64_t	number = 0 ;
	if ( pObj->AsInteger( number ) )
	{
		return	context.new_Integer( m_intValue | number ) ;
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorBitXor( RSContext& context, RSObject * pObj ) const
{
	int64_t	number = 0 ;
	if ( pObj->AsInteger( number ) )
	{
		return	context.new_Integer( m_intValue ^ number ) ;
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue == number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Boolean( m_intValue == number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue != number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Boolean( m_intValue != number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue >= number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Boolean( m_intValue >= number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue > number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Boolean( m_intValue > number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue <= number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Boolean( m_intValue <= number ) ;
		}
	}
	return	NULL ;
}

RSObject * RSInteger::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	if ( pObj->IsIntegerType() )
	{
		int64_t	number = 0 ;
		pObj->AsInteger( number ) ;
		return	context.new_Boolean( m_intValue < number ) ;
	}
	else
	{
		double	number = 0 ;
		if ( pObj->AsRealNumber( number ) )
		{
			return	context.new_Boolean( m_intValue < number ) ;
		}
	}
	return	NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInteger::OperatorMove( RSContext& context, RSObject * pObj )
{
	int64_t	number = 0 ;
	if ( pObj->AsInteger( number ) )
	{
		SetInteger( number ) ;
		AddRef() ;
		return	this ;
	}
	context.ThrowExceptionError( L"整数へ変換できません" ) ;
	return	NULL ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
RSObject * RSInteger::SerializeObject( RSContext& context )
{
	return	CloneObject( context ) ;
}

SSystem::SError RSInteger::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	if ( file.Write( &m_intValue, sizeof(int64_t) ) < sizeof(int64_t) )
	{
		return	errFailed ;
	}
	int32_t	intType = (int32_t) m_intType ;
	if ( file.Write( &intType, sizeof(int32_t) ) < sizeof(int32_t) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError RSInteger::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	xmlDoc.SetAttrHexIntegerAs( L"int64", m_intValue ) ;
	xmlDoc.SetAttrIntegerAs( L"type", m_intType ) ;
	return	errSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInteger::RestoreObject( RSContext& context, RSObject * pObj )
{
	context.ReleaseObjectRef( OperatorMove( context, pObj ) ) ;
	return	errSuccess ;
}

SSystem::SError RSInteger::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	if ( file.Read( &m_intValue, sizeof(int64_t) ) < sizeof(int64_t) )
	{
		return	errFailed ;
	}
	int32_t	intType ;
	if ( file.Read( &intType, sizeof(int32_t) ) < sizeof(int32_t) )
	{
		return	errFailed ;
	}
	m_intType = (IntegerType) intType ;
	return	errSuccess ;
}

SSystem::SError RSInteger::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	m_intValue = xmlDoc.GetAttrHexIntegerAs( L"int64", 0 ) ;
	m_intType = (IntegerType) xmlDoc.GetAttrIntegerAs( L"type", 0 ) ;
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 整数型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSIntegerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSIntegerClass::RSIntegerClass
	( RSClass * pClass,
		const wchar_t * pwszClassName, RSInteger::IntegerType intType )
	: RSClass( pClass, pwszClassName ), m_intType( intType )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSIntegerClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddFunctionDescriptiveAs
		( context, perr, L"longBitsToDouble", L"double", L"long bits",
				NULL, &RSIntegerClass::method_longBitsToDouble, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"intBitsToFloat", L"float", L"int bits",
				NULL, &RSIntegerClass::method_intBitsToFloat, NULL ) ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSIntegerClass::NewInstance( RSContext& context, RSObject * pArg )
{
	if ( (pArg != NULL) && (pArg->GetElementCount() >= 1) )
	{
		if ( pArg->GetElementCount() >= 2 )
		{
			context.ThrowExceptionError
				( m_strClassName + L" の構築引数が多すぎます" ) ;
			return	NULL ;
		}
		RSObject *	pObjArg = pArg->GetElementAt( context, 0 ) ;
		if ( pObjArg != NULL )
		{
			int64_t	num ;
			if ( pObjArg->AsInteger( num ) )
			{
				RSObject *	pNumber = context.new_Integer( num, m_intType ) ;
				pNumber->SetRSClass( this ) ;
				return	pNumber ;
			}
		}
		context.ThrowExceptionError
			( m_strClassName + L" の初期値が不正です" ) ;
		return	NULL ;
	}
	RSObject *	pNumber = context.new_Integer( 0, m_intType ) ;
	pNumber->SetRSClass( this ) ;
	return	pNumber ;
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSIntegerClass::NewVariable( RSContext& context )
{
	RSObject *	pNumber = context.new_Integer( 0, m_intType ) ;
	pNumber->SetRSClass( this ) ;
	return	pNumber ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
bool RSIntegerClass::TestCastInstance
	( RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj != NULL )
	{
		if ( pObj->IsIntegerType() )
		{
			return	true ;
		}
		if ( castMethod == RSClass::castForce )
		{
			double	num ;
			return	pObj->AsRealNumber( num ) ;
		}
	}
	return	false ;
}

RSObject * RSIntegerClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj != NULL )
	{
		if ( pObj->IsIntegerType() )
		{
			int64_t	num ;
			if ( pObj->AsInteger( num ) )
			{
				RSObject *	pNumber = context.new_Integer( num, m_intType ) ;
				pNumber->SetRSClass( this ) ;
				//
				if ( castMethod == RSClass::castNatural )
				{
					int64_t	numCast ;
					if ( !pNumber->AsInteger(numCast) || (num != numCast) )
					{
						context.ReleaseObjectRef( pNumber ) ;
						pNumber = NULL ;
					}
				}
				return	pNumber ;
			}
		}
		if ( castMethod == RSClass::castForce )
		{
			double	num ;
			if ( pObj->AsRealNumber( num ) )
			{
				RSObject *	pNumber =
					context.new_Integer( eslRoundR64ToLInt(num), m_intType ) ;
				pNumber->SetRSClass( this ) ;
				return	pNumber ;
			}
		}
	}
	return	NULL ;
}

// static double longBitsToDouble( long bits )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSIntegerClass::method_longBitsToDouble
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t		value = arg.LongAt( 0 ) ;
	return		context.new_Number( *((float64_t*)&value) ) ;
}

// static float intBitsToFloat( int bits )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSIntegerClass::method_intBitsToFloat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int32_t		value = (int32_t) arg.LongAt( 0 ) ;
	return		context.new_Number( *((float32_t*)&value) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ブール値
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSBoolean, RSNumber )

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSBoolean::GetTypeName( void ) const
{
	return	L"Boolean" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBoolean::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"Boolean" ) == 0 )
	{
		return	this ;
	}
	return	RSNumber::InstanceOf( pwszType ) ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSBoolean::AsInteger( int64_t& number ) const
{
	number = AsBoolean() ? -1 : 0 ;
	return	true ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSBoolean::AsRealNumber( double& number ) const
{
	number = AsBoolean() ? -1 : 0 ;
	return	true ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSBoolean::AsString( SSystem::SString& strValue ) const
{
	if ( AsBoolean() )
	{
		strValue = L"true" ;
	}
	else
	{
		strValue = L"false" ;
	}
	return	true ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSBoolean::IsEqualObject( RSObject * pObj ) const
{
	if ( pObj != NULL )
	{
		pObj = pObj->GetEntityObject() ;
	}
	RSBoolean *	pBoolean = ESLTypeCast<RSBoolean>( pObj ) ;
	if ( pBoolean == NULL )
	{
		return	false ;
	}
	return	(AsBoolean() == pObj->AsBoolean()) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBoolean::CloneObject( RSContext& context ) const
{
	return	context.new_Boolean( AsBoolean() ) ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBoolean::OperatorMove( RSContext& context, RSObject * pObj )
{
	m_flagInt32 = true ;
	m_intValue = (pObj->AsBoolean() ? 1 : 0) ;
	AddRef() ;
	return	this ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSBoolean::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	if ( AsBoolean() )
	{
		xmlDoc.SetAttributeAs( L"boolean", L"true" ) ;
	}
	else
	{
		xmlDoc.SetAttributeAs( L"boolean", L"false" ) ;
	}
	return	errSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSBoolean::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	const SString *	pstrBoolean = xmlDoc.GetAttributeAs( L"boolean" ) ;
	if ( (pstrBoolean != NULL) && (*pstrBoolean == L"true") )
	{
		m_flagInt32 = true ;
		m_intValue = 1 ;
	}
	else
	{
		m_flagInt32 = true ;
		m_intValue = 0 ;
	}
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// ブール型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSBooleanClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSBooleanClass::RSBooleanClass( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSBooleanClass::OverrideVirtuals( RSContext& context )
{
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBooleanClass::NewInstance( RSContext& context, RSObject * pArg )
{
	if ( (pArg != NULL) && (pArg->GetElementCount() >= 1) )
	{
		if ( pArg->GetElementCount() >= 2 )
		{
			context.ThrowExceptionError
				( m_strClassName + L" の構築引数が多すぎます" ) ;
			return	NULL ;
		}
		RSObject *	pObjArg = pArg->GetElementAt( context, 0 ) ;
		if ( pObjArg != NULL )
		{
			return	context.new_Boolean( pObjArg->AsBoolean() ) ;
		}
		context.ThrowExceptionError
			( m_strClassName + L" の初期値が不正です" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( false ) ;
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBooleanClass::NewVariable( RSContext& context )
{
	return	context.new_Boolean( false ) ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
bool RSBooleanClass::TestCastInstance
	( RSObject * pObj, RSClass::CastMethod castMethod )
{
	return	true ;
}

RSObject * RSBooleanClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj != NULL )
	{
		RSObject *	pEntity = pObj->GetEntityObject() ;
		if ( pEntity != NULL )
		{
			if ( (castMethod == RSClass::castForce)
				|| (pEntity->GetRSClass() == this) )
			{
				return	context.new_Boolean( pObj->AsBoolean() ) ;
			}
			else if ( pEntity->IsIntegerType() )
			{
				int64_t	num ;
				if ( pObj->AsInteger( num ) )
				{
					return	context.new_Boolean( num != 0 ) ;
				}
			}
			return	NULL ;
		}
	}
	return	context.new_Boolean( false ) ;
}

