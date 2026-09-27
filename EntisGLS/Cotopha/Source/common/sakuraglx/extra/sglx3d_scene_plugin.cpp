
#include <loquaty.h>
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/extra/sglx3d_scene_plugin.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// プラグイン・実装基底
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposerPluginBase::S3DSceneComposerPluginBase
		( const S3DSceneComposerPluginDescriptor * pDesc, size_t nCount )
	: m_pManager( NULL )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DSceneComposerPluginDescriptor	dsc = pDesc[i] ;
		if ( dsc.pfnCreateItem == NULL )
		{
			dsc.pfnCreateItem = &S3DSceneComposerPluginBase::desc_CreateItem ;
		}
		if ( dsc.pfnOnUpdateMenu == NULL )
		{
			dsc.pfnOnUpdateMenu = &S3DSceneComposerPluginBase::desc_OnUpdateMenu ;
		}
		m_descriptor.Add( dsc ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposerPluginBase::~S3DSceneComposerPluginBase( void )
{
}

// エントリ関数
//////////////////////////////////////////////////////////////////////////////
void * S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::entry_Startup
	( S3DCompositionManager * pManager, const S3DSceneComposerPluginEntry * pEntry )
{
	return	NULL ;
}

void S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::entry_Shoutdown
		( S3DCompositionManager * pManager, void * pPluginInstance )
{
}

const S3DSceneComposerPluginDescriptor *
	S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::entry_GetDescriptorTable
		( void * pPluginInstance, size_t& nTableSize )
{
	ESLAssert( pPluginInstance != NULL ) ;
	if ( pPluginInstance == NULL )
	{
		nTableSize = 0 ;
		return	NULL ;
	}
	S3DSceneComposerPluginBase *
		pPlugin = (S3DSceneComposerPluginBase*) pPluginInstance ;
	return	pPlugin->GetDescriptorTable( nTableSize ) ;
}

void * S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::entry_OnStartScene
	( void * pPluginInstance,
		const S3DSceneComposerPluginSceneInfo& scpsi )
{
	S3DSceneComposerPluginBase *
		pPlugin = (S3DSceneComposerPluginBase*) pPluginInstance ;
	return	pPlugin->OnStartScene( scpsi ) ;
}

void S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::entry_OnEndScene
	( void * pPluginInstance, void * pSceneInstance )
{
	S3DSceneComposerPluginBase *
		pPlugin = (S3DSceneComposerPluginBase*) pPluginInstance ;
	pPlugin->OnEndScene( pSceneInstance ) ;
}

void S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::entry_OnStartComposition
	( void * pPluginInstance, void * pSceneInstance,
		S3DSceneComposer::Composition * pComposition )
{
	S3DSceneComposerPluginBase *
		pPlugin = (S3DSceneComposerPluginBase*) pPluginInstance ;
	pPlugin->OnStartComposition( pSceneInstance, pComposition ) ;
}

void S3DSceneComposerPluginBase::OnStartup( S3DCompositionManager * pManager )
{
	m_pManager = pManager ;
}

void S3DSceneComposerPluginBase::OnShoutdown( S3DCompositionManager * pManager )
{
	ESLAssert( m_pManager == pManager ) ;
	m_pManager = NULL ;
}

const S3DSceneComposerPluginDescriptor *
	S3DSceneComposerPluginBase::GetDescriptorTable( size_t& nTableSize )
{
	nTableSize = m_descriptor.GetLength() ;
	return	m_descriptor.GetConstArray() ;
}

void * S3DSceneComposerPluginBase::OnStartScene
		( const S3DSceneComposerPluginSceneInfo& scpsi )
{
	return	NULL ;
}

void S3DSceneComposerPluginBase::OnEndScene( void * pSceneInstance )
{
}

void S3DSceneComposerPluginBase::OnStartComposition
	( void * pSceneInstance,
		S3DSceneComposer::Composition * pComposition )
{
}

// S3DCompositionManager 取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionManager *
	S3DSceneComposerPluginBase::GetCompositionManager( void ) const
{
	return	m_pManager ;
}

// ディスクリプタ関数
//////////////////////////////////////////////////////////////////////////////
ESLObject * S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::desc_CreateItem
		( void * pPluginInstance,
			const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposerEditorEnvironment& env )
{
	ESLAssert( pPluginInstance != NULL ) ;
	if ( pPluginInstance == NULL )
	{
		return	NULL ;
	}
	S3DSceneComposerPluginBase *
		pPlugin = (S3DSceneComposerPluginBase*) pPluginInstance ;
	return	pPlugin->CreateItem( desc, env ) ;
}

uint32_t S3D_PLUGIN_STDCALL S3DSceneComposerPluginBase::desc_OnUpdateMenu
		( void * pPluginInstance,
			const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposer::ItemSerializer* pParentItem )
{
	ESLAssert( pPluginInstance != NULL ) ;
	if ( pPluginInstance == NULL )
	{
		return	0 ;
	}
	S3DSceneComposerPluginBase *
		pPlugin = (S3DSceneComposerPluginBase*) pPluginInstance ;
	return	pPlugin->OnUpdateMenu( desc, pParentItem ) ;
}

// アイテム作成
//////////////////////////////////////////////////////////////////////////////
ESLObject * S3DSceneComposerPluginBase::CreateItem
		( const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposerEditorEnvironment& env )
{
	switch ( desc.type )
	{
	case	s3dscenePluginTypeItem:
		ESLAssert( m_pManager != NULL ) ;
		return	m_pManager->CreateSceneItem( desc.pwszID ) ;

	case	s3dscenePluginTypeController:
		ESLAssert( m_pManager != NULL ) ;
		return	m_pManager->CreateController( desc.pwszID ) ;

	case	s3dscenePluginTypeProcRsrc:
		ESLAssert( m_pManager != NULL ) ;
		return	m_pManager->CreateResourceProc( desc.pwszID ) ;

	default:
		break ;
	}
	return	NULL ;
}

// メニュー状態
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposerPluginBase::OnUpdateMenu
		( const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposer::ItemSerializer* pParentItem )
{
	return	0 ;
}



//////////////////////////////////////////////////////////////////////////////
// プラグイン・インスタンス（ローダー）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposerPluginInstance, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposerPluginInstance::S3DSceneComposerPluginInstance( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	m_hModule = NULL ;
#endif
	m_pManager = NULL ;
	m_pEntry = NULL ;
	m_pInstance = NULL ;
	m_pSceneInstance = NULL ;
	m_pDescriptor = NULL ;
	m_nDescLength = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposerPluginInstance::~S3DSceneComposerPluginInstance( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( (m_hModule != NULL) || (m_pInstance != NULL) )
#else
	if ( m_pInstance != NULL )
#endif
	{
		Release() ;
	}
}

// プラグイン読み込む
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_WINDOWS__)
SGLError S3DSceneComposerPluginInstance::LoadPlugin
	( const wchar_t * pwszFileName, S3DCompositionManager * pManager )
{
	if ( (m_hModule != nullptr) || (m_pInstance != nullptr) )
	{
		Release() ;
	}
	if ( g_infoPlatform.runtimeOS != platformOS_Windows )
	{
		m_hModule = ::LoadLibraryW( pwszFileName ) ;
	}
	else
	{
		SArray<char>	bufFileName ;
		SString			strFileName = pwszFileName ;
		m_hModule = ::LoadLibrary( strFileName.EncodeDefaultTo(bufFileName) ) ;
	}
	if ( m_hModule == nullptr )
	{
		return	sglErrFailed ;
	}
	API_S3D_GetPluginEntry	apiGetPluginEntry =
		(API_S3D_GetPluginEntry)
			::GetProcAddress( m_hModule, "_s3dscene_GetPluginEntry@0" ) ;
	if ( apiGetPluginEntry == nullptr )
	{
		apiGetPluginEntry =
			(API_S3D_GetPluginEntry)
				::GetProcAddress( m_hModule, "s3dscene_GetPluginEntry" ) ;
		if ( apiGetPluginEntry == nullptr )
		{
			return	sglErrFailed ;
		}
	}
	m_pEntry = apiGetPluginEntry() ;
	if ( m_pEntry == NULL )
	{
		return	sglErrFailed ;
	}
	m_pManager = pManager ;
	//
	m_pInstance = (m_pEntry->pfnStartup)( pManager, m_pEntry ) ;
	m_pDescriptor =
		(m_pEntry->pfnGetDescriptorTable)( m_pInstance, m_nDescLength ) ;
	//
	return	sglErrSuccess ;
}
#endif

