
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_string.h>
#include <rosetta/rosetta_array.h>
#include <sakuraglx/sglx_std_app.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// 名前空間
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSNamespace, RSGenericObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSNamespace::RSNamespace
	( RSObject * pRefSpace,
		uint32_t accMod, RSObject * pBackLink, RSClass * pClass )
	: RSGenericObject( pClass, typeNamespace, accMod ),
		m_pRefBackLink( pBackLink ),
		m_pRefNamespace( pRefSpace ),
		m_accModifier( accMod )
{
	RSObject::AddRef( pRefSpace ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSNamespace::~RSNamespace( void )
{
	RSObject::ReleaseRef( m_pRefBackLink ) ;
	RSObject::ReleaseRef( m_pRefNamespace ) ;
	m_pRefBackLink = NULL ;
	m_pRefNamespace = NULL ;
}

// 参照名前空間設定
//////////////////////////////////////////////////////////////////////////////
void RSNamespace::AttachReference
	( RSObject * pRefSpace, uint32_t accMod, RSObject * pBackLink )
{
	RSObject::AddRef( pRefSpace ) ;
	//
	RSObject::ReleaseRef( m_pRefBackLink ) ;
	RSObject::ReleaseRef( m_pRefNamespace ) ;
	//
	m_pRefBackLink = pBackLink ;
	m_pRefNamespace = pRefSpace ;
	m_accModifier = accMod ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSNamespace::DisposeObject( RSContext& context )
{
	RSGenericObject::DisposeObject( context ) ;
	//
	context.ReleaseObjectRef( m_pRefBackLink ) ;
	context.ReleaseObjectRef( m_pRefNamespace ) ;
	m_pRefBackLink = NULL ;
	m_pRefNamespace = NULL ;
	//
	m_accModifier = RSObject::modifierPublic ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSNamespace::ToDebugDump
	( SSystem::SFileInterface& dump,
		size_t nPtrNest, const wchar_t * pwszIndent )
{
	SString	strIndent = pwszIndent ;
	strIndent += L"\t" ;
	//
	if ( m_pRefBackLink != NULL )
	{
		SString	strBackLink = L"\r\n" ;
		strBackLink += pwszIndent ;
		strBackLink += L"with" ;
		dump.WriteEncodedString( strBackLink ) ;
		//
		m_pRefBackLink->ToDebugDump( dump, nPtrNest, strIndent ) ;
	}
	if ( m_pRefNamespace != NULL )
	{
		SString	strRefNamespace = L"\r\n" ;
		strRefNamespace += pwszIndent ;
		strRefNamespace += L"with" ;
		dump.WriteEncodedString( strRefNamespace ) ;
		//
		m_pRefNamespace->ToDebugDump( dump, nPtrNest, strIndent ) ;
	}
	//
	RSGenericObject::ToDebugDump( dump, nPtrNest, pwszIndent ) ;
}

// メンバ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSNamespace::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	RSObject *	pObj = RSGenericObject::GetMemberAs( context, pwszName ) ;
	if ( pObj != NULL )
	{
		return	pObj ;
	}
	RSObject *	pRef = m_pRefNamespace ;
	if ( pRef != NULL )
	{
		pObj = pRef->GetMemberAs( context, pwszName ) ;
		if ( pObj != NULL )
		{
			if ( pObj->IsEnableAccessModifier( m_accModifier ) )
			{
				return	pObj ;
			}
			pObj->ReleaseRef() ;
		}
	}
	pRef = m_pRefBackLink ;
	if ( pRef != NULL )
	{
		return	pRef->GetMemberAs( context, pwszName ) ;
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// クラスオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSClass, RSNamespace )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSClass::RSClass
	( RSClass * pClass, const wchar_t * pwszClassName,
		RSObject * pRefSpace, uint32_t accMod, RSObject * pBackLink )
	: RSNamespace( pRefSpace, accMod, pBackLink, pClass ), m_strClassName( pwszClassName )
{
	m_typeObj = typeClass ;
	//
	m_flagInitialized = false ;
	m_flagNativeClass = false ;
	m_flagCompiled = false ;
	m_pNamespace = NULL ;
	m_pSuperClass = NULL ;
	m_pPrototype = NULL ;
	m_pConstructor = NULL ;
	m_pDestructor = NULL ;
	m_pArrayClass = NULL ;
	m_pHashMapClass = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSClass::~RSClass( void )
{
	RSObject::ReleaseRef( m_pPrototype ) ;
	RSObject::ReleaseRef( m_pConstructor ) ;
	RSObject::ReleaseRef( m_pDestructor ) ;
	m_pPrototype = NULL ;
	m_pConstructor = NULL ;
	m_pDestructor = NULL ;
	//
	for ( size_t i = 0; i < m_lstImplements.GetLength(); i ++ )
	{
		RSClass *	pClass = m_lstImplements.GetAt( i ) ;
		ESLAssert( pClass != NULL ) ;
		pClass->ReleaseRef() ;
	}
	m_lstImplements.RemoveAll() ;
	//
	RSObject::ReleaseRef( m_pSuperClass ) ;
	m_pSuperClass = NULL ;
	//
	RSObject::ReleaseRef( m_pArrayClass ) ;
	m_pArrayClass = NULL ;
	RSObject::ReleaseRef( m_pHashMapClass ) ;
	m_pHashMapClass = NULL ;
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSClass::Initialize( RSContext& context )
{
	if ( m_pSuperClass == NULL )
	{
		RSClass *	pObjClass = context.GetGenericObjectClass() ;
		if ( pObjClass != this )
		{
			AddSuperClass( context, pObjClass ) ;
		}
	}
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSClass::OverrideVirtuals( RSContext& context )
{
}

// クラス定義の完了
//////////////////////////////////////////////////////////////////////////////
void RSClass::FinishClass( RSContext& context )
{
}

// ネイティブ関数を仮想関数としてクラスにオーバーライドする
//////////////////////////////////////////////////////////////////////////////
void RSClass::AddVirtualNativeMethods
		( RSContext& context, const RSClass::NativeMethodEntry * pnme )
{
	SParserErrorTracer	perr ;
	while ( pnme->pwszName != NULL )
	{
		AddVirtualDescriptiveAs
			( context, perr, pnme->pwszName,
				pnme->pwszReturnType, pnme->pwszArgumentList,
				NULL, pnme->pfnMethod, NULL ) ;
		pnme ++ ;
	}
}

// コメント生成
//////////////////////////////////////////////////////////////////////////////
RSCodeComment * RSClass::ImmediateComment( const wchar_t * pwszText )
{
	RSCodeComment *	pComment = new RSCodeComment( pwszText ) ;
	m_aImmComments.Add( pComment ) ;
	return	pComment ;
}

// クラス名取得
//////////////////////////////////////////////////////////////////////////////
SString RSClass::GetFullClassName( void ) const
{
	SString	strClassName ;
	if ( m_pNamespace != NULL )
	{
		strClassName = m_pNamespace->GetFullClassName() ;
		strClassName += L"." ;
	}
	strClassName += m_strClassName ;
	return	strClassName ;
}

// クラス型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSClass::IsInstanceOf( const wchar_t * pwszClass ) const
{
	if ( m_strClassName == pwszClass )
	{
		return	true ;
	}
	if ( (m_pSuperClass != NULL)
		&& m_pSuperClass->IsInstanceOf( pwszClass ) )
	{
		return	true ;
	}
	size_t	nCount = m_lstImplements.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSClass *	pSuperClass = m_lstImplements.GetAt( i ) ;
		if ( (pSuperClass != NULL)
			&& pSuperClass->IsInstanceOf( pwszClass ) )
		{
			return	true ;
		}
	}
	return	false ;
}

bool RSClass::IsInstanceOf( RSClass * pClass ) const
{
	if ( pClass == this )
	{
		return	true ;
	}
	if ( (m_pSuperClass != NULL)
		&& m_pSuperClass->IsInstanceOf( pClass ) )
	{
		return	true ;
	}
	size_t	nCount = m_lstImplements.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSClass *	pSuperClass = m_lstImplements.GetAt( i ) ;
		if ( (pSuperClass != NULL)
			&& pSuperClass->IsInstanceOf( pClass ) )
		{
			return	true ;
		}
	}
	return	false ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	false ;
}

// 仮想関数取得
//////////////////////////////////////////////////////////////////////////////
RSFunctionObject * RSClass::GetVirtualMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	RSObject *	pObj = m_gcmVirtuals.GetMemberAs( pwszName ) ;
	if ( pObj != NULL )
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
		return	(RSFunctionObject*) pObj ;
	}
	return	NULL ;
}

// 仮想関数設定
//////////////////////////////////////////////////////////////////////////////
void RSClass::SetVirtualMemberAs
	( RSContext& context,
		const wchar_t * pwszName, RSFunctionObject * pFunc )
{
	pFunc->m_strFuncName = pwszName ;
	pFunc->m_pFuncClass = this ;
	//
	pFunc = (RSFunctionObject*)
				m_gcmVirtuals.SetMemberAs( context, pwszName, pFunc ) ;
	ESLAssert( pFunc->IsKindOf( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
	//
	if ( SString::Compare( pwszName, L"<init>" ) == 0 )
	{
		m_pConstructor = pFunc ;
		RSObject::AddRef( pFunc ) ;
	}
	else if ( SString::Compare( pwszName, L"finalize" ) == 0 )
	{
		m_pDestructor = pFunc ;
		RSObject::AddRef( pFunc ) ;
	}
	context.ReleaseObjectRef( pFunc ) ;
}

size_t RSClass::SetVirtualMemberAs
	( RSContext& context,
		const wchar_t * pwszName,
		RSFunctionPrototype * pProto, bool fOverride )
{
	RSObject *	pObjFunc = m_gcmVirtuals.GetMemberAs( pwszName ) ;
	if ( (pObjFunc == NULL)
		|| (pObjFunc->GetBasicType() != RSObject::typeFunction) )
	{
		context.ReleaseObjectRef( pObjFunc ) ;
		pObjFunc = new RSFunctionObject( context.GetFunctionClass() ) ;
		((RSFunctionObject*)pObjFunc)->m_strFuncName = pwszName ;
		((RSFunctionObject*)pObjFunc)->m_pFuncClass = this ;
		pObjFunc = m_gcmVirtuals.SetMemberAs( context, pwszName, pObjFunc ) ;
	}
	ESLAssert( pObjFunc->IsKindOf( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
	size_t	iOverride =
		((RSFunctionObject*)pObjFunc)->AddPrototype( pProto, fOverride ) ;
	//
	if ( SString::Compare( pwszName, L"<init>" ) == 0 )
	{
		RSObject::ReleaseRef( m_pConstructor ) ;
		m_pConstructor = (RSFunctionObject*) pObjFunc ;
		RSObject::AddRef( pObjFunc ) ;
	}
	else if ( SString::Compare( pwszName, L"finalize" ) == 0 )
	{
		RSObject::ReleaseRef( m_pDestructor ) ;
		m_pDestructor = (RSFunctionObject*) pObjFunc ;
		RSObject::AddRef( pObjFunc ) ;
	}
	context.ReleaseObjectRef( pObjFunc ) ;
	//
	return	iOverride ;
}

RSFunctionPrototype * RSClass::AddVirtualDescriptiveAs
	( RSContext& context,
		SParserErrorInterface& perr,
		const wchar_t * pwszName,
		const wchar_t * pwszType, const wchar_t * pwszArgList,
		RSParenthesis * pParenthesis,
		RSObject::METHOD_PROC pfnMethod,
		void * pMethodInstance, uint32_t nFlags,
		const wchar_t * pwszComment )
{
	RSClass *	pClassReturn = NULL ;
	if ( pwszType != NULL )
	{
		pClassReturn = context.GetClassAs( pwszType ) ;
		if ( pClassReturn == NULL )
		{
			RSSourceParser	sparsType = pwszType ;
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
	pProto->m_nFlags |= nFlags ;
	pProto->SetReturnType( pClassReturn ) ;
	pProto->m_pNamespaceClass = this ;
	pProto->m_pParenthesis = pParenthesis ;
	pProto->m_methodNative.pfnMethod = pfnMethod ;
	pProto->m_methodNative.pInstance = pMethodInstance ;
	//
	if ( pwszComment != NULL )
	{
		pProto->m_pComment = ImmediateComment( pwszComment ) ;
	}
	//
	SetVirtualMemberAs( context, pwszName, pProto, true ) ;
	return	pProto ;
}

// 仮想関数削除
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSClass::RemoveVirtualMemberAs
	( RSContext& context, const wchar_t * pwszName )
{
	RSObject *	pObj = m_gcmVirtuals.DetachMemberAs( pwszName ) ;
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	context.ReleaseObjectRef( pObj ) ;
	return	errSuccess ;
}

// 派生元クラス追加
//////////////////////////////////////////////////////////////////////////////
void RSClass::AddSuperClass( RSContext& context, RSClass * pSuperClass )
{
	ESLAssert( pSuperClass != NULL ) ;
	if ( m_pSuperClass != NULL )
	{
		context.ThrowExceptionError( L"クラスの多重派生は出来ません" ) ;
		return ;
	}
	if ( pSuperClass->IsInstanceOf( this ) || IsInstanceOf( pSuperClass ) )
	{
		context.ThrowExceptionError( L"派生元クラスが循環しています" ) ;
		return ;
	}
	if ( !pSuperClass->m_flagInitialized )
	{
		context.ThrowExceptionError( L"派生元クラスが未定義です" ) ;
		return ;
	}
	pSuperClass->AddRef() ;
	RSObject::ReleaseRef( m_pSuperClass ) ;
	m_pSuperClass = pSuperClass ;
	//
	// オブジェクトテンプレート複製
	//
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = NULL ;
	if ( pSuperClass->m_pPrototype != NULL )
	{
		m_pPrototype = pSuperClass->m_pPrototype->CloneObject( context ) ;
		ESLAssert( m_pPrototype != NULL ) ;
		m_pPrototype->SetRSClass( this ) ;
		//
		if ( (ESLTypeCast<RSGenericObject>( m_pPrototype ) != NULL)
			|| (ESLTypeCast<RSDynamicObject>( m_pPrototype ) != NULL) )
		{
			size_t	nCount = m_pPrototype->GetElementCount() ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				RSObject *	pMember =
					m_pPrototype->GetElementAt( context, (int) i ) ;
				if ( pMember != NULL )
				{
					if ( pMember->GetAccessModifier() == RSObject::modifierPrivate )
					{
						pMember->SetModifiers
							( (pMember->GetModifiers() & ~RSObject::accessMask)
											| RSObject::modifierPrivateInvisible ) ;
					}
					pMember->ReleaseRef() ;
				}
			}
		}
	}
	//
	// 仮想関数複製
	//
	m_gcmVirtuals.DuplicateAllMembers
			( context, pSuperClass->m_gcmVirtuals.m_members ) ;
	//
	size_t	nCount = m_gcmVirtuals.GetMemberCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSSmartPtr	pObj( m_gcmVirtuals.GetMemberAt(i), &context ) ;
		RSFunctionObject *
			pFuncObj = ESLTypeCast<RSFunctionObject>( pObj.Ptr() ) ;
		if ( pFuncObj != NULL )
		{
			pFuncObj->m_pFuncClass = this ;
		}
	}
	context.ReleaseObjectRef( m_pConstructor ) ;
	context.ReleaseObjectRef( m_pDestructor ) ;
	m_pConstructor = (RSFunctionObject*)
						m_gcmVirtuals.GetMemberAs( L"<init>" ) ;
	m_pDestructor = (RSFunctionObject*)
						m_gcmVirtuals.GetMemberAs( L"finalize" ) ;
}

void RSClass::AddImplementClass
	( RSContext& context, RSClass * pSuperClass, bool fMatchVirtualIndex )
{
	ESLAssert( pSuperClass != NULL ) ;
	if ( pSuperClass->IsInstanceOf( this ) || IsInstanceOf( pSuperClass ) )
	{
		context.ThrowExceptionError( L"実装インターフェースが循環しています" ) ;
		return ;
	}
	if ( !pSuperClass->m_flagInitialized )
	{
		context.ThrowExceptionError( L"派生元クラスが未定義です" ) ;
		return ;
	}
	pSuperClass->AddRef() ;
	m_lstImplements.Add( pSuperClass ) ;
	//
	if ( pSuperClass->m_pPrototype != NULL )
	{
		if ( pSuperClass->m_pPrototype->GetElementCount() > 0 )
		{
			context.ThrowExceptionError
				( L"実装インターフェースにメンバ変数が含まれています" ) ;
			return ;
		}
	}
	//
	// 仮想関数を登録
	//
	size_t	nVirtCount = pSuperClass->m_gcmVirtuals.GetMemberCount() ;
	for ( size_t i = 0; i < nVirtCount; i ++ )
	{
		RSObject *	pObjVirt = pSuperClass->m_gcmVirtuals.GetMemberAt( i ) ;
		if ( (pObjVirt == NULL)
			|| (pObjVirt->GetBasicType() != RSObject::typeFunction) )
		{
			context.ReleaseObjectRef( pObjVirt ) ;
			continue ;
		}
		SString	strVirtName = pSuperClass->m_gcmVirtuals.GetMemberNameAt( i ) ;
		//
		ESLAssert( pObjVirt->IsKindOf( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
		RSFunctionObject *	pVirtFunc = (RSFunctionObject*) pObjVirt ;
		//
		size_t	nProtoCount = pVirtFunc->m_arrPrototypes.GetLength() ;
		for ( size_t j = 0; j < nProtoCount; j ++ )
		{
			RSFunctionPrototype *
				pSrcProto = pVirtFunc->m_arrPrototypes.GetAt( j ) ;
			ESLAssert( pSrcProto != NULL ) ;
			size_t	iVirt =
				SetVirtualMemberAs
					( context, strVirtName,
						new RSFunctionPrototype( *pSrcProto ), false ) ;
			if ( fMatchVirtualIndex && (iVirt != j) )
			{
				context.ThrowExceptionError
					( SString(L"仮想関数 \'") + strVirtName
						+ L"\' のオーバーライドは \'"
						+ pSuperClass->GetFullClassName()
						+ L"\' の関数指標に適合しません" ) ;
			}
		}
		//
		context.ReleaseObjectRef( pObjVirt ) ;
	}
}

// 親クラス
//////////////////////////////////////////////////////////////////////////////
RSClass * RSClass::GetSuperClass( void ) const
{
	return	m_pSuperClass ;
}

size_t RSClass::GetImplementClassCount( void ) const
{
	return	m_lstImplements.GetLength() ;
}

RSClass * RSClass::GetImplementClassAt( size_t i ) const
{
	return	m_lstImplements.GetAt( i ) ;
}

// メンバ新規作成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSClass::CreateMemberAs
	( RSContext& context, const wchar_t * pwszName, RSObject * pObj )
{
	pObj = RSNamespace::CreateMemberAs( context, pwszName, pObj ) ;
	if ( pObj != NULL )
	{
		RSClass *	pClass = ESLTypeCast<RSClass>( pObj ) ;
		if ( pClass != NULL )
		{
			pClass->m_pNamespace = this ;
		}
	}
	return	pObj ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSClass::NewInstance( RSContext& context, RSObject * pArg )
{
	RSObject *	pObj ;
	if ( m_pPrototype != NULL )
	{
		pObj = m_pPrototype->CloneObject( context ) ;
	}
	else
	{
		pObj = new RSDynamicObject( this ) ;
	}
	if ( m_pConstructor != NULL )
	{
		bool		fArgMatchResult ;
		RSObject *	pRetInstance =
			context.CallFunction
				( *m_pConstructor, pObj, *pArg, false, &fArgMatchResult ) ;
		if ( !fArgMatchResult )
		{
			if ( pArg->GetElementCount() > 0 )
			{
				context.ThrowExceptionError
					( L"構築関数の呼び出しで引数がプロトタイプに一致しません" ) ;
			}
		}
		if ( pRetInstance != NULL )
		{
			context.ReleaseObjectRef( pObj ) ;
			pObj = pRetInstance ;
		}
	}
	return	pObj ;
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSClass::NewVariable( RSContext& context )
{
	return	context.new_Pointer( NULL, this ) ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
bool RSClass::TestCastInstance
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
	RSClass *	pObjType = pObj->GetRSClass() ;
	if ( pObjType == NULL )
	{
		return	false ;
	}
	return	pObjType->IsInstanceOf( this ) ;
}

RSObject * RSClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( pObj != NULL )
	{
		pObj = pObj->GetEntityObject() ;
		if ( pObj != NULL )
		{
			RSClass *	pObjClass = pObj->GetRSClass() ;
			if ( pObjClass != NULL )
			{
				if ( pObjClass->IsInstanceOf( this ) )
				{
					pObj->AddRef() ;
					return	pObj ;
				}
			}
			return	NULL ;
		}
	}
	return	context.new_Pointer( NULL, this ) ;
}

// 構築関数取得
//////////////////////////////////////////////////////////////////////////////
RSFunctionObject * RSClass::GetConstructor( void ) const
{
	return	m_pConstructor ;
}

// 消滅関数取得
//////////////////////////////////////////////////////////////////////////////
RSFunctionObject * RSClass::GetDestructor( void ) const
{
	return	m_pDestructor ;
}

// プロトタイプ取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSClass::GetPrototypeObject( void ) const
{
	return	m_pPrototype ;
}

// 仮想関数配列取得
//////////////////////////////////////////////////////////////////////////////
const RSGenericClassMembers& RSClass::GetVirtualFunctions( void ) const
{
	return	m_gcmVirtuals ;
}

// クラス定義ダンプ
//////////////////////////////////////////////////////////////////////////////
void RSClass::DumpClassDeclaration
	( RSContext& context, SSystem::SString& strDecl )
{
	//
	// クラス定義
	//
	strDecl = L"class " ;
	strDecl += m_strClassName ;
	if ( m_pSuperClass != NULL )
	{
		strDecl += L"\textends " ;
		strDecl += m_pSuperClass->GetFullClassName() ;
	}
	if ( m_lstImplements.GetLength() > 0 )
	{
		strDecl += L"\timplements " ;
		for ( size_t i = 0; i < m_lstImplements.GetLength(); i ++ )
		{
			RSClass *	pImplement = m_lstImplements.GetAt( i ) ;
			if ( pImplement != NULL )
			{
				if ( i > 0 )
				{
					strDecl += L", " ;
				}
				strDecl += pImplement->GetFullClassName() ;
			}
		}
	}
	strDecl += L"\r\n{\r\n" ;
	//
	// クラス静メンバ
	//
	DumpClassStaticMembers( context, strDecl ) ;
	//
	// プロトタイプ
	//
	DumpClassPrototypeMembers( context, strDecl ) ;
	//
	// 仮想関数
	//
	DumpClassVirtualFunctions( context, strDecl ) ;
	//
	strDecl += L"\r\n}" ;
}

// クラス静メンバダンプ
//////////////////////////////////////////////////////////////////////////////
void RSClass::DumpClassStaticMembers
	( RSContext& context, SSystem::SString& strDecl )
{
	size_t	i, nCount ;
	nCount = GetElementCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszName = GetElementNameAt( (int) i ) ;
		RSSmartPtr	pObj( GetElementAt( context, (int) i ), &context ) ;
		if ( (pwszName != NULL) && (pObj != NULL) )
		{
			SString	strDeclVar ;
			DumpVarDeclaration
				( context, strDeclVar,
					pwszName, pObj, RSObject::modifierStatic ) ;
			AddFormatIndentedString( strDecl, strDeclVar, L"\t" ) ;
			strDecl += L" ;\r\n" ;
		}
	}
}

// クラスメンバダンプ
//////////////////////////////////////////////////////////////////////////////
void RSClass::DumpClassPrototypeMembers
	( RSContext& context, SSystem::SString& strDecl )
{
	if ( m_pPrototype != NULL )
	{
		size_t	i, nCount ;
		nCount = m_pPrototype->GetElementCount() ;
		for ( i = 0; i < nCount; i ++ )
		{
			const wchar_t *	pwszName = m_pPrototype->GetElementNameAt( (int) i ) ;
			RSSmartPtr	pObj( m_pPrototype->GetElementAt( context, (int) i ), &context ) ;
			if ( (pwszName != NULL) && (pObj != NULL) )
			{
				SString	strDeclVar ;
				DumpVarDeclaration
					( context, strDeclVar, pwszName, pObj, 0 ) ;
				AddFormatIndentedString( strDecl, strDeclVar, L"\t" ) ;
				strDecl += L" ;\r\n" ;
			}
		}
	}
}

// クラス仮想関数ダンプ
//////////////////////////////////////////////////////////////////////////////
void RSClass::DumpClassVirtualFunctions
	( RSContext& context, SSystem::SString& strDecl )
{
	size_t	i, nCount ;
	nCount = m_gcmVirtuals.GetMemberCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszName = m_gcmVirtuals.GetMemberNameAt( i ) ;
		RSSmartPtr	pObj( m_gcmVirtuals.GetMemberAt( i ), &context ) ;
		if ( (pwszName != NULL) && (pObj != NULL) )
		{
			SString	strDeclVar ;
			DumpVarDeclaration
				( context, strDeclVar, pwszName, pObj, 0 ) ;
			AddFormatIndentedString( strDecl, strDeclVar, L"\t" ) ;
			strDecl += L" ;\r\n" ;
		}
	}
}

// 変数定義ダンプ
//////////////////////////////////////////////////////////////////////////////
void RSClass::DumpVarDeclaration
	( RSContext& context, SSystem::SString& strVar,
		const wchar_t * pwszName, RSObject * pObj, uint32_t accMod )
{
	RSClass *	pClassObj = ESLTypeCast<RSClass>( pObj ) ;
	if ( pClassObj != NULL )
	{
		pClassObj->DumpClassDeclaration( context, strVar ) ;
		return ;
	}
	RSFunctionObject *	pFuncObj = ESLTypeCast<RSFunctionObject>( pObj ) ;
	accMod |= pObj->GetModifiers() ;
	if ( pFuncObj != NULL )
	{
		for ( size_t i = 0; i < pFuncObj->m_arrPrototypes.GetLength(); i ++ )
		{
			RSFunctionPrototype *	pProto = pFuncObj->m_arrPrototypes.GetAt( i ) ;
			if ( pProto == NULL )
			{
				continue ;
			}
			DumpFuncDeclaration( context, strVar, pwszName, pProto, accMod ) ;
		}
	}
	else
	{
		strVar = FormatObjectModifiers( accMod ) ;
		if ( !strVar.IsEmpty() )
		{
			strVar += L" " ;
		}
		RSPointer *	pPtrObj = ESLTypeCast<RSPointer>( pObj ) ;
		if ( (pPtrObj != NULL) && (pPtrObj->m_pPtrClass != NULL) )
		{
			strVar += pPtrObj->m_pPtrClass->GetFullClassName() ;
			strVar += L" " ;
			strVar += pwszName ;
			//
			SString	strInitValue = FormatInitValue( pObj ) ;
			if ( !strInitValue.IsEmpty() )
			{
				strVar += L" = " ;
				strVar += strInitValue ;
			}
		}
		else
		{
			RSClass *	pVarClass = pObj->GetRSClass() ;
			if ( pVarClass != NULL )
			{
				strVar += pVarClass->GetFullClassName() ;
				strVar += L" " ;
				strVar += pwszName ;
				//
				SString	strInitValue = FormatInitValue( pObj ) ;
				if ( !strInitValue.IsEmpty() )
				{
					strVar += L" = " ;
					strVar += strInitValue ;
				}
			}
		}
	}
}

// 関数プロトタイプ
//////////////////////////////////////////////////////////////////////////////
void RSClass::DumpFuncDeclaration
	( RSContext& context, SSystem::SString& strFunc,
		const wchar_t * pwszName,
		RSFunctionPrototype * pProto, uint32_t accMod )
{
	uint32_t	accModProto =
		accMod & ~(RSObject::accessMask | RSObject::modifierConst) ;
	if ( pProto->IsSynchronizedModifier() )
	{
		accModProto |= RSObject::modifierSynchronized ;
	}
	if ( pProto->IsConstantModifier() )
	{
		accModProto |= RSObject::modifierConst ;
	}
	if ( pProto->m_nFlags & RSFunctionPrototype::flagAbstract )
	{
		accModProto |= RSObject::modifierAbstract ;
	}
	switch ( pProto->m_nFlags & RSFunctionPrototype::flagAccessMask )
	{
	case	RSFunctionPrototype::flagAccessPublic:
		accModProto |= RSObject::modifierPublic ;
		break ;
	case	RSFunctionPrototype::flagAccessProtected:
		accModProto |= RSObject::modifierProtected ;
		break ;
	case	RSFunctionPrototype::flagAccessPrivate:
		accModProto |= RSObject::modifierPrivate ;
		break ;
	}
	strFunc += FormatObjectModifiers( accModProto ) ;
	strFunc += L" " ;
	if ( pProto->m_pReturnType != NULL )
	{
		strFunc += pProto->m_pReturnType->GetFullClassName() ;
	}
	else
	{
		strFunc += L"void" ;
	}
	strFunc += L" " ;
	strFunc += pwszName ;
	strFunc += L"( " ;
	strFunc += pProto->FormatArgument() ;
	strFunc += L" )" ;
}

// 初期値ダンプ表示形式
SSystem::SString RSClass::FormatInitValue( RSObject * pObj )
{
	RSPointer *	pPtrObj = ESLTypeCast<RSPointer>( pObj ) ;
	if ( pPtrObj != NULL )
	{
		if ( pPtrObj->m_pRef != NULL )
		{
			if ( pPtrObj->m_pPtrClass != NULL )
			{
				RSObject *	pValue = pPtrObj->m_pRef ;
				if ( pValue->GetBasicType() == RSObject::typeString )
				{
					SString	strValue, strTemp ;
					pValue->AsString( strTemp ) ;
					SStringParser::EncodeCLangString( strValue, strTemp ) ;
					//
					strTemp = L"\"" ;
					strTemp += strValue ;
					strTemp += L"\"" ;
					return	strTemp ;
				}
				else
				{
					RSClass *	pValueClass = pValue->GetRSClass() ;
					if ( pValueClass != NULL )
					{
						SString	strValue = L"new " ;
						strValue += pValueClass->GetFullClassName() ;
						if ( ESLTypeCast<RSGenericArrayClass>( pValueClass ) == NULL )
						{
							strValue += L"()" ;
						}
						return	strValue ;
					}
				}
			}
			else
			{
				return	FormatInitValue( pPtrObj->m_pRef ) ;
			}
		}
		else
		{
			return	SString( L"null" ) ;
		}
	}
	else if ( pObj != NULL )
	{
		RSClass *	pVarClass = pObj->GetRSClass() ;
		SString	strValue ;
		if ( (pVarClass != NULL) && pObj->AsString( strValue ) )
		{
			RSInteger *	pIntVar = ESLTypeCast<RSInteger>( pObj ) ;
			if ( pIntVar != NULL )
			{
				int64_t	num ;
				pIntVar->AsInteger( num ) ;
				if ( num < -16 )
				{
					SString	strHexNum ;
					if ( pIntVar->BitSizeOf() <= 32 )
					{
						strHexNum.Format( L" /* 0x%8lx */", num ) ;
					}
					else
					{
						strHexNum.Format( L" /* 0x%lx */", num ) ;
					}
					strValue += strHexNum ;
				}
				else if ( num > 16 )
				{
					SString	strHexNum ;
					if ( num > 0x7FFFFFFF )
					{
						strHexNum.Format( L" /* 0x%lx */", num ) ;
					}
					else if ( num >= 0x10000 )
					{
						strHexNum.Format( L" /* 0x%08lx */", num ) ;
					}
					else
					{
						strHexNum.Format( L" /* 0x%04lx */", num ) ;
					}
					strValue += strHexNum ;
				}
			}
			else if ( pObj->GetBasicType() == RSObject::typeString )
			{
				SString	strTemp ;
				SStringParser::EncodeCLangString( strTemp, strValue ) ;
				//
				strValue = L"\"" ;
				strValue += strTemp ;
				strValue += L"\"" ;
			}
			return	strValue ;
		}
	}
	return	SString() ;
}

// 修飾文字列生成
//////////////////////////////////////////////////////////////////////////////
SSystem::SString RSClass::FormatObjectModifiers( uint32_t accMod )
{
	SString	strMod ;
	switch ( accMod & RSObject::accessMask )
	{
	case	RSObject::modifierPublic:
	default:
		strMod = L"public" ;
		break ;
	case	RSObject::modifierProtected:
		strMod = L"protected" ;
		break ;
	case	RSObject::modifierPrivate:
		strMod = L"private" ;
		break ;
	}
	if ( accMod & RSObject::modifierStatic )
	{
		strMod += L" static" ;
	}
	if ( accMod & RSObject::modifierAbstract )
	{
		strMod += L" abstract" ;
	}
	if ( accMod & RSObject::modifierConst )
	{
		strMod += L" const" ;
	}
	if ( accMod & RSObject::modifierNative )
	{
		strMod += L" native" ;
	}
	if ( accMod & RSObject::modifierSynchronized )
	{
		strMod += L" synchronized" ;
	}
	return	strMod ;
}

// 文字列インデント追加
//////////////////////////////////////////////////////////////////////////////
void RSClass::AddFormatIndentedString
	( SSystem::SString& strDst,
		SSystem::SString& strSrc, const wchar_t * pwszIndent )
{
	size_t	iLast = 0 ;
	for ( ; ; )
	{
		strDst += pwszIndent ;
		//
		ssize_t	iEndOfLine = strSrc.Find( L'\n', iLast ) ;
		if ( iEndOfLine < 0 )
		{
			strDst += strSrc.Middle( iLast ) ;
			break ;
		}
		strDst += strSrc.Middle
					( iLast, (ssize_t) (iEndOfLine - iLast + 1) ) ;
		iLast = (size_t) (iEndOfLine + 1) ;
	}
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSClass::GetTypeName( void ) const
{
	return	m_strClassName ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSClass::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"Class" ) == 0 )
	{
		return	this ;
	}
	return	RSNamespace::InstanceOf( pwszType ) ;
}

RSObject * RSClass::InstanceOf( RSClass * pClass )
{
	return	RSNamespace::InstanceOf( pClass ) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSClass::DisposeObject( RSContext& context )
{
	RSNamespace::DisposeObject( context ) ;
	//
	m_gcmVirtuals.DisposeAllMembers( context ) ;
//	m_gcmVirtuals.RemoveAllMembers() ;
	//
	RSObject::ReleaseRef( m_pPrototype ) ;
	RSObject::ReleaseRef( m_pConstructor ) ;
	RSObject::ReleaseRef( m_pDestructor ) ;
	m_pPrototype = NULL ;
	m_pConstructor = NULL ;
	m_pDestructor = NULL ;
	//
	for ( size_t i = 0; i < m_lstImplements.GetLength(); i ++ )
	{
		RSClass *	pClass = m_lstImplements.GetAt( i ) ;
		ESLAssert( pClass != NULL ) ;
		pClass->ReleaseRef() ;
	}
	m_lstImplements.RemoveAll() ;
	//
	RSObject::ReleaseRef( m_pSuperClass ) ;
	m_pSuperClass = NULL ;
	//
	RSObject::ReleaseRef( m_pArrayClass ) ;
	m_pArrayClass = NULL ;
	RSObject::ReleaseRef( m_pHashMapClass ) ;
	m_pHashMapClass = NULL ;
}

// メンバ取得 (static 変数)
//////////////////////////////////////////////////////////////////////////////
RSObject * RSClass::GetMemberAs
	( RSContext& context, const wchar_t * pwszName ) const
{
	RSObject *	pObj = RSNamespace::GetMemberAs( context, pwszName ) ;
	if ( pObj != NULL )
	{
		return	pObj ;
	}
	if ( m_pSuperClass != NULL )
	{
		pObj = m_pSuperClass->GetMemberAs( context, pwszName ) ;
		if ( pObj != NULL )
		{
			return	pObj ;
		}
	}
	size_t	nImplements = m_lstImplements.GetLength() ;
	for ( size_t i = 0; i < nImplements; i ++ )
	{
		RSClass *	pInterface = m_lstImplements.GetAt( i ) ;
		if ( pInterface != NULL )
		{
			pObj = pInterface->GetMemberAs( context, pwszName ) ;
			if ( pObj != NULL )
			{
				return	pObj ;
			}
		}
	}
	return	NULL ;
}

// static メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
RSObject * RSClass::CallStaticFunction
	( RSContext& context,
		const wchar_t * pwszFuncName,
		RSObject**const ppArgs, size_t countArg,
		bool fStructCast, bool* pArgMatchResult )
{
	RSSmartPtr	pFuncObj = GetMemberAs( context, pwszFuncName ) ;
	RSFunctionObject *
				pFunc = ESLTypeCast<RSFunctionObject>( pFuncObj.Ptr() ) ;
	if ( pFunc == NULL )
	{
		if ( pArgMatchResult != NULL )
		{
			*pArgMatchResult = false ;
		}
		else
		{
			context.ThrowExceptionError
				( SString(pwszFuncName) + L" 関数が見つかりません" ) ;
		}
		return	NULL ;
	}
	RSObject *	pObjResult =
		context.CallFunction
			( *pFunc, this, ppArgs, countArg, fStructCast, pArgMatchResult ) ;
	return	pObjResult ;
}

// 参照チェーンを解放
//////////////////////////////////////////////////////////////////////////////
void RSClass::ReleaseReferenceChain( void )
{
	size_t	nCount ;
	nCount = m_gcmMembers.GetMemberCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pObj = m_gcmMembers.GetMemberAt( i ) ;
		RSFunctionObject *
				pFuncObj = ESLTypeCast<RSFunctionObject>( pObj ) ;
		if ( pFuncObj != NULL )
		{
			pFuncObj->ReleaseReferenceChain() ;
		}
		RSObject::ReleaseRef( pObj ) ;
	}
	//
	nCount = m_gcmVirtuals.GetMemberCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pObj = m_gcmVirtuals.GetMemberAt( i ) ;
		RSFunctionObject *
				pFuncObj = ESLTypeCast<RSFunctionObject>( pObj ) ;
		if ( pFuncObj != NULL )
		{
			pFuncObj->ReleaseReferenceChain() ;
		}
		RSObject::ReleaseRef( pObj ) ;
	}
	m_gcmVirtuals.RemoveAllMembers() ;
	//
	RSObject::ReleaseRef( m_pPrototype ) ;
	RSObject::ReleaseRef( m_pConstructor ) ;
	RSObject::ReleaseRef( m_pDestructor ) ;
	m_pPrototype = NULL ;
	m_pConstructor = NULL ;
	m_pDestructor = NULL ;
	//
	for ( size_t i = 0; i < m_lstImplements.GetLength(); i ++ )
	{
		RSClass *	pClass = m_lstImplements.GetAt( i ) ;
		ESLAssert( pClass != NULL ) ;
		pClass->ReleaseRef() ;
	}
	m_lstImplements.RemoveAll() ;
	//
	RSObject::ReleaseRef( m_pSuperClass ) ;
	m_pSuperClass = NULL ;
	//
	RSObject::ReleaseRef( m_pRefBackLink ) ;
	RSObject::ReleaseRef( m_pRefNamespace ) ;
	m_pRefBackLink = NULL ;
	m_pRefNamespace = NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// Object クラスオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSGenricObjectClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSGenricObjectClass::RSGenricObjectClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSGenricObjectClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"finalize", NULL, L"",
				NULL, &RSGenricObjectClass::method_finalize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"equals", L"boolean", L"Object obj",
				NULL, &RSGenricObjectClass::method_equals,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"toString", L"String", L"",
				NULL, &RSGenricObjectClass::method_toString,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getClass", L"Class", L"",
				NULL, &RSGenricObjectClass::method_getClass,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"notify", NULL, L"",
				NULL, &RSGenricObjectClass::method_notify, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"notifyAll", NULL, L"",
				NULL, &RSGenricObjectClass::method_notifyAll, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"wait", NULL, L"",
				NULL, &RSGenricObjectClass::method_wait, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"wait", NULL, L"long timeout",
				NULL, &RSGenricObjectClass::method_wait, NULL ) ;
}

// void finalize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenricObjectClass::method_finalize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	NULL ;
}

// boolean equals( Object obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenricObjectClass::method_equals
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( pThis == NULL )
	{
		return	context.new_Boolean
					( (ppArg[0] == NULL)
						|| (ppArg[0]->GetEntityObject() == NULL) ) ;
	}
	return	context.new_Boolean( pThis->IsEqualObject( ppArg[0] ) ) ;
}

// String toString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenricObjectClass::method_toString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( pThis == NULL )
	{
		return	context.new_String( L"null" ) ;
	}
	SString	strTemp ;
	if ( pThis->AsString( strTemp ) )
	{
		return	context.new_String( strTemp ) ;
	}
	context.ThrowExceptionError( L"Object を文字列へ変換できません" ) ;
	return	NULL ;
}

// Class getClass()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenricObjectClass::method_getClass
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSClass *	pClass = pThis->GetRSClass() ;
	if ( pClass == NULL )
	{
		return	NULL ;
	}
	pClass->AddRef() ;
	return	pClass ;
}

// void notify()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenricObjectClass::method_notify
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	pThis->NotifyMonitor( RSObject::Monitor::typeNotification ) ;
	//
	return	NULL ;
}

// void notifyAll()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenricObjectClass::method_notifyAll
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	pThis->NotifyAllMonitor( RSObject::Monitor::typeNotification ) ;
	//
	return	NULL ;
}

