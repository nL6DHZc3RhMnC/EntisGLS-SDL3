
#include <gls.h>
#include <glscs_rosetta.h>

using namespace SSystem ;
using namespace	Rosetta ;

//////////////////////////////////////////////////////////////////////////////
// 詞葉 → Rosetta インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCotophaObject, RSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSCotophaObject::RSCotophaObject
	( RSClass * pClass,
		ECSContext * pContext, ECSObject * pObject, bool flagOwner )
	: RSObject( pClass, typeObject )
{
	m_pContext = pContext ;
	m_pObject = pObject ;
	m_flagOwner = flagOwner ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSCotophaObject::~RSCotophaObject( void )
{
	if ( m_flagOwner )
	{
		m_pContext->delete_CSObject( m_pObject ) ;
		m_flagOwner = false ;
	}
}

// オブジェクト分離
//////////////////////////////////////////////////////////////////////////////
ECSObject * RSCotophaObject::DetachObject( void )
{
	m_flagOwner = false ;
	return	m_pObject ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSCotophaObject::GetTypeName( void ) const
{
	return	m_pObject->GetTypeName() ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::InstanceOf( const wchar_t * pwszType )
{
	ECSObject *	pObj = m_pObject->GetTypeOf( pwszType ) ;
	if ( pObj == m_pObject )
	{
		return	this ;
	}
	return	NULL ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::IsIntegerType( void ) const
{
	return	(m_pObject->m_vtType == csvtInteger) ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::IsFloatType( void ) const
{
	return	(m_pObject->m_vtType == csvtReal) ;
}

// 文字列型か？
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::IsStringType( void ) const
{
	return	(m_pObject->m_vtType == csvtString) ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::IsObjectType( void ) const
{
	return	(m_pObject->m_vtType != csvtInteger)
			&& (m_pObject->m_vtType != csvtReal) ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::AsInteger( int64_t& number ) const
{
	return	!m_pObject->OperateInteger( number ) ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::AsRealNumber( double& number ) const
{
	return	!m_pObject->OperateReal( number ) ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::AsBoolean( void ) const
{
	int	nBoolean ;
	if ( m_pObject->OperateBoolean( nBoolean ) )
	{
		return	false ;
	}
	return	(nBoolean != 0) ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::AsString( SSystem::SString& strValue ) const
{
	EWideString	wstrValue ;
	if ( m_pObject->OperateString( wstrValue ) )
	{
		return	false ;
	}
	strValue = wstrValue ;
	return	true ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSCotophaObject::IsEqualObject( RSObject * pObj ) const
{
	RSCotophaObject *	pcoObj = ESLTypeCast<RSCotophaObject>( pObj ) ;
	if ( pcoObj == NULL )
	{
		return	false ;
	}
	int			nResult = 0 ;
	ESLError	err =
		m_pObject->Compare
			( *m_pContext, nResult, csctEqual, *(pcoObj->m_pObject) ) ;
	if ( err )
	{
		return	false ;
	}
	return	(nResult != 0) ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSCotophaObject::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	EStreamBuffer	buf ;
	m_pObject->DumpObject( buf, (int) nPtrNest, *m_pContext ) ;
	//
	EPtrBuffer	ptrbuf = buf.GetBuffer() ;
	SString	strDump( (const char*) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
	dump.WriteEncodedString( strDump ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::CloneObject( RSContext& context ) const
{
	return	new RSCotophaObject
		( m_pClass, m_pContext,
			m_pContext->new_CSReference(ECSObject::GetEntity(m_pObject)) ) ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::GetElementAt
	( RSContext& context, int nIndex ) const
{
	int	iElement ;
	if ( m_pObject->GetVariableIndex( iElement, nIndex ) )
	{
		return	NULL ;
	}
	ECSObject *	pObj = m_pObject->GetVariableAt( iElement ) ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	ECSReference *	pRef =
		m_pContext->new_CSReference( ECSObject::GetEntity(pObj) ) ;
	return	new RSCotophaObject( m_pClass, m_pContext, pRef ) ;
}

// 要素名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSCotophaObject::GetElementNameAt( int nIndex ) const
{
	int	iElement ;
	if ( m_pObject->GetVariableIndex( iElement, nIndex ) )
	{
		return	NULL ;
	}
	ECSHash *	pHash = ESLTypeCast<ECSHash>( m_pObject ) ;
	if ( pHash != NULL )
	{
		ECSWideString *	pwstrTag =
			pHash->m_varArray.GetTagAt( (unsigned int) iElement ) ;
		if ( pwstrTag != NULL )
		{
			return	*pwstrTag ;
		}
		return	NULL ;
	}
	ECSGlobal *	pGlobal = ESLTypeCast<ECSGlobal>( m_pObject ) ;
	if ( pGlobal != NULL )
	{
		return	pGlobal->m_staObjName.GetAt( (unsigned int) iElement ) ;
	}
	ECSStructure *	pStruct = ESLTypeCast<ECSStructure>( m_pObject ) ;
	if ( pStruct != NULL )
	{
		return	pStruct->m_staMember.GetAt( (unsigned int) iElement ) ;
	}
	return	NULL ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::SetElementAt
	( RSContext& context, int nIndex, RSObject * pObj )
{
	RSCotophaObject *	pcoObj = ESLTypeCast<RSCotophaObject>( pObj ) ;
	if ( pcoObj == NULL )
	{
		context.ThrowExceptionError( L"配列要素に代入できません" ) ;
		return	NULL ;
	}
	int	iElement ;
	if ( m_pObject->GetVariableIndex( iElement, nIndex ) )
	{
		return	NULL ;
	}
	ECSObject *	pElement = m_pObject->GetVariableAt( iElement ) ;
	if ( pElement == NULL )
	{
		return	NULL ;
	}
	ECSReference *
		pRef = m_pContext->new_CSReference
					( ECSObject::GetEntity( pcoObj->m_pObject ) ) ;
	ESLError	err = pElement->Move( *m_pContext, pRef ) ;
	if ( err )
	{
		return	NULL ;
	}
	pRef = m_pContext->new_CSReference( ECSObject::GetEntity(pElement) ) ;
	return	new RSCotophaObject( m_pClass, m_pContext, pRef ) ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSCotophaObject::GetElementCount( void ) const
{
	INT64	nSize ;
	if ( m_pObject->OperateSizeOf( nSize ) )
	{
		return	0 ;
	}
	return	(size_t) nSize ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSCotophaObject::GetElementLimit( void ) const
{
	ECSArray *	pArray = ESLTypeCast<ECSArray>( m_pObject ) ;
	if ( pArray != NULL )
	{
		return	pArray->m_nBounds ;
	}
	return	0x7FFFFFFF ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	int	iElement ;
	if ( m_pObject->GetVariableIndex( iElement, pwszName ) )
	{
		if ( m_pObject->GetFunction( *m_pContext, iElement, pwszName ) )
		{
			return	NULL ;
		}
		RSCotophaFuncObject *	pFuncObj =
			new RSCotophaFuncObject
				( context.GetFunctionClass(),
					context, m_pContext,
					ECSObject::GetEntity( m_pObject ),
					iElement, pwszName ) ;
		return	pFuncObj ;
	}
	ECSObject *	pObj = m_pObject->GetVariableAt( iElement ) ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	ECSReference *	pRef =
		m_pContext->new_CSReference( ECSObject::GetEntity(pObj) ) ;
	return	new RSCotophaObject( m_pClass, m_pContext, pRef ) ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	RSCotophaObject *	pcoObj = ESLTypeCast<RSCotophaObject>( pObj ) ;
	if ( pcoObj == NULL )
	{
		context.ThrowExceptionError( L"配列要素に代入できません" ) ;
		return	NULL ;
	}
	int	iElement ;
	if ( m_pObject->GetVariableIndex( iElement, pwszName ) )
	{
		return	NULL ;
	}
	ECSObject *	pElement = m_pObject->GetVariableAt( iElement ) ;
	if ( pElement == NULL )
	{
		return	NULL ;
	}
	ECSReference *
		pRef = m_pContext->new_CSReference
						( ECSObject::GetEntity(pcoObj->m_pObject) ) ;
	ESLError	err = pElement->Move( *m_pContext, pRef ) ;
	if ( err )
	{
		return	NULL ;
	}
	pRef = m_pContext->new_CSReference( ECSObject::GetEntity(pElement) ) ;
	return	new RSCotophaObject( m_pClass, m_pContext, pRef ) ;
}

// メンバ新規作成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::CreateMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	context.ThrowExceptionError( L"メンバの作成は禁止されています" ) ;
	return	NULL ;
}

// メンバ削除
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::RemoveMemberAs
	( RSContext& context, const wchar_t * pwszName )
{
	context.ThrowExceptionError( L"メンバの削除は禁止されています" ) ;
	return	NULL ;
}

// メソッド取得
//////////////////////////////////////////////////////////////////////////////
RSObject::METHOD_ENTRY * RSCotophaObject::GetMethodAs
	( RSContext& context,
		const wchar_t * pwszName, METHOD_ENTRY& method ) const
{
	int	nIndex ;
	if ( m_pObject->GetFunction( *m_pContext, nIndex, pwszName ) )
	{
		return	NULL ;
	}
	method.pfnMethod = &RSCotophaObject::proc_CallFunction ;
	method.pInstance = (void*) nIndex ;
	return	&method ;
}

RSObject * RSCotophaObject::proc_CallFunction
	( RSContext& context, void * pInstance,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSCotophaObject *	pThisObj =
		ESLTypeCast<RSCotophaObject>( pThis->GetEntityObject() ) ;
	if ( pThisObj == NULL )
	{
		context.ThrowExceptionError( L"this オブジェクトが不正です" ) ;
		return	NULL ;
	}
	ECSContext *	pContext = pThisObj->m_pContext ;
	ECSObjArray<ECSObject>	lstArg ;
	for ( size_t i = 0; i < count; i ++ )
	{
		RSObject *	pArg = ppArg[i] ;
		if ( pArg != NULL )
		{
			pArg = pArg->GetEntityObject() ;
		}
		ECSObject *	pcsObj = pThisObj->CotophaFromRosettaObject( pArg ) ;
		if ( pcsObj == NULL )
		{
			context.ThrowExceptionError( L"関数引数が不正です" ) ;
			return	NULL ;
		}
		lstArg.Add( pcsObj ) ;
	}
	ESLError	err =
		pThisObj->m_pObject->CallFunction
				( *pContext, (int) pInstance, lstArg ) ;
	if ( err )
	{
		context.ThrowExceptionError( SString( GetESLErrorMsg(err) ) ) ;
		return	NULL ;
	}
	ECSObject *	pRetObj = pContext->PopObject() ;
	return	new RSCotophaObject( pThisObj->m_pClass, pContext, pRetObj ) ;
}

// Rosetta -> 詞葉オブジェクト変換
//////////////////////////////////////////////////////////////////////////////
ECSObject * RSCotophaObject::CotophaFromRosettaObject( RSObject * pObj ) const
{
	if ( pObj == NULL )
	{
		return	m_pContext->new_CSReference() ;
	}
	RSCotophaObject *
			pcoObj = ESLTypeCast<RSCotophaObject>( pObj ) ;
	if ( pcoObj != NULL )
	{
		return	m_pContext->new_CSReference( pcoObj->m_pObject ) ;
	}
	switch ( pObj->GetBasicType() )
	{
	case	RSObject::typeNumber:
		{
			double	num ;
			if ( pObj->AsRealNumber( num ) )
			{
				return	m_pContext->new_CSReal( num ) ;
			}
			else
			{
				return	NULL ;
			}
		}
		break ;
	case	RSObject::typeInteger:
		{
			int64_t	num ;
			if ( pObj->AsInteger( num ) )
			{
				return	m_pContext->new_CSInteger( num ) ;
			}
			else
			{
				return	NULL ;
			}
		}
		break ;
	case	RSObject::typeBoolean:
		return	m_pContext->new_CSInteger( pObj->AsBoolean() ? -1 : 0 ) ;
		break ;
	case	RSObject::typeString:
		{
			SString	str ;
			if ( pObj->AsString( str ) )
			{
				return	m_pContext->new_CSString( str ) ;
			}
			else
			{
				return	NULL ;
			}
		}
		break ;
	}
	return	NULL ;
}

// 詞葉 -> Rosetta 型変換
//////////////////////////////////////////////////////////////////////////////
RSFunctionPrototype * RSCotophaObject::RosettaFromCotophaProto
	( RSContext& context, const ECSPrototypeInfo& protoCotopha )
{
	RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
	pProto->m_pReturnType =
		RosettaFromCotophaType( context, protoCotopha.GetReturnType() ) ;

	for ( unsigned int i = 0; i < protoCotopha.GetArgumentCount(); i ++ )
	{
		ECSTypeInfo *	pArgType = protoCotopha.GetArgumentAt( i ) ;
		const wchar_t *	pwszArgName = protoCotopha.GetArgumentNameAt( i ) ;
		if ( pArgType && pwszArgName )
		{
			RSClass *	pArgClass = RosettaFromCotophaType( context, *pArgType ) ;
			if ( pArgClass == nullptr )
			{
				pArgClass = context.GetGenericObjectClass() ;
			}
			pProto->m_aArgNames.Add( new SString( pwszArgName ) ) ;
			pProto->m_aArgTypes.Add( pArgClass ) ;
		}
	}
	return	pProto ;
}

RSClass * RSCotophaObject::RosettaFromCotophaType
	( RSContext& context, const ECSTypeInfo& typeCotopha )
{
	if ( !typeCotopha.IsPureType() )
	{
		return	nullptr ;
	}
	if ( typeCotopha.IsTypeBoolean() )
	{
		return	context.GetBooleanClass() ;
	}
	if ( typeCotopha.IsTypeInteger() )
	{
		return	context.GetIntegerClass() ;
	}
	if ( typeCotopha.IsTypeReal() )
	{
		return	context.GetNumberClass() ;
	}
	if ( typeCotopha.IsTypeString() )
	{
		return	context.GetStringClass() ;
	}
	return	nullptr ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::UnaryOperator
	( RSContext& context, CSUnaryOperatorType uotType ) const
{
	ECSObject *	pObj = m_pObject->Duplicate() ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	ESLError	err = pObj->UnaryOperate( *m_pContext, uotType ) ;
	if ( err )
	{
		context.ThrowExceptionError( SString( GetESLErrorMsg(err) ) ) ;
		return	NULL ;
	}
	return	new RSCotophaObject( m_pClass, m_pContext, pObj ) ;
}

RSObject * RSCotophaObject::OperatorPlus( RSContext& context ) const
{
	return	UnaryOperator( context, csuotPlus ) ;
}

RSObject * RSCotophaObject::OperatorNegate( RSContext& context ) const
{
	return	UnaryOperator( context, csuotNegate ) ;
}

RSObject * RSCotophaObject::OperatorBitNot( RSContext& context ) const
{
	return	UnaryOperator( context, csuotBitNot ) ;
}

RSObject * RSCotophaObject::OperatorIncrement( RSContext& context )
{
	ESLError	err = m_pObject->UnaryOperate( *m_pContext, csuotIncrement ) ;
	if ( err )
	{
		context.ThrowExceptionError( SString( GetESLErrorMsg(err) ) ) ;
		return	NULL ;
	}
	AddRef() ;
	return	this ;
}

RSObject * RSCotophaObject::OperatorDecrement( RSContext& context )
{
	ESLError	err = m_pObject->UnaryOperate( *m_pContext, csuotDecrement ) ;
	if ( err )
	{
		context.ThrowExceptionError( SString( GetESLErrorMsg(err) ) ) ;
		return	NULL ;
	}
	AddRef() ;
	return	this ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::BinaryOperator
	( RSContext& context, CSOperatorType opType, RSObject * pObj ) const
{
	ECSObject *	pcsObj = CotophaFromRosettaObject( pObj ) ;
	if ( pcsObj == NULL )
	{
		context.ThrowExceptionError( L"変換できないオブジェクトです" ) ;
		return	NULL ;
	}
	ECSObject *	pObjThis = m_pObject->Duplicate() ;
	if ( pObjThis == NULL )
	{
		return	NULL ;
	}
	ESLError	err = pObjThis->Operate( *m_pContext, opType, pcsObj ) ;
	if ( err )
	{
		m_pContext->delete_CSObject( pcsObj ) ;
		context.ThrowExceptionError( SString( GetESLErrorMsg(err) ) ) ;
		return	NULL ;
	}
	return	new RSCotophaObject( m_pClass, m_pContext, pObjThis ) ;
}

RSObject * RSCotophaObject::BinaryComparator
	( RSContext& context, CSCompareType cpType, RSObject * pObj ) const
{
	ECSObject *	pObjThis = m_pObject->Duplicate() ;
	if ( pObjThis == NULL )
	{
		return	NULL ;
	}
	ECSObject *	pcsObj = CotophaFromRosettaObject( pObj ) ;
	if ( pcsObj == NULL )
	{
		delete	pObjThis ;
		context.ThrowExceptionError( L"変換できないオブジェクトです" ) ;
		return	NULL ;
	}
	int			nResult ;
	ESLError	err = pObjThis->Compare
					( *m_pContext, nResult, cpType, *pcsObj ) ;
	m_pContext->delete_CSObject( pcsObj ) ;
	if ( err )
	{
		context.ThrowExceptionError( SString( GetESLErrorMsg(err) ) ) ;
		return	NULL ;
	}
	return	context.new_Boolean( (nResult != 0) ) ;
}

RSObject * RSCotophaObject::OperatorMul( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotMul, pObj ) ;
}

RSObject * RSCotophaObject::OperatorDiv( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotDiv, pObj ) ;
}

RSObject * RSCotophaObject::OperatorMod( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotMod, pObj ) ;
}

RSObject * RSCotophaObject::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotAdd, pObj ) ;
}

RSObject * RSCotophaObject::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotSub, pObj ) ;
}

RSObject * RSCotophaObject::OperatorShiftLeft( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotShiftLeft, pObj ) ;
}

RSObject * RSCotophaObject::OperatorShiftRight( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotShiftRight, pObj ) ;
}

RSObject * RSCotophaObject::OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotShiftRight, pObj ) ;
}

RSObject * RSCotophaObject::OperatorBitAnd( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotAnd, pObj ) ;
}

RSObject * RSCotophaObject::OperatorBitOr( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotOr, pObj ) ;
}

RSObject * RSCotophaObject::OperatorBitXor( RSContext& context, RSObject * pObj ) const
{
	return	BinaryOperator( context, csotXor, pObj ) ;
}

RSObject * RSCotophaObject::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	return	BinaryComparator( context, csctEqual, pObj ) ;
}

RSObject * RSCotophaObject::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	return	BinaryComparator( context, csctNotEqual, pObj ) ;
}

RSObject * RSCotophaObject::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	return	BinaryComparator( context, csctGreaterEqual, pObj ) ;
}

RSObject * RSCotophaObject::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	return	BinaryComparator( context, csctGreaterThan, pObj ) ;
}

RSObject * RSCotophaObject::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	return	BinaryComparator( context, csctLessEqual, pObj ) ;
}

RSObject * RSCotophaObject::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	return	BinaryComparator( context, csctLessThan, pObj ) ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCotophaObject::OperatorMove( RSContext& context, RSObject * pObj )
{
	ECSObject *	pcsObj = CotophaFromRosettaObject( pObj ) ;
	if ( pcsObj == NULL )
	{
		context.ThrowExceptionError( L"変換できないオブジェクトです" ) ;
		return	NULL ;
	}
	ESLError	err = m_pObject->Move( *m_pContext, pcsObj ) ;
	if ( err )
	{
		m_pContext->delete_CSObject( pcsObj ) ;
		context.ThrowExceptionError( SString( GetESLErrorMsg(err) ) ) ;
		return	NULL ;
	}
	AddRef() ;
	return	this ;
}


//////////////////////////////////////////////////////////////////////////////
// 詞葉 → Rosetta 関数インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCotophaFuncObject, RSFunctionObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSCotophaFuncObject::RSCotophaFuncObject
	( RSClass * pFuncClass,
		RSContext& context,
		ECSContext * pContext,
		ECSObject * pObject,
		int nFuncIndex,
		const wchar_t * pwszFuncName )
	: RSFunctionObject( pFuncClass ),
		m_pContext( pContext ),
	m_pObject( pObject ), m_nFuncIndex( -1 )
{
	const ECSClassInfo*	pClassInf = nullptr ;
	if ( pObject != nullptr )
	{
		pClassInf = pObject->m_pClassInf ;
		if ( pClassInf != nullptr )
		{
			m_nFuncIndex = pClassInf->FindFunctionAs( pwszFuncName ) ;
		}
	}

	m_strFuncName = pwszFuncName ;

	if ( pClassInf != nullptr )
	{
		ECSClassInfo::MemberFunction *
			pMemberFunc = pClassInf->GetFunctionAt( (unsigned int) m_nFuncIndex ) ;
		if ( pMemberFunc != nullptr )
		{
			RSFunctionPrototype *	pProto =
				RSCotophaObject::RosettaFromCotophaProto( context, *pMemberFunc ) ;
			if ( pProto != nullptr )
			{
				m_pPrototype = pProto ;
				m_arrPrototypes.Add( pProto ) ;
				//
				pProto->m_methodNative.pfnMethod = &RSCotophaObject::proc_CallFunction ;
				pProto->m_methodNative.pInstance = (void*) nFuncIndex ;
			}
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSCotophaFuncObject::~RSCotophaFuncObject( void )
{
}