SGLError S3DSceneComposerPluginInstance::LoadPlugin
	( S3DSceneComposerPluginBase * pPlugin, S3DCompositionManager * pManager )
{
	if ( m_pInstance != NULL )
	{
		Release() ;
	}
	ESLAssert( pPlugin != NULL ) ;
	m_pEntry = pPlugin->GetPluginEntry() ;
	ESLAssert( m_pEntry != NULL ) ;
	//
	m_pManager = pManager ;
	//
	m_pInstance = (m_pEntry->pfnStartup)( pManager, m_pEntry ) ;
	m_pDescriptor =
		(m_pEntry->pfnGetDescriptorTable)( m_pInstance, m_nDescLength ) ;
	//
	return	sglErrSuccess ;
}

// プラグインを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposerPluginInstance::Release( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( m_hModule != NULL )
	{
		// ※解放はしない
		// ::FreeLibrary( m_hModule ) ;
		m_hModule = NULL ;
	}
#endif
	ESLAssert( m_pSceneInstance == NULL ) ;
	if ( m_pInstance != NULL )
	{
		ESLAssert( m_pEntry != NULL ) ;
		(m_pEntry->pfnShoutdown)( m_pManager, m_pInstance ) ;
		m_pInstance = NULL ;
	}
	return	sglErrSuccess ;
}