// void wait()
// void wait( long timeout )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenricObjectClass::method_wait
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	nTimeout = arg.LongAt( 0 ) ;
	pThis->WaitNotification( context, nTimeout ) ;
	//
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// JavaScript 風 Object クラスオブジェクト (HashMap)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDynamicObjectClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDynamicObjectClass::RSDynamicObjectClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSDynamicObjectClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"Class cls",
				NULL, &RSDynamicObjectClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"keyAt", L"String", L"int index",
				NULL, &RSDynamicObjectClass::method_keyAt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"keys", L"String[]", L"",
				NULL, &RSDynamicObjectClass::method_keys,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"size", L"int", L"",
				NULL, &RSDynamicObjectClass::method_size,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clear", NULL, L"",
				NULL, &RSDynamicObjectClass::method_clear, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"remove", L"Object", L"String key",
				NULL, &RSDynamicObjectClass::method_remove, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"freeze", NULL, L"",
				NULL, &RSDynamicObjectClass::method_freeze, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isEmpty", L"boolean", L"String key",
				NULL, &RSDynamicObjectClass::method_isEmpty,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"get", L"Object", L"String key",
				NULL, &RSDynamicObjectClass::method_get,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"put", L"Object", L"String key, Object obj",
				NULL, &RSDynamicObjectClass::method_put, NULL ) ;
}

