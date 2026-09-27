
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_array.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// 配列基底オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSArray, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSArray::~RSArray( void )
{
	RemoveAllElements() ;
}

// 配列型設定
//////////////////////////////////////////////////////////////////////////////
void RSArray::SetArrayPrototype( size_t nLimit, RSClass * pElementClass )
{
	m_limitLength = nLimit ;
	m_pElementClass = pElementClass ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSArray::GetTypeName( void ) const
{
	return	L"Array" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArray::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"Array" ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

RSObject * RSArray::InstanceOf( RSClass * pClass )
{
	if ( m_pClass != NULL )
	{
		if ( m_pClass->IsInstanceOf( pClass ) )
		{
			return	this ;
		}
		return	NULL ;
	}
	return	RSObject::InstanceOf( pClass ) ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSArray::IsObjectType( void ) const
{
	return	true ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSArray::AsString( SSystem::SString& strValue ) const
{
	SString	strElement ;
	strValue = L"[" ;
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		if ( i != 0 )
		{
			strValue += L"," ;
		}
		RSObject *	pObj = m_elements.GetAt( i ) ;
		if ( pObj && pObj->AsString(strElement) )
		{
			if ( (pObj->GetEntityObject() != NULL)
				&& (pObj->GetEntityObject()->
							GetBasicType() == RSObject::typeString) )
			{
				SString	strTemp ;
				SStringParser::EncodeCLangString( strTemp, strElement ) ;
				strValue += L"\"" ;
				strValue += strTemp ;
				strValue += L"\"" ;
			}
			else
			{
				strValue += strElement ;
			}
			strElement = L"" ;
		}
	}
	strValue += L"]" ;
	return	true ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSArray::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	SString	strIndent = pwszIndent ;
	strIndent += L"\t" ;
	//
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		RSObject *	pObj = m_elements.GetAt( i ) ;
		if ( pObj != NULL )
		{
			SString	strDump = L"\r\n" ;
			strDump += pwszIndent ;
			strDump += "[" ;
			strDump += SString( i ) ;
			strDump += "] " ;
			strDump += pObj->GetTypeName() ;
			strDump += L" : " ;
			dump.WriteEncodedString( strDump ) ;
			//
			pObj->ToDebugDump( dump, nPtrNest, strIndent ) ;
		}
	}
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSArray::DisposeObject( RSContext& context )
{
	RSObject *const*	ppElements = m_elements.GetConstArray() ;
	size_t				count = m_elements.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		context.ReleaseObjectRef( ppElements[i] ) ;
	}
	m_elements.FreeArray() ;
	//
	RSObject::DisposeObject( context ) ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArray::DuplicateObject( RSContext& context ) const
{
	RSArray *	pArray = new RSArray( m_pClass, m_limitLength, m_pElementClass ) ;
	pArray->DuplicateAllElements( context, m_elements ) ;
	return	pArray ;
}

void RSArray::DuplicateAllElements
	( RSContext& context,
		const SSystem::SPointerArray<RSObject>& elements )
{
	size_t	count = elements.GetLength() ;
	SetArrayLength( count, context ) ;
	for ( size_t i = 0; i < count; i ++ )
	{
		RSObject *	pObj = elements.GetAt( i ) ;
		if ( pObj != NULL )
		{
			pObj = pObj->DuplicateObject( context ) ;
		}
		context.ReleaseObjectRef( SetElementAt( context, (int) i, pObj ) ) ;
	}
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArray::CloneObject( RSContext& context ) const
{
	RSArray *	pArray = new RSArray( m_pClass, m_limitLength, m_pElementClass ) ;
	pArray->CloneAllElements( context, m_elements ) ;
	return	pArray ;
}

void RSArray::CloneAllElements
	( RSContext& context,
		const SSystem::SPointerArray<RSObject>& elements )
{
	size_t	count = elements.GetLength() ;
	SetArrayLength( count, context ) ;
	for ( size_t i = 0; i < count; i ++ )
	{
		RSObject *	pObj = elements.GetAt( i ) ;
		if ( pObj != NULL )
		{
			pObj = pObj->CloneObject( context ) ;
		}
		context.ReleaseObjectRef( SetElementAt( context, (int) i, pObj ) ) ;
	}
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArray::GetElementAt( RSContext& context, int nIndex ) const
{
	RSObject *	pObj ;
	pObj = m_elements.GetAt( nIndex ) ;
	if ( pObj == NULL )
	{
		if ( (size_t) nIndex < m_limitLength )
		{
			RSClass *	pProto = m_pElementClass ;
			if ( pProto != NULL )
			{
				pObj = pProto->NewVariable( context ) ;
				((RSArray*)this)->m_elements.SetAt( (size_t) nIndex, pObj ) ;
				RSObject::AddRef( pObj ) ;
			}
			else
			{
				((RSArray*)this)->AddRef() ;
				pObj = new RSReferenceElement( (RSArray*) this, nIndex ) ;
			}
		}
		else
		{
			pObj = context.new_Reference( NULL ) ;
			context.ThrowExceptionError( L"配列の指標が範囲を超えています" ) ;
		}
	}
	else
	{
		pObj->AddRef() ;
	}
	return	pObj ;
}

// 要素設定
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArray::SetElementAt
	( RSContext& context, int nIndex, RSObject * pObj )
{
	RSObject *	pLastObj ;
	pLastObj = m_elements.GetAt( nIndex ) ;
	if ( (size_t) nIndex < m_limitLength )
	{
		RSClass *	pProto = m_pElementClass ;
		if ( pProto != NULL )
		{
			RSObject *	pCastObj = pProto->CastInstance( context, pObj ) ;
			context.ReleaseObjectRef( pObj ) ;
			pObj = pCastObj ;
			if ( pCastObj == NULL )
			{
				context.ThrowExceptionError
					( SString(pProto->GetRSClassName())
									+ L" へ型変換できません" ) ;
			}
		}
		m_elements.SetAt( (size_t) nIndex, pObj ) ;
	}
	context.ReleaseObjectRef( pLastObj ) ;
	RSObject::AddRef( pObj ) ;
	return	pObj ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSArray::GetElementCount( void ) const
{
	return	m_elements.GetLength() ;
}

// 要素最大数取得
//////////////////////////////////////////////////////////////////////////////
size_t RSArray::GetElementLimit( void ) const
{
	return	m_limitLength ;
}

// 要素全削除
//////////////////////////////////////////////////////////////////////////////
void RSArray::RemoveAllElements( void )
{
	size_t	count = m_elements.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		RSObject *	pObj = m_elements.GetAt( i ) ;
		RSObject::ReleaseRef( pObj ) ;
	}
	m_elements.SetLength( 0 ) ;
}

void RSArray::RemoveAllElements( RSContext& context )
{
	SetArrayLength( 0, context ) ;
}


// 配列長さ変更（切り詰め／拡張）
//////////////////////////////////////////////////////////////////////////////
void RSArray::SetArrayLength( size_t nLength, RSContext& context )
{
	size_t	count = m_elements.GetLength() ;
	for ( size_t i = nLength; i < count; i ++ )
	{
		RSObject *	pObj = m_elements.GetAt( i ) ;
		if ( pObj != NULL )
		{
			context.ReleaseObjectRef( pObj ) ;
		}
	}
	m_elements.SetLength( nLength ) ;
}

// 要素追加（特定型）
//////////////////////////////////////////////////////////////////////////////
void RSArray::AddIntegerElement( RSContext& context, int64_t num )
{
	m_elements.Add( context.new_Integer( num ) ) ;
}

void RSArray::AddNumberElement( RSContext& context, double num )
{
	m_elements.Add( context.new_Number( num ) ) ;
}

void RSArray::AddStringElement( RSContext& context, const wchar_t * pwszString )
{
	m_elements.Add( context.new_String( pwszString ) ) ;
}

void RSArray::AddObjectElement( RSObject * pObj )
{
	m_elements.Add( pObj ) ;
}

// 配列取得
//////////////////////////////////////////////////////////////////////////////
RSObject*const* RSArray::GetConstArrayPointer( void ) const
{
	return	m_elements.GetConstArray() ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArray::SerializeObject( RSContext& context )
{
	return	CloneObject( context ) ;
}

SSystem::SError RSArray::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	SError	err = errSuccess ;
	SString	strElementClass ;
	if ( m_pElementClass != NULL )
	{
		strElementClass = m_pElementClass->GetRSClassName() ;
	}
	file.WriteString( strElementClass ) ;
	//
	uint32_t	nLimit = (uint32_t) m_limitLength ;
	uint32_t	nLength = (uint32_t) m_elements.GetLength() ;
	file.Write( &nLimit, sizeof(uint32_t) ) ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	//
	for ( size_t i = 0; i < nLength; i ++ )
	{
		if ( SaveObjectBinary( m_elements.GetAt(i), context, file ) )
		{
			err = errFailed ;
		}
	}
	return	err ;
}

SSystem::SError RSArray::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	xmlDoc.SetAttrIntegerAs( L"limit", m_limitLength ) ;
	//
	if ( m_pElementClass != NULL )
	{
		SXMLDocument *	pxmlPrototype = new SXMLDocument ;
		pxmlPrototype->SetTag( L"prototype" ) ;
		pxmlPrototype->SetAttributeAs
				( L"class", m_pElementClass->GetRSClassName() ) ;
		xmlDoc.AddElement( pxmlPrototype ) ;
	}
	//
	SXMLDocument *	pxmlElements = new SXMLDocument ;
	pxmlElements->SetTag( L"elements" ) ;
	pxmlElements->SetAttrIntegerAs( L"length", m_elements.GetLength() ) ;
	xmlDoc.AddElement( pxmlElements ) ;
	//
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		SXMLDocument *	pxmlElement = new SXMLDocument ;
		RSObject *	pObj = m_elements.GetAt( i ) ;
		MakeXMLDocumentOfObject( pObj, context, *pxmlElement ) ;
		pxmlElements->AddElement( pxmlElement ) ;
	}
	return	errSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSArray::RestoreObject
		( RSContext& context, RSObject * pObj )
{
	RSArray *	pSrcArray = ESLTypeCast<RSArray>( pObj ) ;
	if ( pSrcArray == NULL )
	{
		return	errFailed ;
	}
	m_pElementClass = pSrcArray->m_pElementClass ;
	//
	CloneAllElements( context, pSrcArray->m_elements ) ;
	//
	return	errSuccess ;
}

SSystem::SError RSArray::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	SString	strElementClass ;
	file.ReadString( strElementClass ) ;
	m_pElementClass = context.GetClassAs( strElementClass ) ;
	//
	uint32_t	nLimit, nLength ;
	if ( file.Read( &nLimit, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	errFailed ;
	}
	if ( file.Read( &nLength, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	errFailed ;
	}
	m_limitLength = (size_t) nLimit ;
	//
	m_elements.SetLength( nLength ) ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		 m_elements.SetAt
			 ( i, RSObject::LoadObjectBinary( context, file ) ) ;
	}
	return	errSuccess ;
}

SSystem::SError RSArray::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	RemoveAllElements() ;
	//
	RSClass *	pElementClass = NULL ;
	size_t	limitLength = (size_t) xmlDoc.GetAttrIntegerAs( L"limit", 0 ) ;
	//
	SXMLDocument *	pxmlPrototype = xmlDoc.GetElementTagAs( L"prototype" ) ;
	if ( pxmlPrototype != NULL )
	{
		pElementClass =
			context.GetClassAs
				( pxmlPrototype->GetAttrStringAs( L"class" ) ) ;
	}
	SetArrayPrototype( limitLength, pElementClass ) ;
	//
	SXMLDocument *	pxmlElements = xmlDoc.GetElementTagAs( L"elements" ) ;
	if ( pxmlElements != NULL )
	{
		m_elements.SetLength
			( (size_t) pxmlElements->GetAttrIntegerAs( L"length", 0 ) ) ;
		//
		for ( size_t i = 0; i < pxmlElements->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlElement = pxmlElements->GetElementAt( i ) ;
			if ( pxmlElement != NULL )
			{
				m_elements.SetAt
					( i, RSObject::RestoreObjectOfXMLDocument
										( context, *pxmlElement ) ) ;
			}
		}
	}
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 配列ジェネリック型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSGenericArrayClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSGenericArrayClass::RSGenericArrayClass
	( RSClass * pClass,
		const wchar_t * pwszClassName,
		RSClass * pElementClass, RSObject * pRefNamespace )
	: RSClass( pClass, pwszClassName, pRefNamespace )
{
	m_pElementClass = pElementClass ;
	RSObject::AddRef( m_pElementClass ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSGenericArrayClass::~RSGenericArrayClass( void )
{
	RSObject::ReleaseRef( m_pElementClass ) ;
	m_pElementClass = NULL ;
}

// クラス型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSGenericArrayClass::IsInstanceOf( const wchar_t * pwszClass ) const
{
	return	(m_strClassName == pwszClass) ;
}

bool RSGenericArrayClass::IsInstanceOf( RSClass * pClass ) const
{
	RSGenericArrayClass *	pDstClass = ESLTypeCast<RSGenericArrayClass>( pClass ) ;
	if ( pDstClass == NULL )
	{
		return	false ;
	}
	if ( pDstClass->m_pElementClass == NULL )
	{
		return	true ;
	}
	return	(m_pElementClass != NULL)
			&& m_pElementClass->IsInstanceOf( pDstClass->m_pElementClass ) ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenericArrayClass::NewInstance( RSContext& context, RSObject * pArg )
{
	return	context.new_Array( 0x7FFFFFFF, m_pElementClass ) ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
bool RSGenericArrayClass::TestCastInstance
	( RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj == NULL )
	{
		return	true ;
	}
	pObj = pObj->GetEntityObject() ;
	if ( pObj == NULL )
	{
		return	true ;
	}
	RSArray *	pArray = ESLTypeCast<RSArray>( pObj ) ;
	if ( pArray == NULL )
	{
		return	false ;
	}
	if ( pArray->m_pElementClass == m_pElementClass )
	{
		return	true ;
	}
	if ( m_pElementClass == NULL )
	{
		return	true ;
	}
	if ( pArray->m_pElementClass == NULL )
	{
		return	false ;
	}
	return	pArray->m_pElementClass->IsInstanceOf( m_pElementClass ) ;
}

RSObject * RSGenericArrayClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj != NULL )
	{
		pObj = pObj->GetEntityObject() ;
		if ( pObj != NULL )
		{
			RSArray *	pArray = ESLTypeCast<RSArray>( pObj ) ;
			if ( pArray == NULL )
			{
				return	NULL ;
			}
			if ( (m_pElementClass == NULL)
				|| (pArray->m_pElementClass == m_pElementClass) )
			{
				pObj->AddRef() ;
				return	pObj ;
			}
			if ( (pArray->m_pElementClass != NULL)
				&& pArray->m_pElementClass->IsInstanceOf( m_pElementClass ) )
			{
				pObj->AddRef() ;
				return	pObj ;
			}
			if ( castMethod == castForce )
			{
				ESLAssert( m_pElementClass != NULL ) ;
				RSArray *	pDstArray =
					ESLTypeCast<RSArray>
						( context.new_Array( 0x7FFFFFFF, m_pElementClass ) ) ;
				ESLAssert( pDstArray != NULL ) ;
				//
				size_t	nLength = pArray->m_elements.GetLength() ;
				for ( size_t i = 0; i < nLength; i ++ )
				{
					RSObject *	pObj = pArray->m_elements.GetAt( i ) ;
					pObj = m_pElementClass->
								CastInstance( context, pObj, castForce ) ;
					if ( pObj == NULL )
					{
						context.ReleaseObjectRef( pDstArray ) ;
						pDstArray = NULL ;
						break ;
					}
					context.ReleaseObjectRef
						( pDstArray->SetElementAt( context, (int) i, pObj ) ) ;
					if ( context.IsException() )
					{
						context.ClearException() ;
						context.ReleaseObjectRef( pDstArray ) ;
						pDstArray = NULL ;
						break ;
					}
				}
				return	pDstArray ;
			}
			return	NULL ;
		}
	}
	return	context.new_Pointer( NULL, this ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Array 型クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSArrayClass, RSGenericArrayClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSArrayClass::RSArrayClass
	( RSClass * pClass,
		const wchar_t * pwszClassName,
		RSClass * pElementClass, RSObject * pRefNamespace )
	: RSGenericArrayClass( pClass, pwszClassName, pElementClass, pRefNamespace )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSArrayClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"length", L"int", L"",
			NULL, &RSArrayClass::method_length,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clear", NULL, L"",
						NULL, &RSArrayClass::method_clear, NULL ) ;
	//
	OverrideVirtualsGenerics( context ) ;
}

void RSArrayClass::OverrideVirtualsGenerics( RSContext& context )
{
	SString	strGenType = L"Object" ;
	if ( m_pElementClass != NULL )
	{
		strGenType = m_pElementClass->GetFullClassName() ;
		RemoveGenericFunctions( context ) ;
	}
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"indexOf",
			L"int", strGenType + L" obj, int from = 0",
			NULL, &RSArrayClass::method_indexOf,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add", L"boolean", strGenType + L" obj",
						NULL, &RSArrayClass::method_add1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"add", NULL,
			SString(L"int index, ") + strGenType + L" obj",
						NULL, &RSArrayClass::method_add2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"remove", strGenType, L"int index",
						NULL, &RSArrayClass::method_remove, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"push", L"int", strGenType + L" obj",
						NULL, &RSArrayClass::method_push, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pop", strGenType, L"",
						NULL, &RSArrayClass::method_pop, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"shift", strGenType, L"",
						NULL, &RSArrayClass::method_shift, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"unshift", NULL, strGenType + L" obj",
						NULL, &RSArrayClass::method_unshift, NULL ) ;
}

void RSArrayClass::RemoveGenericFunctions( RSContext& context )
{
	RemoveVirtualMemberAs( context, L"indexOf" ) ;
	RemoveVirtualMemberAs( context, L"add" ) ;
	RemoveVirtualMemberAs( context, L"remove" ) ;
	RemoveVirtualMemberAs( context, L"push" ) ;
	RemoveVirtualMemberAs( context, L"pop" ) ;
	RemoveVirtualMemberAs( context, L"shift" ) ;
	RemoveVirtualMemberAs( context, L"unshift" ) ;
}

// int length()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_length
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Integer( pThis->GetElementCount() ) ;
}

// int indexOf( obj, int from = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_indexOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjSearch = arg.ObjectAt( 0 ) ;
	int		iFirst = arg.IntAt( 1 ) ;
	//
	size_t	nCount = pThis->GetElementCount() ;
	int		iResult = -1 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pElement = pThis->GetElementAt( context, (int) i ) ;
		if ( pElement != NULL )
		{
			if ( pElement->IsEqualObject( pObjSearch ) )
			{
				context.ReleaseObjectRef( pElement ) ;
				iResult = (int) i ;
				break ;
			}
			context.ReleaseObjectRef( pElement ) ;
		}
		else if ( pObjSearch == NULL )
		{
			iResult = (int) i ;
			break ;
		}
	}
	return	context.new_Integer( iResult ) ;
}

// boolean add( obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_add1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.add 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	if ( pArray->m_elements.GetLength() >= pArray->m_limitLength )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	RSClass *	pProto = pArray->m_pElementClass ;
	if ( pProto != NULL )
	{
		pObj = pProto->CastInstance( context, pObj ) ;
		if ( pObj == NULL )
		{
			context.ThrowExceptionError
				( SString(pProto->GetRSClassName())
								+ L" へ型変換できません" ) ;
		}
	}
	else
	{
		RSObject::AddRef( pObj ) ;
	}
	pArray->m_elements.Add( pObj ) ;
	return	context.new_Boolean( true ) ;
}

