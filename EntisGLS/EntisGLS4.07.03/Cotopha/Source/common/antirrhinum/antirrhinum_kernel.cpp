

#include <antirrhinum/antirrhinum.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;
using namespace	Loquaty ;
using namespace	AntirrhinumGL ;


//////////////////////////////////////////////////////////////////////////////
// 基底抽象物
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLObject, SGLObject )

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError AGLObject::OnSave( SSystem::SFileInterface& file )
{
	SXMLDocument	xmlTag ;
	xmlTag.SetTag( SString(GetESLClassName()) ) ;
	SError	err = Serialize( xmlTag, GetKernel() ) ;
	//
	uint32_t		nBytes = 0 ;
	SSmartBuffer	sbuf ;
	xmlTag.FormatDocument( sbuf, 0, Charset::encodingUTF8 ) ;
	//
	nBytes = (uint32_t) sbuf.GetLength() ;
	file.Write( &nBytes, sizeof(uint32_t) ) ;
	sbuf.WriteToStream( file, (ssize_t) nBytes ) ;
	//
	return	(SGLError) err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError AGLObject::OnRestore( SSystem::SFileInterface& file )
{
	uint32_t	nBytes = 0 ;
	if ( file.Read( &nBytes, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	SByteBuffer		bbuf ;
	if ( bbuf.ReadFromStream( file, (ssize_t) nBytes ) < nBytes )
	{
		return	sglErrFailed ;
	}
	SStringParser	sparsDoc ;
	sparsDoc.ReadTextFile( bbuf, Charset::encodingUTF8 ) ;
	//
	SXMLDocument					xmlTag ;
	SStrSortObjectArray<SString>	ssoaDTD ;
	SParserErrorTracer				perrTracer ;
	xmlTag.ParseDocument( sparsDoc, ssoaDTD, perrTracer ) ;
	//
	return	(SGLError) Deserialize( xmlTag, GetKernel() ) ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SGLError AGLObject::OnAfterRestore( void )
{
	return	(SGLError) AfterDeserialize( GetKernel() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// コード
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLCode, SXMLDocument )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLCode::AGLCode( void )
	: m_pKernel( nullptr ), m_pfnProc( nullptr ), m_pEpicProc( nullptr )
{
	m_prsElementClass = &ESL_RUNTIME_CLASS(AGLCode) ;
}

// 実行
//////////////////////////////////////////////////////////////////////////////
CodeProcessResult AGLCode::ProcessCode( AGLThread& thread ) const
{
	if ( (m_pfnProc == nullptr) || (thread.GetKernel() != m_pKernel) )
	{
		return	codeUndefined ;
	}
	ESLAssert( m_pEpicProc != nullptr ) ;
	return	(*m_pfnProc)( m_pEpicProc, thread, *this ) ;
}

// 関数設定
//////////////////////////////////////////////////////////////////////////////
void AGLCode::SetEpicProcessor
	( AGLKernel * pKernel, AGLEpicProcessor * pEpicProc, PFUNC_PROCESSOR pfnProc )
{
	m_pKernel = pKernel ;
	m_pEpicProc = pEpicProc ;
	m_pfnProc = pfnProc ;
}

// サブコンテンツ取得
//////////////////////////////////////////////////////////////////////////////
AGLCode * AGLCode::GetElementAt( size_t index ) const
{
	return	ESLTypeCast<AGLCode>( SXMLDocument::GetElementAt( index ) ) ;
}

// SXMLDocument 要素作成
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * AGLCode::new_XMLDocument( void )
{
	return	new AGLCode ;
}



//////////////////////////////////////////////////////////////////////////////
// モジュール
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLModule, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLModule::AGLModule( void )
	: m_nRefCount( 1 ), m_pxmlScript( nullptr ), m_pxmlCode( nullptr )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLModule::~AGLModule( void )
{
}

// 読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLModule::LoadScript( const wchar_t * pwszFilePath )
{
	m_pxmlScript = nullptr ;
	m_pxmlCode = nullptr ;
	m_psoaCodes.RemoveAll() ;
	//
	SError	err = m_xmlDoc.LoadDocument( pwszFilePath, m_xmlDoc ) ;
	if ( err )
	{
		SString	strFilePath = pwszFilePath ;
		if ( !SString(strFilePath.GetFileExtensionPart()).IsEmpty() )
		{
			return	err ;
		}
		strFilePath += L".xmlagl" ;
		//
		m_xmlDoc.RemoveAllContents() ;
		err = m_xmlDoc.LoadDocument( strFilePath, m_xmlDoc ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	ParseScript( pwszFilePath ) ;
}

SSystem::SError AGLModule::ReadScript
	( SSystem::SFileInterface& file, const wchar_t * pwszFilePath )
{
	m_pxmlScript = nullptr ;
	m_pxmlCode = nullptr ;
	m_psoaCodes.RemoveAll() ;
	//
	SError	err = m_xmlDoc.ReadDocument( file, m_xmlDoc ) ;
	if ( err )
	{
		return	err ;
	}
	return	ParseScript( pwszFilePath ) ;
}

SSystem::SError AGLModule::ParseScript( const wchar_t * pwszFilePath )
{
	m_pxmlScript =
		ESLTypeCast<AGLCode>( m_xmlDoc.GetElementTagAs( L"aglx_script" ) ) ;
	if ( m_pxmlScript == nullptr )
	{
		ESLTrace( "not found <xscript> tag.\n" ) ;
		return	errFailed ;
	}
	m_pxmlCode =
		ESLTypeCast<AGLCode>( m_pxmlScript->GetElementTagAs( L"code" ) ) ;
	if ( m_pxmlCode == nullptr )
	{
		ESLTrace( "not found <xscript><code> tag.\n" ) ;
		return	errFailed ;
	}
	m_strFileName = pwszFilePath ;
	m_strFileTitle = m_strFileName.GetFileTitlePart() ;
	//
	BuildCodeArray( m_pxmlCode, nullptr, 0 ) ;
	//
	return	errSuccess ;
}

void AGLModule::BuildCodeArray
	( AGLCode * pxmlCode,
		const AGLModule::CodeArray * pParent, size_t nSubIndex )
{
	CodeArray *	pCodeArray = new CodeArray ;
	pCodeArray->m_pModule = this ;
	pCodeArray->m_pxmlCode = pxmlCode ;
	if ( pParent != nullptr )
	{
		pCodeArray->m_aNestIndex = pParent->m_aNestIndex ;
	}
	pCodeArray->m_aNestIndex.Add( nSubIndex ) ;
	m_psoaCodes.SetAs( pxmlCode, pCodeArray ) ;
	//
	for ( size_t i = 0; i < pxmlCode->GetElementsCount(); i ++ )
	{
		AGLCode *	pxmlTag = pxmlCode->GetElementAt( i ) ;
		if ( pxmlTag == nullptr )
		{
			continue ;
		}
		if ( pxmlTag->GetTag() == L"label" )
		{
			const SString *	pstrLabel = pxmlTag->GetAttributeAs( L"id" ) ;
			if ( pstrLabel != nullptr )
			{
				pCodeArray->m_ssaLabelIndex.Add( *pstrLabel, i ) ;
			}
		}
		if ( pxmlTag->GetElementsCount() >= 1 )
		{
			BuildCodeArray( pxmlTag, pCodeArray, i ) ;
		}
	}
}

// 参照カウンタ
//////////////////////////////////////////////////////////////////////////////
atomic_int_t AGLModule::AddRefCount( void )
{
	return	AtomicAdd( &m_nRefCount, 1 ) ;
}

atomic_int_t AGLModule::ReleaseRefCount( void )
{
	return	AtomicSub( &m_nRefCount, 1 ) ;
}

// コード取得
//////////////////////////////////////////////////////////////////////////////
const AGLCode *
	AGLModule::GetCodeOf( const SSystem::SString * pstrNestIndex ) const
{
	if ( pstrNestIndex == nullptr )
	{
		return	m_pxmlCode ;
	}
	SStringParser	sparsNestIndex ;
	sparsNestIndex.AttachString( *pstrNestIndex ) ;
	//
	const AGLCode *	pxmlCode = m_pxmlCode ;
	sparsNestIndex.NextInteger() ;
	if ( sparsNestIndex.HasToComeChar( L"," ) == L',' )
	{
		while ( sparsNestIndex.PassSpace() && (pxmlCode != nullptr) )
		{
			int	nType = sparsNestIndex.IsNextNumber() ;
			if ( nType == SStringParser::numberInvalid )
			{
				break ;
			}
			size_t	iNest = (size_t) sparsNestIndex.NextInteger( nType ) ;
			pxmlCode = pxmlCode->GetElementAt( iNest ) ;
			//
			if ( sparsNestIndex.HasToComeChar( L"," ) != L',' )
			{
				break ;
			}
		}
	}
	return	pxmlCode ;
}

// CodeArray 取得
//////////////////////////////////////////////////////////////////////////////
const AGLModule::CodeArray * AGLModule::GetCodeArray( const AGLCode * pxmlCode ) const
{
	return	m_psoaCodes.GetAs( pxmlCode ) ;
}

const AGLModule::CodeArray * AGLModule::GetRootCodeArray( void ) const
{
	return	m_psoaCodes.GetAs( m_pxmlCode ) ;
}

// ラベル検索
//////////////////////////////////////////////////////////////////////////////
ssize_t AGLModule::CodeArray::FindLabel( const wchar_t * pwszLabel ) const
{
	size_t *	pIndex = m_ssaLabelIndex.GetAs( pwszLabel ) ;
	if ( pIndex == nullptr )
	{
		return	-1 ;
	}
	return	(ssize_t) *pIndex ;
}



//////////////////////////////////////////////////////////////////////////////
// モジュール・マネージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLModuleManager, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLModuleManager::AGLModuleManager( size_t nCacheCount )
{
	m_nCacheLimit = nCacheCount ;
}

// スクリプト読み込み
//////////////////////////////////////////////////////////////////////////////
AGLModule * AGLModuleManager::LoadScript( const wchar_t * pwszFileName )
{
	AGLModule *	pModule = GetLoadedScript( pwszFileName ) ;
	if ( pModule != nullptr )
	{
		return	pModule ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	SString	strFilePath = pwszFileName ;
	SString	strFileName = strFilePath.GetFileNamePart() ;
	strFileName.MakeLower() ;
	//
	SSmartPointer<SFileInterface>	pFile = OpenScriptFile( strFilePath ) ;
	if ( pFile == nullptr )
	{
		bool	flagNoExt = false ;
		SString	strFileExt = strFileName.GetFileExtensionPart() ;
		if ( strFileExt.IsEmpty() && (strFileName.GetLastAt(0) != L'.') )
		{
			flagNoExt = true ;
			strFilePath += L".xmlagl" ;
		}
		pFile = OpenScriptFile( strFilePath ) ;
		if ( pFile == nullptr )
		{
			return	nullptr ;
		}
	}
	pModule = new AGLModule ;
	if ( pModule->ReadScript( *pFile, strFileName ) )
	{
		delete	pModule ;
		return	nullptr ;
	}
	m_ssoaModules.Add( strFileName, pModule ) ;
	return	pModule ;
}

// スクリプト読み込み（既に読み込み済みのものは破棄する）
//////////////////////////////////////////////////////////////////////////////
AGLModule * AGLModuleManager::ReadScript
	( SSystem::SFileInterface& file, const wchar_t * pwszFileName )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	SString	strFilePath = pwszFileName ;
	SString	strFileName = strFilePath.GetFileNamePart() ;
	strFileName.MakeLower() ;
	//
	AGLModule *	pModule = new AGLModule ;
	if ( pModule->ReadScript( file, strFileName ) )
	{
		delete	pModule ;
		return	nullptr ;
	}
	AGLModule *	pLastModule = m_ssoaModules.GetAs( strFileName ) ;
	if ( pLastModule != nullptr )
	{
		ssize_t	iCache = m_aCacheModules.FindPtr( pLastModule ) ;
		if ( iCache >= 0 )
		{
			m_aCacheModules.RemoveAt( (size_t) iCache ) ;
		}
	}
	m_ssoaModules.SetAs( strFileName, pModule ) ;
	return	pModule ;
}

// 読み込み済みスクリプト取得
//////////////////////////////////////////////////////////////////////////////
AGLModule * AGLModuleManager::GetLoadedScript( const wchar_t * pwszFileName )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	SString	strFilePath = pwszFileName ;
	SString	strFileName = strFilePath.GetFileNamePart() ;
	strFileName.MakeLower() ;
	//
	AGLModule *	pModule = m_ssoaModules.GetAs( strFileName ) ;
	if ( pModule != nullptr )
	{
		ssize_t	iCache = m_aCacheModules.FindPtr( pModule ) ;
		if ( iCache >= 0 )
		{
			m_aCacheModules.RemoveAt( (size_t) iCache ) ;
		}
		pModule->AddRefCount() ;
		return	pModule ;
	}
	return	nullptr ;
}

// スクリプト解放
//////////////////////////////////////////////////////////////////////////////
SError AGLModuleManager::UnloadScript( AGLModule * pModule )
{
	ESLAssert( pModule != nullptr ) ;
	if ( pModule->ReleaseRefCount() <= 0 )
	{
		SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
		//
		ssize_t	iModule = m_ssoaModules.FindPtr( pModule ) ;
		if ( iModule < 0 )
		{
			return	errFailed ;
		}
		ESLAssert( m_aCacheModules.FindPtr( pModule ) < 0 ) ;
		if ( m_aCacheModules.GetLength() >= m_nCacheLimit )
		{
			AGLModule *	pDelayFree = m_aCacheModules.Pop() ;
			if ( (pDelayFree != nullptr)
				&& (pDelayFree->ReleaseRefCount() <= 0) )
			{
				iModule = m_ssoaModules.FindPtr( pDelayFree ) ;
				if ( iModule >= 0 )
				{
					m_ssoaModules.RemoveAt( (size_t) iModule ) ;
				}
			}
		}
		m_aCacheModules.InsertAt( 0, pModule ) ;
	}
	return	errSuccess ;
}


void AGLModuleManager::UnloadAllScript( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_ssoaModules.RemoveAll() ;
	m_aCacheModules.RemoveAll() ;
}

// 参照されていないスクリプトを破棄する
//////////////////////////////////////////////////////////////////////////////
void AGLModuleManager::ClearCacheModules( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aCacheModules.GetLength(); i ++ )
	{
		AGLModule *	pModule = m_aCacheModules.GetAt( i ) ;
		ssize_t	iModule = m_ssoaModules.FindPtr( pModule ) ;
		if ( iModule >= 0 )
		{
			m_ssoaModules.RemoveAt( (size_t) iModule ) ;
		}
	}
	m_aCacheModules.RemoveAll() ;
}

// マネージャーで管理している有効なモジュールか？
//////////////////////////////////////////////////////////////////////////////
bool AGLModuleManager::IsValidModule( AGLModule * pModule ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	return	m_ssoaModules.FindPtr( pModule ) >= 0 ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface *
	AGLModuleManager::OpenScriptFile( const wchar_t * pwszFileName )
{
	return	SFileOpener::DefaultNewOpenFile
					( pwszFileName, SFileOpener::shareRead ) ;
}



//////////////////////////////////////////////////////////////////////////////
//モジュール・ スマートポインタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLModulePtr, SSyncReference )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLModulePtr::AGLModulePtr
		( AGLModule * pModule, AGLModuleManager * pManager )
	: SSyncReference( pModule ), m_refManager( pManager )
{
}

AGLModulePtr::AGLModulePtr( const AGLModulePtr& ptr )
	: SSyncReference( ptr ), m_refManager( ptr.m_refManager )
{
	AGLModule *	pModule = GetModule() ;
	if ( pModule != nullptr )
	{
		pModule->AddRefCount() ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLModulePtr::~AGLModulePtr( void )
{
	Unload() ;
}

// アンロード
//////////////////////////////////////////////////////////////////////////////
void AGLModulePtr::Unload( void )
{
	AGLModule *			pModule = GetModule() ;
	AGLModuleManager *	pManager = GetModuleManager() ;
	if ( pModule != nullptr )
	{
		if ( pManager != nullptr )
		{
			pManager->UnloadScript( pModule ) ;
		}
		SetReference( nullptr ) ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const AGLModulePtr& AGLModulePtr::operator = ( const AGLModulePtr& ptr )
{
	SetReference( ptr.GetModule() ) ;
	m_refManager = ptr.m_refManager ;
	return	*this ;
}



//////////////////////////////////////////////////////////////////////////////
// スクリプト・コンテキスト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLScriptContext::AGLScriptContext( void )
	: m_type( languageInvalid )
{
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void AGLScriptContext::Release( void )
{
	m_type = languageInvalid ;
	m_pContext = nullptr ;
	m_pLTask.Release() ;
}

// RSContext 設定
//////////////////////////////////////////////////////////////////////////////
void AGLScriptContext::SetRosetta( Rosetta::RSContext * pContext )
{
	m_type = languageRosetta ;
	m_pContext = pContext ;
}

// RSContext 取得
//////////////////////////////////////////////////////////////////////////////
Rosetta::RSContext * AGLScriptContext::GetRosetta( void ) const
{
	return	m_pContext ;
}

// LTaskObj 設定
//////////////////////////////////////////////////////////////////////////////
void AGLScriptContext::SetLoquaty( const LPtr<LTaskObj>& pTask )
{
	m_type = languageLoquaty ;
	m_pLTask = pTask ;
}

// LTaskObj 取得
//////////////////////////////////////////////////////////////////////////////
Loquaty::LPtr<Loquaty::LTaskObj> AGLScriptContext::GetLoquaty( void ) const
{
	return	m_pLTask ;
}

// LVirtualMachine 取得
//////////////////////////////////////////////////////////////////////////////
Loquaty::LVirtualMachine * AGLScriptContext::GetLoquatyVM( void ) const
{
	if ( (m_pLTask != nullptr)
		&& (m_pLTask->GetClass() != nullptr) )
	{
		return	&(m_pLTask->GetClass()->VM()) ;
	}
	return	nullptr ;
}

// Integer オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
AGLScriptObject AGLScriptContext::new_Integer( int64_t num ) const
{
	if ( m_type == languageRosetta )
	{
		ESLAssert( m_pContext != nullptr ) ;
		RSSmartPtr	pObj = m_pContext->new_Integer( num ) ;
		return	AGLScriptObject( pObj ) ;
	}
	else if ( m_type == languageLoquaty )
	{
		ESLAssert( m_pLTask != nullptr ) ;
		ESLAssert( m_pLTask->GetClass() != nullptr ) ;
		LObjPtr	pObj
			( new LIntegerObj
				( m_pLTask->GetClass()->VM().GetIntegerObjClass(), num ) ) ;
		return	AGLScriptObject( pObj ) ;
	}
	return	AGLScriptObject() ;
}

// String オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
AGLScriptObject AGLScriptContext::new_String( const wchar_t * str ) const
{
	if ( m_type == languageRosetta )
	{
		ESLAssert( m_pContext != nullptr ) ;
		RSSmartPtr	pObj = m_pContext->new_String( str ) ;
		return	AGLScriptObject( pObj ) ;
	}
	else if ( m_type == languageLoquaty )
	{
		ESLAssert( m_pLTask != nullptr ) ;
		ESLAssert( m_pLTask->GetClass() != nullptr ) ;
		LObjPtr	pObj
			( new LStringObj
				( m_pLTask->GetClass()->VM().GetStringClass(), str ) ) ;
		return	AGLScriptObject( pObj ) ;
	}
	return	AGLScriptObject() ;
}

// Map オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
AGLScriptObject AGLScriptContext::new_Map( void ) const
{
	if ( m_type == languageRosetta )
	{
		ESLAssert( m_pContext != nullptr ) ;
		RSSmartPtr	pObj = m_pContext->new_Object( L"HashMap" ) ;
		return	AGLScriptObject( pObj ) ;
	}
	else if ( m_type == languageLoquaty )
	{
		ESLAssert( m_pLTask != nullptr ) ;
		ESLAssert( m_pLTask->GetClass() != nullptr ) ;
		LObjPtr	pObj
			( new LTaskObj( m_pLTask->GetClass()->VM().GetTaskClass() ) ) ;
		return	AGLScriptObject( pObj ) ;
	}
	return	AGLScriptObject() ;
}



//////////////////////////////////////////////////////////////////////////////
// スクリプト・インスタンス
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLScriptObject::AGLScriptObject( void )
	: m_type( languageInvalid )
{
}

AGLScriptObject::AGLScriptObject( const AGLScriptObject& src )
	: m_type( src.m_type ),
		m_pRosetta( src.m_pRosetta ),
		m_valLoquaty( src.m_valLoquaty )
{
}

AGLScriptObject::AGLScriptObject( const Rosetta::RSSmartPtr& pRosetta )
	: m_type( languageRosetta ), m_pRosetta( pRosetta )
{
}

AGLScriptObject::AGLScriptObject( const Loquaty::LValue& value )
	: m_type( languageLoquaty ), m_valLoquaty( value )
{
}

AGLScriptObject::AGLScriptObject( const Loquaty::LObjPtr& pLoquaty )
	: m_type( languageLoquaty ), m_valLoquaty( pLoquaty )
{
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const AGLScriptObject& AGLScriptObject::operator = ( const AGLScriptObject& src )
{
	m_type = src.m_type ;
	m_pRosetta = src.m_pRosetta ;
	m_valLoquaty = src.m_valLoquaty ;
	return	*this ;
}

const AGLScriptObject& AGLScriptObject::operator = ( const Rosetta::RSSmartPtr& pRosetta )
{
	m_type = languageRosetta ;
	m_pRosetta = pRosetta ;
	m_valLoquaty = LValue() ;
	return	*this ;
}

const AGLScriptObject& AGLScriptObject::operator = ( const Loquaty::LValue& value )
{
	m_type = languageLoquaty ;
	m_pRosetta = nullptr ;
	m_valLoquaty = value ;
	return	*this ;
}

const AGLScriptObject& AGLScriptObject::operator = ( const Loquaty::LObjPtr& pLoquaty )
{
	m_type = languageLoquaty ;
	m_pRosetta = nullptr ;
	m_valLoquaty = LValue( pLoquaty ) ;
	return	*this ;
}

// null 判定
//////////////////////////////////////////////////////////////////////////////
bool AGLScriptObject::IsNull( void ) const
{
	return	(m_pRosetta.GetEntity() == nullptr) && m_valLoquaty.IsNull() ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void AGLScriptObject::Release( void )
{
	m_pRosetta = nullptr ;
	m_valLoquaty = LValue() ;
}

// LObject 取得
//////////////////////////////////////////////////////////////////////////////
const Loquaty::LValue& AGLScriptObject::GetLoquaty( void ) const
{
	return	m_valLoquaty ;
}

const Loquaty::LObjPtr& AGLScriptObject::GetLoquatyObj( void ) const
{
	return	m_valLoquaty.GetObject() ;
}

AGLScriptObject::operator const Loquaty::LObjPtr& ( void ) const
{
	return	m_valLoquaty.GetObject() ;
}

Loquaty::LClass* AGLScriptObject::GetLoquatyClass( void ) const
{
	if ( m_valLoquaty.GetObject() != nullptr )
	{
		return	m_valLoquaty.GetObject()->GetClass() ;
	}
	return	nullptr ;
}

// 値を評価
//////////////////////////////////////////////////////////////////////////////
bool AGLScriptObject::AsBoolean( void ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		return	m_pRosetta->AsBoolean() ;
	}
	return	m_valLoquaty.AsBoolean() ;
}

int64_t AGLScriptObject::AsInteger( void ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		int64_t	num = 0 ;
		if ( m_pRosetta->AsInteger(num) )
		{
			return	num ;
		}
		return	0 ;
	}
	return	m_valLoquaty.AsInteger() ;
}

double AGLScriptObject::AsDouble( void ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		double	num = 0 ;
		if ( m_pRosetta->AsRealNumber(num) )
		{
			return	num ;
		}
		return	0 ;
	}
	return	m_valLoquaty.AsDouble() ;
}

SSystem::SString AGLScriptObject::AsString( void ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		SString	str ;
		if ( m_pRosetta->AsString(str) )
		{
			return	str ;
		}
		return	SString() ;
	}
	LString	str = m_valLoquaty.AsString() ;
	return	SString( str.c_str() ) ;
}

// 値を設定
//////////////////////////////////////////////////////////////////////////////
bool AGLScriptObject::PutInteger( int64_t val )
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		return	(m_pRosetta->SetIntegerAs( val ) == errSuccess) ;
	}
	return	m_valLoquaty.PutInteger( val ) ;
}

bool AGLScriptObject::PutDouble( double val )
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		return	(m_pRosetta->SetNumberAs( val ) == errSuccess) ;
	}
	return	m_valLoquaty.PutDouble( val ) ;
}

bool AGLScriptObject::PutString( const wchar_t * str )
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		return	(m_pRosetta->SetStringAs( str ) == errSuccess) ;
	}
	return	m_valLoquaty.PutString( str ) ;
}

// 要素取得
//////////////////////////////////////////////////////////////////////////////
AGLScriptObject AGLScriptObject::GetMemberAs
	( const AGLScriptContext& context, const wchar_t * pwszName ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		ESLAssert( context.GetRosetta() != nullptr ) ;
		RSSmartPtr	pMember =
			m_pRosetta->GetMemberAs( *(context.GetRosetta()), pwszName ) ;
		return	AGLScriptObject( pMember ) ;
	}
	else if ( m_type == languageLoquaty )
	{
		ESLAssert( context.GetLoquatyVM() != nullptr ) ;
		return	AGLScriptObject
					( m_valLoquaty.GetMemberAs
						( *(context.GetLoquatyVM()), pwszName, true ) ) ;
	}
	return	AGLScriptObject() ;
}

AGLScriptObject AGLScriptObject::GetElementAt
	( const AGLScriptContext& context, size_t index ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		ESLAssert( context.GetRosetta() != nullptr ) ;
		RSSmartPtr	pMember =
			m_pRosetta->GetElementAt( *(context.GetRosetta()), (int) index ) ;
		return	AGLScriptObject( pMember ) ;
	}
	else if ( m_type == languageLoquaty )
	{
		ESLAssert( context.GetLoquatyVM() != nullptr ) ;
		return	AGLScriptObject
					( m_valLoquaty.GetElementAt
						( *(context.GetLoquatyVM()), index, true ) ) ;
	}
	return	AGLScriptObject() ;
}

// 要素数取得
//////////////////////////////////////////////////////////////////////////////
size_t AGLScriptObject::GetElementCount( void ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		return	m_pRosetta->GetElementCount() ;
	}
	else if ( m_type == languageLoquaty )
	{
		return	m_valLoquaty.GetElementCount() ;
	}
	return	0 ;
}