// void <init>( Class clsElement )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSDynamicObject *	pObj = ESLTypeCast<RSDynamicObject>( pThis ) ;
	RSClass *	pClass = ESLTypeCast<RSClass>( arg.ObjectAt( 0 ) ) ;
	if ( pObj != NULL )
	{
		pObj->SetMemberClass( pClass ) ;
	}
	return	NULL ;
}

// String keyAt( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_keyAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String( pThis->GetElementNameAt( arg.IntAt(0) ) ) ;
}

// String[] keys()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_keys
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSArray *	pArray =
		ESLTypeCast<RSArray>
			( context.new_Array( 0x7FFFFFFF, context.GetStringClass() ) ) ;
	ESLAssert( pArray != NULL ) ;
	//
	size_t	nCount = pThis->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pArray->m_elements.Add
			( context.new_String( pThis->GetElementNameAt( (int) i ) ) ) ;
	}
	return	pArray ;
}

// int size()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_size
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Integer( pThis->GetElementCount() ) ;
}

// void clear()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_clear
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	pThis->DisposeObject( context ) ;
	return	NULL ;
}

// Object remove( String key )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_remove
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDynamicObject *	pObj = ESLTypeCast<RSDynamicObject>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"HashMap.remove 関数の this が HashMap ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	pObj->RemoveMemberAs( context, arg.StringAt( 0 ) ) ;
}

