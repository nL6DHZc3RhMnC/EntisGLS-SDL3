
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// オブジェクト参照
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSReference, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSReference::~RSReference( void )
{
	RSObject::ReleaseRef( m_pRef ) ;
	m_pRef = NULL ;
}

// 参照設定
//////////////////////////////////////////////////////////////////////////////
void RSReference::SetReference( RSObject * pRef )
{
	RSObject::ReleaseRef( m_pRef ) ;
	m_pRef = pRef ;
}

// 参照解除
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::DetachReference( void )
{
	RSObject *	pObj = m_pRef ;
	m_pRef = NULL ;
	return	pObj ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSReference::GetTypeName( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->GetTypeName() : NULL ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::InstanceOf( const wchar_t * pwszType )
{
	return	(m_pRef != NULL) ? m_pRef->InstanceOf(pwszType) : NULL ;
}

RSObject * RSReference::InstanceOf( RSClass * pClass )
{
	return	(m_pRef != NULL) ? m_pRef->InstanceOf(pClass) : NULL ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReference::IsIntegerType( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->IsIntegerType() : false ;
}

// 浮動小数点型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReference::IsFloatType( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->IsFloatType() : false ;
}

// 文字列型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReference::IsStringType( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->IsStringType() : false ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSReference::IsObjectType( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->IsObjectType() : false ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSReference::AsInteger( int64_t& number ) const
{
	return	(m_pRef != NULL) ? m_pRef->AsInteger(number) : false ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSReference::AsRealNumber( double& number ) const
{
	return	(m_pRef != NULL) ? m_pRef->AsRealNumber(number) : false ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSReference::AsBoolean( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->AsBoolean() : false ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSReference::AsString( SSystem::SString& strValue ) const
{
	if ( m_pRef != NULL )
	{
		return	m_pRef->AsString(strValue) ;
	}
	strValue = L"undefined" ;
	return	true ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSReference::IsEqualObject( RSObject * pObj ) const
{
	if ( this == pObj )
	{
		return	true ;
	}
	if ( pObj != NULL )
	{
		pObj = pObj->GetEntityObject() ;
	}
	if ( pObj == NULL )
	{
		return	(GetEntityObject() == pObj) ;
	}
	if ( m_pRef == NULL )
	{
		return	false ;
	}
	return	m_pRef->IsEqualObject( pObj ) ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSReference::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	dump.WriteEncodedString( L"--> " ) ;
	//
	if ( m_pRef == NULL )
	{
		dump.WriteEncodedString( L"null" ) ;
	}
	else
	{
		SString	strDump = m_pRef->GetTypeName() ;
		strDump += L" (#" ;
		if ( sizeof(m_pRef) > 4 )
		{
			strDump += SString( (int64_t) m_pRef, 16, 16 ) ;
		}
		else
		{
			strDump += SString( (uint32_t) ((ulong_ptr_t) m_pRef), 8, 16 ) ;
		}
		strDump += L") : " ;
		dump.WriteEncodedString( strDump ) ;
		//
		if ( nPtrNest > 0 )
		{
			m_pRef->ToDebugDump( dump, nPtrNest - 1, pwszIndent ) ;
		}
	}
}

// 値設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSReference::SetIntegerAs( int64_t nValue )
{
	if (m_pRef != NULL )
	{
		return	m_pRef->SetIntegerAs( nValue );
	}
	return	RSObject::SetIntegerAs( nValue ) ;
}

SSystem::SError RSReference::SetNumberAs( double nValue )
{
	if (m_pRef != NULL )
	{
		return	m_pRef->SetNumberAs( nValue );
	}
	return	RSObject::SetNumberAs( nValue ) ;
}

SSystem::SError RSReference::SetStringAs( const wchar_t * pwszValue )
{
	if (m_pRef != NULL )
	{
		return	m_pRef->SetStringAs( pwszValue );
	}
	return	RSObject::SetStringAs( pwszValue ) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSReference::DisposeObject( RSContext& context )
{
	context.ReleaseObjectRef( m_pRef ) ;
	m_pRef = NULL ;
	RSObject::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::DuplicateObject( RSContext& context ) const
{
	return	(m_pRef != NULL) ? m_pRef->DuplicateObject(context) : NULL ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::CloneObject( RSContext& context ) const
{
	return	(m_pRef != NULL) ? m_pRef->CloneObject(context) : NULL ;
}

// 実体
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::GetEntityObject( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->GetEntityObject() : NULL ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::GetElementAt( RSContext& context, int nIndex ) const
{
	return	(m_pRef != NULL) ? m_pRef->GetElementAt(context,nIndex) : NULL ;
}

// 要素名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSReference::GetElementNameAt( int nIndex ) const
{
	return	(m_pRef != NULL) ? m_pRef->GetElementNameAt(nIndex) : NULL ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::SetElementAt
	( RSContext& context, int nIndex, RSObject * pObj )
{
	return	(m_pRef != NULL) ?
				m_pRef->SetElementAt(context,nIndex,pObj) : NULL ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSReference::GetElementCount( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->GetElementCount() : 0 ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSReference::GetElementLimit( void ) const
{
	return	(m_pRef != NULL) ? m_pRef->GetElementLimit() : 0 ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	if ( m_pRef == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObj = m_pRef->GetMemberAs( context, pwszName ) ;
	if ( (pObj == NULL)
		|| !(m_accModifier & modifierConst) )
	{
		return	pObj ;
	}
	uint32_t	accMod = pObj->GetModifiers() & accessMask ;
	pObj = context.new_Reference( pObj ) ;
	pObj->SetModifiers( accMod | modifierConst ) ;
	return	pObj ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	if ( m_pRef == NULL )
	{
		return	NULL ;
	}
	pObj = m_pRef->SetMemberAs( context, pwszName, pObj ) ;
	if ( (pObj == NULL)
		|| !(m_accModifier & modifierConst) )
	{
		return	pObj ;
	}
	uint32_t	accMod = pObj->GetModifiers() & accessMask ;
	pObj = context.new_Reference( pObj ) ;
	pObj->SetModifiers( accMod | modifierConst ) ;
	return	pObj ;
}

// メンバ新規作成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::CreateMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	return	(m_pRef != NULL) ?
				m_pRef->CreateMemberAs(context,pwszName,pObj) : NULL ;
}

// メソッド取得
//////////////////////////////////////////////////////////////////////////////
RSObject::METHOD_ENTRY *
	RSReference::GetMethodAs
		( RSContext& context,
			const wchar_t * pwszName, RSObject::METHOD_ENTRY& method ) const
{
	return	(m_pRef != NULL)
				? m_pRef->GetMethodAs(context, pwszName, method) : NULL ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::OperatorPlus( RSContext& context ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorPlus(context) : NULL ;
}

RSObject * RSReference::OperatorNegate( RSContext& context ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorNegate(context) : NULL ;
}

RSObject * RSReference::OperatorBitNot( RSContext& context ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorBitNot(context) : NULL ;
}

RSObject * RSReference::OperatorIncrement( RSContext& context )
{
	return	(m_pRef != NULL) ? m_pRef->OperatorIncrement(context) : NULL ;
}

RSObject * RSReference::OperatorDecrement( RSContext& context )
{
	return	(m_pRef != NULL) ? m_pRef->OperatorDecrement(context) : NULL ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::OperatorMul( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorMul(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorDiv( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorDiv(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorMod( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorMod(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorAdd(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorSub( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorSub(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorShiftLeft( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorShiftLeft(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorShiftRight( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorShiftRight(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorBitShiftRight(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorBitAnd( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorBitAnd(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorBitOr( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorBitOr(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorBitXor( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorBitXor(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorCompareEQ(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorCompareNE(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorCompareGE(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorCompareGT(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorCompareLE(context,pObj) : NULL ;
}

RSObject * RSReference::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	return	(m_pRef != NULL) ? m_pRef->OperatorCompareLT(context,pObj) : NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::OperatorMove( RSContext& context, RSObject * pObj )
{
	return	(m_pRef != NULL) ? m_pRef->OperatorMove(context,pObj) : NULL ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReference::SerializeObject( RSContext& context )
{
	return	CloneObject( context ) ;
}

SSystem::SError RSReference::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	SaveObjectBinary( m_pRef, context, file ) ;
}

SSystem::SError RSReference::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	SXMLDocument *	pxmlRef = new SXMLDocument ;
	SError	err = MakeXMLDocumentOfObject( m_pRef, context, *pxmlRef ) ;
	xmlDoc.AddElement( pxmlRef ) ;
	xmlDoc.SetTag( L"Reference" ) ;
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSReference::RestoreObject
		( RSContext& context, RSObject * pObj )
{
	context.ReleaseObjectRef( DetachReference() ) ;
	SetReference( pObj->CloneObject( context ) ) ;
	return	errSuccess ;
}

SSystem::SError RSReference::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	RSObject *	pObj = RSObject::LoadObjectBinary( context, file ) ;
	SetReference( pObj ) ;
	return	errSuccess ;
}

SSystem::SError RSReference::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	SXMLDocument *	pxmlRef = xmlDoc.GetElementAt( 0 ) ;
	RSObject *		pObj = NULL ;
	if ( pxmlRef != NULL )
	{
		pObj = RSObject::RestoreObjectOfXMLDocument( context, *pxmlRef ) ;
	}
	SetReference( pObj ) ;
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// オブジェクト参照（ポインタ）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSPointer, RSReference )

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSPointer::AsBoolean( void ) const
{
	if ( m_pRef != NULL )
	{
		return	m_pRef->AsBoolean() ;
	}
	return	false ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSPointer::AsString( SSystem::SString& strValue ) const
{
	if ( m_pRef != NULL )
	{
		return	m_pRef->AsString(strValue) ;
	}
	strValue = L"null" ;
	return	true ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPointer::DuplicateObject( RSContext& context ) const
{
	RSObject::AddRef( m_pRef ) ;
	return	context.new_Pointer( m_pRef, m_pPtrClass ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPointer::CloneObject( RSContext& context ) const
{
	RSObject *	pRef = NULL ;
	if ( m_pRef != NULL )
	{
		pRef = m_pRef->CloneObject( context ) ;
	}
	return	context.new_Pointer( pRef, m_pPtrClass ) ;
}

// 実体型
//////////////////////////////////////////////////////////////////////////////
RSClass * RSPointer::GetEntityClass( void ) const
{
	RSClass *	pClass = RSReference::GetEntityClass() ;
	if ( pClass == NULL )
	{
		pClass = m_pPtrClass ;
	}
	return	pClass ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPointer::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	if ( m_pRef != NULL )
	{
		return	context.new_Boolean( m_pRef->IsEqualObject( pObj ) ) ;
	}
	if ( (pObj == NULL) || (pObj->GetEntityObject() == NULL) )
	{
		return	context.new_Boolean( true ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSPointer::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	if ( m_pRef != NULL )
	{
		return	context.new_Boolean( !m_pRef->IsEqualObject( pObj ) ) ;
	}
	if ( (pObj == NULL) || (pObj->GetEntityObject() == NULL) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSPointer::OperatorMove( RSContext& context, RSObject * pObj )
{
	if ( (m_pPtrClass != NULL) && (pObj != NULL) )
	{
		RSObject *	pEntity = pObj->GetEntityObject() ;
		pObj = pObj->InstanceOf( m_pPtrClass ) ;
		if ( pObj == NULL )
		{
			if ( pEntity != NULL )
			{
				RSClass *	pSrcClass = pEntity->GetRSClass() ;
				if ( pSrcClass != NULL )
				{
					context.ThrowExceptionError
						( SString(pSrcClass->GetRSClassName())
							+ L" から "
							+ SString(m_pPtrClass->GetRSClassName())
							+ L" へ型変換できません" ) ;
				}
				else
				{
					context.ThrowExceptionError
						( SString(m_pPtrClass->GetRSClassName())
										+ L" へ型変換できません" ) ;
				}
				return	NULL ;
			}
		}
	}
	RSObject::AddRef( pObj ) ;
	context.ReleaseObjectRef( m_pRef ) ;
	m_pRef = pObj ;
	AddRef() ;
	return	this ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSPointer::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	SError	err = RSReference::MakeXMLDocument( context, xmlDoc ) ;
	if ( m_pPtrClass != NULL )
	{
		xmlDoc.SetAttributeAs( L"type", m_pPtrClass->GetRSClassName() ) ;
	}
	xmlDoc.SetTag( L"Pointer" ) ;
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSPointer::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	m_pPtrClass = context.GetClassAs( xmlDoc.GetAttrStringAs( L"type" ) ) ;
	return	RSReference::RestoreXMLDocument( context, xmlDoc ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 配列要素参照
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSReferenceElement, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSReferenceElement::~RSReferenceElement( void )
{
	RSObject::ReleaseRef( m_pRefParent ) ;
	m_pRefParent = NULL ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceElement::AsBoolean( void ) const
{
	return	false ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceElement::AsString( SSystem::SString& strValue ) const
{
	strValue = L"undefined" ;
	return	true ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceElement::DuplicateObject( RSContext& context ) const
{
	RSObject::AddRef( m_pRefParent ) ;
	return	new RSReferenceElement( m_pRefParent, m_nRefIndex, m_pClass ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceElement::CloneObject( RSContext& context ) const
{
	RSObject::AddRef( m_pRefParent ) ;
	return	new RSReferenceElement( m_pRefParent, m_nRefIndex, m_pClass ) ;
}

// 実体
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceElement::GetEntityObject( void ) const
{
	return	NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceElement::OperatorMove( RSContext& context, RSObject * pObj )
{
	RSObject *	pResult = NULL ;
	if ( m_pRefParent != NULL )
	{
		RSObject::AddRef( pObj ) ;
		pResult = m_pRefParent->SetElementAt( context, m_nRefIndex, pObj ) ;
		RSObject::ReleaseRef( pResult ) ;
	}
	else
	{
		context.ThrowExceptionError
			( L"null 要素への代入を実行しようとしています" ) ;
	}
	return	pResult ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceElement::OperatorCompareEQ
					( RSContext& context, RSObject * pObj ) const
{
	return	context.new_Boolean
				( (pObj == NULL) || (pObj->GetEntityObject() == NULL) ) ;
}

RSObject * RSReferenceElement::OperatorCompareNE
					( RSContext& context, RSObject * pObj ) const
{
	return	context.new_Boolean
				( (pObj != NULL) && (pObj->GetEntityObject() != NULL) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// メンバ参照
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSReferenceMember, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSReferenceMember::~RSReferenceMember( void )
{
	RSObject::ReleaseRef( m_pRefParent ) ;
	m_pRefParent = NULL ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceMember::AsBoolean( void ) const
{
	return	false ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSReferenceMember::AsString( SSystem::SString& strValue ) const
{
	strValue = L"undefined" ;
	return	true ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceMember::DuplicateObject( RSContext& context ) const
{
	RSObject::AddRef( m_pRefParent ) ;
	return	new RSReferenceMember( m_pRefParent, m_strRefMember, m_pClass ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceMember::CloneObject( RSContext& context ) const
{
	RSObject::AddRef( m_pRefParent ) ;
	return	new RSReferenceMember( m_pRefParent, m_strRefMember, m_pClass ) ;
}

// 実体
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceMember::GetEntityObject( void ) const
{
	return	NULL ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceMember::OperatorMove( RSContext& context, RSObject * pObj )
{
	RSObject *	pResult = NULL ;
	if ( m_pRefParent != NULL )
	{
		RSObject::AddRef( pObj ) ;
		pResult = m_pRefParent->SetMemberAs( context, m_strRefMember, pObj ) ;
	}
	else
	{
		context.ThrowExceptionError
			( L"null メンバへの代入を実行しようとしています" ) ;
	}
	return	pResult ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSReferenceMember::OperatorCompareEQ
					( RSContext& context, RSObject * pObj ) const
{
	return	context.new_Boolean
				( (pObj == NULL) || (pObj->GetEntityObject() == NULL) ) ;
}

RSObject * RSReferenceMember::OperatorCompareNE
					( RSContext& context, RSObject * pObj ) const
{
	return	context.new_Boolean
				( (pObj != NULL) && (pObj->GetEntityObject() != NULL) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ネイティブオブジェクトコンテナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSNativeObject, RSGenericObject )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSNativeObject::ObjectListener, ESLObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSNativeObject::~RSNativeObject( void )
{
	ReleaseNativeRef() ;
}

// オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SObject * RSNativeObject::GetObject( void ) const
{
	SObject *	pObj = m_refObject.GetReference() ;
	if ( pObj != NULL )
	{
		SSmartObject *	pSObj = ESLTypeCast<SSmartObject>( pObj ) ;
		if ( pSObj != NULL )
		{
			return	ESLTypeCast<SObject>( pSObj->GetObject() ) ;
		}
		return	pObj ;
	}
	return	NULL ;
}

SSystem::SObject * RSNativeObject::GetNativeOf( RSObject * pObj )
{
	if ( pObj != nullptr )
	{
		RSNativeObject *	pNObj =
			ESLTypeCast<RSNativeObject>( pObj->GetEntityObject() ) ;
		if ( pNObj != nullptr )
		{
			return	pNObj->GetObject() ;
		}
	}
	return	nullptr ;
}

// オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void RSNativeObject::AttachObject( SSystem::SObject * pObj )
{
	m_refObject.SetReference( pObj ) ;
}

void RSNativeObject::SetObject( ESLObject * pObj )
{
	m_refObject.SetReference( new SSmartObject( pObj ) ) ;
}

bool RSNativeObject::AttachNative( RSObject * pObj, SSystem::SObject * pNObj )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != nullptr )
	{
		pNativeObj->AttachObject( pNObj ) ;
		return	true ;
	}
	RSPointer *	pPtrObj = ESLTypeCast<RSPointer>( pObj ) ;
	if ( pPtrObj != nullptr )
	{
		pPtrObj->SetReference( new RSNativeObject( pNObj ) ) ;
		return	true ;
	}
	return	false ;
}

bool RSNativeObject::SetNative( RSObject * pObj, SSystem::SObject * pNObj )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != nullptr )
	{
		pNativeObj->SetObject( pNObj ) ;
		return	true ;
	}
	RSPointer *	pPtrObj = ESLTypeCast<RSPointer>( pObj ) ;
	if ( pPtrObj != nullptr )
	{
		pPtrObj->SetReference
			( new RSNativeObject( new SSmartObject( pNObj ) ) ) ;
		return	true ;
	}
	return	false ;
}

// オブジェクト分離
//////////////////////////////////////////////////////////////////////////////
SSystem::SObject * RSNativeObject::DetachObject( void )
{
	SSmartObject *	pSmartObj =
		ESLTypeCast<SSmartObject>( m_refObject.GetReference() ) ;
	if ( pSmartObj != NULL )
	{
		ESLObject *	pObj = pSmartObj->DetachObject() ;
		return	ESLSmartCast<SObject>( pObj ) ;
	}
	else
	{
		return	m_refObject.GetReference() ;
	}
}

void RSNativeObject::ReleaseNativeRef( void )
{
	ObjectListener *	pObjListener =
		ESLTypeCast<ObjectListener>( m_refObject.GetReference() ) ;
	if ( pObjListener != nullptr )
	{
		if ( IsObjectOwner() )
		{
			pObjListener->OnRelease( this ) ;
		}
		else
		{
			pObjListener->OnDetach( this ) ;
		}
	}
	m_refObject.ReleaseReference() ;
}

// オブジェクト所有判定
//////////////////////////////////////////////////////////////////////////////
bool RSNativeObject::IsObjectOwner( void ) const
{
	SObject *	pRef = m_refObject.GetReference() ;
	if ( pRef != NULL )
	{
		SSmartObject *	pSObj = ESLTypeCast<SSmartObject>( pRef ) ;
		return	(pSObj != NULL) ;
	}
	return	false ;
}

// オブジェクト所持
//////////////////////////////////////////////////////////////////////////////
void RSNativeObject::AddOwnObject( RSObject * pObj )
{
	RSObject::AddRef( pObj ) ;
	m_ownObjects.Add( new RSSmartPtr( pObj ) ) ;
}

// オブジェクト
//////////////////////////////////////////////////////////////////////////////
void RSNativeObject::ReleaseOwnObject( RSObject * pObj )
{
	size_t	nCount = m_ownObjects.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSSmartPtr *	psptr = m_ownObjects.GetAt( i ) ;
		if ( psptr != NULL )
		{
			if ( psptr->Ptr() == pObj )
			{
				m_ownObjects.RemoveAt( i ) ;
				break ;
			}
		}
	}
}

void RSNativeObject::ReleaseOwnNativeObject( SSystem::SObject * pObj )
{
	size_t	nCount = m_ownObjects.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSSmartPtr *	psptr = m_ownObjects.GetAt( i ) ;
		if ( psptr != NULL )
		{
			RSNativeObject *
				pnObj = ESLTypeCast<RSNativeObject>( psptr->Ptr() ) ;
			if ( (pnObj != NULL)
				&& (pnObj->GetObject() == pObj) )
			{
				m_ownObjects.RemoveAt( i ) ;
				break ;
			}
		}
	}
}

void RSNativeObject::ReleaseAllOwnObjects( void )
{
	m_ownObjects.RemoveAll() ;
}

// 所持オブジェクト検索
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNativeObject::FindOwnObject( const ESLRuntimeClass& rtClass ) const
{
	size_t	nCount = m_ownObjects.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSSmartPtr *	psptr = m_ownObjects.GetAt( i ) ;
		if ( psptr != NULL )
		{
			if ( psptr->Ptr()->IsKindOf( rtClass ) )
			{
				return	psptr->Ptr() ;
			}
		}
	}
	return	NULL ;
}

RSNativeObject * RSNativeObject::FindOwnNativeObject( SSystem::SObject * pObj ) const
{
	size_t	nCount = m_ownObjects.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSSmartPtr *	psptr = m_ownObjects.GetAt( i ) ;
		if ( psptr != NULL )
		{
			RSNativeObject *
				pnObj = ESLTypeCast<RSNativeObject>( psptr->Ptr() ) ;
			if ( (pnObj != NULL)
				&& (pnObj->GetObject() == pObj) )
			{
				return	pnObj ;
			}
		}
	}
	return	NULL ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSNativeObject::IsEqualObject( RSObject * pObj ) const
{
	if ( pObj != NULL )
	{
		pObj = pObj->GetEntityObject() ;
	}
	if ( this == pObj )
	{
		return	true ;
	}
	if ( pObj == NULL )
	{
		return	(GetObject() == NULL) ;
	}
	RSNativeObject *	pNtvObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNtvObj == NULL )
	{
		return	false ;
	}
	return	(GetObject() == pNtvObj->GetObject()) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSNativeObject::DisposeObject( RSContext& context )
{
	ReleaseNativeRef() ;
	ReleaseAllOwnObjects() ;
	RSGenericObject::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNativeObject::DuplicateObject( RSContext& context ) const
{
	RSNativeObject *	pObj = new RSNativeObject( *this ) ;
	pObj->m_gcmMembers.DuplicateAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNativeObject::CloneObject( RSContext& context ) const
{
	RSNativeObject *	pObj = new RSNativeObject( *this ) ;
	pObj->m_gcmMembers.CloneAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}