// ディスクリプタテーブル取得
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposerPluginDescriptor *
	S3DSceneComposerPluginInstance::GetDescriptorTable( size_t& nTableSize ) const
{
	nTableSize = m_nDescLength ;
	return	m_pDescriptor ;
}

// シーン開始時準備処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerPluginInstance::OnStartScene
	( const S3DSceneComposerPluginSceneInfo& scpsi )
{
	ESLAssert( m_pSceneInstance == NULL ) ;
	if ( m_pInstance != NULL )
	{
		ESLAssert( m_pEntry != NULL ) ;
		m_pSceneInstance = (m_pEntry->pfnOnStartScene)( m_pInstance, scpsi ) ;
	}
}

// シーン終了時処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerPluginInstance::OnEndScene( void )
{
	if ( m_pInstance != NULL )
	{
		ESLAssert( m_pEntry != NULL ) ;
		(m_pEntry->pfnOnEndScene)( m_pInstance, m_pSceneInstance ) ;
		m_pSceneInstance = NULL ;
	}
}

// コンポジション開始前処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerPluginInstance::OnStartComposition
	( S3DSceneComposer::Composition * pComposition )
{
	if ( m_pInstance != NULL )
	{
		ESLAssert( m_pEntry != NULL ) ;
		(m_pEntry->pfnOnStartComposition)
				( m_pInstance, m_pSceneInstance, pComposition ) ;
	}
}

// プラグイン・インスタンス取得
//////////////////////////////////////////////////////////////////////////////
void * S3DSceneComposerPluginInstance::GetInstance( void ) const
{
	return	m_pInstance ;
}

// シーン・インスタンス取得
//////////////////////////////////////////////////////////////////////////////
void * S3DSceneComposerPluginInstance::GetSceneInstance( void ) const
{
	return	m_pSceneInstance ;
}