// void freeze()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_freeze
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDynamicObject *	pHashObj = ESLTypeCast<RSDynamicObject>( pThis ) ;
	if ( pHashObj == NULL )
	{
		context.ThrowExceptionError
			( L"HashMap.freeze 関数の this が HashMap ではありません" ) ;
		return	NULL ;
	}
	pHashObj->SetPropertyMode( RSDynamicObject::modePropReadWrite ) ;
	return	NULL ;
}

// boolean isEmpty( String key )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_isEmpty
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDynamicObject *	pHashObj = ESLTypeCast<RSDynamicObject>( pThis ) ;
	if ( pHashObj == NULL )
	{
		context.ThrowExceptionError
			( L"HashMap.isEmpty 関数の this が HashMap ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSmartPtr	pMember
		( pHashObj->GetProperties().
			GetMemberAs( arg.StringAt( 0 ) ), &context ) ;
	return	context.new_Boolean( pMember.Ptr() == NULL ) ;
}

// Object get( String key )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_get
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDynamicObject *	pHashObj = ESLTypeCast<RSDynamicObject>( pThis ) ;
	if ( pHashObj == NULL )
	{
		context.ThrowExceptionError
			( L"HashMap.get 関数の this が HashMap ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	pHashObj->GetProperties().GetMemberAs( arg.StringAt( 0 ) ) ;
}

// Object put( String key, Object obj )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDynamicObjectClass::method_put
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDynamicObject *	pHashObj = ESLTypeCast<RSDynamicObject>( pThis ) ;
	if ( pHashObj == NULL )
	{
		context.ThrowExceptionError
			( L"HashMap.put 関数の this が HashMap ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString		strKey = arg.StringAt( 0 ) ;
	RSObject *	pObj = arg.ObjectAt( 1 ) ;
	RSObject *	pLastObj = pHashObj->GetProperties().GetMemberAs( strKey ) ;
	RSObject::AddRef( pObj ) ;
	context.ReleaseObjectRef
		( pHashObj->GetProperties().SetMemberAs( context, strKey, pObj ) ) ;
	return	pLastObj ;
}



//////////////////////////////////////////////////////////////////////////////
// HashMap ジェネリック型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSGenericHashMapClass, RSDynamicObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSGenericHashMapClass::RSGenericHashMapClass
	( RSClass * pClass,
		const wchar_t * pwszClassName, RSClass * pElementClass )
	: RSDynamicObjectClass( pClass, pwszClassName ),
		m_pElementClass( pElementClass )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSGenericHashMapClass::OverrideVirtuals( RSContext& context )
{
	RemoveVirtualMemberAs( context, L"remove" ) ;
	RemoveVirtualMemberAs( context, L"get" ) ;
	RemoveVirtualMemberAs( context, L"put" ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSGenericHashMapClass::method_init, NULL ) ;
	/*
	AddVirtualDescriptiveAs
		( context, perr, L"keyAt", L"String", L"int index",
				NULL, &RSDynamicObjectClass::method_keyAt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"keys", L"String[]", L"",
				NULL, &RSDynamicObjectClass::method_keys,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"size", L"int", L"",
				NULL, &RSDynamicObjectClass::method_size,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clear", NULL, L"",
				NULL, &RSDynamicObjectClass::method_clear, NULL ) ;
	*/
	AddVirtualDescriptiveAs
		( context, perr, L"remove",
			m_pElementClass->GetFullClassName(), L"String key",
				NULL, &RSDynamicObjectClass::method_remove, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"get",
			m_pElementClass->GetFullClassName(), L"String key",
				NULL, &RSDynamicObjectClass::method_get,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"put",
			m_pElementClass->GetFullClassName(),
			SString(L"String key, ")
				+ m_pElementClass->GetFullClassName() + L" obj",
				NULL, &RSDynamicObjectClass::method_put, NULL ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSGenericHashMapClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDynamicObject *		pObj = ESLTypeCast<RSDynamicObject>( pThis ) ;
	RSGenericHashMapClass *	pClass =
				ESLTypeCast<RSGenericHashMapClass>( pObj->GetRSClass() ) ;
	if ( (pObj != NULL) && (pClass != NULL) )
	{
		pObj->SetMemberClass( pClass->m_pElementClass ) ;
	}
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// Class メタクラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSClassClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSClassClass::RSClassClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSClassClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getName", L"String", L"",
			NULL, &RSClassClass::method_getName, NULL ) ;
}

// String getName()
//////////////////////////////////////////////////////////////////////////
RSObject * RSClassClass::method_getName
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSClass *	pClass = ESLTypeCast<RSClass>( pThis ) ;
	if ( pClass == NULL )
	{
		context.ThrowExceptionError
			( L"Class.getName 関数の this が Class ではありません" ) ;
	}
	return	context.new_String( pClass->GetRSClassName() ) ;
}



//////////////////////////////////////////////////////////////////////////
// 抽象ポインタ型
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSAbstractPointerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////
RSAbstractPointerClass::RSAbstractPointerClass
	( RSClass * pClass,
		const wchar_t * pwszClassName, RSObject * pRefNamespace )
	: RSClass( pClass, pwszClassName, pRefNamespace )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
RSAbstractPointerClass::~RSAbstractPointerClass( void )
{
}

// クラス型テスト
//////////////////////////////////////////////////////////////////////////
bool RSAbstractPointerClass::IsInstanceOf( const wchar_t * pwszClass ) const
{
	return	true ;
}

bool RSAbstractPointerClass::IsInstanceOf( RSClass * pClass ) const
{
	return	true ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////
RSObject * RSAbstractPointerClass::NewInstance( RSContext& context, RSObject * pArg )
{
	return	context.new_Pointer( NULL ) ;
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////
RSObject * RSAbstractPointerClass::NewVariable( RSContext& context )
{
	return	context.new_Pointer( NULL ) ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
bool RSAbstractPointerClass::TestCastInstance
	( RSObject * pObj, RSClass::CastMethod castMethod )
{
	return	true ;
}

RSObject * RSAbstractPointerClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	RSObject::AddRef( pObj ) ;
	return	pObj ;
}


//////////////////////////////////////////////////////////////////////////////
// Function 型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSFunctionClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSFunctionClass::RSFunctionClass
	( RSClass * pClass,
		const wchar_t * pwszClassName, RSFunctionPrototype * pProto )
	: RSClass( pClass, pwszClassName )
{
	m_pProto = pProto ;
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSFunctionClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"HashMap" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}


//////////////////////////////////////////////////////////////////////////////
// Function ジェネリック型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSGenericFunctionClass, RSFunctionClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSGenericFunctionClass::RSGenericFunctionClass
	( RSClass * pClass,
		const wchar_t * pwszClassName,
		const RSFunctionPrototype & proto )
	: RSFunctionClass( pClass, pwszClassName, NULL )
{
	m_pProtoGen = new RSFunctionPrototype( proto ) ;
	m_pProto = m_pProtoGen ;
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSGenericFunctionClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetFunctionClass() ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// 参照チェーンを解放
//////////////////////////////////////////////////////////////////////////////
void RSGenericFunctionClass::ReleaseReferenceChain( void )
{
	RSFunctionClass::ReleaseReferenceChain() ;

	if ( m_pProtoGen != nullptr )
	{
		m_pProtoGen->ReleaseReferenceChain() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 例外型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSExceptionClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSExceptionClass::RSExceptionClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSExceptionClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String err",
				NULL, &RSExceptionClass::method_init, NULL ) ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSExceptionClass::NewInstance( RSContext& context, RSObject * pArg )
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
			SString	str ;
			if ( pObjArg->AsString( str ) )
			{
				return	context.new_Exception( str ) ;
			}
		}
		context.ThrowExceptionError
			( m_strClassName + L" の初期値が不正です" ) ;
		return	NULL ;
	}
	return	context.new_Exception( NULL ) ;
}

// void <init>( String err )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSExceptionClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSException *	pException = ESLTypeCast<RSException>( pThis ) ;
	if ( pException != NULL )
	{
		RSContext::SArgList	arg( ppArg, count ) ;
		pException->m_strMessage = arg.StringAt(0) ;
	}
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// NativeObject クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RNativeObjectClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RNativeObjectClass::RNativeObjectClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}


// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RNativeObjectClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"dispose", NULL, L"",
			NULL, &RNativeObjectClass::method_dispose, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getNativePointer", L"long", L"",
			NULL, &RNativeObjectClass::method_getNativePointer,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"queryObject", L"NativeObject", L"Class cls",
			NULL, &RNativeObjectClass::method_getNativePointer,
			NULL, RSFunctionPrototype::flagConstant ) ;
}

// void dispose()
//////////////////////////////////////////////////////////////////////////////
RSObject * RNativeObjectClass::method_dispose
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( pThis == NULL )
	{
		return	NULL ;
	}
	RSNativeObject *	pNObj = ESLTypeCast<RSNativeObject>( pThis->GetEntityObject() ) ;
	if ( pThis == NULL )
	{
		return	NULL ;
	}
	pNObj->ReleaseNativeRef() ;
	pNObj->ReleaseAllOwnObjects() ;
	return	NULL ;
}

