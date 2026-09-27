
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_generic_object.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// RSScript コンテナ
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSScriptOwner, ESLObject )

RSScriptOwner::~RSScriptOwner( void )
{
	delete	m_prsScript ;
}

void RSScriptOwner::SetOwnScript( RSScript * pScript )
{
	ESLAssert( m_prsScript == nullptr ) ;
	delete	m_prsScript ;
	m_prsScript = pScript ;
}



//////////////////////////////////////////////////////////////////////////////
// 汎用クラス実装
//////////////////////////////////////////////////////////////////////////////

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSGenericClassMembers::~RSGenericClassMembers( void )
{
	RemoveAllMembers() ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSGenericClassMembers::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	SString	strIndent = pwszIndent ;
	strIndent += L"\t" ;
	//
	const size_t	count = m_members.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
					pElement = m_members.GetElementAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		RSObject *	pObj = pElement->m_obj ;
		//
		SString	strDump = L"\r\n" ;
		strDump += pwszIndent ;
		strDump += pElement->m_tag ;
		strDump += L" = " ;
		//
		if ( pObj != NULL )
		{
			strDump += pObj->GetTypeName() ;
			strDump += L" : " ;
			dump.WriteEncodedString( strDump ) ;
			//
			pObj->ToDebugDump( dump, nPtrNest, strIndent ) ;
		}
		else
		{
			strDump += L" : null" ;
			dump.WriteEncodedString( strDump ) ;
		}
	}
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
void RSGenericClassMembers::DuplicateAllMembers
	( RSContext& context,
		const SSystem::SStrSortArray<RSObject*>& members )
{
	const size_t	count = members.GetLength() ;
	RemoveAllMembers() ;
	m_members.SetLimit( count ) ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
					pElement = members.GetElementAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		RSObject *	pObj = pElement->m_obj ;
		if ( pObj != NULL )
		{
			RSObject *	pDup = pObj->DuplicateObject( context ) ;
			pDup->SetModifiers( pObj->GetModifiers() ) ;
			pObj = pDup ;
		}
		m_members.AddElement
			( new SStringSortElement<RSObject*>( pElement->m_tag, pObj ) ) ;
	}
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
void RSGenericClassMembers::CloneAllMembers
	( RSContext& context,
		const SSystem::SStrSortArray<RSObject*>& members )
{
	const size_t	count = members.GetLength() ;
	RemoveAllMembers() ;
	m_members.SetLimit( count ) ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
					pElement = members.GetElementAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		RSObject *	pObj = pElement->m_obj ;
		if ( pObj != NULL )
		{
			RSObject *	pDup = pObj->CloneObject( context ) ;
			pDup->SetModifiers( pObj->GetModifiers() ) ;
			pObj = pDup ;
		}
		m_members.AddElement
			( new SStringSortElement<RSObject*>( pElement->m_tag, pObj ) ) ;
	}
}

// 全メンバに DisposeObject を呼び出す
//////////////////////////////////////////////////////////////////////////////
void RSGenericClassMembers::DisposeAllMembers( RSContext& context )
{
	const size_t	count = m_members.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( i ) ;
		ESLAssert( pElement != nullptr ) ;
		if ( pElement->m_obj != nullptr )
		{
			pElement->m_obj->DisposeObject( context ) ;
		}
		context.ReleaseObjectRef( pElement->m_obj ) ;
	}
	m_members.RemoveAll() ;
}

// 全メンバ削除
//////////////////////////////////////////////////////////////////////////////
void RSGenericClassMembers::RemoveAllMembers( void )
{
	const size_t	count = m_members.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		RSObject *	pObj = pElement->m_obj ;
		RSObject::ReleaseRef( pObj ) ;
	}
	m_members.RemoveAll() ;
}

