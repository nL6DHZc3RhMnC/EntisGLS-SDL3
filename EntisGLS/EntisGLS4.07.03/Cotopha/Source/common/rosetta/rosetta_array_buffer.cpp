
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSArrayBuffer, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSArrayBuffer::~RSArrayBuffer( void )
{
}

// バッファ確保
//////////////////////////////////////////////////////////////////////////////
void RSArrayBuffer::AllocateBuffer( size_t nBytes )
{
	m_ptrBuf = m_buffer.GetArray( nBytes ) ;
	m_lenBuf = nBytes ;
}

// バッファ関連付け
//////////////////////////////////////////////////////////////////////////////
void RSArrayBuffer::AttachBuffer( uint8_t * ptrBuf, size_t nBytes )
{
	m_buffer.FreeArray() ;
	m_ptrBuf = ptrBuf ;
	m_lenBuf = nBytes ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSArrayBuffer::GetTypeName( void ) const
{
	return	L"ArrayBuffer" ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSArrayBuffer::AsString( SSystem::SString& strValue ) const
{
	strValue = L"buffer:{#" ;
	if ( sizeof(uint8_t*) > 4 )
	{
		strValue += SString( (ulong_ptr_t) m_ptrBuf, 16, 16 ) ;
	}
	else
	{
		strValue += SString( (ulong_ptr_t) m_ptrBuf, 8, 16 ) ;
	}
	strValue += L"," ;
	strValue += SString( m_lenBuf ) ;
	strValue += L"bytes}" ;
	return	true ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSArrayBuffer::DisposeObject( RSContext& context )
{
	m_buffer.FreeArray() ;
	m_ptrBuf = NULL ;
	m_lenBuf = 0 ;
	//
	RSObject::DisposeObject( context ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayBuffer::CloneObject( RSContext& context ) const
{
	RSArrayBuffer *	pBuf = new RSArrayBuffer( m_pClass ) ;
	pBuf->m_buffer = m_buffer ;
	if ( m_ptrBuf == m_buffer.GetArray() )
	{
		pBuf->m_ptrBuf = pBuf->m_buffer.GetArray() ;
		pBuf->m_lenBuf = pBuf->m_buffer.GetLength() ;
	}
	else
	{
		pBuf->m_ptrBuf = m_ptrBuf ;
		pBuf->m_lenBuf = m_lenBuf ;
	}
	return	pBuf ;
}


//////////////////////////////////////////////////////////////////////////////
// バッファ参照数値
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSReferenceNumber, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSReferenceNumber::~RSReferenceNumber( void )
{
	RSObject::ReleaseRef( m_pRef ) ;
	m_pRef = NULL ;
}

// 要素サイズ計算
//////////////////////////////////////////////////////////////////////////////
size_t RSReferenceNumber::GetNumberSizeOf( NumberType type )
{
	static const size_t	nBytes[] =
	{
		1, 1, 2, 2, 4, 4, 8, 4, 8, sizeof(void*)
	} ;
	return	nBytes[type] ;
}

// 要素型名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSReferenceNumber::GetNumberTypeName( NumberType type )
{
	static const wchar_t *	pwszNames[] =
	{
		L"uint8", L"byte", L"char", L"short", L"uint", L"int",
		L"long", L"float", L"double", L"Object"
	} ;
	return	pwszNames[type] ;
}

// プリミティブデータ型変換
//////////////////////////////////////////////////////////////////////////////
RSReferenceNumber::NumberType
	RSReferenceNumber::FromPrimitiveType( RSPrimitiveNumberType type )
{
	static const RSReferenceNumber::NumberType	refNumType[] =
	{
		typeUint8, typeInt8,
		typeUint16, typeInt16,
		typeUint32, typeInt32,
		typeInt64, typeFloat32,
		typeFloat64, typeCountOfNumber,
		typeObject,
	} ;
	ESLAssert( type >= 0 ) ;
	ESLAssert( type < sizeof(refNumType)/sizeof(refNumType[0]) ) ;
	return	refNumType[type] ;
}

// 参照設定
//////////////////////////////////////////////////////////////////////////////
void RSReferenceNumber::SetReference
	( void * ptrBuf, NumberType type, RSObject * pRef )
{
	RSObject::ReleaseRef( m_pRef ) ;
	m_pRef = pRef ;
	//
	m_type = type ;
	m_ptrBuffer = ptrBuf ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
int64_t RSReferenceNumber::LoadInteger( void ) const
{
	ESLAssert( m_ptrBuffer != NULL ) ;
	uint8_t *	pbytBuf = (uint8_t*) m_ptrBuffer ;
	switch ( m_type )
	{
	case	typeUint8:
		return	*pbytBuf ;
	case	typeInt8:
		return	(int8_t) *pbytBuf ;
	case	typeUint16:
		return	*((uint16_t*)pbytBuf) ;
	case	typeInt16:
		return	*((int16_t*)pbytBuf) ;
	case	typeUint32:
		return	*((uint32_t*)pbytBuf) ;
	case	typeInt32:
		return	*((int32_t*)pbytBuf) ;
	case	typeInt64:
		return	*((int64_t*)pbytBuf) ;
	case	typeFloat32:
		return	eslRoundR32ToInt( *((float32_t*)pbytBuf) ) ;
	case	typeFloat64:
		return	eslRoundR64ToLInt( *((float64_t*)pbytBuf) ) ;
	default:
		break ;
	}
	return	0 ;
}

// 整数値設定
//////////////////////////////////////////////////////////////////////////////
void RSReferenceNumber::StoreInteger( int64_t num ) const
{
	ESLAssert( m_ptrBuffer != NULL ) ;
	uint8_t *	pbytBuf = (uint8_t*) m_ptrBuffer ;
	switch ( m_type )
	{
	case	typeUint8:
	case	typeInt8:
		*pbytBuf = (uint8_t) num ;
		break ;
	case	typeUint16:
		*((uint16_t*)pbytBuf) = (uint16_t) num ;
		break ;
	case	typeInt16:
		*((int16_t*)pbytBuf) = (int16_t) num ;
		break ;
	case	typeUint32:
		*((uint32_t*)pbytBuf) = (uint32_t) num ;
		break ;
	case	typeInt32:
		*((int32_t*)pbytBuf) = (int32_t) num ;
		break ;
	case	typeInt64:
		*((int64_t*)pbytBuf) = num ;
		break ;
	case	typeFloat32:
		*((float32_t*)pbytBuf) = (float32_t) num ;
		break ;
	case	typeFloat64:
		*((float64_t*)pbytBuf) = (float64_t) num ;
		break ;
	default:
		break ;
	}
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
double RSReferenceNumber::LoadRealNumber( void ) const
{
	ESLAssert( m_ptrBuffer != NULL ) ;
	uint8_t *	pbytBuf = (uint8_t*) m_ptrBuffer ;
	switch ( m_type )
	{
	case	typeUint8:
		return	*pbytBuf ;
	case	typeInt8:
		return	*((int8_t*)pbytBuf) ;
	case	typeUint16:
		return	*((uint16_t*)pbytBuf) ;
	case	typeInt16:
		return	*((int16_t*)pbytBuf) ;
	case	typeUint32:
		return	*((uint32_t*)pbytBuf) ;
	case	typeInt32:
		return	*((int32_t*)pbytBuf) ;
	case	typeInt64:
		return	(double) *((int64_t*)pbytBuf) ;
	case	typeFloat32:
		return	*((float32_t*)pbytBuf) ;
	case	typeFloat64:
		return	*((float64_t*)pbytBuf) ;
	default:
		break ;
	}
	return	0 ;
}

// 実数値設定
//////////////////////////////////////////////////////////////////////////////
void RSReferenceNumber::StoreRealNumber( double num ) const
{
	ESLAssert( m_ptrBuffer != NULL ) ;
	uint8_t *	pbytBuf = (uint8_t*) m_ptrBuffer ;
	switch ( m_type )
	{
	case	typeUint8:
		*pbytBuf = (uint8_t) eslRoundR64ToLInt( num ) ;
		break ;
	case	typeInt8:
		*((int8_t*)pbytBuf) = (int8_t) eslRoundR64ToLInt( num ) ;
		break ;
	case	typeUint16:
		*((uint16_t*)pbytBuf) = (uint16_t) eslRoundR64ToLInt( num ) ;
		break ;
	case	typeInt16:
		*((int16_t*)pbytBuf) = (int16_t) eslRoundR64ToLInt( num ) ;
		break ;
	case	typeUint32:
		*((uint32_t*)pbytBuf) = (uint32_t) eslRoundR64ToLInt( num ) ;
		break ;
	case	typeInt32:
		*((int32_t*)pbytBuf) = (int32_t) eslRoundR64ToLInt( num ) ;
		break ;
	case	typeInt64:
		*((int64_t*)pbytBuf) = (int64_t) eslRoundR64ToLInt( num ) ;
		break ;
	case	typeFloat32:
		*((float32_t*)pbytBuf) = (float32_t) num ;
		break ;
	case	typeFloat64:
		*((float64_t*)pbytBuf) = num ;
		break ;
	default:
		break ;
	}
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSReferenceNumber::GetTypeName( void ) const
{
	return	L"ReferenceNumber" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceNumber::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"ReferenceNumber" ) == 0 )
	{
		return	this ;
	}
	return	RSObject::InstanceOf( pwszType ) ;
}

RSObject * RSReferenceNumber::InstanceOf( RSClass * pClass )
{
	return	RSObject::InstanceOf( pClass ) ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::IsIntegerType( void ) const
{
	return	(m_type < typeFloat32) ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::IsFloatType( void ) const
{
	return	(m_type >= typeFloat32) ;
}

// 文字列型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::IsStringType( void ) const
{
	return	false ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::IsObjectType( void ) const
{
	return	false ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::AsInteger( int64_t& number ) const
{
	if ( m_ptrBuffer == NULL )
	{
		return	false ;
	}
	number = LoadInteger() ;
	return	true ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::AsRealNumber( double& number ) const
{
	if ( m_ptrBuffer == NULL )
	{
		return	false ;
	}
	number = LoadRealNumber() ;
	return	true ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::AsBoolean( void ) const
{
	if ( m_ptrBuffer == NULL )
	{
		return	false ;
	}
	switch ( m_type )
	{
	case	typeUint8:
		return	*((uint8_t*)m_ptrBuffer) != 0 ;
	case	typeInt8:
		return	*((int8_t*)m_ptrBuffer) != 0 ;
	case	typeUint16:
		return	*((uint16_t*)m_ptrBuffer) != 0 ;
	case	typeInt16:
		return	*((int16_t*)m_ptrBuffer) != 0 ;
	case	typeUint32:
		return	*((uint32_t*)m_ptrBuffer) != 0 ;
	case	typeInt32:
		return	*((int32_t*)m_ptrBuffer) != 0 ;
	case	typeInt64:
		return	*((int64_t*)m_ptrBuffer) != 0 ;
	case	typeFloat32:
		return	*((float32_t*)m_ptrBuffer) != 0 ;
	case	typeFloat64:
		return	*((float64_t*)m_ptrBuffer) != 0 ;
	default:
		break ;
	}
	return	false ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::AsString( SSystem::SString& strValue ) const
{
	if ( m_type < typeFloat32 )
	{
		int64_t	num ;
		if ( AsInteger( num ) )
		{
			strValue.FromInteger( num ) ;
			return	true ;
		}
	}
	else
	{
		double	num ;
		if ( AsRealNumber( num ) )
		{
			strValue.FromReal( num ) ;
			return	true ;
		}
	}
	return	false ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceNumber::IsEqualObject( RSObject * pObj ) const
{
	if ( pObj == NULL )
	{
		return	(m_pRef == NULL) ;
	}
	if ( pObj->IsIntegerType() )
	{
		if ( m_type < typeFloat32 )
		{
			int64_t	num1, num2 ;
			if ( AsInteger( num1 ) && pObj->AsInteger( num2 ) )
			{
				return	(num1 == num2) ;
			}
		}
	}
	else
	{
		if ( m_type >= typeFloat32 )
		{
			double	num1, num2 ;
			if ( AsRealNumber( num1 ) && pObj->AsRealNumber( num2 ) )
			{
				return	(num1 == num2) ;
			}
		}
	}
	return	false ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSReferenceNumber::DisposeObject( RSContext& context )
{
	context.ReleaseObjectRef( m_pRef ) ;
	m_pRef = NULL ;
	m_ptrBuffer = NULL ;
	//
	RSObject::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceNumber::DuplicateObject( RSContext& context ) const
{
	if ( m_pRef != NULL )
	{
		m_pRef->AddRef() ;
		return	new RSReferenceNumber
						( m_pClass, m_ptrBuffer, m_type, m_pRef ) ;
	}
	return	NULL ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceNumber::CloneObject( RSContext& context ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( m_type < typeFloat32 )
		{
			return	context.new_Integer( LoadInteger() ) ;
		}
		else
		{
			return	context.new_Number( LoadRealNumber() ) ;
		}
	}
	return	NULL ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceNumber::OperatorPlus( RSContext& context ) const
{
	return	CloneObject( context ) ;
}

RSObject * RSReferenceNumber::OperatorNegate( RSContext& context ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( m_type < typeFloat32 )
		{
			return	context.new_Integer( - LoadInteger() ) ;
		}
		else
		{
			return	context.new_Number( - LoadRealNumber() ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorBitNot( RSContext& context ) const
{
	if ( m_ptrBuffer != NULL )
	{
		return	context.new_Integer( ~ LoadInteger() ) ;
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorIncrement( RSContext& context )
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num = LoadInteger() + 1 ;
		StoreInteger( num ) ;
		return	context.new_Integer( num ) ;
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorDecrement( RSContext& context )
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num = LoadInteger() - 1 ;
		StoreInteger( num ) ;
		return	context.new_Integer( num ) ;
	}
	return	NULL ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceNumber::OperatorMul( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Integer( LoadInteger() * num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Number( LoadRealNumber() * num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorDiv( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				if ( num2 != 0 )
				{
					return	context.new_Integer( LoadInteger() / num2 ) ;
				}
				context.SetException( context.new_Exception( L"零除算エラー" ) ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Number( LoadRealNumber() / num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorMod( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num2 ;
		if ( pObj->AsInteger( num2 ) )
		{
			if ( num2 != 0 )
			{
				return	context.new_Integer( LoadInteger() % num2 ) ;
			}
			context.SetException( context.new_Exception( L"零除算エラー" ) ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Integer( LoadInteger() + num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Number( LoadRealNumber() + num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Integer( LoadInteger() - num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Number( LoadRealNumber() - num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorShiftLeft( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num2 ;
		if ( pObj->AsInteger( num2 ) )
		{
			return	context.new_Integer( LoadInteger() << num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorShiftRight( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num2 ;
		if ( pObj->AsInteger( num2 ) )
		{
			return	context.new_Integer( LoadInteger() >> num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num2 ;
		if ( pObj->AsInteger( num2 ) )
		{
			return	context.new_Integer( (uint64_t) LoadInteger() >> num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorBitAnd( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num2 ;
		if ( pObj->AsInteger( num2 ) )
		{
			return	context.new_Integer( LoadInteger() & num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorBitOr( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num2 ;
		if ( pObj->AsInteger( num2 ) )
		{
			return	context.new_Integer( LoadInteger() | num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorBitXor( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		int64_t	num2 ;
		if ( pObj->AsInteger( num2 ) )
		{
			return	context.new_Integer( LoadInteger() ^ num2 ) ;
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Boolean( LoadInteger() == num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Boolean( LoadRealNumber() == num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Boolean( LoadInteger() != num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Boolean( LoadRealNumber() != num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Boolean( LoadInteger() >= num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Boolean( LoadRealNumber() >= num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Boolean( LoadInteger() > num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Boolean( LoadRealNumber() > num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Boolean( LoadInteger() <= num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Boolean( LoadRealNumber() <= num2 ) ;
			}
		}
	}
	return	NULL ;
}

RSObject * RSReferenceNumber::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	if ( m_ptrBuffer != NULL )
	{
		if ( (m_type < typeFloat32) && pObj->IsIntegerType() )
		{
			int64_t	num2 ;
			if ( pObj->AsInteger( num2 ) )
			{
				return	context.new_Boolean( LoadInteger() < num2 ) ;
			}
		}
		else
		{
			double	num2 ;
			if ( pObj->AsRealNumber( num2 ) )
			{
				return	context.new_Boolean( LoadRealNumber() < num2 ) ;
			}
		}
	}
	return	NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceNumber::OperatorMove( RSContext& context, RSObject * pObj )
{
	if ( m_ptrBuffer == NULL )
	{
		context.ThrowExceptionError
			( L"ヌルポインタへの代入です", L"NullPointerException" ) ;
		return	NULL ;
	}
	if ( m_type < typeFloat32 )
	{
		int64_t	num ;
		if ( pObj->AsInteger( num ) )
		{
			StoreInteger( num ) ;
			AddRef() ;
			return	this ;
		}
	}
	else
	{
		double	num ;
		if ( pObj->AsRealNumber( num ) )
		{
			StoreRealNumber( num ) ;
			AddRef() ;
			return	this ;
		}
	}
	context.ThrowExceptionError
		( L"バッファへ書き込めないオブジェクトです" ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// 型付配列（ポインタ）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSTypedArrayPointer, RSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSTypedArrayPointer::RSTypedArrayPointer
	( RSClass * pClass,
		RSArrayBuffer * pBuf,
		RSReferenceNumber::NumberType type,
		size_t iOffset, ssize_t nLimit )
	: RSObject( pClass, typePointerNumber ),
		m_typeElement( type ),
		m_pRefBuffer( pBuf ), m_iOffset(0), m_nLimit(0)
{
	m_nElementBytes = RSReferenceNumber::GetNumberSizeOf( type ) ;
	//
	if ( (pBuf != NULL) && (iOffset < pBuf->m_lenBuf) )
	{
		m_iOffset = iOffset ;
		m_nLimit = pBuf->m_lenBuf - iOffset ;
		if ( (nLimit >= 0) && ((size_t) nLimit < m_nLimit) )
		{
			m_nLimit = (size_t) nLimit ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSTypedArrayPointer::~RSTypedArrayPointer( void )
{
	RSObject::ReleaseRef( m_pRefBuffer ) ;
	m_pRefBuffer = NULL ;
}

// ポインタ設定
//////////////////////////////////////////////////////////////////////////////
void RSTypedArrayPointer::SetPointer
	( RSArrayBuffer * pBuf,
		RSReferenceNumber::NumberType type,
		size_t iOffset, ssize_t nLimit )
{
	RSObject::ReleaseRef( m_pRefBuffer ) ;
	m_pRefBuffer = NULL ;
	//
	m_pRefBuffer = pBuf ;
	m_typeElement = type ;
	m_nElementBytes = RSReferenceNumber::GetNumberSizeOf( type ) ;
	m_iOffset = 0 ;
	m_nLimit = 0 ;
	//
	if ( (pBuf != NULL) && (iOffset < pBuf->m_lenBuf) )
	{
		m_iOffset = iOffset ;
		m_nLimit = pBuf->m_lenBuf - iOffset ;
		if ( (nLimit >= 0) && ((size_t) nLimit < m_nLimit) )
		{
			m_nLimit = (size_t) nLimit ;
		}
	}
}

// ポインタ取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * RSTypedArrayPointer::GetPointer( void ) const
{
	if ( m_pRefBuffer != NULL )
	{
		if ( m_pRefBuffer->m_ptrBuf != NULL )
		{
			return	m_pRefBuffer->m_ptrBuf + m_iOffset ;
		}
	}
	return	NULL ;
}

uint8_t * RSTypedArrayPointer::GetPointer( size_t nReqBytes ) const
{
	if ( nReqBytes > m_nLimit )
	{
		return	NULL ;
	}
	if ( m_pRefBuffer != NULL )
	{
		if ( m_pRefBuffer->m_ptrBuf != NULL )
		{
			return	m_pRefBuffer->m_ptrBuf + m_iOffset ;
		}
	}
	return	NULL ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSTypedArrayPointer::GetTypeName( void ) const
{
	if ( m_pClass != NULL )
	{
		return	m_pClass->GetRSClassName() ;
	}
	return	L"TypedPointer" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::InstanceOf( const wchar_t * pwszType )
{
	if ( m_pClass != NULL )
	{
		if ( m_pClass->IsInstanceOf( pwszType ) )
		{
			return	this ;
		}
	}
	return	NULL ;
}

RSObject * RSTypedArrayPointer::InstanceOf( RSClass * pClass )
{
	if ( m_pClass != NULL )
	{
		if ( pClass == m_pClass )
		{
			return	this ;
		}
		if ( m_pClass->IsInstanceOf( pClass ) )
		{
			return	this ;
		}
	}
	return	NULL ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSTypedArrayPointer::IsObjectType( void ) const
{
	return	true ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSTypedArrayPointer::AsBoolean( void ) const
{
	return	(m_pRefBuffer != NULL) ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSTypedArrayPointer::AsString( SSystem::SString& strValue ) const
{
	if ( m_pRefBuffer != NULL )
	{
		SString	strBuf ;
		if ( m_pRefBuffer->AsString( strBuf ) )
		{
			static const wchar_t *	pwszTypeName[] =
			{
				L"uint8", L"int8", L"uint16", L"int16",
				L"uint32", L"int32", L"int64", L"float32", L"float64",
				L"<object>"
			} ;
			strValue = L"pointer:{" ;
			strValue += strBuf ;
			strValue += L",offset:" ;
			strValue += SString( m_iOffset ) ;
			strValue += L",length:" ;
			strValue += SString( m_nLimit ) ;
			strValue += L",type:" ;
			if ( (GetRSClass() != NULL)
				&& (m_typeElement == RSReferenceNumber::typeObject) )
			{
				strValue += GetRSClass()->GetRSClassName() ;
			}
			else
			{
				strValue += pwszTypeName[m_typeElement] ;
			}
			strValue += L"}" ;
			return	true ;
		}
	}
	else
	{
		strValue = L"null" ;
		return	true ;
	}
	return	false ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSTypedArrayPointer::IsEqualObject( RSObject * pObj ) const
{
	RSTypedArrayPointer *
		pTypedPtr = ESLTypeCast<RSTypedArrayPointer>( pObj ) ;
	if ( pTypedPtr != NULL )
	{
		return	(m_pRefBuffer == pTypedPtr->m_pRefBuffer)
				&& (m_typeElement == pTypedPtr->m_typeElement)
				&& (m_iOffset == pTypedPtr->m_iOffset)
				&& (m_nLimit == pTypedPtr->m_nLimit) ;
	}
	RSPointer *	pPtr = ESLTypeCast<RSPointer>( pObj ) ;
	if ( pPtr != NULL )
	{
		if ( pPtr->m_pRef == NULL )
		{
			return	(m_pRefBuffer == NULL) ;
		}
	}
	if ( pObj == NULL )
	{
		return	(m_pRefBuffer == NULL) ;
	}
	return	false ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::GetElementAt
	( RSContext& context, int nIndex ) const
{
	if ( (nIndex >= 0) && ((nIndex + 1) * m_nElementBytes <= m_nLimit) )
	{
		RSArrayBuffer *	pRef = m_pRefBuffer ;
		ESLAssert( pRef != NULL ) ;
		pRef->AddRef() ;
		return	context.new_ReferenceNumber
			( pRef->m_ptrBuf
				+ m_iOffset
				+ (nIndex * m_nElementBytes), m_typeElement, pRef ) ;
	}
	context.ThrowExceptionError
		( L"指標が範囲外です", L"IndexOutOfBoundsException" ) ;
	return	NULL ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::SetElementAt
	( RSContext& context, int nIndex, RSObject * pObj )
{
	RSObject *	pResult = NULL ;
	RSObject *	pElement = GetElementAt( context, nIndex ) ;
	if ( pElement != NULL )
	{
		pResult = pElement->OperatorMove( context, pObj ) ;
	}
	context.ReleaseObjectRef( pObj ) ;
	return	pResult ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSTypedArrayPointer::GetElementCount( void ) const
{
	return	m_nLimit / m_nElementBytes ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSTypedArrayPointer::GetElementLimit( void ) const
{
	return	m_nLimit / m_nElementBytes ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSTypedArrayPointer::DisposeObject( RSContext& context )
{
	context.ReleaseObjectRef( m_pRefBuffer ) ;
	m_pRefBuffer = NULL ;
	m_iOffset = 0 ;
	m_nLimit = 0 ;
	//
	RSObject::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::DuplicateObject( RSContext& context ) const
{
	RSObject::AddRef( m_pRefBuffer ) ;
	return	context.new_PointerNumber
		( m_pRefBuffer, m_typeElement, m_iOffset, (ssize_t) m_nLimit ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::CloneObject( RSContext& context ) const
{
	RSObject::AddRef( m_pRefBuffer ) ;
	return	context.new_PointerNumber
		( m_pRefBuffer, m_typeElement, m_iOffset, (ssize_t) m_nLimit ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::OperatorIncrement( RSContext& context )
{
	m_iOffset += m_nElementBytes ;
	if ( m_nLimit > m_nElementBytes )
	{
		m_nLimit -= m_nElementBytes ;
	}
	else
	{
		m_nLimit = 0 ;
	}
	AddRef() ;
	return	this ;
}

RSObject * RSTypedArrayPointer::OperatorDecrement( RSContext& context )
{
	if ( m_iOffset > m_nElementBytes )
	{
		m_iOffset -= m_nElementBytes ;
		m_nLimit += m_nElementBytes ;
	}
	else
	{
		m_nLimit += m_iOffset ;
		m_iOffset = 0 ;
	}
	AddRef() ;
	return	this ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	int64_t	num ;
	if ( pObj->AsInteger( num ) )
	{
		size_t	i = (size_t) num * m_nElementBytes ;
		if ( i <= m_nLimit )
		{
			RSObject::AddRef( m_pRefBuffer ) ;
			return	context.new_PointerNumber
				( m_pRefBuffer, m_typeElement,
					m_iOffset + i, (ssize_t) (m_nLimit - i) ) ;
		}
	}
	return	NULL ;
}

RSObject * RSTypedArrayPointer::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	int64_t	num ;
	if ( pObj->AsInteger( num ) )
	{
		size_t	i = (size_t) num * m_nElementBytes ;
		if ( i <= m_iOffset )
		{
			RSObject::AddRef( m_pRefBuffer ) ;
			return	context.new_PointerNumber
				( m_pRefBuffer, m_typeElement,
					m_iOffset - i, (ssize_t) (m_nLimit + i) ) ;
		}
	}
	return	NULL ;
}

RSObject * RSTypedArrayPointer::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	return	context.new_Boolean( IsEqualObject( pObj ) ) ;
}

RSObject * RSTypedArrayPointer::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	return	context.new_Boolean( !IsEqualObject( pObj ) ) ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointer::OperatorMove( RSContext& context, RSObject * pObj )
{
	RSTypedArrayPointer *	pTypedPtr =
		ESLTypeCast<RSTypedArrayPointer>
			( (pObj != NULL) ? pObj->GetEntityObject() : NULL ) ;
	if ( pTypedPtr != NULL )
	{
		if ( (m_typeElement != RSReferenceNumber::typeUint8)
				&& (m_typeElement != pTypedPtr->m_typeElement) )
		{
			context.ThrowExceptionError( L"ポインタ型が一致しません" ) ;
			return	NULL ;
		}
		RSObject::AddRef( pTypedPtr->m_pRefBuffer ) ;
		SetPointer
			( pTypedPtr->m_pRefBuffer,
				pTypedPtr->m_typeElement,
				pTypedPtr->m_iOffset, (ssize_t) pTypedPtr->m_nLimit ) ;
		if ( pTypedPtr->m_iOffset & (m_nElementBytes - 1) )
		{
			context.ThrowExceptionError( L"アライメントエラー" ) ;
		}
		AddRef() ;
		return	this ;
	}
	RSArrayBuffer *	pBuf = ESLTypeCast<RSArrayBuffer>( pObj ) ;
	if ( pBuf != NULL )
	{
		pBuf->AddRef() ;
		SetPointer( pBuf, m_typeElement, 0, -1 ) ;
		AddRef() ;
		return	this ;
	}
	RSPointer *	pPtr = ESLTypeCast<RSPointer>( pObj ) ;
	if ( pPtr != NULL )
	{
		if ( pPtr->m_pRef == NULL )
		{
			SetPointer( NULL, m_typeElement, 0, -1 ) ;
			AddRef() ;
			return	this ;
		}
	}
	if ( pObj == NULL )
	{
		SetPointer( NULL, m_typeElement, 0, -1 ) ;
		AddRef() ;
		return	this ;
	}
	context.ThrowExceptionError( L"ポインタに変換できません" ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// 構造体ポインタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSStructuredPointer, RSTypedArrayPointer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSStructuredPointer::RSStructuredPointer
	( RSStructuredPointerClass * pType,
		RSArrayBuffer * pBuf, size_t iOffset, ssize_t nLimit )
: RSTypedArrayPointer
	( pType, pBuf, RSReferenceNumber::typeObject, iOffset, nLimit )
{
	m_typeObj = typeStrucuredPointer ;
	m_nElementBytes = pType->GetStructureBytes() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSStructuredPointer::~RSStructuredPointer( void )
{
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSStructuredPointer::IsEqualObject( RSObject * pObj ) const
{
	RSStructuredPointer *
		pStructPtr = ESLTypeCast<RSStructuredPointer>( pObj ) ;
	if ( pStructPtr != NULL )
	{
		return	(m_pRefBuffer == pStructPtr->m_pRefBuffer)
				&& (GetRSClass() == pStructPtr->GetRSClass())
				&& (m_iOffset == pStructPtr->m_iOffset)
				&& (m_nLimit == pStructPtr->m_nLimit) ;
	}
	RSPointer *	pPtr = ESLTypeCast<RSPointer>( pObj ) ;
	if ( pPtr != NULL )
	{
		if ( pPtr->m_pRef == NULL )
		{
			return	(m_pRefBuffer == NULL) ;
		}
	}
	if ( pObj == NULL )
	{
		return	(m_pRefBuffer == NULL) ;
	}
	return	false ;
}

// ポインタ設定
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointer::SetPointer
	( RSArrayBuffer * pBuf,
		RSStructuredPointerClass * pType, size_t iOffset, ssize_t nLimit )
{
	RSTypedArrayPointer::SetPointer
		( pBuf, RSReferenceNumber::typeObject, iOffset, nLimit ) ; 
	SetRSClass( pType ) ;
	m_nElementBytes = pType->GetStructureBytes() ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::GetElementAt
	( RSContext& context, int nIndex ) const
{
	RSStructuredPointerClass *
			pType = (RSStructuredPointerClass*) GetRSClass() ;
	ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
	size_t	nStructBytes = pType->GetStructureBytes() ;
	//
	if ( (nIndex >= 0) && ((nIndex + 1) * nStructBytes <= m_nLimit) )
	{
		RSArrayBuffer *	pRef = m_pRefBuffer ;
		RSObject::AddRef( pRef ) ;
		return	context.new_StructuredPointer
			( pType, pRef,
				m_iOffset + (nIndex * nStructBytes),
				(ssize_t) nStructBytes ) ;
	}
	context.ThrowExceptionError
		( L"指標が範囲外です", L"IndexOutOfBoundsException" ) ;
	return	NULL ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::SetElementAt
	( RSContext& context, int nIndex, RSObject * pObj )
{
	context.ThrowExceptionError( L"構造体配列への代入操作は無効です" ) ;
	return	NULL ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSStructuredPointer::GetElementCount( void ) const
{
	RSStructuredPointerClass *
			pType = (RSStructuredPointerClass*) GetRSClass() ;
	ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
	size_t	nStructBytes = pType->GetStructureBytes() ;
	if ( nStructBytes == 0 )
	{
		return	0x7FFFFFFF ;
	}
	return	m_nLimit / nStructBytes ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSStructuredPointer::GetElementLimit( void ) const
{
	return	RSStructuredPointer::GetElementCount() ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	RSStructuredPointerClass *
			pType = (RSStructuredPointerClass*) GetRSClass() ;
	ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
	//
	RSStructuredPointerClass::ElementInfo *
					pei = pType->GetArrayMemberAs( pwszName ) ;
	if ( pei == NULL )
	{
		return	NULL ;
	}
	if ( m_nLimit < pType->GetStructureBytes() )
	{
		context.ThrowExceptionError
			( L"不正な構造体ポインタのメンバ参照です" ) ;
		return	NULL ;
	}
	if ( pei->m_type != RSReferenceNumber::typeObject )
	{
		RSArrayBuffer *	pRef = m_pRefBuffer ;
		RSObject::AddRef( pRef ) ;
		//
		RSObject *	pMember =
			context.new_ReferenceNumber
				( pRef->m_ptrBuf
					+ (m_iOffset + pei->m_iOffset), pei->m_type, pRef ) ;
		pMember->SetModifiers( pei->m_accMod ) ;
		pMember->SetDefinitionComment( pei->m_pComment ) ;
		return	pMember ;
	}
	RSStructuredPointerClass *
		pStruct = ESLTypeCast<RSStructuredPointerClass>( pei->m_pClass ) ;
	if ( pStruct != NULL )
	{
		RSArrayBuffer *	pRef = m_pRefBuffer ;
		RSObject::AddRef( pRef ) ;
		//
		RSObject *	pMember =
			context.new_StructuredPointer
				( pStruct, pRef,
					(m_iOffset + pei->m_iOffset),
					(ssize_t) pei->m_nBytes ) ;
		pMember->SetModifiers( pei->m_accMod ) ;
		pMember->SetDefinitionComment( pei->m_pComment ) ;
		return	pMember ;
	}
	RSTypedArrayPointerClass *
		pNumPtr = ESLTypeCast<RSTypedArrayPointerClass>( pei->m_pClass ) ;
	if ( pNumPtr != NULL )
	{
		RSArrayBuffer *	pRef = m_pRefBuffer ;
		RSObject::AddRef( pRef ) ;
		//
		RSObject *	pMember =
			context.new_PointerNumber
				( pRef, pNumPtr->m_typeElement,
					(m_iOffset + pei->m_iOffset),
					(ssize_t) pei->m_nBytes ) ;
		pMember->SetModifiers( pei->m_accMod ) ;
		pMember->SetDefinitionComment( pei->m_pComment ) ;
		return	pMember ;
	}
	return	NULL ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	RSObject *	pMember = GetMemberAs( context, pwszName ) ;
	if ( pMember != NULL )
	{
		if ( pObj != NULL )
		{
			context.ReleaseObjectRef
				( pMember->OperatorMove( context, pObj ) ) ;
			context.ReleaseObjectRef( pObj ) ;
		}
		else
		{
			context.ThrowExceptionError
				( L"null を代入しようとしています" ) ;
		}
		return	pMember ;
	}
	else
	{
		context.ThrowExceptionError
			( SString(pwszName) + L" は未定義のメンバです" ) ;
	}
	return	NULL ;
}

// 要素名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSStructuredPointer::GetMemberNameAt( int nIndex ) const
{
	RSStructuredPointerClass *
			pType = (RSStructuredPointerClass*) GetRSClass() ;
	ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
	//
	return	pType->GetArrayMemberNameAt( nIndex ) ;
}

// メンバ数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSStructuredPointer::GetMemberCount( void ) const
{
	RSStructuredPointerClass *
			pType = (RSStructuredPointerClass*) GetRSClass() ;
	ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
	//
	return	pType->GetArrayMemberCount() ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointer::DisposeObject( RSContext& context )
{
	RSTypedArrayPointer::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::DuplicateObject( RSContext& context ) const
{
	RSStructuredPointerClass *
			pType = (RSStructuredPointerClass*) GetRSClass() ;
	ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
	//
	RSArrayBuffer *	pRef = m_pRefBuffer ;
	RSObject::AddRef( pRef ) ;
	return	context.new_StructuredPointer
					( pType, pRef, m_iOffset, (ssize_t) m_nLimit ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::CloneObject( RSContext& context ) const
{
	RSStructuredPointerClass *
			pType = (RSStructuredPointerClass*) GetRSClass() ;
	ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
	//
	RSArrayBuffer *	pRef = m_pRefBuffer ;
	RSArrayBuffer *	pBuf = NULL ;
	if ( pRef != NULL )
	{
		pBuf = new RSArrayBuffer( pRef->GetRSClass() ) ;
		pBuf->AllocateBuffer( pRef->m_lenBuf ) ;
		eslMoveMemory( pBuf->m_ptrBuf, pRef->m_ptrBuf, pRef->m_lenBuf ) ;
	}
	return	context.new_StructuredPointer
					( pType, pBuf, m_iOffset, (ssize_t) m_nLimit ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::OperatorIncrement( RSContext& context )
{
	m_iOffset += m_nElementBytes ;
	if ( m_nLimit > m_nElementBytes )
	{
		m_nLimit -= m_nElementBytes ;
	}
	else
	{
		m_nLimit = 0 ;
	}
	AddRef() ;
	return	this ;
}

RSObject * RSStructuredPointer::OperatorDecrement( RSContext& context )
{
	if ( m_iOffset > m_nElementBytes )
	{
		m_iOffset -= m_nElementBytes ;
		m_nLimit += m_nElementBytes ;
	}
	else
	{
		m_nLimit += m_iOffset ;
		m_iOffset = 0 ;
	}
	AddRef() ;
	return	this ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	int64_t	num ;
	if ( pObj->AsInteger( num ) )
	{
		size_t	i = (size_t) num * m_nElementBytes ;
		if ( i <= m_nLimit )
		{
			RSStructuredPointerClass *
					pType = (RSStructuredPointerClass*) GetRSClass() ;
			ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
			//
			RSArrayBuffer *	pRef = m_pRefBuffer ;
			RSObject::AddRef( pRef ) ;
			return	context.new_StructuredPointer
				( pType, pRef, m_iOffset + i, (ssize_t) (m_nLimit - i) ) ;
		}
	}
	return	NULL ;
}

RSObject * RSStructuredPointer::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	int64_t	num ;
	if ( pObj->AsInteger( num ) )
	{
		size_t	i = (size_t) num * m_nElementBytes ;
		if ( i <= m_iOffset )
		{
			RSStructuredPointerClass *
					pType = (RSStructuredPointerClass*) GetRSClass() ;
			ESLAssert( pType->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
			//
			RSArrayBuffer *	pRef = m_pRefBuffer ;
			RSObject::AddRef( pRef ) ;
			return	context.new_StructuredPointer
				( pType, pRef, m_iOffset - i, (ssize_t) (m_nLimit + i) ) ;
		}
	}
	return	NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointer::OperatorMove( RSContext& context, RSObject * pObj )
{
	RSStructuredPointer *	pStructPtr =
		ESLTypeCast<RSStructuredPointer>
			( (pObj != NULL) ? pObj->GetEntityObject() : NULL ) ;
	if ( pStructPtr != NULL )
	{
		RSStructuredPointerClass *	pStructType =
					(RSStructuredPointerClass*) pStructPtr->GetRSClass() ;
		ESLAssert( pStructType->IsKindOf
						( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
		RSStructuredPointerClass *	pThisType =
					(RSStructuredPointerClass*) GetRSClass() ;
		ESLAssert( pThisType->IsKindOf
						( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
		ssize_t	iCastOffset = pStructType->OffsetSuperStruct( pThisType ) ;
		if ( iCastOffset < 0 )
		{
			context.ThrowExceptionError( L"ポインタ型を変換できません" ) ;
			return	NULL ;
		}
		if ( (pStructPtr->m_pRefBuffer == NULL)
			|| (pStructPtr->m_nLimit < pStructType->GetStructureBytes()) )
		{
			SetPointer( NULL, pThisType, 0, -1 ) ;
			AddRef() ;
			return	this ;
		}
		RSObject::AddRef( pStructPtr->m_pRefBuffer ) ;
		if ( pStructType != pThisType )
		{
			SetPointer
				( pStructPtr->m_pRefBuffer,
					pThisType,
					pStructPtr->m_iOffset + iCastOffset,
					(ssize_t) pThisType->GetStructureBytes() ) ;
		}
		else
		{
			SetPointer
				( pStructPtr->m_pRefBuffer,
					pThisType,
					pStructPtr->m_iOffset + iCastOffset,
					(ssize_t) pStructPtr->m_nLimit - iCastOffset ) ;
		}
		AddRef() ;
		return	this ;
	}
	RSPointer *	pPtr = ESLTypeCast<RSPointer>( pObj ) ;
	if ( pPtr != NULL )
	{
		if ( pPtr->m_pRef == NULL )
		{
			RSStructuredPointerClass *	pThisType =
							(RSStructuredPointerClass*) GetRSClass() ;
			ESLAssert( pThisType->IsKindOf
							( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
			SetPointer( NULL, pThisType, 0, -1 ) ;
			AddRef() ;
			return	this ;
		}
	}
	if ( pObj == NULL )
	{
		RSStructuredPointerClass *	pThisType =
						(RSStructuredPointerClass*) GetRSClass() ;
		ESLAssert( pThisType->IsKindOf
						( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) ) ;
		SetPointer( NULL, pThisType, 0, -1 ) ;
		AddRef() ;
		return	this ;
	}
	context.ThrowExceptionError( L"ポインタに変換できません" ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// ArrayBuffer 型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSArrayBufferClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSArrayBufferClass::RSArrayBufferClass( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSArrayBufferClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSArrayBuffer( this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"int length",
				NULL, &RSArrayBufferClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"length", L"int", L"",
				NULL, &RSArrayBufferClass::method_length,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"memset", NULL, L"Uint8Pointer ptrDst, byte fill, int nBytes",
				NULL, &RSArrayBufferClass::method_memset, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"memmove", NULL, L"Uint8Pointer ptrDst, Uint8Pointer ptrSrc, int nBytes",
				NULL, &RSArrayBufferClass::method_memmove, NULL ) ;
}

// void <init>( int length )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayBufferClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArrayBuffer *	pObj = ESLTypeCast<RSArrayBuffer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"ArrayBuffer 構築関数の this が ArrayBuffer ではありません" ) ;
		return	NULL ;
	}
	pObj->AllocateBuffer( (size_t) arg.IntAt(0) ) ;
	return	NULL ;
}

// int length()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayBufferClass::method_length
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArrayBuffer *	pObj = ESLTypeCast<RSArrayBuffer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"ArrayBuffer.length 関数の this が ArrayBuffer ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObj->m_lenBuf ) ;
}

// static void memset( ptrDst, byte fill, int nBytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayBufferClass::method_memset
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSTypedArrayPointer *
		ptrDst = ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt(0) ) ;
	if ( ptrDst == NULL )
	{
		context.ThrowExceptionError
			( L"ArrayBuffer.memset 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	int		fill = arg.IntAt( 1 ) ;
	size_t	nBytes = (size_t) arg.IntAt( 2 ) ;
	if ( ptrDst->m_nLimit < nBytes )
	{
		nBytes = ptrDst->m_nLimit ;
	}
	RSArrayBuffer *	pBufDst = ptrDst->m_pRefBuffer ;
	if ( (pBufDst != NULL) && (pBufDst->m_ptrBuf != NULL) && (nBytes > 0) )
	{
		memset( pBufDst->m_ptrBuf, fill, nBytes ) ;
	}
	return	NULL ;
}

// static void memmove( ptrDst, ptrSrc, int nBytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayBufferClass::method_memmove
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSTypedArrayPointer *
		ptrDst = ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt(0) ) ;
	RSTypedArrayPointer *
		ptrSrc = ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt(1) ) ;
	if ( (ptrDst == NULL) || (ptrSrc == NULL) )
	{
		context.ThrowExceptionError
			( L"ArrayBuffer.memmove 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	size_t	nBytes = (size_t) arg.IntAt( 2 ) ;
	if ( ptrDst->m_nLimit < nBytes )
	{
		nBytes = ptrDst->m_nLimit ;
	}
	if ( ptrSrc->m_nLimit < nBytes )
	{
		nBytes = ptrSrc->m_nLimit ;
	}
	RSArrayBuffer *	pBufDst = ptrDst->m_pRefBuffer ;
	RSArrayBuffer *	pBufSrc = ptrSrc->m_pRefBuffer ;
	if ( (pBufDst != NULL) && (pBufDst->m_ptrBuf != NULL)
		&& (pBufSrc != NULL) && (pBufSrc->m_ptrBuf != NULL) && (nBytes > 0) )
	{
		memmove( pBufDst->m_ptrBuf, pBufSrc->m_ptrBuf, nBytes ) ;
	}
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// 型付配列（ポインタ）型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSTypedArrayPointerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSTypedArrayPointerClass::RSTypedArrayPointerClass
	( RSClass * pClass,
		const wchar_t * pwszClassName,
		RSReferenceNumber::NumberType type )
	: RSClass( pClass, pwszClassName )
{
	m_typeElement = type ;
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSTypedArrayPointerClass::Initialize( RSContext& context )
{
	if ( m_typeElement == RSReferenceNumber::typeUint8 )
	{
		AddSuperClass( context, context.GetGenericObjectClass() ) ;
	}
	else
	{
		AddSuperClass
			( context,
				context.GetTypedPointerClass
						( RSReferenceNumber::typeUint8 ) ) ;
	}
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSTypedArrayPointerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSTypedArrayPointer( this, NULL, m_typeElement ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"int length",
				NULL, &RSTypedArrayPointerClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
			NULL, L"Uint8Pointer ptr",
			NULL, &RSTypedArrayPointerClass::method_init2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
			NULL, L"ArrayBuffer buf, int iOffset = 0, int nLength = -1",
			NULL, &RSTypedArrayPointerClass::method_init3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBuffer", L"ArrayBuffer", L"",
			NULL, &RSTypedArrayPointerClass::method_getBuffer,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getOffset", L"int", L"",
			NULL, &RSTypedArrayPointerClass::method_getOffset,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBytes", L"int", L"",
			NULL, &RSTypedArrayPointerClass::method_getBytes,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"length", L"int", L"",
			NULL, &RSTypedArrayPointerClass::method_length,
			NULL, RSFunctionPrototype::flagConstant ) ;
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::NewVariable( RSContext& context )
{
	return	context.new_PointerNumber( NULL, m_typeElement ) ;
}

// ポインタ要素サイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t RSTypedArrayPointerClass::GetElementBytes( void ) const
{
	return	RSReferenceNumber::GetNumberSizeOf( m_typeElement ) ;
}

// アライメントサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t RSTypedArrayPointerClass::GetAlignment( void ) const
{
	return	RSReferenceNumber::GetNumberSizeOf( m_typeElement ) ;
}

// エレメント型名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSTypedArrayPointerClass::GetElementTypeName( void ) const
{
	return	RSReferenceNumber::GetNumberTypeName( m_typeElement ) ;
}

// void <init>( int length )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSTypedArrayPointer *	pObj = ESLTypeCast<RSTypedArrayPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"TypedArrayPointer 構築関数の this が TypedArrayPointer ではありません" ) ;
		return	NULL ;
	}
	RSArrayBuffer *	pBuf =
			new RSArrayBuffer( context.GetClassAs( L"ArrayBuffer" ) ) ;
	pBuf->AllocateBuffer
		( (size_t) arg.IntAt(0)
				* RSReferenceNumber::GetNumberSizeOf(pObj->m_typeElement) ) ;
	pObj->SetPointer( pBuf, pObj->m_typeElement ) ;
	return	NULL ;
}

// void <init>( Uint8Pointer ptr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSTypedArrayPointer *	pObj = ESLTypeCast<RSTypedArrayPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"TypedArrayPointer 構築関数の this が TypedArrayPointer ではありません" ) ;
		return	NULL ;
	}
	RSTypedArrayPointer *	pPtr = ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt(0) ) ;
	if ( pPtr != NULL )
	{
		RSArrayBuffer *	pBuf = pPtr->m_pRefBuffer ;
		if ( pBuf != NULL )
		{
			pBuf->AddRef() ;
			pObj->SetPointer
				( pBuf, pObj->m_typeElement,
					pPtr->m_iOffset, (ssize_t) pPtr->m_nLimit ) ;
		}
	}
	return	NULL ;
}

// void <init>( ArrayBuffer buf, int iOffset = 0, int nLength = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::method_init3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSTypedArrayPointer *	pObj = ESLTypeCast<RSTypedArrayPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"TypedArrayPointer 構築関数の this が TypedArrayPointer ではありません" ) ;
		return	NULL ;
	}
	RSArrayBuffer *	pBuf = ESLTypeCast<RSArrayBuffer>( arg.ObjectAt(0) ) ;
	RSObject::AddRef( pBuf ) ;
	pObj->SetPointer
		( pBuf, pObj->m_typeElement,
			(size_t) arg.IntAt(1), (ssize_t) arg.IntAt(2,-1) ) ;
	return	NULL ;
}

// ArrayBuffer getBuffer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::method_getBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSTypedArrayPointer *	pObj = ESLTypeCast<RSTypedArrayPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"TypedArrayPointer.getBuffer 関数の this が TypedArrayPointer ではありません" ) ;
		return	NULL ;
	}
	RSArrayBuffer *	pBuf = pObj->m_pRefBuffer ;
	RSObject::AddRef( pBuf ) ;
	return	pBuf ;
}

// int getOffset()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::method_getOffset
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSTypedArrayPointer *	pObj = ESLTypeCast<RSTypedArrayPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"TypedArrayPointer.getBuffer 関数の this が TypedArrayPointer ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObj->m_iOffset ) ;
}

// int getBytes()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::method_getBytes
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSTypedArrayPointer *	pObj = ESLTypeCast<RSTypedArrayPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"TypedArrayPointer.getBytes 関数の this が TypedArrayPointer ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObj->m_nLimit ) ;
}

// int length()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTypedArrayPointerClass::method_length
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSTypedArrayPointer *	pObj = ESLTypeCast<RSTypedArrayPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"TypedArrayPointer.length 関数の this が TypedArrayPointer ではありません" ) ;
		return	NULL ;
	}
	if ( pObj->m_nElementBytes == 0 )
	{
		return	context.new_Integer( 0 ) ;
	}
	return	context.new_Integer( pObj->m_nLimit / pObj->m_nElementBytes ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 構造体型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSStructuredPointerClass, RSTypedArrayPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSStructuredPointerClass::RSStructuredPointerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSTypedArrayPointerClass
		( pClass, pwszClassName, RSReferenceNumber::typeObject )
{
	m_iNextOffset = 0 ;
	m_nBaseAlign = 8 ;
	m_nMaxAlign = 1 ;
}

// ポインタ要素サイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t RSStructuredPointerClass::GetElementBytes( void ) const
{
	return	GetStructureBytes() ;
}

// アライメントサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t RSStructuredPointerClass::GetAlignment( void ) const
{
	return	GetStructureAlign() ;
}

// エレメント型名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSStructuredPointerClass::GetElementTypeName( void ) const
{
	return	GetRSClassName() ;
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointerClass::Initialize( RSContext& context )
{
	if ( m_pSuperClass == NULL )
	{
		AddSuperClass( context, context.GetStructureClass() ) ;
	}
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
}

// クラス定義の完了
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointerClass::FinishClass( RSContext& context )
{
	size_t	nOdd = m_iNextOffset % m_nMaxAlign ;
	if ( nOdd > 0 )
	{
		m_iNextOffset += m_nMaxAlign - nOdd ;
	}
	size_t			nBytes = m_bufInit.GetLength() ;
	const uint8_t *	pbytBuf = m_bufInit.GetConstArray() ;
	bool			fInitValue = false ;
	for ( size_t i = 0; i < nBytes; i ++ )
	{
		if ( pbytBuf[i] != 0 )
		{
			fInitValue = true ;
			break ;
		}
	}
	if ( fInitValue )
	{
		m_bufInit.SetLength( m_iNextOffset ) ;
	}
	else
	{
		m_bufInit.FreeArray() ;
	}
	RSTypedArrayPointerClass::FinishClass( context ) ;
}

// 派生元クラス追加
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointerClass::AddSuperClass( RSContext& context, RSClass * pSuperClass )
{
	RSTypedArrayPointerClass::AddSuperClass( context, pSuperClass ) ;
	//
	RSStructuredPointerClass *
		pStruct = ESLTypeCast<RSStructuredPointerClass>( pSuperClass ) ;
	if ( pStruct != NULL )
	{
		SuperStructCast	ssc ;
		ssc.pStruct = pStruct ;
		ssc.nOffset = 0 ;
		m_lstStructCast.Add( ssc ) ;
		//
		m_ssaElements.DuplicateArray( pStruct->m_ssaElements ) ;
		m_iNextOffset = pStruct->m_iNextOffset ;
		m_nBaseAlign = pStruct->m_nBaseAlign ;
		m_nMaxAlign = pStruct->m_nMaxAlign ;
	}
}

void RSStructuredPointerClass::AddImplementClass( RSContext& context, RSClass * pSuperClass )
{
	RSTypedArrayPointerClass::AddImplementClass( context, pSuperClass ) ;
	//
	RSStructuredPointerClass *
		pStruct = ESLTypeCast<RSStructuredPointerClass>( pSuperClass ) ;
	if ( pStruct != NULL )
	{
		size_t	nOdd = m_iNextOffset % pStruct->m_nMaxAlign ;
		if ( nOdd != 0 )
		{
			m_iNextOffset += pStruct->m_nMaxAlign - nOdd ;
		}
		SuperStructCast	ssc ;
		ssc.pStruct = pStruct ;
		ssc.nOffset = m_iNextOffset ;
		m_lstStructCast.Add( ssc ) ;
		//
		size_t	iBaseOffset = m_iNextOffset ;
		m_iNextOffset += pStruct->m_iNextOffset ;
		if ( m_nMaxAlign < pStruct->m_nMaxAlign )
		{
			m_nMaxAlign = pStruct->m_nMaxAlign ;
		}
		for ( size_t i = 0; i < pStruct->m_ssaElements.GetLength(); i ++ )
		{
			const SString *	pstrName = pStruct->m_ssaElements.GetTagAt( i ) ;
			ElementInfo *	peiMember = pStruct->m_ssaElements.GetAt( i ) ;
			if ( pstrName && peiMember
				&& (m_ssaElements.GetAs( *pstrName ) == NULL) )
			{
				ElementInfo	ei( *peiMember ) ;
				ei.m_iOffset += iBaseOffset ;
				m_ssaElements.Add( *pstrName, ei ) ;
			}
		}
	}
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointerClass::NewVariable( RSContext& context )
{
	return	context.new_StructuredPointer( this, NULL ) ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructuredPointerClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	do
	{
		if ( pObj == NULL )
		{
			break ;
		}
		pObj = pObj->GetEntityObject() ;
		if ( pObj == NULL )
		{
			break ;
		}
		RSStructuredPointer *
				pPtr = ESLTypeCast<RSStructuredPointer>( pObj ) ;
		if ( pPtr == NULL )
		{
			return	NULL ;
		}
		if ( pPtr->m_pRefBuffer == NULL )
		{
			break ;
		}
		RSStructuredPointerClass *	pType =
			ESLTypeCast<RSStructuredPointerClass>( pPtr->GetRSClass() ) ;
		if ( pType == NULL )
		{
			return	NULL ;
		}
		if ( pType == this )
		{
			pObj->AddRef() ;
			return	pObj ;
		}
		if ( pPtr->m_nLimit < pType->GetStructureBytes() )
		{
			break ;
		}
		ssize_t	iCastOffset = pType->OffsetSuperStruct( this ) ;
		if ( iCastOffset >= 0 )
		{
			RSArrayBuffer *	pBuf = pPtr->m_pRefBuffer ;
			RSObject::AddRef( pBuf ) ;
			return	context.new_StructuredPointer
						( this, pBuf,
							pPtr->m_iOffset + iCastOffset,
									(ssize_t) GetStructureBytes() ) ;
		}
		if ( castMethod == RSClass::castForce )
		{
			iCastOffset = OffsetSuperStruct( pType ) ;
			if ( iCastOffset >= 0 )
			{
				if ( (size_t) iCastOffset > pPtr->m_iOffset )
				{
					break ;
				}
				if ( pPtr->m_nLimit + iCastOffset < GetStructureBytes() )
				{
					break ;
				}
				RSArrayBuffer *	pBuf = pPtr->m_pRefBuffer ;
				RSObject::AddRef( pBuf ) ;
				return	context.new_StructuredPointer
							( this, pBuf,
								pPtr->m_iOffset - iCastOffset,
										(ssize_t) GetStructureBytes() ) ;
			}
		}
		return	NULL ;
	}
	while ( false ) ;
	return	context.new_StructuredPointer( this, NULL ) ;
}

// 親構造体キャストオフセット計算
//////////////////////////////////////////////////////////////////////////////
ssize_t RSStructuredPointerClass::OffsetSuperStruct
					( const RSStructuredPointerClass * pStruct ) const
{
	if ( pStruct == this )
	{
		return	0 ;
	}
	const SuperStructCast *	pssc = m_lstStructCast.GetConstArray() ;
	size_t					nCount = m_lstStructCast.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pssc->pStruct == pStruct )
		{
			return	(ssize_t) pssc->nOffset ;
		}
		ssize_t	nOffset = pssc->pStruct->OffsetSuperStruct( pStruct ) ;
		if ( nOffset >= 0 )
		{
			return	(ssize_t) pssc->nOffset + nOffset ;
		}
		pssc ++ ;
	}
	return	-1 ;
}

// クラスメンバダンプ
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointerClass::DumpClassPrototypeMembers
	( RSContext& context, SSystem::SString& strDecl )
{
	RSClass::DumpClassPrototypeMembers( context, strDecl ) ;
	//
	SArray<size_t>	bufIndex ;
	GetOrderedMemberIndex( bufIndex ) ;
	//
	size_t			i ;
	size_t			nCount = m_ssaElements.GetLength() ;
	const size_t *	pIndex = bufIndex.GetConstArray() ;
	//
	for ( i = 0; i < nCount; i ++ )
	{
		const SString *	pstrName = m_ssaElements.GetTagAt( pIndex[i] ) ;
		ElementInfo *	peiMember = m_ssaElements.GetAt( pIndex[i] ) ;
		if ( pstrName && peiMember )
		{
			DumpElementDeclaration( context, strDecl, *pstrName, peiMember ) ;
			strDecl += L" ;\r\n" ;
		}
	}
}

void RSStructuredPointerClass::DumpElementDeclaration
	( RSContext& context, SSystem::SString& strDecl,
		const wchar_t * pwszName, const ElementInfo * peiMember )
{
	SString	strVar = FormatObjectModifiers( peiMember->m_accMod ) ;
	if ( !strVar.IsEmpty() )
	{
		strVar += L" " ;
	}
	RSTypedArrayPointerClass *	pPtrType =
		ESLTypeCast<RSTypedArrayPointerClass>
							( peiMember->m_pClass ) ;
	SString	strArrayOpt ;
	if ( pPtrType != NULL )
	{
		size_t	nElBytes = pPtrType->GetElementBytes() ;
		strVar += pPtrType->GetElementTypeName() ;
		strArrayOpt = L"[" ;
		if ( nElBytes != 0 )
		{
			strArrayOpt += SString
				( (int) (peiMember->m_nBytes / nElBytes) ) ;
		}
		strArrayOpt += L"]" ;
	}
	else
	{
		strVar += peiMember->m_pClass->GetFullClassName() ;
	}
	strVar += L" " ;
	strVar += pwszName ;
	strVar += strArrayOpt ;
	//
	if ( (peiMember->m_type != RSReferenceNumber::typeObject)
		&& (peiMember->m_pInitObj != NULL) )
	{
		SString	strInitValue = RSClass::FormatInitValue( peiMember->m_pInitObj ) ;
		if ( !strInitValue.IsEmpty() )
		{
			strVar += L" = " ;
			strVar += strInitValue ;
		}
	}
	//
	AddFormatIndentedString( strDecl, strVar, L"\t" ) ;
}

// オフセット順のメンバ指標取得
//////////////////////////////////////////////////////////////////////////////
const size_t * RSStructuredPointerClass::GetOrderedMemberIndex( void )
{
	if ( m_aOrderedIndex.GetLength() < m_ssaElements.GetLength() )
	{
		GetOrderedMemberIndex( m_aOrderedIndex ) ;
	}
	return	m_aOrderedIndex.GetConstArray() ;
}

void RSStructuredPointerClass::GetOrderedMemberIndex( SSystem::SArray<size_t>& bufIndex ) const
{
	size_t		i ;
	size_t		nCount = m_ssaElements.GetLength() ;
	size_t *	pIndex = bufIndex.GetArray( nCount ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		pIndex[i] = i ;
	}
	for ( i = 0; i < nCount; i ++ )
	{
		ElementInfo *	peiMember = m_ssaElements.GetAt( pIndex[i] ) ;
		size_t			iMinIndex = i ;
		size_t			iMinOffset = peiMember->m_iOffset ;
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			peiMember = m_ssaElements.GetAt( pIndex[j] ) ;
			if ( iMinOffset > peiMember->m_iOffset )
			{
				iMinIndex = j ;
				iMinOffset = peiMember->m_iOffset ;
			}
		}
		size_t	k = pIndex[i] ;
		pIndex[i] = pIndex[iMinIndex] ;
		pIndex[iMinIndex] = k ;
	}
	bufIndex.FinishArray() ;
}

// メンバ変数情報追加
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointerClass::AddArrayMemberAs
	( RSContext& context,
		const wchar_t * pwszName,
		RSClass * pType, uint32_t accMod,
		size_t nArray, RSObject * pInitObj, RSCodeComment * pComment )
{
	RSReferenceNumber::NumberType	type = RSReferenceNumber::typeObject ;
	RSCodeControl::WordIndex		wiType = RSCodeControl::wiInvalid ;
	for ( int i = RSCodeControl::wiBoolean; i <= RSCodeControl::wiDouble; i ++ )
	{
		if ( context.GetBasicTypeClass
				( (RSCodeControl::WordIndex) i ) == pType )
		{
			wiType = (RSCodeControl::WordIndex) i ;
			break ;
		}
	}
	RSStructuredPointerClass *	pStructClass = NULL ;
	size_t	nElementBytes = 0 ;
	size_t	nAlignBytes = 1 ;
	if ( wiType != RSCodeControl::wiInvalid )
	{
		static const RSReferenceNumber::NumberType	s_typeTable[] =
		{
			RSReferenceNumber::typeUint8,
			RSReferenceNumber::typeInt8,
			RSReferenceNumber::typeInt16,
			RSReferenceNumber::typeUint16,
			RSReferenceNumber::typeInt32,
			RSReferenceNumber::typeInt64,
			RSReferenceNumber::typeFloat32,
			RSReferenceNumber::typeFloat64,
		} ;
		type = s_typeTable[wiType - RSCodeControl::wiBoolean] ;
		nElementBytes = RSReferenceNumber::GetNumberSizeOf( type ) ;
		nAlignBytes = nElementBytes ;
	}
	else
	{
		pStructClass = ESLTypeCast<RSStructuredPointerClass>( pType ) ;
		if ( pStructClass != NULL )
		{
			if ( this == pType )
			{
				context.ThrowExceptionError
						( L"構造体が入れ子になっています" ) ;
			}
			nElementBytes = pStructClass->GetStructureBytes() ;
			nAlignBytes = pStructClass->GetStructureAlign() ;
		}
		else
		{
			nElementBytes = sizeof(void*) ;
			nAlignBytes = nElementBytes ;
		}
	}
	if ( nAlignBytes > m_nBaseAlign )
	{
		nAlignBytes = m_nBaseAlign ;
	}
	size_t	nOdd = m_iNextOffset % nAlignBytes ;
	if ( nOdd > 0 )
	{
		m_iNextOffset += nAlignBytes - nOdd ;
	}
	//
	ElementInfo	ei ;
	ei.m_iOffset = m_iNextOffset ;
	ei.m_nBytes = nElementBytes ;
	ei.m_accMod = accMod ;
	ei.m_pInitObj = pInitObj ;
	ei.m_pComment = pComment ;
	RSObject::AddRef( pInitObj ) ;
	//
	if ( nArray == 0 )
	{
		ei.m_type = type ;
		ei.m_pClass = pType ;
	}
	else
	{
		ei.m_type = RSReferenceNumber::typeObject ;
		ei.m_nBytes *= nArray ;
		//
		if ( type != RSReferenceNumber::typeObject )
		{
			ei.m_pClass = context.GetTypedPointerClass( type ) ;
		}
		else
		{
			ei.m_pClass = pType ;
		}
	}
	m_ssaElements.Add( pwszName, ei ) ;
	//
	m_iNextOffset += ei.m_nBytes ;
	if ( m_nMaxAlign < nAlignBytes )
	{
		m_nMaxAlign = nAlignBytes ;
	}
	//
	if ( pInitObj != NULL )
	{
		m_bufInit.SetLength( m_iNextOffset ) ;
		//
		SetStructureInitValue
			( context, ei.m_iOffset, ei.m_nBytes,
					ei.m_type, ei.m_pClass, pInitObj ) ;
		//
		pInitObj->ReleaseRef() ;
	}
	else if ( (pStructClass != NULL)
			&& (pStructClass->m_bufInit.GetLength()
						>= pStructClass->GetStructureBytes())
			&& (pStructClass->GetStructureBytes() != 0) )
	{
		m_bufInit.SetLength( m_iNextOffset ) ;
		//
		uint8_t *	pbytInit = m_bufInit.GetArray() + ei.m_iOffset ;
		size_t		nLength = ei.m_nBytes / pStructClass->GetStructureBytes() ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			eslMoveMemory
				( pbytInit,
					pStructClass->m_bufInit.GetConstArray(),
					pStructClass->GetStructureBytes() ) ;
			pbytInit += pStructClass->GetStructureBytes() ;
		}
		m_bufInit.FinishArray() ;
	}
}

// 構造体初期値設定
//////////////////////////////////////////////////////////////////////////////
void RSStructuredPointerClass::SetStructureInitValue
	( RSContext& context, size_t iOffset, size_t nBytes,
		RSReferenceNumber::NumberType type,
		RSClass * pClass, RSObject * pInitObj )
{
	if ( m_bufInit.GetLength() < iOffset + nBytes )
	{
		m_bufInit.SetLength( iOffset + nBytes ) ;
	}
	if ( type != RSReferenceNumber::typeObject )
	{
		uint8_t *	pbytInit = m_bufInit.GetArray() + iOffset ;
		int64_t		i64 ;
		double		fp64 ;
		bool		fFailedType = false ;
		switch ( type )
		{
		case	RSReferenceNumber::typeUint8:
			if ( pInitObj->AsInteger( i64 ) )
				*pbytInit = (uint8_t) i64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeInt8:
			if ( pInitObj->AsInteger( i64 ) )
				*((int8_t*)pbytInit) = (int8_t) i64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeUint16:
			if ( pInitObj->AsInteger( i64 ) )
				*((uint16_t*)pbytInit) = (uint16_t) i64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeInt16:
			if ( pInitObj->AsInteger( i64 ) )
				*((int16_t*)pbytInit) = (int16_t) i64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeUint32:
			if ( pInitObj->AsInteger( i64 ) )
				*((uint32_t*)pbytInit) = (uint32_t) i64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeInt32:
			if ( pInitObj->AsInteger( i64 ) )
				*((int32_t*)pbytInit) = (int32_t) i64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeInt64:
			if ( pInitObj->AsInteger( i64 ) )
				*((int64_t*)pbytInit) = i64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeFloat32:
			if ( pInitObj->AsRealNumber( fp64 ) )
				*((float32_t*)pbytInit) = (float32_t) fp64 ;
			else
				fFailedType = true ;
			break ;
		case	RSReferenceNumber::typeFloat64:
			if ( pInitObj->AsRealNumber( fp64 ) )
				*((float64_t*)pbytInit) = fp64 ;
			else
				fFailedType = true ;
			break ;
		default:
			break ;
		}
		m_bufInit.FinishArray() ;
		//
		if ( fFailedType )
		{
			context.ThrowExceptionError
				( L"構造体メンバの不正な初期値です" ) ;
		}
	}
	else
	{
		RSStructuredPointerClass *
			pStruct = ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
		if ( pStruct != NULL )
		{
			SArray<size_t>	bufIndex ;
			GetOrderedMemberIndex( bufIndex ) ;
			for ( size_t i = 0; i < bufIndex.GetLength(); i ++ )
			{
				if ( i >= pInitObj->GetElementCount() )
				{
					break ;
				}
				RSSmartPtr		sptrInit
					( pInitObj->GetElementAt( context, (int) i ), &context ) ;
				ElementInfo *	peiMember =
									m_ssaElements.GetAt( bufIndex.At(i) ) ;
				RSObject *		pInit = sptrInit.GetEntity() ;
				if ( peiMember && pInit )
				{
					SetStructureInitValue
						( context, iOffset + peiMember->m_iOffset,
							peiMember->m_nBytes,
							peiMember->m_type,
							peiMember->m_pClass, pInit ) ;
					if ( context.IsException() )
					{
						return ;
					}
				}
			}
		}
		else
		{
			RSTypedArrayPointerClass *
				pNumPtr = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
			if ( pNumPtr != NULL )
			{
				size_t	nDataBytes =
							RSReferenceNumber::GetNumberSizeOf
										( pNumPtr->m_typeElement ) ;
				size_t	nLength = nBytes / nDataBytes ;
				for ( size_t i = 0; i < nLength; i ++ )
				{
					if ( i >= pInitObj->GetElementCount() )
					{
						break ;
					}
					RSSmartPtr	sptrInit
						( pInitObj->GetElementAt( context, (int) i ), &context ) ;
					RSObject *	pInit = sptrInit.GetEntity() ;
					if ( pInit )
					{
						SetStructureInitValue
							( context, iOffset + nDataBytes * i,
								nDataBytes, pNumPtr->m_typeElement, NULL, pInit ) ;
						if ( context.IsException() )
						{
							return ;
						}
					}
				}
			}
			else
			{
				context.ThrowExceptionError( L"構造体の初期値が不正です" ) ;
			}
		}
	}
}

// メンバ変数情報取得
//////////////////////////////////////////////////////////////////////////////
RSStructuredPointerClass::ElementInfo *
	RSStructuredPointerClass::GetArrayMemberAs( const wchar_t * pwszName ) const
{
	return	m_ssaElements.GetAs( pwszName ) ;
}

RSStructuredPointerClass::ElementInfo *
	RSStructuredPointerClass::GetArrayMemberAt( size_t nIndex ) const
{
	return	m_ssaElements.GetAt( nIndex ) ;
}

//メンバ変数名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSStructuredPointerClass::GetArrayMemberNameAt( size_t nIndex ) const
{
	const SString *	pstrTag = m_ssaElements.GetTagAt( nIndex ) ;
	if ( pstrTag != NULL )
	{
		return	*pstrTag ;
	}
	return	NULL ;
}

// メンバ変数数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSStructuredPointerClass::GetArrayMemberCount( void ) const
{
	return	m_ssaElements.GetLength() ;
}

// バッファ生成
//////////////////////////////////////////////////////////////////////////////
RSArrayBuffer * RSStructuredPointerClass::NewBuffer( RSContext& context, size_t nLength )
{
	RSArrayBuffer *
		pBuf = new RSArrayBuffer( context.GetArrayBufferClass() ) ;
	pBuf->AllocateBuffer( GetStructureBytes() * nLength ) ;
	//
	const uint8_t *	pbytInit = GetStructureInit() ;
	if ( pbytInit != NULL )
	{
		uint8_t *	pbytBuf = pBuf->m_ptrBuf ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			eslMoveMemory( pbytBuf, pbytInit, GetStructureBytes() ) ;
			pbytBuf += GetStructureBytes() ;
		}
	}
	return	pBuf ;
}


//////////////////////////////////////////////////////////////////////////////
// 構造体基底クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSStructureClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSStructureClass::RSStructureClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSStructureClass::Initialize( RSContext& context )
{
	if ( m_pSuperClass == NULL )
	{
		AddSuperClass
			( context,
				context.GetTypedPointerClass
						( RSReferenceNumber::typeUint8 ) ) ;
	}
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSStructureClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"int length = 1",
				NULL, &RSStructureClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
			NULL, L"Uint8Pointer ptr",
			NULL, &RSStructureClass::method_init2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
			NULL, L"ArrayBuffer buf, int iOffset = 0, int nLength = -1",
			NULL, &RSStructureClass::method_init3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"equals",
			L"boolean", L"Object obj",
			NULL, &RSStructureClass::method_equals,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"sizeof", L"int", L"Class cls",
				NULL, &RSStructureClass::method_sizeof1, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"sizeof", L"int", L"Structure cls",
				NULL, &RSStructureClass::method_sizeof2, NULL ) ;
}

// void <init>( int length )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructureClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Structure 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	size_t	nLength = (size_t) arg.IntAt(0,1) ;
	size_t	nStructBytes = pStructType->GetStructureBytes() ;
	//
	RSArrayBuffer *	pBuf = pObj->m_pRefBuffer ;
	if ( pBuf != NULL )
	{
		pBuf->AllocateBuffer( nLength * nStructBytes ) ;
		//
		const uint8_t *	pbytInit = pStructType->GetStructureInit() ;
		if ( pbytInit != NULL )
		{
			uint8_t *	pPtr = pBuf->m_ptrBuf ;
			for ( size_t i = 1; i < nLength; i ++ )
			{
				eslMoveMemory( pPtr, pbytInit, nStructBytes ) ;
				pPtr += nStructBytes ;
			}
		}
	}
	else
	{
		pObj->SetPointer
			( pStructType->NewBuffer( context, nLength ), pStructType ) ;
	}
	return	NULL ;
}

// void <init>( Uint8Pointer ptr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructureClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Structure 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	RSTypedArrayPointer *	pPtr = ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt(0) ) ;
	if ( pPtr != NULL )
	{
		RSArrayBuffer *	pBuf = pPtr->m_pRefBuffer ;
		if ( pBuf != NULL )
		{
			pBuf->AddRef() ;
			pObj->SetPointer
				( pBuf, pStructType,
					pPtr->m_iOffset, (ssize_t) pPtr->m_nLimit ) ;
		}
	}
	return	NULL ;
}

// void <init>( ArrayBuffer buf, int iOffset = 0, int nLength = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructureClass::method_init3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *	pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Structure 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	RSArrayBuffer *	pBuf = ESLTypeCast<RSArrayBuffer>( arg.ObjectAt(0) ) ;
	RSObject::AddRef( pBuf ) ;
	pObj->SetPointer
		( pBuf, pStructType,
			(size_t) arg.IntAt(1), (ssize_t) arg.IntAt(2,-1) ) ;
	return	NULL ;
}

// boolean equals( Object obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructureClass::method_equals
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointer *
			pObj = ESLTypeCast<RSStructuredPointer>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"Structure 構築関数の this が Structure ではありません" ) ;
		return	NULL ;
	}
	RSStructuredPointerClass *
		pStructType = ESLTypeCast<RSStructuredPointerClass>( pThis->GetRSClass() ) ;
	if ( pStructType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	RSStructuredPointer *
		pPtr = ESLTypeCast<RSStructuredPointer>( arg.ObjectAt(0) ) ;
	if ( pPtr == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSStructuredPointerClass *
		pObjType = ESLTypeCast<RSStructuredPointerClass>( pPtr->GetRSClass() ) ;
	if ( pObjType == NULL )
	{
		context.ThrowExceptionError( L"構造体情報がありません" ) ;
		return	NULL ;
	}
	ssize_t	iCastOffset = pObjType->OffsetSuperStruct( pStructType ) ;
	if ( iCastOffset < 0 )
	{
		context.ThrowExceptionError
			( SString(pObjType->GetRSClassName())
				+ L" から " + pStructType->GetRSClassName()
				+ L" へキャストできません" ) ;
		return	NULL ;
	}
	if ( (pObj->m_nLimit < pStructType->GetStructureBytes())
		|| (pPtr->m_nLimit < pObjType->GetStructureBytes() + iCastOffset) )
	{
		context.ThrowExceptionError
			( L"ポインタが領域を超えています", L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	const uint8_t *	pThisPtr = pObj->GetPointer() ;
	const uint8_t *	pObjPtr = pPtr->GetPointer() + iCastOffset ;
	size_t	nBytes = pStructType->GetStructureBytes() ;
	for ( size_t i = 0; i < nBytes; i ++ )
	{
		if ( pThisPtr[i] != pObjPtr[i] )
		{
			return	context.new_Boolean( false ) ;
		}
	}
	return	context.new_Boolean( true ) ;
}

// static int sizeof( Class cls )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructureClass::method_sizeof1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStructuredPointerClass *	pClass =
		ESLTypeCast<RSStructuredPointerClass>( arg.ObjectAt( 0 ) ) ;
	if ( pClass == NULL )
	{
		return	context.new_Integer( 0 ) ;
	}
	return	context.new_Integer( pClass->GetStructureBytes() ) ;
}

// static int sizeof( Structure cls )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStructureClass::method_sizeof2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	if ( pObj == NULL )
	{
		return	context.new_Integer( 0 ) ;
	}
	RSStructuredPointerClass *	pClass =
		ESLTypeCast<RSStructuredPointerClass>( pObj->GetRSClass() ) ;
	if ( pClass == NULL )
	{
		return	context.new_Integer( 0 ) ;
	}
	return	context.new_Integer( pClass->GetStructureBytes() ) ;
}