// long getNativePointer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RNativeObjectClass::method_getNativePointer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( pThis == NULL )
	{
		return	context.new_Integer( 0 ) ;
	}
	RSNativeObject *	pNObj = ESLTypeCast<RSNativeObject>( pThis->GetEntityObject() ) ;
	if ( pThis == NULL )
	{
		return	context.new_Integer( 0 ) ;
	}
	return	context.new_Integer( (int64_t) ((ulong_ptr_t) pNObj->GetObject()) ) ;
}

// NativeObject queryObject( Class cls )
//////////////////////////////////////////////////////////////////////////////
RSObject * RNativeObjectClass::method_queryObject
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( pThis == nullptr )
	{
		return	nullptr ;
	}
	RSNativeObject *	pNObj = ESLTypeCast<RSNativeObject>( pThis->GetEntityObject() ) ;
	if ( pThis == nullptr )
	{
		return	nullptr ;
	}
	SSystem::SObject *	pObj = pNObj->GetObject() ;
	if ( pObj == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSClass *	pClass = ESLTypeCast<RSClass>( arg.ObjectAt(0) ) ;
	if ( pClass == nullptr )
	{
		return	nullptr ;
	}
	if ( !pClass->IsNativeObjectOf( pObj ) )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pObj, pClass ) ;
}