// メンバ削除
//////////////////////////////////////////////////////////////////////////////
void RSGenericClassMembers::RemoveMemberAs( const wchar_t * pwszName )
{
	ssize_t	iElement = m_members.FindAs( pwszName ) ;
	if ( iElement >= 0 )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( iElement ) ;
		ESLAssert( pElement != NULL ) ;
		RSObject::ReleaseRef( pElement->m_obj ) ;
		m_members.RemoveAt( iElement ) ;
	}
}

RSObject * RSGenericClassMembers::DetachMemberAs( const wchar_t * pwszName )
{
	RSObject *	pObj = NULL ;
	ssize_t	iElement = m_members.FindAs( pwszName ) ;
	if ( iElement >= 0 )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( iElement ) ;
		ESLAssert( pElement != NULL ) ;
		pObj = pElement->m_obj ;
		m_members.RemoveAt( iElement ) ;
	}
	return	pObj ;
}

// メンバ数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSGenericClassMembers::GetMemberCount( void ) const
{
	return	m_members.GetLength() ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenericClassMembers::GetMemberAt( size_t nIndex ) const
{
	RSObject**	ppObj = m_members.GetAt( nIndex ) ;
	if ( ppObj != NULL )
	{
		RSObject *	pObj = *ppObj ;
		RSObject::AddRef( pObj ) ;
		return	pObj ;
	}
	return	NULL ;
}

// メンバ数取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSGenericClassMembers::GetMemberNameAt( size_t nIndex ) const
{
	const SString *	pstrTag = m_members.GetTagAt( nIndex ) ;
	if ( pstrTag != NULL )
	{
		return	*pstrTag ;
	}
	return	NULL ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenericClassMembers::GetMemberAs( const wchar_t * pwszName ) const
{
	RSObject**	ppObj = m_members.GetAs( pwszName ) ;
	if ( ppObj != NULL )
	{
		RSObject *	pObj = *ppObj ;
		RSObject::AddRef( pObj ) ;
		return	pObj ;
	}
	return	NULL ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenericClassMembers::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	ssize_t	iElement = m_members.FindAs( pwszName ) ;
	if ( iElement >= 0 )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( iElement ) ;
		ESLAssert( pElement != NULL ) ;
		context.ReleaseObjectRef( pElement->m_obj ) ;
		pElement->m_obj = pObj ;
	}
	else
	{
		m_members.Add( pwszName, pObj ) ;
	}
	RSObject::AddRef( pObj ) ;
	return	pObj ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSGenericClassMembers::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	const uint32_t	count = (uint32_t) m_members.GetLength() ;
	if ( file.Write( &count, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	errFailed ;
	}
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( file.WriteString( pElement->m_tag ) )
		{
			return	errFailed ;
		}
		if ( RSObject::SaveObjectBinary( pElement->m_obj, context, file ) )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

SSystem::SError RSGenericClassMembers::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	size_t	nCount = m_members.GetLength() ;
	xmlDoc.SetAttrIntegerAs( L"count", nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		SXMLDocument *	pxmlElement = new SXMLDocument ;
		pxmlElement->SetTag( L"member" ) ;
		pxmlElement->SetAttributeAs( L"id", pElement->m_tag ) ;
		xmlDoc.AddElement( pxmlElement ) ;
		//
		SXMLDocument *	pxmlObject = new SXMLDocument ;
		RSObject::MakeXMLDocumentOfObject
				( pElement->m_obj, context, *pxmlObject ) ;
		pxmlElement->AddElement( pxmlObject ) ;
	}
	return	errSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSGenericClassMembers::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	uint32_t	count ;
	if ( file.Read( &count, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	errFailed ;
	}
//	RemoveAllMembers() ;
	//
	for ( size_t i = 0; i < count; i ++ )
	{
		SString	strName ;
		if ( file.ReadString( strName ) )
		{
			return	errFailed ;
		}
		context.ReleaseObjectRef
			( SetMemberAs
				( context, strName,
					RSObject::LoadObjectBinary( context, file ) ) ) ;
	}
	return	errSuccess ;
}

SSystem::SError RSGenericClassMembers::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
//	RemoveAllMembers() ;
	//
	size_t	nElements = xmlDoc.GetElementsCount() ;
	for ( size_t i = 0; i < nElements; i ++ )
	{
		SXMLDocument *	pxmlElement = xmlDoc.GetElementAt( i ) ;
		if ( pxmlElement->GetTag() != L"member" )
		{
			continue ;
		}
		SString	strID = pxmlElement->GetAttrStringAs( L"id" ) ;
		//
		SXMLDocument *	pxmlObject = pxmlElement->GetElementAt( 0 ) ;
		RSObject *	pObj = NULL ;
		if ( pxmlObject != NULL )
		{
			pObj = RSObject::RestoreObjectOfXMLDocument
									( context, *pxmlObject ) ;
		}
		context.ReleaseObjectRef( SetMemberAs( context, strID, pObj ) ) ;
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 汎用クラス実装（動的メンバ型）
//////////////////////////////////////////////////////////////////////////////

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSDynamicTypeClassMembers::~RSDynamicTypeClassMembers( void )
{
	RemoveAllMembers() ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
void RSDynamicTypeClassMembers::DuplicateAllMembers
	( RSContext& context,
		const SSystem::SStrSortArray<RSObject*>& members )
{
	RemoveAllMembers() ;
	//
	QuickLock() ;
	const size_t	count = members.GetLength() ;
	m_members.SetLimit( count ) ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
					pElement = members.GetElementAt( i ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		RSObject *	pObjSrc = pElement->m_obj ;
		RSObject *	pObjDup = NULL ;
		if ( pObjSrc != NULL )
		{
			pObjSrc->AddRef() ;
			QuickUnlock() ;
			pObjDup = pObjSrc->DuplicateObject( context ) ;
			QuickLock() ;
			pObjSrc->ReleaseRef() ;
		}
		m_members.AddElement
			( new SStringSortElement<RSObject*>( pElement->m_tag, pObjDup ) ) ;
	}
	QuickUnlock() ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
void RSDynamicTypeClassMembers::CloneAllMembers
	( RSContext& context,
		const SSystem::SStrSortArray<RSObject*>& members )
{
	RemoveAllMembers() ;
	//
	QuickLock() ;
	const size_t	count = members.GetLength() ;
	m_members.SetLimit( count ) ;
	for ( size_t i = 0; i < count; i ++ )
	{
		SStringSortElement<RSObject*> *
					pElement = members.GetElementAt( i ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		RSObject *	pObjSrc = pElement->m_obj ;
		RSObject *	pObjDup = NULL ;
		if ( pObjSrc != NULL )
		{
			pObjSrc->AddRef() ;
			QuickUnlock() ;
			pObjDup = pObjSrc->CloneObject( context ) ;
			QuickLock() ;
			pObjSrc->ReleaseRef() ;
		}
		m_members.AddElement
			( new SStringSortElement<RSObject*>( pElement->m_tag, pObjDup ) ) ;
	}
	QuickUnlock() ;
}

// 全メンバ削除
//////////////////////////////////////////////////////////////////////////////
void RSDynamicTypeClassMembers::RemoveAllMembers( void )
{
	SPointerArray<RSObject>	arrElements ;
	RSObject**	ppElements ;
	//
	QuickLock() ;
	const size_t	count = m_members.GetLength() ;
	if ( count > 0 )
	{
		arrElements.SetLength( count ) ;
		ppElements = arrElements.GetArray() ;
		//
		for ( size_t i = 0; i < count; i ++ )
		{
			SStringSortElement<RSObject*> *
					pElement = m_members.GetElementAt( i ) ;
			ESLAssert( pElement != NULL ) ;
			ppElements[i] = pElement->m_obj ;
		}
		m_members.RemoveAll() ;
	}
	QuickUnlock() ;
	//
	ppElements = arrElements.GetArray() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		RSObject *	pObj = ppElements[i] ;
		if ( pObj != NULL )
		{
			pObj->ReleaseRef() ;
		}
	}
	arrElements.FinishArray() ;
}

// メンバ削除
//////////////////////////////////////////////////////////////////////////////
void RSDynamicTypeClassMembers::RemoveMemberAs( const wchar_t * pwszName )
{
	RSObject *	pObj = NULL ;
	QuickLock() ;
	ssize_t	iElement = m_members.FindAs( pwszName ) ;
	if ( iElement >= 0 )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( iElement ) ;
		ESLAssert( pElement != NULL ) ;
		pObj = pElement->m_obj ;
		m_members.RemoveAt( iElement ) ;
	}
	QuickUnlock() ;
	//
	RSObject::ReleaseRef( pObj ) ;
}

// メンバ数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSDynamicTypeClassMembers::GetMemberCount( void ) const
{
	size_t	count ;
	QuickLock() ;
	count = m_members.GetLength() ;
	QuickUnlock() ;
	return	count ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicTypeClassMembers::GetMemberAt( size_t nIndex ) const
{
	QuickLock() ;
	RSObject *	pObj = RSGenericClassMembers::GetMemberAt( nIndex ) ;
	QuickUnlock() ;
	return	pObj ;
}

// メンバ数取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSDynamicTypeClassMembers::GetMemberNameAt( size_t nIndex ) const
{
	QuickLock() ;
	const wchar_t * pwszName = RSGenericClassMembers::GetMemberNameAt( nIndex ) ;
	QuickUnlock() ;
	return	pwszName ;		// not safe!!
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicTypeClassMembers::GetMemberAs( const wchar_t * pwszName ) const
{
	QuickLock() ;
	RSObject *	pObj =
		RSGenericClassMembers::GetMemberAs( pwszName ) ;
	QuickUnlock() ;
	return	pObj ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicTypeClassMembers::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	RSObject *	pReleaseObj = NULL ;
	if ( pObj != NULL )
	{
		RSObject *	pEntity = pObj->GetEntityObject() ;
		RSObject::AddRef( pEntity ) ;
		context.ReleaseObjectRef( pObj ) ;
		pObj = pEntity ;
	}
	QuickLock() ;
	ssize_t	iElement = m_members.FindAs( pwszName ) ;
	if ( iElement >= 0 )
	{
		SStringSortElement<RSObject*> *
				pElement = m_members.GetElementAt( iElement ) ;
		ESLAssert( pElement != NULL ) ;
		RSPointer *	pPtr = ESLTypeCast<RSPointer>( pElement->m_obj ) ;
		if ( pPtr != NULL )
		{
			pReleaseObj = pPtr->DetachReference() ;
			pPtr->SetReference( pObj ) ;
			pObj = pPtr ;
		}
		else
		{
			pReleaseObj = pElement->m_obj ;
			pObj = context.new_Pointer( pObj ) ;
			pElement->m_obj = pObj ;
		}
	}
	else
	{
		pObj = context.new_Pointer( pObj ) ;
		m_members.Add( pwszName, pObj ) ;
	}
	QuickUnlock() ;
	//
	if ( pReleaseObj != NULL )
	{
		context.ReleaseObjectRef( pReleaseObj ) ;
	}
	RSObject::AddRef( pObj ) ;
	return	pObj ;
}


//////////////////////////////////////////////////////////////////////////////
// 汎用オブジェクト（JavaScript Object 相当）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( Rosetta::RSDynamicObject, RSObject, RSScriptOwner )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDynamicObject::RSDynamicObject
		( RSContext& context, const RSDynamicObject& obj )
	: RSObject( obj.m_pClass, obj.m_typeObj, obj.m_accModifier ),
			m_modeProp( obj.m_modeProp ),
			m_pPropClass( obj.m_pPropClass ),
			m_pDefElement( NULL )
{
	m_dtcmProperties.DuplicateAllMembers
			( context, obj.m_dtcmProperties.m_members ) ;
	if ( obj.m_pDefElement != NULL )
	{
		m_pDefElement = obj.m_pDefElement->CloneObject( context ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSDynamicObject::~RSDynamicObject( void )
{
	if ( m_pDefElement != NULL )
	{
		m_pDefElement->ReleaseRef() ;
		m_pDefElement = NULL ;
	}
}

// メンバ型情報
//////////////////////////////////////////////////////////////////////////////
void RSDynamicObject::SetMemberClass( RSClass * pPropClass )
{
	m_pPropClass = pPropClass ;
}

// デフォルトエレメント設定
//////////////////////////////////////////////////////////////////////////////
void RSDynamicObject::SetDefaultElement( RSObject * pDefElement )
{
	if ( m_pDefElement != pDefElement )
	{
		if ( m_pDefElement != NULL )
		{
			m_pDefElement->ReleaseRef() ;
		}
		m_pDefElement = pDefElement ;
	}
}

// 即値型か？
//////////////////////////////////////////////////////////////////////////////
bool RSDynamicObject::IsObjectType( void ) const
{
	return	true ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSDynamicObject::AsString( SSystem::SString& strValue ) const
{
	strValue = L"{" ;
	//
	const size_t	count = m_dtcmProperties.GetMemberCount() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		RSObject *	pObj = m_dtcmProperties.GetMemberAt( i ) ;
		if ( pObj != NULL )
		{
			strValue += m_dtcmProperties.GetMemberNameAt(i) ;
			strValue += L":" ;
			//
			SString	strObjValue ;
			if ( pObj->AsString( strObjValue ) )
			{
				if ( (pObj->GetEntityObject() != NULL)
					&& (pObj->GetEntityObject()->
								GetBasicType() == RSObject::typeString) )
				{
					SString	strTemp ;
					SStringParser::EncodeCLangString( strTemp, strObjValue ) ;
					strValue += L"\"" ;
					strValue += strTemp ;
					strValue += L"\"" ;
				}
				else
				{
					strValue += strObjValue ;
				}
			}
			strValue += L"," ;
			//
			RSObject::ReleaseRef( pObj ) ;
		}
	}
	//
	strValue += L"}" ;
	return	true ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSDynamicObject::ToDebugDump
	( SSystem::SFileInterface& dump,
		size_t nPtrNest, const wchar_t * pwszIndent )
{
	m_dtcmProperties.ToDebugDump( dump, nPtrNest, pwszIndent ) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSDynamicObject::DisposeObject( RSContext& context )
{
	if ( m_pDefElement != NULL )
	{
		context.ReleaseObjectRef( m_pDefElement ) ;
		m_pDefElement = NULL ;
	}
	m_dtcmProperties.DisposeAllMembers( context ) ;
	m_dtcmProperties.RemoveAllMembers() ;
	RSObject::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::DuplicateObject( RSContext& context ) const
{
	RSDynamicObject *	pObj = new RSDynamicObject( m_pClass ) ;
	pObj->m_dtcmProperties.DuplicateAllMembers
				( context, m_dtcmProperties.m_members ) ;
	if ( m_pDefElement != NULL )
	{
		pObj->SetDefaultElement( m_pDefElement->CloneObject( context ) ) ;
	}
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::CloneObject( RSContext& context ) const
{
	RSDynamicObject *	pObj = new RSDynamicObject( m_pClass ) ;
	pObj->m_dtcmProperties.CloneAllMembers
				( context, m_dtcmProperties.m_members ) ;
	if ( m_pDefElement != NULL )
	{
		pObj->SetDefaultElement( m_pDefElement->CloneObject( context ) ) ;
	}
	return	pObj ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::GetElementAt( RSContext& context, int nIndex ) const
{
	return	m_dtcmProperties.GetMemberAt( (size_t) nIndex ) ;
}

// 要素名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSDynamicObject::GetElementNameAt( int nIndex ) const
{
	return	m_dtcmProperties.GetMemberNameAt( (size_t) nIndex ) ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSDynamicObject::GetElementCount( void ) const
{
	return	m_dtcmProperties.GetMemberCount() ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSDynamicObject::GetElementLimit( void ) const
{
	return	m_dtcmProperties.GetMemberCount() ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	RSObject *	pObj = m_dtcmProperties.GetMemberAs( pwszName ) ;
	if ( pObj == NULL )
	{
		if ( m_modeProp == modePropAllAccess )
		{
			if ( m_pDefElement != NULL )
			{
				return	const_cast<RSDynamicObject*>(this)->
					m_dtcmProperties.SetMemberAs
						( context, pwszName, m_pDefElement->CloneObject( context ) ) ;
			}
			else
			{
				((RSDynamicObject*)this)->AddRef() ;
				pObj = new RSReferenceMember
							( (RSDynamicObject*) this, pwszName ) ;
			}
		}
	}
	return	pObj ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	RSClass *	pClass = m_pPropClass ;
	if ( pClass != NULL )
	{
		RSObject *	pCastObj = pClass->CastInstance( context, pObj ) ;
		if ( pCastObj == NULL )
		{
			if ( (pObj != NULL)
				&& (pObj->GetEntityObject() != NULL) )
			{
				context.ThrowExceptionError
					( SString(pClass->GetRSClassName()) + L" へ型変換できません" ) ;
				context.ReleaseObjectRef( pObj ) ;
				return	NULL ;
			}
		}
		context.ReleaseObjectRef( pObj ) ;
		pObj = pCastObj ;
	}
	if ( m_modeProp == modePropAllAccess )
	{
		return	m_dtcmProperties.SetMemberAs( context, pwszName, pObj ) ;
	}
	else if ( m_modeProp == modePropReadWrite )
	{
		if ( m_dtcmProperties.GetMemberAs( pwszName ) == NULL )
		{
			context.ReleaseObjectRef( pObj ) ;
			context.ThrowExceptionError( L"メンバの追加は禁止されています" ) ;
			return	NULL ;
		}
		return	m_dtcmProperties.SetMemberAs( context, pwszName, pObj ) ;
	}
	context.ReleaseObjectRef( pObj ) ;
	context.ThrowExceptionError( L"メンバの変更は禁止されています" ) ;
	return	NULL ;
}

// メンバ新規作成
//////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::CreateMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	return	SetMemberAs( context, pwszName, pObj ) ;
}

// メンバ削除
//////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::RemoveMemberAs
	( RSContext& context, const wchar_t * pwszName )
{
	return	m_dtcmProperties.DetachMemberAs( pwszName ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObject::SerializeObject( RSContext& context )
{
	return	CloneObject( context ) ;
}

SSystem::SError RSDynamicObject::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	m_dtcmProperties.SerializeBinary( context, file ) ;
}

SSystem::SError RSDynamicObject::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	return	m_dtcmProperties.MakeXMLDocument( context, xmlDoc ) ;
}

// 復元
//////////////////////////////////////////////////////////////////////////
SSystem::SError RSDynamicObject::RestoreObject
		( RSContext& context, RSObject * pObj )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	m_dtcmProperties.RemoveAllMembers() ;
	//
	size_t	nCount = pObj->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pElementObj = pObj->GetElementAt( context, (int) i ) ;
		context.ReleaseObjectRef
			( m_dtcmProperties.SetMemberAs
				( context, pObj->GetElementNameAt( (int) i ),
					pElementObj->CloneObject( context ) ) ) ;
		context.ReleaseObjectRef( pElementObj ) ;
	}
	return	errSuccess ;
}

SSystem::SError RSDynamicObject::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	m_dtcmProperties.RemoveAllMembers() ;
	return	m_dtcmProperties.RestoreBinary( context, file ) ;
}

SSystem::SError RSDynamicObject::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	m_dtcmProperties.RemoveAllMembers() ;
	return	m_dtcmProperties.RestoreXMLDocument( context, xmlDoc ) ;
}


//////////////////////////////////////////////////////////////////////////
// 汎用オブジェクト（静的メンバ型）
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( Rosetta::RSGenericObject, RSObject, RSScriptOwner )

// 消滅関数
//////////////////////////////////////////////////////////////////////////
RSGenericObject::~RSGenericObject( void )
{
}

// static 関数設定
//////////////////////////////////////////////////////////////////////////////
void RSGenericObject::SetFunctionMemberAs
	( RSContext& context,
		const wchar_t * pwszName,
		RSFunctionPrototype * pProto, bool fOverride )
{
	RSObject *	pObjFunc = GetMemberAs( context, pwszName ) ;
	if ( (pObjFunc == NULL)
		|| (pObjFunc->GetBasicType() != RSObject::typeFunction) )
	{
		context.ReleaseObjectRef( pObjFunc ) ;
		pObjFunc = new RSFunctionObject( context.GetFunctionClass() ) ;
		pObjFunc = CreateMemberAs( context, pwszName, pObjFunc ) ;
	}
	ESLAssert( pObjFunc->IsKindOf( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
	((RSFunctionObject*)pObjFunc)->AddPrototype( pProto, fOverride ) ;
	context.ReleaseObjectRef( pObjFunc ) ;
}

RSFunctionPrototype * RSGenericObject::AddFunctionDescriptiveAs
	( RSContext& context,
		SSystem::SParserErrorInterface& perr,
		const wchar_t * pwszName,
		const wchar_t * pwszType, const wchar_t * pwszArgList,
		RSParenthesis * pParenthesis,
		RSObject::METHOD_PROC pfnMethod, void * pMethodInstance,
		RSCodeComment * pComment )
{
	RSClass *	pClassReturn = NULL ;
	if ( pwszType != NULL )
	{
		context.GetClassAs( pwszType ) ;
		if ( pClassReturn == NULL )
		{
			SStringParser	sparsType = pwszType ;
			RSScript		script ;
			SError	err = script.ParseScript
				( *(context.GetVM()->LockMacroContext()), sparsType, perr ) ;
			context.GetVM()->UnlockMacroContext() ;
			if ( err )
			{
				return	NULL ;
			}
			RSCodeStream	cs( script ) ;
			pClassReturn = context.ParseClassExpression( cs ) ;
			if ( context.IsException() )
			{
				context.OutputExceptionError( perr ) ;
				return	NULL ;
			}
		}
	}
	RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
	SError	err = pProto->ParseArgument( context, pwszArgList, perr ) ;
	if ( err )
	{
		delete	pProto ;
		return	NULL ;
	}
	pProto->SetReturnType( pClassReturn ) ;
	pProto->m_pParenthesis = pParenthesis ;
	pProto->m_methodNative.pfnMethod = pfnMethod ;
	pProto->m_methodNative.pInstance = pMethodInstance ;
	pProto->m_pComment = pComment ;
	//
	SetFunctionMemberAs( context, pwszName, pProto, true ) ;
	return	pProto ;
}

// 即値型か？
//////////////////////////////////////////////////////////////////////////
bool RSGenericObject::IsObjectType( void ) const
{
	return	true ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSGenericObject::AsString( SSystem::SString& strValue ) const
{
	strValue = L"{" ;
	//
	const size_t	count = m_gcmMembers.GetMemberCount() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		RSObject *	pObj = m_gcmMembers.GetMemberAt( i ) ;
		if ( pObj != NULL )
		{
			strValue += m_gcmMembers.GetMemberNameAt(i) ;
			strValue += L":" ;
			//
			SString	strObjValue ;
			if ( pObj->AsString( strObjValue ) )
			{
				if ( (pObj->GetEntityObject() != NULL)
					&& (pObj->GetEntityObject()->
								GetBasicType() == RSObject::typeString) )
				{
					SString	strTemp ;
					SStringParser::EncodeCLangString( strTemp, strObjValue ) ;
					strValue += L"\"" ;
					strValue += strTemp ;
					strValue += L"\"" ;
				}
				else
				{
					strValue += strObjValue ;
				}
			}
			strValue += L"," ;
			//
			RSObject::ReleaseRef( pObj ) ;
		}
	}
	//
	strValue += L"}" ;
	return	true ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////
void RSGenericObject::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	m_gcmMembers.ToDebugDump( dump, nPtrNest, pwszIndent ) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////
void RSGenericObject::DisposeObject( RSContext& context )
{
	m_gcmMembers.DisposeAllMembers( context ) ;
	m_gcmMembers.RemoveAllMembers() ;
	RSObject::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::DuplicateObject( RSContext& context ) const
{
	RSGenericObject *	pObj = new RSGenericObject( m_pClass ) ;
	pObj->m_gcmMembers.DuplicateAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::CloneObject( RSContext& context ) const
{
	RSGenericObject *	pObj = new RSGenericObject( m_pClass ) ;
	pObj->m_gcmMembers.CloneAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::GetElementAt( RSContext& context, int nIndex ) const
{
	RSObject *	pObj = m_gcmMembers.GetMemberAt( (size_t) nIndex ) ;
	return	pObj ;
}

// 要素名取得
//////////////////////////////////////////////////////////////////////////
const wchar_t * RSGenericObject::GetElementNameAt( int nIndex ) const
{
	return	m_gcmMembers.GetMemberNameAt( (size_t) nIndex ) ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////
size_t RSGenericObject::GetElementCount( void ) const
{
	return	m_gcmMembers.GetMemberCount() ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////
size_t RSGenericObject::GetElementLimit( void ) const
{
	return	m_gcmMembers.GetMemberCount() ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	RSObject *	pObj = m_gcmMembers.GetMemberAs( pwszName ) ;
	return	pObj ;
}

// メンバ設定
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::SetMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	RSObject *	pMember = m_gcmMembers.GetMemberAs( pwszName ) ;
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

// メンバ新規作成
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::CreateMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	RSObject *	pMember = m_gcmMembers.GetMemberAs( pwszName ) ;
	if ( pMember != NULL )
	{
		context.ThrowExceptionError
			( SString(pwszName) + L" は二重定義です" ) ;
		context.ReleaseObjectRef( pMember ) ;
	}
	else
	{
		pMember = m_gcmMembers.SetMemberAs( context, pwszName, pObj ) ;
		return	pMember ;
	}
	return	NULL ;
}

// メンバ削除
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::RemoveMemberAs
	( RSContext& context, const wchar_t * pwszName )
{
	return	m_gcmMembers.DetachMemberAs( pwszName ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////
RSObject * RSGenericObject::SerializeObject( RSContext& context )
{
	return	CloneObject( context ) ;
}

SSystem::SError RSGenericObject::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	m_gcmMembers.SerializeBinary( context, file ) ;
}

SSystem::SError RSGenericObject::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	return	m_gcmMembers.MakeXMLDocument( context, xmlDoc ) ;
}

// 復元
//////////////////////////////////////////////////////////////////////////
SSystem::SError RSGenericObject::RestoreObject
		( RSContext& context, RSObject * pObj )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
//	m_gcmMembers.RemoveAllMembers() ;
	//
	size_t	nCount = pObj->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pElementObj =
				pObj->GetElementAt( context, (int) i ) ;
		context.ReleaseObjectRef
			( m_gcmMembers.SetMemberAs
				( context, pObj->GetElementNameAt( (int) i ),
					pElementObj->CloneObject( context ) ) ) ;
		context.ReleaseObjectRef( pElementObj ) ;
	}
	return	errSuccess ;
}

SSystem::SError RSGenericObject::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	m_gcmMembers.RestoreBinary( context, file ) ;
}

SSystem::SError RSGenericObject::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	return	m_gcmMembers.RestoreXMLDocument( context, xmlDoc ) ;
}