// 要素名取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SString AGLScriptObject::GetElementNameAt( size_t index ) const
{
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		return	SString( m_pRosetta->GetElementNameAt( (int) index ) ) ;
	}
	else if ( m_type == languageLoquaty )
	{
		LString	strName ;
		return	SString( m_valLoquaty.GetElementNameAt( strName, index ) ) ;
	}
	return	SString() ;
}

// 要素設定
//////////////////////////////////////////////////////////////////////////////
void AGLScriptObject::SetMemberAs
	( const AGLScriptContext& context,
		const wchar_t * pwszName, const AGLScriptObject obj )
{
	ESLAssert( (m_type == obj.m_type) || obj.IsNull() ) ;
	if ( (m_type == languageRosetta) && (m_pRosetta != nullptr) )
	{
		ESLAssert( context.GetRosetta() != nullptr ) ;
		RSSmartPtr	pMember =
			m_pRosetta->SetMemberAs
				( *(context.GetRosetta()),
					pwszName, obj.GetRosetta().AddRef() ) ;
		return ;
	}
	else if ( (m_type == languageLoquaty)
					&& (m_valLoquaty.GetObject() != nullptr) )
	{
		LObjPtr	pMember
			( m_valLoquaty.GetObject()->SetElementAs
					( pwszName, obj.GetLoquaty().AddRef() ) ) ;
		return ;
	}
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
void AGLScriptObject::Serialize
	( AGLScriptContext& context,
		SSystem::SXMLDocument& xmlTag, bool flagStatic )
{
	if ( m_type == languageRosetta )
	{
		ESLAssert( context.GetRosetta() != nullptr ) ;
		SXMLDocument *	pxmlObj = new SXMLDocument ;
		RSObject::MakeXMLDocumentOfObject
			( m_pRosetta, *(context.GetRosetta()), *pxmlObj ) ;

		xmlTag.SetAttributeAs( L"type", L"rosetta" ) ;
		xmlTag.SetAttrIntegerAs( L"rosetta", xmlTag.GetElementsCount() ) ;
		xmlTag.AddElement( pxmlObj ) ;
		return ;
	}
	else if ( m_type == languageLoquaty )
	{
		LString	strJSON =
			LObject::ToExpression
				( m_valLoquaty.GetObject().Ptr(), LObject::expressionForJSON ) ;

		xmlTag.SetAttributeAs( L"type", L"loquaty" ) ;
		xmlTag.SetAttributeAs( L"json", strJSON.c_str() ) ;
		return ;
	}
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
void AGLScriptObject::Deserialize
	( AGLScriptContext& context,
		const SSystem::SXMLDocument& xmlTag, bool flagStatic )
{
	if ( m_type == languageRosetta )
	{
		ESLAssert( context.GetRosetta() != nullptr ) ;
		SXMLDocument *	pxmlObj =
			xmlTag.GetElementAt
				( (size_t) xmlTag.GetAttrIntegerAs( L"rosetta" ) ) ;
		if ( pxmlObj != nullptr )
		{
			if ( flagStatic )
			{
				ESLAssert( m_pRosetta != nullptr ) ;
				if ( m_pRosetta != nullptr )
				{
					m_pRosetta->RestoreXMLDocument
							( *(context.GetRosetta()), *pxmlObj ) ;
				}
			}
			else
			{
				*this = RSSmartPtr
							( RSObject::RestoreObjectOfXMLDocument
								( *(context.GetRosetta()), *pxmlObj ) ) ;
			}
		}
		return ;
	}
	else if ( m_type == languageLoquaty )
	{
		ESLAssert( context.GetLoquaty() != nullptr ) ;
		ESLAssert( context.GetLoquaty()->GetClass() != nullptr ) ;
		SString		strJSON = xmlTag.GetAttrStringAs( L"json" ) ;
		LSourceFile	srcJSON( strJSON ) ;
		LValue	valEval =
			LCompiler::EvaluateConstExpr
				( srcJSON, context.GetLoquaty()->GetClass()->VM() ) ;
		if ( flagStatic )
		{
			m_valLoquaty.PutMembers( valEval ) ;
		}
		else
		{
			m_valLoquaty = valEval ;
		}
		return ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// スレッド
//////////////////////////////////////////////////////////////////////////////

const SXMLDocument::AttrInteger	AGLThread::m_aiStatus[AGLThread::statusCount+1] =
{
	{	L"execute", AGLThread::statusExecute	},
	{	L"suspend", AGLThread::statusSuspend	},
	{	L"halt", AGLThread::statusHalt	},
	{	nullptr, 0	},
} ;

const SSystem::SXMLDocument::AttrInteger	AGLThread::m_aiControlFlags[4] =
{
	{	L"thread", AGLThread::ctrlThread	},
	{	L"called", AGLThread::ctrlCalled	},
	{	L"loop", AGLThread::ctrlLoop	},
	{	nullptr, 0	},
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLThread, AGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLThread::AGLThread
	( AGLModuleManager * pModuleManager, const AGLScriptObject& instance )
		: m_pModuleManager( pModuleManager ),
			m_instance( instance ),
			m_pKernel( nullptr ), m_idThread( 0 ),
			m_nSuspended( 0 ),
			m_maskSkippable( 0xFFFFFFFF ),
			m_msecFrameTimeout( 100 )
{
	m_ip.pCodeArray = nullptr ;
	m_ip.iCode = 0 ;
	m_status = statusExecute ;
	m_nQueuedCode = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLThread::~AGLThread( void )
{
	if ( (m_pModuleManager != nullptr)
		&& (m_ip.pCodeArray != nullptr)
		&& (m_ip.pCodeArray->m_pModule != nullptr) )
	{
		m_pModuleManager->UnloadScript( m_ip.pCodeArray->m_pModule ) ;
	}
}

// Module, ModuleManager 解除
// ※デストラクタでの UnloadScript 呼び出し抑制
//////////////////////////////////////////////////////////////////////////////
void AGLThread::DetachModule( void )
{
	if ( (m_pModuleManager != nullptr)
		&& (m_ip.pCodeArray != nullptr)
		&& (m_ip.pCodeArray->m_pModule != nullptr) )
	{
		m_pModuleManager->UnloadScript( m_ip.pCodeArray->m_pModule ) ;
	}
	m_pModuleManager = nullptr ;
}

// ModuleManager 取得
//////////////////////////////////////////////////////////////////////////////
AGLModuleManager * AGLThread::GetModuleManager( void )
{
	return	m_pModuleManager ;
}

// 現在のコード配列を取得する
//////////////////////////////////////////////////////////////////////////////
const AGLModule::CodeArray * AGLThread::GetCurrentCodeArray( void ) const
{
	return	m_ip.pCodeArray ;
}

// 現在のスクリプトモジュールを取得する
//////////////////////////////////////////////////////////////////////////////
AGLModule * AGLThread::GetCurrentModule( void ) const
{
	if ( m_ip.pCodeArray != nullptr )
	{
		return	m_ip.pCodeArray->m_pModule ;
	}
	return	nullptr ;
}

const wchar_t * AGLThread::GetCurrentModuleFileTitle( void ) const
{
	AGLModule *	pModule = GetCurrentModule() ;
	if ( pModule != nullptr )
	{
		return	pModule->GetFileTitle() ;
	}
	return	nullptr ;
}

const wchar_t * AGLThread::GetCurrentModuleFile( void ) const
{
	AGLModule *	pModule = GetCurrentModule() ;
	if ( pModule != nullptr )
	{
		return	pModule->GetFileName() ;
	}
	return	nullptr ;
}

// インスタンス
//////////////////////////////////////////////////////////////////////////////
const AGLScriptObject& AGLThread::GetInstance( void ) const
{
	return	m_instance ;
}

// スレッドID取得
//////////////////////////////////////////////////////////////////////////////
uint32_t AGLThread::GetThreadID( void ) const
{
	return	m_idThread ;
}

// スレッド名取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& AGLThread::GetThreadName( void ) const
{
	return	m_strName ;
}

// スレッド名設定
//////////////////////////////////////////////////////////////////////////////
void AGLThread::SetThreadName( const wchar_t * pwszName )
{
	m_strName = pwszName ;
}

// AGLKernel 取得
//////////////////////////////////////////////////////////////////////////////
AGLKernel * AGLThread::GetKernel( void ) const
{
	return	m_pKernel ;
}

// ステータスを取得する
//////////////////////////////////////////////////////////////////////////////
AGLThread::Status AGLThread::GetStatus( void ) const
{
	return	m_status ;
}

// 一時停止
//////////////////////////////////////////////////////////////////////////////
void AGLThread::Suspend( void )
{
	if ( AtomicAdd( &m_nSuspended, 1 ) == 1 )
	{
		m_status = statusSuspend ;
		m_timer.Freeze() ;
	}
}

// 再開
//////////////////////////////////////////////////////////////////////////////
void AGLThread::Resume( void )
{
	if ( AtomicSub( &m_nSuspended, 1 ) <= 0 )
	{
		if ( m_status == statusSuspend )
		{
			m_status = statusExecute ;
			m_timer.Restart() ;
		}
	}
}

// シリアライズ可能状態か？
//////////////////////////////////////////////////////////////////////////////
bool AGLThread::CanSerialize( void ) const
{
	return	(m_pLTask == nullptr)
			|| !m_pLTask->IsRunning() || m_pLTask->IsFinished() ;
}

// スレッド実行時間 [ms]
//////////////////////////////////////////////////////////////////////////////
int64_t AGLThread::GetThreadTick( void ) const
{
	return	m_timer.GetTime() ;
}

// スレッド実行フレームタイムアウト時間 [ms]
//////////////////////////////////////////////////////////////////////////////
int64_t AGLThread::GetFrameTimeot( void ) const
{
	return	m_msecFrameTimeout ;
}

void AGLThread::SetFrameTimeout( int64_t msecTimeout )
{
	m_msecFrameTimeout = msecTimeout ;
}

// 次のコードを取得する
//////////////////////////////////////////////////////////////////////////////
AGLCode * AGLThread::GetNextCode( void )
{
	if ( m_status != statusExecute )
	{
		return	nullptr ;
	}
	if ( m_aMicroCodes.GetLength() > 0 )
	{
		m_nQueuedCode = 1 ;
		return	m_aMicroCodes.GetAt( 0 ) ;
	}
	if ( (m_ip.pCodeArray == nullptr)
		|| (m_ip.pCodeArray->m_pxmlCode == nullptr) )
	{
		return	nullptr ;
	}
	return	m_ip.pCodeArray->m_pxmlCode->GetElementAt( m_ip.iCode ) ;
}

// 命令ポインタを進める
//////////////////////////////////////////////////////////////////////////////
void AGLThread::NextCodeIndex( void )
{
	if ( m_status != statusExecute )
	{
		return ;
	}
	FetchInterruption() ;
	//
	if ( (m_nQueuedCode > 0) && (m_aMicroCodes.GetLength() > 0) )
	{
		m_nQueuedCode -- ;
		m_aMicroCodes.RemoveAt( 0 ) ;
		return ;
	}
	m_ip.iCode ++ ;
	//
	while ( (m_ip.pCodeArray == nullptr)
			|| (m_ip.pCodeArray->m_pxmlCode == nullptr)
			|| (m_ip.pCodeArray->m_pxmlCode->GetElementsCount() <= m_ip.iCode) )
	{
		PopCodeIndex() ;
		//
		if ( m_status == statusHalt )
		{
			break ;
		}
	}
}

// 割り込みでの制御移行
//////////////////////////////////////////////////////////////////////////////
bool AGLThread::FetchInterruption( void )
{
	if ( !(m_ip.nCtrl & ctrlDisableInterrupt)
		&& (m_aInterrupter.GetLength() > 0) )
	{
		Interrupter *	pIntterrupter ;
		m_csSync.Lock() ;
		pIntterrupter = m_aInterrupter.DetachAt( 0 ) ;
		m_csSync.Unlock() ;
		//
		if ( pIntterrupter != nullptr )
		{
			m_aMicroCodes.RemoveAll() ;
			//
			if ( pIntterrupter->m_flagCall )
			{
				PushCodeIndex( 0 ) ;
				if ( JumpCodeScript
					( pIntterrupter->m_strScript,
						pIntterrupter->m_strLabel, ctrlCalled ) )
				{
					PopCodeIndex() ;
				}
			}
			else
			{
				JumpCodeScript
					( pIntterrupter->m_strScript,
						pIntterrupter->m_strLabel, 0 ) ;
			}
			ClearAllLocalStrage() ;
			return	true ;
		}
	}
	return	false ;
}

// 命令ポインタを変更する
//////////////////////////////////////////////////////////////////////////////
SError AGLThread::JumpCodeLabel
	( const wchar_t * pwszLabel, uint32_t nCtrlFlags )
{
	if ( m_ip.pCodeArray == nullptr )
	{
		return	errFailed ;
	}
	ssize_t	iLabel = m_ip.pCodeArray->FindLabel( pwszLabel ) ;
	while ( iLabel < 0 )
	{
		if ( m_stack.GetLength() == 0 )
		{
			return	errFailed ;
		}
		PopCodeIndex() ;
		iLabel = m_ip.pCodeArray->FindLabel( pwszLabel ) ;
	}
	m_ip.iCode = (size_t) iLabel ;
	m_ip.nCtrl |= nCtrlFlags ;
	return	errSuccess ;
}

// 現在の命令ポインタをスタックにプッシュする
//////////////////////////////////////////////////////////////////////////////
SError AGLThread::PushCodeIndex( size_t iCodeOffset )
{
	if ( m_ip.pCodeArray == nullptr )
	{
		return	errFailed ;
	}
	if ( m_ip.pCodeArray->m_pModule != nullptr )
	{
		m_ip.pCodeArray->m_pModule->AddRefCount() ;
	}
	m_ip.iCode += iCodeOffset ;
	//
	m_stack.Push( m_ip ) ;
	return	errSuccess ;
}

// 現在の命令ポインタをスタックからポップする
//////////////////////////////////////////////////////////////////////////////
SError AGLThread::PopCodeIndex( void )
{
	if ( (m_pModuleManager != nullptr)
		&& (m_ip.pCodeArray != nullptr) )
	{
		m_pModuleManager->UnloadScript( m_ip.pCodeArray->m_pModule ) ;
	}
	if ( m_stack.GetLength() == 0 )
	{
		m_ip.pCodeArray = nullptr ;
		m_ip.iCode = 0 ;
		m_status = statusHalt ;
		return	errSuccess ;
	}
	m_ip = m_stack.Pop() ;
	return	errSuccess ;
}

SSystem::SError AGLThread::PopCodeIndexUntil( uint32_t nCtrlFlag )
{
	for ( ; ; )
	{
		bool	flagExit = ((m_ip.nCtrl & nCtrlFlag) != 0) ;
		SError	err = PopCodeIndex() ;
		if ( err || flagExit )
		{
			return	err ;
		}
	}
}

// 現在の命令ポインタを設定する
//////////////////////////////////////////////////////////////////////////////
SError AGLThread::JumpCodeIndex
	( const AGLModule::CodeArray * pCodeArray,
					size_t nIndex, uint32_t nCtrlFlags )
{
	ESLAssert( pCodeArray != nullptr ) ;
	ESLAssert( pCodeArray->m_pModule != nullptr ) ;
	pCodeArray->m_pModule->AddRefCount() ;
	m_ip.pCodeArray = pCodeArray ;
	m_ip.iCode = nIndex ;
	m_ip.nCtrl = nCtrlFlags ;
	return	errSuccess ;
}

SError AGLThread::JumpCodeScript
	( const wchar_t * pwszFilePath,
			const wchar_t * pwszLabel, uint32_t nCtrlFlags )
{
	if ( (pwszFilePath == nullptr) || (pwszFilePath[0] == 0) )
	{
		return	JumpCodeLabel( pwszLabel, nCtrlFlags ) ;
	}
	if ( m_pModuleManager == nullptr )
	{
		return	errFailed ;
	}
	AGLModule *	pModule = m_pModuleManager->LoadScript( pwszFilePath ) ;
	if ( pModule == nullptr )
	{
		return	errFailed ;
	}
	const AGLModule::CodeArray * pCodeArray = pModule->GetRootCodeArray() ;
	if ( pCodeArray == nullptr )
	{
		m_pModuleManager->UnloadScript( pModule ) ;
		return	errFailed ;
	}
	ssize_t	nIndex = 0 ;
	if ( (pwszLabel != nullptr) && (pwszLabel[0] != 0) )
	{
		nIndex = pCodeArray->FindLabel( pwszLabel ) ;
		if ( nIndex < 0 )
		{
			ESLTrace( "not found label \'%s\' of script \'%s\'\n",
				SString(pwszLabel).ToCharArray().GetConstArray(),
				SString(pwszFilePath).ToCharArray().GetConstArray() ) ;
			nIndex = 0 ;
		}
	}
	SError	err = JumpCodeIndex( pCodeArray, (size_t) nIndex, nCtrlFlags ) ;
	//
	return	err ;
}

// Loquaty スクリプトを利用するか？
//////////////////////////////////////////////////////////////////////////////
bool AGLThread::IsUsingLoquaty( void ) const
{
	if ( (m_pKernel != nullptr) && (m_pKernel->GetLoquatyVM() != nullptr) )
	{
		if ( m_instance.GetLanguageType() == languageLoquaty )
		{
			return	true ;
		}
		return	(m_pKernel->GetDefaultThisObject().GetLanguageType() != languageInvalid) ;
	}
	return	false ;
}

// Loquaty 式評価コンテキスト取得
//////////////////////////////////////////////////////////////////////////////
LInstantEvaluator&
	AGLThread::GetLoquatyExprEvaluator( const wchar_t * pwszExpr )
{
	ESLAssert( m_pKernel != nullptr ) ;
	ESLAssert( m_pKernel->GetLoquatyVM() != nullptr ) ;

	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;

	LInstantEvaluator*	pEval = m_ssoaExprEvals.GetAs( pwszExpr ) ;
	if ( pEval == nullptr )
	{
		pEval = new LInstantEvaluator( *(m_pKernel->GetLoquatyVM()) ) ;
		m_ssoaExprEvals.SetAs( pwszExpr, pEval ) ;

		if ( !pEval->MakeExpression( pwszExpr, GetLoquatyInstanceClass() ) )
		{
			m_pKernel->OutputTrace( pEval->GetErrorMessages() ) ;
		}
	}
	return	*pEval ;
}

// Loquaty 文評価コンテキスト取得
//////////////////////////////////////////////////////////////////////////////
LInstantEvaluator&
	AGLThread::GetLoquatyStatementsEvaluator( const wchar_t * pwszStatements )
{
	ESLAssert( m_pKernel != nullptr ) ;
	ESLAssert( m_pKernel->GetLoquatyVM() != nullptr ) ;

	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;

	LInstantEvaluator*	pEval = m_psoaEvaluators.GetAs( pwszStatements ) ;
	if ( pEval == nullptr )
	{
		pEval = new LInstantEvaluator( *(m_pKernel->GetLoquatyVM()) ) ;
		m_psoaEvaluators.SetAs( pwszStatements, pEval ) ;

		if ( !pEval->MakeStatement( pwszStatements, GetLoquatyInstanceClass() ) )
		{
			m_pKernel->OutputTrace( pEval->GetErrorMessages() ) ;
		}
	}
	return	*pEval ;
}

// Loquaty インスタンス・クラス
//////////////////////////////////////////////////////////////////////////////
LClass * AGLThread::GetLoquatyInstanceClass( void ) const
{
	if ( m_instance.GetLoquatyObj() != nullptr )
	{
		return	m_instance.GetLoquatyObj()->GetClass() ;
	}
	else if ( (m_pKernel != nullptr)
			&& (m_pKernel->GetDefaultThisObject().GetLoquatyObj() != nullptr) )
	{
		return	m_pKernel->GetDefaultThisObject().GetLoquatyObj()->GetClass() ;
	}
	return	nullptr ;
}

// Loquaty インスタンス取得
//////////////////////////////////////////////////////////////////////////////
const Loquaty::LObjPtr& AGLThread::GetLoquatyInstance( void ) const
{
	if ( m_instance.GetLoquatyObj() != nullptr )
	{
		return	m_instance.GetLoquatyObj() ;
	}
	return	m_pKernel->GetDefaultThisObject().GetLoquatyObj() ;
}

// 実行用 LTask 取得
//////////////////////////////////////////////////////////////////////////////
const Loquaty::LPtr<Loquaty::LTaskObj>& AGLThread::GetLoquatyTask( void )
{
	if ( (m_pLTask == nullptr)
		&& (m_pKernel != nullptr)
		&& (m_pKernel->GetLoquatyVM() != nullptr) )
	{
		m_pLTask = m_pKernel->GetLoquatyVM()->new_Task() ;
	}
	return	m_pLTask ;
}

// 待機命令をスキップ可能か？
//////////////////////////////////////////////////////////////////////////////
bool AGLThread::IsPermittedSkip( SynchronismType type ) const
{
	return	(m_maskSkippable & (1 << type)) != 0 ;
}

// スキップ許可
//////////////////////////////////////////////////////////////////////////////
void AGLThread::PermitSkip( SynchronismType type )
{
	m_maskSkippable |= 1 << type ;
}

void AGLThread::PermitSkipMask( uint32_t mask )
{
	m_maskSkippable |= mask ;
}

// スキップ不許可
//////////////////////////////////////////////////////////////////////////////
void AGLThread::ProhibitSkip( SynchronismType type )
{
	m_maskSkippable &= ~((uint32_t)1 << type) ;
}

void AGLThread::ProhibitSkipMask( uint32_t mask )
{
	m_maskSkippable &= ~mask ;
}

// 遅延ジャンプ（割り込み処理）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLThread::PostInterrupter
	( const wchar_t * pwszFilePath,
		const wchar_t * pwszLabel, bool flagCall )
{
	if ( m_ip.nCtrl & ctrlDisableInterrupt )
	{
		return	errFailed ;
	}
	m_csSync.Lock() ;
	m_aInterrupter.Add( new Interrupter( pwszFilePath, pwszLabel, flagCall ) ) ;
	m_csSync.Unlock() ;
	return	errSuccess ;
}

// 割り込み禁止フラグ設定
//////////////////////////////////////////////////////////////////////////////
void AGLThread::SetDisableInterruptFlag( bool flag )
{
	if ( flag )
	{
		m_ip.nCtrl |= ctrlDisableInterrupt ;
	}
	else
	{
		m_ip.nCtrl &= ~ctrlDisableInterrupt ;
	}
}

// コマンド処理用のローカル記憶域（タグの取得／ない場合は作成）
//////////////////////////////////////////////////////////////////////////////
SSystem::SXMLDocument * AGLThread::GetLocalStrageAs( const wchar_t * pwszTag )
{
	return	m_xmlLocalStorage.CreateElementTagAs( pwszTag ) ;
}

// コマンド処理用のローカル記憶域削除
//////////////////////////////////////////////////////////////////////////////
void AGLThread::ClearLocalStrageAs( const wchar_t * pwszTag )
{
	ssize_t	iTag = m_xmlLocalStorage.FindElementTag( pwszTag ) ;
	if ( iTag >= 0 )
	{
		m_xmlLocalStorage.RemoveElementAt( (size_t) iTag ) ;
	}
}

void AGLThread::ClearLocalStrage( SSystem::SXMLDocument * pxmlStorage )
{
	if ( pxmlStorage == nullptr )
	{
		return ;
	}
	ssize_t	iTag = m_xmlLocalStorage.FindElementTag( pxmlStorage->GetTag() ) ;
	if ( iTag >= 0 )
	{
		ESLAssert( m_xmlLocalStorage.GetElementAt( (size_t) iTag ) == pxmlStorage ) ;
		m_xmlLocalStorage.RemoveElementAt( (size_t) iTag ) ;
	}
}

void AGLThread::ClearAllLocalStrage( void )
{
	m_xmlLocalStorage.RemoveAllElements() ;
}

// マイクロコードを追加する
//////////////////////////////////////////////////////////////////////////////
void AGLThread::AddMicroCode( AGLCode * pCode )
{
	m_aMicroCodes.Add( pCode ) ;
}

void AGLThread::AddMicroCodeTag( const wchar_t * pwszTag )
{
	AGLCode *	pxmlTag = new AGLCode ;
	pxmlTag->SetTag( pwszTag ) ;
	AddMicroCode( pxmlTag ) ;
}

void AGLThread::AddMicroCodeTagParam1
	( const wchar_t * pwszTag,
		const wchar_t * pwszAttr1, const wchar_t * pwszValue1 )
{
	AGLCode *	pxmlTag = new AGLCode ;
	pxmlTag->SetTag( pwszTag ) ;
	pxmlTag->SetAttributeAs( pwszAttr1, pwszValue1 ) ;
	AddMicroCode( pxmlTag ) ;
}

void AGLThread::AddMicroCodeTagParam2
	( const wchar_t * pwszTag,
		const wchar_t * pwszAttr1, const wchar_t * pwszValue1,
		const wchar_t * pwszAttr2, const wchar_t * pwszValue2 )
{
	AGLCode *	pxmlTag = new AGLCode ;
	pxmlTag->SetTag( pwszTag ) ;
	pxmlTag->SetAttributeAs( pwszAttr1, pwszValue1 ) ;
	pxmlTag->SetAttributeAs( pwszAttr2, pwszValue2 ) ;
	AddMicroCode( pxmlTag ) ;
}

void AGLThread::AddMicroCodeTagIntParam1
	( const wchar_t * pwszTag,
		const wchar_t * pwszAttr1, int64_t nValue1 )
{
	AGLCode *	pxmlTag = new AGLCode ;
	pxmlTag->SetTag( pwszTag ) ;
	pxmlTag->SetAttrIntegerAs( pwszAttr1, nValue1 ) ;
	AddMicroCode( pxmlTag ) ;
}

void AGLThread::AddMicroCodeTagIntParam2
	( const wchar_t * pwszTag,
		const wchar_t * pwszAttr1, int64_t nValue1,
		const wchar_t * pwszAttr2, int64_t nValue2 )
{
	AGLCode *	pxmlTag = new AGLCode ;
	pxmlTag->SetTag( pwszTag ) ;
	pxmlTag->SetAttrIntegerAs( pwszAttr1, nValue1 ) ;
	pxmlTag->SetAttrIntegerAs( pwszAttr2, nValue2 ) ;
	AddMicroCode( pxmlTag ) ;
}

// マイクロコード削除
//////////////////////////////////////////////////////////////////////////////
void AGLThread::RemoveAllMicroCode( void )
{
	m_aMicroCodes.RemoveAll() ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLThread::Serialize
	( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	xmlTag.SetTag( L"thread" ) ;
	xmlTag.SetAttrIntegerAs( L"tick", m_timer.GetTime() ) ;
	xmlTag.SetAttrSymbolizedIntegerAs( L"status", m_aiStatus, m_status ) ;
	xmlTag.SetAttrIntegerAs( L"thread_id", m_idThread ) ;
	xmlTag.SetAttrIntegerAs( L"suspended", m_nSuspended ) ;
	xmlTag.SetAttributeAs( L"name", m_strName ) ;
	xmlTag.SetAttrHexIntegerAs( L"skippable", m_maskSkippable ) ;
	xmlTag.SetAttrIntegerAs( L"frame_timeout", m_msecFrameTimeout ) ;
	//
	SXMLDocument *	pxmlStack ;
	pxmlStack = new SXMLDocument ;
	SerializeStack( *pxmlStack, m_ip ) ;
	xmlTag.AddElement( pxmlStack ) ;
	//
	if ( m_aMicroCodes.GetLength() > 0 )
	{
		SXMLDocument *	pxmlMicroCodes = new SXMLDocument ;
		pxmlMicroCodes->SetTag( L"microcodes" ) ;
		xmlTag.AddElement( pxmlMicroCodes ) ;
		//
		pxmlMicroCodes->SetAttrIntegerAs( L"queue_count", m_nQueuedCode ) ;
		//
		for ( size_t i = 0; i < m_aMicroCodes.GetLength(); i ++ )
		{
			AGLCode *	pCode = m_aMicroCodes.GetAt( i ) ;
			if ( pCode != nullptr )
			{
				pxmlMicroCodes->AddElement( new SXMLDocument( *pCode ) );
			}
		}
	}
	//
	for ( size_t i = 0; i < m_stack.GetLength(); i ++ )
	{
		Stack *	pStack = m_stack.GetLastAt( i ) ;
		ESLAssert( pStack != nullptr ) ;
		//
		pxmlStack = new SXMLDocument ;
		SerializeStack( *pxmlStack, *pStack ) ;
		xmlTag.AddElement( pxmlStack ) ;
	}
	//
	if ( !m_instance.IsNull() )
	{
		SXMLDocument *	pxmlInstance ;
		pxmlInstance = new SXMLDocument ;
		pxmlInstance->SetTag( L"instance" ) ;
		xmlTag.AddElement( pxmlInstance ) ;
		//
		if ( m_instance.GetLanguageType() == languageRosetta )
		{
			SXMLDocument *	pxmlObj = new SXMLDocument ;
			RSObject::MakeXMLDocumentOfObject
				( m_instance,
					*(pKernel->GetRosettaVM()->GetSystemContext()), *pxmlObj ) ;
			pxmlInstance->AddElement( pxmlObj ) ;
			pxmlInstance->SetAttributeAs( L"type", L"rosetta" ) ;
		}
		else if ( m_instance.GetLanguageType() == languageLoquaty )
		{
			LObjPtr	pObj = m_instance ;
			LString	strJSON =
				LObject::ToExpression( pObj.Ptr(), LObject::expressionForJSON ) ;

			pxmlInstance->SetAttributeAs( L"type", L"loquaty" ) ;
			pxmlInstance->SetAttributeAs( L"json", strJSON.c_str() ) ;
		}
	}
	//
	m_xmlLocalStorage.SetTag( L"local_storage" ) ;
	xmlTag.AddElement( new SXMLDocument( m_xmlLocalStorage ) ) ;
	return	errSuccess ;
}

void AGLThread::SerializeStack
	( SSystem::SXMLDocument& xmlTag, const AGLThread::Stack& stack )
{
	xmlTag.SetTag( L"stack" ) ;
	xmlTag.SetAttrIntegerAs( L"ip", stack.iCode ) ;
	xmlTag.SetAttrComplexIntegerAs( L"ctrl", m_aiControlFlags, stack.nCtrl ) ;
	//
	if ( stack.pCodeArray != nullptr )
	{
		ESLAssert( stack.pCodeArray->m_pModule != nullptr ) ;
		xmlTag.SetAttributeAs
			( L"module", stack.pCodeArray->m_pModule->GetFileName() ) ;
		//
		SString	strNest ;
		SString	strNum ;
		for ( size_t i = 0; i < stack.pCodeArray->m_aNestIndex.GetLength(); i ++ )
		{
			strNum.FromInteger( stack.pCodeArray->m_aNestIndex.At(i) ) ;
			if ( i > 0 )
			{
				strNest += L"," ;
			}
			strNest += strNum ;
		}
		xmlTag.SetAttributeAs( L"code_nest", strNest ) ;
	}
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLThread::Deserialize
	( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	m_status = (Status) xmlTag.GetAttrSymbolizedIntegerAs
							( L"status", m_aiStatus, m_status ) ;
	m_idThread = (uint32_t) xmlTag.GetAttrIntegerAs( L"thread_id", m_idThread ) ;
	m_nSuspended = (atomic_int_t) xmlTag.GetAttrIntegerAs( L"suspended", m_nSuspended ) ;
	m_strName = xmlTag.GetAttrStringAs( L"name", m_strName ) ;
	m_maskSkippable = (uint32_t) xmlTag.GetAttrHexIntegerAs( L"skippable", m_maskSkippable ) ;
	m_msecFrameTimeout = xmlTag.GetAttrIntegerAs( L"frame_timeout", m_msecFrameTimeout ) ;
	//
	m_timer.Reset( xmlTag.GetAttrIntegerAs( L"tick", m_timer.GetTime() ) ) ;
	//
	bool	flagIP = true ;
	for ( size_t i = 0; i < xmlTag.GetElementsCount(); i ++ )
	{
		const SSystem::SXMLDocument *	pxmlStack = xmlTag.GetElementAt( i ) ;
		if ( pxmlStack == nullptr )
		{
			continue ;
		}
		if ( pxmlStack->GetTag() == L"stack" )
		{
			if ( flagIP )
			{
				DeserializeStack( m_ip, *pxmlStack ) ;
				flagIP = false ;
			}
			else
			{
				Stack	stack ;
				DeserializeStack( stack, *pxmlStack ) ;
				m_stack.InsertAt( 0, stack ) ;
			}
		}
		else if ( pxmlStack->GetTag() == L"instance" )
		{
			SString	strType = pxmlStack->GetAttrStringAs( L"type" ) ;
			if ( (strType == L"loquaty") && (pKernel->GetLoquatyVM() != nullptr) )
			{
				SString		strJSON = pxmlStack->GetAttrStringAs( L"json" ) ;
				LSourceFile	srcJSON( strJSON ) ;
				LValue	valEval =
					LCompiler::EvaluateConstExpr
						( srcJSON, *(pKernel->GetLoquatyVM()) ) ;
				if ( m_instance.GetLoquatyObj() == nullptr )
				{
					m_instance = AGLScriptObject( valEval ) ;
				}
				else
				{
					LValue	valTemp = m_instance.GetLoquaty() ;
					valTemp.PutMembers( valEval ) ;
					m_instance = AGLScriptObject( valTemp ) ;
				}
			}
			else if ( pKernel->GetRosettaVM() != nullptr )
			{
				m_instance =
					RSSmartPtr( RSObject::RestoreObjectOfXMLDocument
						( *(pKernel->GetRosettaVM()->GetSystemContext()), *pxmlStack ) ) ;
			}
		}
		else if ( pxmlStack->GetTag() == L"local_storage" )
		{
			m_xmlLocalStorage = *pxmlStack ;
		}
		else if ( pxmlStack->GetTag() == L"microcodes" )
		{
			m_nQueuedCode =
				(size_t) pxmlStack->GetAttrIntegerAs( L"queue_count", 0 ) ;
			for ( size_t j = 0; j < pxmlStack->GetElementsCount(); j ++ )
			{
				const SSystem::SXMLDocument *
							pxmlCode = pxmlStack->GetElementAt( j ) ;
				if ( pxmlCode != nullptr )
				{
					AGLCode *	pCode = new AGLCode ;
					pCode->CopyAllContentsFrom( *pxmlCode ) ;
					m_aMicroCodes.Add( pCode ) ;
				}
			}
		}
	}
	return	errSuccess ;
}

void AGLThread::DeserializeStack
	( AGLThread::Stack& stack, const SSystem::SXMLDocument& xmlTag )
{
	stack.pCodeArray = nullptr ;
	stack.iCode = (size_t) xmlTag.GetAttrIntegerAs( L"ip", 0 ) ;
	stack.nCtrl = (uint32_t)
		xmlTag.GetAttrComplexIntegerAs( L"ctrl", m_aiControlFlags ) ;
	//
	const SString *	pstrModule = xmlTag.GetAttributeAs( L"module" ) ;
	if ( pstrModule != nullptr )
	{
		AGLModule *	pModule = m_pModuleManager->LoadScript( *pstrModule ) ;
		if ( pModule != nullptr )
		{
			const AGLCode *	pxmlCode =
				pModule->GetCodeOf( xmlTag.GetAttributeAs( L"code_nest" ) ) ;
			const AGLModule::CodeArray *
				pCodeArray = pModule->GetCodeArray( pxmlCode ) ;
			//
			stack.pCodeArray = pCodeArray ;
		}
	}
}

// デシリアライズ後の参照解決処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLThread::AfterDeserialize( AGLKernel * pKernel )
{
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// スクリプト・エンジン・基底抽象物
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLEpicProcessor, AGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLEpicProcessor::AGLEpicProcessor( void )
	: m_pKernel( nullptr )
{
}

// ゲーム開始時処理
//////////////////////////////////////////////////////////////////////////////
void AGLEpicProcessor::InitializeGame( void )
{
}

// ゲーム終了前フェードアウト処理
//////////////////////////////////////////////////////////////////////////////
void AGLEpicProcessor::FadeoutGame( uint32_t msecFadeout )
{
}

// ゲーム終了時処理
//////////////////////////////////////////////////////////////////////////////
void AGLEpicProcessor::ReleaseGame( void )
{
}

// 待機関数を（ユーザー入力等により）即時に脱出すべきか判定する
//////////////////////////////////////////////////////////////////////////////
bool AGLEpicProcessor::ShouldAbortSync( SynchronismType type )
{
	return	false ;
}

// 待機関数を ShouldAbortSync を理由に脱出したことの通知
//////////////////////////////////////////////////////////////////////////////
void AGLEpicProcessor::NotifyAbortedSync( SynchronismType type )
{
}

// フェード処理などの効果継続時間の効果
//////////////////////////////////////////////////////////////////////////////
uint32_t AGLEpicProcessor::EffectTime( uint32_t msecTime, SynchronismType type )
{
	return	msecTime ;
}

// AGLKernel 関連付け
//////////////////////////////////////////////////////////////////////////////
void AGLEpicProcessor::AttachKernel( AGLKernel * pKernel )
{
	m_pKernel = pKernel ;
}

AGLKernel * AGLEpicProcessor::GetKernel( void ) const
{
	return	m_pKernel ;
}

// リリース時処理
//////////////////////////////////////////////////////////////////////////////
void AGLEpicProcessor::OnReleaseKernel( void )
{
}

// 数式評価
//////////////////////////////////////////////////////////////////////////////
SSystem::SString AGLEpicProcessor::EvaluateStringExpression
	( AGLThread& thread,
		const wchar_t * pwszExpr, const wchar_t * pwszDefault )
{
	if ( m_pKernel == nullptr )
	{
		return	pwszDefault ;
	}
	if ( thread.IsUsingLoquaty() )
	{
		LInstantEvaluator&	eval = thread.GetLoquatyExprEvaluator( pwszExpr ) ;
		if ( eval.IsCompiled() )
		{
			return	eval.EvaluateAsString
						( thread.GetLoquatyTask(), thread.GetLoquatyInstance() ) ;
		}
	}
	else
	{
		RSSmartPtr	pObj =
			m_pKernel->EvaluateRosettaExpression( pwszExpr, thread.GetInstance() ) ;
		if ( pObj != nullptr )
		{
			SString	str ;
			if ( pObj->AsString( str ) )
			{
				return	str ;
			}
		}
	}
	return	pwszDefault ;
}

int64_t AGLEpicProcessor::EvaluateIntExpression
	( AGLThread& thread,
			const wchar_t * pwszExpr, int64_t nDefault )
{
	if ( m_pKernel == nullptr )
	{
		return	nDefault ;
	}
	if ( thread.IsUsingLoquaty() )
	{
		LInstantEvaluator&	eval = thread.GetLoquatyExprEvaluator( pwszExpr ) ;
		if ( eval.IsCompiled() )
		{
			return	eval.EvaluateAsLong
						( thread.GetLoquatyTask(), thread.GetLoquatyInstance() ) ;
		}
	}
	else
	{
		RSSmartPtr	pObj =
			m_pKernel->EvaluateRosettaExpression( pwszExpr, thread.GetInstance() ) ;
		if ( pObj != nullptr )
		{
			int64_t	num ;
			if ( pObj->AsInteger( num ) )
			{
				return	num ;
			}
		}
	}
	return	nDefault ;
}

double AGLEpicProcessor::EvaluateNumberExpression
	( AGLThread& thread,
			const wchar_t * pwszExpr, double nDefault )
{
	if ( m_pKernel == nullptr )
	{
		return	nDefault ;
	}
	if ( thread.IsUsingLoquaty() )
	{
		LInstantEvaluator&	eval = thread.GetLoquatyExprEvaluator( pwszExpr ) ;
		if ( eval.IsCompiled() )
		{
			return	eval.EvaluateAsDouble
						( thread.GetLoquatyTask(), thread.GetLoquatyInstance() ) ;
		}
	}
	else
	{
		RSSmartPtr	pObj =
			m_pKernel->EvaluateRosettaExpression( pwszExpr, thread.GetInstance() ) ;
		if ( pObj != nullptr )
		{
			double	num ;
			if ( pObj->AsRealNumber( num ) )
			{
				return	num ;
			}
		}
	}
	return	nDefault ;
}

bool AGLEpicProcessor::EvaluateBoolExpression
	( AGLThread& thread, const wchar_t * pwszExpr, bool bDefault )
{
	if ( m_pKernel == nullptr )
	{
		return	bDefault ;
	}
	if ( thread.IsUsingLoquaty() )
	{
		LInstantEvaluator&	eval = thread.GetLoquatyExprEvaluator( pwszExpr ) ;
		if ( eval.IsCompiled() )
		{
			return	eval.EvaluateAsBool
						( thread.GetLoquatyTask(), thread.GetLoquatyInstance() ) ;
		}
	}
	else
	{
		RSSmartPtr	pObj =
			m_pKernel->EvaluateRosettaExpression( pwszExpr, thread.GetInstance() ) ;
		if ( pObj != nullptr )
		{
			return	pObj->AsBoolean() ;
		}
	}
	return	bDefault ;
}

bool AGLEpicProcessor::EvaluateBoolExpression
	( AGLThread& thread, const AGLCode& code,
				const wchar_t * pwszExpr, bool bDefault )
{
	if ( m_pKernel == nullptr )
	{
		return	bDefault ;
	}
	if ( thread.IsUsingLoquaty() )
	{
		LInstantEvaluator&	eval = thread.GetLoquatyExprEvaluator( pwszExpr ) ;
		if ( eval.IsCompiled() )
		{
			return	eval.EvaluateAsBool
				( thread.GetLoquatyTask(), thread.GetLoquatyInstance() ) ;
		}
	}
	else
	{
		RSScript *	pScript = ESLTypeCast<RSScript>( code.GetApplicationData() ) ;
		if ( pScript == nullptr )
		{
			pScript = new RSScript ;
			const_cast<AGLCode*>(&code)->SetApplicationData( pScript ) ;
			//
			ESLAssert( m_pKernel != nullptr ) ;
			m_pKernel->CompileRosettaStatements( *pScript, pwszExpr ) ;
		}
		RSSmartPtr	pObj =
			m_pKernel->EvaluateRosettaExpression( *pScript, thread.GetInstance() ) ;
		if ( pObj != nullptr )
		{
			return	pObj->AsBoolean() ;
		}
	}
	return	bDefault ;
}

// 数式評価（オブジェクト）
//////////////////////////////////////////////////////////////////////////////
AGLScriptObject AGLEpicProcessor::EvaluateExpression
	( AGLThread& thread, const wchar_t * pwszExpr, bool flagRef )
{
	if ( m_pKernel == nullptr )
	{
		return	AGLScriptObject() ;
	}
	return	m_pKernel->EvaluateExpression( pwszExpr, thread.GetInstance(), flagRef ) ;
}

// 文実行
//////////////////////////////////////////////////////////////////////////////
void AGLEpicProcessor::PerformStatements
	( AGLThread& thread, AGLCode& code, const wchar_t * pwszStatements )
{
	if ( thread.IsUsingLoquaty() )
	{
		LInstantEvaluator&	eval =
				thread.GetLoquatyStatementsEvaluator( pwszStatements ) ;
		if ( eval.IsCompiled() )
		{
			eval.Execute
				( thread.GetLoquatyTask(), thread.GetLoquatyInstance() ) ;

			LObjPtr	pException =
					thread.GetLoquatyTask()->GetUnhandledException() ;
			if ( pException != nullptr )
			{
				m_pKernel->TraceException( pException ) ;
			}
		}
	}
	else
	{
		RSScript *	pScript = ESLTypeCast<RSScript>( code.GetApplicationData() ) ;
		if ( pScript == nullptr )
		{
			pScript = new RSScript ;
			((AGLCode*)&code)->SetApplicationData( pScript ) ;
			//
			ESLAssert( m_pKernel != nullptr ) ;
			m_pKernel->CompileRosettaStatements( *pScript, pwszStatements ) ;
		}
		ESLAssert( m_pKernel != nullptr ) ;
		m_pKernel->PerformRosettaStatements( *pScript, thread.GetInstance() ) ;
	}
}

// 文字列内式展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SString AGLEpicProcessor::EvaluateExprInText
	( AGLThread& thread, const wchar_t * pwszText )
{
	if ( pwszText == nullptr )
	{
		return	SString() ;
	}
	if ( m_pKernel == nullptr )
	{
		return	SString( pwszText ) ;
	}
	return	m_pKernel->EvaluateExprInText( pwszText, thread.GetInstance() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// スクリプト・エンジン・関数プロセッサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLEpicFuncProcessor, AGLEpicProcessor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLEpicFuncProcessor::AGLEpicFuncProcessor
		( const EpicFuncDescriptor * pDesc, const wchar_t * pwszType )
	: m_strTypeID( pwszType )
{
	while ( pDesc != nullptr )
	{
		m_iaCmdMap.Add( new SString( pDesc->pwszCmd ) ) ;
		m_aFuncDesc.Add( pDesc ) ;
		pDesc = pDesc->pDescNext ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLEpicFuncProcessor::~AGLEpicFuncProcessor( void )
{
}

// 識別子
//////////////////////////////////////////////////////////////////////////////
const wchar_t * AGLEpicFuncProcessor::GetObjectType( void ) const
{
	return	m_strTypeID ;
}

// 設定
//////////////////////////////////////////////////////////////////////////////
void AGLEpicFuncProcessor::LoadConfiguration
					( const SSystem::SXMLDocument& xmlConfig )
{
}

		// コード処理関数取得
//////////////////////////////////////////////////////////////////////////////
AGLCode::PFUNC_PROCESSOR
	AGLEpicFuncProcessor::GetCodeProcesser( const wchar_t * pwszTag )
{
	ssize_t	iCmd = m_iaCmdMap.FindIndex( pwszTag ) ;
	if ( iCmd >= 0 )
	{
		const EpicFuncDescriptor *
				pDesc = m_aFuncDesc.GetAt( (size_t) iCmd ) ;
		if ( pDesc != nullptr )
		{
			return	pDesc->pfnProc ;
		}
	}
	return	nullptr ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void AGLEpicFuncProcessor::OnKernelTimer( void )
{
}



//////////////////////////////////////////////////////////////////////////////
// スクリプト・フロー制御プロセッサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLEpicCoreProcessor, AGLEpicFuncProcessor )
AGL_IMPLEMENT_EPIC_PROCESSOR( AntirrhinumGL::AGLEpicCoreProcessor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLEpicCoreProcessor::AGLEpicCoreProcessor( void )
	: AGLEpicFuncProcessor( m_pFirstFuncDesc, L"core" )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLEpicCoreProcessor::~AGLEpicCoreProcessor( void )
{
}

// ブロック内部へ入る
//////////////////////////////////////////////////////////////////////////////
CodeProcessResult AGLEpicCoreProcessor::EnterBlock
	( AGLThread& thread, const AGLCode& code,
			size_t iBreakOffset, uint32_t nCtrlFlags )
{
	const AGLModule::CodeArray *
			pCodeArray = thread.GetCurrentCodeArray() ;
	if ( (pCodeArray == nullptr)
		|| (pCodeArray->m_pModule == nullptr) )
	{
		return	codeProcessed ;
	}
	const AGLModule::CodeArray *
		pBlock = pCodeArray->m_pModule->GetCodeArray( &code ) ;
	if ( pBlock == nullptr )
	{
		if ( iBreakOffset == 0 )
		{
			return	codeControlled ;
		}
		return	codeProcessed ;
	}
	thread.PushCodeIndex( iBreakOffset ) ;
	thread.JumpCodeIndex( pBlock, 0, nCtrlFlags ) ;
	return	codeControlled ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLEpicCoreProcessor::Serialize
		( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	return	errSuccess ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLEpicCoreProcessor::Deserialize
		( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	return	errSuccess ;
}

// デシリアライズ後の参照解決処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLEpicCoreProcessor::AfterDeserialize( AGLKernel * pKernel )
{
	return	errSuccess ;
}

// コマンド実装
//////////////////////////////////////////////////////////////////////////////
IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,jump)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( pstrCondition != nullptr )
	{
		if ( !EvaluateBoolExpression( thread, code, *pstrCondition ) )
		{
			return	codeProcessed ;
		}
	}
	const SString *	pstrLabel = code.GetAttributeAs( L"label" ) ;
	const SString *	pstrModule = code.GetAttributeAs( L"file" ) ;
	if ( pstrModule != nullptr )
	{
		if ( !thread.JumpCodeScript
			( *pstrModule,
				(pstrLabel != nullptr)
					? (const wchar_t*) *pstrLabel : nullptr, 0 ) )
		{
			return	codeControlled ;
		}
	}
	else if ( pstrLabel != nullptr )
	{
		if ( !thread.JumpCodeLabel( *pstrLabel, 0 ) )
		{
			return	codeControlled ;
		}
	}
	SString	strErrMsg ;
	strErrMsg.Format
		( L"failed to .jump: %s#%s\n",
			((pstrModule != nullptr)
				? (const wchar_t*) *pstrModule : L""),
			((pstrLabel != nullptr)
				? (const wchar_t*) *pstrLabel : L"") ) ;
	m_pKernel->OutputTrace( strErrMsg ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,call)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( pstrCondition != nullptr )
	{
		if ( !EvaluateBoolExpression( thread, code, *pstrCondition ) )
		{
			return	codeProcessed ;
		}
	}
	const SString *	pstrLabel = code.GetAttributeAs( L"label" ) ;
	const SString *	pstrModule = code.GetAttributeAs( L"file" ) ;
	if ( pstrModule != nullptr )
	{
		thread.PushCodeIndex() ;
		if ( !thread.JumpCodeScript
			( *pstrModule,
				(pstrLabel != nullptr)
					? (const wchar_t*) *pstrLabel : nullptr,
				AGLThread::ctrlCalled ) )
		{
			return	codeControlled ;
		}
		thread.PopCodeIndex() ;
		return	codeControlled ;
	}
	else if ( pstrLabel != nullptr )
	{
		thread.PushCodeIndex() ;
		if ( !thread.JumpCodeLabel( *pstrLabel, AGLThread::ctrlCalled ) )
		{
			return	codeControlled ;
		}
		thread.PopCodeIndex() ;
		return	codeControlled ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,return)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( pstrCondition != nullptr )
	{
		if ( !EvaluateBoolExpression( thread, code, *pstrCondition ) )
		{
			return	codeProcessed ;
		}
	}
	thread.PopCodeIndexUntil( AGLThread::ctrlThread | AGLThread::ctrlCalled ) ;
	return	codeControlled ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,if)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( (pstrCondition != nullptr)
		&& EvaluateBoolExpression( thread, code, *pstrCondition ) )
	{
		return	EnterBlock( thread, code, 1, 0 ) ;
	}
	for ( ; ; )
	{
		thread.NextCodeIndex() ;
		//
		AGLCode *	pNextCode = thread.GetNextCode() ;
		if ( pNextCode == nullptr )
		{
			break ;
		}
		if ( pNextCode->GetTag() == L"elseif" )
		{
			const SString *	pstrElifCond = pNextCode->GetAttributeAs( L"cond" ) ;
			if ( (pstrElifCond != nullptr)
				&& EvaluateBoolExpression( thread, *pNextCode, *pstrElifCond ) )
			{
				return	EnterBlock( thread, *pNextCode, 1, 0 ) ;
			}
		}
		else if ( pNextCode->GetTag() == L"else" )
		{
			return	EnterBlock( thread, *pNextCode, 1, 0 ) ;
		}
		else
		{
			break ;
		}
	}
	return	codeControlled ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,elseif)
{
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,else)
{
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,while)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( (pstrCondition == nullptr)
		|| EvaluateBoolExpression( thread, code, *pstrCondition ) )
	{
		return	EnterBlock( thread, code, 0, AGLThread::ctrlLoop ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,break)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( (pstrCondition != nullptr)
		&& !EvaluateBoolExpression( thread, code, *pstrCondition ) )
	{
		return	codeProcessed ;
	}
	thread.PopCodeIndexUntil( AGLThread::ctrlLoop ) ;
	thread.NextCodeIndex() ;
	return	codeControlled ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,continue)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( (pstrCondition != nullptr)
		&& !EvaluateBoolExpression( thread, code, *pstrCondition ) )
	{
		return	codeProcessed ;
	}
	thread.PopCodeIndexUntil( AGLThread::ctrlLoop ) ;
	return	codeControlled ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,thread)
{
	const SString *	pstrCondition = code.GetAttributeAs( L"cond" ) ;
	if ( (pstrCondition == nullptr)
		|| EvaluateBoolExpression( thread, code, *pstrCondition ) )
	{
		const SString *	pstrLabel = code.GetAttributeAs( L"label" ) ;
		const SString *	pstrModule = code.GetAttributeAs( L"file" ) ;
		const SString *	pstrName = code.GetAttributeAs( L"name" ) ;
		ESLAssert( m_pKernel != nullptr ) ;
		if ( pstrModule != nullptr )
		{
			AGLThread *	pThread = m_pKernel->BeginThread
				( *pstrModule,
					(pstrLabel != nullptr)
						? (const wchar_t*) *pstrLabel : nullptr,
					thread.GetInstance(),
					(pstrName != nullptr)
						? (const wchar_t*) *pstrName : nullptr ) ;
			if ( pThread != nullptr )
			{
				pThread->ProhibitSkipMask( 0xFFFFFFFF ) ;
			}
		}
		else
		{
			const AGLModule::CodeArray *
					pCodeArray = thread.GetCurrentCodeArray() ;
			if ( (pCodeArray == nullptr)
				|| (pCodeArray->m_pModule == nullptr) )
			{
				return	codeProcessed ;
			}
			const AGLModule::CodeArray *
				pBlock = pCodeArray->m_pModule->GetCodeArray( &code ) ;
			if ( pBlock == nullptr )
			{
				return	codeProcessed ;
			}
			ssize_t	iLabel = 0 ;
			if ( pstrLabel != nullptr )
			{
				ssize_t	iLabel = pBlock->FindLabel( *pstrLabel ) ;
				if ( iLabel < 0 )
				{
					iLabel = 0 ;
				}
			}
			AGLThread *	pThread =
				new AGLThread( thread.GetModuleManager(), thread.GetInstance() ) ;
			if ( pstrName != nullptr )
			{
				pThread->SetThreadName( *pstrName ) ;
			}
			pThread->JumpCodeIndex
				( pBlock, (size_t) iLabel, AGLThread::ctrlThread ) ;
			pThread->ProhibitSkipMask( 0xFFFFFFFF ) ;
			m_pKernel->BeginThread( pThread ) ;
		}
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,terminate)
{
	const SString *	pstrName = code.GetAttributeAs( L"name" ) ;
	if ( pstrName != nullptr )
	{
		ESLAssert( m_pKernel != nullptr ) ;
		m_pKernel->TerminateThread( m_pKernel->GetThreadByName( *pstrName ) ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,eval)
{
	const SString *	pstrSrc = code.GetAttributeAs( L"src" ) ;
	if ( pstrSrc == nullptr )
	{
		return	codeProcessed ;
	}
	if ( thread.IsUsingLoquaty() )
	{
		LInstantEvaluator&	eval =
				thread.GetLoquatyStatementsEvaluator( *pstrSrc ) ;
		if ( eval.IsCompiled() )
		{
			LPtr<LTaskObj>	pTask = thread.GetLoquatyTask() ;
			if ( !pTask->IsRunning() || pTask->IsFinished() )
			{
				if ( eval.BeginAsync( pTask, thread.GetLoquatyInstance() ) )
				{
					LObjPtr	pException = pTask->GetUnhandledException() ;
					if ( pException != nullptr )
					{
						m_pKernel->TraceException( pException ) ;
					}
					return	codeProcessed ;
				}
			}
			if ( pTask->AsyncProceed( 1 ) )
			{
				LObjPtr	pException = pTask->GetUnhandledException() ;
				if ( pException != nullptr )
				{
					m_pKernel->TraceException( pException ) ;
				}
				return	codeProcessed ;
			}
			return	codePending ;
		}
	}
	else
	{
		PerformStatements( thread, *((AGLCode*)&code), *pstrSrc ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,fwait)
{
	thread.NextCodeIndex() ;
	return	codePending ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,wait)
{
	const SString *	pstrTime = code.GetAttributeAs( L"time" ) ;
	if ( pstrTime == nullptr )
	{
		return	codeProcessed ;
	}
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( L"wait" ) ;
	if ( pxmlStorage->GetAttributeAs( L"timeout_tick" ) == nullptr )
	{
		int64_t	nTickTimeout =
					thread.GetThreadTick()
						+ EvaluateIntExpression( thread, *pstrTime ) ;
		pxmlStorage->SetAttrIntegerAs( L"timeout_tick", nTickTimeout ) ;
	}
	if ( thread.IsPermittedSkip( syncTypeTime )
		&& m_pKernel->ShouldAbortSync( syncTypeTime ) )
	{
		m_pKernel->NotifyAbortedSync( syncTypeTime ) ;
		thread.ClearLocalStrage( pxmlStorage ) ;
		return	codeProcessed ;
	}
	if ( thread.GetThreadTick()
			< pxmlStorage->GetAttrIntegerAs( L"timeout_tick", 0 ) )
	{
		return	codePending ;
	}
	thread.ClearLocalStrage( pxmlStorage ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,wait_for)
{
	const SString *	pstrTimeout = code.GetAttributeAs( L"timeout" ) ;
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( L"wait" ) ;
	if ( pxmlStorage->GetAttributeAs( L"start_tick" ) == nullptr )
	{
		pxmlStorage->SetAttrIntegerAs
				( L"start_tick", thread.GetThreadTick() ) ;
		//
		if ( (pstrTimeout != nullptr) && !pstrTimeout->IsEmpty() )
		{
			int64_t	nTickTimeout = EvaluateIntExpression( thread, *pstrTimeout ) ;
			pxmlStorage->SetAttrIntegerAs( L"timeout", nTickTimeout ) ;
		}
	}
	if ( thread.IsPermittedSkip( syncTypeEvent )
		&& m_pKernel->ShouldAbortSync( syncTypeEvent ) )
	{
		m_pKernel->NotifyAbortedSync( syncTypeEvent ) ;
		thread.ClearLocalStrage( pxmlStorage ) ;
		return	codeProcessed ;
	}
	if ( (pstrTimeout != nullptr) && !pstrTimeout->IsEmpty() )
	{
		int64_t	nStartTick = pxmlStorage->GetAttrIntegerAs( L"start_tick", 0 ) ;
		int64_t	nTickTimeout = pxmlStorage->GetAttrIntegerAs( L"timeout", 0 ) ;
		if ( thread.GetThreadTick() >= nStartTick + nTickTimeout )
		{
			thread.ClearLocalStrage( pxmlStorage ) ;
			return	codeProcessed ;
		}
	}
	const SString *	pstrCond = code.GetAttributeAs( L"cond" ) ;
	if ( pstrCond != nullptr )
	{
		if ( EvaluateBoolExpression( thread, code, *pstrCond ) )
		{
			thread.ClearLocalStrage( pxmlStorage ) ;
			return	codeProcessed ;
		}
	}
	return	codePending ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,start_timer)
{
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( L"timer" ) ;
	pxmlStorage->SetAttrIntegerAs( L"start_tick", thread.GetThreadTick() ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,wait_timer)
{
	const SString *	pstrTime = code.GetAttributeAs( L"time" ) ;
	if ( pstrTime == nullptr )
	{
		return	codeProcessed ;
	}
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( L"timer" ) ;
	//
	int64_t	nTimeout = 0 ;
	if ( pxmlStorage->GetAttributeAs( L"timeout" ) == nullptr )
	{
		nTimeout = EvaluateIntExpression( thread, *pstrTime ) ;
		pxmlStorage->SetAttrIntegerAs( L"timeout", nTimeout ) ;
	}
	else
	{
		nTimeout = pxmlStorage->GetAttrIntegerAs( L"timeout", 0 ) ;
	}
	//
	int64_t	nStartTick = pxmlStorage->GetAttrIntegerAs( L"start_tick", 0 ) ;
	if ( thread.GetThreadTick() >= nStartTick + nTimeout )
	{
		pxmlStorage->RemoveAttributeAs( L"timeout" ) ;
		return	codeProcessed ;
	}
	if ( thread.IsPermittedSkip( syncTypeTime )
		&& m_pKernel->ShouldAbortSync( syncTypeTime ) )
	{
		m_pKernel->NotifyAbortedSync( syncTypeTime ) ;
		pxmlStorage->RemoveAttributeAs( L"timeout" ) ;
		return	codeProcessed ;
	}
	return	codePending ;
}

static const SXMLDocument::AttrInteger	s_aiSkipPermissionType[] =
{
	{ L"all", (1 << syncTypeTime)
				| (1 << syncTypeMessage)
				| (1 << syncTypeEffect)
				| (1 << syncTypeEvent)
				| (1 << syncTypeSystemEffect) },
	{ L"time", 1 << syncTypeTime },
	{ L"message", 1 << syncTypeMessage },
	{ L"effect", 1 << syncTypeEffect },
	{ L"event", 1 << syncTypeEvent },
	{ nullptr, 0 },
} ;

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,permit_skip)
{
	uint32_t	mask =
		(uint32_t) code.GetAttrComplexIntegerAs
						( L"flags", s_aiSkipPermissionType, 0 ) ;
	thread.PermitSkipMask( mask ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,prohibit_skip)
{
	uint32_t	mask =
		(uint32_t) code.GetAttrComplexIntegerAs
						( L"flags", s_aiSkipPermissionType, 0 ) ;
	thread.ProhibitSkipMask( mask ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,dis_interrupt)
{
	bool	flagDisable = (code.GetAttrIntegerAs( L"flag", 1 ) != 0) ;
	thread.SetDisableInterruptFlag( flagDisable ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,trace)
{
	const SString *	pstrText = code.GetAttributeAs( L"text" ) ;
	if ( pstrText != nullptr )
	{
		m_pKernel->OutputTrace( EvaluateExprInText( thread, *pstrText ) ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,nop)
{
	return	codeProcessed ;
}



//////////////////////////////////////////////////////////////////////////////
// スクリプト・エンジン・カーネル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLKernel, AGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLKernel::AGLKernel( void )
	: m_pVM( nullptr ), m_pLVM( nullptr ),
		m_pErrorTracer( nullptr ),
		m_pModuleManager( nullptr ),
		m_tsmodeSerializeThread( tsmodeSerializeAllThreads ),
		m_idNextThread( 1 ), m_nSuspended( 0 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLKernel::~AGLKernel( void )
{
	ReleaseKernel() ;
}

// RSVirtualMachine 関連付け
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::AttachRosettaVM( Rosetta::RSVirtualMachine * pVM )
{
	ESLAssert( pVM != nullptr ) ;
	m_pVM = pVM ;
	m_context.SetRosetta( new RSContext( pVM ) ) ;
}

Rosetta::RSVirtualMachine * AGLKernel::GetRosettaVM( void ) const
{
	return	m_pVM ;
}

// LVirtualMachine 関連付け
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::AttachLoquatyVM( Loquaty::LVirtualMachine * pVM )
{
	ESLAssert( pVM != nullptr ) ;
	m_pLVM = pVM ;
	m_context.SetLoquaty( pVM->new_Task() ) ;
}

Loquaty::LVirtualMachine * AGLKernel::GetLoquatyVM( void ) const
{
	return	m_pLVM ;
}

// スクリプト実行コンテキスト
//////////////////////////////////////////////////////////////////////////////
const AGLScriptContext& AGLKernel::GetContext( void ) const
{
	return	m_context ;
}

Rosetta::RSContext * AGLKernel::GetRSContext( void ) const
{
	return	m_context.GetRosetta() ;
}

// Loquaty スクリプトを利用するか？
//////////////////////////////////////////////////////////////////////////////
bool AGLKernel::IsUsingLoquaty( void ) const
{
	if ( m_pLVM != nullptr )
	{
		return	(m_context.GetLanguageType() == languageLoquaty)
				|| (m_pDefThisObj.GetLanguageType() == languageLoquaty) ;
	}
	return	false ;
}

// デフォルト this オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::SetDefaultThisObject( const AGLScriptObject& pThisObj )
{
	m_pDefThisObj = pThisObj ;
}

const AGLScriptObject& AGLKernel::GetDefaultThisObject( void ) const
{
	return	m_pDefThisObj ;
}

// SParserErrorInterface 関連付け
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::AttachErrorTracer( SSystem::SParserErrorInterface * pErrorTracer )
{
	m_pErrorTracer = pErrorTracer ;
}

// AGLModuleManager 関連付け
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::AttachModuleManager( AGLModuleManager * pModuleManager )
{
	m_csSync.Lock() ;
	m_pModuleManager = pModuleManager ;
	m_csSync.Unlock() ;
}

AGLModuleManager * AGLKernel::GetModuleManager( void ) const
{
	return	m_pModuleManager ;
}

// AGLEpicProcessor 追加
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::AttachEpicProcessor( AGLEpicProcessor * pEpicProc )
{
	if ( m_aEpicProc.FindPtr( pEpicProc ) < 0 )
	{
		ESLAssert( pEpicProc->GetKernel() == nullptr ) ;
		pEpicProc->AttachKernel( this )  ;
		m_aEpicProc.Add( pEpicProc ) ;
	}
}

// AGLEpicProcessor 分離
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::DetachEpicProcessor( AGLEpicProcessor * pEpicProc )
{
	ssize_t	iProc = m_aEpicProc.FindPtr( pEpicProc ) ;
	if ( iProc >= 0 )
	{
		m_aEpicProc.RemoveAt( (size_t) iProc ) ;
	}
}

// AGLEpicProcessor 取得
//////////////////////////////////////////////////////////////////////////////
AGLEpicProcessor * AGLKernel::GetEpicProcessor( const ESLRuntimeClass& rtClass ) const
{
	EpicProcessorIterator	iNext = FirstEpicProcessor() ;
	return	NextEpicProcessor( rtClass, iNext ) ;
}

size_t AGLKernel::FirstEpicProcessor( void ) const
{
	return	0 ;
}

AGLEpicProcessor *
	AGLKernel::NextEpicProcessor
		( const ESLRuntimeClass& rtClass, EpicProcessorIterator& iNext ) const
{
	for ( size_t i = iNext; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		if ( (pProc != nullptr) && pProc->IsKindOf( rtClass ) )
		{
			iNext = i + 1 ;
			return	pProc ;
		}
	}
	return	nullptr ;
}

// 設定
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::LoadConfiguration( const SSystem::SXMLDocument& xmlConfig )
{
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		if ( pProc != nullptr )
		{
			pProc->LoadConfiguration( xmlConfig ) ;
		}
	}
}

// 関連付け解除
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::ReleaseKernel( void )
{
	m_csSync.Lock() ;
	m_threads.RemoveAll() ;
	m_threadTrash.RemoveAll() ;
	m_csSync.Unlock() ;
	//
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		if ( pProc != nullptr )
		{
			pProc->OnReleaseKernel() ;
		}
	}
	m_aEpicProc.RemoveAll() ;
	m_pModuleManager = nullptr ;
	//
	m_pDefThisObj.Release() ;
	m_context.Release() ;
	m_pVM = nullptr ;
}

// スクリプトコード実行
//////////////////////////////////////////////////////////////////////////////
AGLThread::Status AGLKernel::Execute( void )
{
	if ( m_nSuspended > 0 )
	{
		return	AGLThread::statusSuspend ;
	}
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		if ( pProc != nullptr )
		{
			pProc->OnKernelTimer() ;
		}
	}
	m_csSync.Lock() ;
	SSmartObjectArray<AGLThread>::Iterator	iter( &m_threads ) ;
	while ( iter.HasNext() )
	{
		AGLThread *	pThread = iter.Next() ;
		m_csSync.Unlock() ;
		//
		if ( pThread != nullptr )
		{
			if ( ExecuteThread( *pThread ) == AGLThread::statusHalt )
			{
				m_csSync.Lock() ;
				if ( !iter.IsElementExpired() )
				{
					m_threads.SetAt( iter.Index(), nullptr ) ;
				}
				m_csSync.Unlock() ;
			}
		}
		m_csSync.Lock() ;
	}
	m_threads.TrimEmpty() ;
	m_threadTrash.RemoveAll() ;
	//
	if ( m_threads.GetLength() == 0 )
	{
		m_csSync.Unlock() ;
		return	AGLThread::statusHalt ;
	}
	m_csSync.Unlock() ;
	return	AGLThread::statusExecute ;
}

AGLThread::Status AGLKernel::ExecuteThread( AGLThread& thread )
{
	const int64_t	msecTimeout = thread.GetFrameTimeot() ;
	size_t			nSteps = 0 ;
	m_timerExec.Reset() ;
	//
	while ( thread.GetStatus() == AGLThread::statusExecute )
	{
		AGLCode *			pCode = thread.GetNextCode() ;
		CodeProcessResult	cpr = codeProcessed ;
		if ( pCode  != nullptr )
		{
			cpr = ExecuteCode( thread, *pCode ) ;
		}
		if ( cpr == codePending )
		{
			thread.FetchInterruption() ;
			return	AGLThread::statusExecute ;
		}
		if ( cpr != codeControlled )
		{
			thread.NextCodeIndex() ;
		}
		if ( ++ nSteps > 10 )
		{
			if ( m_timerExec.GetTime() > msecTimeout )
			{
				SString	strMsg ;
				if ( thread.GetThreadName().IsEmpty() )
				{
					strMsg.Format
						( L"Antirrhinum script frame timeout %d [ms]\n",
							m_timerExec.GetTime() ) ;
				}
				else
				{
					strMsg.Format
						( L"Antirrhinum script frame timeout %d [ms] (thread:%s)\n",
							m_timerExec.GetTime(), (const wchar_t*) thread.GetThreadName() ) ;
				}
				OutputTrace( strMsg ) ;
				return	thread.GetStatus() ;
			}
			nSteps = 0 ;
		}
	}
	return	thread.GetStatus() ;
}

CodeProcessResult AGLKernel::ExecuteCode( AGLThread& thread, AGLCode& code )
{
	CodeProcessResult	cpr = code.ProcessCode( thread ) ;
	if ( cpr == codeUndefined )
	{
		ssize_t	iCmd = m_iaCmdMap.FindIndex( code.GetTag() ) ;
		if ( iCmd >= 0 )
		{
			const EpicCodeProcessor *
					pecp = m_aFuncDesc.GetAt( (size_t) iCmd ) ;
			ESLAssert( pecp != nullptr ) ;
			if ( pecp != nullptr )
			{
				code.SetEpicProcessor( this, pecp->pEpicProc, pecp->pfnProc ) ;
				cpr = code.ProcessCode( thread ) ;
			}
		}
		else
		{
			for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
			{
				AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
				ESLAssert( pProc != nullptr ) ;
				AGLCode::PFUNC_PROCESSOR
						pfnProc = pProc->GetCodeProcesser( code.GetTag() ) ;
				if ( pfnProc != nullptr )
				{
					EpicCodeProcessor	ecp ;
					ecp.pEpicProc = pProc ;
					ecp.pfnProc= pfnProc ;
					//
					ESLAssert( m_iaCmdMap.GetLength() == m_aFuncDesc.GetLength() ) ;
					m_iaCmdMap.Add( new SString( code.GetTag() ) ) ;
					m_aFuncDesc.Add( ecp ) ;
					//
					code.SetEpicProcessor( this, pProc, pfnProc ) ;
					cpr = code.ProcessCode( thread ) ;
					break ;
				}
			}
		}
		if ( cpr == codeUndefined )
		{
		#if	defined(__DEBUG__)
			ESLTrace( "<%s> is unprocessed.\n",
						code.GetTag().ToCharArray().GetConstArray() ) ;
			return	codeUnprocessed ;
		#endif
		}
	}
	return	cpr ;
}

// 全スレッド一時停止
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::SuspendAllThreads( void )
{
	m_csSync.Lock() ;
	SSmartObjectArray<AGLThread>::Iterator	iter( &m_threads ) ;
	while ( iter.HasNext() )
	{
		AGLThread *	pThread = iter.Next() ;
		if ( pThread != nullptr )
		{
			pThread->Suspend() ;
		}
	}
	AtomicAdd( &m_nSuspended, 1 ) ;
	m_csSync.Unlock() ;
}

// 全スレッド再開
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::ResumeAllThreads( void )
{
	m_csSync.Lock() ;
	SSmartObjectArray<AGLThread>::Iterator	iter( &m_threads ) ;
	while ( iter.HasNext() )
	{
		AGLThread *	pThread = iter.Next() ;
		if ( pThread != nullptr )
		{
			pThread->Resume() ;
		}
	}
	ESLVerify( AtomicSub( &m_nSuspended, 1 ) >= 0 ) ;
	m_csSync.Unlock() ;
}

// スレッド開始
//////////////////////////////////////////////////////////////////////////////
SSystem::SSmartRef<AGLThread> AGLKernel::BeginThread
	( const wchar_t * pwszModuleFile,
		const wchar_t * pwszLabel,
		const AGLScriptObject& instance, const wchar_t * pwszName )
{
	AGLThread *	pThread = new AGLThread( m_pModuleManager, instance ) ;
	if ( pwszName != nullptr )
	{
		pThread->SetThreadName( pwszName ) ;
	}
	pThread->JumpCodeScript
		( pwszModuleFile, pwszLabel, AGLThread::ctrlThread ) ;
	//
	SSmartRef<AGLThread>	refThread( pThread ) ;
	//
	BeginThread( pThread ) ;
	//
	return	refThread ;
}

SSystem::SError AGLKernel::BeginThread( AGLThread * pThread )
{
	m_csSync.Lock() ;
	ESLAssert( GetThreadByID(m_idNextThread) == nullptr ) ;
	pThread->m_idThread = m_idNextThread ++ ;
	if ( m_idNextThread == 0 )
	{
		m_idNextThread = 100 ;
	}
	ESLAssert( pThread->m_pKernel == nullptr ) ;
	pThread->m_pKernel = this ;
	//
	m_threads.Add( pThread ) ;
	m_csSync.Unlock() ;
	return	errSuccess ;
}

// スレッド取得
//////////////////////////////////////////////////////////////////////////////
AGLThread * AGLKernel::GetThreadByID( uint32_t idThread ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_threads.GetLength(); i ++ )
	{
		AGLThread *	pThread = m_threads.GetAt( i ) ;
		ESLAssert( pThread != nullptr ) ;
		if ( (pThread != nullptr) && (pThread->m_idThread == idThread) )
		{
			return	pThread ;
		}
	}
	return	nullptr ;
}

AGLThread * AGLKernel::GetThreadByName( const wchar_t * pwszName ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_threads.GetLength(); i ++ )
	{
		AGLThread *	pThread = m_threads.GetAt( i ) ;
		ESLAssert( pThread != nullptr ) ;
		if ( (pThread != nullptr) && (pThread->m_strName == pwszName) )
		{
			return	pThread ;
		}
	}
	return	nullptr ;
}

// スレッド生存確認
//////////////////////////////////////////////////////////////////////////////
bool AGLKernel::IsValidThread( AGLThread * pThread ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	return	(m_threads.FindPtr( pThread ) >= 0) ;
}

// スレッドの強制終了
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::TerminateThread( AGLThread * pThread )
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	ssize_t	i = m_threads.FindPtr( pThread ) ;
	if ( i >= 0 )
	{
		m_threadTrash.Add( m_threads.DetachAt( (size_t) i ) ) ;
	}
}

// 全スレッドの強制終了
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::TerminateAllThreads( void )
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	SSmartObjectArray<AGLThread>::Iterator	iter( &m_threads ) ;
	while ( iter.HasNext() )
	{
		AGLThread *	pThread = iter.Next() ;
		if ( pThread != nullptr )
		{
			m_threadTrash.Add( m_threads.DetachAt( iter.Index() ) ) ;
		}
	}
}

// AGLModuleManager に関連するすべてのスレッドを強制終了する
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::TerminateAllThreadsOf( AGLModuleManager * pManager )
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	SSmartObjectArray<AGLThread>::Iterator	iter( &m_threads ) ;
	while ( iter.HasNext() )
	{
		AGLThread *	pThread = iter.Next() ;
		if ( (pThread != nullptr)
			&& (pThread->GetModuleManager() == pManager) )
		{
			pThread->DetachModule() ;
			m_threadTrash.Add( m_threads.DetachAt( iter.Index() ) ) ;
		}
	}
}

// スレッドのシリアライズモードを取得
//////////////////////////////////////////////////////////////////////////////
AGLKernel::ThreadSerializeMode AGLKernel::GetThreadSerializeMode( void ) const
{
	return	m_tsmodeSerializeThread ;
}

// スレッドのシリアライズモードを設定
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::SetThreadSerializeMode( AGLKernel::ThreadSerializeMode tsmode )
{
	m_tsmodeSerializeThread = tsmode ;
}

// ゲーム開始時処理
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::InitializeGame( void )
{
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		ESLAssert( pProc != nullptr ) ;
		pProc->InitializeGame() ;
	}
}

// ゲーム終了前フェードアウト処理
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::FadeoutGame( uint32_t msecFadeout )
{
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		ESLAssert( pProc != nullptr ) ;
		pProc->FadeoutGame( msecFadeout ) ;
	}
}

// ゲーム終了時処理
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::ReleaseGame( void )
{
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		ESLAssert( pProc != nullptr ) ;
		pProc->ReleaseGame() ;
	}
}

// 待機関数を（ユーザー入力等により）即時に脱出すべきか判定する
//////////////////////////////////////////////////////////////////////////////
bool AGLKernel::ShouldAbortSync( SynchronismType type )
{
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		ESLAssert( pProc != nullptr ) ;
		if ( pProc->ShouldAbortSync( type ) )
		{
			return	true ;
		}
	}
	return	false ;
}

// 待機関数を ShouldAbortSync を理由に脱出したことの通知
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::NotifyAbortedSync( SynchronismType type )
{
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		ESLAssert( pProc != nullptr ) ;
		pProc->NotifyAbortedSync( type ) ;
	}
}

// 効果時間の修正処理
//////////////////////////////////////////////////////////////////////////////
uint32_t AGLKernel::EffectTime( uint32_t msecTime, SynchronismType type )
{
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		ESLAssert( pProc != nullptr ) ;
		msecTime = pProc->EffectTime( msecTime, type ) ;
	}
	return	msecTime ;
}

// Rosetta 文コンパイル
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLKernel::CompileRosettaStatements
	( Rosetta::RSScript& script, const SSystem::SString& strSrc )
{
	ESLAssert( m_context.GetRosetta() != nullptr ) ;
	if ( m_context.GetRosetta() == nullptr )
	{
		return	errFailed ;
	}
	SParserErrorTracer	perrTrace ;
	SError	err = script.ParseSource
		( *(m_pVM->LockMacroContext()), strSrc, perrTrace ) ;
	m_pVM->UnlockMacroContext() ;
	return	err ;
}

// Rosetta 文実行
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::PerformRosettaStatements
	( Rosetta::RSScript& script, Rosetta::RSObject * pThisObj )
{
	ESLAssert( m_context.GetRosetta() != nullptr ) ;
	if ( m_context.GetRosetta() == nullptr )
	{
		return ;
	}
	if ( pThisObj == nullptr )
	{
		pThisObj = m_pDefThisObj.GetRosetta() ;
	}
	RSCodeStream	cstrm( script ) ;
	m_context.GetRosetta()->PerformStatement( cstrm, pThisObj ) ;
	if ( m_context.GetRosetta()->IsException() )
	{
		if ( m_pErrorTracer != nullptr )
		{
			m_context.GetRosetta()->OutputExceptionError( *m_pErrorTracer ) ;
		}
		else
		{
			SParserErrorTracer	perrTrace ;
			m_context.GetRosetta()->OutputExceptionError( perrTrace ) ;
		}
		m_context.GetRosetta()->ClearException() ;
	}
}

// Rosetta 数式評価
//////////////////////////////////////////////////////////////////////////////
Rosetta::RSSmartPtr AGLKernel::EvaluateRosettaExpression
	( Rosetta::RSScript& script, Rosetta::RSObject * pThisObj )
{
	ESLAssert( m_context.GetRosetta() != nullptr ) ;
	if ( m_context.GetRosetta() == nullptr )
	{
		return	RSSmartPtr( nullptr ) ;
	}
	if ( pThisObj == nullptr )
	{
		pThisObj = m_pDefThisObj.GetRosetta() ;
	}
	RSCodeStream	cstrm ;
	cstrm.AttachCode( script ) ;
	//
	RSObject *	pObj = m_context.GetRosetta()->PerformExpression( cstrm, pThisObj ) ;
	if ( m_context.GetRosetta()->IsException() )
	{
		if ( m_pErrorTracer != nullptr )
		{
			m_context.GetRosetta()->OutputExceptionError( *m_pErrorTracer ) ;
		}
		else
		{
			SParserErrorTracer	perrTrace ;
			m_context.GetRosetta()->OutputExceptionError( perrTrace ) ;
		}
		m_context.GetRosetta()->ClearException() ;
	}
	return	RSSmartPtr( pObj, m_context.GetRosetta() ) ;
}

Rosetta::RSSmartPtr AGLKernel::EvaluateRosettaExpression
	( const wchar_t * pwszExpr, Rosetta::RSObject * pThisObj )
{
	ESLAssert( m_context.GetRosetta() != nullptr ) ;
	if ( m_context.GetRosetta() == nullptr )
	{
		return	RSSmartPtr( nullptr ) ;
	}
	if ( pThisObj == nullptr )
	{
		pThisObj = m_pDefThisObj.GetRosetta() ;
	}
	RSObject *	pObj = m_context.GetRosetta()->PerformExpression( pwszExpr, pThisObj ) ;
	if ( m_context.GetRosetta()->IsException() )
	{
		if ( m_pErrorTracer != nullptr )
		{
			m_context.GetRosetta()->OutputExceptionError( *m_pErrorTracer, pwszExpr ) ;
		}
		else
		{
			SParserErrorTracer	perrTrace ;
			m_context.GetRosetta()->OutputExceptionError( perrTrace, pwszExpr ) ;
		}
		m_context.GetRosetta()->ClearException() ;
	}
	return	RSSmartPtr( pObj, m_context.GetRosetta() ) ;
}

// 数式評価
//////////////////////////////////////////////////////////////////////////////
AGLScriptObject AGLKernel::EvaluateExpression
	( const wchar_t * pwszExpr, AGLScriptObject objThis, bool flagRef )
{
	if ( objThis.IsNull() )
	{
		objThis = m_pDefThisObj ;
	}
	if ( IsUsingLoquaty() )
	{
		LInstantEvaluator	eval( *m_pLVM ) ;
		if ( eval.MakeExpression( pwszExpr, objThis.GetLoquatyClass() ) )
		{
			LPtr<LTaskObj>	pLTask = m_context.GetLoquaty() ;
			if ( pLTask == nullptr )
			{
				pLTask = m_pLVM->new_Task() ;
			}
			if ( flagRef )
			{
				const LPtr<LFunctionObj>&	func = eval.GetFunction() ;
				if ( func->GetReturnType().IsPrimitive() )
				{
					LClass*	pPtrClass =
								m_pLVM->GetPointerClassAs
									( LType( func->GetReturnType().GetPrimitive() ) ) ;
					LInstantEvaluator	evalRef( *m_pLVM ) ;
					if ( evalRef.MakeExpression
						( pwszExpr, objThis.GetLoquatyClass(),
							LInstantEvaluator::typeObject, pPtrClass ) )
					{
						return	AGLScriptObject( evalRef.EvaluateValue( pLTask, objThis ) ) ;
					}
					else
					{
						OutputTrace( eval.GetErrorMessages() ) ;
					}
					return	AGLScriptObject() ;
				}
			}
			return	AGLScriptObject( eval.EvaluateValue( pLTask, objThis ) ) ;
		}
		else
		{
			OutputTrace( eval.GetErrorMessages() ) ;
		}
	}
	else if ( m_context.GetRosetta() != nullptr )
	{
		RSObject *	pObj =
			m_context.GetRosetta()->
				PerformExpression( pwszExpr, objThis.GetRosetta() ) ;
		if ( m_context.GetRosetta()->IsException() )
		{
			if ( m_pErrorTracer != nullptr )
			{
				m_context.GetRosetta()->OutputExceptionError( *m_pErrorTracer, pwszExpr ) ;
			}
			else
			{
				SParserErrorTracer	perrTrace ;
				m_context.GetRosetta()->OutputExceptionError( perrTrace, pwszExpr ) ;
			}
			m_context.GetRosetta()->ClearException() ;
		}
		return	AGLScriptObject( RSSmartPtr( pObj, m_context.GetRosetta() ) ) ;
	}
	return	AGLScriptObject() ;
}

// 文字列内式展開
//////////////////////////////////////////////////////////////////////////////
SSystem::SString AGLKernel::EvaluateExprInText
	( const wchar_t * pwszText, const AGLScriptObject& instance )
{
	if ( pwszText == nullptr )
	{
		return	SString() ;
	}
	const AGLScriptObject *	pThis = &instance ;
	if ( instance.GetLanguageType() == languageInvalid )
	{
		pThis = &m_pDefThisObj ;
	}
	const bool		flagLoquaty = (m_pLVM != nullptr)
								&& (pThis->GetLanguageType() != languageRosetta) ;
	LPtr<LTaskObj>	pLTask ;
	LClass *		pThisClass = nullptr ;
	if ( flagLoquaty && (pThis->GetLoquatyObj() != nullptr) )
	{
		pThisClass = pThis->GetLoquatyObj()->GetClass() ;
	}

	SStringParser	sparsText = pwszText ;
	SString			strBuf ;
	size_t			iLast = 0 ;
	while ( !sparsText.IsIndexOverflow() )
	{
		size_t	i = sparsText.GetIndex() ;
		if ( sparsText.GetCharacter() == L'%' )
		{
			strBuf += sparsText.SubString( iLast, (ssize_t) (i - iLast) ) ;
			//
			wchar_t	wch = sparsText.GetCharacter() ;
			if ( wch == L'(' )
			{
				SString	strExpr = sparsText.GetExpression( L")" ) ;
				if ( flagLoquaty )
				{
					LInstantEvaluator	eval( *m_pLVM ) ;
					if ( eval.MakeExpression( strExpr, pThisClass ) )
					{
						if ( pLTask == nullptr )
						{
							pLTask = m_context.GetLoquaty() ;
							if ( pLTask == nullptr )
							{
								pLTask = m_pLVM->new_Task() ;
							}
						}
						strBuf += eval.EvaluateAsString( pLTask, pThis->GetLoquatyObj() ) ;
					}
				}
				else
				{
					RSSmartPtr	pEval =
						EvaluateRosettaExpression
							( strExpr, instance.GetRosetta().Ptr() ) ;
					if ( pEval != nullptr )
					{
						if ( pEval->AsString( strExpr ) )
						{
							strBuf += strExpr ;
						}
					}
				}
			}
			else
			{
				strBuf += wch ;
			}
			iLast = sparsText.GetIndex() ;
		}
	}
	if ( iLast < sparsText.GetIndex() )
	{
		strBuf += sparsText.SubString( iLast ) ;
	}
	return	strBuf ;
}

// デバッグ出力
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::OutputTrace( const wchar_t * pwszTrace )
{
	Trace( SString(pwszTrace).ToCharArray().GetConstArray() ) ;
	//
	if ( m_pErrorTracer != nullptr )
	{
		SStringParser	ss ;
		m_pErrorTracer->OutputWarning( ss, pwszTrace ) ;
	}
}

// Loquaty の例外エラーをデバッグ出力する
//////////////////////////////////////////////////////////////////////////////
void AGLKernel::TraceException( LObjPtr pException )
{
	if ( pException == nullptr )
	{
		return ;
	}
	SString	strErrMsg ;
	LString	strMsg ;
	pException->AsString( strMsg ) ;
	strErrMsg = L"exception: " ;
	strErrMsg += strMsg.c_str() ;
	strErrMsg += L"\n" ;

	LExceptionObj *	pExceptObj =
			dynamic_cast<LExceptionObj*>( pException.Ptr() ) ;
	if ( pExceptObj != nullptr )
	{
		LString	strSource ;
		LStringParser::LineInfo	linf ;
		if ( pExceptObj->GetThrownSourceInfo( strSource, linf ) )
		{
			strErrMsg += L"  thrown from \'" ;
			strErrMsg += strSource.c_str() ;
			strErrMsg += L"\' at line " ;
			strErrMsg += SString( linf.iLine ) ;
			strErrMsg += L"\n" ;
		}
	}
	OutputTrace( strErrMsg ) ;
}

// シリアライズ可能状態か？
//////////////////////////////////////////////////////////////////////////////
bool AGLKernel::CanSerialize( void ) const
{
	if ( m_tsmodeSerializeThread != tsmodeNoSerializeAnyThreads )
	{
		for ( size_t iThread = 0; iThread < m_threads.GetLength(); iThread ++ )
		{
			AGLThread *	pThread = m_threads.GetAt( iThread ) ;
			if ( (pThread != nullptr) && !pThread->CanSerialize() )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

// カーネル取得
//////////////////////////////////////////////////////////////////////////////
AGLKernel * AGLKernel::GetKernel( void ) const
{
	return	const_cast<AGLKernel*>( this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLKernel::Serialize( SSystem::SXMLDocument& xmlTag )
{
	return	Serialize( xmlTag, this ) ;
}

SSystem::SError AGLKernel::Serialize
		( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	if ( m_tsmodeSerializeThread != tsmodeNoSerializeAnyThreads )
	{
		SXMLDocument *	pxmlThreads = xmlTag.CreateElementTagAs( L"threads" ) ;
		pxmlThreads->SetAttrIntegerAs( L"next_thread_id", m_idNextThread ) ;
		pxmlThreads->SetAttrIntegerAs( L"suspended", m_nSuspended ) ;
		for ( size_t iThread = 0; iThread < m_threads.GetLength(); iThread ++ )
		{
			AGLThread *	pThread = m_threads.GetAt( iThread ) ;
			if ( pThread != nullptr )
			{
				SXMLDocument *	pxmlThread = new SXMLDocument ;
				pThread->Serialize( *pxmlThread, pKernel ) ;
				pxmlThread->SetTag( L"thread" ) ;
				pxmlThreads->AddElement( pxmlThread ) ;
			}
		}
	}
	for ( size_t iEpic = 0; iEpic < m_aEpicProc.GetLength(); iEpic ++ )
	{
		AGLEpicProcessor *	pEpicProc = m_aEpicProc.GetAt( iEpic ) ;
		ESLAssert( pEpicProc != nullptr ) ;
		SXMLDocument *	pxmlEpic = new SXMLDocument ;
		pEpicProc->Serialize( *pxmlEpic, pKernel ) ;
		pxmlEpic->SetTag( pEpicProc->GetObjectType() ) ;
		xmlTag.AddElement( pxmlEpic ) ;
	}
	return	errSuccess ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLKernel::Deserialize( const SSystem::SXMLDocument& xmlTag )
{
	return	Deserialize( xmlTag, this ) ;
}

SSystem::SError AGLKernel::Deserialize
		( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	if ( m_tsmodeSerializeThread != tsmodeNoSerializeAnyThreads )
	{
		SXMLDocument *	pxmlThreads = xmlTag.GetElementTagAs( L"threads" ) ;
		m_threads.RemoveAll() ;
		if ( pxmlThreads != nullptr )
		{
			m_idNextThread =
				(uint32_t) pxmlThreads->GetAttrIntegerAs
								( L"next_thread_id", m_idNextThread ) ;
			m_nSuspended =
				(atomic_int_t) pxmlThreads->GetAttrIntegerAs
								( L"suspended", m_nSuspended ) ;
			for ( size_t i = 0; i < pxmlThreads->GetElementsCount(); i ++ )
			{
				SXMLDocument *	pxmlThread = pxmlThreads->GetElementAt( i ) ;
				if ( (pxmlThread == nullptr)
					|| (pxmlThread->GetTag() != L"thread") )
				{
					continue ;
				}
				AGLThread *	pThread = new AGLThread( m_pModuleManager ) ;
				pThread->Deserialize( *pxmlThread, pKernel ) ;
				pThread->m_pKernel = this ;
				m_threads.Add( pThread ) ;
			}
		}
	}
	for ( size_t iEpic = 0; iEpic < m_aEpicProc.GetLength(); iEpic ++ )
	{
		AGLEpicProcessor *	pEpicProc = m_aEpicProc.GetAt( iEpic ) ;
		ESLAssert( pEpicProc != nullptr ) ;
		SXMLDocument *	pxmlEpic =
				xmlTag.GetElementTagAs( pEpicProc->GetObjectType() ) ;
		if ( pxmlEpic != nullptr )
		{
			pEpicProc->Deserialize( *pxmlEpic, pKernel ) ;
		}
	}
	return	errSuccess ;
}

// デシリアライズ後の参照解決処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLKernel::AfterDeserialize( void )
{
	return	AfterDeserialize( this ) ;
}

SSystem::SError AGLKernel::AfterDeserialize( AGLKernel * pKernel )
{
	for ( size_t iThread = 0; iThread < m_threads.GetLength(); iThread ++ )
	{
		AGLThread *	pThread = m_threads.GetAt( iThread ) ;
		if ( pThread != nullptr )
		{
			pThread->AfterDeserialize( pKernel ) ;
		}
	}
	for ( size_t i = 0; i < m_aEpicProc.GetLength(); i ++ )
	{
		AGLEpicProcessor *	pProc = m_aEpicProc.GetAt( i ) ;
		ESLAssert( pProc != nullptr ) ;
		pProc->AfterDeserialize( pKernel ) ;
	}
	return	errSuccess ;
}