//////////////////////////////////////////////////////////////////////////////
// NativeObject 派生クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RGenericNativeObjectClass, RNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RGenericNativeObjectClass::RGenericNativeObjectClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RNativeObjectClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RGenericNativeObjectClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetVM()->GetNativeObjectClass() ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}



//////////////////////////////////////////////////////////////////////////////
// System.ShellResult クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSystemClass::ShellResultClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSystemClass::ShellResultClass::ShellResultClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSystemClass::ShellResultClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"nFlags", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nExitCode", 0 ) ;
}

// Object <- ShellOpenResult 変換
//////////////////////////////////////////////////////////////////////////////
void RSSystemClass::ShellResultClass::ResultToObject
	( RSContext& context, RSObject * obj,
		const SSystem::ShellOpenResult& sorResult )
{
	obj->SetMemberIntegerAs( context, L"nFlags", sorResult.nFlags ) ;
	obj->SetMemberIntegerAs( context, L"nExitCode", sorResult.nExitCode ) ;
}



//////////////////////////////////////////////////////////////////////////////
// System 型クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSystemClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSystemClass::RSSystemClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSSystemClass::~RSSystemClass( void )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSSystemClass::OverrideVirtuals( RSContext& context )
{
	ShellResultClass *	pResultClass =
		new ShellResultClass( context.GetClassClass(), L"ShellResult" ) ;
	pResultClass->Initialize( context ) ;
	pResultClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"ShellResult", pResultClass ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"shellOpenSync", shellOpenSync, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shellExeSync", shellExeSync, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shellNoConsole", shellNoConsole, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shellResultSuccess", shellResultSuccess, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"shellResultExitCode", shellResultExitCode, modifierConst ) ;
	//
	CreateMemberStringAs
		( context, L"languageSigJapanese",
			g_pwszLanguageSignatures[languageJapanese], modifierConst ) ;
	CreateMemberStringAs
		( context, L"languageSigEnglish",
			g_pwszLanguageSignatures[languageEnglish], modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddFunctionDescriptiveAs
		( context, perr, L"console", L"Console", L"",
			NULL, &RSSystemClass::method_console, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"currentTimeMillis", L"long", L"",
			NULL, &RSSystemClass::method_currentTimeMillis, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"trace", NULL, L"String fmt, ...",
			NULL, &RSSystemClass::method_trace, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"eval", L"Object", L"String expr",
			NULL, &RSSystemClass::method_eval, NULL,
			ImmediateComment(
				L"　現在の名前空間で式を評価します。\n"
				L"　関数のローカル変数も対象になりますが、関数がコンパイルされている場合、"
				L"ローカル変数は評価されません。\n　with 文は有効です。" ) ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"addReadableFilePath",
			L"boolean", L"String path, String id = null",
			NULL, &RSSystemClass::method_addReadableFilePath, NULL,
			ImmediateComment(
				L"<desc>　デフォルトで読み込み可能なファイルパスを追加します。<br/>\n"
				L"　追加されたパスのファイルはディレクトリ無指定で読み込み用に開くことができます。</desc>\n"
				L"<param name=\"path\">追加するファイルパス<br/>\n"
				L"$(CURRENT), $(APPDATA), $(DOCUMENTS) 等のパス用環境変数"
				L"（getEnvironmentVariables で取得できるものとは異なります）"
				L"を含むことができます。<br/>\n"
				L"rosetta コマンドでは $(SCRIPT_DIR) にスクリプトファイルのディレクトリが設定されています。</param>\n"
				L"<param name=\"id\">パスの識別子（省略可）<br/>後で removeReadableFilePath で削除したい場合には必要です。</param>" ) ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"addReadableStorageFile",
			L"boolean", L"String path, String id = null",
			NULL, &RSSystemClass::method_addReadableStorageFile, NULL,
			ImmediateComment(
				L"<desc>　デフォルトで読み込み可能なファイルを格納した書庫ファイルファイルを追加します。<br/>\n"
				L"　追加された書庫ファイル内のファイルは読み込み用ファイルとして開くことが出来るようになります。</desc>\n"
				L"<param name=\"path\">追加する書庫ファイル<br/>\n"
				L"addReadableFilePath とは異なり、パスに環境変数を含むことはできませんが、"
				L"既に addReadableFilePath で追加されたパス上のファイルの場合"
				L"ディレクトリの指定は不要です。</param>\n"
				L"<param name=\"id\">パスの識別子（省略可）<br/>後で removeReadableFilePath で削除したい場合には必要です。</param>" ) ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"removeReadableFilePath",
			L"boolean", L"String id",
			NULL, &RSSystemClass::method_removeReadableFilePath, NULL,
			ImmediateComment(
				L"　addReadableFilePath や addReadableStorageFile で追加されたファイルパスを削除します。" ) ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"setWritableFilePath",
			L"boolean", L"String path",
			NULL, &RSSystemClass::method_setWritableFilePath, NULL,
			ImmediateComment(
				L"<desc>　書き込み可能なディレクトリを設定します。<br/>\n"
				L"　この関数を呼び出すと、書き込み用に開いたファイルは"
				L"設定したディレクトリに限定されます。<br/>\n"
				L"　書き込み可能なディレクトリは読み込み用パスとは異なり１つだけ設定できます。</desc>\n"
				L"<param name=\"path\">設定する書き込み可能ディレクトリ<br/>\n"
				L"$(CURRENT), $(APPDATA), $(DOCUMENTS) 等のパス用環境変数"
				L"（getEnvironmentVariables で取得できるものとは異なります）"
				L"を含むことができます。</param>" ) ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"enableToWriteAllFilePath",
			L"boolean", L"boolean flagEnable",
			NULL, &RSSystemClass::method_enableToWriteAllFilePath, NULL,
			ImmediateComment(
				L"　書き込み用ディレクトリ以外のディレクトリへの書き込みを許可するか設定します。\n"
				L"　rosetta コマンドでは、デフォルトで全ディレクトリへの書き込みは許可されています。\n"
				L"　スクリプトの実行環境によってはデフォルトで禁止されている場合もあることに注意してください。" ) ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"currentLanguageType", L"String", L"",
			NULL, &RSSystemClass::method_currentLanguageType, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"getEnvironmentVariables", L"HashMap<String>", L"",
			NULL, &RSSystemClass::method_getEnvironmentVariables, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"shellOpenFile", L"boolean",
			L"String strURL, String strAppPath = null, "
			L"String strAppPlacement = null, "
			L"int nFlags = 0, System.ShellResult pResult = null",
			NULL, &RSSystemClass::method_shellOpenFile, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"serializeToXML", L"String", L"Object obj",
			NULL, &RSSystemClass::method_serializeToXML, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"serializeObject", L"Uint8Pointer", L"Object obj",
			NULL, &RSSystemClass::method_serializeObject, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"restoreFromXML", L"Object", L"String xml",
			NULL, &RSSystemClass::method_restoreFromXML, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"restoreObject", L"Object", L"Uint8Pointer bin",
			NULL, &RSSystemClass::method_restoreObject, NULL ) ;
}

// static Console console()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_console
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	new RSNativeObject( NULL, context.GetClassAs( L"Console" ) ) ;
}