//////////////////////////////////////////////////////////////////////////////
// 動的プラグイン
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposerDynamicPlugin, S3DSceneComposerPluginInstance )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposerDynamicPlugin::S3DSceneComposerDynamicPlugin( void )
{
	m_entry.pfnStartup = &S3DSceneComposerDynamicPlugin::StartupProc ;
	m_entry.pfnShoutdown = &S3DSceneComposerDynamicPlugin::ShoutdownProc ;
	m_entry.pfnGetDescriptorTable = &S3DSceneComposerDynamicPlugin::GetDescriptorTableProc ;
	m_entry.pfnOnStartScene = &S3DSceneComposerDynamicPlugin::OnStartSceneProc ;
	m_entry.pfnOnEndScene = &S3DSceneComposerDynamicPlugin::OnEndSceneProc ;
	m_entry.pfnOnStartComposition = &S3DSceneComposerDynamicPlugin::OnStartCompositionProc ;
	m_entry.pPlugin = this ;
	m_pEntry = &m_entry ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposerDynamicPlugin::~S3DSceneComposerDynamicPlugin( void )
{
	if ( m_pInstance != nullptr )
	{
		Release() ;
		m_pInstance = nullptr ;
	}
}

// プラグイン・インスタンス生成
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerDynamicPlugin::InitPlugin( S3DCompositionManager * pManager )
{
	m_pManager = pManager ;
	m_pInstance = (m_pEntry->pfnStartup)( pManager, m_pEntry ) ;
}

// Descriptor 追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerDynamicPlugin::AddDescriptor( Descriptor * pDesc )
{
	m_aDescriptors.Add( pDesc ) ;

	S3DSceneComposerPluginDescriptor	dsc ;
	dsc.type = pDesc->m_type ;
	dsc.pwszMenuPath = pDesc->m_strMenuPath ;
	dsc.pwszID = pDesc->m_strID ;
	dsc.pfnCreateItem = &S3DSceneComposerDynamicPlugin::CreateItemProc ;
	dsc.pfnOnUpdateMenu = &S3DSceneComposerDynamicPlugin::OnUpdateMenuProc ;

	m_aDescTable.Add( dsc ) ;

	m_pDescriptor = m_aDescTable.GetConstArray() ;
	m_nDescLength = m_aDescTable.GetLength() ;
}

ESLObject * S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::CreateItemProc
		( void * pPluginInstance,
			const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposerEditorEnvironment& env )
{
	Instance *	pInstance = reinterpret_cast<Instance*>( pPluginInstance ) ;

	const size_t	iDesc =
			&desc - pInstance->pPlugin->m_aDescTable.GetConstArray() ;
	ESLAssert( iDesc < pInstance->pPlugin->m_aDescTable.GetLength() ) ;
	ESLAssert( pInstance->pPlugin->m_aDescTable.GetAt(iDesc) == &desc ) ;

	Descriptor *	pDesc = pInstance->pPlugin->m_aDescriptors.GetAt( iDesc ) ;
	if ( pDesc != nullptr )
	{
		return	pDesc->CreateItem( desc, env ) ;
	}
	return	nullptr ;
}

uint32_t S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::OnUpdateMenuProc
		( void * pPluginInstance,
			const S3DSceneComposerPluginDescriptor& desc,
			const S3DSceneComposer::ItemSerializer* pParentItem )
{
	Instance *	pInstance = reinterpret_cast<Instance*>( pPluginInstance ) ;

	const size_t	iDesc =
			&desc - pInstance->pPlugin->m_aDescTable.GetConstArray() ;
	ESLAssert( iDesc < pInstance->pPlugin->m_aDescTable.GetLength() ) ;

	Descriptor *	pDesc = pInstance->pPlugin->m_aDescriptors.GetAt( iDesc ) ;
	if ( pDesc != nullptr )
	{
		return	pDesc->OnUpdateMenu( desc, pParentItem ) ;
	}
	return	0 ;
}

void * S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::StartupProc
	( S3DCompositionManager * pManager, const S3DSceneComposerPluginEntry * pEntry )
{
	PluginEntry *	pPluginEntry = (PluginEntry*) pEntry ;

	Instance *	pInstance = new Instance ;
	pInstance->pPlugin = pPluginEntry->pPlugin ;
	pInstance->pInstance = pPluginEntry->pPlugin->Startup( pManager ) ;
	return	pInstance ;
}

void S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::ShoutdownProc
		( S3DCompositionManager * pManager, void * pPluginInstance )
{
	Instance *	pInstance = reinterpret_cast<Instance*>( pPluginInstance ) ;
	pInstance->pPlugin->Shoutdown( pManager, pInstance->pInstance ) ;
	delete	pInstance ;
}

const S3DSceneComposerPluginDescriptor *
	S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::GetDescriptorTableProc
		( void * pPluginInstance, size_t& nTableSize )
{
	Instance *	pInstance = reinterpret_cast<Instance*>( pPluginInstance ) ;
	nTableSize = pInstance->pPlugin->m_aDescTable.GetLength() ;
	return	pInstance->pPlugin->m_aDescTable.GetConstArray() ;
}

void * S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::OnStartSceneProc
	( void * pPluginInstance,
		const S3DSceneComposerPluginSceneInfo& scpsi )
{
	Instance *	pInstance = reinterpret_cast<Instance*>( pPluginInstance ) ;
	return	pInstance->pPlugin->StartScene( pInstance->pInstance, scpsi ) ;
}

void S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::OnEndSceneProc
	( void * pPluginInstance, void * pSceneInstance )
{
	Instance *	pInstance = reinterpret_cast<Instance*>( pPluginInstance ) ;
	pInstance->pPlugin->EndScene( pInstance->pInstance, pSceneInstance ) ;
}

void S3D_PLUGIN_STDCALL S3DSceneComposerDynamicPlugin::OnStartCompositionProc
	( void * pPluginInstance, void * pSceneInstance,
		S3DSceneComposer::Composition * pComposition )
{
	Instance *	pInstance = reinterpret_cast<Instance*>( pPluginInstance ) ;
	pInstance->pPlugin->StartComposition
		( pInstance->pInstance, pSceneInstance, pComposition ) ;
}

// プラグイン開始処理実装
//////////////////////////////////////////////////////////////////////////////
void * S3DSceneComposerDynamicPlugin::Startup
	( S3DCompositionManager * pManager )
{
	return	nullptr ;
}

// プラグイン後始末処理実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerDynamicPlugin::Shoutdown
		( S3DCompositionManager * pManager, void * pPluginInstance )
{
}

// プラグイン・シーン開始処理実装
//////////////////////////////////////////////////////////////////////////////
void * S3DSceneComposerDynamicPlugin::StartScene
	( void * pPluginInstance, const S3DSceneComposerPluginSceneInfo& scpsi )
{
	return	nullptr ;
}

// プラグイン・シーン終了処理実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerDynamicPlugin::EndScene
	( void * pPluginInstance, void * pSceneInstance )
{
}

// プラグイン・コンポジション開始処理実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposerDynamicPlugin::StartComposition
	( void * pPluginInstance,
		void * pSceneInstance, S3DSceneComposer::Composition * pComposition )
{
}