// void add( int index, obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_add2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.add 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t	index = (size_t) arg.IntAt( 0 ) ;
	if ( (pArray->m_elements.GetLength() >= pArray->m_limitLength)
		|| (index > pArray->m_elements.GetLength()) )
	{
		context.ThrowExceptionError
			( L"指標が範囲内でないか配列長を拡張できません",
							L"ArrayIndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	RSObject *	pObj = arg.ObjectAt( 1 ) ;
	RSClass *	pProto = pArray->m_pElementClass ;
	if ( pProto != NULL )
	{
		pObj = pProto->CastInstance( context, pObj ) ;
		if ( pObj == NULL )
		{
			context.ThrowExceptionError
				( SString(pProto->GetRSClassName())
								+ L" へ型変換できません" ) ;
		}
	}
	else
	{
		RSObject::AddRef( pObj ) ;
	}
	pArray->m_elements.InsertAt( index, pObj ) ;
	return	NULL ;
}

// Object remove( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_remove
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.remove 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t	index = (size_t) arg.IntAt( 0 ) ;
	if ( index > pArray->m_elements.GetLength() )
	{
		context.ThrowExceptionError
			( L"指標が範囲内ではありません",
					L"ArrayIndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	RSObject *	pObj = pArray->m_elements.GetAt( index ) ;
	pArray->m_elements.RemoveAt( index ) ;
	return	pObj ;
}

// void clear()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_clear
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.clear 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	pArray->RemoveAllElements( context ) ;
	return	NULL ;
}

// int push( obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_push
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.push 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	if ( pArray->m_elements.GetLength() >= pArray->m_limitLength )
	{
		context.ThrowExceptionError
			( L"配列長を拡張できません",
					L"ArrayIndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	RSClass *	pProto = pArray->m_pElementClass ;
	if ( pProto != NULL )
	{
		pObj = pProto->CastInstance( context, pObj ) ;
		if ( pObj == NULL )
		{
			context.ThrowExceptionError
				( SString(pProto->GetRSClassName())
								+ L" へ型変換できません" ) ;
		}
	}
	else
	{
		RSObject::AddRef( pObj ) ;
	}
	pArray->m_elements.Add( pObj ) ;
	return	context.new_Integer( pArray->m_elements.GetLength() ) ;
}

// Object pop()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_pop
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.pop 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	if ( pArray->m_elements.GetLength() == 0 )
	{
		return	NULL ;
	}
	return	pArray->m_elements.Pop() ;
}

// Object shift()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_shift
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.shift 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	RSObject *	pObj = pArray->m_elements.GetAt( 0 ) ;
	pArray->m_elements.RemoveAt( 0 ) ;
	return	pObj ;
}

// void unshift( obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSArrayClass::method_unshift
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray = ESLTypeCast<RSArray>( pThis ) ;
	if ( pArray == NULL )
	{
		context.ThrowExceptionError
			( L"Array.unshift 関数の this が Array ではありません" ) ;
		return	NULL ;
	}
	if ( pArray->m_elements.GetLength() >= pArray->m_limitLength )
	{
		context.ThrowExceptionError
			( L"配列長を拡張できません",
					L"ArrayIndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	RSClass *	pProto = pArray->m_pElementClass ;
	if ( pProto != NULL )
	{
		pObj = pProto->CastInstance( context, pObj ) ;
		if ( pObj == NULL )
		{
			context.ThrowExceptionError
				( SString(pProto->GetRSClassName())
								+ L" へ型変換できません" ) ;
		}
	}
	else
	{
		RSObject::AddRef( pObj ) ;
	}
	pArray->m_elements.InsertAt( 0, pObj ) ;
	return	NULL ;
}