// static long currentTimeMillis()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_currentTimeMillis
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	DATE_TIME	dtLocal ;
	CurrentLocalDate( dtLocal ) ;
	//
	uint64_t	dayLocal = dtLocal.GetAccumulatedDayCount() ;
	uint64_t	dayUTC = DATE_TIME::GetAccumulatedDayCount( 1970, 1, 1 ) ;
	int64_t		dayCurrent = (int64_t) dayLocal - (int64_t) dayUTC ;
	int64_t		msecCurrent =
					(dayCurrent * (24 * 60 * 60 * 1000))
						+ dtLocal.nHour * (60 * 60 * 1000)
						+ dtLocal.nMinute * (60 * 1000)
						+ dtLocal.nSecond * 1000
						+ dtLocal.nMilliSec
						+ DifferenceInLocalTime() * 1000 ;
	return	context.new_Integer( msecCurrent ) ;
}

// static void trace( String fmt, ... )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_trace
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( count >= 1 )
	{
		RSContext::SArgList	arg( ppArg, count ) ;
		SString				strFormat = arg.StringAt( 0 ) ;
		SString				strDst ;
		context.FormatStringVlist
					( strDst, strFormat, ppArg + 1, count - 1 ) ;
		Trace( "%s", strDst.ToCharArray().GetConstArray() ) ;
	}
	return	NULL ;
}

// static Object eval( String expr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_eval
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.EvaluateExpression( arg.StringAt( 0 ), NULL ) ;
//	return	context.PerformExpression( arg.StringAt( 0 ), NULL, NULL ) ;
}

// static boolean addReadableFilePath( String path, String id = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_addReadableFilePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SEnvironment *	pEnv =
		ESLTypeCast<SEnvironment>( SEnvironmentInterface::GetInstance() ) ;
	if ( pEnv == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt(0) ;
	pEnv->FilterEnvironmentString( strPath ) ;
	//
	SFileOpener *	pOpener = g_defURLOpener.NewOffsetOpener( strPath, L'/' ) ;
	if ( pOpener == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	pEnv->AddFileOpener( pOpener, arg.StringAt(1) );
	//
	return	context.new_Boolean( true ) ;
}

// static void addReadableStorageFile( String path, String id = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_addReadableStorageFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SEnvironment *	pEnv =
		ESLTypeCast<SEnvironment>( SEnvironmentInterface::GetInstance() ) ;
	if ( pEnv == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile
			( arg.StringAt(0), SFileOpener::shareRead ) ;
	if ( pFile == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	ERISA::SGLArchiveFile *	pArcFile = new ERISA::SGLArchiveFile ;
	if ( pArcFile->OpenArchive( pFile, true ) )
	{
		delete	pArcFile ;
		return	context.new_Boolean( false ) ;
	}
	pEnv->AddFileOpener( pArcFile, arg.StringAt(1) );
	//
	return	context.new_Boolean( true ) ;
}

// static void removeReadableFilePath( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_removeReadableFilePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SEnvironment *	pEnv =
		ESLTypeCast<SEnvironment>( SEnvironmentInterface::GetInstance() ) ;
	if ( pEnv == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pEnv->RemoveFileOpener( arg.StringAt(0) ) ;
	return	context.new_Boolean( true ) ;
}

// static void setWritableFilePath( String path )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_setWritableFilePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SEnvironment *	pEnv =
		ESLTypeCast<SEnvironment>( SEnvironmentInterface::GetInstance() ) ;
	if ( pEnv == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strPath = arg.StringAt(0) ;
	pEnv->FilterEnvironmentString( strPath ) ;
	//
	SFileOpener *	pOpener = g_defURLOpener.NewOffsetOpener( strPath, L'/' ) ;
	if ( pOpener == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	pEnv->SetWritableFileOpener( pOpener );
	pEnv->AcceptAllFileForWriting( false );
	//
	return	context.new_Boolean( true ) ;
}

// static void enableToWriteAllFilePath( boolean flagEnable )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_enableToWriteAllFilePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SEnvironment *	pEnv =
		ESLTypeCast<SEnvironment>( SEnvironmentInterface::GetInstance() ) ;
	if ( pEnv == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pEnv->AcceptAllFileForWriting( arg.BooleanAt(0) ) ;
	//
	return	context.new_Boolean( true ) ;
}

// static String currentLanguageType()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_currentLanguageType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_String( g_pwszLanguageSignatures[g_languageTarget] ) ;
}

// static HashMap<String> getEnvironmentVariables()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_getEnvironmentVariables
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SObjectArray<SString>	aVarNames ;
	SGLStdApplication::EnumerateEnvironmentVariableNames( aVarNames ) ;
	//
	RSObject *	pEnvVars = context.new_Object( L"HashMap<String>" ) ;
	for ( size_t i = 0; i < aVarNames.GetLength(); i ++ )
	{
		const SString *	pstrName = aVarNames.GetAt( i ) ;
		ESLAssert( pstrName != NULL ) ;
		if ( pstrName == NULL )
		{
			continue ;
		}
		SString	strVar ;
		SGLStdApplication::GetEnvironmentVariable( *pstrName, strVar ) ;
		//
		pEnvVars->SetMemberStringAs( context, *pstrName, strVar ) ;
	}
	return	pEnvVars ;
}

// static boolean shellOpenFile
//	( String strURL,
//		String strAppPath = null, String strAppPlacement = null,
//		int nFlags = 0, System.ShellResult pResult = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_shellOpenFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strURL = arg.StringAt( 0 ) ;
	SString	strAppPath = arg.StringAt( 1 ) ;
	SString	strAppPlacement = arg.StringAt( 2 ) ;
	//
	ShellOpenResult	sorResult ;
	eslFillMemory( &sorResult, 0, sizeof(ShellOpenResult) ) ;
	//
	SError	err =
		OpenShellFile
			( strURL, shellOpenURI, strAppPath,
				strAppPlacement, (uint32_t) arg.IntAt(3), &sorResult ) ;
	//
	RSObject *	pObjResult = arg.ObjectAt( 4 ) ;
	if ( pObjResult != NULL )
	{
		ShellResultClass::ResultToObject( context, pObjResult, sorResult ) ;
	}
	//
	return	context.new_Boolean( err == errSuccess ) ;
}

