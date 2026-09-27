

#include <antirrhinum/antirrhinum.h>

using namespace	SSystem ;
using namespace SakuraGL ;
using namespace	Rosetta ;
using namespace	Loquaty ;
using namespace	AntirrhinumGL ;


//////////////////////////////////////////////////////////////////////////////
// フラグ管理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLVariablesProcessor, AGLEpicFuncProcessor )
AGL_IMPLEMENT_EPIC_PROCESSOR( AntirrhinumGL::AGLVariablesProcessor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLVariablesProcessor::AGLVariablesProcessor( void )
	: AGLEpicFuncProcessor( m_pFirstFuncDesc, L"variables" ),
		m_pVM( NULL ), m_strReadElement( L".read" ), m_strDeedElement( L".deed" )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLVariablesProcessor::~AGLVariablesProcessor( void )
{
}

// Rosetta 仮想マシン関連付け
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::AttachRosettaVM( Rosetta::RSVirtualMachine * pVM )
{
	m_pVM = pVM ;
	m_context.SetRosetta( new RSContext( pVM ) ) ;
}

// フラグルート設定
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::SetGameFlagsRoot( const AGLScriptObject& pFlags )
{
	m_pGameFlags = pFlags ;
}

void AGLVariablesProcessor::SetSharedFlagsRoot( const AGLScriptObject& pFlags )
{
	m_pSharedFlags = pFlags ;
}

void AGLVariablesProcessor::SetSharedReadFlags( const AGLScriptObject& pFlags )
{
	m_pReadFlags = pFlags ;
	DeerializeReadFlags() ;
}

void AGLVariablesProcessor::SetSharedDeedFlags( const AGLScriptObject& pFlags )
{
	m_pDeedFlags = pFlags ;
}

void AGLVariablesProcessor::RefSharedReadFlags( void )
{
	AGLScriptObject	pReadFlags ;
	if ( (m_pSharedFlags != NULL) && !m_strReadElement.IsEmpty() )
	{
		pReadFlags = m_pSharedFlags.GetMemberAs( m_context, m_strReadElement ) ;
		if ( pReadFlags.IsNull() )
		{
			pReadFlags = m_context.new_Map() ;
			m_pSharedFlags.SetMemberAs
					( m_context, m_strReadElement, pReadFlags ) ;
		}
	}
	SetSharedReadFlags( pReadFlags ) ;
}

void AGLVariablesProcessor::RefSharedDeedFlags( void )
{
	AGLScriptObject	pDeedFlags ;
	if ( !m_pSharedFlags.IsNull() && !m_strDeedElement.IsEmpty() )
	{
		pDeedFlags = m_pSharedFlags.GetMemberAs( m_context, m_strDeedElement ) ;
		if ( pDeedFlags.IsNull() )
		{
			pDeedFlags = m_context.new_Map() ;
			m_pSharedFlags.SetMemberAs
					( m_context, m_strDeedElement, pDeedFlags ) ;
		}
	}
	SetSharedDeedFlags( pDeedFlags ) ;
}

// 共有フラグ・シリアライズ
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::SerializeSharedFlags( SSystem::SXMLDocument& xmlTag )
{
	if ( !m_pSharedFlags.IsNull() )
	{
		SerializeReadFlags() ;
		m_pSharedFlags.Serialize( m_context, xmlTag, true ) ;
	}
}

// 共有フラグ・デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::DeserializeSharedFlags( const SSystem::SXMLDocument& xmlTag )
{
	if ( !m_pSharedFlags.IsNull() )
	{
		m_pSharedFlags.Deserialize( m_context, xmlTag, true ) ;
		RefSharedReadFlags() ;
		RefSharedDeedFlags() ;
	}
}

// 既読フラグ
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::SetReadFlag( const wchar_t * pwszScript, size_t iMessage )
{
	ReadHistory *	prh = m_ssoaReadHistory.GetAs( pwszScript ) ;
	if ( prh == NULL )
	{
		if ( m_pReadFlags.IsNull() )
		{
			return ;
		}
		prh = new ReadHistory ;
		prh->m_pBase64 = m_pReadFlags.GetMemberAs( m_context, pwszScript ) ;
		if ( prh->m_pBase64.IsNull() )
		{
			prh->m_pBase64 = m_context.new_String() ;
			m_pReadFlags.SetMemberAs( m_context, pwszScript, prh->m_pBase64 ) ;
		}
		m_ssoaReadHistory.SetAs( pwszScript, prh ) ;
	}
	if ( prh->GetLength() <= iMessage )
	{
		prh->SetLength( iMessage + 1 ) ;
	}
	prh->SetAt( iMessage, true ) ;
	prh->m_flagModified = true ;
}

bool AGLVariablesProcessor::GetReadFlag( const wchar_t * pwszScript, size_t iMessage ) const
{
	ReadHistory *	prh = m_ssoaReadHistory.GetAs( pwszScript ) ;
	if ( prh == NULL )
	{
		return	false ;
	}
	if ( prh->GetLength() <= iMessage )
	{
		return	false ;
	}
	return	prh->GetAt( iMessage ) ;
}

// 既読フラグをシリアライズ
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::SerializeReadFlags( void )
{
	for ( size_t i = 0; i < m_ssoaReadHistory.GetLength(); i ++ )
	{
		ReadHistory *	prh = m_ssoaReadHistory.GetAt( i ) ;
		if ( (prh != NULL)
			&& prh->m_flagModified && !(prh->m_pBase64.IsNull()) )
		{
			SString	strBase64 ;
			SStringParser::EncodeBase64String
				( strBase64, prh->GetConstArray(), (prh->GetLength() + 0x07) / 8 ) ;
			prh->m_pBase64.PutString( strBase64 ) ;
			prh->m_flagModified = false ;
		}
	}
}

// 既読フラグをデシリアライズ
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::DeerializeReadFlags( void )
{
	m_ssoaReadHistory.RemoveAll() ;
	//
	if ( m_pReadFlags.IsNull() )
	{
		return ;
	}
	size_t	nMemberCount = m_pReadFlags.GetElementCount() ;
	for ( size_t i = 0; i < nMemberCount; i ++ )
	{
		SString	strName = m_pReadFlags.GetElementNameAt( i ) ;
		if ( !strName.IsEmpty() )
		{
			ReadHistory *	prh = new ReadHistory ;
			prh->m_pBase64 = m_pReadFlags.GetElementAt( m_context, i ) ;
			if ( prh->m_pBase64.IsNull() )
			{
				prh->m_pBase64 = m_context.new_String() ;
				m_pReadFlags.SetMemberAs
						( m_context, strName, prh->m_pBase64 ) ;
			}
			SArray<uint8_t>	buf ;
			SString			strBase64 = prh->m_pBase64.AsString() ;
			SStringParser::DecodeBase64String( buf, strBase64 ) ;
			//
			prh->SetLength( buf.GetLength() * 8 ) ;
			eslCopyMemory( prh->GetArray(), buf.GetConstArray(), buf.GetLength() ) ;
			prh->FinidhArray() ;
			//
			m_ssoaReadHistory.SetAs( strName, prh ) ;
		}
	}
}

// 功績フラグ
//////////////////////////////////////////////////////////////////////////////
int64_t AGLVariablesProcessor::AddDeedFlag( const wchar_t * pwszDeed )
{
	if ( m_pDeedFlags.IsNull() )
	{
		return	0 ;
	}
	int64_t	num = m_pDeedFlags.GetMemberAs( m_context, pwszDeed ).AsInteger() ;
	if ( num < (int64_t) 0x7FFFFFFFFFFFFFFFL )
	{
		num ++ ;
	}
	m_pDeedFlags.GetMemberAs( m_context, pwszDeed ).PutInteger( num ) ;
	return	num ;
}

int64_t AGLVariablesProcessor::GetDeedFlag( const wchar_t * pwszDeed ) const
{
	if ( m_pDeedFlags.IsNull() )
	{
		return	0 ;
	}
	return	m_pDeedFlags.GetMemberAs( m_context, pwszDeed ).AsInteger() ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLVariablesProcessor::Serialize
		( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	if ( !m_pGameFlags.IsNull() )
	{
		m_pGameFlags.Serialize( m_context, xmlTag, true ) ;
	}
	return	errSuccess ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLVariablesProcessor::Deserialize
		( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	if ( !m_pGameFlags.IsNull() )
	{
		m_pGameFlags.Deserialize( m_context, xmlTag, true ) ;
	}
	return	errSuccess ;
}

// デシリアライズ後の参照解決処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLVariablesProcessor::AfterDeserialize( AGLKernel * pKernel )
{
	return	errSuccess ;
}

// 設定
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::LoadConfiguration
	( const SSystem::SXMLDocument& xmlConfig )
{
	const SXMLDocument *
			pxmlVars = xmlConfig.GetElementTagAs( L"variables" ) ;
	if ( pxmlVars == NULL )
	{
		return ;
	}
	ESLAssert( m_pKernel != NULL ) ;
	const SString *	pstrGameFlags = pxmlVars->GetAttributeAs( L"game_flags" ) ;
	if ( pstrGameFlags != NULL )
	{
		m_pGameFlags = m_pKernel->EvaluateExpression( *pstrGameFlags ) ;
	}
	const SString *	pstrSharedFlags = pxmlVars->GetAttributeAs( L"shared_flags" ) ;
	if ( pstrSharedFlags != NULL )
	{
		m_pSharedFlags = m_pKernel->EvaluateExpression( *pstrSharedFlags ) ;
	}
	if ( m_pSharedFlags != NULL )
	{
		m_strReadElement = pxmlVars->GetAttrStringAs( L"shared_read", L".read" ) ;
		RefSharedReadFlags() ;
		//
		m_strDeedElement = pxmlVars->GetAttrStringAs( L"shared_deed", L".deed" ) ;
		RefSharedDeedFlags() ;
	}
}

// リリース時処理
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::OnReleaseKernel( void )
{
	m_ssoaReadHistory.RemoveAll() ;
	m_pGameFlags.Release() ;
	m_pSharedFlags.Release() ;
	m_pReadFlags.Release() ;
	m_pDeedFlags.Release() ;
	//
	m_context.Release() ;
	m_pVM = nullptr ;
}

// ゲーム開始時処理
//////////////////////////////////////////////////////////////////////////////
void AGLVariablesProcessor::InitializeGame( void )
{
	if ( !m_pGameFlags.IsNull() )
	{
		if ( m_pGameFlags.GetLanguageType() == languageLoquaty )
		{
			LArrayObj *	pArrayObj =
				dynamic_cast<LArrayObj*>( m_pGameFlags.GetLoquatyObj().Ptr() ) ;
			if ( pArrayObj != nullptr )
			{
				pArrayObj->RemoveAll() ;
			}
		}
		else if ( (m_context.GetRosetta() != nullptr)
				&& (m_pGameFlags.GetRosetta() != nullptr) )
		{
			m_context.GetRosetta()->ReleaseObjectRef
				( m_pGameFlags.GetRosetta()->CallMethodAs
					( *(m_context.GetRosetta()), L"clear", nullptr, 0 ) ) ;
			m_context.GetRosetta()->ClearException() ;
		}
	}
}

// コマンド実装
//////////////////////////////////////////////////////////////////////////////
IMPL_ANTIRRHINUM_PROC(AGLVariablesProcessor,label)
{
	AGLModule *		pModule = thread.GetCurrentModule() ;
	const SString *	pstrLabel = code.GetAttributeAs( L"id" ) ;
	if ( (pModule != NULL) && (pstrLabel != NULL) )
	{
		SString	strLabelPath =
			pModule->GetFileTitle() + L"#" + *pstrLabel ;
		AddDeedFlag( strLabelPath ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLVariablesProcessor,add_deed)
{
	const SString *	pstrVarName = code.GetAttributeAs( L"var" ) ;
	if ( (pstrVarName != NULL) && !pstrVarName->IsEmpty() )
	{
		AddDeedFlag( *pstrVarName ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLVariablesProcessor,init_sflag)
{
	const SString *	pstrVarName = code.GetAttributeAs( L"var" ) ;
	const SString *	pstrExpr = code.GetAttributeAs( L"expr" ) ;
	if ( (pstrVarName != NULL) && !pstrVarName->IsEmpty()
		&& (pstrExpr != NULL) && !pstrExpr->IsEmpty()
		&& !m_pSharedFlags.IsNull() )
	{
		AGLScriptObject	objFlag =
			m_pSharedFlags.GetMemberAs( m_context, *pstrVarName ) ;
		if ( objFlag.IsNull() )
		{
			ESLAssert( m_pKernel != nullptr ) ;
			objFlag = m_pKernel->EvaluateExpression
								( *pstrExpr, thread.GetInstance() ) ;
			m_pSharedFlags.SetMemberAs
							( m_context, *pstrVarName, objFlag ) ;
		}
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLVariablesProcessor,set_sflag)
{
	const SString *	pstrVarName = code.GetAttributeAs( L"var" ) ;
	const SString *	pstrExpr = code.GetAttributeAs( L"expr" ) ;
	if ( (pstrVarName != NULL) && !pstrVarName->IsEmpty()
		&& (pstrExpr != NULL) && !pstrExpr->IsEmpty()
		&& !m_pSharedFlags.IsNull() )
	{
		ESLAssert( m_pKernel != nullptr ) ;
		AGLScriptObject	objFlag =
				m_pKernel->EvaluateExpression
						( *pstrExpr, thread.GetInstance() ) ;
		m_pSharedFlags.SetMemberAs
						( m_context, *pstrVarName, objFlag ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLVariablesProcessor,add_sflag)
{
	const SString *	pstrVarName = code.GetAttributeAs( L"var" ) ;
	const SString *	pstrExpr = code.GetAttributeAs( L"expr" ) ;
	if ( (pstrVarName != NULL) && !pstrVarName->IsEmpty()
		&& (pstrExpr != NULL) && !pstrExpr->IsEmpty()
		&& !m_pSharedFlags.IsNull() )
	{
		AGLScriptObject	objFlag =
			m_pSharedFlags.GetMemberAs( m_context, *pstrVarName ) ;
		if ( objFlag.IsNull() )
		{
			objFlag = m_context.new_Integer() ;
			m_pSharedFlags.SetMemberAs( m_context, *pstrVarName, objFlag ) ;
			objFlag = m_pSharedFlags.GetMemberAs( m_context, *pstrVarName ) ;
		}
		if ( !objFlag.IsNull() )
		{
			int64_t	num = objFlag.AsInteger() ;
			num += EvaluateIntExpression( thread, *pstrExpr, 0 ) ;
			objFlag.PutInteger( num ) ;
		}
	}
	return	codeProcessed ;
}

