
#include <sakuragl/sakuragl.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_parser.h>
#include <rosetta/rosetta_array.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// コンパイル時型情報
//////////////////////////////////////////////////////////////////////////////

const size_t	RSTypeInfo::m_bytesNumber[RSTypeInfo::typeLastNumber + 1] =
{
	1, 1, 1, 2, 2, 4, 4, 8, 4, 8,
} ;

const size_t	RSTypeInfo::m_bitsNumber[RSTypeInfo::typeLastNumber + 1] =
{
	1, 8, 8, 16, 16, 32, 32, 64, 32, 64,
} ;

const RSInteger::IntegerType
		RSTypeInfo::m_typeInteger[RSTypeInfo::typeLastNumber + 1] =
{
	RSInteger::typeBoolean,
	RSInteger::typeUint8,
	RSInteger::typeInt8,
	RSInteger::typeUint16,
	RSInteger::typeInt32,
	RSInteger::typeUint32,
	RSInteger::typeInt32,
	RSInteger::typeInt64,
	RSInteger::typeInt64,
	RSInteger::typeInt64,
} ;

const RSCodeControl::WordIndex
		RSTypeInfo::m_wiBasicType[RSTypeInfo::typeLastNumber + 1] =
{
	RSCodeControl::wiBoolean,
	RSCodeControl::wiByte,
	RSCodeControl::wiByte,
	RSCodeControl::wiChar,
	RSCodeControl::wiShort,
	RSCodeControl::wiInt,
	RSCodeControl::wiInt,
	RSCodeControl::wiLong,
	RSCodeControl::wiFloat,
	RSCodeControl::wiDouble,
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSTypeInfo::RSTypeInfo( void )
	: m_pClass( NULL ), m_accMod( 0 ),
		m_typeNum( typeObject ), m_flagPointer( false ),
		m_flagPtrRef( false ), m_flagReference( false ),
		m_pImmediate( NULL ), m_iFuncProto( 0 ), m_pLoadInst( NULL )
{
}

RSTypeInfo::RSTypeInfo( const RSTypeInfo& ti )
	: m_pClass( NULL ), m_accMod( 0 ),
		m_typeNum( typeObject ),m_flagPointer( false ),
		m_flagPtrRef( false ), m_flagReference( false ),
		m_pImmediate( NULL ), m_iFuncProto( 0 ), m_pLoadInst( NULL )
{
	CopyType( ti ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSTypeInfo::~RSTypeInfo( void )
{
	if ( m_pImmediate != NULL )
	{
		m_pImmediate->ReleaseRef() ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
RSTypeInfo& RSTypeInfo::operator = ( const RSTypeInfo& ti )
{
	CopyType( ti ) ;
	return	*this ;
}

// 一時計算領域に割り当てられているか？
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsAllocated( void ) const
{
	return	(m_fp.iLocal >= 0) || (m_fp.iNumber >= 0) || (m_fp.iPointer >= 0) ;
}

// オブジェクトか？
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsObject( void ) const
{
	return	(m_typeNum == typeObject) && !IsPointer() ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsInteger( void ) const
{
	return	(m_typeNum >= typeFirstInt)
				&& (m_typeNum <= typeLastInt) && !IsPointer() ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsFloatingPoint( void ) const
{
	return	(m_typeNum >= typeFirstFloat)
				&& (m_typeNum <= typeLastFloat) && !IsPointer() ;
}

// ポインタ型か？
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsPointer( void ) const
{
	return	m_flagPointer && !m_flagPtrRef ;
}

// 構造体か？
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsStructure( void ) const
{
	return	IsPointer() && (m_typeNum == typeObject) ;
}

// 左辺値（参照）か？
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsReference( void ) const
{
	return	m_flagReference || m_flagPtrRef ;
}

// const 修飾
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsConstant( void ) const
{
	return	(m_accMod & RSObject::modifierConst) != 0 ;
}

// 型設定
//////////////////////////////////////////////////////////////////////////////
void RSTypeInfo::SetType( const RSContext& context, RSClass * pClass )
{
	m_pClass = pClass ;
	//
	// ポインタ判定
	//
	RSTypedArrayPointerClass *
		pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
	if ( pPtrClass != NULL )
	{
		m_flagPointer = true ;
		//
		switch ( pPtrClass->m_typeElement )
		{
		case	RSReferenceNumber::typeUint8:
			m_typeNum = typeUint8 ;
			break ;
		case	RSReferenceNumber::typeInt8:
			m_typeNum = typeInt8 ;
			break ;
		case	RSReferenceNumber::typeUint16:
			m_typeNum = typeUint16 ;
			break ;
		case	RSReferenceNumber::typeInt16:
			m_typeNum = typeInt16 ;
			break ;
		case	RSReferenceNumber::typeUint32:
			m_typeNum = typeUint32 ;
			break ;
		case	RSReferenceNumber::typeInt32:
			m_typeNum = typeInt32 ;
			break ;
		case	RSReferenceNumber::typeInt64:
			m_typeNum = typeInt64 ;
			break ;
		case	RSReferenceNumber::typeFloat32:
			m_typeNum = typeFloat32 ;
			break ;
		case	RSReferenceNumber::typeFloat64:
			m_typeNum = typeFloat64 ;
			break ;
		case	RSReferenceNumber::typeObject:
			m_typeNum = typeObject ;
			break ;
		}
	}
	else
	{
		m_flagPointer = false ;
		//
		// 数値型判定
		//
		m_typeNum = GetNumberTypeOf( context, pClass );
	}
}

void RSTypeInfo::CopyType( const RSTypeInfo & ti )
{
	m_pClass = ti.m_pClass ;
	m_accMod = ti.m_accMod ;
	m_typeNum = ti.m_typeNum ;
	m_flagPointer = ti.m_flagPointer ;
	m_flagReference = ti.m_flagReference ;
	m_fp = ti.m_fp ;
	//
	if ( ti.m_pImmediate != NULL )
	{
		ti.m_pImmediate->AddRef() ;
	}
	if ( m_pImmediate != NULL )
	{
		m_pImmediate->ReleaseRef() ;
	}
	m_pImmediate = ti.m_pImmediate ;
	m_iFuncProto = ti.m_iFuncProto ;
	m_pLoadInst = ti.m_pLoadInst ;
}

// 数値型判定
//////////////////////////////////////////////////////////////////////////////
RSTypeInfo::NumberType
	RSTypeInfo::GetNumberTypeOf
		( const RSContext& context, RSClass * pClass )
{
	if ( context.GetBasicTypeClass
				( RSCodeControl::wiBoolean ) == pClass )
	{
		return	typeBoolean ;
	}
	else if ( context.GetBasicTypeClass
				( RSCodeControl::wiByte ) == pClass )
	{
		return	typeInt8 ;
	}
	else if ( context.GetBasicTypeClass
				( RSCodeControl::wiShort ) == pClass )
	{
		return	typeInt16 ;
	}
	else if ( context.GetBasicTypeClass
				( RSCodeControl::wiChar ) == pClass )
	{
		return	typeUint16 ;
	}
	else if ( context.GetBasicTypeClass
				( RSCodeControl::wiInt ) == pClass )
	{
		return	typeInt32 ;
	}
	else if ( context.GetBasicTypeClass
				( RSCodeControl::wiLong ) == pClass )
	{
		return	typeInt64 ;
	}
	else if ( context.GetBasicTypeClass
				( RSCodeControl::wiFloat ) == pClass )
	{
		return	typeFloat32 ;
	}
	else if ( context.GetBasicTypeClass
				( RSCodeControl::wiDouble ) == pClass )
	{
		return	typeFloat64 ;
	}
	else if ( context.GetBooleanClass() == pClass )
	{
		return	typeBoolean ;
	}
	else if ( context.GetIntegerClass() == pClass )
	{
		return	typeInt64 ;
	}
	else if ( context.GetNumberClass() == pClass )
	{
		return	typeFloat64 ;
	}
	else
	{
		return	typeObject ;
	}
}

// 即値設定
//////////////////////////////////////////////////////////////////////////////
void RSTypeInfo::SetImmediate
	( RSContext& context, RSObject * pObj,
				RSClass * pClass, size_t iFuncProto )
{
	SetType( context, pClass ) ;
	//
	if ( pObj != NULL )
	{
		pObj->AddRef() ;
	}
	if ( m_pImmediate != NULL )
	{
		m_pImmediate->ReleaseRef() ;
	}
	m_fp.iLocal = -1 ;
	m_fp.iNumber = -1 ;
	m_fp.iPointer = -1 ;
	//
	m_pImmediate = pObj ;
	m_iFuncProto = iFuncProto ;
}

// ローカル変数割り当てサイズ取得
//////////////////////////////////////////////////////////////////////////////
void RSTypeInfo::GetLocalFrameSize( RSFramePointer& fp ) const
{
	fp.iLocal = 0 ;
	fp.iNumber = 0 ;
	fp.iPointer = 0 ;
	//
	if ( m_flagPointer )
	{
		fp.iPointer = 1 ;
	}
	else if ( m_typeNum != typeObject )
	{
		fp.iNumber = 1 ;
		if ( m_bytesNumber[m_typeNum] > 4 )
		{
			fp.iNumber = 2 ;
		}
	}
	else
	{
		fp.iLocal = 1 ;
	}
}

// キャスト可能判定
//////////////////////////////////////////////////////////////////////////////
bool RSTypeInfo::IsMatchTypeFor
	( const RSContext& context, RSClass * pClass ) const
{
	if ( m_pClass == NULL )
	{
		return	false ;
	}
	if ( IsPointer() )
	{
		return	m_pClass->IsInstanceOf( pClass ) ;
	}
	else if ( IsInteger() )
	{
		NumberType	typeNum = GetNumberTypeOf( context, pClass ) ;
		if ( (typeNum >= typeFirstInt)
			&& (typeNum <= typeLastInt) )
		{
			return	true ;
		}
		else if ( (typeNum >= typeFirstFloat)
				&& (typeNum <= typeLastFloat) )
		{
			return	(m_bitsNumber[m_typeNum] < m_bitsNumber[typeNum]) ;
		}
		return	(pClass == context.GetGenericObjectClass()) ;
	}
	else if ( IsFloatingPoint() )
	{
		NumberType	typeNum = GetNumberTypeOf( context, pClass ) ;
		if ( (typeNum >= typeFirstFloat)
				&& (typeNum <= typeLastFloat) )
		{
			return	true ;
		}
		return	(pClass == context.GetGenericObjectClass()) ;
	}
	else
	{
		return	m_pClass->IsInstanceOf( pClass ) ;
	}
}

// 型名取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString RSTypeInfo::GetTypeName( void ) const
{
	if ( m_pClass != NULL )
	{
		return	m_pClass->GetFullClassName() ;
	}
	return	L"void" ;
}



//////////////////////////////////////////////////////////////////////////////
// 中間処理コード
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSInstruction, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSInstruction::RSInstruction( void )
{
	m_code = codeInvalid ;
	m_opCode = RSCodeOperator::opInvalid ;
	m_index = 0 ;
	m_limit = 0 ;
	m_ipTarget = 0 ;
	m_memory = memoryLocal ;
	m_nLocalNest = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSInstruction::~RSInstruction( void )
{
}

// 即値ロード
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::LoadImmediate( RSContext& context, RSObject * pObj )
{
	m_code = codeLoad ;
	m_memory = memoryImmediate ;
	m_typeDst.SetImmediate( context, pObj, pObj->GetRSClass() ) ;
	m_typeDst.m_pLoadInst = this ;
}

// ローカル変数ロード
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::LoadLocal
	( RSContext& context, RSClass * pType, const RSFramePointer& fp )
{
	m_code = codeLoad ;
	m_memory = memoryLocal ;
	m_nLocalNest = 0 ;
	m_typeSrc1.SetType( context, pType ) ;
	m_typeSrc1.m_fp = fp ;
	m_typeDst.SetType( context, pType ) ;
	m_typeDst.m_flagReference = true ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::LoadLocal
	( RSContext& context, const RSTypeInfo& typeLocal, int nLocalNest )
{
	m_code = codeLoad ;
	m_memory = memoryLocal ;
	m_nLocalNest = nLocalNest ;
	m_typeSrc1 = typeLocal ;
	m_typeDst = typeLocal ;
	m_typeDst.m_flagReference = true ;
	m_typeDst.m_pLoadInst = this ;
}

// グローバル変数ロード
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::LoadGlobal
	( RSContext& context,
		RSClass * pType, const wchar_t * pwszName )
{
	m_code = codeLoad ;
	m_memory = memoryGlobal ;
	m_literal = pwszName ;
	m_typeDst.SetType( context, pType ) ;
	m_typeDst.m_flagReference = true ;
	m_typeDst.m_pLoadInst = this ;
}

// 配列要素直接参照
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::LoadElementInt
	( RSContext& context,
		const RSTypeInfo& typeArray, int nIndex, bool fReference )
{
	m_code = codeElementInt ;
	m_index = nIndex ;
	m_typeSrc1 = typeArray ;
	//
	RSClass *	pElementType = NULL ;
	RSGenericArrayClass *	pArrayClass =
			ESLTypeCast<RSGenericArrayClass>( typeArray.m_pClass ) ;
	if ( pArrayClass != NULL )
	{
		pElementType = pArrayClass->m_pElementClass ;
	}
	else
	{
		RSGenericHashMapClass *	pHashClass =
				ESLTypeCast<RSGenericHashMapClass>( typeArray.m_pClass ) ;
		if ( pHashClass != NULL )
		{
			pElementType = pHashClass->m_pElementClass ;
		}
	}
	if ( pElementType == NULL )
	{
		pElementType = context.GetGenericObjectClass() ;
	}
	m_typeDst.SetType( context, pElementType ) ;
	m_typeDst.m_flagReference = fReference ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::LoadElementStr
	( RSContext& context,
		RSClass * pElementType,
		const RSTypeInfo& typeObj,
		const wchar_t * pwszName, bool fReference )
{
	m_code = codeElementStr ;
	m_literal = pwszName ;
	m_typeSrc1 = typeObj ;
	m_typeDst.SetType( context, pElementType ) ;
	m_typeDst.m_flagReference = fReference ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::LoadIndirectElementInt
	( RSContext& context,
		const RSTypeInfo& typeArray,
		const RSTypeInfo& typeIndex, bool fReference )
{
	m_code = codeElementIndirectInt ;
	m_typeSrc1 = typeArray ;
	m_typeSrc2 = typeIndex ;
	//
	RSClass *	pElementType = NULL ;
	RSGenericArrayClass *	pArrayClass =
			ESLTypeCast<RSGenericArrayClass>( typeArray.m_pClass ) ;
	if ( pArrayClass != NULL )
	{
		pElementType = pArrayClass->m_pElementClass ;
	}
	else
	{
		RSGenericHashMapClass *	pHashClass =
				ESLTypeCast<RSGenericHashMapClass>( typeArray.m_pClass ) ;
		if ( pHashClass != NULL )
		{
			pElementType = pHashClass->m_pElementClass ;
		}
	}
	if ( pElementType == NULL )
	{
		pElementType = context.GetGenericObjectClass() ;
	}
	m_typeDst.SetType( context, pElementType ) ;
	m_typeDst.m_flagReference = fReference ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::LoadIndirectElementStr
	( RSContext& context,
		RSClass * pElementType,
		const RSTypeInfo& typeObj,
		const RSTypeInfo& typeIndex, bool fReference )
{
	m_code = codeElementIndirectStr ;
	m_typeSrc1 = typeObj ;
	m_typeSrc2 = typeIndex ;
	m_typeDst.SetType( context, pElementType ) ;
	m_typeDst.m_flagReference = fReference ;
	m_typeDst.m_pLoadInst = this ;
}

// 構造体・ポインタ要素参照
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::RefStructureMember
	( RSContext& context,
		RSClass * pElementType,
		const RSTypeInfo& typeObj,
		int iOffsetBytes, int nLimitBytes )
{
	m_code = codeElementRef ;
	m_index = iOffsetBytes ;
	m_limit = nLimitBytes ;
	m_typeSrc1 = typeObj ;
	m_typeDst.SetType( context, pElementType ) ;
	m_typeDst.m_flagPtrRef = true ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::PtrStructureMember
	( RSContext& context,
		RSClass * pElementType,
		const RSTypeInfo& typeObj,
		int iOffsetBytes, int nLimitBytes )
{
	m_code = codeElementPtr ;
	m_index = iOffsetBytes ;
	m_limit = nLimitBytes ;
	m_typeSrc1 = typeObj ;
	m_typeDst.SetType( context, pElementType ) ;
	m_typeDst.m_pLoadInst = this ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::UniOperator
	( RSContext& context,
		RSCodeOperator::OperatorIndex opIndex,
		RSClass * pDstType, const RSTypeInfo& typeSrc )
{
	m_code = codeUniOperate ;
	m_opCode = opIndex ;
	m_typeSrc1 = typeSrc ;
	m_typeDst.SetType( context, pDstType ) ;
	m_typeDst.m_pLoadInst = this ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::BinOperator
	( RSContext& context,
		RSCodeOperator::OperatorIndex opIndex,
		RSClass * pDstType,
		const RSTypeInfo& typeSrc1, const RSTypeInfo& typeSrc2 )
{
	m_code = codeOperate ;
	m_opCode = opIndex ;
	m_typeSrc1 = typeSrc1 ;
	m_typeSrc2 = typeSrc2 ;
	m_typeDst.SetType( context, pDstType ) ;
	m_typeDst.m_pLoadInst = this ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::MoveOperator
	( RSContext& context,
		RSCodeOperator::OperatorIndex opIndex,
		const RSTypeInfo& typeDst, const RSTypeInfo& typeSrc )
{
	m_code = codeOperate ;
	m_opCode = opIndex ;
	m_typeSrc1 = typeDst ;
	m_typeSrc2 = typeSrc ;
	m_typeDst = typeDst ;
	m_typeDst.m_pLoadInst = this ;
}

// 型キャスト
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::CastObject
	( RSContext& context,
		RSClass * pCastType, const RSTypeInfo& typeSrc )
{
	m_code = codeCast ;
	m_typeSrc1 = typeSrc ;
	m_typeDst.SetType( context, pCastType ) ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::ConvertPointer
	( RSContext& context,
		RSClass * pCastType, const RSTypeInfo& typeSrc )
{
	m_code = codeConvertPointer ;
	m_typeSrc1 = typeSrc ;
	m_typeDst.SetType( context, pCastType ) ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::ConvertNumber
	( RSContext& context,
		RSClass * pCastType, const RSTypeInfo& typeSrc )
{
	m_code = codeConvertNumber ;
	m_typeSrc1 = typeSrc ;
	m_typeDst.SetType( context, pCastType ) ;
	m_typeDst.m_pLoadInst = this ;
}

void RSInstruction::ConvertObject
	( RSContext& context,
		RSInstruction::Code code,
		RSClass * pCastType, const RSTypeInfo& typeSrc )
{
	ESLAssert( (code >= codeConvertNum2Str) && (code <= codeConvert2Boolean) ) ;
	m_code = code ;
	m_typeSrc1 = typeSrc ;
	m_typeDst.SetType( context, pCastType ) ;
	m_typeDst.m_pLoadInst = this ;
}

// オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::NewObject
	( RSContext& context, RSClass * pClass )
{
	m_code = codeNewObject ;
	m_typeDst.SetType( context, pClass ) ;
}

// 配列長取得
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::LengthOf( RSContext& context, const RSTypeInfo& typeSrc )
{
	m_code = codeExOperate ;
	m_xopCode = xopLength ;
	m_typeSrc1 = typeSrc ;
	m_typeDst.SetType( context, context.GetIntegerClass() ) ;
}

// キー配列取得
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::KeysOf( RSContext& context, const RSTypeInfo& typeSrc )
{
	m_code = codeExOperate ;
	m_xopCode = xopKeys ;
	m_typeSrc1 = typeSrc ;
	m_typeDst.SetType
		( context, context.GetVM()->
						GetArrayClassAs( context.GetStringClass() ) ) ;
}

// ジャンプ
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::Jump( size_t ipTarget )
{
	m_code = codeJump ;
	m_ipTarget = ipTarget ;
}

void RSInstruction::JumpIf
	( RSContext& context,
		const RSTypeInfo& typeSrc, size_t ipTarget )
{
	m_code = codeJumpIf ;
	m_ipTarget = ipTarget ;
	m_typeSrc1 = typeSrc ;
}

void RSInstruction::JumpNotIf
	( RSContext& context,
		const RSTypeInfo& typeSrc, size_t ipTarget )
{
	m_code = codeJumpNotIf ;
	m_ipTarget = ipTarget ;
	m_typeSrc1 = typeSrc ;
}

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::CallDirect
	( RSContext& context,
		RSFunctionObject * pFunc, size_t iFunc,
		const RSTypeInfo& typeThis,
		const SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	m_code = codeCall ;
	m_typeSrc1 = typeThis ;
	//
	pFunc->AddRef() ;
	m_typeSrc2.SetImmediate( context, pFunc, pFunc->GetRSClass() ) ;
	m_index = (int) iFunc ;
	//
	m_typeArg = aArgType ;
	//
	RSFunctionPrototype *	pProto = pFunc->m_arrPrototypes.GetAt( iFunc ) ;
	if ( pProto != NULL )
	{
		if ( pProto->m_pReturnType != NULL )
		{
			m_typeDst.SetType( context, pProto->m_pReturnType ) ;
		}
		else
		{
			m_typeDst.SetType( context, context.GetGenericObjectClass() ) ;
		}
	}
	else
	{
		m_typeDst.SetType( context, context.GetGenericObjectClass() ) ;
	}
}

void RSInstruction::CallIndirect
	( RSContext& context,
		const RSTypeInfo& typeFunc, size_t iFunc,
		const RSTypeInfo& typeThis,
		const SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	m_code = codeCallIndirect ;
	m_typeSrc1 = typeThis ;
	m_typeSrc2 = typeFunc ;
	m_index = (int) iFunc ;
	m_typeArg = aArgType ;
	//
	RSGenericFunctionClass *	pFuncClass =
		ESLTypeCast<RSGenericFunctionClass>( typeFunc.m_pClass ) ;
	if ( (pFuncClass != NULL)
		&& (pFuncClass->m_pProto != NULL) )
	{
		m_typeDst.SetType( context, pFuncClass->m_pProto->m_pReturnType ) ;
	}
	else
	{
		m_typeDst.SetType( context, context.GetGenericObjectClass() ) ;
	}
}

void RSInstruction::CallVirtual
	( RSContext& context,
		RSClass * pReturnType,
		const wchar_t * pwszFuncName, size_t iFunc,
		const RSTypeInfo& typeThis,
		const SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	m_code = codeCallIndirect ;
	m_typeSrc1 = typeThis ;
	m_literal = pwszFuncName ;
	m_index = (int) iFunc ;
	m_typeArg = aArgType ;
	m_typeDst.SetType( context, pReturnType ) ;
}

// 関数復帰
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::Return( const RSTypeInfo& typeSrc )
{
	m_code = codeReturn ;
	m_typeSrc1 = typeSrc ;
}

// 例外ブロック
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::BeginTry( size_t ipTarget )
{
	m_code = codeTry ;
	m_ipTarget = ipTarget ;
}

void RSInstruction::EndTry( void )
{
	m_code = codeEndTry ;
}

void RSInstruction::Throw( const RSTypeInfo& typeSrc )
{
	m_code = codeThrow ;
	m_typeSrc1 = typeSrc ;
}

void RSInstruction::GetException( const RSTypeInfo& typeDst )
{
	m_code = codeGetException ;
	m_typeDst = typeDst ;
}

void RSInstruction::ClearException( void )
{
	m_code = codeClearException ;
}

// 同期
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::Synchronize( const RSTypeInfo& typeSrc )
{
	m_code = codeSynchronize ;
	m_typeSrc1 = typeSrc ;
}

void RSInstruction::Unsynchronize( const RSTypeInfo& typeSrc )
{
	m_code = codeUnsynchronize ;
	m_typeSrc1 = typeSrc ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::FreeLocal( const RSTypeInfo& typeFree )
{
	m_code = codeFree ;
	m_typeDst = typeFree ;
}

// フェンス命令
//////////////////////////////////////////////////////////////////////////////
void RSInstruction::FenceStack
	( const RSFramePointer& fpMin, const RSFramePointer& fpMax )
{
	m_code = codeFence ;
	m_typeSrc1.m_fp = fpMax ;
	m_typeDst.m_fp = fpMin ;
}



//////////////////////////////////////////////////////////////////////////////
// 中間処理コードブロック
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSInstructionBlock, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSInstructionBlock::RSInstructionBlock( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSInstructionBlock::~RSInstructionBlock( void )
{
}

// コード追加
//////////////////////////////////////////////////////////////////////////////
void RSInstructionBlock::AddCode( RSInstruction * pInst )
{
	m_block.Add( pInst ) ;
}

// コード挿入
//////////////////////////////////////////////////////////////////////////////
void RSInstructionBlock::InsertCode( size_t iPos, RSInstruction * pInst )
{
	m_block.InsertAt( iPos, pInst ) ;
}

// コード削除
//////////////////////////////////////////////////////////////////////////////
void RSInstructionBlock::ClearCodeAfter( size_t iPos )
{
	if ( iPos < m_block.GetLength() )
	{
		m_block.Remove( iPos, m_block.GetLength() - iPos ) ;
	}
}

// コード全削除
//////////////////////////////////////////////////////////////////////////////
void RSInstructionBlock::ClearAllCodes( void )
{
	m_block.RemoveAll() ;
}

// 現在のコード位置
//////////////////////////////////////////////////////////////////////////////
size_t RSInstructionBlock::GetCurrentPos( void ) const
{
	return	m_block.GetLength() ;
}



//////////////////////////////////////////////////////////////////////////////
// 関数コードブロック
//////////////////////////////////////////////////////////////////////////////

const RSInstructionParser::PFUNC_COMPILE_STATEMENT
	RSInstructionParser::m_pfnCompileStatement[RSCodeControl::wiCount + 1] =
{
	&RSInstructionParser::CompileStatementImport,			// import
	&RSInstructionParser::CompileStatementClass,			// class
	&RSInstructionParser::CompileStatementStruct,			// struct
	&RSInstructionParser::CompileStatementExpression,		// function
	&RSInstructionParser::CompileStatementInvalid,			// extends
	&RSInstructionParser::CompileStatementInvalid,			// implements
	&RSInstructionParser::CompileStatementFor,				// for
	&RSInstructionParser::CompileStatementWhile,			// while
	&RSInstructionParser::CompileStatementDo,				// do
	&RSInstructionParser::CompileStatementIf,				// if
	&RSInstructionParser::CompileStatementInvalid,			// else
	&RSInstructionParser::CompileStatementSwitch,			// switch
	&RSInstructionParser::CompileStatementCase,				// case
	&RSInstructionParser::CompileStatementDefault,			// default
	&RSInstructionParser::CompileStatementBreak,			// break
	&RSInstructionParser::CompileStatementContinue,			// continue
	&RSInstructionParser::CompileStatementTry,				// try
	&RSInstructionParser::CompileStatementInvalid,			// catch
	&RSInstructionParser::CompileStatementInvalid,			// finally
	&RSInstructionParser::CompileStatementThrow,			// throw
	&RSInstructionParser::CompileStatementReturn,			// return
	&RSInstructionParser::CompileStatementWith,				// with
	&RSInstructionParser::CompileStatementSynchronized,		// synchronized
	&RSInstructionParser::CompileStatementAccessModifier,	// static
	&RSInstructionParser::CompileStatementAccessModifier,	// abstract
	&RSInstructionParser::CompileStatementAccessModifier,	// native
	&RSInstructionParser::CompileStatementAccessModifier,	// const
	&RSInstructionParser::CompileStatementAccessModifier,	// public
	&RSInstructionParser::CompileStatementAccessModifier,	// protected
	&RSInstructionParser::CompileStatementAccessModifier,	// private
	&RSInstructionParser::CompileStatementVar,				// var
	&RSInstructionParser::CompileStatementVar,				// void
	&RSInstructionParser::CompileStatementVar,				// boolean
	&RSInstructionParser::CompileStatementVar,				// byte
	&RSInstructionParser::CompileStatementVar,				// short
	&RSInstructionParser::CompileStatementVar,				// char
	&RSInstructionParser::CompileStatementVar,				// int
	&RSInstructionParser::CompileStatementVar,				// long
	&RSInstructionParser::CompileStatementVar,				// float
	&RSInstructionParser::CompileStatementVar,				// double
	&RSInstructionParser::CompileStatementExpression,		// this
	&RSInstructionParser::CompileStatementExpression,		// super
	&RSInstructionParser::CompileStatementAccessModifier,	// extern
	&RSInstructionParser::CompileStatementInvalid,			// debug
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSInstructionParser::FunctionBlock, RSInstructionBlock )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSInstructionParser::FunctionBlock::FunctionBlock
		( RSFunctionPrototype * pProto, FunctionBlock * pParentScope )
	: m_fpLocal( 0, 0, 0 ), m_fpExpr( 0, 0, 0 )
{
	m_pParentScope = pParentScope ;
	m_pProto = pProto ;
	m_pClassSpace = NULL ;
	m_fPreConstruction = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSInstructionParser::FunctionBlock::~FunctionBlock( void )
{
	for ( size_t i = 0; i < m_aNamelessFuncs.GetLength() ; i ++ )
	{
		RSFunctionObject *	pFunc = m_aNamelessFuncs.GetAt( i ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
}

// 新しいフェーズのための初期化処理
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::FunctionBlock::ResetPhase
				( RSInstructionParser::CompilePhase phase )
{
	m_fpExpr = m_fpLocal ;
	//
	if ( phase == phaseImplementation )
	{
		m_block.RemoveAll() ;
	}
}

// 一時計算用割り当てをすべて解放
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::FunctionBlock::FreeAllTemporary( void )
{
	m_fpExpr = m_fpLocal ;
}

// 一時関数追加
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::FunctionBlock::AddNamelessFunction( RSFunctionObject * pFunc )
{
	m_aNamelessFuncs.Add( pFunc ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ローカル入れ子空間
//////////////////////////////////////////////////////////////////////////////

// 変数追加
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::LocalNest::AddVariable
			( const wchar_t * pwszName, RSTypeInfo * pType )
{
	ESLAssert( m_names.FindIndex( pwszName ) < 0 ) ;
	m_names.Add( new SSystem::SString( pwszName ) ) ;
	m_types.Add( pType ) ;
}

// 変数検索
//////////////////////////////////////////////////////////////////////////////
RSTypeInfo * RSInstructionParser::LocalNest::GetVariable
			( const wchar_t * pwszName ) const
{
	ssize_t	i = m_names.FindIndex( pwszName ) ;
	if ( i < 0 )
	{
		return	NULL ;
	}
	return	m_types.GetAt( (size_t) i ) ;
}

// break ジャンプ先を確定する
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::LocalNest::CommitBreakTargets( size_t ipTarget )
{
	for ( size_t i = 0; i < m_breaks.GetLength(); i ++ )
	{
		RSInstruction *	pInst = m_breaks.GetAt( i ) ;
		if ( pInst != NULL )
		{
			pInst->m_ipTarget = ipTarget ;
		}
	}
}

// continue ジャンプ先を確定する
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::LocalNest::CommitContinueTargets( size_t ipTarget )
{
	for ( size_t i = 0; i < m_continues.GetLength(); i ++ )
	{
		RSInstruction *	pInst = m_continues.GetAt( i ) ;
		if ( pInst != NULL )
		{
			pInst->m_ipTarget = ipTarget ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// ローカル名前空間
//////////////////////////////////////////////////////////////////////////////

// 入れ子作成
//////////////////////////////////////////////////////////////////////////////
RSInstructionParser::LocalNest *
	RSInstructionParser::LocalSpace::DescendNest
		( RSInstructionParser::NestControlType ctrl )
{
	LocalNest *	pNest = new LocalNest( ctrl ) ;
	m_nest.Push( pNest ) ;
	return	pNest ;
}

// 入れ子削除
//////////////////////////////////////////////////////////////////////////////
RSInstructionParser::LocalNest *
	RSInstructionParser::LocalSpace::AscendNest( void )
{
	return	m_nest.Pop() ;
}

// 入れ子取得
//////////////////////////////////////////////////////////////////////////////
RSInstructionParser::LocalNest *
	RSInstructionParser::LocalSpace::GetCurrentNest( void ) const
{
	return	m_nest.GetLastAt( 0 ) ;
}

RSInstructionParser::LocalNest *
	RSInstructionParser::LocalSpace::GetNestAt( size_t iNest ) const
{
	return	m_nest.GetLastAt( iNest ) ;
}

// 入れ子数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSInstructionParser::LocalSpace::GetNestDepth( void ) const
{
	return	m_nest.GetLength() ;
}

// 変数追加
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::LocalSpace::AddVariable
			( const wchar_t * pwszName, RSTypeInfo * pType )
{
	ESLAssert( m_names.FindIndex( pwszName ) < 0 ) ;
	m_names.Add( new SSystem::SString( pwszName ) ) ;
	m_types.Add( pType ) ;
	//
	LocalNest *	pNest = m_nest.GetLastAt(0) ;
	if ( pNest != NULL )
	{
		pNest->AddVariable( pwszName, pType ) ;
	}
}

RSTypeInfo *
	RSInstructionParser::LocalSpace::ReAddVariable( const wchar_t * pwszName )
{
	RSTypeInfo *	pType = GetVariableAs( pwszName ) ;
	if ( pType != NULL )
	{
		LocalNest *	pNest = m_nest.GetLastAt(0) ;
		if ( pNest != NULL )
		{
			pNest->AddVariable( pwszName, pType ) ;
		}
	}
	return	pType ;
}

// 変数検索
//////////////////////////////////////////////////////////////////////////////
RSTypeInfo * RSInstructionParser::LocalSpace::GetNestedVariable
			( const wchar_t * pwszName ) const
{
	for ( size_t i = 0; i < m_nest.GetLength(); i ++ )
	{
		LocalNest *	pNest = m_nest.GetLastAt( i ) ;
		if ( pNest != NULL )
		{
			RSTypeInfo *	pType = pNest->GetVariable( pwszName ) ;
			if ( pType != NULL )
			{
				return	pType ;
			}
		}
	}
	return	NULL ;
}

RSTypeInfo *
	RSInstructionParser::LocalSpace::GetVariableAs
			( const wchar_t * pwszName ) const
{
	ssize_t	i = m_names.FindIndex( pwszName ) ;
	if ( i < 0 )
	{
		return	NULL ;
	}
	return	m_types.GetAt( (size_t) i ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 中間処理コードコンパイラ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSInstructionParser, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSInstructionParser::RSInstructionParser( RSVirtualMachine * vm )
	: m_context( vm ), m_ctxMacro( vm ), m_blockGlobalInit( NULL, NULL )
{
	m_vm = vm ;
	//
	m_blockGlobalInit.m_pProto = &m_protoGlobalInit ;
	//
	m_nError = 0 ;
	m_nWarning = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSInstructionParser::~RSInstructionParser( void )
{
}

// スクリプト読み込み（クラス宣言解釈のみ）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	RSInstructionParser::LoadScript
		( const wchar_t * pwszFileName, Charset::EncodingType encoding )
{
	if ( m_aScripts.GetAs( pwszFileName ) == NULL )
	{
		//
		// ファイルを開く
		//
		SSmartPointer<SFileInterface>
			pFile = m_vm->OpenScriptFile( pwszFileName ) ;
		if ( pFile == NULL )
		{
			OutputError
				( SString(L"\'") + pwszFileName + L"\' を開けません" ) ;
			return	errFailed ;
		}
		//
		// 読み込む
		//
		RSSourceParser	sparsImport ;
		if ( sparsImport.ReadTextFile( *pFile, encoding ) )
		{
			OutputError
				( SString(L"\'") + pwszFileName + L"\' の読み込みに失敗しました" ) ;
			return	errFailed ;
		}
		RSScript *	pScript = new RSScript ;
		pScript->SetSourcePath( pwszFileName ) ;
		//
		// 基本解釈（プリプロセス）
		//
		SParserErrorLogger	perrLog ;
		pScript->ParseSource
			( pScript, m_ctxMacro, 0, sparsImport, perrLog ) ;
		if ( perrLog.GetErrorCount()
			|| perrLog.GetWarningCount() )
		{
			OutputErrorLog( perrLog ) ;
			delete	pScript ;
			return	errFailed ;
		}
		m_aScripts.SetAs( pwszFileName, pScript ) ;
		//
		// クラス宣言
		//
		RSCodeStream	cstrm( *pScript ) ;
		CompilePhase	phaseOld = m_phase ;
		m_phase = phaseDeclaration ;
		CompileAllStatements( m_blockGlobalInit, cstrm, NULL ) ;
		m_phase = phaseOld ;
	}
	return	errSuccess ;
}

// コンパイル実行
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::CompilePhaseDefinition( void )
{
	m_phase = phaseDefinition ;
	//
	// ソースコードに対して phaseDefinition フェーズ処理
	//
	CompileAllScripts() ;
	//
	// クラスの派生とメンバ定義処理
	//
	for ( size_t i = 0; i < m_psoaClass.GetLength(); i ++ )
	{
		ClassAppendix *	pAppendix = m_psoaClass.GetAt( i ) ;
		if ( pAppendix && pAppendix->m_pClass )
		{
			CompilePhaseClassDefinition( pAppendix->m_pClass ) ;
		}
	}
}


void RSInstructionParser::CompilePhaseAllocation( void )
{
	m_phase = phaseAllocation ;
	//
	// ソースコードに対して phaseAllocation フェーズ処理
	//
	CompileAllScripts() ;
	//
	// クラス初期化実装に対して phaseAllocation フェーズ処理
	//
	CompileAllClassCodes() ;
	//
	// 関数実装に対して phaseAllocation フェーズ処理
	//
	CompileAllFunctionCodes() ;
}

void RSInstructionParser::CompilePhaseImplementation( void )
{
	m_phase = phaseImplementation ;
	//
	// ソースコードに対して phaseAllocation フェーズ処理
	//
	CompileAllScripts() ;
	//
	// クラス初期化実装に対して phaseAllocation フェーズ処理
	//
	CompileAllClassCodes() ;
	//
	// 関数実装に対して phaseAllocation フェーズ処理
	//
	while ( CompileAllFunctionCodes() )
	{
		if ( m_nError != 0 )
		{
			break ;
		}
	}
}

void RSInstructionParser::CompileAllScripts( void )
{
	for ( size_t i = 0; i < m_aScripts.GetLength(); i ++ )
	{
		RSScript *	pScript = m_aScripts.GetAt( i ) ;
		if ( pScript == NULL )
		{
			continue ;
		}
		RSCodeStream	cstrm( *pScript ) ;
		m_blockGlobalInit.ResetPhase( m_phase ) ;
		CompileAllStatements( m_blockGlobalInit, cstrm, NULL ) ;
	}
}

void RSInstructionParser::CompileAllClassCodes( void )
{
	for ( size_t i = 0; i < m_psoaClass.GetLength(); i ++ )
	{
		ClassAppendix *	pAppendix = m_psoaClass.GetAt( i ) ;
		if ( (pAppendix == NULL)
			|| (pAppendix->m_protoInit.m_pParenthesis) )
		{
			continue ;
		}
		RSCodeStream	cstrm( *(pAppendix->m_protoInit.m_pParenthesis) ) ;
		pAppendix->m_pClass->AddRef() ;
		m_context.PushNamespace
			( NULL, pAppendix->m_pClass, RSObject::modifierPrivate ) ;
		pAppendix->m_codeInit.ResetPhase( m_phase ) ;
		CompileAllStatements
			( pAppendix->m_codeInit, cstrm, pAppendix->m_pClass ) ;
		m_context.PopNamespace() ;
	}
}

bool RSInstructionParser::CompileAllFunctionCodes( void )
{
	bool	fCompiled = false ;
	for ( size_t i = 0; i < m_psoaFunc.GetLength(); i ++ )
	{
		FunctionAppendix *	pAppendix = m_psoaFunc.GetAt( i ) ;
		if ( (pAppendix == NULL)
			|| (pAppendix->m_phaseCompiled >= m_phase) )
		{
			continue ;
		}
		if ( CompileFunctionImplementation( pAppendix ) )
		{
			fCompiled = true ;
		}
	}
	return	fCompiled ;
}

void RSInstructionParser::CompilePhaseClassDefinition( RSClass * pClass )
{
	if ( pClass->m_flagInitialized )
	{
		return ;
	}
	ClassAppendix *	pAppendix = m_psoaClass.GetAs( pClass ) ;
	if ( pAppendix == NULL )
	{
		return ;
	}
	if ( pAppendix->m_protoInit.m_pParenthesis == NULL )
	{
		return ;
	}
	//
	// クラス派生
	//
	if ( pAppendix->m_pSuperClass
		&& !(pAppendix->m_pSuperClass->m_flagInitialized) )
	{
		CompilePhaseClassDefinition( pAppendix->m_pSuperClass ) ;
	}
	pClass->AddSuperClass( m_context, pAppendix->m_pSuperClass ) ;
	TestContextException( m_context ) ;
	//
	for ( size_t i = 0; i < pAppendix->m_lstImplements.GetLength(); i ++ )
	{
		RSClass *	pSuperClass = pAppendix->m_lstImplements.GetAt( i ) ;
		if ( pSuperClass )
		{
			if ( !(pSuperClass->m_flagInitialized) )
			{
				CompilePhaseClassDefinition( pSuperClass ) ;
			}
			pClass->AddImplementClass( m_context, pSuperClass, true ) ;
			TestContextException( m_context ) ;
		}
	}
	pClass->Initialize( m_context ) ;
	TestContextException( m_context ) ;
	//
	// クラスメンバ定義
	//
	RSCodeStream	cs( *(pAppendix->m_protoInit.m_pParenthesis) ) ;
	pClass->AddRef() ;
	m_context.PushNamespace( NULL, pClass, RSObject::modifierPrivate ) ;
	pAppendix->m_codeInit.ResetPhase( m_phase ) ;
	CompileAllStatements( pAppendix->m_codeInit, cs, pClass ) ;
	m_context.PopNamespace() ;
	//
	pClass->FinishClass( m_context ) ;
	TestContextException( m_context ) ;
}

bool RSInstructionParser::CompileFunctionImplementation
	( FunctionAppendix * pFuncAppendix )
{
	RSFunctionPrototype *	pProto = pFuncAppendix->m_pProto ;
	if ( pProto && pProto->m_pParenthesis
				&& (pFuncAppendix->m_phaseCompiled < m_phase) )
	{
		RSCodeStream	cs( *(pProto->m_pParenthesis) ) ;
		RSClass *		pClass = pProto->m_pNamespaceClass ;
		RSObject::AddRef( pClass ) ;
		m_context.PushNamespace( NULL, pClass, RSObject::modifierPrivate ) ;
		pFuncAppendix->m_codeFunc.ResetPhase( m_phase ) ;
		CompileAllStatements( pFuncAppendix->m_codeFunc, cs, pClass ) ;
		m_context.PopNamespace() ;
		//
		pFuncAppendix->m_phaseCompiled = m_phase ;
		return	true ;
	}
	return	false ;
}

// 複文解釈
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::CompileAllStatements
	( RSInstructionParser::FunctionBlock& block,
			RSCodeStream& cstrm, RSClass * pClassSpace )
{
	RSClass *	pOldClassSpace = block.m_pClassSpace ;
	block.m_pClassSpace = pClassSpace ;
	//
	const RSParenthesis *	pLastParenthesis = m_pCurParenthesis ;
	m_pCurParenthesis = cstrm.GetParenthesis() ;
	//
	while ( !cstrm.IsEndOfStream() )
	{
		CompileAStatement( block, cstrm ) ;
	}
	m_pCurParenthesis = pLastParenthesis ;
	//
	block.m_pClassSpace = pOldClassSpace ;
}

// 文解釈
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::CompileStatements
	( RSInstructionParser::FunctionBlock& block, RSCodeStream& cstrm )
{
	RSParenthesis *	pPrth =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrth != NULL )
	{
		RSClass *	pClassSpace = block.m_pClassSpace ;
		block.m_pClassSpace = NULL ;
		//
		RSCodeStream	cs( *pPrth ) ;
		while ( !cs.IsEndOfStream() )
		{
			CompileAStatement( block, cs ) ;
		}
		//
		block.m_pClassSpace = pClassSpace ;
	}
	else
	{
		CompileAStatement( block, cstrm ) ;
	}
}

// 一文解釈
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::CompileAStatement
	( RSInstructionParser::FunctionBlock& block, RSCodeStream& cstrm )
{
	ResetStatementContext( block ) ;
	//
	RSCodeControl *	pCode = cstrm.NextStatementControlWord() ;
	RSCode *	pcd = cstrm.GetTerm() ;
	if ( pcd != NULL )
	{
		m_iSrcStatement = pcd->m_iSrc ;
		//
		if ( pcd->m_type == RSCode::typeParenthesis )
		{
			ESLAssert( ESLTypeCast<RSParenthesis>( pcd ) != NULL ) ;
			RSParenthesis *	pPrth = ESLTypeCast<RSParenthesis>( pcd ) ;
			if ( pPrth->m_parenthesis == RSParenthesis::ptInvalid )
			{
				// #import されたブロック
				RSCodeStream	cs( *pPrth ) ;
				CompileAllStatements( block, cs, block.m_pClassSpace ) ;
				return ;
			}
		}
	}
	SError	err = errSuccess ;
	if ( pCode != NULL )
	{
		// <control-word> statement ;
		m_pCurParenthesis = cstrm.GetParenthesis() ;
		m_iSrcStatement = pCode->m_iSrc ;
		err = (this->*m_pfnCompileStatement[pCode->m_word])
								( block, cstrm, pCode->m_word ) ;
	}
	else if ( m_phase != phaseDeclaration )
	{
		RSClass *	pClass = ParseClassExpression( cstrm ) ;
		if ( pClass != NULL )
		{
			// <type-expr> <var-name>
			//	{ [= <init-expr>] | ( <arg-list> ) <function-implements> }
			err = CompileDeclareVariable
						( block, cstrm, 0, pClass, declModeAny ) ;
		}
		else if ( cstrm.NextOperator
					( RSCodeOperator::opEndOfStatement ) == NULL )
		{
			// expression ;
			if ( m_phase == phaseImplementation )
			{
				RSTypeInfo	typeTemp ;
				err = CompileExpression
					( typeTemp, block, cstrm,
							RSCodeOperator::priorityNothing ) ;
				if ( !err )
				{
					cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
				}
			}
			else
			{
				PassExpression( cstrm ) ;
				cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
			}
		}
	}
	else
	{
		PassExpression( cstrm ) ;
		cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	}
	if ( err )
	{
		PassExpression( cstrm ) ;
		cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	}
	WriteFreeAllTemporary( block ) ;
}

// 式を読み飛ばす
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::PassExpression( RSCodeStream& cs, int priority )
{
	RSContext::PassExpression( cs, priority ) ;
}

// クラス文／関数文（... {} | ... ;）を読み飛ばす
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::PassBlockStatement( RSCodeStream& cs )
{
	while ( !cs.IsEndOfStream() )
	{
		RSCode *	pCode = cs.NextTerm() ;
		if ( pCode != NULL )
		{
			if ( (pCode->m_type == RSCode::typeOperator)
				&& (((RSCodeOperator*)pCode)->m_operator
						== RSCodeOperator::opEndOfStatement) )
			{
				break ;
			}
			if ( (pCode->m_type == RSCode::typeParenthesis)
				&& (((RSParenthesis*)pCode)->m_parenthesis
									== RSParenthesis::ptBrace) )
			{
				break ;
			}
		}
	}
}

// 型式を評価
//////////////////////////////////////////////////////////////////////////////
RSClass * RSInstructionParser::ParseClassExpression( RSCodeStream& cs )
{
	return	m_context.ParseClassExpression( cs ) ;
}

// 定義文解釈／実行
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileDeclareVariable
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, uint32_t accMod,
		RSClass * pClass, uint32_t modeDecl )
{
	RSCodeSymbol *	pSymName = cstrm.NextSymbol() ;
	if ( pSymName == NULL )
	{
		RSParenthesis *	pPrth =
				cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
		if ( (pPrth != NULL)
			&& (pClass == block.m_pClassSpace)
			&& (modeDecl & declModeFunc) )
		{
			m_iSrcStatement = pPrth->m_iSrc ;
			//
			// 構築関数
			//
			return	CompileDeclareConstructor
						( block, cstrm, pPrth, accMod, pClass ) ;
		}
	}
	size_t		iCode = cstrm.GetIndex() ;
	RSCode *	pCode = cstrm.NextTerm() ;
	if ( (pCode != NULL)
		&& (pCode->m_type == RSCode::typeParenthesis)
		&& (((RSParenthesis*)pCode)->m_parenthesis
							== RSParenthesis::ptParenthesis)
		&& (modeDecl & declModeFunc) )
	{
		m_iSrcStatement = pCode->m_iSrc ;
		//
		// 関数定義解釈
		//
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
		return	CompileDeclareFunction
					( block, cstrm,
						pSymName, (RSParenthesis*) pCode, accMod, pClass ) ;
	}
	else if ( (pClass != NULL) && (modeDecl & declModeVar) )
	{
		//
		// 変数定義解釈
		//
		cstrm.SeekIndex( iCode ) ;
		for ( ; ; )
		{
			SError	err =
				CompileDeclareVariable
					( block, cstrm, pSymName, accMod, pClass ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cstrm.NextOperator( RSCodeOperator::opSequencing ) == NULL )
			{
				if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
				{
					break ;
				}
				if ( cstrm.IsEndOfStream() )
				{
					break ;
				}
				else
				{
					OutputError
						( L"変数の初期値と次の変数が"
							L" \',\' で区切られていません" ) ;
					return	errFailed ;
				}
			}
			pSymName = cstrm.NextSymbol() ;
			if ( pSymName == NULL )
			{
				OutputError
					( L"変数定義で、\',\' の後に変数名がありません" ) ;
				return	errFailed ;
			}
		}
		return	errSuccess ;
	}
	else
	{
		OutputError( L"void 変数は定義できません" ) ;
		return	errFailed ;
	}
}

// 構築関数解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileDeclareConstructor
	( FunctionBlock& block, RSCodeStream& cstrm,
		RSParenthesis * pPrthArg, uint32_t accMod, RSClass * pClass )
{
	// 引数解釈
	RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
	SParserErrorLogger		perrLog ;
	RSCodeStream			csArg( *pPrthArg ) ;
	if ( pProto->ParseArgument( m_context, csArg, perrLog ) )
	{
		if ( perrLog.GetErrorCount() > 0 )
		{
			OutputErrorLog( perrLog ) ;
		}
		else
		{
			OutputError( L"構築関数の引数の書式が不正です" ) ;
		}
		delete	pProto ;
		return	errFailed ;
	}
	//
	// コンストラクタ定義
	//
	ESLAssert( block.m_pClassSpace != NULL ) ;
	SetPrototypeAccessModifier( pProto, accMod ) ;
	pProto->m_pNamespaceClass = block.m_pClassSpace ;
	//
	// 宣言／実装
	//
	DeclareFunctionPrototype
		( block, L"<init>",
			pProto, cstrm, (accMod & ~RSObject::modifierStatic) ) ;
	return	errSuccess ;
}

// 関数定義解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileDeclareFunction
	( FunctionBlock& block,
		RSCodeStream& cstrm,
		RSCodeSymbol * pSymName,
		RSParenthesis * pPrthArg,
		uint32_t accMod, RSClass * pClass )
{
	//
	// 引数解釈
	//
	ESLAssert( pPrthArg->m_parenthesis == RSParenthesis::ptParenthesis ) ;
	//
	RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
	SParserErrorLogger		perrLog ;
	RSCodeStream			csArg( *pPrthArg ) ;
	if ( pProto->ParseArgument( m_context, csArg, perrLog ) )
	{
		if ( perrLog.GetErrorCount() > 0 )
		{
			OutputErrorLog( perrLog ) ;
		}
		else
		{
			OutputError( L"関数の引数の書式が不正です" ) ;
		}
		delete	pProto ;
		return	errFailed ;
	}
	SetPrototypeAccessModifier( pProto, accMod ) ;
	pProto->SetReturnType( pClass ) ;
	pProto->m_pNamespaceClass = block.m_pClassSpace ;
	//
	// 宣言／実装
	//
	DeclareFunctionPrototype
		( block, pSymName->m_symbol, pProto, cstrm, accMod ) ;
	//
	return	errSuccess ;
}

// 関数宣言
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::DeclareFunctionPrototype
	( FunctionBlock& block,
		const wchar_t * pwszFuncName,
		RSFunctionPrototype * pProto,
		RSCodeStream& cstrm, uint32_t accMod )
{
	//
	// 関数実装取得
	//
	if ( accMod & (RSObject::modifierAbstract
						| RSObject::modifierNative
						| RSObject::modifierExtern) )
	{
		if ( cstrm.NextOperator
			( RSCodeOperator::opEndOfStatement ) == NULL )
		{
			OutputError( L"関数宣言の文末に ; がありません" ) ;
		}
	}
	else
	{
		RSParenthesis *
			pPrth = cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
		if ( pPrth == NULL )
		{
			OutputError( L"関数の実装がありません" ) ;
		}
		pProto->m_pParenthesis = pPrth ;
		//
		cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	}
	//
	// 関数追加
	//
	RSFunctionObject *	pFunc = NULL ;
	RSClass *	pDefClass = block.m_pClassSpace ;
	if ( pDefClass != NULL )
	{
		if ( m_phase != phaseDefinition )
		{
			delete	pProto ;
			return ;
		}
		if ( !(accMod & RSObject::modifierStatic) )
		{
			//
			// 仮想関数
			//
			pFunc = pDefClass->GetVirtualMemberAs( m_context, pwszFuncName ) ;
			if ( pFunc == NULL )
			{
				pFunc = new RSFunctionObject
							( m_vm->GetFunctionClassAs( *pProto ) ) ;
				pDefClass->SetVirtualMemberAs
					( m_context, pwszFuncName, pFunc ) ;
				pFunc->AddRef() ;
				pFunc->SetModifiers( accMod | RSObject::modifierConst ) ;
			}
		}
		else
		{
			//
			// 静関数
			//
			RSObject *	pObjFunc =
				pDefClass->GetMemberAs( m_context, pwszFuncName ) ;
			m_context.ClearException() ;
			if ( pObjFunc != NULL )
			{
				RSObject *	pObjEntity = pObjFunc->GetEntityObject() ;
				if ( (pObjEntity == NULL)
					|| (pObjEntity->GetBasicType() != RSObject::typeFunction) )
				{
					m_context.ReleaseObjectRef( pObjFunc ) ;
					OutputError( L"関数名が重複しています" ) ;
					return ;
				}
				ESLAssert( pObjEntity->IsKindOf
							( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
				pFunc = (RSFunctionObject*) pObjEntity ;
			}
			else
			{
				pFunc = new RSFunctionObject
							( m_vm->GetFunctionClassAs( *pProto ) ) ;
				pFunc->m_strFuncName = pwszFuncName ;
				pFunc->m_pFuncClass = pDefClass ;
				//
				pDefClass->CreateMemberAs
						( m_context, pwszFuncName, pFunc ) ;
				pFunc->SetModifiers( accMod | RSObject::modifierConst ) ;
			}
		}
	}
	else
	{
		//
		// 関数
		//
		if ( m_phase != phaseAllocation )
		{
			delete	pProto ;
			return ;
		}
		pFunc = new RSFunctionObject
					( m_vm->GetFunctionClassAs( *pProto ) ) ;
		pFunc->m_strFuncName = pwszFuncName ;
		pFunc->AddRef() ;
		//
		if ( CreateVariableImmediateAs
			( block, pwszFuncName,
				(accMod | RSObject::modifierConst), pFunc ) )
		{
			pFunc->ReleaseRef() ;
			return ;
		}
		pFunc->SetModifiers( accMod | RSObject::modifierConst ) ;
	}
	//
	// プロトタイプ追加
	//
	ESLAssert( pFunc != NULL ) ;
	RSFunctionPrototype *
		pProtoDef = pFunc->GetEqualArgumentPrototype( *pProto ) ;
	if ( pProtoDef != NULL )
	{
		if ( pProtoDef->m_pReturnType != pProto->m_pReturnType )
		{
			OutputError
				( L"関数の引数が一致していますが返り値型が一致していません" ) ;
		}
		if ( (pProtoDef->m_pNamespaceClass == pProto->m_pNamespaceClass)
			&& (pProtoDef->m_pParenthesis != NULL) )
		{
			OutputError( L"関数の二重実装です" ) ;
			delete	pProto ;
			pFunc->ReleaseRef() ;
			return ;
		}
	}
	pFunc->AddPrototype( pProto ) ;
	pFunc->ReleaseRef() ;
	//
	if ( m_psoaFunc.GetAs( pProto ) == NULL )
	{
		FunctionAppendix *
			pAppendix = new FunctionAppendix( pProto, NULL ) ;
		m_psoaFunc.SetAs( pProto, pAppendix ) ;
	}
}

// 変数定義解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileDeclareVariable
	( FunctionBlock& block,
		RSCodeStream& cstrm,
		RSCodeSymbol * pSymName,
		uint32_t accMod, RSClass * pClass )
{
	RSStructuredPointerClass *	pDefStruct = NULL ;
	bool			fStaticMember = false ;
	CompilePhase	pahseInit = phaseImplementation ;
	if ( block.m_pClassSpace != NULL )
	{
		if ( !(accMod & RSObject::modifierStatic) )
		{
			pDefStruct =
				ESLTypeCast<RSStructuredPointerClass>( block.m_pClassSpace ) ;
			if ( pDefStruct != NULL )
			{
				//
				// 構造体メンバ
				//
				RSParenthesis *	pPrthArray =
					cstrm.NextParenthesis( RSParenthesis::ptBracket ) ;
				size_t		nArray = 0 ;
				RSObject *	pInitObj = NULL ;
				if ( pPrthArray != NULL )
				{
					RSCodeStream	csSize( *pPrthArray ) ;
					RSTypeInfo		typeSize ;
					SError	err = CompileExpression
						( typeSize, block, csSize,
							RSCodeOperator::priorityNothing, exprNoOutputCode ) ;
					if ( err )
					{
						return	err ;
					}
					int64_t	numSize ;
					if ( (typeSize.m_pImmediate == NULL)
						|| !typeSize.m_pImmediate->AsInteger( numSize ) )
					{
						OutputError( L"構造体メンバの配列長が不正です" ) ;
						return	errFailed ;
					}
					nArray = (size_t) numSize ;
				}
				if ( m_phase == phaseDefinition )
				{
					pDefStruct->AddArrayMemberAs
						( m_context, pSymName->m_symbol,
							pClass, accMod, nArray, pInitObj ) ;
				}
				if ( cstrm.NextParenthesis( RSParenthesis::ptBracket ) != NULL )
				{
					OutputError
						( L"構造体メンバに多次元配列は使用できません" ) ;
					return	errFailed ;
				}
			}
			else
			{
				//
				// クラスメンバ
				//
				if ( m_phase == phaseDefinition )
				{
					if ( block.m_pClassSpace->m_pPrototype == NULL )
					{
						block.m_pClassSpace->m_pPrototype =
								new RSGenericObject( block.m_pClassSpace ) ;
					}
					RSObject *	pVar = pClass->NewVariable( m_context ) ;
					pVar->SetModifiers( accMod ) ;
					//
					m_context.ReleaseObjectRef
						( block.m_pClassSpace->m_pPrototype->
							CreateMemberAs
								( m_context, pSymName->m_symbol, pVar ) ) ;
				}
			}
		}
		else
		{
			//
			// static メンバ
			//
			if ( m_phase == phaseDefinition )
			{
				RSObject *	pVar = pClass->NewVariable( m_context ) ;
				pVar->SetModifiers( accMod ) ;
				//
				m_context.ReleaseObjectRef
					( block.m_pClassSpace->CreateMemberAs
						( m_context, pSymName->m_symbol, pVar ) ) ;
			}
			pahseInit = phaseDefinition ;
			fStaticMember = true ;
		}
	}
	else
	{
		//
		// ローカル変数定義
		//
		CreateVariableTypeAs
			( block, pSymName->m_symbol, accMod, pClass ) ;
		//
		if ( accMod & RSObject::modifierStatic )
		{
			OutputError( L"static は不正な修飾です" ) ;
		}
	}
	if ( cstrm.NextOperator( RSCodeOperator::opMove ) != NULL )
	{
		if ( pDefStruct != NULL )
		{
			OutputError
				( L"構造体メンバの初期値が記述されています" ) ;
			return	errFailed ;
		}
		if ( m_phase == pahseInit )
		{
			//
			// 初期値計算コード
			//
			RSTypeInfo	typeInit ;
			uint32_t	nFlags = 0 ;
			if ( fStaticMember )
			{
				nFlags |= exprNoOutputCode ;
			}
			SError	err =
				CompileExpression
					( typeInit, block, cstrm,
						RSCodeOperator::priorityList, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileTypeCast
				( typeInit, block, nFlags, pClass, castNatural, typeInit ) ;
			if ( err )
			{
				return	err ;
			}
			RSTypeInfo	typeVar ;
			if ( block.m_pClassSpace != NULL )
			{
				if ( !(accMod & RSObject::modifierStatic) )
				{
					// メンバ変数初期値
					RSTypeInfo	typeThis ;
					err = CompileLoadThis( typeThis, block ) ;
					if ( err )
					{
						return	err ;
					}
					RSTypeInfo	typeMember ;
					err = CompileReferenceMember
						( typeMember, block, cstrm,
							0, typeThis, pSymName->m_symbol ) ;
					if ( err )
					{
						return	err ;
					}
					RSInstruction *	pInstMove = new RSInstruction ;
					pInstMove->BinOperator
						( m_context, RSCodeOperator::opMove,
								pClass, typeMember, typeInit ) ;
					block.AddCode( pInstMove ) ;
				}
				else
				{
					// static メンバ初期値
					ESLAssert( typeInit.m_pImmediate != NULL ) ;
					if ( typeInit.m_pImmediate != NULL )
					{
						RSObject *	pInitObj =
							typeInit.m_pImmediate->CloneObject( m_context ) ;
						m_context.ReleaseObjectRef
							( block.m_pClassSpace->SetMemberAs
								( m_context, pSymName->m_symbol, pInitObj ) ) ;
						TestContextException( m_context ) ;
					}
					else
					{
						OutputError
							( L"static メンバ変数の初期値が実行時式です" ) ;
					}
				}
			}
			else
			{
				// ローカル変数初期値
				err = CompileMoveToLocalReference
						( block, pSymName->m_symbol, typeInit ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else
		{
			PassExpression( cstrm, RSCodeOperator::priorityList ) ;
		}
	}
	return	errSuccess ;
}

// ローカル変数作成（即値）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CreateVariableImmediateAs
	( FunctionBlock& block,
		const wchar_t * pwszName, uint32_t accMod, RSObject * pObj )
{
	if ( m_phase != phaseAllocation )
	{
		return	errSuccess ;
	}
	RSTypeInfo *	pti = new RSTypeInfo ;
	pti->SetImmediate( m_context, pObj, pObj->GetRSClass() ) ;
	pti->m_accMod = accMod ;
	//
	CreateVariableTypeAs( block, pwszName, pti ) ;
	//
	return	errSuccess ;
}

// ローカル変数作成
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::CreateVariableTypeAs
	( FunctionBlock& block,
		const wchar_t * pwszName,
		uint32_t accMod, RSClass * pType )
{
	if ( m_phase != phaseAllocation )
	{
		return ;
	}
	RSTypeInfo *	pti = new RSTypeInfo ;
	pti->SetType( m_context, pType ) ;
	pti->m_accMod = accMod ;
	//
	CreateVariableTypeAs( block, pwszName, pti ) ;
}

void RSInstructionParser::CreateVariableTypeAs
	( FunctionBlock& block,
		const wchar_t * pwszName, RSTypeInfo * ptiVar )
{
	if ( m_phase == phaseImplementation )
	{
		RSTypeInfo *
			ptiDefined = block.m_local.ReAddVariable( pwszName ) ;
		if ( ptiDefined != NULL )
		{
			if ( ptiDefined->m_pClass != ptiVar->m_pClass )
			{
				OutputError
					( SString(L"内部エラー：\'")
						+ SString(pwszName) + L"\' の型が一致しません" ) ;
			}
			else
			{
				delete	ptiVar ;
			}
			return ;
		}
	}
	if ( (m_phase == phaseAllocation)
		|| (m_phase == phaseImplementation) )
	{
		RSFramePointer	fpSize ;
		ptiVar->GetLocalFrameSize( fpSize ) ;
		//
		if ( fpSize.iLocal > 0 )
		{
			ptiVar->m_fp.iLocal = block.m_fpLocal.iLocal ;
			block.m_fpLocal.iLocal += fpSize.iLocal ;
		}
		if ( fpSize.iNumber > 0 )
		{
			ptiVar->m_fp.iNumber = block.m_fpLocal.iNumber ;
			block.m_fpLocal.iNumber += fpSize.iNumber ;
		}
		if ( fpSize.iPointer > 0 )
		{
			ptiVar->m_fp.iPointer = block.m_fpLocal.iPointer ;
			block.m_fpLocal.iPointer += fpSize.iPointer ;
		}
		if ( block.m_local.GetVariableAs( pwszName ) != NULL )
		{
			OutputError
				( SString(L"\'")
					+ SString(pwszName) + L"\' の二重定義です" ) ;
		}
		block.m_local.AddVariable( pwszName, ptiVar ) ;
	}
	else
	{
		delete	ptiVar ;
	}
}

// 無名ローカル変数作成
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::CreateNamelessVariableAs
	( FunctionBlock& block,
		SSystem::SString& strTempName, RSTypeInfo * ptiVar )
{
	do
	{
		strTempName.Format( L"@%05d", block.m_local.m_iTempVar ++ ) ;
	}
	while ( block.m_local.GetVariableAs( strTempName ) != NULL ) ;
	//
	CreateVariableTypeAs( block, strTempName, ptiVar ) ;
}

// 関数修飾子設定
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::SetPrototypeAccessModifier
	( RSFunctionPrototype * pProto, uint32_t& accMod )
{
	if ( accMod & RSObject::modifierSynchronized )
	{
		pProto->m_nFlags |= RSFunctionPrototype::flagSynchronized ;
		accMod &= ~RSObject::modifierSynchronized ;
	}
	if ( accMod & RSObject::modifierConst )
	{
		pProto->m_nFlags |= RSFunctionPrototype::flagConstant ;
		accMod &= ~RSObject::modifierConst ;
	}
	if ( accMod & RSObject::modifierAbstract )
	{
		pProto->m_nFlags |= RSFunctionPrototype::flagAbstract ;
		accMod &= ~RSObject::modifierAbstract ;
	}
	switch ( accMod & RSObject::accessMask )
	{
	case	RSObject::modifierPublic:
		pProto->m_nFlags |= RSFunctionPrototype::flagAccessPublic ;
		accMod &= ~RSObject::modifierPublic ;
		break ;
	case	RSObject::modifierProtected:
		pProto->m_nFlags |= RSFunctionPrototype::flagAccessProtected ;
		accMod &= ~RSObject::modifierProtected ;
		break ;
	case	RSObject::modifierPrivate:
		pProto->m_nFlags |= RSFunctionPrototype::flagAccessPrivate ;
		accMod &= ~RSObject::modifierPrivate ;
		break ;
	}
}

// ローカル変数代入処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileMoveToLocalReference
	( FunctionBlock& block,
		const wchar_t * pwszDstName, const RSTypeInfo& typeSrc )
{
	RSTypeInfo		typeVar ;
	RSCodeStream	csDummy ;
	SError	err =
		CompileLocalReference( typeVar, block, csDummy, pwszDstName, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	RSInstruction *	pInstMove = new RSInstruction ;
	pInstMove->BinOperator
		( m_context, RSCodeOperator::opMove,
				typeVar.m_pClass, typeVar, typeSrc ) ;
	block.AddCode( pInstMove ) ;
	return	errSuccess ;
}

// 数式評価
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileExpression
	( RSTypeInfo& typeResult,
		FunctionBlock& block,
		RSCodeStream& cs, int priority, uint32_t nFlags )
{
	//
	// 第一項解釈
	//
	RSCode *	pCode = cs.NextTerm() ;
	if ( pCode == NULL )
	{
		OutputError( L"式の解釈中に文末に到達しました" ) ;
		return	errFailed ;
	}
	m_iSrcStatement = pCode->m_iSrc ;
	typeResult = RSTypeInfo() ;
	//
	SError			err ;
	RSCode *		pNextCode ;
	RSClass *		pThisClass ;
	RSInstruction *	pInst = NULL ;
	RSTypeInfo		typeLast ;
	switch ( pCode->m_type )
	{
	case	RSCode::typeLiteral:
		// 即値
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeLiteral) ) ) ;
		ESLAssert( ((RSCodeLiteral*)pCode)->m_literal != NULL ) ;
		{
			RSObject *	pObj =
				((RSCodeLiteral*)pCode)->
						m_literal->DuplicateObject( m_context ) ;
			typeLast.SetImmediate
				( m_context, pObj, pObj->GetRSClass() ) ;
			m_typeExprThis = RSTypeInfo() ;
		}
		break ;

	case	RSCode::typeControlCode:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeControl) ) ) ;
		switch ( ((RSCodeControl*)pCode)->m_word )
		{
		case	RSCodeControl::wiThis:
			// this
			err = CompileLoadThis( typeLast, block, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			break ;

		case	RSCodeControl::wiFunction:
			// function(...) : type -> class { ... }
			err = CompileNamelessFunction
				( typeLast, block, cs, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			break ;

		case	RSCodeControl::wiSuper:
			// super
			pThisClass = block.m_pProto->m_pNamespaceClass ;
			if ( pThisClass == NULL )
			{
				OutputError( L"this オブジェクトがありません" ) ;
				return	errFailed ;
			}
			if ( (pThisClass == NULL)
				|| (pThisClass->m_pSuperClass == NULL) )
			{
				OutputError( L"super は存在しません" ) ;
				return	errFailed ;
			}
			pNextCode = cs.GetTerm() ;
			if ( (pNextCode != NULL)
				&& (pNextCode->m_type == RSCode::typeParenthesis)
				&& (((RSParenthesis*)pNextCode)->
						m_parenthesis == RSParenthesis::ptParenthesis) )
			{
				// super()
				err = CompileSuperConstructor
					( typeLast, block, cs,
						nFlags, *((RSParenthesis*)pNextCode) ) ;
				if ( err )
				{
					return	err ;
				}
				m_typeExprThis = RSTypeInfo() ;
				break ;
			}
			else if ( (pNextCode != NULL)
				&& (pNextCode->m_type == RSCode::typeOperator)
				&& (((RSCodeOperator*)pNextCode)->
						m_operator == RSCodeOperator::opMemberOf) )
			{
				// super.member
				err = CompileSuperMember
						( typeLast, block, cs, priority, nFlags,
							((RSCodeSymbol*)pNextCode)->m_symbol ) ;
				if ( err )
				{
					return	err ;
				}
				break ;
			}

		default:
			if ( (((RSCodeControl*)pCode)->m_word >= RSCodeControl::wiFirstBasicType)
				&& (((RSCodeControl*)pCode)->m_word <= RSCodeControl::wiLastBasicType) )
			{
				// 基本型
				RSCodeControl::WordIndex
							wi = ((RSCodeControl*)pCode)->m_word ;
				RSClass *	pClass = m_context.GetBasicTypeClass( wi ) ;
				if ( pClass == NULL )
				{
					OutputError
						( SString(RSCodeControl::ControlWordAt(wi))
									+ L" は定義されていない基本型です" ) ;
					return	errFailed ;
				}
				pClass->AddRef() ;
				typeLast.SetImmediate
					( m_context, pClass, pClass->GetRSClass() ) ;
				m_typeExprThis = RSTypeInfo() ;
			}
			else
			{
				OutputError( L"数式中に不正な予約語が含まれています" ) ;
				return	errFailed ;
			}
			break ;
		}
		break ;

	case	RSCode::typeOperator:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
		{
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnary )
			{
				if ( pOpCode->m_operator == RSCodeOperator::opNew )
				{
					// new 演算子
					err = CompileNewOperator( typeLast, block, cs, nFlags ) ;
					if ( err )
					{
						return	err ;
					}
				}
				else
				{
					// 単項演算子
					err = CompileExpression
						( typeLast, block, cs, opinf.priorityUnary, nFlags ) ;
					if ( err )
					{
						return	err ;
					}
					err = CompileUnaryOperator
							( typeLast, block,
								nFlags, pOpCode->m_operator, false ) ;
					if ( err )
					{
						return	err ;
					}
				}
			}
			else
			{
				OutputError
					( SString(pOpCode->GetOperatorString())
									+ L" 演算子には左辺が必要です" ) ;
				return	errFailed ;
			}
		}
		break ;

	case	RSCode::typeParenthesis:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
		{
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			switch ( pPrth->m_parenthesis )
			{
			case	RSParenthesis::ptParenthesis:
				// ( expr )
				{
					RSCodeStream	csPrth( *pPrth ) ;
					err = CompileExpression
						( typeLast, block, csPrth,
							RSCodeOperator::priorityNothing, nFlags ) ;
					if ( err )
					{
						return	err ;
					}
					m_typeExprThis = RSTypeInfo() ;
					//
					if ( (typeLast.m_pImmediate != NULL)
						&& (typeLast.m_pImmediate->
								GetBasicType() == RSObject::typeClass)
						&& !cs.IsEndOfStream() )
					{
						// (type) 型キャスト
						ESLAssert( typeLast.m_pImmediate->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
						RSClass *	pClass = (RSClass*) typeLast.m_pImmediate ;
						//
						RSTypeInfo	typeExpr ;
						err = CompileExpression
							( typeExpr, block, cs,
								RSCodeOperator::priorityUnary, nFlags ) ;
						if ( err )
						{
							return	err ;
						}
						err = CompileTypeCast
							( typeLast, block, nFlags,
								pClass, castForce, typeExpr ) ;
						if ( err )
						{
							return	err ;
						}
					}
				}
				break ;

			case	RSParenthesis::ptBracket:
				// [ expr, expr, ... ]
				err = CompileArrayExpression
						( typeLast, block, *pPrth, nFlags ) ;
				if ( err )
				{
					return	err ;
				}
				m_typeExprThis = RSTypeInfo() ;
				break ;

			case	RSParenthesis::ptBrace:
				// { id : expr, ... }
				err = CompileHashMapExpression
						( typeLast, block, *pPrth, nFlags ) ;
				if ( err )
				{
					return	err ;
				}
				m_typeExprThis = RSTypeInfo() ;
				break ;

			default:
				break;
			}
		}
		break ;

	case	RSCode::typeSymbol:
		// 任意シンボル
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
		err = CompileAutoReferenceSymbol
			( typeLast, block, cs, ((RSCodeSymbol*)pCode)->m_symbol, nFlags ) ;
		if ( err )
		{
			return	err ;
		}
		break ;

	default:
		OutputError( L"内部エラー：未定義コードを解釈できません" ) ;
		return	errFailed ;
	}
	//
	// 第二項以降解釈
	//
	for ( ; ; )
	{
		pCode = cs.GetTerm() ;
		if ( pCode == NULL )
		{
			break ;
		}
		if ( pCode->m_type == RSCode::typeOperator )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnaryPost )
			{
				//
				// 後置単項演算子
				//
				if ( opinf.priorityUnaryPost <= priority )
				{
					break ;
				}
				err = CompileUnaryOperator
					( typeLast, block, nFlags, pOpCode->m_operator, true ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else if ( CompileGenericClass
						( typeLast, block, cs, nFlags, pOpCode ) )
			{
				// HashMap<type>
				continue ;
			}
			else if ( opinf.flagsRule & RSCodeOperator::ruleBinary )
			{
				//
				// 二項演算子
				//
				if ( opinf.priorityBinary <= priority )
				{
					break ;
				}
				RSCodeOperator::OperatorIndex	iOp = pOpCode->m_operator ;
				if ( opinf.flagsRule & RSCodeOperator::ruleSpecialRight )
				{
					if ( (iOp == RSCodeOperator::opStaticMemberOf)
								|| (iOp == RSCodeOperator::opMemberOf) )
					{
						// メンバ参照
						err = CompileOperatorReferenceMember
							( typeLast, block, cs, nFlags, iOp ) ;
						if ( err )
						{
							return	err ;
						}
					}
					else if ( iOp == RSCodeOperator::opLogicalAnd )
					{
						// expr && expr
						err = CompileOperatorLogicalAnd
							( typeLast, block, cs, nFlags ) ;
						if ( err )
						{
							return	err ;
						}
					}
					else if ( iOp == RSCodeOperator::opLogicalOr )
					{
						// expr || expr
						err = CompileOperatorLogicalOr
							( typeLast, block, cs, nFlags ) ;
						if ( err )
						{
							return	err ;
						}
					}
					else if ( iOp == RSCodeOperator::opConditional )
					{
						// expr ? expr : expr
						err = CompileOperatorSelect
							( typeLast, block, cs, nFlags ) ;
						if ( err )
						{
							return	err ;
						}
					}
					else
					{
						OutputError
							( SString(L"内部エラー：")
								+ RSCodeOperator::m_pwszOperators[iOp]
								+ L" は未定義の特殊演算子です" ) ;
						return	errFailed ;
					}
				}
				else
				{
					//
					// 一般的な二項演算子
					//
					int	nOpPriority = opinf.priorityBinary ;
					if ( opinf.flagsRule & RSCodeOperator::ruleRightToLeft )
					{
						nOpPriority -- ;
					}
					RSTypeInfo	typeRight ;
					err = CompileExpression
						( typeRight, block, cs, nOpPriority, nFlags ) ;
					if ( err )
					{
						return	err ;
					}
					err = CompileBinaryOperator
							( typeLast, block, nFlags,
								typeRight, pOpCode->m_operator ) ;
					if ( err )
					{
						return	err ;
					}
				}
			}
			else
			{
				OutputError
					( L"数式の書式エラー：二項演算子の用法が不正です" ) ;
				return	errFailed ;
			}
		}
		else if ( pCode->m_type == RSCode::typeParenthesis )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			if ( pPrth->m_parenthesis == RSParenthesis::ptParenthesis )
			{
				//
				// expr(...)
				//
				err = CompileCallFunction( typeLast, block, nFlags, *pPrth ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else if ( pPrth->m_parenthesis == RSParenthesis::ptBracket )
			{
				if ( (typeLast.m_pImmediate != NULL)
					&& (typeLast.m_pImmediate->
							GetBasicType() == RSObject::typeClass) )
				{
					//
					// type[]
					//
					ESLAssert( typeLast.m_pImmediate->
									IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
					RSClass *	pClass = (RSClass*) typeLast.m_pImmediate ;
					pClass = m_vm->GetArrayClassAs( pClass ) ;
					//
					pClass->AddRef() ;
					typeLast.SetImmediate
						( m_context, pClass, pClass->GetRSClass() ) ;
				}
				else
				{
					//
					// expr[expr]
					//
					err = CompileReferenceElement
							( typeLast, block, nFlags, *pPrth ) ;
					if ( err )
					{
						return	err ;
					}
				}
			}
			else
			{
				OutputError( L"書式エラー：{} 括弧が不正です" ) ;
				return	errFailed ;
			}
		}
		else
		{
			OutputError( L"書式エラー：項間に演算子がありません" ) ;
			return	errFailed ;
		}
	}
	typeResult = typeLast ;
	return	errSuccess ;
}

// コード出力検証
//////////////////////////////////////////////////////////////////////////////
bool RSInstructionParser::VerifyExprOutputCode( uint32_t nFlags )
{
	if ( nFlags & exprNoOutputCode )
	{
		OutputError( L"コンパイル時に式を評価できません" ) ;
		return	false ;
	}
	return	true ;
}

// RSContext 例外をエラー出力
//////////////////////////////////////////////////////////////////////////////
bool RSInstructionParser::TestContextException( RSContext& context )
{
	if ( m_context.IsException() )
	{
		RSSmartPtr	pException( context.PopException() ) ;
		SString		strException ;
		if ( (pException != NULL)
			&& pException->AsString( strException ) )
		{
			OutputError( strException ) ;
			return	false ;
		}
	}
	return	true ;
}

// 一時計算領域へ割り当てられていない場合、割り当ててロード命令を出力
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::NormalizeAllocation
	( FunctionBlock& block, uint32_t nFlags, RSTypeInfo& typeTemp )
{
	if ( !typeTemp.IsAllocated() )
	{
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSTypeInfo	typeDst ;
		typeDst.SetType( m_context, typeTemp.m_pClass ) ;
		//
		RSInstruction *	pInst = new RSInstruction ;
		pInst->MoveOperator
			( m_context, RSCodeOperator::opMove, typeDst, typeTemp ) ;
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		typeTemp = pInst->m_typeDst ;
	}
	return	errSuccess ;
}

// 一時計算用変数割り当て
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::AllocateTemporary
	( RSInstructionParser::FunctionBlock& block, RSTypeInfo& typeTemp )
{
	RSFramePointer	fpSize ;
	typeTemp.GetLocalFrameSize( fpSize ) ;
	//
	if ( fpSize.iLocal > 0 )
	{
		typeTemp.m_fp.iLocal = block.m_fpExpr.iLocal ;
		block.m_fpExpr.iLocal += fpSize.iLocal ;
	}
	if ( fpSize.iNumber > 0 )
	{
		typeTemp.m_fp.iNumber = block.m_fpExpr.iNumber ;
		block.m_fpExpr.iNumber += fpSize.iNumber ;
	}
	if ( fpSize.iPointer > 0 )
	{
		typeTemp.m_fp.iPointer = block.m_fpExpr.iPointer ;
		block.m_fpExpr.iPointer += fpSize.iPointer ;
	}
}

// 一時計算用割り当てを解放し、一時的な参照を解放するコード出力
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::WriteFreeAllTemporary( RSInstructionParser::FunctionBlock& block )
{
	for ( int i = block.m_fpLocal.iLocal; i < block.m_fpExpr.iLocal; i ++ )
	{
		RSInstruction *	pInst = new RSInstruction ;
		pInst->m_code = RSInstruction::codeFree ;
		pInst->m_typeSrc1.m_fp.iLocal = i ;
		block.AddCode( pInst ) ;
	}
	block.FreeAllTemporary() ;
	//
	RSInstruction *	pInst = new RSInstruction ;
	pInst->FenceStack( block.m_fpLocal, block.m_fpExpr ) ;
	block.AddCode( pInst ) ;
}

// 文の文脈初期化
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::ResetStatementContext( FunctionBlock& block )
{
	block.FreeAllTemporary() ;
	m_typeExprThis = RSTypeInfo() ;
}

// this インスタンス取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileLoadThis
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags )
{
	if ( block.m_pProto->m_pNamespaceClass == NULL )
	{
		OutputError( L"this オブジェクトがありません" ) ;
		return	errFailed ;
	}
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	RSFramePointer	fp ;
	if ( ESLTypeCast<RSTypedArrayPointerClass>
				( block.m_pProto->m_pNamespaceClass ) != NULL )
	{
		fp.iPointer = localIndexThis ;
	}
	else
	{
		fp.iLocal = localIndexThis ;
	}
	//
	RSInstruction *	pInst = new RSInstruction ;
	pInst->LoadLocal
		( m_context, block.m_pProto->m_pNamespaceClass, fp ) ;
	block.AddCode( pInst ) ;
	AllocateTemporary( block, pInst->m_typeDst ) ;
	typeResult = pInst->m_typeDst ;
	//
	if ( block.m_pProto
		&& block.m_pProto->IsConstantModifier() )
	{
		typeResult.m_accMod |= RSObject::modifierConst ;
	}
	m_typeExprThis = RSTypeInfo() ;
	return	errSuccess ;
}

// 無名 function インスタンス式
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileNamelessFunction
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags )
{
	//
	// function(...) : type -> class { ... }
	//
	RSParenthesis *	pPrth =
			cs.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrth == NULL )
	{
		OutputError( L"関数に引数がありません" ) ;
		return	errFailed ;
	}
	//
	// 引数解釈
	//
	RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
	SParserErrorLogger		perrLog ;
	RSCodeStream			csArg( *pPrth ) ;
	if ( pProto->ParseArgument( m_context, csArg, perrLog ) )
	{
		SParserErrorLogger::ErrorLog *	pLog = perrLog.GetErrorLogAt( 0 ) ;
		if ( pLog != NULL )
		{
			OutputError( pLog->m_strError ) ;
		}
		else
		{
			OutputError( L"関数の引数の書式が不正です" ) ;
		}
		delete	pProto ;
		return	errFailed ;
	}
	if ( cs.NextOperator( RSCodeOperator::opSeparator ) != NULL )
	{
		//
		// 返り値型
		//
		RSClass *	pRetType = ParseClassExpression( cs ) ;
		if ( pRetType != NULL )
		{
			pProto->SetReturnType( pRetType ) ;
		}
	}
	if ( cs.NextOperator( RSCodeOperator::opPointerMemberOf ) != NULL )
	{
		//
		// this クラス
		//
		RSClass *	pThisClass = ParseClassExpression( cs ) ;
		if ( pThisClass != NULL )
		{
			pProto->m_pNamespaceClass = pThisClass ;
		}
	}
	pProto->m_pParenthesis = cs.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pProto->m_pParenthesis == NULL )
	{
		OutputError( L"関数の実装がありません" ) ;
		delete	pProto ;
		return	errFailed ;
	}
	//
	// 関数インスタンス
	//
	RSFunctionObject *	pFunc =
		new RSFunctionObject( m_vm->GetFunctionClassAs( *pProto ) ) ;
	pFunc->AddPrototype( pProto ) ;
	//
	block.AddNamelessFunction( pFunc ) ;
	//
	pFunc->AddRef() ;
	typeResult.SetImmediate( m_context, pFunc, pFunc->GetRSClass() ) ;
	m_typeExprThis = RSTypeInfo() ;
	//
	if ( m_psoaFunc.GetAs( pProto ) == NULL )
	{
		FunctionAppendix *
			pAppendix = new FunctionAppendix( pProto, &block ) ;
		m_psoaFunc.SetAs( pProto, pAppendix ) ;
		//
		CompilePhase	phaseOld = m_phase ;
		m_phase = phaseAllocation ;
		CompileFunctionImplementation( pAppendix ) ;
		m_phase = phaseOld ;
	}
	return	errSuccess ;
}

// super 構築関数呼び出し式
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileSuperConstructor
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, RSCodeStream& cs,
		uint32_t nFlags, RSParenthesis& prthArg )
{
	//
	// コンストラクタ取得
	//
	RSClass *	pThisClass = block.m_pProto->m_pNamespaceClass ;
	ESLAssert( pThisClass != NULL ) ;
	//
	if ( pThisClass->GetConstructor() != block.m_pProto->m_pFuncGroup )
	{
		OutputError
			( L"super() の呼び出し元がコンストラクタではありません" ) ;
		return	errFailed ;
	}
	if ( !block.m_fPreConstruction )
	{
		OutputError
			( L"super() の呼び出しがコンストラクタの先頭ではありません" ) ;
		return	errFailed ;
	}
	//
	RSClass *	pSuperClass = pThisClass->m_pSuperClass ;
	ESLAssert( pSuperClass != NULL ) ;
	//
	RSFunctionObject *	pConstructor = pSuperClass->GetConstructor() ;
	if ( pConstructor == NULL )
	{
		OutputError( L"super クラスにコンストラクタは存在しません" ) ;
		return	errFailed ;
	}
	//
	// 引数解釈
	//
	SObjectArray<RSTypeInfo>	aArgType ;
	SError	err = CompileArgument( aArgType, block, nFlags, prthArg ) ;
	if ( err )
	{
		return	err ;
	}
	ssize_t	iProto = FindMatchFunctionPrototype( *pConstructor, aArgType ) ;
	if ( iProto < 0 )
	{
		OutputError( L"呼び出し引数に適合するコンストラクタが見つかりません" ) ;
		return	errFailed ;
	}
	//
	// 関数呼び出し
	//
	RSTypeInfo	typeThis ;
	err = CompileLoadThis( typeThis, block, nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCallDirectFunction
		( typeResult, block, nFlags,
			pConstructor, (size_t) iProto, typeThis, aArgType ) ;
	//
	m_typeExprThis = RSTypeInfo() ;
	return	err ;
}

// super メンバ参照式
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileSuperMember
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, int priority, uint32_t nFlags,
		const SSystem::SString& strMember )
{
	SError		err ;
	RSClass *	pThisClass = block.m_pProto->m_pNamespaceClass ;
	ESLAssert( pThisClass != NULL ) ;
	//
	RSClass *	pSuperClass = pThisClass->m_pSuperClass ;
	ESLAssert( pSuperClass != NULL ) ;
	//
	RSFunctionObject *	pFunc =
		pSuperClass->GetVirtualMemberAs( m_context, strMember ) ;
	if ( pFunc != NULL )
	{
		// 親クラスの仮想関数
		RSParenthesis *	pPrthArg =
				cs.NextParenthesis( RSParenthesis::ptParenthesis ) ;
		if ( pPrthArg != NULL )
		{
			RSSmartPtr	ptrFunc( pFunc ) ;
			//
			// super.member( ... )
			//
			SObjectArray<RSTypeInfo>	aArgType ;
			err = CompileArgument( aArgType, block, nFlags, *pPrthArg ) ;
			if ( err )
			{
				return	err ;
			}
			ssize_t	iProto = FindMatchFunctionPrototype( *pFunc, aArgType ) ;
			if ( iProto < 0 )
			{
				OutputError( L"呼び出し引数に適合する関数が見つかりません" ) ;
				return	errFailed ;
			}
			//
			// メンバ関数呼び出し
			//
			RSTypeInfo	typeThis ;
			err = CompileLoadThis( typeThis, block, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileCallDirectFunction
				( typeResult, block, nFlags,
					pFunc, (size_t) iProto, typeThis, aArgType ) ;
			//
			m_typeExprThis = RSTypeInfo() ;
			return	err ;
		}
		else
		{
			//
			// super.member
			//
			RSFunctionPrototype *
					pProto = pFunc->m_arrPrototypes.GetAt( 0 ) ;
			if ( (pProto == NULL)
				|| (pFunc->m_arrPrototypes.GetLength() > 1) )
			{
				OutputError
					( L"同名の関数が複数存在するため関数を取得できません" ) ;
				return	errFailed ;
			}
			typeResult.SetImmediate
				( m_context, pFunc,
					m_vm->GetFunctionClassAs( *pProto ), 0 ) ;
			return	errSuccess ;
		}
	}
	//
	// クラス静メンバ検索
	//
	err = CompileClassStaticMember
		( typeResult, block, cs, pSuperClass, strMember, nFlags ) ;
	if ( err != errContinue )
	{
		return	err ;
	}
	//
	// this.member 同等
	//
	RSTypeInfo	typeThis ;
	err = CompileLoadThis( typeThis, block, nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileReferenceMember
		( typeResult, block, cs, nFlags, typeThis, strMember ) ;
	if ( err != errContinue )
	{
		return	err ;
	}
	OutputError
		( SString(L"\'") + strMember + L"\' は \'"
			+ pSuperClass->GetFullClassName()
			+ L"\' のメンバではありません" ) ;
	return	errFailed ;
}

// new 演算子
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileNewOperator
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags )
{
	//
	// new <class-expr> ( ... )
	//
	RSClass *	pClass = m_context.ParseClassExpression( cs ) ;
	if ( pClass == NULL )
	{
		OutputError( L"new 演算子で構築型指定が不正です" ) ;
		return	errFailed ;
	}
	//
	// 引数取得
	//
	RSParenthesis *	pPrthArg =
			cs.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthArg == NULL )
	{
		OutputError( L"new 演算子には構築関数引数が必要です" ) ;
		return	errFailed ;
	}
	//
	// 構築
	//
	return	CompileCallConstructor
				( typeResult, block, nFlags, *pPrthArg, pClass ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileUnaryOperator
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags,
		RSCodeOperator::OperatorIndex opIndex, bool fPostOp )
{
	if ( typeResult.m_pImmediate != NULL )
	{
		//
		// 即値に対する処理
		//
		if ( RSCodeOperator::IsMoveLeftOperator( opIndex ) )
		{
			OutputError
				( SString(RSCodeOperator::m_pwszOperators[opIndex])
							+ " 演算子に対する左辺式ではありません" ) ;
			return	errFailed ;
		}
		RSSmartPtr	pObjDup
			( typeResult.m_pImmediate->CloneObject( m_context ) ) ;
		if ( pObjDup == NULL )
		{
			OutputError
				( SString(RSCodeOperator::m_pwszOperators[opIndex])
												+ " 演算子は無効です" ) ;
			return	errFailed ;
		}
		RSObject *	pObjResult = NULL ;
		switch ( opIndex )
		{
		case	RSCodeOperator::opAdd:
			pObjResult = pObjDup->OperatorPlus( m_context ) ;
			break ;
		case	RSCodeOperator::opSub:
			pObjResult = pObjDup->OperatorNegate( m_context ) ;
			break ;
		case	RSCodeOperator::opBitNot:
			pObjResult = pObjDup->OperatorBitNot( m_context ) ;
			break ;
		case	RSCodeOperator::opIncrement:
			pObjResult = pObjDup->OperatorIncrement( m_context ) ;
			break ;
		case	RSCodeOperator::opDecrement:
			pObjResult = pObjDup->OperatorDecrement( m_context ) ;
			break ;
		default:
			break ;
		}
		if ( !TestContextException( m_context ) )
		{
			RSObject::ReleaseRef( pObjResult ) ;
			return	errFailed ;
		}
		typeResult.SetImmediate
			( m_context, pObjResult, pObjResult->GetRSClass() ) ;
		return	errSuccess ;
	}
	//
	// 実行時式
	//
	bool	fValidOperator = false ;
	if ( (opIndex == RSCodeOperator::opIncrement)
		|| (opIndex == RSCodeOperator::opDecrement) )
	{
		if ( !typeResult.IsReference() )
		{
			OutputError
				( SString(RSCodeOperator::m_pwszOperators[opIndex])
							+ " 演算子に対する左辺式ではありません" ) ;
			return	errFailed ;
		}
	}
	if ( typeResult.IsPointer() )
	{
		// ポインタ
		if ( (opIndex == RSCodeOperator::opIncrement)
			|| (opIndex == RSCodeOperator::opDecrement) )
		{
			fValidOperator = true ;
		}
	}
	else if ( typeResult.IsInteger() )
	{
		// 整数
		switch ( opIndex )
		{
		case	RSCodeOperator::opAdd:
		case	RSCodeOperator::opSub:
		case	RSCodeOperator::opBitNot:
		case	RSCodeOperator::opIncrement:
		case	RSCodeOperator::opDecrement:
			fValidOperator = true ;
			break ;
		default:
			break ;
		}
	}
	else if ( typeResult.IsFloatingPoint() )
	{
		// 浮動小数点
		switch ( opIndex )
		{
		case	RSCodeOperator::opAdd:
		case	RSCodeOperator::opSub:
			fValidOperator = true ;
			break ;
		default:
			break ;
		}
	}
	if ( fValidOperator )
	{
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInst = new RSInstruction ;
		pInst->UniOperator
			( m_context, opIndex, typeResult.m_pClass, typeResult ) ;
		block.AddCode( pInst ) ;
		//
		AllocateTemporary( block, pInst->m_typeDst ) ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
	else
	{
		OutputError
			( SString(L"\'") + typeResult.GetTypeName()
				+ L"\' に対する \'"
				+ SString(RSCodeOperator::m_pwszOperators[opIndex])
				+ L"\' 演算子は定義されていません" ) ;
		return	errFailed ;
	}
}

// 型キャスト
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileTypeCast
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags,
		RSClass * pCastType, uint32_t nCastFlags, const RSTypeInfo& typeSrc )
{
	RSTypeInfo	typeCast ;
	typeCast.SetType( m_context, pCastType ) ;
	//
	bool	flagCastObject = false ;
	bool	flagCvtPointer = false ;
	bool	flagCvtNumber = false ;
	bool	flagCvtObject = false ;
	bool	flagNoConversion = false ;
	bool	flagImmediate = false ;
	bool	flagWarning = false ;
	bool	flagError = false ;
	RSInstruction::Code
			codeConvert = RSInstruction::codeInvalid ;
	//
	if ( pCastType == m_vm->GetBooleanClass() )
	{
		// (boolean) any Object
		flagWarning = (typeSrc.m_pClass != m_vm->GetBooleanClass()) ;
		//
		if ( typeSrc.m_pImmediate != NULL )
		{
			bool	fBoolean = typeSrc.m_pImmediate->AsBoolean() ;
			typeResult.SetImmediate
				( m_context,
					m_context.new_Boolean( fBoolean ),
					m_vm->GetBooleanClass() ) ;
			flagNoConversion = true ;
		}
		else
		{
			flagCvtObject = true ;
			codeConvert = RSInstruction::codeConvert2Boolean ;
		}
	}
	else if ( (typeSrc.m_pImmediate != NULL)
		&& (typeSrc.m_pImmediate->GetBasicType() == RSObject::typePointer)
		&& (typeSrc.m_pImmediate->GetEntityObject() == NULL) )
	{
		// (any Type) null
		if ( typeCast.IsPointer() || typeCast.IsObject() )
		{
			// null は Object / Pointer 型なら常に変換できる
			typeResult.SetImmediate
				( m_context,
					m_context.new_Pointer( NULL, pCastType ), pCastType ) ;
			return	errSuccess ;
		}
		OutputError
			( SString( L"null を " )
				+ pCastType->GetFullClassName() + L" へ変換できません" ) ;
		return	errFailed ;
	}
	else if ( typeSrc.IsPointer() )
	{
		if ( typeCast.IsPointer() )
		{
			// (any Pointer) pointer
			if ( !typeSrc.m_pClass->IsInstanceOf( pCastType ) )
			{
				flagWarning = !(nCastFlags & castForce) ;
			}
			flagCvtPointer = (typeSrc.m_pClass != pCastType) ;
		}
		else if ( pCastType == m_vm->GetGenericObjectClass() )
		{
			// (Object) pointer
			flagWarning = !(nCastFlags & castForce) ;
			codeConvert = RSInstruction::codeConvertNum2Object ;
			flagCvtObject = true ;
		}
		else
		{
			flagError = true ;
		}
	}
	else if ( typeSrc.IsInteger() )
	{
		if ( typeCast.IsInteger() )
		{
			// (any int) int
			if ( !(nCastFlags & castForce) )
			{
				flagWarning =
					(RSTypeInfo::m_bytesNumber[typeCast.m_typeNum]
						< RSTypeInfo::m_bytesNumber[typeSrc.m_typeNum])
					|| ((RSTypeInfo::m_bytesNumber[typeCast.m_typeNum]
							== RSTypeInfo::m_bytesNumber[typeSrc.m_typeNum])
						&& (typeCast.m_typeNum != typeSrc.m_typeNum)) ;
			}
			int64_t	num ;
			if ( (typeSrc.m_pImmediate != NULL)
				&& typeSrc.m_pImmediate->AsInteger( num ) )
			{
				RSInteger *	pCastInt =
					new RSInteger
						( m_vm->GetBasicTypeClass
							( RSTypeInfo::m_wiBasicType[typeCast.m_typeNum] ),
							num, RSTypeInfo::m_typeInteger[typeCast.m_typeNum] ) ;
				typeResult.SetImmediate
					( m_context, pCastInt, pCastInt->GetRSClass() ) ;
				flagNoConversion = true ;
			}
			else
			{
				flagCvtNumber = true ;
			}
		}
		else if ( typeCast.IsFloatingPoint() )
		{
			// (any float) int
			if ( !(nCastFlags & castForce) )
			{
				if ( typeCast.m_typeNum == RSTypeInfo::typeFloat32 )
				{
					flagWarning =
						(RSTypeInfo::m_bytesNumber[typeSrc.m_typeNum] >= 24) ;
				}
				else
				{
					flagWarning =
						(RSTypeInfo::m_bytesNumber[typeSrc.m_typeNum] >= 56) ;
				}
			}
			double	num ;
			if ( (typeSrc.m_pImmediate != NULL)
				&& typeSrc.m_pImmediate->AsRealNumber( num ) )
			{
				RSNumber *	pCastNum =
					new RSNumber
						( m_vm->GetBasicTypeClass
							( RSTypeInfo::m_wiBasicType[typeCast.m_typeNum] ), num ) ;
				typeResult.SetImmediate
					( m_context, pCastNum, pCastNum->GetRSClass() ) ;
				flagNoConversion = true ;
			}
			else
			{
				flagCvtNumber = true ;
			}
		}
		else if ( pCastType == m_vm->GetStringClass() )
		{
			// (String) int
			flagWarning = !(nCastFlags & castForce) ;
			//
			SString	str ;
			if ( (typeSrc.m_pImmediate != NULL)
				&& typeSrc.m_pImmediate->AsString( str ) )
			{
				typeResult.SetImmediate
					( m_context,
						m_context.new_String( str ),
						m_vm->GetStringClass() ) ;
				flagNoConversion = true ;
			}
			else
			{
				codeConvert = RSInstruction::codeConvertNum2Str ;
				flagCvtObject = true ;
			}
		}
		else if ( pCastType == m_vm->GetGenericObjectClass() )
		{
			// (Object) int
			flagWarning = !(nCastFlags & castForce) ;
			codeConvert = RSInstruction::codeConvertNum2Object ;
			flagCvtObject = true ;
		}
		else
		{
			flagError = true ;
		}
	}
	else if ( typeSrc.IsFloatingPoint() )
	{
		if ( typeCast.IsInteger() )
		{
			// (any int) float
			flagWarning = !(nCastFlags & castForce) ;
			flagCvtNumber = true ;
		}
		else if ( typeCast.IsFloatingPoint() )
		{
			// (any float) float
			if ( !(nCastFlags & castForce) )
			{
				flagWarning =
					(RSTypeInfo::m_bytesNumber[typeCast.m_typeNum]
						< RSTypeInfo::m_bytesNumber[typeSrc.m_typeNum]) ;
			}
			flagCvtNumber = true ;
		}
		else if ( pCastType == m_vm->GetStringClass() )
		{
			// (String) float
			flagWarning = !(nCastFlags & castForce) ;
			codeConvert = RSInstruction::codeConvertNum2Str ;
			flagCvtObject = true ;
		}
		else if ( pCastType == m_vm->GetGenericObjectClass() )
		{
			// (Object) float
			flagWarning = !(nCastFlags & castForce) ;
			codeConvert = RSInstruction::codeConvertNum2Object ;
			flagCvtObject = true ;
		}
		else
		{
			flagError = true ;
		}
	}
	else if ( typeSrc.m_pClass != NULL )
	{
		// 型キャスト
		if ( typeSrc.m_pClass->IsInstanceOf( pCastType ) )
		{
			flagCastObject = true ;
		}
		else if ( pCastType->IsInstanceOf( typeSrc.m_pClass ) )
		{
			flagWarning = !(nCastFlags & castForce) ;
			flagCastObject = true ;
		}
		else
		{
			flagError = true ;
		}
	}
	else
	{
		flagError = true ;
	}
	if ( flagError )
	{
		OutputError
			( typeSrc.GetTypeName()
				+ L" から " + pCastType->GetFullClassName()
				+ L" へ型変換できません" ) ;
		return	errFailed ;
	}
	if ( flagWarning )
	{
		OutputWarning
			( typeSrc.GetTypeName()
				+ L" から " + pCastType->GetFullClassName()
				+ L" への暗黙の型変換です" ) ;
	}
	if ( !flagNoConversion )
	{
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInst = NULL ;
		if ( flagCastObject )
		{
			pInst = new RSInstruction ;
			pInst->CastObject( m_context, pCastType, typeSrc ) ;
		}
		else if ( flagCvtPointer )
		{
			pInst = new RSInstruction ;
			pInst->ConvertPointer( m_context, pCastType, typeSrc ) ;
		}
		else if ( flagCvtNumber )
		{
			pInst = new RSInstruction ;
			pInst->ConvertNumber( m_context, pCastType, typeSrc ) ;
		}
		else if ( flagCvtObject )
		{
			ESLAssert( codeConvert != RSInstruction::codeInvalid ) ;
			pInst = new RSInstruction ;
			pInst->ConvertObject
				( m_context, codeConvert, pCastType, typeSrc ) ;
		}
		if ( pInst != NULL )
		{
			AllocateTemporary( block, pInst->m_typeDst ) ;
			typeResult = pInst->m_typeDst ;
		}
		else if ( &typeResult != &typeSrc )
		{
			typeResult = typeSrc ;
		}
	}
	return	errSuccess ;
}

SSystem::SError RSInstructionParser::CompileCastBoolean
	( RSTypeInfo& typeResult,
		FunctionBlock& block,
		uint32_t nFlags, uint32_t nCastFlags )
{
	return	CompileTypeCast
		( typeResult, block, nFlags,
			m_vm->GetBooleanClass(), nCastFlags, typeResult ) ;
}

// 配列インスタンス生成式 [ expr, expr, ... ]
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileArrayExpression
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSParenthesis& prth, uint32_t nFlags )
{
	SmartParenthesis	sp( *this ) ;
	RSCodeStream		cs( prth ) ;
	RSClass *			pElementType = NULL ;
	RSSmartPtr			spArrayObj( m_context.new_Array() ) ;
	bool				fRuntimeObj = false ;
	bool				fElementType = true ;
	int					nIndex = 0 ;
	//
	m_pCurParenthesis = &prth ;
	while ( !cs.IsEndOfStream() )
	{
		RSTypeInfo	typeElement ;
		SError	err =
			CompileExpression
				( typeElement, block, cs,
					RSCodeOperator::priorityList, nFlags ) ;
		if ( err )
		{
			return	err ;
		}
		if ( fElementType )
		{
			pElementType = MatchMultiType( pElementType, typeElement ) ;
			if ( pElementType == NULL )
			{
				fElementType = false ;
			}
			else
			{
				err = CompileTypeCast
					( typeElement, block, nFlags,
						pElementType, castNatural, typeElement ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		if ( (typeElement.m_pImmediate == NULL) || fRuntimeObj )
		{
			if ( !VerifyExprOutputCode( nFlags ) )
			{
				return	errFailed ;
			}
			if ( !fRuntimeObj )
			{
				RSInstruction *	pInst = new RSInstruction ;
				pInst->LoadImmediate( m_context, spArrayObj.Detach() ) ;
				AllocateTemporary( block, pInst->m_typeDst ) ;
				block.AddCode( pInst ) ;
				//
				typeResult = pInst->m_typeDst ;
				fRuntimeObj = true ;
			}
			RSInstruction *	pInstRef = new RSInstruction ;
			pInstRef->LoadElementInt( m_context, typeResult, nIndex, true ) ;
			AllocateTemporary( block, pInstRef->m_typeDst ) ;
			block.AddCode( pInstRef ) ;
			//
			RSInstruction *	pInstMov = new RSInstruction ;
			pInstMov->MoveOperator
				( m_context, RSCodeOperator::opMove,
						pInstRef->m_typeDst, typeElement ) ;
			block.AddCode( pInstMov ) ;
		}
		else
		{
			ESLAssert( spArrayObj.Ptr() != NULL ) ;
			m_context.ReleaseObjectRef
				( spArrayObj->SetElementAt
					( m_context, nIndex,
						typeElement.m_pImmediate->
							DuplicateObject( m_context ) ) ) ;
		}
		nIndex ++ ;
		//
		if ( cs.NextOperator( RSCodeOperator::opSequencing ) == NULL )
		{
			break ;
		}
	}
	if ( !cs.IsEndOfStream() )
	{
		OutputError( L"配列要素が \',\' で区切られていません" ) ;
	}
	if ( !fRuntimeObj )
	{
		RSArray *	pObjArray = ESLTypeCast<RSArray>( spArrayObj.Ptr() ) ;
		if ( (pObjArray != NULL) && (pElementType != NULL) )
		{
			pObjArray->SetArrayPrototype( 0x7FFFFFFF, pElementType ) ;
			pObjArray->SetRSClass( m_vm->GetArrayClassAs( pElementType ) ) ;
		}
		RSClass *	pClass = spArrayObj->GetRSClass() ;
		typeResult.SetImmediate( m_context, spArrayObj.Detach(), pClass ) ;
	}
	else
	{
		if ( pElementType != NULL )
		{
			typeResult.SetType
				( m_context, m_vm->GetArrayClassAs( pElementType ) ) ;
		}
	}
	return	errSuccess ;
}

// 辞書配列インスタンス生成式 { id : expr, ... }
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileHashMapExpression
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSParenthesis& prth, uint32_t nFlags )
{
	SmartParenthesis	sp( *this ) ;
	RSCodeStream		cs( prth ) ;
	RSClass *			pElementType = NULL ;
	RSSmartPtr			spHashObj
							( new RSDynamicObject
								( m_vm->GetDynamicObjectClass(),
											RSObject::typeObject ) ) ;
	bool				fRuntimeObj = false ;
	bool				fElementType = true ;
	//
	m_pCurParenthesis = &prth ;
	while ( !cs.IsEndOfStream() )
	{
		RSCode *	pCodeName = cs.NextTerm() ;
		SString		strKeyName ;
		if ( pCodeName && (pCodeName->m_type == RSCode::typeLiteral) )
		{
			ESLAssert( pCodeName->IsKindOf( ESL_RUNTIME_CLASS(RSCodeLiteral) ) ) ;
			RSCodeLiteral *	pLiteral = (RSCodeLiteral*) pCodeName ;
			if ( (pLiteral->m_literal != NULL)
				&& (pLiteral->m_literal->GetBasicType()
										== RSObject::typeString) )
			{
				pLiteral->m_literal->AsString( strKeyName ) ;
			}
		}
		else if ( pCodeName && (pCodeName->m_type == RSCode::typeSymbol) )
		{
			ESLAssert( pCodeName->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
			strKeyName = ((RSCodeSymbol*)pCodeName)->m_symbol ;
		}
		if ( strKeyName.IsEmpty() )
		{
			OutputError( L"構文エラー：辞書配列のキーが不正です" ) ;
			return	errFailed ;
		}
		if ( cs.NextOperator( RSCodeOperator::opSeparator ) == NULL )
		{
			OutputError
				( L"構文エラー：辞書配列のキーと値が"
					L" \':\' で区切られていません" ) ;
			return	errFailed ;
		}
		RSTypeInfo	typeElement ;
		SError	err =
			CompileExpression
				( typeElement, block, cs,
					RSCodeOperator::priorityList, nFlags ) ;
		if ( err )
		{
			return	err ;
		}
		if ( fElementType )
		{
			pElementType = MatchMultiType( pElementType, typeElement ) ;
			if ( pElementType == NULL )
			{
				fElementType = false ;
			}
			else
			{
				err = CompileTypeCast
					( typeElement, block, nFlags,
						pElementType, castNatural, typeElement ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		if ( (typeElement.m_pImmediate == NULL) || fRuntimeObj )
		{
			if ( !VerifyExprOutputCode( nFlags ) )
			{
				return	errFailed ;
			}
			if ( !fRuntimeObj )
			{
				RSInstruction *	pInst = new RSInstruction ;
				pInst->LoadImmediate( m_context, spHashObj.Detach() ) ;
				AllocateTemporary( block, pInst->m_typeDst ) ;
				block.AddCode( pInst ) ;
				//
				typeResult = pInst->m_typeDst ;
				fRuntimeObj = true ;
			}
			RSClass *	pElType =
				fElementType ? pElementType : m_vm->GetGenericObjectClass() ;
			RSInstruction *	pInstRef = new RSInstruction ;
			pInstRef->LoadElementStr
				( m_context, pElType, typeResult, strKeyName, true ) ;
			AllocateTemporary( block, pInstRef->m_typeDst ) ;
			block.AddCode( pInstRef ) ;
			//
			RSInstruction *	pInstMov = new RSInstruction ;
			pInstMov->MoveOperator
				( m_context, RSCodeOperator::opMove,
						pInstRef->m_typeDst, typeElement ) ;
			block.AddCode( pInstMov ) ;
		}
		else
		{
			ESLAssert( spHashObj.Ptr() != NULL ) ;
			m_context.ReleaseObjectRef
				( spHashObj->SetMemberAs
					( m_context, strKeyName,
						typeElement.m_pImmediate->
							DuplicateObject( m_context ) ) ) ;
		}
		if ( cs.NextOperator( RSCodeOperator::opSequencing ) == NULL )
		{
			break ;
		}
	}
	if ( !cs.IsEndOfStream() )
	{
		OutputError( L"配列要素が \',\' で区切られていません" ) ;
	}
	if ( !fRuntimeObj )
	{
		if ( (spHashObj.Ptr() != NULL) && (pElementType != NULL) )
		{
			spHashObj->SetRSClass( m_vm->GetHashMapClassAs( pElementType ) ) ;
		}
		RSClass *	pClass = spHashObj->GetRSClass() ;
		typeResult.SetImmediate( m_context, spHashObj.Detach(), pClass ) ;
	}
	else
	{
		if ( pElementType != NULL )
		{
			typeResult.SetType
				( m_context, m_vm->GetHashMapClassAs( pElementType ) ) ;
		}
	}
	return	errSuccess ;
}

// メンバ参照
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileOperatorReferenceMember
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags,
		RSCodeOperator::OperatorIndex iOp )
{
	RSCodeSymbol *	pSymbol = cs.NextSymbol() ;
	if ( pSymbol == NULL )
	{
		OutputError( L"構文エラー：メンバの記述がありません" ) ;
		return	errFailed ;
	}
	if ( iOp == RSCodeOperator::opStaticMemberOf )
	{
		if ( (typeResult.m_pImmediate != NULL)
			&& (typeResult.m_pImmediate->GetBasicType()
										== RSObject::typeClass) )
		{
			RSClass *	pClass =
				ESLTypeCast<RSClass>( typeResult.m_pImmediate ) ;
			RSObject *	pMember =
				typeResult.m_pImmediate->
					GetMemberAs( m_context, pSymbol->m_symbol ) ;
			m_context.ClearException() ;
			if ( (pClass != NULL) && (pMember != NULL) )
			{
				// クラス静メンバ
				return	CompileLoadClassStaticMember
							( typeResult, block, nFlags,
								pClass, pMember, pSymbol->m_symbol, true ) ;
			}
			RSObject::ReleaseRef( pMember ) ;
		}
		OutputError( L"構文エラー： \'::\' メンバがクラスではありません" ) ;
		return	errFailed ;
	}
	if ( (typeResult.m_pImmediate != NULL)
		&& (typeResult.m_pImmediate->GetBasicType()
									== RSObject::typeClass) )
	{
		RSClass *	pClass = ESLTypeCast<RSClass>( typeResult.m_pImmediate ) ;
		if ( pClass != NULL )
		{
			RSObject *	pMember =
					pClass->GetMemberAs( m_context, pSymbol->m_symbol ) ;
			m_context.ClearException() ;
			if ( pMember != NULL )
			{
				// クラス静メンバ
				return	CompileLoadClassStaticMember
							( typeResult, block, nFlags,
								pClass, pMember, pSymbol->m_symbol, false ) ;
			}
		}
	}
	RSTypeInfo	typeObj = typeResult ;
	SError	err =
		CompileReferenceMember
			( typeResult, block, cs,
				nFlags, typeObj, pSymbol->m_symbol ) ;
	if ( err != errContinue )
	{
		return	err ;
	}
	OutputError
		( SString(L"\'") + pSymbol->m_symbol + L"\' は \'"
			+ typeObj.GetTypeName()
			+ L"\' のメンバではありません" ) ;
	return	errFailed ;
}

// 論理積 &&
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileOperatorLogicalAnd
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags )
{
	SError	err =
		CompileCastBoolean( typeResult, block, nFlags, castNatural ) ;
	if ( err )
	{
		return	err ;
	}
	if ( typeResult.m_pImmediate != NULL )
	{
		if ( typeResult.m_pImmediate->AsBoolean() )
		{
			// true && expr
			err = CompileExpression
				( typeResult, block, cs,
					RSCodeOperator::priorityLAnd, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			return	CompileCastBoolean
					( typeResult, block, nFlags, castNatural ) ;
		}
		else
		{
			// false && expr
			size_t	iPos = block.GetCurrentPos() ;
			//
			RSTypeInfo	typeExpr2 ;
			err = CompileExpression
					( typeExpr2, block, cs,
						RSCodeOperator::priorityLAnd, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileCastBoolean
					( typeExpr2, block, nFlags, castNatural ) ;
			if ( err )
			{
				return	err ;
			}
			if ( iPos != block.GetCurrentPos() )
			{
				block.ClearCodeAfter( iPos ) ;
			}
			return	errSuccess ;
		}
	}
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	ESLAssert( typeResult.m_fp.iNumber >= 0 ) ;
	RSInstruction *	pInstJump = new RSInstruction ;
	pInstJump->JumpNotIf
		( m_context, typeResult, block.GetCurrentPos() ) ;
	block.AddCode( pInstJump ) ;
	//
	RSTypeInfo	typeExpr2 ;
	err = CompileExpression
		( typeExpr2, block, cs,
			RSCodeOperator::priorityLAnd, nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCastBoolean( typeExpr2, block, nFlags, castNatural ) ;
	if ( err )
	{
		return	err ;
	}
	//
	RSInstruction *	pInstMov = new RSInstruction ;
	pInstMov->MoveOperator
		( m_context, RSCodeOperator::opMove, typeResult, typeExpr2 ) ;
	block.AddCode( pInstMov ) ;
	//
	pInstJump->m_ipTarget = block.GetCurrentPos() ;
	//
	return	errSuccess ;
}

// 論理積 ||
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileOperatorLogicalOr
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags )
{
	SError	err =
		CompileCastBoolean( typeResult, block, nFlags, castNatural ) ;
	if ( err )
	{
		return	err ;
	}
	if ( typeResult.m_pImmediate != NULL )
	{
		if ( typeResult.m_pImmediate->AsBoolean() )
		{
			// false || expr
			err = CompileExpression
				( typeResult, block, cs,
					RSCodeOperator::priorityLAnd, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			return	CompileCastBoolean
					( typeResult, block, nFlags, castNatural ) ;
		}
		else
		{
			// true || expr
			size_t	iPos = block.GetCurrentPos() ;
			//
			RSTypeInfo	typeExpr2 ;
			err = CompileExpression
					( typeExpr2, block, cs,
						RSCodeOperator::priorityLAnd, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileCastBoolean
					( typeExpr2, block, nFlags, castNatural ) ;
			if ( err )
			{
				return	err ;
			}
			if ( iPos != block.GetCurrentPos() )
			{
				block.ClearCodeAfter( iPos ) ;
			}
			return	errSuccess ;
		}
	}
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	ESLAssert( typeResult.m_fp.iNumber >= 0 ) ;
	RSInstruction *	pInstJump = new RSInstruction ;
	pInstJump->JumpIf
		( m_context, typeResult, block.GetCurrentPos() ) ;
	block.AddCode( pInstJump ) ;
	//
	RSTypeInfo	typeExpr2 ;
	err = CompileExpression
		( typeExpr2, block, cs,
			RSCodeOperator::priorityLAnd, nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCastBoolean( typeExpr2, block, nFlags, castNatural ) ;
	if ( err )
	{
		return	err ;
	}
	//
	RSInstruction *	pInstMov = new RSInstruction ;
	pInstMov->MoveOperator
		( m_context, RSCodeOperator::opMove, typeResult, typeExpr2 ) ;
	block.AddCode( pInstMov ) ;
	//
	pInstJump->m_ipTarget = block.GetCurrentPos() ;
	//
	return	errSuccess ;
}

// 式選択 ? expr : expr
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileOperatorSelect
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags )
{
	SError	err =
		CompileCastBoolean( typeResult, block, nFlags, castNatural ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	RSInstruction *	pInstJump1 = new RSInstruction ;
	pInstJump1->JumpNotIf
		( m_context, typeResult, block.GetCurrentPos() ) ;
	block.AddCode( pInstJump1 ) ;
	//
	RSTypeInfo	typeExpr1 ;
	err = CompileExpression
			( typeExpr1, block, cs,
				RSCodeOperator::prioritySeparator, nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	err = NormalizeAllocation( block, nFlags, typeExpr1 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	if ( cs.NextOperator( RSCodeOperator::opSeparator ) == NULL )
	{
		OutputError( L"\'?\' \':\' 構文エラー" ) ;
		return	errFailed ;
	}
	RSInstruction *	pInstJump2 = new RSInstruction ;
	pInstJump2->m_code = RSInstruction::codeJump ;
	block.AddCode( pInstJump2 ) ;
	//
	pInstJump1->m_ipTarget = block.GetCurrentPos() ;
	//
	RSTypeInfo	typeExpr2 ;
	err = CompileExpression
			( typeExpr2, block, cs,
				RSCodeOperator::prioritySelector, nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileTypeCast
		( typeExpr2, block, nFlags,
			typeExpr1.m_pClass, castNatural, typeExpr2 ) ;
	if ( err )
	{
		return	err ;
	}
	RSInstruction *	pInst = new RSInstruction ;
	pInst->MoveOperator
		( m_context, RSCodeOperator::opMove, typeExpr1, typeExpr2 ) ;
	block.AddCode( pInst ) ;
	//
	pInstJump2->m_ipTarget = block.GetCurrentPos() ;
	//
	return	errSuccess ;
}

// HashMap<type> / Function<type,...>  式の判定
//////////////////////////////////////////////////////////////////////////////
bool RSInstructionParser::CompileGenericClass
	( RSTypeInfo& typeResult,
		FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags,
		RSCodeOperator * pOpCode )
{
	if ( (pOpCode->m_operator == RSCodeOperator::opLessThan)
		&& (typeResult.m_pImmediate == m_vm->GetDynamicObjectClass()) )
	{
		size_t		nIndex = cs.GetIndex() ;
		RSClass *	pClass = ParseClassExpression( cs ) ;
		if ( pClass != NULL )
		{
			if ( cs.NextOperator( RSCodeOperator::opGraterThan ) != NULL )
			{
				pClass = m_vm->GetHashMapClassAs( pClass ) ;
				typeResult = RSTypeInfo() ;
				typeResult.SetImmediate
					( m_context, pClass, pClass->GetRSClass() ) ;
				return	true ;
			}
		}
		cs.SeekIndex( nIndex ) ;
	}
	else if ( (pOpCode->m_operator == RSCodeOperator::opLessThan)
		&& (typeResult.m_pImmediate == m_vm->GetFunctionClass()) )
	{
		RSFunctionPrototype	proto ;
		//
		size_t		nIndex = cs.GetIndex() ;
		if ( cs.NextControlWord( RSCodeControl::wiVoid ) == NULL )
		{
			RSClass *	pRetType = ParseClassExpression( cs ) ;
			proto.SetReturnType( pRetType ) ;
		}
		if ( cs.NextOperator( RSCodeOperator::opSequencing ) )
		{
			proto.m_pNamespaceClass = ParseClassExpression( cs ) ;
		}
		int	nArg = 0 ;
		while ( cs.NextOperator( RSCodeOperator::opSequencing ) )
		{
			RSClass *	pArgType = ParseClassExpression( cs ) ;
			proto.AddArgument
				( pArgType, SString(L"a") + SString( nArg ) ) ;
		}
		if ( cs.NextOperator( RSCodeOperator::opGraterThan ) )
		{
			RSClass*	pClass = m_vm->GetFunctionClassAs( proto ) ;
			typeResult = RSTypeInfo() ;
			typeResult.SetImmediate
				( m_context, pClass, pClass->GetRSClass() ) ;
			return	true ;
		}
		else
		{
			cs.SeekIndex( nIndex ) ;
		}
	}
	return	false ;
}

// 一般的な二項演算子
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileBinaryOperator
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags,
		RSTypeInfo& typeRight, RSCodeOperator::OperatorIndex iOp )
{
	if ( RSCodeOperator::IsMoveLeftOperator( iOp ) )
	{
		if ( !typeResult.IsReference() )
		{
			OutputError
				( SString(RSCodeOperator::m_pwszOperators[iOp])
							+ L" 演算子に対する左辺式ではありません" ) ;
			return	errFailed ;
		}
		if ( typeResult.m_accMod & RSObject::modifierConst )
		{
			OutputError
				( L"const オブジェクトへの不正な代入操作です" ) ;
			return	errFailed ;
		}
	}
	if ( (typeResult.m_pImmediate != NULL)
		&& (typeRight.m_pImmediate != NULL) )
	{
		// 即値計算
		RSObject *	pResult =
			m_context.ExecuteBinaryOperator
				( typeResult.m_pImmediate, typeRight.m_pImmediate, iOp ) ;
		if ( !TestContextException( m_context ) )
		{
			RSObject::ReleaseRef( pResult ) ;
			return	errFailed ;
		}
	}
	SError	err ;
	if ( iOp == RSCodeOperator::opMemberCallOf )
	{
		// expr .* expr
		if ( ESLTypeCast<RSFunctionClass>( typeRight.m_pClass ) == NULL )
		{
			OutputError( L"\'.*\' の右辺が関数ではありません" ) ;
			return	errFailed ;
		}
		m_typeExprThis = typeResult ;
		typeResult = typeRight ;
		return	errSuccess ;
	}
	else if ( iOp == RSCodeOperator::opInstanceOf )
	{
		// expr instanceof type
		RSClass *	pClass = ESLTypeCast<RSClass>( typeRight.m_pImmediate ) ;
		if ( pClass == NULL )
		{
			OutputError( L"instanceof の右辺が型指定ではありません" ) ;
			return	errFailed ;
		}
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInst = new RSInstruction ;
		pInst->BinOperator
			( m_context, iOp,
				m_vm->GetBooleanClass(), typeResult, typeRight ) ;
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
	else if ( iOp == RSCodeOperator::opSequencing )
	{
		// expr , expr
		m_typeExprThis = RSTypeInfo() ;
		typeResult = typeRight ;
		return	errSuccess ;
	}
	if ( typeResult.IsPointer() )
	{
		// ポインタ演算
		RSClass *	pDstType = NULL ;
		switch ( iOp )
		{
		case	RSCodeOperator::opAdd:
		case	RSCodeOperator::opSub:
		case	RSCodeOperator::opMoveAdd:
		case	RSCodeOperator::opMoveSub:
			err = CompileTypeCast
				( typeRight, block, nFlags,
					m_vm->GetIntegerClass(), castNatural, typeRight ) ;
			if ( err )
			{
				return	err ;
			}
			pDstType = typeResult.m_pClass ;
			break ;
		case	RSCodeOperator::opEqual:
		case	RSCodeOperator::opNotEqual:
		case	RSCodeOperator::opPointerEqual:
		case	RSCodeOperator::opPointerNotEqual:
		case	RSCodeOperator::opMove:
			err = CompileTypeCast
				( typeRight, block, nFlags,
					typeResult.m_pClass, castNatural, typeRight ) ;
			if ( err )
			{
				return	err ;
			}
			pDstType = m_vm->GetBooleanClass() ;
			break ;
		default:
			OutputError
				( SString(L"\'")
					+ SString(RSCodeOperator::m_pwszOperators[iOp])
					+ "\' はポインタに対する定義されない演算子です" ) ;
			return	errFailed ;
		}
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInst = new RSInstruction ;
		pInst->BinOperator
			( m_context, iOp, pDstType, typeResult, typeRight ) ;
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
	else if ( typeResult.IsFloatingPoint() || typeRight.IsFloatingPoint() )
	{
		// 浮動小数点演算
		RSClass *	pResultType = m_vm->GetNumberClass() ;
		RSClass *	pFloatType =
						m_vm->GetBasicTypeClass( RSCodeControl::wiFloat ) ;
		if ( (typeResult.m_pClass == pFloatType)
			&& (typeRight.m_pClass == pFloatType) )
		{
			pResultType = pFloatType ;
		}
		if ( !typeResult.IsFloatingPoint() )
		{
			err = CompileTypeCast
				( typeResult, block, nFlags,
					pResultType, castNatural, typeResult ) ;
			if ( err )
			{
				return	err ;
			}
		}
		if ( !typeRight.IsFloatingPoint() )
		{
			err = CompileTypeCast
				( typeRight, block, nFlags,
					pResultType, castNatural, typeRight ) ;
			if ( err )
			{
				return	err ;
			}
		}
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInst = new RSInstruction ;
		switch ( iOp )
		{
		case	RSCodeOperator::opAdd:
		case	RSCodeOperator::opSub:
		case	RSCodeOperator::opMul:
		case	RSCodeOperator::opDiv:
			pInst->BinOperator
				( m_context, iOp, pResultType, typeResult, typeRight ) ;
			AllocateTemporary( block, pInst->m_typeDst ) ;
			break ;
		case	RSCodeOperator::opMove:
		case	RSCodeOperator::opMoveAdd:
		case	RSCodeOperator::opMoveSub:
		case	RSCodeOperator::opMoveMul:
		case	RSCodeOperator::opMoveDiv:
			if ( !typeResult.IsReference() )
			{
				OutputError
					( SString(RSCodeOperator::m_pwszOperators[iOp])
								+ " 演算子に対する左辺式ではありません" ) ;
				return	errFailed ;
			}
			pInst->BinOperator
				( m_context, iOp, pResultType, typeResult, typeRight ) ;
			AllocateTemporary( block, pInst->m_typeDst ) ;
			break ;
		case	RSCodeOperator::opEqual:
		case	RSCodeOperator::opNotEqual:
		case	RSCodeOperator::opLessEqual:
		case	RSCodeOperator::opLessThan:
		case	RSCodeOperator::opGraterEqual:
		case	RSCodeOperator::opGraterThan:
			pInst->BinOperator
				( m_context, iOp,
					m_vm->GetBooleanClass(), typeResult, typeRight ) ;
			AllocateTemporary( block, pInst->m_typeDst ) ;
			break ;
		default:
			delete	pInst ;
			OutputError
				( SString(L"\'")
					+ SString(RSCodeOperator::m_pwszOperators[iOp])
					+ "\' は浮動小数点値に対する定義されない演算子です" ) ;
			return	errFailed ;
		}
		block.AddCode( pInst ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
	else if ( typeResult.IsInteger() && typeRight.IsInteger() )
	{
		// 整数演算
		RSInstruction *	pInst = new RSInstruction ;
		if ( RSCodeOperator::IsMoveOperator( iOp ) )
		{
			if ( RSTypeInfo::m_bitsNumber[typeResult.m_typeNum]
				< RSTypeInfo::m_bitsNumber[typeRight.m_typeNum] )
			{
				OutputWarning
					( SString(L"左辺値は ")
						+ typeResult.GetTypeName()
						+ L" に切り詰められます" ) ;
			}
			pInst->BinOperator
				( m_context, iOp,
					typeResult.m_pClass, typeResult, typeRight ) ;
		}
		else if ( RSCodeOperator::IsComparator( iOp ) )
		{
			pInst->BinOperator
				( m_context, iOp,
					m_vm->GetBooleanClass(), typeResult, typeRight ) ;
		}
		else
		{
			RSClass *	pResultType =
					MatchMultiIntegerType( typeResult, typeRight ) ;
			pInst->BinOperator
				( m_context, iOp, pResultType, typeResult, typeRight ) ;
		}
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
	else
	{
		err = CompileTypeCast
			( typeRight, block, nFlags,
				typeResult.m_pClass, castNatural, typeRight ) ;
		if ( err )
		{
			return	err ;
		}
		RSInstruction *	pInst = new RSInstruction ;
		switch ( iOp )
		{
		case	RSCodeOperator::opEqual:
		case	RSCodeOperator::opNotEqual:
		case	RSCodeOperator::opPointerEqual:
		case	RSCodeOperator::opPointerNotEqual:
			pInst->BinOperator
				( m_context, iOp,
					m_vm->GetBooleanClass(), typeResult, typeRight ) ;
			break ;
		case	RSCodeOperator::opMove:
			pInst->BinOperator
				( m_context, iOp,
					typeResult.m_pClass, typeResult, typeRight ) ;
			break ;
		default:
			delete	pInst ;
			OutputError
				( SString(L"\'")
					+ SString(RSCodeOperator::m_pwszOperators[iOp])
					+ "\' は \'"
					+ typeResult.GetTypeName()
					+ L"\' に対する定義されない演算子です" ) ;
			return	errFailed ;
		}
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
}

// 関数呼び出し／オブジェクト構築
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileCallFunction
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		uint32_t nFlags, RSParenthesis& prthArg )
{
	RSClass *	pClass = ESLTypeCast<RSClass>( typeResult.m_pImmediate ) ;
	if ( pClass != NULL )
	{
		return	CompileCallConstructor
					( typeResult, block, nFlags, prthArg, pClass ) ;
	}
	SObjectArray<RSTypeInfo>	aArgType ;
	RSTypeInfo	typeThis = m_typeExprThis ;
	RSTypeInfo	typeFunc = typeResult ;
	SError	err = CompileArgument( aArgType, block, nFlags, prthArg ) ;
	if ( err )
	{
		return	err ;
	}
	RSFunctionObject *	pFunc =
			ESLTypeCast<RSFunctionObject>( typeFunc.m_pImmediate ) ;
	if ( pFunc != NULL )
	{
		return	CompileCallDirectFunction
			( typeResult, block, nFlags, pFunc, 0, typeThis, aArgType ) ;
	}
	RSFunctionClass *
		pFuncClass = ESLTypeCast<RSFunctionClass>( typeFunc.m_pClass ) ;
	if ( pFuncClass == NULL )
	{
		OutputError
			( typeFunc.GetTypeName()
				+ L" は Function 型に適合しません" ) ;
		return	errFailed ;
	}
	return	CompileInvokeFunction
				( typeResult, block, nFlags,
						typeFunc, 0, typeThis, aArgType ) ;
}

// オブジェクト構築
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileCallConstructor
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags,
		RSParenthesis& prthArg, RSClass * pClass )
{
	//
	// 引数解釈
	//
	SObjectArray<RSTypeInfo>	aArgType ;
	SError	err = CompileArgument( aArgType, block, nFlags, prthArg ) ;
	if ( err )
	{
		return	err ;
	}
	RSTypedArrayPointerClass *
		pPtrClass = ESLTypeCast<RSTypedArrayPointerClass>( pClass ) ;
	if ( pPtrClass != NULL )
	{
		// ポインタ型
		return	CompileCallPointerConstructor
			( typeResult, block, nFlags, aArgType, pPtrClass ) ;
	}
	RSTypeInfo::NumberType	typeNum =
				RSTypeInfo::GetNumberTypeOf( m_context, pClass ) ;
	if ( typeNum != RSTypeInfo::typeObject )
	{
		// 数値型
		return	CompileCallNumberConstructor
			( typeResult, block, nFlags, aArgType, typeNum ) ;
	}
	//
	// オブジェクト生成
	//
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	RSInstruction *	pInstNew = new RSInstruction ;
	pInstNew->NewObject( m_context, pClass ) ;
	AllocateTemporary( block, pInstNew->m_typeDst ) ;
	block.AddCode( pInstNew ) ;
	typeResult = pInstNew->m_typeDst ;
	//
	// 構築関数呼び出し
	//
	RSFunctionObject *	pFunc = pClass->GetConstructor() ;
	if ( pFunc != NULL )
	{
		ssize_t	iProto = FindMatchFunctionPrototype( *pFunc, aArgType ) ;
		if ( iProto >= 0 )
		{
			RSTypeInfo	typeTemp ;
			err = CompileCallDirectFunction
				( typeTemp, block, nFlags,
					pFunc, (size_t) iProto, typeResult, aArgType ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			OutputError( L"適合する構築関数がありません" ) ;
			return	errFailed ;
		}
	}
	else
	{
		if ( aArgType.GetLength() >= 1 )
		{
			OutputError( L"適合する構築関数がありません" ) ;
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// 数値型構築
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileCallNumberConstructor
	( RSTypeInfo& typeResult,
		FunctionBlock& block, uint32_t nFlags,
		SObjectArray<RSTypeInfo>& aArgType, RSTypeInfo::NumberType typeNum )
{
	RSClass *	pClass =
		m_vm->GetBasicTypeClass( RSTypeInfo::m_wiBasicType[typeNum] ) ;
	ESLAssert( pClass != NULL ) ;
	//
	RSTypeInfo *	pArgType = aArgType.GetAt( 0 ) ;
	if ( (aArgType.GetLength() != 1) && (pArgType == NULL) )
	{
		OutputError
			( pClass->GetFullClassName()
				+ L" の適合する構築関数がありません" ) ;
		return	errFailed ;
	}
	SError	err =
		CompileTypeCast
			( typeResult, block, nFlags, pClass, castForce, *pArgType ) ;
	if ( err )
	{
		return	err ;
	}
	return	errSuccess ;
}

// ポインタ型構築
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileCallPointerConstructor
	( RSTypeInfo& typeResult,
		FunctionBlock& block, uint32_t nFlags,
		SObjectArray<RSTypeInfo>& aArgType, RSTypedArrayPointerClass * pClass )
{
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	//
	// 構築関数呼び出し
	//
	SError				err ;
	RSFunctionObject *	pFunc = pClass->GetConstructor() ;
	if ( pFunc != NULL )
	{
		ssize_t	iProto = FindMatchFunctionPrototype( *pFunc, aArgType ) ;
		if ( iProto >= 0 )
		{
			RSFunctionPrototype *	pProto =
					pFunc->m_arrPrototypes.GetAt( (size_t) iProto ) ;
			if ( (pProto->m_methodNative.pfnMethod
						== &RSTypedArrayPointerClass::method_init1)
				|| (pProto->m_methodNative.pfnMethod
						== &RSStructureClass::method_init1) )
			{
				// pointer( int length )
				RSInstruction *	pInstNew = new RSInstruction ;
				pInstNew->NewObject( m_context, pClass ) ;
				AllocateTemporary( block, pInstNew->m_typeDst ) ;
				block.AddCode( pInstNew ) ;
				typeResult = pInstNew->m_typeDst ;
				//
				RSTypeInfo	typeTemp ;
				err = CompileCallDirectFunction
					( typeTemp, block, nFlags,
						pFunc, (size_t) iProto, typeResult, aArgType ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else if ( (pProto->m_methodNative.pfnMethod
						== &RSTypedArrayPointerClass::method_init2)
				|| (pProto->m_methodNative.pfnMethod
						== &RSStructureClass::method_init2) )
			{
				// pointer( Uint8Pointer ptr )
				RSTypeInfo *	pArgType = aArgType.GetAt( 0 ) ;
				ESLAssert( pArgType != NULL ) ;
				return	CompileTypeCast
					( typeResult, block, nFlags, pClass, castForce, *pArgType ) ;
			}
			/*
			else if ( (pProto->m_methodNative.pfnMethod
						== &RSTypedArrayPointerClass::method_init3)
				|| (pProto->m_methodNative.pfnMethod
						== &RSStructureClass::method_init3) )
			{
				// pointer( ArrayBuffer buf, int off, int len )
			}
			*/
			else
			{
				RSInstruction *	pInstNew = new RSInstruction ;
				pInstNew->NewObject( m_context, pClass ) ;
				AllocateTemporary( block, pInstNew->m_typeDst ) ;
				block.AddCode( pInstNew ) ;
				typeResult = pInstNew->m_typeDst ;
				//
				RSTypeInfo	typeTemp ;
				err = CompileCallDirectFunction
					( typeTemp, block, nFlags,
						pFunc, (size_t) iProto, typeResult, aArgType ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else
		{
			OutputError( L"適合する構築関数がありません" ) ;
			return	errFailed ;
		}
	}
	else
	{
		OutputError( L"適合する構築関数がありません" ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// 配列要素参照 expr[expr]
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileReferenceElement
	( RSTypeInfo& typeResult,
		FunctionBlock& block,
		uint32_t nFlags, RSParenthesis& prthIndex )
{
	RSCodeStream	csPrth( prthIndex ) ;
	RSTypeInfo		typeIndex ;
	SError	err = CompileExpression
		( typeIndex, block, csPrth,
			RSCodeOperator::priorityNothing, nFlags ) ;
	if ( err )
	{
		return	err ;
	}
	RSGenericArrayClass *	pArrayClass =
			ESLTypeCast<RSGenericArrayClass>( typeResult.m_pClass ) ;
	if ( pArrayClass != NULL )
	{
		// 配列参照
		err = CompileTypeCast
			( typeIndex, block, nFlags,
				m_vm->GetIntegerClass(), castNatural, typeIndex ) ;
		if ( err )
		{
			return	err ;
		}
		int64_t	num ;
		if ( typeResult.m_pImmediate
			&& typeIndex.m_pImmediate
			&& typeIndex.m_pImmediate->AsInteger( num ) )
		{
			// 即値
			RSObject *	pElement =
				typeResult.m_pImmediate->
					GetElementAt( m_context, (int) num ) ;
			if ( !TestContextException( m_context ) )
			{
				RSObject::ReleaseRef( pElement ) ;
				return	errFailed ;
			}
			if ( pElement == NULL )
			{
				OutputError( L"不正な配列要素参照です" ) ;
			}
			typeResult.SetImmediate
				( m_context, pElement, pElement->GetRSClass() ) ;
			return	errSuccess ;
		}
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInst = new RSInstruction ;
		pInst->LoadIndirectElementInt
			( m_context, typeResult, typeIndex, true ) ;
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
	RSDynamicObjectClass *	pHashClass =
		ESLTypeCast<RSDynamicObjectClass>( typeResult.m_pClass ) ;
	if ( pHashClass != NULL )
	{
		err = CompileTypeCast
			( typeIndex, block, nFlags,
				m_vm->GetStringClass(), castNatural, typeIndex ) ;
		if ( err )
		{
			return	err ;
		}
		RSClass *	pElementType = m_vm->GetGenericObjectClass() ;
		RSGenericHashMapClass *
			pGenHashClass = ESLTypeCast<RSGenericHashMapClass>( pHashClass ) ;
		if ( pGenHashClass != NULL )
		{
			pElementType = pGenHashClass->m_pElementClass ;
		}
		SString	str ;
		if ( typeResult.m_pImmediate
			&& typeIndex.m_pImmediate
			&& typeIndex.m_pImmediate->AsString( str ) )
		{
			// 即値
			RSObject *	pElement =
				typeResult.m_pImmediate->GetMemberAs( m_context, str ) ;
			if ( !TestContextException( m_context ) )
			{
				RSObject::ReleaseRef( pElement ) ;
				return	errFailed ;
			}
			if ( pElement == NULL )
			{
				OutputError( L"不正な配列要素参照です" ) ;
			}
			typeResult.SetImmediate
				( m_context, pElement, pElement->GetRSClass() ) ;
			return	errSuccess ;
		}
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInst = new RSInstruction ;
		pInst->LoadIndirectElementStr
			( m_context, pElementType, typeResult, typeIndex, true ) ;
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		typeResult = pInst->m_typeDst ;
		return	errSuccess ;
	}
	OutputError( L"配列でないオブジェクトへの要素参照です" ) ;
	return	errFailed ;
}

// 関数呼び出しアクセス修飾子判定
//////////////////////////////////////////////////////////////////////////////
bool RSInstructionParser::ValidationPrototypeAccess
	( FunctionBlock& block,
		RSFunctionPrototype& proto,
		RSClass * pObjClass, uint32_t accMod )
{
	ESLAssert( block.m_pProto != NULL ) ;
	if ( block.m_pProto->m_pNamespaceClass != proto.m_pNamespaceClass )
	{
		// 同一クラス外から
		uint32_t	nAcc = proto.m_nFlags
							& RSFunctionPrototype::flagAccessMask ;
		if ( block.m_pProto->m_pNamespaceClass
			&& block.m_pProto->m_pNamespaceClass->
					IsInstanceOf( proto.m_pNamespaceClass ) )
		{
			// 派生クラスのメンバ関数内から
			if ( nAcc == RSFunctionPrototype::flagAccessPrivate )
			{
				OutputError( L"private な関数の参照です" ) ;
				return	false ;
			}
		}
		else if ( nAcc != RSFunctionPrototype::flagAccessPublic )
		{
			if ( nAcc == RSFunctionPrototype::flagAccessProtected )
			{
				OutputError( L"protected な関数の参照です" ) ;
			}
			else
			{
				OutputError( L"private な関数の参照です" ) ;
			}
			return	false ;
		}
	}
	if ( accMod & RSObject::modifierConst )
	{
		if ( !proto.IsConstantModifier() )
		{
			OutputError
				( L"const オブジェクトへの非 const 関数の呼び出しです" ) ;
			return	false ;
		}
	}
	return	true ;
}

// メンバアクセス修飾子判定
//////////////////////////////////////////////////////////////////////////////
bool RSInstructionParser::ValidationMemberAccess
	( FunctionBlock& block,
		uint32_t accModMember,
		RSClass * pObjClass, uint32_t accModObj )
{
	ESLAssert( block.m_pProto != NULL ) ;
	uint32_t	nAcc = accModMember & RSObject::accessMask ;
	if ( block.m_pProto->m_pNamespaceClass == pObjClass )
	{
		// 同一クラスのメンバ関数内から
		if ( nAcc == RSObject::modifierPrivateInvisible )
		{
			OutputError( L"private なメンバの参照です" ) ;
			return	false ;
		}
		return	true ;
	}
	if ( block.m_pProto->m_pNamespaceClass
		&& block.m_pProto->m_pNamespaceClass->IsInstanceOf( pObjClass ) )
	{
		// 派生クラスのメンバ関数内から
		if ( accModMember >= RSObject::modifierPrivate )
		{
			OutputError( L"private なメンバの参照です" ) ;
			return	false ;
		}
		return	true ;
	}
	if ( nAcc == RSObject::modifierPublic )
	{
		return	true ;
	}
	if ( nAcc == RSObject::modifierProtected )
	{
		OutputError( L"protected な関数の参照です" ) ;
	}
	else
	{
		OutputError( L"private な関数の参照です" ) ;
	}
	return	true ;
}

// 関数引数解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileArgument
	( SSystem::SObjectArray<RSTypeInfo>& aArgType,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags, RSParenthesis& prthArg )
{
	RSCodeStream	csArg( prthArg ) ;
	while ( !csArg.IsEndOfStream() )
	{
		RSTypeInfo *	pArgType = new RSTypeInfo ;
		SError	err =
			CompileExpression
				( *pArgType, block, csArg,
					RSCodeOperator::priorityList, nFlags ) ;
		if ( err )
		{
			delete	pArgType ;
			return	err ;
		}
		aArgType.Add( pArgType ) ;
		//
		if ( csArg.NextOperator( RSCodeOperator::opSequencing ) == NULL )
		{
			break ;
		}
	}
	if ( !csArg.IsEndOfStream() )
	{
		OutputError( L"引数が \',\' で区切られていません" ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// 適合関数検索
//////////////////////////////////////////////////////////////////////////////
ssize_t RSInstructionParser::FindMatchFunctionPrototype
	( const RSFunctionObject& func,
		const SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	size_t	nPrototypes = func.m_arrPrototypes.GetLength() ;
	for ( size_t i = 0; i < nPrototypes; i ++ )
	{
		RSFunctionPrototype *	pProto = func.m_arrPrototypes.GetAt( i ) ;
		if ( pProto != NULL )
		{
			if ( IsMatchFunctionPrototype( *pProto, aArgType ) )
			{
				return	(ssize_t) i ;
			}
		}
	}
	return	-1 ;
}

bool RSInstructionParser::IsMatchFunctionPrototype
	( const RSFunctionPrototype& proto,
		const SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	size_t	nProtoArgs = proto.m_aArgTypes.GetLength() ;
	if ( (nProtoArgs < aArgType.GetLength())
			&& !(proto.m_nFlags & RSFunctionPrototype::flagVarArg) )
	{
		return	false ;
	}
	if ( nProtoArgs > aArgType.GetLength() )
	{
		size_t	nDefArgs = proto.m_aArgDefault.GetLength() ;
		if ( nDefArgs < nProtoArgs )
		{
			return	false ;
		}
		for ( size_t i = aArgType.GetLength(); i < nProtoArgs; i ++ )
		{
			if ( proto.m_aArgDefault.GetAt( i ) == NULL )
			{
				return	false ;
			}
		}
	}
	for ( size_t i = 0; i < aArgType.GetLength(); i ++ )
	{
		RSClass *	pArgClass = proto.m_aArgTypes.GetAt( i ) ;
		if ( pArgClass == NULL )
		{
			if ( (i < proto.m_aArgTypes.GetLength())
				|| (proto.m_nFlags & RSFunctionPrototype::flagVarArg) )
			{
				continue ;
			}
			return	false ;
		}
		RSTypeInfo *	pArgType = aArgType.GetAt( i ) ;
		ESLAssert( pArgType != NULL ) ;
		if ( pArgType == NULL )
		{
			return	false ;
		}
		if ( !pArgType->IsMatchTypeFor( m_context, pArgClass ) )
		{
			return	false ;
		}
	}
	return	true ;
}

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileCallDirectFunction
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags,
		RSFunctionObject * pFunc, size_t iFunc,
		const RSTypeInfo& typeThis,
		SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	RSFunctionPrototype *	pProto = pFunc->m_arrPrototypes.GetAt( iFunc ) ;
	if ( pProto != NULL )
	{
		SError	err = CompileFunctionArgument
						( block, nFlags, *pProto, aArgType ) ;
		if ( err )
		{
			return	err ;
		}
	}
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	RSInstruction *	pInst = new RSInstruction ;
	pInst->CallDirect( m_context, pFunc, iFunc, typeThis, aArgType ) ;
	AllocateTemporary( block, pInst->m_typeDst ) ;
	block.AddCode( pInst ) ;
	//
	typeResult = pInst->m_typeDst ;
	return	errSuccess ;
}

SSystem::SError RSInstructionParser::CompileInvokeFunction
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags,
		const RSTypeInfo& typeFunc, size_t iFunc,
		const RSTypeInfo& typeThis,
		SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	RSGenericFunctionClass *	pFuncClass =
		ESLTypeCast<RSGenericFunctionClass>( typeFunc.m_pClass ) ;
	if ( (pFuncClass != NULL)
		&& (pFuncClass->m_pProto != NULL) )
	{
		SError	err =
			CompileFunctionArgument
				( block, nFlags, *(pFuncClass->m_pProto), aArgType ) ;
		if ( err )
		{
			return	err ;
		}
	}
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	RSInstruction *	pInst = new RSInstruction ;
	pInst->CallIndirect( m_context, typeFunc, iFunc, typeThis, aArgType ) ;
	AllocateTemporary( block, pInst->m_typeDst ) ;
	block.AddCode( pInst ) ;
	//
	typeResult = pInst->m_typeDst ;
	return	errSuccess ;
}

SSystem::SError RSInstructionParser::CompileCallVirtualFunction
	( RSTypeInfo& typeResult,
		FunctionBlock& block, uint32_t nFlags,
		const RSFunctionPrototype& proto,
		const wchar_t * pwszFuncName, size_t iFunc,
		const RSTypeInfo& typeThis,
		SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	SError	err =
		CompileFunctionArgument( block, nFlags, proto, aArgType ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	RSInstruction *	pInst = new RSInstruction ;
	pInst->CallVirtual
		( m_context, proto.m_pReturnType,
			pwszFuncName, iFunc, typeThis, aArgType ) ;
	AllocateTemporary( block, pInst->m_typeDst ) ;
	block.AddCode( pInst ) ;
	//
	typeResult = pInst->m_typeDst ;
	return	errSuccess ;
}

// 関数引数正規化
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileFunctionArgument
	( FunctionBlock& block, uint32_t nFlags,
		const RSFunctionPrototype& proto,
		SSystem::SObjectArray<RSTypeInfo>& aArgType )
{
	for ( size_t i = 0; i < proto.m_aArgTypes.GetLength(); i ++ )
	{
		RSClass *	pArgClass = proto.m_aArgTypes.GetAt( i ) ;
		if ( pArgClass != NULL )
		{
			RSTypeInfo *	pArg = aArgType.GetAt( i ) ;
			if ( pArg != NULL )
			{
				SError	err =
					CompileTypeCast
						( *pArg, block, nFlags,
							pArgClass, castNatural, *pArg ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else
			{
				RSObject *	pDefValue =
								proto.m_aArgDefault.GetAt( i ) ;
				if ( pDefValue != NULL )
				{
					pArg = new RSTypeInfo ;
					pArg->SetImmediate
						( m_context,
							pDefValue->CloneObject( m_context ),
							pDefValue->GetEntityClass() ) ;
					aArgType.SetAt( i, pArg ) ;
				}
				else
				{
					OutputError( L"関数引数が不足しています" ) ;
					return	errFailed ;
				}
			}
		}
	}
	if ( aArgType.GetLength() > proto.m_aArgTypes.GetLength() )
	{
		OutputError( L"関数引数が多すぎます" ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// 現在の名前空間で定義済みクラスを取得する
//////////////////////////////////////////////////////////////////////////////
RSClass * RSInstructionParser::GetDefinedClassAs
	( RSInstructionParser::FunctionBlock& block, const wchar_t * pwszName )
{
	if ( block.m_pClassSpace != NULL )
	{
		RSSmartPtr	spMember
			( block.m_pClassSpace->GetMemberAs( m_context, pwszName ) ) ;
		if ( spMember != NULL )
		{
			RSClass *	pClass = ESLTypeCast<RSClass>( spMember.Ptr() ) ;
			if ( pClass != NULL )
			{
				return	pClass ;
			}
		}
	}
	return	m_vm->GetClassAs( pwszName ) ;
}

// シンボル参照
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileAutoReferenceSymbol
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, const SString & strName, uint32_t nFlags )
{
	SError	err ;
	//
	// ローカル変数検索
	//
	err = CompileLocalReference
			( typeResult, block, cs, strName, nFlags ) ;
	if ( err != errContinue )
	{
		return	err ;
	}
	RSClass *	pThisClass = block.m_pProto->m_pNamespaceClass ;
	if ( pThisClass != NULL )
	{
		RSStructuredPointerClass *	pStructClass =
			ESLTypeCast<RSStructuredPointerClass>( pThisClass ) ;
		//
		// 仮想関数検索
		//
		RSFunctionObject *	pFunc =
			pThisClass->GetVirtualMemberAs( m_context, strName ) ;
		if ( pFunc != NULL )
		{
			RSTypeInfo	typeThis ;
			err = CompileLoadThis( typeThis, block, nFlags ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileReferenceMember
				( typeResult, block, cs, nFlags, typeThis, strName ) ;
			if ( err != errContinue )
			{
				return	err ;
			}
		}
		//
		// this メンバ検索
		//
		if ( pStructClass != NULL )
		{
			RSStructuredPointerClass::ElementInfo *
				pElInfo = pStructClass->GetArrayMemberAs( strName ) ;
			if ( pElInfo != NULL )
			{
				RSTypeInfo	typeThis ;
				err = CompileLoadThis( typeThis, block, nFlags ) ;
				if ( err )
				{
					return	err ;
				}
				err = CompileReferenceMember
					( typeResult, block, cs, nFlags, typeThis, strName ) ;
				if ( err != errContinue )
				{
					return	err ;
				}
			}
		}
		else if ( pThisClass->m_pPrototype != NULL )
		{
			RSSmartPtr	pObj
				( pThisClass->m_pPrototype->GetMemberAs( m_context, strName ) ) ;
			m_context.ClearException() ;
			if ( pObj != NULL )
			{
				RSTypeInfo	typeThis ;
				err = CompileLoadThis( typeThis, block, nFlags ) ;
				if ( err )
				{
					return	err ;
				}
				err = CompileReferenceMember
					( typeResult, block, cs, nFlags, typeThis, strName ) ;
				if ( err != errContinue )
				{
					return	err ;
				}
			}
		}
	}
	//
	// this クラス静メンバ検索
	//
	err = CompileClassStaticMember
		( typeResult, block, cs, pThisClass, strName, nFlags ) ;
	if ( err != errContinue )
	{
		return	err ;
	}
	//
	// クラス名検索
	//
	RSClass *	pClass = m_vm->GetClassAs( strName ) ;
	if ( pClass != NULL )
	{
		pClass->AddRef() ;
		typeResult.SetImmediate( m_context, pClass, pClass->GetRSClass() ) ;
		return	errSuccess ;
	}
	//
	// グローバルオブジェクト検索
	//
	RSSmartPtr	pObj( m_vm->GetMemberAs( m_context, strName ) ) ;
	m_context.ClearException() ;
	if ( pObj != NULL )
	{
		if ( !VerifyExprOutputCode( nFlags ) )
		{
			return	errFailed ;
		}
		RSInstruction *	pInstRef = new RSInstruction ;
		pInstRef->LoadGlobal
			( m_context, pObj->GetRSClass(), strName ) ;
		AllocateTemporary( block, pInstRef->m_typeDst ) ;
		block.AddCode( pInstRef ) ;
		//
		typeResult = pInstRef->m_typeDst ;
		return	errSuccess ;
	}
	//
	// 定義済みリテラル
	//
	if ( strName == L"null" )
	{
		RSClass *	pObjClass = m_vm->GetGenericObjectClass() ;
		typeResult.SetImmediate
			( m_context,
				m_context.new_Pointer( NULL, pObjClass ), pObjClass ) ;
		return	errSuccess ;
	}
	else if ( strName == L"true" )
	{
		typeResult.SetImmediate
			( m_context,
				m_context.new_Boolean( true ), m_vm->GetBooleanClass() ) ;
		return	errSuccess ;
	}
	else if ( strName == L"false" )
	{
		typeResult.SetImmediate
			( m_context,
				m_context.new_Boolean( false ), m_vm->GetBooleanClass() ) ;
		return	errSuccess ;
	}
	//
	OutputError( SString(L"\'") + strName + L"\' は定義されていません" ) ;
	return	errFailed ;
}

// ローカル変数参照
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileLocalReference
	( RSTypeInfo& typeResult,
		FunctionBlock& block, RSCodeStream& cs,
		const SSystem::SString & strName, uint32_t nFlags )
{
	FunctionBlock *	pBlock = &block ;
	int				nLocalNest = 0 ;
	while ( pBlock != NULL )
	{
		RSTypeInfo *	pLocalVar ;
		if ( pBlock == &block )
		{
			pLocalVar = pBlock->m_local.GetNestedVariable( strName ) ;
		}
		else
		{
			pLocalVar = pBlock->m_local.GetVariableAs( strName ) ;
		}
		if ( pLocalVar != NULL )
		{
			if ( !VerifyExprOutputCode( nFlags ) )
			{
				return	errFailed ;
			}
			RSInstruction *	pInst = new RSInstruction ;
			pInst->LoadLocal( m_context, *pLocalVar, nLocalNest ) ;
			AllocateTemporary( block, pInst->m_typeDst ) ;
			block.AddCode( pInst ) ;
			//
			typeResult = pInst->m_typeDst ;
			return	errSuccess ;
		}
		for ( size_t i = 0; i < pBlock->m_local.m_nest.GetLength(); i ++ )
		{
			LocalNest *	pNest = pBlock->m_local.m_nest.GetLastAt( i ) ;
			if ( (pNest != NULL)
				&& (pNest->m_control == nestWith)
				&& pNest->m_fSpaceObj
				&& pNest->m_typeSpaceObj.m_pClass )
			{
				RSClass *	pClass = pNest->m_typeSpaceObj.m_pClass ;
				RSStructuredPointerClass *	pStructClass =
					ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
				RSFunctionObject *	pFunc =
					pClass->GetVirtualMemberAs( m_context, strName ) ;
				if ( pFunc != NULL )
				{
					SError	err = CompileReferenceMember
						( typeResult, block, cs,
							nFlags, pNest->m_typeSpaceObj, strName ) ;
					if ( err != errContinue )
					{
						return	err ;
					}
				}
				if ( pStructClass != NULL )
				{
					RSStructuredPointerClass::ElementInfo *
						pElInfo = pStructClass->GetArrayMemberAs( strName ) ;
					if ( pElInfo != NULL )
					{
						SError	err = CompileReferenceMember
							( typeResult, block, cs,
								nFlags, pNest->m_typeSpaceObj, strName ) ;
						if ( err != errContinue )
						{
							return	err ;
						}
					}
				}
				else if ( pClass->m_pPrototype != NULL )
				{
					RSSmartPtr	pObj
						( pClass->m_pPrototype->GetMemberAs
											( m_context, strName ) ) ;
					m_context.ClearException() ;
					if ( pObj != NULL )
					{
						SError	err = CompileReferenceMember
							( typeResult, block, cs,
								nFlags, pNest->m_typeSpaceObj, strName ) ;
						if ( err != errContinue )
						{
							return	err ;
						}
					}
				}
			}
		}
		pBlock = pBlock->m_pParentScope ;
		nLocalNest ++ ;
	}
	return	errContinue ;
}

// クラス静メンバ参照
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileClassStaticMember
	( RSTypeInfo& typeResult,
		FunctionBlock& block, RSCodeStream& cs,
		RSClass * pClass,
		const SSystem::SString & strMember, uint32_t nFlags )
{
	RSObject *	pMember = pClass->GetMemberAs( m_context, strMember ) ;
	m_context.ClearException() ;
	if ( pMember == NULL )
	{
		SError	err ;
		if ( pClass->m_pSuperClass != NULL )
		{
			err = CompileClassStaticMember
				( typeResult, block, cs,
					pClass->m_pSuperClass, strMember, nFlags ) ;
			if ( err != errContinue )
			{
				return	err ;
			}
		}
		for ( size_t i = 0; i < pClass->m_lstImplements.GetLength(); i ++ )
		{
			RSClass *	pSuperClass = pClass->m_lstImplements.GetAt( i ) ;
			if ( pSuperClass != NULL )
			{
				err = CompileClassStaticMember
					( typeResult, block, cs, pSuperClass, strMember, nFlags ) ;
				if ( err != errContinue )
				{
					return	err ;
				}
			}
		}
		return	errContinue ;
	}
	RSParenthesis *	pPrthArg =
			cs.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthArg != NULL )
	{
		RSSmartPtr	ptrFunc( pMember ) ;
		RSFunctionObject *
				pFunc = ESLTypeCast<RSFunctionObject>( pMember ) ;
		if ( pFunc == NULL )
		{
			OutputError
				( SString(L"\'") + pClass->GetFullClassName()
					+ L"." + strMember
					+ SString(L"\' は関数ではありません") ) ;
			return	errFailed ;
		}
		//
		// class.member( ... )
		//
		SObjectArray<RSTypeInfo>	aArgType ;
		SError	err = CompileArgument( aArgType, block, nFlags, *pPrthArg ) ;
		if ( err )
		{
			return	err ;
		}
		ssize_t	iProto = FindMatchFunctionPrototype( *pFunc, aArgType ) ;
		if ( iProto < 0 )
		{
			OutputError( L"呼び出し引数に適合する関数が見つかりません" ) ;
			return	errFailed ;
		}
		//
		// static 関数呼び出し
		//
		RSTypeInfo	typeThis ;
		typeThis.SetImmediate
			( m_context, pClass, pClass->GetRSClass() ) ;
		//
		err = CompileCallDirectFunction
			( typeResult, block, nFlags,
				pFunc, (size_t) iProto, typeThis, aArgType ) ;
		//
		m_typeExprThis = RSTypeInfo() ;
		return	err ;
	}
	else
	{
		//
		// クラス静メンバ
		//
		return	CompileLoadClassStaticMember
			( typeResult, block, nFlags, pClass, pMember, strMember, false ) ;
	}
}

// オブジェクトメンバロード
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileReferenceMember
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cs, uint32_t nFlags,
		const RSTypeInfo& typeObj, const SSystem::SString& strMember )
{
	if ( typeObj.m_pImmediate != NULL )
	{
		RSClass *	pClass = ESLTypeCast<RSClass>( typeObj.m_pImmediate ) ;
		if ( pClass != NULL )
		{
			RSObject *	pMember = pClass->GetMemberAs( m_context, strMember ) ;
			m_context.ClearException() ;
			if ( pMember != NULL )
			{
				return	CompileLoadClassStaticMember
							( typeResult, block, nFlags,
								pClass, pMember, strMember, false ) ;
			}
		}
	}
	if ( typeObj.m_pClass != NULL )
	{
		//
		// 仮想関数検索
		//
		RSFunctionObject *	pFunc =
			typeObj.m_pClass->GetVirtualMemberAs( m_context, strMember ) ;
		if ( pFunc != NULL )
		{
			RSSmartPtr	ptrFunc( pFunc ) ;
			RSParenthesis *	pPrthArg =
					cs.NextParenthesis( RSParenthesis::ptParenthesis ) ;
			if ( pPrthArg != NULL )
			{
				//
				// obj.virtual_member( ... )
				//
				SObjectArray<RSTypeInfo>	aArgType ;
				SError	err = CompileArgument( aArgType, block, nFlags, *pPrthArg ) ;
				if ( err )
				{
					return	err ;
				}
				ssize_t	iProto = FindMatchFunctionPrototype( *pFunc, aArgType ) ;
				RSFunctionPrototype *
					pProto = pFunc->m_arrPrototypes.GetAt( (size_t) iProto ) ;
				if ( (iProto < 0) || (pProto == NULL) )
				{
					OutputError( L"呼び出し引数に適合する関数が見つかりません" ) ;
					return	errFailed ;
				}
				if ( pProto->IsConstantModifier() && !typeObj.IsConstant() )
				{
					OutputError
						( L"非 const なポインタから const 関数を呼び出すことは出来ません" ) ;
					return	errFailed ;
				}
				if ( !ValidationPrototypeAccess
					( block, *pProto, typeObj.m_pClass, typeObj.m_accMod ) )
				{
					return	errFailed ;
				}
				//
				// メンバ関数呼び出し
				//
				if ( typeObj.IsStructure() )
				{
					err = CompileCallDirectFunction
						( typeResult, block, nFlags,
							pFunc, (size_t) iProto, typeObj, aArgType ) ;
				}
				else
				{
					err = CompileCallVirtualFunction
						( typeResult, block, nFlags, *pProto,
							strMember, (size_t) iProto, typeObj, aArgType ) ;
				}
				m_typeExprThis = RSTypeInfo() ;
				return	err ;
			}
			else
			{
				//
				// this.virtual_member
				//
				OutputError( L"仮想関数は取得できません" ) ;
				return	errFailed ;
			}
		}
		//
		// メンバ検索
		//
		if ( typeObj.IsStructure() )
		{
			RSStructuredPointerClass *	pStructClass =
				ESLTypeCast<RSStructuredPointerClass>( typeObj.m_pClass ) ;
			RSStructuredPointerClass::ElementInfo *
				pElInfo = pStructClass->GetArrayMemberAs( strMember ) ;
			if ( pElInfo != NULL )
			{
				if ( !ValidationMemberAccess
						( block, pElInfo->m_accMod, typeObj.m_pClass ) )
				{
					return	errFailed ;
				}
				if ( !VerifyExprOutputCode( nFlags ) )
				{
					return	errFailed ;
				}
				RSTypedArrayPointerClass *	pElArrayType =
					ESLTypeCast<RSTypedArrayPointerClass>( pElInfo->m_pClass ) ;
				RSInstruction *	pInstRef = new RSInstruction ;
				if ( pElArrayType != NULL )
				{
					pInstRef->PtrStructureMember
						( m_context, pElInfo->m_pClass, typeObj,
							(int) pElInfo->m_iOffset, (int) pElInfo->m_nBytes ) ;
				}
				else
				{
					pInstRef->RefStructureMember
						( m_context, pElInfo->m_pClass, typeObj,
							(int) pElInfo->m_iOffset, (int) pElInfo->m_nBytes ) ;
				}
				AllocateTemporary( block, pInstRef->m_typeDst ) ;
				pInstRef->m_typeDst.m_accMod = pElInfo->m_accMod ;
				block.AddCode( pInstRef ) ;
				typeResult = pInstRef->m_typeDst ;
				return	errSuccess ;
			}
		}
		else if ( (typeObj.m_pClass != NULL)
				&& (typeObj.m_pClass->m_pPrototype != NULL) )
		{
			RSSmartPtr	pObj
				( typeObj.m_pClass->m_pPrototype->
						GetMemberAs( m_context, strMember ) ) ;
			m_context.ClearException() ;
			if ( pObj != NULL )
			{
				if ( !ValidationMemberAccess
					( block, pObj->GetAccessModifier(), typeObj.m_pClass ) )
				{
					return	errFailed ;
				}
				if ( !VerifyExprOutputCode( nFlags ) )
				{
					return	errFailed ;
				}
				RSInstruction *	pInstRef = new RSInstruction ;
				pInstRef->LoadElementStr
					( m_context, pObj->GetEntityClass(),
								typeObj, strMember, true ) ;
				AllocateTemporary( block, pInstRef->m_typeDst ) ;
				pInstRef->m_typeDst.m_accMod = pObj->GetModifiers() ;
				block.AddCode( pInstRef ) ;
				//
				m_typeExprThis = typeObj ;
				typeResult = pInstRef->m_typeDst ;
				return	errSuccess ;
			}
		}
	}
	return	errContinue ;
}

// クラス静メンバロード
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileLoadClassStaticMember
	( RSTypeInfo& typeResult,
		RSInstructionParser::FunctionBlock& block, uint32_t nFlags,
		RSClass * pClass, RSObject * pMember,
		const SSystem::SString& strMember, bool fOnlyNamespace )
{
	if ( !fOnlyNamespace )
	{
		pClass->AddRef() ;
		m_typeExprThis.SetImmediate
			( m_context, pClass, pClass->GetRSClass() ) ;
	}
	RSFunctionObject *	pFunc = ESLTypeCast<RSFunctionObject>( pMember ) ;
	if ( pFunc != NULL )
	{
		// 関数
		RSFunctionPrototype *
				pProto = pFunc->m_arrPrototypes.GetAt( 0 ) ;
		if ( (pProto == NULL)
			|| (pFunc->m_arrPrototypes.GetLength() > 1) )
		{
			OutputError
				( SString(L"\'") + pClass->GetFullClassName()
					+ L"." + strMember
					+ SString(L"\' は同名関数が複数存在します") ) ;
			return	errFailed ;
		}
		if ( !ValidationPrototypeAccess( block, *pProto, pClass ) )
		{
			return	errFailed ;
		}
		RSClass *	pMemberType = pMember->GetRSClass() ;
		if ( (pFunc->m_pPrototype != NULL)
			&& (pFunc->m_arrPrototypes.GetLength() == 1)
			&& (ESLTypeCast<RSGenericFunctionClass>(pMemberType) == NULL) )
		{
			pMemberType =
				m_vm->GetFunctionClassAs( *(pFunc->m_pPrototype) ) ;
		}
		typeResult.SetImmediate( m_context, pMember, pMemberType ) ;
		return	errSuccess ;
	}
	else if ( (pMember->GetModifiers() & RSObject::modifierConst)
				|| (pMember->GetBasicType() == RSObject::typeClass) )
	{
		// 即値
		if ( !ValidationMemberAccess
				( block, pMember->GetModifiers(), pClass ) )
		{
			return	errFailed ;
		}
		typeResult.SetImmediate( m_context, pMember, pMember->GetRSClass() ) ;
		typeResult.m_accMod = pMember->GetModifiers() ;
		return	errSuccess ;
	}
	//
	// 実行時式
	//
	if ( !ValidationMemberAccess
			( block, pMember->GetModifiers(), pClass ) )
	{
		return	errFailed ;
	}
	RSSmartPtr	spMember( pMember ) ;
	if ( !VerifyExprOutputCode( nFlags ) )
	{
		return	errFailed ;
	}
	RSInstruction *	pInstRef = new RSInstruction ;
	pInstRef->LoadElementStr
		( m_context, pMember->GetRSClass(), m_typeExprThis, strMember, true ) ;
	AllocateTemporary( block, pInstRef->m_typeDst ) ;
	pInstRef->m_typeDst.m_accMod = pMember->GetModifiers() ;
	block.AddCode( pInstRef ) ;
	//
	typeResult = pInstRef->m_typeDst ;
	return	errSuccess ;
}

// 配列や選択式で複数の型をマッチングする
//////////////////////////////////////////////////////////////////////////////
RSClass * RSInstructionParser::MatchMultiType
	( RSClass * pLastType, const RSTypeInfo& typeNew ) const
{
	if ( pLastType == NULL )
	{
		if ( typeNew.IsInteger() )
		{
			RSTypeInfo::NumberType	typeNum = typeNew.m_typeNum ;
			int64_t	num ;
			if ( (typeNum != RSTypeInfo::typeBoolean)
				&& (typeNew.m_pImmediate != NULL)
				&& typeNew.m_pImmediate->AsInteger( num ) )
			{
				typeNum = NormalizeIntegerTypeOf( num ) ;
			}
			return	m_vm->GetBasicTypeClass
							( RSTypeInfo::m_wiBasicType[typeNum] ) ;
		}
		return	typeNew.m_pClass ;
	}
	if ( pLastType == typeNew.m_pClass )
	{
		return	typeNew.m_pClass ;
	}
	if ( pLastType->IsInstanceOf( typeNew.m_pClass ) )
	{
		return	typeNew.m_pClass ;
	}
	if ( typeNew.m_pClass
		&& typeNew.m_pClass->IsInstanceOf( pLastType ) )
	{
		return	pLastType ;
	}
	RSTypeInfo	typeLast ;
	typeLast.SetType( m_context, pLastType ) ;
	if ( typeLast.IsFloatingPoint() )
	{
		return	m_vm->GetBasicTypeClass( RSCodeControl::wiDouble ) ;
	}
	else if ( typeLast.IsInteger() && typeNew.IsInteger() )
	{
		RSTypeInfo::NumberType	typeNum = typeNew.m_typeNum ;
		int64_t	num ;
		if ( (typeNum != RSTypeInfo::typeBoolean)
			&& (typeNew.m_pImmediate != NULL)
			&& typeNew.m_pImmediate->AsInteger( num ) )
		{
			typeNum = NormalizeIntegerTypeOf( num ) ;
		}
		if ( RSTypeInfo::m_bitsNumber[typeLast.m_typeNum]
							> RSTypeInfo::m_bitsNumber[typeNum] )
		{
			typeNum = typeLast.m_typeNum ;
		}
		else if ( RSTypeInfo::m_bitsNumber[typeLast.m_typeNum]
							== RSTypeInfo::m_bitsNumber[typeNum] )
		{
			if ( typeLast.m_typeNum != typeNum )
			{
				switch ( typeNum )
				{
				case	RSTypeInfo::typeUint8:
				case	RSTypeInfo::typeInt8:
					typeNum = RSTypeInfo::typeInt16 ;
					break ;
				case	RSTypeInfo::typeUint16:
				case	RSTypeInfo::typeInt16:
					typeNum = RSTypeInfo::typeInt32 ;
					break ;
				case	RSTypeInfo::typeUint32:
				case	RSTypeInfo::typeInt32:
				default:
					typeNum = RSTypeInfo::typeInt64 ;
					break ;
				}
			}
		}
		return	m_vm->GetBasicTypeClass
						( RSTypeInfo::m_wiBasicType[typeNum] ) ;
	}
	return	NULL ;
}

// 二つの整数型を格納できる整数型を取得する
//////////////////////////////////////////////////////////////////////////////
RSClass * RSInstructionParser::MatchMultiIntegerType
	( const RSTypeInfo& typeLeft, const RSTypeInfo& typeRight ) const
{
	RSTypeInfo::NumberType	typeNum = typeRight.m_typeNum ;
	int64_t	num ;
	if ( (typeNum != RSTypeInfo::typeBoolean)
		&& (typeRight.m_pImmediate != NULL)
		&& typeRight.m_pImmediate->AsInteger( num ) )
	{
		typeNum = NormalizeIntegerTypeOf( num ) ;
	}
	if ( RSTypeInfo::m_bitsNumber[typeLeft.m_typeNum]
						> RSTypeInfo::m_bitsNumber[typeNum] )
	{
		typeNum = typeLeft.m_typeNum ;
	}
	else if ( RSTypeInfo::m_bitsNumber[typeLeft.m_typeNum]
						== RSTypeInfo::m_bitsNumber[typeNum] )
	{
		if ( typeLeft.m_typeNum != typeNum )
		{
			switch ( typeNum )
			{
			case	RSTypeInfo::typeUint8:
			case	RSTypeInfo::typeInt8:
				typeNum = RSTypeInfo::typeInt16 ;
				break ;
			case	RSTypeInfo::typeUint16:
			case	RSTypeInfo::typeInt16:
				typeNum = RSTypeInfo::typeInt32 ;
				break ;
			case	RSTypeInfo::typeUint32:
			case	RSTypeInfo::typeInt32:
			default:
				typeNum = RSTypeInfo::typeInt64 ;
				break ;
			}
		}
	}
	return	m_vm->GetBasicTypeClass
					( RSTypeInfo::m_wiBasicType[typeNum] ) ;
}

// 整数の値から最小の整数型を取得する
//////////////////////////////////////////////////////////////////////////////
RSTypeInfo::NumberType
	RSInstructionParser::NormalizeIntegerTypeOf( int64_t num )
{
	if ( (-0x80 <= num) && (num < 0x80) )
	{
		return	RSTypeInfo::typeInt8 ;
	}
	else if ( (-0x8000 <= num) && (num < 0x8000) )
	{
		return	RSTypeInfo::typeInt16 ;
	}
	else if ( (0 <= num) && (num < 0x10000) )
	{
		return	RSTypeInfo::typeUint16 ;
	}
	else if ( (- (int64_t) 0x80000000UL <= num) && (num <= 0x7FFFFFFF) )
	{
		return	RSTypeInfo::typeInt32 ;
	}
	else
	{
		return	RSTypeInfo::typeInt64 ;
	}
}

// 入れ子のローカル変数解放処理
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::FreeLocalNestVariable
	( RSInstructionParser::FunctionBlock& block,
				RSInstructionParser::LocalNest * pNest )
{
	for ( size_t i = 0; i < pNest->m_types.GetLength(); i ++ )
	{
		RSTypeInfo *	pLocal = pNest->m_types.GetAt( i ) ;
		if ( (pLocal != NULL) && pLocal->IsAllocated() )
		{
			if ( pLocal->IsObject() || pLocal->IsPointer() )
			{
				RSInstruction *	pInst = new RSInstruction ;
				pInst->FreeLocal( *pLocal ) ;
				block.AddCode( pInst ) ;
			}
		}
	}
	if ( pNest->m_control == nestTry )
	{
		RSInstruction *	pInst = new RSInstruction ;
		pInst->EndTry() ;
		block.AddCode( pInst ) ;
	}
	if ( pNest->m_control == nestSynchronized )
	{
		ESLAssert( pNest->m_fSpaceObj ) ;
		RSInstruction *	pInst = new RSInstruction ;
		pInst->Unsynchronize( pNest->m_typeSpaceObj ) ;
		block.AddCode( pInst ) ;
	}
	if ( pNest->m_fSpaceObj )
	{
		if ( pNest->m_typeSpaceObj.IsObject()
			|| pNest->m_typeSpaceObj.IsPointer() )
		{
			RSInstruction *	pInst = new RSInstruction ;
			pInst->FreeLocal( pNest->m_typeSpaceObj ) ;
			block.AddCode( pInst ) ;
		}
	}
}

// import 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementImport
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// import <script-file> ;
	//
	RSTypeInfo	typeFile ;
	SError	err =
		CompileExpression
			( typeFile, block, cstrm,
				RSCodeOperator::priorityNothing, exprNoOutputCode ) ;
	if ( err )
	{
		return	err ;
	}
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	//
	SString	strFile ;
	if ( (typeFile.m_pImmediate == NULL)
		|| !typeFile.m_pImmediate->AsString( strFile ) )
	{
		OutputError( L"インポートファイルを評価できません" ) ;
		return	errSuccess ;
	}
	if ( m_phase == phaseDeclaration )
	{
		LoadScript( strFile ) ;
	}
	return	errSuccess ;
}

// class 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementClass
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	if ( (m_phase != phaseDeclaration)
		&& (m_phase != phaseDefinition) )
	{
		PassBlockStatement( cstrm ) ;
		return	errSuccess ;
	}
	//
	// class [native] <name> [= <assign-class>]
	//		[extends <super-class>] [implements <super-class>, ...] { ... }
	//
	RSCodeSymbol *	pSymName = cstrm.NextSymbol() ;
	bool			flagNativeClass = false ;
	if ( pSymName == NULL )
	{
		if ( cstrm.NextControlWord( RSCodeControl::wiNative ) == NULL )
		{
			PassBlockStatement( cstrm ) ;
			OutputError( L"クラス名が指定されていません" ) ;
			return	errSuccess ;
		}
		pSymName = cstrm.NextSymbol() ;
		if ( pSymName == NULL )
		{
			PassBlockStatement( cstrm ) ;
			OutputError( L"クラス名が指定されていません" ) ;
			return	errSuccess ;
		}
		flagNativeClass = true ;
	}
	//
	// クラス定義
	//
	RSClass *	pClass = GetDefinedClassAs( block, pSymName->m_symbol ) ;
	if ( pClass != NULL )
	{
		if ( m_phase == phaseDeclaration )
		{
			PassBlockStatement( cstrm ) ;
			OutputError
				( pSymName->m_symbol + L" クラスが二重定義されています" ) ;
			return	errSuccess ;
		}
	}
	else
	{
		if ( m_phase == phaseDefinition )
		{
			OutputError
				( pSymName->m_symbol + L" クラスは宣言されていません" ) ;
		}
		pClass = new RSClass( m_vm->GetClassClass(), pSymName->m_symbol ) ;
		//
		RSObject *	pNamespace = block.m_pClassSpace ;
		if ( pNamespace == NULL )
		{
			pNamespace = m_vm ;
		}
		RSObject::ReleaseRef
			( pNamespace->CreateMemberAs
				( m_context, pSymName->m_symbol, pClass ) ) ;
		//
		pClass->m_pNamespace = block.m_pClassSpace ;
		pClass->m_flagNativeClass = flagNativeClass ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
	{
		return	errSuccess ;
	}
	//
	// クラス付加情報
	//
	ClassAppendix *	pAppendix ;
	if ( m_phase == phaseDeclaration )
	{
		pAppendix = new ClassAppendix ;
		pAppendix->m_pClass = pClass ;
		m_psoaClass.SetAs( pClass, pAppendix ) ;
	}
	else
	{
		pAppendix = m_psoaClass.GetAs( pClass ) ;
		if ( pAppendix == NULL )
		{
			PassBlockStatement( cstrm ) ;
			OutputError
				( pSymName->m_symbol + L" クラスは宣言されていません" ) ;
			return	errSuccess ;
		}
	}
	if ( m_phase == phaseDefinition )
	{
		//
		// JavaScript クラス名
		//
		if ( cstrm.NextOperator( RSCodeOperator::opMove ) != NULL )
		{
			pAppendix->m_strJSName = L"" ;
			for ( ; ; )
			{
				RSCodeSymbol *	pJSName = cstrm.NextSymbol() ;
				if ( pJSName == NULL )
				{
					PassBlockStatement( cstrm ) ;
					OutputError( L"クラス別名の構文エラーです" ) ;
					return	errSuccess ;
				}
				pAppendix->m_strJSName += pJSName->m_symbol ;
				//
				if ( cstrm.NextOperator( RSCodeOperator::opMemberOf ) == NULL )
				{
					break ;
				}
				pAppendix->m_strJSName += L"." ;
			}
		}
		//
		// クラス派生
		//
		if ( cstrm.NextControlWord( RSCodeControl::wiExtends ) != NULL )
		{
			RSClass *	pSuperClass = ParseClassExpression( cstrm ) ;
			if ( pSuperClass == NULL )
			{
				PassBlockStatement( cstrm ) ;
				OutputError( L"派生元クラスの指定が不正です" ) ;
				return	errSuccess ;
			}
			if ( pSuperClass->IsKindOf
					( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
			{
				PassBlockStatement( cstrm ) ;
				OutputError( L"クラスが構造体から派生しています" ) ;
				return	errSuccess ;
			}
			pAppendix->m_pSuperClass = pSuperClass ;
		}
		else
		{
			pAppendix->m_pSuperClass = m_vm->GetGenericObjectClass() ;
		}
		//
		// インターフェース
		//
		if ( cstrm.NextControlWord( RSCodeControl::wiImplements ) != NULL )
		{
			for ( ; ; )
			{
				RSClass *	pSuperClass = ParseClassExpression( cstrm ) ;
				if ( pSuperClass == NULL )
				{
					PassBlockStatement( cstrm ) ;
					OutputError( L"実装インターフェースの指定が不正です" ) ;
					return	errSuccess ;
				}
				if ( pSuperClass->IsKindOf
						( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
				{
					PassBlockStatement( cstrm ) ;
					OutputError( L"クラスが構造体から派生しています" ) ;
					return	errSuccess ;
				}
				if ( pAppendix->m_lstImplements.FindPtr( pSuperClass ) < 0 )
				{
					pAppendix->m_lstImplements.Add( pSuperClass ) ;
				}
				if ( cstrm.NextOperator( RSCodeOperator::opSequencing ) == NULL )
				{
					break ;
				}
			}
		}
		RSParenthesis *	pPrth =
				cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
		if ( pPrth == NULL )
		{
			OutputError( L"クラス構文エラーです" ) ;
			return	errSuccess ;
		}
		pAppendix->m_codeInit.m_pClassSpace = pClass ;
		pAppendix->m_protoInit.m_pNamespaceClass = pClass ;
		pAppendix->m_protoInit.m_pParenthesis = pPrth ;
	}
	else
	{
		RSParenthesis *	pPrth =
			cstrm.FindParenthesis( RSParenthesis::ptBrace ) ;
		if ( pPrth == NULL )
		{
			PassBlockStatement( cstrm ) ;
			return	errSuccess ;
		}
		pAppendix->m_codeInit.m_pClassSpace = pClass ;
		pAppendix->m_protoInit.m_pNamespaceClass = pClass ;
		pAppendix->m_protoInit.m_pParenthesis = pPrth ;
		//
		RSCodeStream	cs( *pPrth ) ;
		pClass->AddRef() ;
		m_context.PushNamespace( NULL, pClass, RSObject::modifierPrivate ) ;
		CompileAllStatements( pAppendix->m_codeInit, cs, pClass ) ;
		m_context.PopNamespace() ;
	}
	return	errSuccess ;
}

// struct 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementStruct
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	if ( (m_phase != phaseDeclaration)
		&& (m_phase != phaseDefinition) )
	{
		PassBlockStatement( cstrm ) ;
		return	errSuccess ;
	}
	//
	// struct <name> [extends <super-class>, ...] { ... }
	//
	RSCodeSymbol *	pSymName = cstrm.NextSymbol() ;
	if ( pSymName == NULL )
	{
		PassBlockStatement( cstrm ) ;
		OutputError( L"構造体名が指定されていません" ) ;
		return	errSuccess ;
	}
	//
	// 構造体定義
	//
	RSClass *	pClass = GetDefinedClassAs( block, pSymName->m_symbol ) ;
	RSStructuredPointerClass *
		pStructClass = ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
	if ( pClass != NULL )
	{
		if ( (m_phase == phaseDeclaration) || (pStructClass == NULL) )
		{
			PassBlockStatement( cstrm ) ;
			OutputError
				( pSymName->m_symbol + L" クラスが二重定義されています" ) ;
			return	errSuccess ;
		}
	}
	else
	{
		if ( m_phase == phaseDefinition )
		{
			OutputError
				( pSymName->m_symbol + L" クラスは宣言されていません" ) ;
		}
		pStructClass =
			new RSStructuredPointerClass
				( m_vm->GetClassClass(), pSymName->m_symbol ) ;
		pClass = pStructClass ;
		//
		RSObject *	pNamespace = block.m_pClassSpace ;
		if ( pNamespace == NULL )
		{
			pNamespace = m_vm ;
		}
		RSObject::ReleaseRef
			( pNamespace->CreateMemberAs
				( m_context, pSymName->m_symbol, pClass ) ) ;
		//
		pClass->m_pNamespace = block.m_pClassSpace ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
	{
		return	errSuccess ;
	}
	//
	// クラス付加情報
	//
	ClassAppendix *	pAppendix ;
	if ( m_phase == phaseDeclaration )
	{
		pAppendix = new ClassAppendix ;
		pAppendix->m_pClass = pClass ;
		m_psoaClass.SetAs( pClass, pAppendix ) ;
	}
	else
	{
		pAppendix = m_psoaClass.GetAs( pClass ) ;
		if ( pAppendix == NULL )
		{
			PassBlockStatement( cstrm ) ;
			OutputError
				( pSymName->m_symbol + L" 構造体は宣言されていません" ) ;
			return	errSuccess ;
		}
	}
	if ( m_phase == phaseDefinition )
	{
		//
		// 構造体派生
		//
		if ( cstrm.NextControlWord( RSCodeControl::wiExtends ) != NULL )
		{
			RSClass *	pSuperClass = ParseClassExpression( cstrm ) ;
			if ( (pSuperClass == NULL)
				|| !pSuperClass->IsKindOf
						( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
			{
				PassBlockStatement( cstrm ) ;
				OutputError( L"派生元構造体が不正です" ) ;
				return	errSuccess ;
			}
			pAppendix->m_pSuperClass = pSuperClass ;
			//
			while ( cstrm.NextOperator( RSCodeOperator::opSequencing ) != NULL )
			{
				pSuperClass = ParseClassExpression( cstrm ) ;
				if ( (pSuperClass == NULL)
					|| !pSuperClass->IsKindOf
							( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
				{
					PassBlockStatement( cstrm ) ;
					OutputError( L"派生元構造体が不正です" ) ;
					return	errSuccess ;
				}
				if ( pAppendix->m_lstImplements.FindPtr( pSuperClass ) < 0 )
				{
					pAppendix->m_lstImplements.Add( pSuperClass ) ;
				}
			}
		}
		else
		{
			pAppendix->m_pSuperClass = m_vm->GetStructureClass() ;
		}
		RSParenthesis *	pPrth =
				cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
		if ( pPrth == NULL )
		{
			OutputError( L"構造体構文エラーです" ) ;
			return	errSuccess ;
		}
		pAppendix->m_codeInit.m_pClassSpace = pClass ;
		pAppendix->m_protoInit.m_pNamespaceClass = pClass ;
		pAppendix->m_protoInit.m_pParenthesis = pPrth ;
	}
	else
	{
		RSParenthesis *	pPrth =
			cstrm.FindParenthesis( RSParenthesis::ptBrace ) ;
		if ( pPrth == NULL )
		{
			PassBlockStatement( cstrm ) ;
			return	errSuccess ;
		}
		pAppendix->m_codeInit.m_pClassSpace = pClass ;
		pAppendix->m_protoInit.m_pNamespaceClass = pClass ;
		pAppendix->m_protoInit.m_pParenthesis = pPrth ;
		//
		RSCodeStream	cs( *pPrth ) ;
		pClass->AddRef() ;
		m_context.PushNamespace( NULL, pClass, RSObject::modifierPrivate ) ;
		CompileAllStatements( pAppendix->m_codeInit, cs, pClass ) ;
		m_context.PopNamespace() ;
	}
	return	errSuccess ;
}

// for 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementFor
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSParenthesis *	pPrthFor =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthFor == NULL )
	{
		OutputError( L"for 文の構文エラーです" ) ;
		return	errSuccess ;
	}
	RSCodeStream	csFor( *pPrthFor ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( m_phase != phaseImplementation )
	{
		if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCode, NULL ) ;
		}
		return	errSuccess ;
	}
	RSCode *	pCodeInSym = csFor.GetTerm( 1 ) ;
	if ( (pCodeInSym != NULL)
		&& (pCodeInSym->m_type == RSCode::typeSymbol)
		&& (((RSCodeSymbol*)pCodeInSym)->m_symbol == L"in") )
	{
		CompileStatementForIn( block, csFor, csCode ) ;
	}
	else
	{
		CompileStatementForIter( block, csFor, csCode ) ;
	}
	return	errSuccess ;
}

SSystem::SError RSInstructionParser::CompileStatementForIn
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& csFor, RSCodeStream& csCode )
{
	//
	// for ( <iter-name> in <obj-expr> ) { ... }
	//
	RSCodeSymbol *	pSymVarName = csFor.NextSymbol() ;
	if ( pSymVarName == NULL )
	{
		OutputError( L"for in 文の反復変数名が不正です" ) ;
		return	errFailed ;
	}
	ESLVerify( csFor.NextSymbol() != NULL ) ;	// = 'in'
	//
	LocalNest *	pForNest = block.m_local.DescendNest( nestFor ) ;
	block.m_fpExpr = block.m_fpLocal ;
	block.m_fpExpr.iLocal += 2 ;
	block.m_fpExpr.iNumber += 2 ;
	//
	SError		err ;
	RSTypeInfo	typeArray ;
	err = CompileExpression( typeArray, block, csFor ) ;
	if ( err )
	{
		return	err ;
	}
	RSDynamicObjectClass *	pHashClass =
		ESLTypeCast<RSDynamicObjectClass>( typeArray.m_pClass ) ;
	RSGenericArrayClass *	pArrayClass =
		ESLTypeCast<RSGenericArrayClass>( typeArray.m_pClass ) ;
	SString	strArrayObj, strIterName, strIterCount ;
	if ( pHashClass != NULL )
	{
		//
		// キー配列
		//
		RSInstruction *	pInstKeys = new RSInstruction ;
		pInstKeys->KeysOf( m_context, typeArray ) ;
		AllocateTemporary( block, pInstKeys->m_typeDst ) ;
		block.AddCode( pInstKeys ) ;
		//
		RSTypeInfo *	ptiArrayObj = new RSTypeInfo ;
		ptiArrayObj->SetType
			( m_context,
				m_vm->GetArrayClassAs( m_vm->GetStringClass() ) ) ;
		CreateNamelessVariableAs( block, strArrayObj, ptiArrayObj ) ;
		//
		err = CompileMoveToLocalReference
				( block, strArrayObj, pInstKeys->m_typeDst ) ;
		if ( err )
		{
			return	err ;
		}
		typeArray = pInstKeys->m_typeDst ;
		//
		// 反復変数
		//
		CreateVariableTypeAs
			( block, pSymVarName->m_symbol, 0, m_vm->GetStringClass() ) ;
	}
	else if ( pArrayClass != NULL )
	{
		//
		// 反復変数
		//
		CreateVariableTypeAs
			( block, pSymVarName->m_symbol, 0, m_vm->GetIntegerClass() ) ;
	}
	else
	{
		OutputError( L"for in 反復対象が配列ではありません" ) ;
		return	errFailed ;
	}
	//
	// 配列長
	//
	RSInstruction *	pInstLen = new RSInstruction ;
	pInstLen->LengthOf( m_context, typeArray ) ;
	AllocateTemporary( block, pInstLen->m_typeDst ) ;
	block.AddCode( pInstLen ) ;
	//
	RSTypeInfo *	ptiLength = new RSTypeInfo ;
	ptiLength->SetType( m_context, m_vm->GetIntegerClass() ) ;
	CreateNamelessVariableAs( block, strIterCount, ptiLength ) ;
	ptiLength = block.m_local.GetNestedVariable( strIterCount ) ;
	ESLAssert( ptiLength != NULL ) ;
	//
	err = CompileMoveToLocalReference
			( block, strIterCount, pInstLen->m_typeDst ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 反復変数
	//
	RSTypeInfo *	ptiIter = new RSTypeInfo ;
	ptiIter->SetType( m_context, m_vm->GetIntegerClass() ) ;
	CreateNamelessVariableAs( block, strIterName, ptiIter ) ;
	ptiIter = block.m_local.GetNestedVariable( strIterName ) ;
	ESLAssert( ptiIter != NULL ) ;
	//
	RSTypeInfo	typeZero ;
	typeZero.SetImmediate
		( m_context, m_context.new_Integer( 0 ),
							m_context.GetIntegerClass() ) ;
	err = CompileMoveToLocalReference( block, strIterName, typeZero ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 反復開始位置：反復継続判定
	//
	size_t	ipLoopStart = block.GetCurrentPos() ;
	//
	RSInstruction *	pInstCond = new RSInstruction ;
	pInstCond->BinOperator
		( m_context, RSCodeOperator::opLessThan,
			m_vm->GetBooleanClass(), *ptiIter, *ptiLength ) ;
	AllocateTemporary( block, pInstCond->m_typeDst ) ;
	block.AddCode( pInstCond ) ;
	//
	RSInstruction *	pInstJumpBreak = new RSInstruction ;
	pInstJumpBreak->JumpNotIf
		( m_context, pInstCond->m_typeDst, ipLoopStart ) ;
	block.AddCode( pInstJumpBreak ) ;
	ResetStatementContext( block ) ;
	//
	// 反復処理
	//
	LocalNest *	pLoopNest = block.m_local.DescendNest( nestFor ) ;
	//
	RSTypeInfo		tiIndex ;
	RSCodeStream	csDummy ;
	err = CompileLocalReference
				( tiIndex, block, csDummy, strIterName, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	if ( pHashClass != NULL )
	{
		RSInstruction *	pInst = new RSInstruction ;
		pInst->LoadIndirectElementInt( m_context, typeArray, tiIndex, true ) ;
		AllocateTemporary( block, pInst->m_typeDst ) ;
		block.AddCode( pInst ) ;
		//
		err = CompileMoveToLocalReference
				( block, pSymVarName->m_symbol, pInst->m_typeDst ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else if ( pArrayClass != NULL )
	{
		err = CompileMoveToLocalReference
				( block, pSymVarName->m_symbol, tiIndex ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	CompileAllStatements( block, csCode, NULL ) ;
	//
	FreeLocalNestVariable( block, pLoopNest ) ;
	ESLVerify( block.m_local.AscendNest() == pLoopNest ) ;
	//
	pLoopNest->CommitContinueTargets( block.GetCurrentPos() ) ;
	//
	// 反復継続
	//
	RSTypeInfo	typeRefIter ;
	err = CompileLocalReference
			( typeRefIter, block, csDummy, strIterName, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	RSTypeInfo	typeOne ;
	typeOne.SetImmediate
		( m_context, m_context.new_Integer( 1 ), m_vm->GetIntegerClass() ) ;
	err = CompileBinaryOperator
		( typeRefIter, block, 0, typeOne, RSCodeOperator::opMoveAdd ) ;
	if ( err )
	{
		return	err ;
	}
	ResetStatementContext( block ) ;
	//
	RSInstruction *	pJumpRewind = new RSInstruction ;
	pJumpRewind->Jump( ipLoopStart ) ;
	block.AddCode( pJumpRewind ) ;
	//
	pInstJumpBreak->m_ipTarget = block.GetCurrentPos() ;
	pLoopNest->CommitBreakTargets( block.GetCurrentPos() ) ;
	FreeLocalNestVariable( block, pForNest ) ;
	//
	ESLVerify( block.m_local.AscendNest() == pForNest ) ;
	delete	pLoopNest ;
	delete	pForNest ;
	//
	return	errSuccess ;
}

SSystem::SError RSInstructionParser::CompileStatementForIter
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& csFor, RSCodeStream& csCode )
{
	//
	// for ( init-decl cond-expr; step-expr ) { ... }
	// for ( init-expr; cond-expr; step-expr ) { ... }
	//
	SError		err ;
	LocalNest *	pForNest = block.m_local.DescendNest( nestFor ) ;
	//
	// 初期値
	RSClass *	pIterVarType = ParseClassExpression( csFor ) ;
	if ( pIterVarType != NULL )
	{
		err = CompileDeclareVariable
			( block, csFor, 0, pIterVarType, declModeVar ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else if ( csFor.NextOperator
				( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		RSTypeInfo	typeTemp ;
		err = CompileExpression( typeTemp, block, csFor ) ;
		if ( err )
		{
			return	err ;
		}
		csFor.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	}
	//
	// 反復開始位置：反復継続判定
	//
	size_t	ipLoopStart = block.GetCurrentPos() ;
	//
	RSInstruction *	pInstJumpBreak = NULL ;
	if ( csFor.NextOperator
			( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		RSTypeInfo	typeCondExpr ;
		err = CompileExpression( typeCondExpr, block, csFor ) ;
		if ( err )
		{
			return	err ;
		}
		csFor.NextOperator( RSCodeOperator::opEndOfStatement ) ;
		//
		err = CompileCastBoolean( typeCondExpr, block, 0, castNatural ) ;
		if ( err )
		{
			return	err ;
		}
		pInstJumpBreak = new RSInstruction ;
		pInstJumpBreak->JumpNotIf
			( m_context, typeCondExpr, ipLoopStart ) ;
		block.AddCode( pInstJumpBreak ) ;
		ResetStatementContext( block ) ;
	}
	//
	// 反復処理
	//
	LocalNest *	pLoopNest = block.m_local.DescendNest( nestFor ) ;
	//
	CompileAllStatements( block, csCode, NULL ) ;
	//
	FreeLocalNestVariable( block, pLoopNest ) ;
	ESLVerify( block.m_local.AscendNest() == pLoopNest ) ;
	//
	pLoopNest->CommitContinueTargets( block.GetCurrentPos() ) ;
	//
	// 反復継続
	//
	if ( !csFor.IsEndOfStream() )
	{
		RSTypeInfo	typeTemp ;
		err = CompileExpression( typeTemp, block, csFor ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	RSInstruction *	pJumpRewind = new RSInstruction ;
	pJumpRewind->Jump( ipLoopStart ) ;
	block.AddCode( pJumpRewind ) ;
	//
	if ( pInstJumpBreak != NULL )
	{
		pInstJumpBreak->m_ipTarget = block.GetCurrentPos() ;
	}
	pLoopNest->CommitBreakTargets( block.GetCurrentPos() ) ;
	FreeLocalNestVariable( block, pForNest ) ;
	//
	ESLVerify( block.m_local.AscendNest() == pForNest ) ;
	delete	pLoopNest ;
	delete	pForNest ;
	//
	return	errSuccess ;
}

// while 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementWhile
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// while ( expr ) { ... }
	//
	RSParenthesis *	pPrthWhile =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthWhile == NULL )
	{
		OutputError( L"while 文の構文エラーです" ) ;
		return	errSuccess ;
	}
	RSCodeStream	csWhile( *pPrthWhile ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( m_phase != phaseImplementation )
	{
		if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCode, NULL ) ;
		}
		return	errSuccess ;
	}
	//
	// 反復開始位置：反復継続判定
	//
	size_t		ipLoopStart = block.GetCurrentPos() ;
	//
	RSTypeInfo	typeCondExpr ;
	SError		err ;
	err = CompileExpression( typeCondExpr, block, csWhile ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCastBoolean( typeCondExpr, block, 0, castNatural ) ;
	if ( err )
	{
		return	err ;
	}
	RSInstruction *	pInstJumpBreak = new RSInstruction ;
	pInstJumpBreak->JumpNotIf( m_context, typeCondExpr, ipLoopStart ) ;
	block.AddCode( pInstJumpBreak ) ;
	ResetStatementContext( block ) ;
	//
	// 反復処理
	//
	LocalNest *	pLoopNest = block.m_local.DescendNest( nestWhile ) ;
	//
	CompileAllStatements( block, csCode, NULL ) ;
	//
	FreeLocalNestVariable( block, pLoopNest ) ;
	ESLVerify( block.m_local.AscendNest() == pLoopNest ) ;
	//
	pLoopNest->CommitContinueTargets( ipLoopStart ) ;
	//
	// 反復継続
	//
	RSInstruction *	pJumpRewind = new RSInstruction ;
	pJumpRewind->Jump( ipLoopStart ) ;
	block.AddCode( pJumpRewind ) ;
	//
	// 脱出
	//
	pInstJumpBreak->m_ipTarget = block.GetCurrentPos() ;
	pLoopNest->CommitBreakTargets( block.GetCurrentPos() ) ;
	delete	pLoopNest ;
	//
	return	errSuccess ;
}

// do 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementDo
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// do { ... } while ( expr ) ;
	//
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( cstrm.NextControlWord( RSCodeControl::wiWhile ) == NULL )
	{
		OutputError( L"do ～ while 文の構文エラーです" ) ;
		return	errFailed ;
	}
	RSParenthesis *	pPrthWhile =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthWhile == NULL )
	{
		OutputError( L"do ～ while 文の反復条件がありません" ) ;
		return	errFailed ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		OutputError( L"do ～ while 文末のセミコロンがありません" ) ;
		return	errSuccess ;
	}
	if ( m_phase != phaseImplementation )
	{
		if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCode, NULL ) ;
		}
		return	errSuccess ;
	}
	//
	// 反復処理
	//
	size_t		ipLoopStart = block.GetCurrentPos() ;
	//
	LocalNest *	pLoopNest = block.m_local.DescendNest( nestDo ) ;
	//
	CompileAllStatements( block, csCode, NULL ) ;
	//
	FreeLocalNestVariable( block, pLoopNest ) ;
	ESLVerify( block.m_local.AscendNest() == pLoopNest ) ;
	//
	pLoopNest->CommitContinueTargets( block.GetCurrentPos() ) ;
	//
	// 反復継続判定
	//
	RSCodeStream	csWhile( *pPrthWhile ) ;
	RSTypeInfo		typeCondExpr ;
	SError			err ;
	err = CompileExpression( typeCondExpr, block, csWhile ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCastBoolean( typeCondExpr, block, 0, castNatural ) ;
	if ( err )
	{
		return	err ;
	}
	RSInstruction *	pInstJumpWhile = new RSInstruction ;
	pInstJumpWhile->JumpIf( m_context, typeCondExpr, ipLoopStart ) ;
	block.AddCode( pInstJumpWhile ) ;
	ResetStatementContext( block ) ;
	//
	// 脱出
	//
	pLoopNest->CommitBreakTargets( block.GetCurrentPos() ) ;
	delete	pLoopNest ;
	//
	return	errSuccess ;
}

// if 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementIf
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// if ( expr ) statement [else statement]
	//
	RSParenthesis *	pPrthIf =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthIf == NULL )
	{
		OutputError( L"if 文の条件式がありません" ) ;
		return	errSuccess ;
	}
	RSCodeStream	csCode ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	RSInstruction *	pInstJumpIf = NULL ;
	if ( m_phase == phaseImplementation )
	{
		//
		// 条件判定
		//
		RSCodeStream	csIf( *pPrthIf ) ;
		RSTypeInfo		typeCondExpr ;
		SError			err ;
		err = CompileExpression( typeCondExpr, block, csIf ) ;
		if ( err )
		{
			return	err ;
		}
		err = CompileCastBoolean( typeCondExpr, block, 0, castNatural ) ;
		if ( err )
		{
			return	err ;
		}
		pInstJumpIf = new RSInstruction ;
		pInstJumpIf->JumpNotIf
			( m_context, typeCondExpr, block.GetCurrentPos() ) ;
		block.AddCode( pInstJumpIf ) ;
		ResetStatementContext( block ) ;
		//
		// 条件一致時処理
		//
		LocalNest *	pNestIf = block.m_local.DescendNest( nestIf ) ;
		CompileAllStatements( block, csCode, NULL ) ;
		FreeLocalNestVariable( block, pNestIf ) ;
		ESLVerify( block.m_local.AscendNest() == pNestIf ) ;
		delete	pNestIf ;
	}
	else if ( m_phase == phaseAllocation )
	{
		CompileAllStatements( block, csCode, NULL ) ;
	}
	if ( cstrm.NextControlWord( RSCodeControl::wiElse ) != NULL )
	{
		pPrthCode = cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
		if ( pPrthCode != NULL )
		{
			csCode.AttachCode( *pPrthCode ) ;
		}
		else
		{
			size_t	iStart = cstrm.GetIndex() ;
			cstrm.PassAStatement() ;
			csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
		}
		if ( m_phase == phaseImplementation )
		{
			RSInstruction *	pInstJumpElse = new RSInstruction ;
			pInstJumpElse->Jump( block.GetCurrentPos() ) ;
			block.AddCode( pInstJumpElse ) ;
			//
			pInstJumpIf->m_ipTarget = block.GetCurrentPos() ;
			//
			// 条件不一致時処理
			//
			LocalNest *	pNestElse = block.m_local.DescendNest( nestElse ) ;
			CompileAllStatements( block, csCode, NULL ) ;
			FreeLocalNestVariable( block, pNestElse ) ;
			ESLVerify( block.m_local.AscendNest() == pNestElse ) ;
			delete	pNestElse ;
			//
			pInstJumpElse->m_ipTarget = block.GetCurrentPos() ;
		}
		else if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCode, NULL ) ;
		}
	}
	else
	{
		if ( pInstJumpIf != NULL )
		{
			pInstJumpIf->m_ipTarget = block.GetCurrentPos() ;
		}
	}
	return	errSuccess ;
}

// switch 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementSwitch
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSParenthesis *	pPrthSwitch =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthSwitch == NULL )
	{
		OutputError( L"switch 文評価式がありません" ) ;
		PassBlockStatement( cstrm ) ;
		return	errFailed ;
	}
	RSParenthesis *	pPrthCase =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthCase == NULL )
	{
		OutputError( L"switch 文ブロックがありません" ) ;
		return	errSuccess ;
	}
	RSCodeStream	csCase( *pPrthCase ) ;
	if ( m_phase != phaseImplementation )
	{
		if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCase, NULL ) ;
		}
		return	errSuccess ;
	}
	RSInstruction *	pInstJumpSwitch = new RSInstruction ;
	pInstJumpSwitch->Jump( block.GetCurrentPos() ) ;
	block.AddCode( pInstJumpSwitch ) ;
	//
	// switch ブロック
	//
	LocalNest *	pSwitchNest = block.m_local.DescendNest( nestSwitch ) ;
	//
	CompileAllStatements( block, csCase, NULL ) ;
	//
	FreeLocalNestVariable( block, pSwitchNest ) ;
	ESLVerify( block.m_local.AscendNest() == pSwitchNest ) ;
	//
	RSInstruction *	pInstContinue = new RSInstruction ;
	pInstContinue->Jump( block.GetCurrentPos() ) ;
	block.AddCode( pInstContinue ) ;
	//
	// 分岐値評価
	//
	SError		err ;
	RSTypeInfo	typeExpr ;
	err = CompileExpression( typeExpr, block, csCase ) ;
	if ( err )
	{
		return	err ;
	}
	for ( size_t i = 0; i < pSwitchNest->m_caseLabels.GetLength(); i ++ )
	{
		LabelInfo *	pLabelInf = pSwitchNest->m_caseLabels.GetAt( i ) ;
		ESLAssert( pLabelInf != NULL ) ;
		if ( pLabelInf == NULL )
		{
			continue ;
		}
		ESLAssert( pLabelInf->m_typeCase.m_pImmediate != NULL ) ;
		//
		RSTypeInfo	typeTemp = typeExpr ;
		err = CompileBinaryOperator
			( typeTemp, block, 0,
				pLabelInf->m_typeCase, RSCodeOperator::opEqual ) ;
		if ( err )
		{
			continue ;
		}
		RSInstruction *	pInstJumpCase = new RSInstruction ;
		pInstJumpCase->JumpIf( m_context, typeTemp, pLabelInf->m_ipLabelPos ) ;
		block.AddCode( pInstJumpCase ) ;
	}
	if ( pSwitchNest->m_ipDefault >= 0 )
	{
		RSInstruction *	pInstJump = new RSInstruction ;
		pInstJump->Jump( (size_t) pSwitchNest->m_ipDefault ) ;
		block.AddCode( pInstJump ) ;
	}
	ResetStatementContext( block ) ;
	//
	pInstContinue->m_ipTarget = block.GetCurrentPos() ;
	pSwitchNest->CommitBreakTargets( block.GetCurrentPos() ) ;
	pSwitchNest->CommitContinueTargets( block.GetCurrentPos() ) ;
	delete	pSwitchNest ;
	//
	return	errSuccess ;
}

// case 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementCase
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	LocalNest *	pSwitchNest = block.m_local.GetCurrentNest() ;
	if ( (pSwitchNest == NULL)
		|| (pSwitchNest->m_control != nestSwitch) )
	{
		cstrm.FindOperator( RSCodeOperator::opSeparator ) ;
		OutputError( L"case 文が switch 文に対応しません" ) ;
		return	errSuccess ;
	}
	if ( m_phase != phaseImplementation )
	{
		cstrm.FindOperator( RSCodeOperator::opSeparator ) ;
		return	errSuccess ;
	}
	//
	// case 値評価
	//
	SError		err ;
	RSTypeInfo	typeExpr ;
	err = CompileExpression
		( typeExpr, block, cstrm,
			RSCodeOperator::prioritySeparator, exprNoOutputCode ) ;
	if ( err )
	{
		cstrm.FindOperator( RSCodeOperator::opSeparator ) ;
		return	err ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opSeparator ) == NULL )
	{
		OutputError( L"case 文に \':\' が見つかりません" ) ;
		return	errSuccess ;
	}
	for ( size_t i = 0; i < pSwitchNest->m_caseLabels.GetLength(); i ++ )
	{
		LabelInfo *	pli = pSwitchNest->m_caseLabels.GetAt( i ) ;
		if ( pli == NULL )
		{
			continue ;
		}
		if ( typeExpr.m_pImmediate
			&& pli->m_typeCase.m_pImmediate
			&& typeExpr.m_pImmediate->IsEqualObject
						( pli->m_typeCase.m_pImmediate ) )
		{
			OutputError( L"同値 の case 文が複数記述されています" ) ;
		return	errSuccess ;
		}
	}
	LabelInfo *	pliCase = new LabelInfo ;
	pliCase->m_typeCase = typeExpr ;
	pliCase->m_ipLabelPos = block.GetCurrentPos() ;
	pSwitchNest->m_caseLabels.Add( pliCase ) ;
	//
	return	errSuccess ;
}

// default 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementDefault
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	LocalNest *	pSwitchNest = block.m_local.GetCurrentNest() ;
	if ( (pSwitchNest == NULL)
		|| (pSwitchNest->m_control != nestSwitch) )
	{
		cstrm.FindOperator( RSCodeOperator::opSeparator ) ;
		OutputError( L"default 文が switch 文に対応しません" ) ;
		return	errSuccess ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opSeparator ) == NULL )
	{
		OutputError( L"default 文に \':\' が見つかりません" ) ;
		return	errSuccess ;
	}
	if ( m_phase != phaseImplementation )
	{
		return	errSuccess ;
	}
	if ( pSwitchNest->m_ipDefault >= 0 )
	{
		OutputError( L"default 文が複数記述されています" ) ;
		return	errSuccess ;
	}
	pSwitchNest->m_ipDefault = (ssize_t) block.GetCurrentPos() ;
	return	errSuccess ;
}

// break 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementBreak
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		OutputError( L"break 文に \';\' が見つかりません" ) ;
		return	errSuccess ;
	}
	if ( m_phase != phaseImplementation )
	{
		return	errSuccess ;
	}
	bool	flagBreak = false ;
	for ( size_t i = 0; i < block.m_local.GetNestDepth(); i ++ )
	{
		LocalNest *	pNest = block.m_local.GetNestAt( i ) ;
		if ( pNest != NULL )
		{
			FreeLocalNestVariable( block, pNest ) ;
			//
			if ( (pNest->m_control == nestFor)
				|| (pNest->m_control == nestWhile)
				|| (pNest->m_control == nestDo)
				|| (pNest->m_control == nestSwitch) )
			{
				RSInstruction *	pInstJump = new RSInstruction ;
				pInstJump->Jump( block.GetCurrentPos() ) ;
				block.AddCode( pInstJump ) ;
				pNest->m_breaks.Add( pInstJump ) ;
				flagBreak = true ;
				break ;
			}
		}
	}
	if ( !flagBreak )
	{
		OutputError( L"break に対応する反復文が見つかりません" ) ;
		return	errSuccess ;
	}
	return	errSuccess ;
}

// continue 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementContinue
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		OutputError( L"continue 文に \';\' が見つかりません" ) ;
		return	errSuccess ;
	}
	if ( m_phase != phaseImplementation )
	{
		return	errSuccess ;
	}
	bool	flagContinue = false ;
	for ( size_t i = 0; i < block.m_local.GetNestDepth(); i ++ )
	{
		LocalNest *	pNest = block.m_local.GetNestAt( i ) ;
		if ( pNest != NULL )
		{
			FreeLocalNestVariable( block, pNest ) ;
			//
			if ( (pNest->m_control == nestFor)
				|| (pNest->m_control == nestWhile)
				|| (pNest->m_control == nestDo)
				|| (pNest->m_control == nestSwitch) )
			{
				RSInstruction *	pInstJump = new RSInstruction ;
				pInstJump->Jump( block.GetCurrentPos() ) ;
				block.AddCode( pInstJump ) ;
				pNest->m_continues.Add( pInstJump ) ;
				flagContinue = true ;
				break ;
			}
		}
	}
	if ( !flagContinue )
	{
		OutputError( L"continue に対応する反復文が見つかりません" ) ;
		return	errSuccess ;
	}
	return	errSuccess ;
}

// try 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementTry
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	//
	// try { ... } catch ( expr ) { ... } ...
	//
	RSParenthesis *	pPrthTryCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	RSCodeStream	csCode ;
	if ( pPrthTryCode != NULL )
	{
		csCode.AttachCode( *pPrthTryCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( m_phase == phaseImplementation )
	{
		//
		// try ブロック
		//
		RSInstruction *	pInstTry = new RSInstruction ;
		pInstTry->BeginTry( block.GetCurrentPos() ) ;
		block.AddCode( pInstTry ) ;
		//
		LocalNest *	pTryNest = block.m_local.DescendNest( nestTry ) ;
		//
		CompileAllStatements( block, csCode, NULL ) ;
		FreeLocalNestVariable( block, pTryNest ) ;
		//
		ESLVerify( block.m_local.AscendNest() == pTryNest ) ;
		delete	pTryNest ;
		//
		RSInstruction *	pInstJumpEndTry = new RSInstruction ;
		pInstJumpEndTry->Jump( block.GetCurrentPos() ) ;
		block.AddCode( pInstJumpEndTry ) ;
		//
		pInstTry->m_ipTarget = block.GetCurrentPos() ;
		//
		// catch リスト
		//
		SPointerArray<RSInstruction>	breakCatch ;
		while ( cstrm.NextControlWord( RSCodeControl::wiCatch ) != NULL )
		{
			RSParenthesis *	pPrthCatch =
					cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
			RSParenthesis *	pPrthCatchCode =
					cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
			if ( pPrthCatchCode != NULL )
			{
				csCode.AttachCode( *pPrthCatchCode ) ;
			}
			else
			{
				size_t	iStart = cstrm.GetIndex() ;
				cstrm.PassAStatement() ;
				csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
			}
			if ( pPrthCatch == NULL )
			{
				OutputError( L"catch 文の受け取り型が記述されていません" ) ;
				continue ;
			}
			RSCodeStream	csCatch ;
			csCatch.AttachCode( *pPrthCatch ) ;
			RSClass *	pClass = ParseClassExpression( csCatch ) ;
			if ( pClass == NULL )
			{
				OutputError( L"catch の受け取り型を評価できません" ) ;
				continue ;
			}
			RSCodeSymbol *	pSymName = csCatch.NextSymbol() ;
			if ( pSymName != NULL )
			{
				CreateVariableTypeAs
					( block, pSymName->m_symbol, 0, pClass ) ;
			}
			//
			// 例外判定
			//
			RSTypeInfo	typeException ;
			typeException.SetType
				( m_context, m_context.GetGenericObjectClass() ) ;
			AllocateTemporary( block, typeException ) ;
			//
			RSInstruction *	pInstGetExcept = new RSInstruction ;
			pInstGetExcept->GetException( typeException ) ;
			block.AddCode( pInstGetExcept ) ;
			//
			RSTypeInfo	typeExceptType ;
			typeExceptType.SetImmediate
				( m_context, pClass, pClass->GetRSClass() ) ;
			//
			RSInstruction *	pInstInstanceOf = new RSInstruction ;
			pInstInstanceOf->BinOperator
				( m_context, RSCodeOperator::opInstanceOf,
					m_context.GetBooleanClass(),
					typeException, typeExceptType ) ;
			block.AddCode( pInstInstanceOf ) ;
			AllocateTemporary( block, pInstInstanceOf->m_typeDst ) ;
			//
			RSInstruction *	pInstJumpNoCatch = new RSInstruction ;
			pInstJumpNoCatch->JumpNotIf
				( m_context,
					pInstInstanceOf->m_typeDst, block.GetCurrentPos() ) ;
			block.AddCode( pInstJumpNoCatch ) ;
			//
			WriteFreeAllTemporary( block ) ;
			//
			RSInstruction *	pInstClearExcept = new RSInstruction ;
			pInstClearExcept->ClearException() ;
			block.AddCode( pInstClearExcept ) ;
			//
			// catch ブロック処理
			//
			LocalNest *	pCatchNest = block.m_local.DescendNest( nestSpace ) ;
			//
			CompileAllStatements( block, csCode, NULL ) ;
			FreeLocalNestVariable( block, pCatchNest ) ;
			//
			ESLVerify( block.m_local.AscendNest() == pCatchNest ) ;
			delete	pCatchNest ;
			//
			RSInstruction *	pInstJumpEndCatch = new RSInstruction ;
			pInstJumpEndCatch->Jump( block.GetCurrentPos() ) ;
			block.AddCode( pInstJumpEndCatch ) ;
			breakCatch.Add( pInstJumpEndCatch ) ;
			//
			pInstJumpNoCatch->m_ipTarget = block.GetCurrentPos() ;
		}
		//
		// キャッチされない例外はそのまま投げる
		//
		RSTypeInfo	typeException ;
		typeException.SetType
			( m_context, m_context.GetGenericObjectClass() ) ;
		AllocateTemporary( block, typeException ) ;
		//
		RSInstruction *	pInstGetExcept = new RSInstruction ;
		pInstGetExcept->GetException( typeException ) ;
		block.AddCode( pInstGetExcept ) ;
		//
		RSInstruction *	pInstThrow = new RSInstruction ;
		pInstThrow->Throw( typeException ) ;
		block.AddCode( pInstThrow ) ;
		//
		// 終了
		//
		for ( size_t i = 0; i < breakCatch.GetLength(); i ++ )
		{
			RSInstruction *	pInstBreak = breakCatch.GetAt( i ) ;
			pInstBreak->m_ipTarget = block.GetCurrentPos() ;
		}
		pInstJumpEndTry->m_ipTarget = block.GetCurrentPos() ;
	}
	else
	{
		if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCode, NULL ) ;
		}
		while ( cstrm.NextControlWord( RSCodeControl::wiCatch ) != NULL )
		{
			RSParenthesis *	pPrthCatch =
					cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
			RSParenthesis *	pPrthCatchCode =
					cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
			if ( pPrthCatchCode != NULL )
			{
				csCode.AttachCode( *pPrthCatchCode ) ;
			}
			else
			{
				size_t	iStart = cstrm.GetIndex() ;
				cstrm.PassAStatement() ;
				csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
			}
			if ( pPrthCatch == NULL )
			{
				OutputError( L"catch 文の受け取り型が記述されていません" ) ;
				continue ;
			}
			RSCodeStream	csCatch ;
			csCatch.AttachCode( *pPrthCatch ) ;
			RSClass *	pClass = ParseClassExpression( csCatch ) ;
			if ( pClass == NULL )
			{
				OutputError( L"catch の受け取り型を評価できません" ) ;
				continue ;
			}
			RSCodeSymbol *	pSymName = csCatch.NextSymbol() ;
			if ( pSymName != NULL )
			{
				CreateVariableTypeAs
					( block, pSymName->m_symbol, 0, pClass ) ;
			}
			if ( m_phase == phaseAllocation )
			{
				CompileAllStatements( block, csCode, NULL ) ;
			}
		}
	}
	return	errSuccess ;
}

// throw 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementThrow
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	if ( m_phase != phaseImplementation )
	{
		PassExpression( cstrm ) ;
		return	errSuccess ;
	}
	RSTypeInfo	typeTemp ;
	SError	err = CompileExpression( typeTemp, block, cstrm ) ;
	if ( err )
	{
		return	err ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		OutputError( L"throw 文に \';\' が見つかりません" ) ;
		return	errSuccess ;
	}
	RSInstruction *	pInstThrow = new RSInstruction ;
	pInstThrow->Throw( typeTemp ) ;
	block.AddCode( pInstThrow ) ;
	return	errSuccess ;
}

// return 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementReturn
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	if ( m_phase != phaseImplementation )
	{
		PassExpression( cstrm ) ;
		return	errSuccess ;
	}
	RSTypeInfo	typeTemp ;
	SError	err = CompileExpression( typeTemp, block, cstrm ) ;
	if ( err )
	{
		return	err ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		OutputError( L"throw 文に \';\' が見つかりません" ) ;
		return	errSuccess ;
	}
	for ( size_t i = 0; i < block.m_local.GetNestDepth(); i ++ )
	{
		LocalNest *	pNest = block.m_local.GetNestAt( i ) ;
		if ( pNest != NULL )
		{
			FreeLocalNestVariable( block, pNest ) ;
		}
	}
	RSInstruction *	pInstRet = new RSInstruction ;
	pInstRet->Return( typeTemp ) ;
	block.AddCode( pInstRet ) ;
	return	errSuccess ;
}

// with 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementWith
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSParenthesis *	pPrthWith =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthWith == NULL )
	{
		OutputError( L"with 文の構文エラーです" ) ;
		return	errSuccess ;
	}
	RSCodeStream	csWith( *pPrthWith ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( m_phase == phaseImplementation )
	{
		LocalNest *	pWithNest = block.m_local.DescendNest( nestWith ) ;
		//
		RSTypeInfo *	ptiWithExpr = new RSTypeInfo ;
		SError		err ;
		err = CompileExpression( *ptiWithExpr, block, csWith ) ;
		if ( err )
		{
			delete	ptiWithExpr ;
			//
			block.m_local.AscendNest() ;
			delete	pWithNest ;
			return	errSuccess ;
		}
		SString	strTempName ;
		CreateNamelessVariableAs( block, strTempName, ptiWithExpr ) ;
		//
		CompileAllStatements( block, csCode, NULL ) ;
		//
		FreeLocalNestVariable( block, pWithNest ) ;
		ESLVerify( block.m_local.AscendNest() == pWithNest ) ;
		delete	pWithNest ;
		return	errSuccess ;
	}
	else
	{
		if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCode, NULL ) ;
		}
		return	errSuccess ;
	}
}

// synchronized 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementSynchronized
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSParenthesis *	pPrthSync =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthSync == NULL )
	{
		return	CompileStatementAccessModifier( block, cstrm, wiIndex ) ;
	}
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	RSCodeStream	csSync( *pPrthSync ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( m_phase == phaseImplementation )
	{
		LocalNest *	pSyncNest = block.m_local.DescendNest( nestSynchronized ) ;
		//
		RSTypeInfo *	ptiSyncExpr = new RSTypeInfo ;
		SError		err ;
		err = CompileExpression( *ptiSyncExpr, block, csSync ) ;
		if ( err )
		{
			delete	ptiSyncExpr ;
			//
			block.m_local.AscendNest() ;
			delete	pSyncNest ;
			return	errSuccess ;
		}
		SString	strTempName ;
		CreateNamelessVariableAs( block, strTempName, ptiSyncExpr ) ;
		//
		RSInstruction *	pInst = new RSInstruction ;
		pInst->Synchronize( *ptiSyncExpr ) ;
		block.AddCode( pInst ) ;
		//
		CompileAllStatements( block, csCode, NULL ) ;
		//
		FreeLocalNestVariable( block, pSyncNest ) ;
		ESLVerify( block.m_local.AscendNest() == pSyncNest ) ;
		delete	pSyncNest ;
		return	errSuccess ;
	}
	else
	{
		if ( m_phase == phaseAllocation )
		{
			CompileAllStatements( block, csCode, NULL ) ;
		}
		return	errSuccess ;
	}
}

// static|abstract|const|public|protected|private 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementAccessModifier
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	uint32_t	accMod = 0 ;
	do
	{
		switch ( wiIndex )
		{
		case	RSCodeControl::wiStatic:
			accMod |= RSObject::modifierStatic ;
			break ;
		case	RSCodeControl::wiAbstract:
			accMod |= RSObject::modifierAbstract ;
			break ;
		case	RSCodeControl::wiNative:
			accMod |= RSObject::modifierNative ;
			break ;
		case	RSCodeControl::wiConst:
			accMod |= RSObject::modifierConst ;
			break ;
		case	RSCodeControl::wiSynchronized:
			accMod |= RSObject::modifierSynchronized ;
			break ;
		case	RSCodeControl::wiPublic:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierPublic ;
			break ;
		case	RSCodeControl::wiProtected:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierProtected ;
			break ;
		case	RSCodeControl::wiPrivate:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierPrivate ;
			break ;
		case	RSCodeControl::wiVar:
			return	CompileDeclareVariable
				( block, cstrm, accMod,
					m_context.GetVariableClass(), declModeAny ) ;
		case	RSCodeControl::wiVoid:
			return	CompileDeclareVariable
				( block, cstrm, accMod, NULL, declModeAny ) ;
		case	RSCodeControl::wiExtern:
			accMod |= RSObject::modifierExtern ;
			break ;
		default:
			if ( (wiIndex >= RSCodeControl::wiFirstBasicType)
				&& (wiIndex <= RSCodeControl::wiLastBasicType) )
			{
				RSClass *	pClass = m_context.GetBasicTypeClass( wiIndex ) ;
				pClass = m_context.ParseClassArrayDecoration( cstrm, pClass ) ;
				TestContextException( m_context ) ;
				return	CompileDeclareVariable
					( block, cstrm, accMod, pClass, declModeAny ) ;
			}
			else
			{
				return	(this->*m_pfnCompileStatement[wiIndex])
										( block, cstrm, wiIndex ) ;
			}
		}
		RSCodeControl *	pCode = cstrm.NextControlWord() ;
		if ( pCode == NULL )
		{
			RSClass *	pClass = ParseClassExpression( cstrm ) ;
			if ( pClass != NULL )
			{
				return	CompileDeclareVariable
					( block, cstrm, accMod, pClass, declModeAny ) ;
			}
			else
			{
				OutputError( L"アクセス修飾子が無効です" ) ;
				return	errFailed ;
			}
		}
		wiIndex = pCode->m_word ;
	}
	while ( !cstrm.IsEndOfStream() ) ;
	return	errSuccess ;
}

// var|void|boolean|byte|short|char|int|long|float|double 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementVar
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	RSClass *	pClass = NULL ;
	if ( (wiIndex >= RSCodeControl::wiFirstBasicType)
				&& (wiIndex <= RSCodeControl::wiLastBasicType) )
	{
		pClass =
			m_context.ParseClassArrayDecoration
				( cstrm, m_context.GetBasicTypeClass( wiIndex ) ) ;
		TestContextException( m_context ) ;
	}
	else if ( wiIndex == RSCodeControl::wiVar )
	{
		pClass = m_context.GetVariableClass() ;
	}
	return	CompileDeclareVariable
		( block, cstrm, 0, pClass, declModeAny ) ;
}

// function|this|super 文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementExpression
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	ESLAssert( cstrm.GetIndex() > 0 ) ;
	cstrm.SeekIndex( cstrm.GetIndex() - 1 ) ;
	//
	RSTypeInfo	typeExpr ;
	SError	err = CompileExpression( typeExpr, block, cstrm ) ;
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	return	err ;
}

// 不正文
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::CompileStatementInvalid
	( RSInstructionParser::FunctionBlock& block,
		RSCodeStream& cstrm, RSCodeControl::WordIndex wiIndex )
{
	OutputError( L"構文エラーです" ) ;
	return	errSuccess ;
}

// エラー数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSInstructionParser::GetErrorCount( void ) const
{
	return	m_nError ;
}

size_t RSInstructionParser::GetWarningCount( void ) const
{
	return	m_nWarning ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::OutputError( const wchar_t * pwszErr )
{
	SString	strSrcPath ;
	SString	strSrcLine ;
	size_t	iSrcIndex, iSrcLine ;
	//
	GetCurrentPositionInfo( strSrcPath, strSrcLine, iSrcIndex, iSrcLine ) ;
	OnError( strSrcPath, strSrcLine, iSrcIndex, iSrcLine, pwszErr ) ;
	m_nError ++ ;
}

void RSInstructionParser::OutputErrorLog( SSystem::SParserErrorLogger& perrLog )
{
	for ( size_t i = 0; i < perrLog.GetErrorCount(); i ++ )
	{
		SParserErrorLogger::ErrorLog *	pLog = perrLog.GetErrorLogAt( i ) ;
		OnError
			( pLog->m_strFilePath, pLog->m_strLine,
				pLog->m_nLineNum, pLog->m_nColNum, pLog->m_strError ) ;
		m_nError ++ ;
	}
	for ( size_t i = 0; i < perrLog.GetWarningCount(); i ++ )
	{
		SParserErrorLogger::ErrorLog *	pLog = perrLog.GetWarningLogAt( i ) ;
		OnWarning
			( pLog->m_strFilePath, pLog->m_strLine,
				pLog->m_nLineNum, pLog->m_nColNum, pLog->m_strError ) ;
		m_nWarning ++ ;
	}
}

void RSInstructionParser::OnError
	( const SSystem::SString& strSrcPath,
		const SSystem::SString& strSrcLine,
		size_t iSrcLine, size_t iColIndex, const wchar_t * pwszErr )
{
}

// 警告出力
//////////////////////////////////////////////////////////////////////////////
void RSInstructionParser::OutputWarning( const wchar_t * pwszErr )
{
	SString	strSrcPath ;
	SString	strSrcLine ;
	size_t	iSrcIndex, iSrcLine ;
	//
	GetCurrentPositionInfo( strSrcPath, strSrcLine, iSrcIndex, iSrcLine ) ;
	OnWarning( strSrcPath, strSrcLine, iSrcIndex, iSrcLine, pwszErr ) ;
	m_nWarning ++ ;
}

void RSInstructionParser::OnWarning
	( const SSystem::SString& strSrcPath,
		const SSystem::SString& strSrcLine,
		size_t iSrcLine, size_t iColIndex, const wchar_t * pwszErr )
{
}

// ソース位置情報取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSInstructionParser::GetSourcePositionInfo
	( SSystem::SString& strSrcPath,
		SSystem::SString& strSrcLine,
		size_t& iSrcIndex, size_t& iSrcLine,
		const RSParenthesis * pParenthesis, size_t iSrcInChars )
{
	strSrcPath = L"" ;
	strSrcLine = L"" ;
	iSrcIndex = iSrcInChars ;
	iSrcLine = 0 ;
	//
	if ( pParenthesis == NULL )
	{
		return	errFailed ;
	}
	const RSParenthesis *	pSrcParenthesis = pParenthesis ;
	const RSScript *	pSrcScript = ESLTypeCast<RSScript>( pSrcParenthesis ) ;
	while ( (pSrcScript == NULL)
		&& (pSrcParenthesis->m_parent != NULL) )
	{
		pSrcParenthesis = pSrcParenthesis->m_parent ;
		pSrcScript = ESLTypeCast<RSScript>( pSrcParenthesis ) ;
	}
	if ( pSrcScript == NULL )
	{
		return	errFailed ;
	}
	strSrcPath = pSrcScript->GetSourcePath() ;
	//
	RSSourceParser	sparsSource ;
	sparsSource.AttachString( pSrcScript->GetSourceText() ) ;
	//
	size_t	iLineIndex ;
	iSrcLine = sparsSource.GetLineNumberOf( iSrcInChars, &iLineIndex ) ;
	sparsSource.SeekIndex( iLineIndex ) ;
	sparsSource.NextLine( strSrcLine ) ;
	strSrcLine.TrimRight() ;
	iSrcIndex -= iLineIndex ;
	//
	return	errSuccess ;
}

SSystem::SError RSInstructionParser::GetCurrentPositionInfo
	( SSystem::SString& strSrcPath,
		SSystem::SString& strSrcLine,
		size_t& iSrcIndex, size_t& iSrcLine ) const
{
	return	GetSourcePositionInfo
				( strSrcPath, strSrcLine, iSrcIndex, iSrcLine,
							m_pCurParenthesis, m_iSrcStatement ) ;
}