// static String serializeToXML( Object obj ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_serializeToXML
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SXMLDocument	xmlDoc ;
	MakeXMLDocumentOfObject( arg.ObjectAt(0), context, xmlDoc ) ;
	//
	RSString *	pstrXML = new RSString( context.GetStringClass(), NULL ) ;
	xmlDoc.FormatDocumentToString( pstrXML->m_strValue ) ;
	return	pstrXML ;
}

// static Uint8Pointer serializeObject( Object obj ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_serializeObject
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SSmartBuffer	sbuf ;
	SaveObjectBinary( arg.ObjectAt(0), context, sbuf ) ;
	//
	size_t	nBytes = (size_t) sbuf.GetLength() ;
	//
	RSArrayBuffer *	pBuf = new RSArrayBuffer( context.GetArrayBufferClass() ) ;
	pBuf->AllocateBuffer( nBytes ) ;
	//
	sbuf.Seek( 0 ) ;
	sbuf.Read( pBuf->m_ptrBuf, nBytes ) ;
	//
	return	context.new_PointerNumber( pBuf, RSReferenceNumber::typeUint8 ) ;
}

// static Object restoreFromXML( String xml ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_restoreFromXML
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SString			strXML = arg.StringAt( 0 ) ;
	SStringParser	sparsXML ;
	SXMLDocument	xmlDoc ;
	sparsXML.AttachString( strXML ) ;
	//
	SStrSortObjectArray<SString>	ssoaDTD ;
	xmlDoc.ParseDocument( sparsXML, ssoaDTD, xmlDoc ) ;
	//
	return	RSObject::RestoreObjectOfXMLDocument( context, xmlDoc ) ;
}

// static Object restoreObject( Uint8Pointer bin ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSystemClass::method_restoreObject
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nBytes ;
	uint8_t *	ptrBuf = arg.PointerAt( 0, &nBytes ) ;
	//
	SMemoryReferenceFile	mrfile ;
	mrfile.AttachMemory( ptrBuf, nBytes ) ;
	//
	return	RSObject::LoadObjectBinary( context, mrfile ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Console 型クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSConsoleClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSConsoleClass::RSConsoleClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSConsoleClass::~RSConsoleClass( void )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSConsoleClass::OverrideVirtuals( RSContext& context )
{
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"flush", NULL, L"",
						NULL, &RSConsoleClass::method_flush, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"printf", L"Console", L"String fmt, ...",
						NULL, &RSConsoleClass::method_printf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readLine", L"String", L"",
						NULL, &RSConsoleClass::method_readLine, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCharEncoding", L"String", L"",
						NULL, &RSConsoleClass::method_getCharEncoding, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCharEncoding", L"boolean", L"String sCharset",
						NULL, &RSConsoleClass::method_setCharEncoding, NULL ) ;
}

// void flush()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSConsoleClass::method_flush
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	context.GetVM()->GetStandardOutput().FlushBuffer() ;
	return	NULL ;
}

// Console printf( String fmt, ... )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSConsoleClass::method_printf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( count < 1 )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString				strFormat = arg.StringAt( 0 ) ;
	SString				strDst ;
	context.FormatStringVlist
				( strDst, strFormat, ppArg + 1, count - 1 ) ;
	//
	SBufferedFile&	bfStdOut = context.GetVM()->GetStandardOutput() ;
	bfStdOut << strDst ;
	bfStdOut.FlushBuffer() ;
	//
	RSObject::AddRef( pThis ) ;
	return	pThis ;
}

// String readLine()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSConsoleClass::method_readLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strLine ;
	context.GetVM()->GetStandardInput().ReadStringLine( strLine ) ;
	return	context.new_String( strLine ) ;
}

// String getCharEncoding()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSConsoleClass::method_getCharEncoding
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strCharset =
				Charset::GetEncodingName
					( context.GetVM()->GetStandardInput().GetCharsetEncoding() ) ;
	return	context.new_String( strCharset ) ;
}

// boolean setCharEncoding( String sCharsetType )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSConsoleClass::method_setCharEncoding
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( count < 1 )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList		arg( ppArg, count ) ;
	SString					strCharset = arg.StringAt( 0 ) ;
	Charset::EncodingType	typeCharset = Charset::GetEncodingType( strCharset ) ;
	if ( typeCharset == Charset::encodingUnknown )
	{
		return	context.new_Boolean( false ) ;
	}
	context.GetVM()->GetStandardInput().SetCharsetEncoding( typeCharset ) ;
	context.GetVM()->GetStandardOutput().SetCharsetEncoding( typeCharset ) ;
	return	context.new_Boolean( true ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Math 型クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMathClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMathClass::RSMathClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSMathClass::~RSMathClass( void )
{
}

// クラス固有仮想関数オーバーライド
//////////////////////////////////////////////////////////////////////////////
void RSMathClass::OverrideVirtuals( RSContext& context )
{
	CreateMemberNumberAs( context, L"E", exp(1.0), modifierConst ) ;
	CreateMemberNumberAs( context, L"PI", PI, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddFunctionDescriptiveAs
		( context, perr, L"abs", L"double", L"double a",
						NULL, &RSMathClass::method_abs, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"acos", L"double", L"double a",
						NULL, &RSMathClass::method_acos, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"asin", L"double", L"double a",
						NULL, &RSMathClass::method_asin, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"atan", L"double", L"double a",
						NULL, &RSMathClass::method_atan, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"atan2", L"double", L"double y, double x",
						NULL, &RSMathClass::method_atan2, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"cos", L"double", L"double a",
						NULL, &RSMathClass::method_cos, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"sin", L"double", L"double a",
						NULL, &RSMathClass::method_sin, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"tan", L"double", L"double a",
						NULL, &RSMathClass::method_tan, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"log", L"double", L"double a",
						NULL, &RSMathClass::method_log, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"log10", L"double", L"double a",
						NULL, &RSMathClass::method_log10, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"max", L"double", L"double a, double b",
						NULL, &RSMathClass::method_max, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"min", L"double", L"double a, double b",
						NULL, &RSMathClass::method_min, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"pow", L"double", L"double a, double b",
						NULL, &RSMathClass::method_pow, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"sqrt", L"double", L"double a",
						NULL, &RSMathClass::method_sqrt, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"floor", L"double", L"double a",
						NULL, &RSMathClass::method_floor, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"rint", L"double", L"double a",
						NULL, &RSMathClass::method_rint, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"round", L"long", L"double a",
						NULL, &RSMathClass::method_round, NULL ) ;
}

// static double abs( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_abs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( fabs( arg.DoubleAt(0) ) ) ;
}

// static double acos( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_acos
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( acos( arg.DoubleAt(0) ) ) ;
}

// static double asin( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_asin
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( asin( arg.DoubleAt(0) ) ) ;
}

// static double atan( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_atan
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( atan( arg.DoubleAt(0) ) ) ;
}

// static double atan2( double y, double x )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_atan2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( atan2( arg.DoubleAt(0), arg.DoubleAt(1) ) ) ;
}

// static double cos( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_cos
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( cos( arg.DoubleAt(0) ) ) ;
}

// static double sin( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_sin
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( sin( arg.DoubleAt(0) ) ) ;
}

// static double tan( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_tan
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( tan( arg.DoubleAt(0) ) ) ;
}

// static double log( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_log
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( log( arg.DoubleAt(0) ) ) ;
}

// static double log10( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_log10
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( log10( arg.DoubleAt(0) ) ) ;
}

// static double max( double a, double b )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_max
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( esl_fmax( arg.DoubleAt(0), arg.DoubleAt(1) ) ) ;
}

// static double min( double a, double b )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_min
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( esl_fmin( arg.DoubleAt(0), arg.DoubleAt(1) ) ) ;
}

// static double pow( double a, double b )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_pow
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( pow( arg.DoubleAt(0), arg.DoubleAt(1) ) ) ;
}

// static double sqrt( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_sqrt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( sqrt( arg.DoubleAt(0) ) ) ;
}

// static double floor( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_floor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number( floor( arg.DoubleAt(0) ) ) ;
}

// static double rint( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_rint
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
				( (double) eslRoundR64ToLInt( arg.DoubleAt(0) ) ) ;
}

// static long round( double a )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMathClass::method_round
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer( eslRoundR64ToLInt( arg.DoubleAt(0) ) ) ;
}

