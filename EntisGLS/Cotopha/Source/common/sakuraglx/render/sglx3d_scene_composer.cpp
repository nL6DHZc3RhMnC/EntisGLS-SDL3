
#include <loquaty.h>
#include <loquaty/gls4_loquaty.h>
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuraglx/render/sglx3d_scene_composer.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <sakuraglx/render/sglx3d_scene_item_curve.h>
#include <sakuraglx/render/sglx3d_scene_edit_mesh.h>
#include <sakuraglx/render/sglx3d_scene_effector.h>
#include <sakuraglx/render/sglx3d_scene_rsrc_proc.h>
#include <sakuraglx/render/sglx3d_scene_vr_item.h>
#include <sakuragl/sgl2d/sgl_spline_curve.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// 標準的なリソース種別
//////////////////////////////////////////////////////////////////////////////

const wchar_t * const
	S3DSceneComposer::m_pwszResourceType
			[S3DSceneComposer::resourceTypeCount] =
{
	L"folder",
	L"image",
	L"audio",
	L"movie",
	L"model",
	L"pose_lib",
	L"material_lib",
	L"shader_def",
	L"basic_form",
	L"script_rosetta",
	L"script_antirrhinum",
	L"binary",
	L"script_loquaty",
} ;

S3DSceneComposer::ResourceType
	S3DSceneComposer::GetResourceType( const wchar_t * pwszType )
{
	for ( int i = 0; i < resourceTypeCount; i ++ )
	{
		if ( SString::Compare( pwszType, m_pwszResourceType[i] ) == 0 )
		{
			return	(ResourceType) i ;
		}
	}
	return	resourceTypeInvalid ;
}


//////////////////////////////////////////////////////////////////////////////
// OnExtendNotify コマンド
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DSceneComposer::CmdInitializeItem	= L"InitializeItem" ;
const wchar_t *	S3DSceneComposer::CmdFinishItem		= L"FinishItem" ;
const wchar_t *	S3DSceneComposer::CmdStartItem		= L"StartItem" ;
const wchar_t *	S3DSceneComposer::CmdStopItem		= L"StopItem" ;



//////////////////////////////////////////////////////////////////////////////
// ユーザー・シェーダ―
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::UserShader::NotifyContainer, ESLObject )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::UserShader, ShaderDescriptor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::UserShader::UserShader( const wchar_t * pwszShaderID )
	: m_idShader( pwszShaderID )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::UserShader::~UserShader( void )
{
}

// シェーダ―コンパイル／コンパイル済み取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DSceneComposer::UserShader::LoadShaderFor
	( S3DRenderDevice * pDev,
		SSystem::SString * pstrErrMsg, SSystem::SString * pstrErrSource )
{
	m_csSync.Lock() ;
	NotifyContainer *	pnc = m_psoaShaders.GetAs( pDev ) ;
	if ( pnc != nullptr )
	{
		m_csSync.Unlock() ;
		return	pnc->GetShader() ;
	}
	m_csSync.Unlock() ;
	//
	S3DCustomShader *	pShader =
			pDev->BuildCustomShader
				( m_idShader, this, nullptr, pstrErrMsg, pstrErrSource ) ;
	if ( pShader == nullptr )
	{
		ESLTrace( "error shader: \'%s\'\n",
					m_idShader.ToCharArray().GetConstArray() ) ;
	}
	m_csSync.Lock() ;
	pnc = new NotifyContainer( this, pDev, pShader ) ;
	pDev->AddNotifyObject( pnc ) ;
	m_psoaShaders.Add( pDev, pnc ) ;
	m_csSync.Unlock() ;
	return	pShader ;
}

// シェーダ―参照削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::UserShader::ReleaseAllShaderRef( void )
{
	m_csSync.Lock() ;
	m_psoaShaders.RemoveAll() ;
	m_csSync.Unlock() ;
}

// デバイスの削除前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::UserShader::OnReleaseDevice( S3DRenderDevice * pDev )
{
	m_csSync.Lock() ;
	m_psoaShaders.RemoveAs( pDev ) ;
	m_csSync.Unlock() ;
}



//////////////////////////////////////////////////////////////////////////////
// Antirrhinum モジュール・マネージャ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::AGLSubManager, AGLModuleManager )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::AGLSubManager::AGLSubManager( void )
	: m_pComposer( nullptr )
{
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::AGLSubManager::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

S3DSceneComposer * S3DSceneComposer::AGLSubManager::GetSceneComposer( void ) const
{
	return	m_pComposer ;
}

// 参照マネージャー追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::AGLSubManager::AddReferenceManager
				( AntirrhinumGL::AGLModuleManager * pManager )
{
	ESLAssert( m_refManager.FindPtr( pManager ) < 0 ) ;
	m_refManager.Add( pManager ) ;
}

// 参照マネージャー削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::AGLSubManager::DetachReferenceManager
				( AntirrhinumGL::AGLModuleManager * pManager )
{
	ssize_t	i = m_refManager.FindPtr( pManager ) ;
	if ( i >= 0 )
	{
		m_refManager.RemoveAt( (size_t) i ) ;
	}
}

// 読み込み済みスクリプト取得
//（取得に成功した場合には UnloadScript 呼び出しで参照解放）
//////////////////////////////////////////////////////////////////////////////
AntirrhinumGL::AGLModule *
	S3DSceneComposer::AGLSubManager::GetLoadedScript( const wchar_t * pwszFileName )
{
	AntirrhinumGL::AGLModule *
		pModule = AGLModuleManager::GetLoadedScript( pwszFileName ) ;
	if ( pModule != nullptr )
	{
		return	pModule ;
	}
	m_csSync.Lock() ;
	SReferenceArray<AntirrhinumGL::AGLModuleManager>::Iterator	iter( &m_refManager, 0 ) ;
	while ( iter.HasNext() )
	{
		AntirrhinumGL::AGLModuleManager *	pManager = iter.Next() ;
		m_csSync.Unlock() ;
		if ( pManager != nullptr )
		{
			pModule = pManager->GetLoadedScript( pwszFileName ) ;
			if ( pModule != nullptr )
			{
				return	pModule ;
			}
		}
		m_csSync.Lock() ;
	}
	m_csSync.Unlock() ;
	return	nullptr ;
}

// スクリプト解放
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	S3DSceneComposer::AGLSubManager::UnloadScript
					( AntirrhinumGL::AGLModule * pModule )
{
	if ( AGLModuleManager::IsValidModule( pModule ) )
	{
		return	AGLModuleManager::UnloadScript( pModule ) ;
	}
	m_csSync.Lock() ;
	SReferenceArray<AntirrhinumGL::AGLModuleManager>::Iterator	iter( &m_refManager, 0 ) ;
	while ( iter.HasNext() )
	{
		AntirrhinumGL::AGLModuleManager *	pManager = iter.Next() ;
		m_csSync.Unlock() ;
		if ( (pManager != nullptr)
			&& pManager->IsValidModule( pModule ) )
		{
			return	pManager->UnloadScript( pModule ) ;
		}
		m_csSync.Lock() ;
	}
	m_csSync.Unlock() ;
	return	errFailed ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface *
	S3DSceneComposer::AGLSubManager::OpenScriptFile( const wchar_t * pwszFileName )
{
	if ( m_pComposer != nullptr )
	{
		SFileInterface *	pFile = m_pComposer->OpenAssetFile( pwszFileName ) ;
		if ( pFile != nullptr )
		{
			return	pFile ;
		}
	}
	return	AGLModuleManager::OpenScriptFile( pwszFileName ) ;
}



//////////////////////////////////////////////////////////////////////////////
// リソースコンテナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ResourceProcedure, ESLObject )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ImageRsrcProcedure, ResourceProcedure )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ResourceContainer, SSyncReference )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ResourceContainer::ResourceContainer( void )
	: m_pRsrcProc( nullptr ), m_flagPending( false ), m_pAppData( nullptr )
{
}

S3DSceneComposer::ResourceContainer::ResourceContainer( const ResourceContainer& rsrc )
	: SSyncReference( rsrc ),
		m_strType( rsrc.m_strType ),
		m_strSrcFile( rsrc.m_strSrcFile ),
		m_strTreePath( rsrc.m_strTreePath ),
		m_pRsrcProc( nullptr ), m_flagPending( false ), m_pAppData( nullptr )
{
}

S3DSceneComposer::ResourceContainer::ResourceContainer
	( SObject * pRef, const wchar_t * pwszType,
		const wchar_t * pwszSrcFile, const wchar_t * pwszTreePath,
		const SSystem::SXMLDocument * pxmlOptions )
	: SSyncReference( pRef ),
		m_strType( pwszType ),
		m_strSrcFile( pwszSrcFile ),
		m_strTreePath( pwszTreePath ),
		m_pRsrcProc( nullptr ), m_flagPending( false ), m_pAppData( nullptr )
{
	if ( pxmlOptions != nullptr )
	{
		m_xmlOptions = *pxmlOptions ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ResourceContainer::~ResourceContainer( void )
{
	delete	m_pRsrcProc ;
	delete	m_pAppData ;
}

// 種別
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ResourceContainer::IsResourceType
			( S3DSceneComposer::ResourceType type ) const
{
	return	m_strType == m_pwszResourceType[type] ;
}

void S3DSceneComposer::ResourceContainer::SetType( const wchar_t * pwszType )
{
	m_strType = pwszType ;
}

// ソースファイル名
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceContainer::SetSourceFile( const wchar_t * pwszSrcFile )
{
	m_strSrcFile = pwszSrcFile ;
}

// ツリーパス（編集用）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceContainer::SetTreePath( const wchar_t * pwszPath )
{
	m_strTreePath = pwszPath ;
}

// プロシージャル
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceContainer::SetProcedure
	( S3DSceneComposer::ResourceProcedure * pProc )
{
	delete	m_pRsrcProc ;
	m_pRsrcProc = pProc ;
	m_flagPending = (pProc != nullptr) ;
}

S3DSceneComposer::ResourceProcedure *
	S3DSceneComposer::ResourceContainer::GetProcedure( void ) const
{
	return	m_pRsrcProc ;
}

bool S3DSceneComposer::ResourceContainer::CreateProceduralRsrc
		( S3DSceneComposer::ResourceAssets& assets )
{
	if ( m_pRsrcProc != nullptr )
	{
		ESLObject *	pRsrc = nullptr ;
		if ( m_pRsrcProc->CreateResource
			( pRsrc, assets ) == sglErrPending )
		{
			m_flagPending = true ;
			return	false ;
		}
		assets.ReloadResource( this, pRsrc ) ;
		m_flagPending = false ;
		return	true ;
	}
	else
	{
		m_flagPending = false ;
		return	true ;
	}
}

bool S3DSceneComposer::ResourceContainer::RecreateProceduralRsrc
			( S3DSceneComposer::ResourceAssets& assets )
{
	if ( m_pRsrcProc != nullptr )
	{
		ESLObject *	pRsrc = nullptr ;
		if ( m_pRsrcProc->CreateResource
			( pRsrc, assets ) == sglErrPending )
		{
			return	false ;
		}
		assets.ReloadResource( this, pRsrc ) ;
		m_flagPending = false ;
		return	true ;
	}
	return	false ;
}

bool S3DSceneComposer::ResourceContainer::IsPendingProceduralRsrc( void ) const
{
	return	m_flagPending ;
}

void S3DSceneComposer::ResourceContainer::SaveProceduralOption( void )
{
	if ( m_pRsrcProc != nullptr )
	{
		m_pRsrcProc->FormatParameter( m_xmlOptions ) ;
	}
}

// アプリケーションデータ
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceContainer::SetApplicationData( ESLObject * pAppData )
{
	delete	m_pAppData ;
	m_pAppData = pAppData ;
}



//////////////////////////////////////////////////////////////////////////////
// リソース資産
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::ResourceAssets, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ResourceAssets::ResourceAssets( void )
{
	m_pAppData = nullptr ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ResourceAssets::~ResourceAssets( void )
{
	Release() ;
}

// 全リソース削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::Release( void )
{
	m_ssoaResources.RemoveAll() ;
	m_arrRefAssets.RemoveAll() ;
	delete	m_pAppData ;
	m_pAppData = nullptr ;
	//
	m_libPose.Release() ;
	m_libTexture.Release() ;
	m_libMaterial.Release() ;
	m_aglManager.UnloadAllScript() ;
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_aglManager.AttachSceneComposer( pComposer ) ;
}

S3DSceneComposer * S3DSceneComposer::ResourceAssets::GetSceneComposer( void ) const
{
	return	m_aglManager.GetSceneComposer() ;
}

// 参照キャッシュ追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::AddReferenceAssets
		( S3DSceneComposer::ResourceAssets * pRef )
{
	ESLAssert( pRef != nullptr ) ;
	m_arrRefAssets.Add( pRef ) ;
	//
	m_libPose.AddReferenceLibrary( &(pRef->m_libPose) ) ;
	m_libTexture.AddReferenceLibrary( &(pRef->m_libTexture) ) ;
	m_libMaterial.AddReferenceLibrary( &(pRef->m_libMaterial) ) ;
	m_aglManager.AddReferenceManager( &(pRef->m_aglManager) ) ;
}

// 参照キャッシュ削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::DetachReferenceAssets
		( S3DSceneComposer::ResourceAssets * pRef )
{
	ssize_t	iRef = m_arrRefAssets.FindPtr( pRef ) ;
	if ( iRef >= 0 )
	{
		m_arrRefAssets.RemoveAt( (size_t) iRef ) ;
	}
	m_libPose.DetachReferenceLibraryOf( &(pRef->m_libPose) ) ;
	m_libTexture.DetachReferenceLibraryOf( &(pRef->m_libTexture) ) ;
	m_libMaterial.DetachReferenceLibraryOf( &(pRef->m_libMaterial) ) ;
	m_aglManager.DetachReferenceManager( &(pRef->m_aglManager) ) ;
}

// 参照キャッシュ全削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::ClearAllReferenceAssets( void )
{
	m_arrRefAssets.RemoveAll() ;
	//
	m_libPose.DetachAllReferenceLibrarys() ;
	m_libTexture.DetachAllReferenceLibrarys() ;
	m_libMaterial.DetachAllReferenceLibrarys() ;
}

// 特定クラスリソースID列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::EnumerateResourceIDsAs
	( SSystem::SObjectArray<SSystem::SString>& aStrSet,
				const ESLRuntimeClass& rtClass, bool flagRefAsserts ) const
{
	for ( size_t i = 0; i < m_ssoaResources.GetLength(); i ++ )
	{
		ResourceContainer *	prc = m_ssoaResources.GetAt( i ) ;
		if ( prc != nullptr )
		{
			SObject *	pRsrc = prc->GetReference() ;
			if ( pRsrc != nullptr )
			{
				if ( pRsrc->IsKindOf( rtClass ) )
				{
					const SString *	pstrKey = m_ssoaResources.GetTagAt( i ) ;
					if ( pstrKey != nullptr )
					{
						aStrSet.Add( new SString( *pstrKey ) ) ;
					}
				}
			}
		}
	}
	if ( flagRefAsserts )
	{
		for ( size_t i = 0; i < m_arrRefAssets.GetLength(); i ++ )
		{
			ResourceAssets *	pRefAssets = m_arrRefAssets.GetAt( i ) ;
			if ( pRefAssets != nullptr )
			{
				pRefAssets->EnumerateResourceIDsAs( aStrSet, rtClass ) ;
			}
		}
	}
}

// 特定タイプリソースID列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::EnumerateResourceIDsAs
	( SSystem::SObjectArray<SSystem::SString>& aStrSet,
			S3DSceneComposer::ResourceType rsrcType, bool flagRefAsserts ) const
{
	for ( size_t i = 0; i < m_ssoaResources.GetLength(); i ++ )
	{
		ResourceContainer *	prc = m_ssoaResources.GetAt( i ) ;
		if ( prc != nullptr )
		{
			if ( prc->IsResourceType( rsrcType ) )
			{
				const SString *	pstrKey = m_ssoaResources.GetTagAt( i ) ;
				if ( pstrKey != nullptr )
				{
					aStrSet.Add( new SString( *pstrKey ) ) ;
				}
			}
		}
	}
	if ( flagRefAsserts )
	{
		for ( size_t i = 0; i < m_arrRefAssets.GetLength(); i ++ )
		{
			ResourceAssets *	pRefAssets = m_arrRefAssets.GetAt( i ) ;
			if ( pRefAssets != nullptr )
			{
				pRefAssets->EnumerateResourceIDsAs( aStrSet, rsrcType ) ;
			}
		}
	}
}

// テクスチャ列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::EnumerateTextureStringSet
	( SSystem::SStringArray& aStrSet ) const
{
	EnumerateTextureStringSetOf( aStrSet, &m_libTexture ) ;
	aStrSet.SortAscending() ;
}

void S3DSceneComposer::ResourceAssets:: EnumerateTextureStringSetOf
	( SSystem::SStringArray& aStrSet,
				const S3DTextureLibrary * pTextureLib )
{
	if ( pTextureLib == nullptr )
	{
		return ;
	}
	const SString&	strLibName = pTextureLib->GetLibraryName() ;
	size_t	nCount = pTextureLib->GetTextureCount() ;
	size_t	 i ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszID = pTextureLib->GetTextureIdentityAt( i ) ;
		if ( pwszID != nullptr )
		{
			if ( strLibName.IsEmpty() )
			{
				aStrSet.Add( new SString( pwszID ) ) ;
			}
			else
			{
				aStrSet.Add( new SString( strLibName + L"#" + pwszID ) ) ;
			}
		}
	}
	S3DTextureLibrary *	pParentLib = pTextureLib->GetParentLibrary() ;
	if ( pParentLib != nullptr )
	{
		EnumerateTextureStringSetOf( aStrSet, pParentLib ) ;
	}
	nCount = pTextureLib->GetReferenceLibraryCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DTextureLibrary *
				pRefLib = pTextureLib->GetReferenceLibraryAt( i ) ;
		if ( pRefLib != nullptr )
		{
			EnumerateTextureStringSetOf( aStrSet, pRefLib ) ;
		}
	}
}

// マテリアル列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::EnumerateMaterialStringSet
	( SSystem::SStringArray& aStrSet ) const
{
	EnumerateMaterialStringSetOf( aStrSet, &m_libMaterial ) ;
	aStrSet.SortAscending() ;
}

void S3DSceneComposer::ResourceAssets::EnumerateMaterialStringSetOf
	( SSystem::SStringArray& aStrSet,
				const S3DMaterialLibrary *	pMaterialLib )
{
	if ( pMaterialLib == nullptr )
	{
		return ;
	}
	const SString&	strLibName = pMaterialLib->GetLibraryName() ;
	size_t	nCount = pMaterialLib->GetMaterialCount() ;
	size_t	 i ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszID = pMaterialLib->GetMaterialIdentityAt( i ) ;
		if ( pwszID != nullptr )
		{
			if ( strLibName.IsEmpty() )
			{
				aStrSet.Add( new SString( pwszID ) ) ;
			}
			else
			{
				aStrSet.Add( new SString( strLibName + L"#" + pwszID ) ) ;
			}
		}
	}
	S3DMaterialLibrary *	pParentLib = pMaterialLib->GetParentLibrary() ;
	if ( pParentLib != nullptr )
	{
		EnumerateMaterialStringSetOf( aStrSet, pParentLib ) ;
	}
	nCount = pMaterialLib->GetReferenceLibraryCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DMaterialLibrary *
				pRefLib = pMaterialLib->GetReferenceLibraryAt( i ) ;
		if ( pRefLib != nullptr )
		{
			EnumerateMaterialStringSetOf( aStrSet, pRefLib ) ;
		}
	}
}

// ポーズ列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::EnumeratePoseStringSet
	( SSystem::SStringArray& aStrSet ) const
{
	EnumeratePoseStringSetOf( aStrSet, &m_libPose ) ;
}

void S3DSceneComposer::ResourceAssets::EnumeratePoseStringSetOf
	( SSystem::SStringArray& aStrSet,
					const S3DModelPoseLibrary *	pPoseLib )
{
	if ( pPoseLib == nullptr )
	{
		return ;
	}
	const SString&	strLibName = pPoseLib->GetLibraryName() ;
	size_t	nCount = pPoseLib->GetPoseCount() ;
	size_t	 i ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszID = pPoseLib->GetPoseIdentityAt( i ) ;
		if ( pwszID != nullptr )
		{
			if ( strLibName.IsEmpty() )
			{
				aStrSet.Add( new SString( pwszID ) ) ;
			}
			else
			{
				aStrSet.Add( new SString( strLibName + L"#" + pwszID ) ) ;
			}
		}
	}
	S3DModelPoseLibrary *	pParentLib = pPoseLib->GetParentLibrary() ;
	if ( pParentLib != nullptr )
	{
		EnumeratePoseStringSetOf( aStrSet, pParentLib ) ;
	}
	nCount = pPoseLib->GetReferenceLibraryCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DModelPoseLibrary *
				pRefLib = pPoseLib->GetReferenceLibraryAt( i ) ;
		if ( pRefLib != nullptr )
		{
			EnumeratePoseStringSetOf( aStrSet, pRefLib ) ;
		}
	}
}

// スクリプトラベル列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::EnumerateScriptLabels
	( SSystem::SObjectArray<SSystem::SString>& aStrSet, bool flagRefAsserts ) const
{
	for ( size_t i = 0; i < m_ssoaResources.GetLength(); i ++ )
	{
		ResourceContainer *	prc = m_ssoaResources.GetAt( i ) ;
		if ( prc == nullptr )
		{
			continue ;
		}
		const SString *	pstrKey = m_ssoaResources.GetTagAt( i ) ;
		if ( pstrKey == nullptr )
		{
			continue ;
		}
		AntirrhinumGL::AGLModulePtr *
			pModulePtr = prc->GetResource<AntirrhinumGL::AGLModulePtr>() ;
		if ( pModulePtr == nullptr )
		{
			continue ;
		}
		AntirrhinumGL::AGLModule *	pModule = pModulePtr->GetModule() ;
		if ( pModule == nullptr )
		{
			continue ;
		}
		const AntirrhinumGL::AGLModule::CodeArray *
					pCodeArray = pModule->GetRootCodeArray() ;
		if ( pCodeArray == nullptr )
		{
			continue ;
		}
		for ( size_t j = 0; j < pCodeArray->m_ssaLabelIndex.GetLength(); j ++ )
		{
			const SString *	pstrLabel = pCodeArray->m_ssaLabelIndex.GetTagAt(j) ;
			if ( pstrLabel == nullptr )
			{
				continue ;
			}
			aStrSet.Add( new SString( *pstrKey + L"#" + *pstrLabel ) ) ;
		}
	}
	if ( flagRefAsserts )
	{
		for ( size_t i = 0; i < m_arrRefAssets.GetLength(); i ++ )
		{
			ResourceAssets *	pRefAssets = m_arrRefAssets.GetAt( i ) ;
			if ( pRefAssets != nullptr )
			{
				pRefAssets->EnumerateScriptLabels( aStrSet, true ) ;
			}
		}
	}
}

// 文字列配列のソートと重複要素の削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::SortStringSet
	( SSystem::SStringArray& aStrSet )
{
	for ( size_t i = 0; i < aStrSet.GetLength(); i ++ )
	{
		SString *	pstr0 = aStrSet.GetAt( i ) ;
		if ( pstr0 == nullptr )
		{
			continue ;
		}
		SString *	pstrMin = pstr0 ;
		size_t		iMin = i ;
		for ( size_t j = i + 1; j < aStrSet.GetLength(); j ++ )
		{
			SString *	pstr1 = aStrSet.GetAt( j ) ;
			if ( pstr1 == nullptr )
			{
				continue ;
			}
			int	cmp = pstrMin->Compare( *pstr1 ) ;
			if ( cmp == 0 )
			{
				aStrSet.SetAt( j, nullptr ) ;
			}
			else if ( cmp > 0 )
			{
				pstrMin = pstr1 ;
				iMin = j ;
			}
		}
		aStrSet.Swap( i, iMin ) ;
	}
	aStrSet.TrimEmpty() ;
}

// モデルリソース
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer *
	S3DSceneComposer::ResourceAssets::GetModelAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<S3DModelBuffer>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// 画像リソース
//////////////////////////////////////////////////////////////////////////////
SGLImageObject *
	S3DSceneComposer::ResourceAssets::GetImageAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<SGLImageObject>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// オーディオリソース
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayer *
	S3DSceneComposer::ResourceAssets::GetAudioAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<SGLAudioPlayer>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// 動画リソース
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayer *
	S3DSceneComposer::ResourceAssets::GetMovieAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<SGLMediaPlayer>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// ポーズライブラリ
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary *
	S3DSceneComposer::ResourceAssets::GetPoseLibraryAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<S3DModelPoseLibrary>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// マテリアルライブラリ
//////////////////////////////////////////////////////////////////////////////
S3DMaterialLibrary *
	S3DSceneComposer::ResourceAssets::GetMaterialLibraryAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<S3DMaterialLibrary>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// シェーダーリソース
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::UserShader *
	S3DSceneComposer::ResourceAssets::GetUserShaderAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<S3DSceneComposer::UserShader>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// フォームリソース
//////////////////////////////////////////////////////////////////////////////
SGLBasicFormParser *
	S3DSceneComposer::ResourceAssets::GetBasicFormAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<SGLBasicFormParser>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// バイナリリソース
//////////////////////////////////////////////////////////////////////////////
SSystem::SByteBuffer *
	S3DSceneComposer::ResourceAssets::GetBinaryResourceAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = GetResourceContainerAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	ESLTypeCast<SByteBuffer>( prc->GetReference() ) ;
	}
	return	nullptr ;
}

// リソースコンテナ
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ResourceContainer *
	S3DSceneComposer::ResourceAssets::GetResourceContainerAs( const wchar_t * pwszID ) const
{
	ResourceContainer *	prc = m_ssoaResources.GetAs( pwszID ) ;
	if ( prc != nullptr )
	{
		return	prc ;
	}
	for ( size_t i = 0; i < m_arrRefAssets.GetLength(); i ++ )
	{
		ResourceAssets *	pRefAssets = m_arrRefAssets.GetAt( i ) ;
		if ( pRefAssets != nullptr )
		{
			prc = pRefAssets->GetResourceContainerAs( pwszID ) ;
			if ( prc != nullptr )
			{
				return	prc ;
			}
		}
	}
	return	nullptr ;
}

// リソース逆引き
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	S3DSceneComposer::ResourceAssets::GetContainerIdentityOf
		( ResourceContainer * prcTarget ) const
{
	size_t	nCount = m_ssoaResources.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( m_ssoaResources.GetAt( i ) == prcTarget )
		{
			const SString *	pstrTag = m_ssoaResources.GetTagAt( i ) ;
			if ( pstrTag != nullptr )
			{
				return	*pstrTag ;
			}
			return	nullptr ;
		}
	}
	for ( size_t i = 0; i < m_arrRefAssets.GetLength(); i ++ )
	{
		ResourceAssets *	pRefAssets = m_arrRefAssets.GetAt( i ) ;
		if ( pRefAssets != nullptr )
		{
			const wchar_t *	pwszID =
				pRefAssets->GetContainerIdentityOf( prcTarget ) ;
			if ( pwszID != nullptr )
			{
				return	pwszID ;
			}
		}
	}
	return	nullptr ;
}

const wchar_t *
	S3DSceneComposer::ResourceAssets::GetResourceIdentityOf( ESLObject * pObject ) const
{
	ESLAssert( pObject != nullptr ) ;
	if ( pObject == nullptr )
	{
		return	nullptr ;
	}
	const ESLRuntimeClass *	pClass = pObject->GetESLRuntimeClass() ;
	ESLAssert( pClass != nullptr ) ;
	void *	ptrObj = pObject->DynamicCast( *pClass ) ;
	if ( ptrObj == nullptr )
	{
		return	nullptr ;
	}
	size_t	nCount = m_ssoaResources.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ResourceContainer *	prc = m_ssoaResources.GetAt( i ) ;
		ESLAssert( prc != nullptr ) ;
		if ( prc == nullptr )
		{
			continue ;
		}
		SObject *	pRef = prc->GetReference() ;
		if ( pRef == nullptr )
		{
			continue ;
		}
		if ( ptrObj == pRef->DynamicCast( *pClass ) )
		{
			const SString *	pstrTag = m_ssoaResources.GetTagAt( i ) ;
			if ( pstrTag != nullptr )
			{
				return	*pstrTag ;
			}
			return	nullptr ;
		}
	}
	for ( size_t i = 0; i < m_arrRefAssets.GetLength(); i ++ )
	{
		ResourceAssets *	pRefAssets = m_arrRefAssets.GetAt( i ) ;
		if ( pRefAssets != nullptr )
		{
			const wchar_t *	pwszID =
				pRefAssets->GetResourceIdentityOf( pObject ) ;
			if ( pwszID != nullptr )
			{
				return	pwszID ;
			}
		}
	}
	return	nullptr ;
}

// リソース検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSceneComposer::ResourceAssets::FindResourceFile
			( const wchar_t * pwszFilePath, size_t iFirst ) const
{
	size_t	nCount = m_ssoaResources.GetLength() ;
	for ( size_t i = iFirst; i < nCount; i ++ )
	{
		ResourceContainer *	prc = m_ssoaResources.GetAt( i ) ;
		ESLAssert( prc != nullptr ) ;
		if ( prc == nullptr )
		{
			continue ;
		}
		if ( prc->GetSourceFile().CompareNoCase( pwszFilePath ) == 0 )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// リソースIDを使用可能な文字でかつ重複しないように正規化
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::NormalizeResourceID
									( SSystem::SString& strID ) const
{
	size_t		nBufLen = strID.GetLength() ;
	uint16_t *	pwID = strID.LockBuffer( nBufLen ) ;
	for ( size_t i = 0; i < nBufLen; i ++ )
	{
		uint16_t	w = pwID[i] ;
		if ( ((L'0' <= w) && (w <= L'9'))
			|| ((L'A' <= w) && (w <= L'Z'))
			|| ((L'a' <= w) && (w <= L'z'))
			|| (w == L'_') || (w == L'.')
			|| (w == L'@') || (w >= 0x80) )
		{
		}
		else
		{
			pwID[i] = L'_' ;
		}
	}
	strID.UnlockBuffer( (ssize_t) nBufLen ) ;
	//
	if ( m_ssoaResources.GetAs( strID ) != nullptr )
	{
		SString	strBaseID = strID.GetFileTitlePart() ;
		SString	strExtID = strID.GetFileExtensionPart() ;
		for ( int i = 1; i < 0x10000; i ++ )
		{
			strID = strBaseID ;
			strID += SString( i ) ;
			if ( !strExtID.IsEmpty() )
			{
				strID += L"." ;
				strID += strExtID ;
			}
			if ( m_ssoaResources.GetAs( strID ) == nullptr )
			{
				break ;
			}
		}
	}
}

// リソース列挙
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::ResourceAssets::GetResourceCount( void ) const
{
	return	m_ssoaResources.GetLength() ;
}

S3DSceneComposer::ResourceContainer *
	S3DSceneComposer::ResourceAssets::GetResourceAt( size_t i ) const
{
	return	m_ssoaResources.GetAt( i ) ;
}

const wchar_t * S3DSceneComposer::ResourceAssets::GetResourceIdentityAt( size_t i ) const
{
	const SString *	pstrID = m_ssoaResources.GetTagAt( i ) ;
	if ( pstrID == nullptr )
	{
		return	nullptr ;
	}
	return	*pstrID ;
}

// リソース参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::UpdateAllResourceReference( void )
{
	MaterialLibrary().UpdateAllTextureReference( GetTextureLibrary() ) ;

	for ( size_t i = 0; i < m_ssoaResources.GetLength(); i ++ )
	{
		ResourceContainer *	prc = m_ssoaResources.GetAt( i ) ;
		if ( prc == nullptr )
		{
			continue ;
		}
		SGLBasicFormParser *	pFormParser =
			ESLTypeCast<SGLBasicFormParser>( prc->GetReference() ) ;
		if ( pFormParser != nullptr )
		{
			pFormParser->UpdateResourceRef( &(TextureLibrary()) ) ;
		}
	}
}

// 画像バッファと参照元画像バッファをリストする
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ResourceAssets::CollectReferenceSubImage
	( SSystem::SPointerArray<SGLImageBuffer>& aSubImages,
								const wchar_t * pwszImageID )
{
	S3DSceneComposer::ResourceContainer *
				prc = GetResourceContainerAs( pwszImageID ) ;
	if ( (prc != nullptr) && prc->IsPendingProceduralRsrc() )
	{
		return	sglErrPending ;
	}
	ImageRsrcProcedure *	pImageProc =
		ESLTypeCast<ImageRsrcProcedure>( prc->GetProcedure() ) ;
	if ( pImageProc != nullptr )
	{
		SGLError	err =
			pImageProc->CollectReferenceSubImages( *this, aSubImages ) ;
		if ( err == sglErrPending )
		{
			return	err ;
		}
	}
	SGLImageObject *	pImage = GetImageAs( pwszImageID ) ;
	if ( pImage == nullptr )
	{
		return	sglErrSuccess ;
	}
	size_t	nFrames = pImage->GetFrameCount() ;
	size_t	iSelFrame = pImage->GetSelectedFrame() ;
	for ( size_t j = 0; j < nFrames; j ++ )
	{
		pImage->SelectFrame( j ) ;
		//
		SGLImageBuffer *	pImageBuf = pImage->GetImageBuffer() ;
		if ( pImageBuf != nullptr )
		{
			aSubImages.Add( pImageBuf ) ;
		}
	}
	pImage->SelectFrame( iSelFrame ) ;
	//
	return	sglErrSuccess ;
}

// 参照元の画像バッファの参照を更新する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::UpdateImageReference
	( SGLImageBuffer * pAtlasImageBuf,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	for ( size_t i = 0; i < aSubImages.GetLength(); i ++ )
	{
		SGLImageBuffer *	pSubImage = aSubImages.GetAt( i ) ;
		if ( pSubImage == nullptr )
		{
			continue ;
		}
		SGLImageRect	rectRef ;
		rectRef.x = 0 ;
		rectRef.y = 0 ;
		rectRef.w = (int32_t) pSubImage->width ;
		rectRef.h = (int32_t) pSubImage->height ;
		//
		SGLImageBuffer *	pTemp = pSubImage ;
		while ( (pAtlasImageBuf != pTemp)
			&& (pTemp->ptrRefOriginal != nullptr) )
		{
			rectRef.x += pTemp->rctRefOriginal.x ;
			rectRef.y += pTemp->rctRefOriginal.y ;
			pTemp = pTemp->ptrRefOriginal ;
		}
		ESLAssert( pAtlasImageBuf == pTemp ) ;
		if ( pAtlasImageBuf == pTemp )
		{
			sglMakeReferenceImageBuffer( pSubImage, pAtlasImageBuf, &rectRef ) ;
		}
	}
}

// ポーズライブラリ
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary&
		S3DSceneComposer::ResourceAssets::PoseLibrary( void )
{
	return	m_libPose ;
}

const S3DModelPoseLibrary&
		S3DSceneComposer::ResourceAssets::GetPoseLibrary( void ) const
{
	return	m_libPose ;
}

// テクスチャライブラリ
//////////////////////////////////////////////////////////////////////////////
S3DTextureLibrary& S3DSceneComposer::ResourceAssets::TextureLibrary( void )
{
	return	m_libTexture ;
}

const S3DTextureLibrary& S3DSceneComposer::ResourceAssets::GetTextureLibrary( void ) const
{
	return	m_libTexture ;
}

// マテリアルライブラリ（参照ハブ）
//////////////////////////////////////////////////////////////////////////////
S3DMaterialLibrary& S3DSceneComposer::ResourceAssets::MaterialLibrary( void )
{
	return	m_libMaterial ;
}

const S3DMaterialLibrary& S3DSceneComposer::ResourceAssets::GetMaterialLibrary( void ) const
{
	return	m_libMaterial ;
}

// Antirrhinum モジュール・マネージャー
S3DSceneComposer::AGLSubManager&
	S3DSceneComposer::ResourceAssets::AntirrhinumManager( void )
{
	return	m_aglManager ;
}

const S3DSceneComposer::AGLSubManager&
	S3DSceneComposer::ResourceAssets::GetAntirrhinumManager( void ) const
{
	return	m_aglManager ;
}

// アプリケーションデータ
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::SetApplicationData( ESLObject * pAppData )
{
	delete	m_pAppData ;
	m_pAppData = pAppData ;
}

// リソース登録
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ResourceAssets::RegisterResource
	( const wchar_t * pwszID, SObject * pRsrc,
		const wchar_t * pwszType,
		const wchar_t * pwszSrcFile, const wchar_t * pwszPath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	ResourceContainer *	prc =
		new ResourceContainer
				( pRsrc, pwszType, pwszSrcFile, pwszPath, pxmlOptions ) ;
	m_ssoaResources.SetAs( pwszID, prc ) ;
	//
	OnAddResource( pwszID, pRsrc ) ;
	//
	return	sglErrSuccess ;
}

// リソース追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ResourceAssets::AddResourceAs
	( const wchar_t * pwszID, ESLObject * pRsrc,
		const wchar_t * pwszType,
		const wchar_t * pwszSrcFile, const wchar_t * pwszPath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	ResourceContainer *	prc =
		new ResourceContainer
			( new SSmartObject( pRsrc ),
				pwszType, pwszSrcFile, pwszPath, pxmlOptions ) ;
	m_ssoaResources.SetAs( pwszID, prc ) ;
	//
	OnAddResource( pwszID, pRsrc ) ;
	//
	return	sglErrSuccess ;
}

// リソース削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ResourceAssets::RemoveResourceAs( const wchar_t * pwszID )
{
	ssize_t	i = m_ssoaResources.FindAs( pwszID ) ;
	if ( i < 0 )
	{
		return	sglErrFailed ;
	}
	ResourceContainer *	prc = m_ssoaResources.GetAt( (size_t) i ) ;
	if ( prc != nullptr )
	{
		OnRemoveResource( pwszID, prc->GetReference() ) ;
	}
	m_ssoaResources.RemoveAt( (size_t) i ) ;
	return	sglErrSuccess ;
}

// リソースID変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ResourceAssets::ChangeResourceID
	( const wchar_t * pwszID, S3DSceneComposer::ResourceContainer * pRsrc )
{
	ssize_t	i = m_ssoaResources.FindPtr( pRsrc ) ;
	if ( i >= 0 )
	{
		OnRemoveResource( pwszID, pRsrc->GetReference() ) ;
		//
		ESLVerify( m_ssoaResources.DetachAt(i) == pRsrc ) ;
		m_ssoaResources.Add( pwszID, pRsrc ) ;
		//
		OnAddResource( pwszID, pRsrc->GetReference() ) ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// リソース実体の差し替え
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ResourceAssets::ReloadResource
	( S3DSceneComposer::ResourceContainer * pRsrc, ESLObject * pData )
{
	ssize_t	i = m_ssoaResources.FindPtr( pRsrc ) ;
	if ( i < 0 )
	{
		return	sglErrFailed ;
	}
	const SString *	pstrID = m_ssoaResources.GetTagAt( (size_t) i ) ;
	ESLAssert( pstrID != nullptr ) ;
	if ( pstrID == nullptr )
	{
		return	sglErrFailed ;
	}
	//
	OnRemoveResource( *pstrID, pRsrc->GetReference() ) ;
	//
	pRsrc->SetReference( new SSmartObject( pData ) ) ;
	//
	OnAddResource( *pstrID, pData ) ;
	//
	return	sglErrSuccess ;
}

// リソースの追加時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::OnAddResource
			( const wchar_t * pwszID, ESLObject * pRsrc )
{
	do
	{
		SGLImageObject *	pImage =
			ESLTypeCast<SGLImageObject>( pRsrc ) ;
		if ( pImage != nullptr )
		{
			m_libTexture.AddTextureAs( pwszID, pImage ) ;
			break ;
		}
		S3DModelPoseLibrary *	pPoseLib =
			ESLTypeCast<S3DModelPoseLibrary>( pRsrc ) ;
		if ( pPoseLib != nullptr )
		{
			m_libPose.AddReferenceLibrary( pPoseLib ) ;
			break ;
		}
		S3DMaterialLibrary *	pMaterialLib =
			ESLTypeCast<S3DMaterialLibrary>( pRsrc ) ;
		if ( pMaterialLib != nullptr )
		{
			m_libMaterial.AddReferenceLibrary( pMaterialLib ) ;
			break ;
		}
		S3DModelBuffer *	pModel =
			ESLTypeCast<S3DModelBuffer>( pRsrc ) ;
		if ( pModel != nullptr )
		{
			pModel->GetTextureLibrary().SetLibraryName( pwszID ) ;
			pModel->GetMaterialLibrary().SetLibraryName( pwszID ) ;
			pModel->GetPoseLibrary().SetLibraryName( pwszID ) ;
			m_libTexture.AddReferenceLibrary( &(pModel->GetTextureLibrary()) ) ;
			m_libMaterial.AddReferenceLibrary( &(pModel->GetMaterialLibrary()) ) ;
//			m_libPose.AddReferenceLibrary( &(pModel->GetPoseLibrary()) ) ;
			pModel->GetPoseLibrary().SetParentLibrary( &m_libPose ) ;
			break ;
		}
	}
	while ( false ) ;
}

// リソース削除時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ResourceAssets::OnRemoveResource
			( const wchar_t * pwszID, ESLObject * pRsrc )
{
	do
	{
		SGLImageObject *	pImage =
			ESLTypeCast<SGLImageObject>( pRsrc ) ;
		if ( pImage != nullptr )
		{
			m_libTexture.RemoveTextureAs( pwszID ) ;
			break ;
		}
		S3DModelPoseLibrary *	pPoseLib =
					ESLTypeCast<S3DModelPoseLibrary>( pRsrc ) ;
		if ( pPoseLib != nullptr )
		{
			m_libPose.DetachReferenceLibraryOf( pPoseLib ) ;
			break ;
		}
		S3DMaterialLibrary *	pMaterialLib =
			ESLTypeCast<S3DMaterialLibrary>( pRsrc ) ;
		if ( pMaterialLib != nullptr )
		{
			m_libMaterial.DetachReferenceLibraryOf( pMaterialLib ) ;
			break ;
		}
		S3DModelBuffer *	pModel =
			ESLTypeCast<S3DModelBuffer>( pRsrc ) ;
		if ( pModel != nullptr )
		{
			m_libTexture.DetachReferenceLibraryOf( &(pModel->GetTextureLibrary()) ) ;
			m_libMaterial.DetachReferenceLibraryOf( &(pModel->GetMaterialLibrary()) ) ;
//			m_libPose.DetachReferenceLibraryOf( &(pModel->GetPoseLibrary()) ) ;
			pModel->GetPoseLibrary().SetParentLibrary( nullptr ) ;
			break ;
		}
	}
	while ( false ) ;
}



//////////////////////////////////////////////////////////////////////////////
// コンポジション情報
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::CompositionInfo, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::CompositionInfo::CompositionInfo( void )
{
	m_sizeScreen.w = 1280 ;
	m_sizeScreen.h = 720 ;
	m_projParam.vScreen.x = (float32_t) m_sizeScreen.w * 0.5f ;
	m_projParam.vScreen.y = (float32_t) m_sizeScreen.h * 0.5f ;
	m_projParam.vScreen.z = (float32_t) (m_projParam.vScreen.x
										/ 0.57735026918962576450914878050196) ;
	//
	m_rgbaBack.ui32 = 0x00000000 ;
	m_fFillBack = true ;
	//
	m_fpVisibleNearDistance = 100.0 ;
	m_fpVisibleFarDistance = 5000.0 ;
	//
	m_fEnableFog = false ;
	//
	m_framesTotal = 0 ;
	m_framesPerSec = 60 ;
	m_framesScale = 1 ;
}

S3DSceneComposer::CompositionInfo::CompositionInfo( const CompositionInfo& ci )
	: m_sizeScreen( ci.m_sizeScreen ),
		m_projParam( ci.m_projParam ),
		m_rgbaBack( ci.m_rgbaBack ), m_fFillBack( ci.m_fFillBack ),
		m_fpVisibleNearDistance( ci.m_fpVisibleNearDistance ),
		m_fpVisibleFarDistance( ci.m_fpVisibleFarDistance ),
		m_fEnableFog( ci.m_fEnableFog ),
		m_fogParam( ci.m_fogParam ),
		m_borderParam( ci.m_borderParam ),
		m_framesTotal( ci.m_framesTotal ),
		m_framesPerSec( ci.m_framesPerSec ),
		m_framesScale( ci.m_framesScale ),
		m_xmlComposition( ci.m_xmlComposition )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::CompositionInfo::~CompositionInfo( void )
{
}

// 読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::CompositionInfo::ParseComposition
							( const SSystem::SXMLDocument& xmlTag )
{
	//
	// 表示設定
	//
	m_sizeScreen.w =
		(int32_t) xmlTag.GetAttrIntegerAs
						( L"screen_width", m_sizeScreen.w ) ;
	m_sizeScreen.h =
		(int32_t) xmlTag.GetAttrIntegerAs
						( L"screen_height", m_sizeScreen.h ) ;
	m_projParam.vScreen.x =
		(float32_t) xmlTag.GetAttrRealAs
						( L"pars_center_x", m_projParam.vScreen.x ) ;
	m_projParam.vScreen.y =
		(float32_t) xmlTag.GetAttrRealAs
						( L"pars_center_y", m_projParam.vScreen.y ) ;
	m_projParam.vScreen.z =
		(float32_t) xmlTag.GetAttrRealAs
						( L"screen_z", m_projParam.vScreen.z ) ;
	m_projParam.fpZoom =
		(float32_t) xmlTag.GetAttrRealAs
						( L"projection_zoom", m_projParam.fpZoom ) ;
	m_projParam.fpPixelAspect =
		(float32_t) xmlTag.GetAttrRealAs
				( L"projection_pixel_aspect", m_projParam.fpPixelAspect ) ;
	m_projParam.zNear =
		(float32_t) xmlTag.GetAttrRealAs
						( L"perspective_near", m_projParam.zNear ) ;
	m_projParam.zFar =
		(float32_t) xmlTag.GetAttrRealAs
						( L"perspective_far", m_projParam.zFar ) ;
	m_strDefCamera = xmlTag.GetAttrStringAs( L"default_camera" ) ;
	//
	// 背景色
	//
	m_fFillBack =
		(xmlTag.GetAttrStringAs
			( L"fill_back_color",
				(m_fFillBack ? L"true" : L"false") ) == L"true") ;
	m_rgbaBack.ui32 =
		(uint32_t) xmlTag.GetAttrHexIntegerAs
							( L"back_color", m_rgbaBack.ui32 ) ;
	//
	// 有効表示距離
	//
	m_fpVisibleNearDistance =
		xmlTag.GetAttrRealAs
			( L"visible_near_distance", m_fpVisibleNearDistance ) ;
	m_fpVisibleFarDistance =
		xmlTag.GetAttrRealAs
			( L"visible_far_distance", m_fpVisibleFarDistance ) ;
	//
	// 大域フォッグ
	//
	m_fEnableFog =
		(xmlTag.GetAttrStringAs
			( L"enable_fog",
					(m_fEnableFog ? L"true" : L"false") ) == L"true") ;
	m_fogParam.rgbFog.ui32 =
		(uint32_t) xmlTag.GetAttrHexIntegerAs
						( L"fog_color", m_fogParam.rgbFog.ui32 ) ;
	m_fogParam.zNear =
		xmlTag.GetAttrRealAs( L"fog_near", m_fogParam.zNear ) ;
	m_fogParam.zFar =
		xmlTag.GetAttrRealAs( L"fog_far", m_fogParam.zFar ) ;
	//
	// 輪郭線
	//
	m_borderParam.rgbBorder.ui32 =
		(uint32_t) xmlTag.GetAttrHexIntegerAs
						( L"border_color", m_borderParam.rgbBorder.ui32 ) ;
	m_borderParam.aThickness =
		(float32_t) xmlTag.GetAttrRealAs
						( L"border_thickness_a", m_borderParam.aThickness ) ;
	m_borderParam.bThickness =
		(float32_t) xmlTag.GetAttrRealAs
						( L"border_thickness_b", m_borderParam.bThickness ) ;
	//
	// アニメーション
	//
	m_framesTotal = xmlTag.GetAttrIntegerAs( L"total_frames" ) ;
	m_framesPerSec =
		(uint32_t) xmlTag.GetAttrIntegerAs( L"frames_per_sec" ) ;
	m_framesScale =
		(uint32_t) xmlTag.GetAttrIntegerAs( L"frame_scale" ) ;
	//
	// アイテム
	//
	SXMLDocument *	pxmlSpace = xmlTag.GetElementTagAs( L"space" ) ;
	if ( pxmlSpace != nullptr )
	{
		m_xmlComposition = *pxmlSpace ;
	}
	else
	{
		m_xmlComposition.RemoveAllContents() ;
	}
	return	sglErrSuccess ;
}

// 保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::CompositionInfo::FormatComposition
							( SSystem::SXMLDocument& xmlTag )
{
	//
	// 表示設定
	//
	xmlTag.SetAttrIntegerAs( L"screen_width", m_sizeScreen.w ) ;
	xmlTag.SetAttrIntegerAs( L"screen_height", m_sizeScreen.h ) ;
	xmlTag.SetAttrRealAs( L"pars_center_x", m_projParam.vScreen.x ) ;
	xmlTag.SetAttrRealAs( L"pars_center_y", m_projParam.vScreen.y ) ;
	xmlTag.SetAttrRealAs( L"screen_z", m_projParam.vScreen.z ) ;
	xmlTag.SetAttrRealAs( L"projection_zoom", m_projParam.fpZoom ) ;
	xmlTag.SetAttrRealAs( L"projection_pixel_aspect", m_projParam.fpPixelAspect ) ;
	xmlTag.SetAttrRealAs( L"perspective_near", m_projParam.zNear ) ;
	xmlTag.SetAttrRealAs( L"perspective_far", m_projParam.zFar ) ;
	xmlTag.SetAttributeAs( L"default_camera", m_strDefCamera ) ;
	//
	// 背景色
	//
	xmlTag.SetAttributeAs
		( L"fill_back_color", (m_fFillBack ? L"true" : L"false") ) ;
	xmlTag.SetAttrHexIntegerAs( L"back_color", m_rgbaBack.ui32 ) ;
	//
	// 有効表示距離
	//
	xmlTag.SetAttrRealAs
		( L"visible_near_distance", m_fpVisibleNearDistance ) ;
	xmlTag.SetAttrRealAs
		( L"visible_far_distance", m_fpVisibleFarDistance ) ;
	//
	// 大域フォッグ
	//
	xmlTag.SetAttributeAs
		( L"enable_fog", (m_fEnableFog ? L"true" : L"false") ) ;
	xmlTag.SetAttrHexIntegerAs( L"fog_color", m_fogParam.rgbFog.ui32 ) ;
	xmlTag.SetAttrRealAs( L"fog_near", m_fogParam.zNear ) ;
	xmlTag.SetAttrRealAs( L"fog_far", m_fogParam.zFar ) ;
	//
	// 輪郭線
	//
	xmlTag.SetAttrHexIntegerAs
		( L"border_color", m_borderParam.rgbBorder.ui32 ) ;
	xmlTag.SetAttrRealAs
		( L"border_thickness_a", m_borderParam.aThickness ) ;
	xmlTag.SetAttrRealAs
		( L"border_thickness_b", m_borderParam.bThickness ) ;
	//
	// アニメーション
	//
	xmlTag.SetAttrIntegerAs( L"total_frames", m_framesTotal ) ;
	xmlTag.SetAttrIntegerAs( L"frames_per_sec", m_framesPerSec ) ;
	xmlTag.SetAttrIntegerAs( L"frame_scale", m_framesScale ) ;
	//
	// アイテム
	//
	if ( m_xmlComposition.GetTag() == L"space" )
	{
		xmlTag.AddElement( new SXMLDocument( m_xmlComposition ) ) ;
	}
	return	sglErrSuccess ;
}

// 画面サイズ設定
//////////////////////////////////////////////////////////////////////////////
const SGLSize& S3DSceneComposer::CompositionInfo::GetScreenSize( void ) const
{
	return	m_sizeScreen ;
}

void S3DSceneComposer::CompositionInfo::SetScreenSize( const SGLSize& sizeScreen )
{
	m_sizeScreen = sizeScreen ;
}

// 透視変換設定
//////////////////////////////////////////////////////////////////////////////
const S3DScene::ProjectionParam&
	S3DSceneComposer::CompositionInfo::GetProjectionParam( void ) const
{
	return	m_projParam ;
}

void S3DSceneComposer::CompositionInfo::SetProjectionParam
					( const S3DScene::ProjectionParam& projParam )
{
	m_projParam = projParam ;
}

// 透視変換設定を適用
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CompositionInfo::ApplyProjection
					( S3DScene& scene, const SGLSize& sizeView ) const
{
	float32_t	wZoom = (float32_t) sizeView.w / (float32_t) m_sizeScreen.w ;
	float32_t	hZoom = (float32_t) sizeView.h / (float32_t) m_sizeScreen.h ;
	float32_t	fpZoom = (wZoom < hZoom) ? wZoom : hZoom ;
	//
	S3DScene::ProjectionParam	pp = m_projParam ;
	pp.vScreen.x *= wZoom ;
	pp.vScreen.y *= hZoom ;
	pp.fpZoom *= fpZoom ;
	//
	scene.SetProjection( pp ) ;
}

// デフォルトカメラ
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString&
		S3DSceneComposer::CompositionInfo::GetDefaultCameraID( void ) const
{
	return	m_strDefCamera ;
}

void S3DSceneComposer::CompositionInfo::SetDefaultCameraID( const wchar_t * pwszCamera )
{
	m_strDefCamera = pwszCamera ;
}

// 背景色
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::CompositionInfo::IsEnabledFillBack( void ) const
{
	return	m_fFillBack ;
}

const SGLPalette& S3DSceneComposer::CompositionInfo::GetFillBack( void ) const
{
	return	m_rgbaBack ;
}

void S3DSceneComposer::CompositionInfo::SetFillBack
		( const SGLPalette& rgbaBack, bool fFillBack )
{
	m_rgbaBack = rgbaBack ;
	m_fFillBack = fFillBack ;
}

// 有効表示距離
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::CompositionInfo::GetVisibleNearDistance( void ) const
{
	return	m_fpVisibleNearDistance ;
}

double S3DSceneComposer::CompositionInfo::GetVisibleFarDistance( void ) const
{
	return	m_fpVisibleFarDistance ;
}

void S3DSceneComposer::CompositionInfo::SetVisibleDistance( double fpNear, double fpFar )
{
	m_fpVisibleNearDistance = fpNear ;
	m_fpVisibleFarDistance = fpFar ;
}

// 大域フォッグ
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::CompositionInfo::IsEnabledFog( void ) const
{
	return	m_fEnableFog ;
}

const S3DScene::FogParam&
	S3DSceneComposer::CompositionInfo::GetFog( void ) const
{
	return	m_fogParam ;
}

void S3DSceneComposer::CompositionInfo::SetFog
			( const S3DScene::FogParam& fogParam, bool fFog )
{
	m_fogParam = fogParam ;
	m_fEnableFog = fFog ;
}

// 輪郭線
//////////////////////////////////////////////////////////////////////////////
const S3DScene::OffsetBorderParam&
	S3DSceneComposer::CompositionInfo::GetOffsetBorder( void ) const
{
	return	m_borderParam ;
}

void S3DSceneComposer::CompositionInfo::SetOffsetBorder
					( const S3DScene::OffsetBorderParam& param )
{
	m_borderParam = param ;
}

// アニメーション
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DSceneComposer::CompositionInfo::GetTotalFrameCount( void ) const
{
	return	m_framesTotal ;
}

uint32_t S3DSceneComposer::CompositionInfo::GetFramesPerSecond( void ) const
{
	return	m_framesPerSec ;
}

uint32_t S3DSceneComposer::CompositionInfo::GetFrameRateScale( void ) const
{
	return	m_framesScale ;
}

void S3DSceneComposer::CompositionInfo::SetTotalFrameCount( uint64_t nFrames )
{
	m_framesTotal = nFrames ;
}

void S3DSceneComposer::CompositionInfo::SetFrameRate( uint32_t framesPerSec, uint32_t framesScale )
{
	m_framesPerSec = framesPerSec ;
	m_framesScale = framesScale ;
}

double S3DSceneComposer::CompositionInfo::FrameIndexToSecond( double frame ) const
{
	ESLAssert( m_framesPerSec != 0 ) ;
	if ( m_framesPerSec != 0 )
	{
		return	frame * m_framesScale / m_framesPerSec ;
	}
	return	0.0 ;
}

double S3DSceneComposer::CompositionInfo::FrameIndexFromSecond( double sec ) const
{
	ESLAssert( m_framesScale != 0 ) ;
	if ( m_framesScale != 0 )
	{
		return	sec * m_framesPerSec / m_framesScale ;
	}
	return	0.0 ;
}

// 表示設定を適用
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CompositionInfo::ApplyAllParameters
	( S3DScene& scene, const SGLSize& sizeView ) const
{
	ApplyProjection( scene, sizeView ) ;
	//
	scene.SetBackColor( m_rgbaBack, m_fFillBack ) ;
	scene.SetVisibleDistance
		( m_fpVisibleNearDistance, m_fpVisibleFarDistance ) ;
	scene.SetGlobalFog( m_fogParam, m_fEnableFog ) ;
	scene.SetOffsetBorderParameter( m_borderParam ) ;
}

// コンポジション
//////////////////////////////////////////////////////////////////////////////
const SSystem::SXMLDocument&
	S3DSceneComposer::CompositionInfo::GetComposition( void ) const
{
	return	m_xmlComposition  ;
}

SSystem::SXMLDocument&
	S3DSceneComposer::CompositionInfo::EditComposition( void )
{
	return	m_xmlComposition  ;
}



// コンポジション・インスタンス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::Composition, SpaceSerializer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition::Composition
	( S3DSceneComposer * pComposer,
		const CompositionInfo * pciCreateRef,
		Composition * pOwnerComposition, bool flagParseAsSubComp )
	: SpaceSerializer
		( L"composition", &S3DSceneComposer::SpaceSerializer::m_pscClass )
{
	m_flagsSpaceBehavior |= S3DScene::itemOwnerBehavior ;
	//
	m_pMasterComposer = pComposer ;
	m_pOwnerComposition = pOwnerComposition ;
	m_refCompositionInfo.SetReference( (CompositionInfo*) pciCreateRef ) ;
	m_pTargetScene = nullptr ;
	m_pEditorInteface = nullptr ;
	m_flagParseAsSubComp = flagParseAsSubComp ;
	//
	m_fpCurrentFrame = 0.0 ;
	m_fpNextJumpFrame = 0.0 ;
	m_fpFrameSpeed = 1.0 ;
	m_flagFinished = false ;
	m_flagPlaying = false ;
	m_flagPaused = false ;
	m_nPausedCounter = 0 ;
	m_flagPauseTimer = false ;
	m_flagTimerByFrame = false ;
	m_flagFrameByTimer = true ;
	m_flagFrameSpeedByTimer = false ;
	m_flagEditMode = false ;
	m_flagPostJump = false ;
	//
	m_flagFixTimerInterval = true ;
	if ( pciCreateRef != nullptr )
	{
		m_msecTimerInterval =
			(uint32_t) floor( pciCreateRef->FrameIndexToSecond( 1.0 ) * 1000.0 ) ;
	}
	else
	{
		m_msecTimerInterval = 16 ;
	}
	m_msecOddTimer = 0 ;
}

S3DSceneComposer::Composition::Composition
	( const wchar_t * pwszClassID, const ParamSetClass * pClass,
		S3DSceneComposer * pComposer,
		const CompositionInfo * pciCreateRef,
		Composition * pOwnerComposition, bool flagParseAsSubComp )
	: SpaceSerializer( pwszClassID, pClass )
{
	m_pMasterComposer = pComposer ;
	m_pOwnerComposition = pOwnerComposition ;
	m_refCompositionInfo.SetReference( (CompositionInfo*) pciCreateRef ) ;
	m_pTargetScene = nullptr ;
	m_pEditorInteface = nullptr ;
	m_flagParseAsSubComp = flagParseAsSubComp ;
	//
	m_fpCurrentFrame = 0.0 ;
	m_fpNextJumpFrame = 0.0 ;
	m_fpFrameSpeed = 1.0 ;
	m_flagFinished = false ;
	m_flagPlaying = false ;
	m_flagPaused = false ;
	m_nPausedCounter = 0 ;
	m_flagPauseTimer = false ;
	m_flagTimerByFrame = false ;
	m_flagFrameByTimer = true ;
	m_flagFrameSpeedByTimer = false ;
	m_flagEditMode = false ;
	m_flagPostJump = false ;
	//
	m_flagFixTimerInterval = true ;
	if ( pciCreateRef != nullptr )
	{
		m_msecTimerInterval =
			(uint32_t) floor( pciCreateRef->FrameIndexToSecond( 1.0 ) * 1000.0 ) ;
	}
	else
	{
		m_msecTimerInterval = 16 ;
	}
	m_msecOddTimer = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition::~Composition( void )
{
}

// 表示設定を適用
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::ApplySceneParameters
	( S3DScene& scene, const SGLSize& sizeView ) const
{
	const CompositionInfo *	pci = GetCompositionInfo() ;
	ESLAssert( pci != nullptr ) ;
	if ( pci != nullptr )
	{
		pci->ApplyAllParameters( scene, sizeView ) ;
		//
		S3DScene::Camera *	pDefCamera =
			ESLTypeCast<S3DScene::Camera>
				( GetSceneItemAs( pci->GetDefaultCameraID() ) ) ;
		scene.SetMainCamera( pDefCamera ) ;
	}
	scene.SetEnvironmentMappingSource
		( S3DScene::classLayeredItem1,
			m_emsReflectionSource, m_emsRefractionSource ) ;
	scene.SetEnvironmentMappingSource
		( S3DScene::classLayeredItem2,
			m_emsReflectionSource, m_emsRefractionSource ) ;
}

// ターゲット S3DScene を設定する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::AttachTargetScene( S3DScene * pScene )
{
	m_pTargetScene = pScene ;
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::PrepareToRender
	( S3DRenderDevice * pDevice, S3DScene * pScene, uint32_t nFlags )
{
	m_pTargetScene = pScene ;
	//
	SpaceSerializer::PrepareToRender( pDevice, nFlags ) ;
}

// コンポジション・パースモード
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Composition::IsSubCompositionToParseChildren( void ) const
{
	return	(m_pOwnerComposition != nullptr) || m_flagParseAsSubComp ;
}

// サブコンポジションの親アイテム
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::AttachOwnerItem( S3DScene::Item * pItem )
{
	m_refOwnerItem.SetReference( pItem ) ;
}

S3DScene::Item * S3DSceneComposer::Composition::GetOwnerItem( void ) const
{
	return	m_refOwnerItem.GetReference() ;
}

// グローバル空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::CalcGlobalTransformation
	( S3DDMatrix& matrix, S3DDVector& pos ) const
{
	S3DScene::Item *	pOwner = GetOwnerItem() ;
	if ( pOwner != nullptr )
	{
		pOwner->CalcGlobalTransformation( matrix, pos ) ;
		pos += matrix * m_vLink ;
		matrix *= m_matLink ;
		pos += matrix * m_vCenter ;
		matrix *= m_matTransformation ;
	}
	else
	{
		SpaceSerializer::CalcGlobalTransformation( matrix, pos ) ;
	}
}

const S3DDVector&
		S3DSceneComposer::Composition::CalcGlobalPosition( S3DDVector& pos ) const
{
	S3DDMatrix	matTemp ;
	CalcGlobalTransformation( matTemp, pos ) ;
	return	pos ;
}

// 相対空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::CalcOffsetTransformation
	( S3DDMatrix& matrix, S3DDVector& pos,
					const Space * pStandardSpace ) const
{
	CalcGlobalTransformation( matrix, pos ) ;
	//
	if ( pStandardSpace != nullptr )
	{
		S3DDMatrix	matStandard ;
		S3DDVector	vStandard ;
		pStandardSpace->CalcGlobalTransformation( matStandard, vStandard ) ;
		//
		S3DDMatrix	matIStandard ;
		matIStandard.InverseOf( matStandard ) ;
		//
		pos -= vStandard ;
		matIStandard.RevolveVector( pos ) ;
		matrix = matIStandard * matrix ;
	}
}

// コンポジション取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
		S3DSceneComposer::Composition::GetComposition( void ) const
{
	return	(Composition*) this ;
}

// シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DSceneComposer::Composition::GetScene( void ) const
{
	if ( m_pTargetScene != nullptr )
	{
		return	m_pTargetScene ;
	}
	if ( m_pOwnerComposition != nullptr )
	{
		return	m_pOwnerComposition->GetScene() ;
	}
	else
	{
		return	S3DScene::Space::GetScene() ;
	}
}

// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::GetItemLinkTransformation
		( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const
{
	S3DSubCompositionSerializer *
		pOwner = m_refOwnerItem.GetRef<S3DSubCompositionSerializer>() ;
	if ( pOwner != nullptr )
	{
		pOwner->GetGlobalTransformation( matLink, vLinkPos ) ;
		return ;
	}
	SpaceSerializer::GetItemLinkTransformation( matLink, vLinkPos ) ;
}

// 空間行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::GetGlobalTransformation
	( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const
{
	S3DSubCompositionSerializer *
		pOwner = m_refOwnerItem.GetRef<S3DSubCompositionSerializer>() ;
	if ( pOwner != nullptr )
	{
		S3DDMatrix	matParent( 1, 1, 1 ) ;
		S3DDVector	vParent( 0, 0, 0 ) ;
		pOwner->GetGlobalTransformation( matParent, vParent ) ;
		//
		if ( m_pSpace != nullptr )
		{
			matGlobal = matParent * m_pSpace->m_matTransformation ;
			vGlobalPos = matParent * m_pSpace->m_vCenter + vParent ;
		}
		else
		{
			matGlobal = matParent * m_matTransformation ;
			vGlobalPos = matParent * m_vCenter + vParent ;
		}
	}
	else
	{
		SpaceSerializer::GetGlobalTransformation( matGlobal, vGlobalPos ) ;
	}
}

// 空間色効果取得（乗算色α要素に不透明度取得）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::GetGlobalColorEffect( S3DColor& clrEffect ) const
{
	S3DSubCompositionSerializer *
		pOwner = m_refOwnerItem.GetRef<S3DSubCompositionSerializer>() ;
	if ( pOwner != nullptr )
	{
		S3DColor	clrParent ;
		pOwner->GetGlobalColorEffect( clrParent ) ;
		//
		S3DColor	clrSpace ;
		if ( m_pSpace != nullptr )
		{
			m_pSpace->CalcGlobalColorEffect( clrSpace ) ;
		}
		else
		{
			CalcGlobalColorEffect( clrSpace ) ;
		}
		clrEffect = clrParent * clrSpace ;
	}
	else
	{
		SpaceSerializer::GetGlobalColorEffect( clrEffect ) ;
	}
}

// エディター取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionEditorInterface *
	S3DSceneComposer::Composition::GetEditor( void ) const
{
	return	m_pEditorInteface ;
}

// フレームの初期化処理（デフォルトは0フレームへのシーク）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::InitializeFrameParameters( void )
{
	SpaceSerializer::InitializeFrameParameters() ;
	//
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		ItemSerializer *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( pItem != nullptr )
		{
			pItem->InitializeFrameParameters() ;
		}
	}
	m_fpCurrentFrame = 0.0 ;
	//
	OnUpdateFrame( 0.0, seekJumpReset ) ;
}

// レンダリング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::OnSetupSceneSettings
	( S3DScene& scene,
		const SGLSize& sizeFrame,
		SGLSecondaryViewProducer * psvp )
{
	SpaceSerializer::OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
	//
/*
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		ItemSerializer *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( pItem != nullptr )
		{
			pItem->OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
		}
	}
*/
}

// レンダリング後始末
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::OnShoutdownSceneSettings
	( S3DScene& scene, SGLSecondaryViewProducer * psvp )
{
	SpaceSerializer::OnShoutdownSceneSettings( scene, psvp ) ;
	//
/*
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		ItemSerializer *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( pItem != nullptr )
		{
			pItem->OnShoutdownSceneSettings( scene, psvp ) ;
		}
	}
*/
}

// フレーム反映
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::SetAllItemsFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	SpaceSerializer::SetAllItemsFrame( fpFrame, seek ) ;
	//
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		ItemSerializer *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( pItem != nullptr )
		{
			pItem->SetFrameParameters( fpFrame, seek ) ;
		}
	}
	OnUpdateFrame( fpFrame, seek ) ;
	//
	if ( m_flagPauseTimer && m_flagTimerByFrame
		&& ((seek == seekStream) || (seek == seekStreamPaused)) )
	{
		const CompositionInfo *	pci = GetCompositionInfo() ;
		S3DScene *	pScene = GetScene() ;
		if ( (pScene != nullptr) && (pci != nullptr) )
		{
			double	secLast = pci->FrameIndexToSecond( m_fpCurrentFrame ) ;
			double	secSeek = pci->FrameIndexToSecond( fpFrame ) ;
			double	secPast = secSeek - secLast ;
			if ( secPast > 0.001 )
			{
				SpaceSerializer::OnTimer
					( *pScene, (uint32_t) eslRoundR64ToLInt( secPast * 1000.0 ) ) ;
			}
		}
	}
	m_fpCurrentFrame = fpFrame ;
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		ItemSerializer *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( pItem != nullptr )
		{
			pItem->OnUpdateFrame( fpFrame, seek ) ;
		}
	}
	SpaceSerializer::OnUpdateFrame( fpFrame, seek ) ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	FlushDelayRemoveItems() ;
	//
	if ( m_flagFixTimerInterval && (m_msecTimerInterval != 0) )
	{
		int32_t	nFrameCount =
					((int32_t)msecPast + m_msecOddTimer) / m_msecTimerInterval ;
		if ( nFrameCount < 0 )
		{
			nFrameCount = 0 ;
		}
		else if ( nFrameCount > 1 )
		{
			nFrameCount -- ;
		}
		m_msecOddTimer = (msecPast + m_msecOddTimer)
							- nFrameCount * m_msecTimerInterval ;
		//
		const CompositionInfo *	pci = GetCompositionInfo() ;
		for ( int32_t i = 0; i < nFrameCount; i ++ )
		{
			if ( (pci != nullptr) && m_flagPlaying && !m_flagPaused && m_flagFrameByTimer )
			{
				double	nNextFrame = floor( m_fpCurrentFrame ) + 1.0 ;
				if ( m_flagFrameSpeedByTimer )
				{
					nNextFrame = m_fpCurrentFrame + m_fpFrameSpeed ;
				}
				bool	flagJump = GetPostTimelineFrame( nNextFrame ) ;
				AdvanceCompositionFrame( scene, pci, nNextFrame, flagJump ) ;
			}
			else if ( m_flagFrameByTimer )
			{
				OnUpdateFrame( m_fpCurrentFrame, seekStreamPaused ) ;
			}
			if ( !m_flagPauseTimer )
			{
				S3DScene::SyncItemDispatcher *
					pSync = scene.AddCurrentThreadSyncDispatcher() ;
				SpaceSerializer::OnTimer( scene, m_msecTimerInterval ) ;
				scene.ReleaseCurrentThreadSyncDispatcher( pSync ) ;
			}
		}
	}
	else
	{
		if ( m_flagPlaying && !m_flagPaused && m_flagFrameByTimer )
		{
			AdvanceCompositionTime( scene, msecPast ) ;
		}
		else if ( m_flagFrameByTimer )
		{
			OnUpdateFrame( m_fpCurrentFrame, seekStreamPaused ) ;
		}
		if ( !m_flagPauseTimer )
		{
			SpaceSerializer::OnTimer( scene, msecPast ) ;
		}
	}
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::OnUpdateBehavior( S3DScene& scene )
{
	SpaceSerializer::OnUpdateBehavior( scene ) ;
	//
	FlushDelayRemoveItems() ;
}

// 再生中フレーム番号取得
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::Composition::GetCurrentPlayingFrame( void ) const
{
	return	m_fpCurrentFrame ;
}

// 初期化処理通知（CmdInitializeItem 通知）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::InitializeItems( S3DScene * pScene )
{
	S3DScene *	pSaveScene = m_pTargetScene ;
	if ( pScene != nullptr )
	{
		m_pTargetScene = pScene ;
	}
	OnExtendNotify( S3DSceneComposer::CmdInitializeItem, nullptr, nullptr, 0 ) ;
	//
	m_pTargetScene = pSaveScene ;
}

// コンポジションの再生開始（Timer駆動）（CmdStartItem 通知あり）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::PlayComposition( void )
{
	m_flagFinished = false ;
	m_flagPlaying = true ;
	m_flagPaused = false ;
	m_nPausedCounter = 0 ;
	//
	OnExtendNotify( S3DSceneComposer::CmdStartItem, nullptr, nullptr, 0 ) ;
}

// コンポジションの停止（CmdStartItem 通知なし）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::StopCompositoin( void )
{
	m_flagPlaying = false ;
	m_flagPaused = false ;
	m_nPausedCounter = 0 ;
	//
	OnExtendNotify( S3DSceneComposer::CmdStopItem, nullptr, nullptr, 0 ) ;
}

// コンポジションの再生開始（Timer駆動）（CmdStartItem 通知なし）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::RestartComposition( void )
{
	if ( AtomicSub( &m_nPausedCounter, 1 ) <= 0 )
	{
		m_flagPlaying = true ;
		m_flagPaused = false ;
	}
}

// コンポジションの停止（CmdStopItem 通知なし）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::PauseCompositoin( void )
{
	AtomicAdd( &m_nPausedCounter, 1 ) ;
	m_flagPaused = true ;
}

// コンポジションの終了
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::FinishComposition( void )
{
	m_flagFinished = true ;
	m_flagPlaying = false ;
	m_flagPaused = false ;
	m_nPausedCounter = 0 ;
	//
	OnExtendNotify( S3DSceneComposer::CmdFinishItem, nullptr, nullptr, 0 ) ;
}

// コンポジションの再生中か？
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Composition::IsPlayingComposition( void ) const
{
	return	m_flagPlaying ;
}

// コンポジションの一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Composition::IsPausedComposition( void ) const
{
	return	m_flagPlaying && m_flagPaused ;
}

// コンポジション終了か？
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Composition::IsCompositionFinished( void ) const
{
	return	m_flagFinished ;
}

// OnTimer 再開／停止
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::EnableTimerEvent
					( bool flagEnable, bool flagTimerByFrame )
{
	m_flagPauseTimer = !flagEnable ;
	m_flagTimerByFrame = flagTimerByFrame ;
}

bool S3DSceneComposer::Composition::IsEnabledTimerEvent( void ) const
{
	return	!m_flagPauseTimer ;
}

bool S3DSceneComposer::Composition::IsTimerByFrame( void ) const
{
	return	m_flagTimerByFrame ;
}

// OnTimer 駆動での OnUpdateFrame 呼び出しの有効化
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::EnableFrameOnTimer( bool flagEnable )
{
	m_flagFrameByTimer = flagEnable ;
}

// フレームを 1.0 単位で SetAllItemsFrame, OnTimer を呼び出すか？
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::SetFixTimerInterval( bool flagFixTimer )
{
	m_flagFixTimerInterval = flagFixTimer ;
}

bool S3DSceneComposer::Composition::IsFixedTimerInterval( void ) const
{
	return	m_flagFixTimerInterval ;
}

// フレーム進行速度（フレーム比）の設定（FixTimer 時に有効）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::SetFrameSpeed( double fpFrameSpeed, bool flagEnable )
{
	m_fpFrameSpeed = fpFrameSpeed ;
	m_flagFrameSpeedByTimer = flagEnable ;
}

bool S3DSceneComposer::Composition::GetFrameSpeedFlag( void ) const
{
	return	m_flagFrameSpeedByTimer ;
}

double S3DSceneComposer::Composition::GetFrameSpeed( void ) const
{
	return	m_fpFrameSpeed ;
}

// ジャンプ先フレーム設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::PostTimelineFrame( double fpFrame )
{
	m_fpNextJumpFrame = fpFrame ;
	m_flagPostJump = true ;
	if ( m_flagPlaying )
	{
		m_flagPaused = false ;
		m_nPausedCounter = 0 ;
	}
}

// PostTimelineFrame で設定したジャンプ先へ未移行か？
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Composition::IsPendingPostTimeline( void ) const
{
	return	m_flagPostJump ;
}

// コンポジション終了フラグのリセット
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::ResetCompositionFinished( void )
{
	m_flagFinished = false ;
}

// ジャンプ先フレームが設定されたかの判定とジャンプ先の取得と削除
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Composition::GetPostTimelineFrame( double& fpFrame )
{
	if ( !m_flagPostJump )
	{
		return	false ;
	}
	fpFrame = m_fpNextJumpFrame ;
	m_flagPostJump = false ;
	return	true ;
}

// フレーム進行（マニュアル実行）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::AdvanceCompositionTime
	( S3DScene& scene, uint32_t msecPast )
{
	const CompositionInfo *	pci = GetCompositionInfo() ;
	if ( pci != nullptr )
	{
		double	framesPast =
					pci->FrameIndexFromSecond( (double) msecPast / 1000.0 ) ;
		double	nNextFrame = m_fpCurrentFrame + framesPast ;
		//
		bool	flagJump = GetPostTimelineFrame( nNextFrame ) ;
		AdvanceCompositionFrame( scene, pci, nNextFrame, flagJump ) ;
	}
}

void S3DSceneComposer::Composition::AdvanceCompositionFrame
	( S3DScene& scene,
		const CompositionInfo * pci,  double nNextFrame, bool flagJump )
{
	if ( nNextFrame > pci->GetTotalFrameCount() )
	{
		nNextFrame = (double) pci->GetTotalFrameCount() ;
		m_flagPlaying = false ;
	}
	if ( nNextFrame != m_fpCurrentFrame )
	{
		SetAllItemsFrame
			( nNextFrame, (flagJump ? seekJump : seekStream) ) ;
		//
		while ( m_flagPostJump )
		{
			m_flagPostJump = false ;
			SetAllItemsFrame( m_fpNextJumpFrame, seekJump ) ;
		}
		scene.PostSceneUpdate() ;
	}
}

// エディットモードか？
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Composition::IsEditMode( void ) const
{
	return	m_flagEditMode ;
}

// エディットモード設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::SetEditMode( bool flagEdit )
{
	m_flagEditMode = flagEdit ;
}

// エディター・インターフェース関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::AttachEditor( S3DCompositionEditorInterface * pEditor )
{
	m_pEditorInteface = pEditor ;
}

// 子アイテムを登録と追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::AddSpaceChild
	( S3DSceneComposer::SpaceSerializer& space,
		S3DSceneComposer::ItemSerializer * pItem, const wchar_t * pwszID )
{
	if ( (pwszID != nullptr) && (pwszID[0] != 0) )
	{
		RegisterSceneItem( pwszID, pItem ) ;
	}
	else
	{
		space.m_arrItemAssets.Add( pItem ) ;
	}
	S3DScene::Space *
		pSpace = ESLTypeCast<S3DScene::Space>( pItem ) ;
	if ( pSpace != nullptr )
	{
		space.m_pSpace->AddChild( pSpace ) ;
		pItem->OnAttachedCompositionTree( *this ) ;
	}
	else
	{
		S3DScene::Item *
			pSubItem = ESLTypeCast<S3DScene::Item>( pItem ) ;
		if ( pSubItem != nullptr )
		{
			space.m_pSpace->AddItem( pSubItem ) ;
			pItem->OnAttachedCompositionTree( *this ) ;

		}
	}
}

// 子アイテムを削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Composition::RemoveSpaceChild
	( S3DSceneComposer::SpaceSerializer& space,
				S3DSceneComposer::ItemSerializer * pItem )
{
	S3DScene::Space *
		pSpace = ESLTypeCast<S3DScene::Space>( pItem ) ;
	if ( pSpace != nullptr )
	{
		space.m_pSpace->RemoveChild( pSpace ) ;
	}
	else
	{
		S3DScene::Item *
			pSubItem = ESLTypeCast<S3DScene::Item>( pItem ) ;
		if ( pSubItem != nullptr )
		{
			space.m_pSpace->RemoveItem( pSubItem ) ;
		}
	}
	ssize_t	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_ssoaItems.RemoveAt( (size_t) i ) ;
		return	sglErrSuccess ;
	}
	i = space.m_arrItemAssets.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		space.m_arrItemAssets.RemoveAt( (size_t) i ) ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

SGLError S3DSceneComposer::Composition::DelayRemoveSpaceChild
	( SpaceSerializer& space, ItemSerializer * pItem )
{
	S3DScene::Space *
		pSpace = ESLTypeCast<S3DScene::Space>( pItem ) ;
	if ( pSpace != nullptr )
	{
		space.m_pSpace->RemoveChild( pSpace ) ;
	}
	else
	{
		S3DScene::Item *
			pSubItem = ESLTypeCast<S3DScene::Item>( pItem ) ;
		if ( pSubItem != nullptr )
		{
			space.m_pSpace->RemoveItem( pSubItem ) ;
		}
	}
	ssize_t	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_ssoaItems.DetachAt( (size_t) i ) ;
		m_aDelayRemove.Add( pItem ) ;
		return	sglErrSuccess ;
	}
	i = space.m_arrItemAssets.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		space.m_arrItemAssets.DetachAt( (size_t) i ) ;
		m_aDelayRemove.Add( pItem ) ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

SGLError S3DSceneComposer::Composition::DetachSpaceChild
	( S3DSceneComposer::SpaceSerializer& space,
		S3DSceneComposer::ItemSerializer * pItem )
{
	S3DScene::Space *
		pSpace = ESLTypeCast<S3DScene::Space>( pItem ) ;
	if ( pSpace != nullptr )
	{
		space.m_pSpace->RemoveChild( pSpace ) ;
	}
	else
	{
		S3DScene::Item *
			pSubItem = ESLTypeCast<S3DScene::Item>( pItem ) ;
		if ( pSubItem != nullptr )
		{
			space.m_pSpace->RemoveItem( pSubItem ) ;
		}
	}
	ssize_t	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_ssoaItems.DetachAt( (size_t) i ) ;
		return	sglErrSuccess ;
	}
	i = space.m_arrItemAssets.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		space.m_arrItemAssets.DetachAt( (size_t) i ) ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 遅延削除リストに追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::PostDelayRemoveItem
	( S3DSceneComposer::SpaceSerializer& space,
			S3DSceneComposer::ItemSerializer * pItem )
{
	DelayRemoveEntry	dre ;
	dre.pParent = &space ;
	dre.pItem = pItem ;
	dre.pChild = ESLTypeCast<S3DScene::Space>( pItem ) ;
	//
	if ( dre.pChild != nullptr )
	{
		PostDelayRemoveItemEntry( m_aDelayRemoveSpace, dre ) ;
	}
	else
	{
		PostDelayRemoveItemEntry( m_aDelayRemoveItem, dre ) ;
	}
}

void S3DSceneComposer::Composition::PostDelayRemoveItemEntry
	( SSystem::SArray<S3DSceneComposer::Composition::DelayRemoveEntry>& list,
					const S3DSceneComposer::Composition::DelayRemoveEntry& dre )
{
	m_csDelayRemove.Lock() ;
	//
	const DelayRemoveEntry *	pdre = list.GetConstArray() ;
	size_t	nCount = list.GetLength() ;
	bool	flagFound = false ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( (pdre[i].pParent == dre.pParent)
			&& (pdre[i].pItem == dre.pItem) )
		{
			flagFound = true ;
			break ;
		}
	}
	if ( !flagFound )
	{
		list.Add( dre ) ;
	}
	//
	m_csDelayRemove.Unlock() ;
}

// 遅延削除実行
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::FlushDelayRemoveItems( void )
{
	m_csDelayRemove.Lock() ;
	//
	RemoveDelayItemEntries( m_aDelayRemoveItem ) ;
	RemoveDelayItemEntries( m_aDelayRemoveSpace ) ;
	//
	m_csDelayRemove.Unlock() ;
}

void S3DSceneComposer::Composition::RemoveDelayItemEntries
	( SSystem::SArray<S3DSceneComposer::Composition::DelayRemoveEntry>& list )
{
	const DelayRemoveEntry *	pdre = list.GetConstArray() ;
	size_t	nCount = list.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DelayRemoveSpaceChild( *(pdre[i].pParent), pdre[i].pItem ) ;
	}
	list.RemoveAll() ;
}

// アイテムIDが重複する場合、固有IDを生成する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::NormalizeSceneItemID( SSystem::SString& strID ) const
{
	if ( m_ssoaItems.GetAs( strID ) != nullptr )
	{
		SString	strBaseID = strID ;
		while ( (strBaseID.GetLength() > 1)
			&& (strBaseID.GetLastAt(0) >= L'0')
			&& (strBaseID.GetLastAt(0) <= L'9') )
		{
			strBaseID.ChopRight(1) ;
		}
		for ( int i = 1; i < 0x10000; i ++ )
		{
			strID = strBaseID ;
			strID += SString( i ) ;
			if ( m_ssoaItems.GetAs( strID ) == nullptr )
			{
				break ;
			}
		}
	}
}

// アイテム登録
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::RegisterSceneItem
	( const wchar_t * pwszID, S3DSceneComposer::ItemSerializer * pItem )
{
	SString	strID = pwszID ;
	NormalizeSceneItemID( strID ) ;
	pItem->SetItemIdentity( strID ) ;
	m_ssoaItems.Add( strID, pItem ) ;
}

// アイテム登録解除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::UnregisterSceneItem
	( S3DSceneComposer::ItemSerializer * pItem )
{
	ssize_t	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_ssoaItems.DetachAt( (size_t) i ) ;
	}
}

// アイテムID変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Composition::ChangeSceneItemID
	( S3DSceneComposer::ItemSerializer * pItem, const wchar_t * pwszID )
{
	if ( m_ssoaItems.GetAs( pwszID ) != nullptr )
	{
		if ( m_ssoaItems.GetAs( pwszID ) != pItem )
		{
			return	sglErrFailed ;
		}
	}
	ssize_t	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		ESLVerify( m_ssoaItems.DetachAt( (size_t) i ) == pItem ) ;
	}
	else
	{
		return	sglErrFailed ;
	}
	pItem->SetItemIdentity( pwszID ) ;
	m_ssoaItems.Add( pwszID, pItem ) ;
	return	sglErrSuccess ;
}

// アイテム取得（コンポジションの階層は \ で区切る）
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::Composition::GetSceneItemAs( const wchar_t * pwszID ) const
{
	if ( pwszID != nullptr )
	{
		while ( pwszID[0] == L'\\' )
		{
			if ( m_pOwnerComposition != nullptr )
			{
				ItemSerializer *	pItem =
					m_pOwnerComposition->GetSceneItemAs( pwszID + 1 ) ;
				if ( pItem != nullptr )
				{
					return	pItem ;
				}
			}
			pwszID ++ ;
		}
		if ( pwszID[0] == 0 )
		{
			return	nullptr ;
		}
		ssize_t	iSep = -1 ;
		for ( size_t i = 1; pwszID[i]; i ++ )
		{
			if ( pwszID[i] == L'\\' )
			{
				ItemSerializer *	pItem =
					m_ssoaItems.GetAs( SString( pwszID, (ssize_t) i ) ) ;
				if ( pItem == nullptr )
				{
					if ( m_pOwnerComposition != nullptr )
					{
						return	m_pOwnerComposition->GetSceneItemAs( pwszID ) ;
					}
					return	nullptr ;
				}
				Composition *	pComp =
					ESLTypeCast<Composition>( pItem ) ;
				if ( pComp == nullptr )
				{
					S3DSubCompositionSerializer *	pSubComp =
						ESLTypeCast<S3DSubCompositionSerializer>( pItem ) ;
					if ( pSubComp == nullptr )
					{
						return	nullptr ;
					}
					pComp = pSubComp->GetSubComposition() ;
					if ( pComp == nullptr )
					{
						return	nullptr ;
					}
				}
				return	pComp->GetSceneItemAs( pwszID + i + 1 ) ;
			}
		}
	}
	S3DSceneComposer::ItemSerializer *
				pItem = m_ssoaItems.GetAs( pwszID ) ;
	if ( pItem == nullptr )
	{
		if ( m_pOwnerComposition != nullptr )
		{
			return	m_pOwnerComposition->GetSceneItemAs( pwszID ) ;
		}
	}
	return	pItem ;
}

// 空間取得（モデルアイテムのボーンは @ で区切る）
//////////////////////////////////////////////////////////////////////////////
S3DScene::Space *
	S3DSceneComposer::Composition::GetSceneSpaceAs( const wchar_t * pwszID ) const
{
	if ( pwszID != nullptr )
	{
		ssize_t	iSep = -1 ;
		for ( size_t i = 0; pwszID[i]; i ++ )
		{
			if ( pwszID[i] == L'\\' )
			{
				Composition *	pItem =
					ESLTypeCast<Composition>
						( m_ssoaItems.GetAs( SString( pwszID, (ssize_t) i ) ) ) ;
				if ( pItem == nullptr )
				{
					return	nullptr ;
				}
				return	pItem->GetSceneSpaceAs( pwszID + i + 1 ) ;
			}
			if ( pwszID[i] == L'@' )
			{
				ItemCommonSerializer *	pItem =
					ESLTypeCast<ItemCommonSerializer>
						( m_ssoaItems.GetAs( SString( pwszID, (ssize_t) i ) ) ) ;
				if ( pItem == nullptr )
				{
					return	nullptr ;
				}
				S3DScene::ModelItem *	pModelItem =
					ESLTypeCast<S3DScene::ModelItem>( pItem->GetSceneItem() ) ;
				if ( pModelItem != nullptr )
				{
					return	nullptr ;
				}
				S3DModelBuffer *	pModel =
					ESLTypeCast<S3DModelBuffer>( pModelItem->GetModel() ) ;
				if ( pModel == nullptr )
				{
					return	nullptr ;
				}
				return	pModel->GetBonePropertyAs( pwszID + i + 1 ) ;
			}
		}
	}
	SpaceSerializer *	pSpace =
		ESLTypeCast<SpaceSerializer>( m_ssoaItems.GetAs( pwszID ) ) ;
	if ( pSpace == nullptr )
	{
		return	nullptr ;
	}
	return	pSpace->GetSceneSpace() ;
}

// アイテムID取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	S3DSceneComposer::Composition::GetSceneItemIDOf
		( S3DSceneComposer::ItemSerializer * pItem ) const
{
	if ( (pItem == nullptr)
		|| (pItem->GetItemIdentity() == nullptr) )
	{
		return	nullptr ;
	}
/*
	if ( m_ssoaItems.GetAs( pItem->GetItemIdentity() ) != pItem )
	{
		return	nullptr ;
	}
	return	pItem->GetItemIdentity() ;
*/
	ssize_t	i = m_ssoaItems.FindPtr( pItem ) ;
	if ( i < 0 )
	{
		return	nullptr ;
	}
	const SString *	pstrTag = m_ssoaItems.GetTagAt( (size_t) i ) ;
	if ( pstrTag == nullptr )
	{
		return	nullptr ;
	}
	return	*pstrTag ;
}

// 全アイテムID列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::EnumerateAllItemIDs
	( SSystem::SObjectArray<SSystem::SString>& aIDs, const wchar_t * pwszBasePath ) const
{
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		const SString *	pstrID = m_ssoaItems.GetTagAt( i ) ;
		if ( pstrID != nullptr )
		{
			SString	strID = SString(pwszBasePath).OffsetFilePath( *pstrID ) ;
			aIDs.Add( new SString( strID ) ) ;
			//
			Composition *	pComp =
				ESLTypeCast<Composition>( m_ssoaItems.GetAt( i ) ) ;
			if ( pComp == nullptr )
			{
				S3DSubCompositionSerializer *	pSubComp =
					ESLTypeCast<S3DSubCompositionSerializer>
									( m_ssoaItems.GetAt( i ) ) ;
				if ( pSubComp != nullptr )
				{
					pComp = pSubComp->GetSubComposition() ;
				}
			}
			if ( pComp != nullptr )
			{
				pComp->EnumerateAllItemIDs( aIDs, strID ) ;
			}
		}
	}
}

// 特定クラスアイテムID列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::EnumerateItemIDsAs
	( SSystem::SObjectArray<SSystem::SString>& aIDs,
		const ESLRuntimeClass& rtClass, const wchar_t * pwszBasePath ) const
{
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		const SString *		pstrID = m_ssoaItems.GetTagAt( i ) ;
		ItemSerializer *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		SString	strID = SString(pwszBasePath).OffsetFilePath( *pstrID ) ;
		if ( (pItem != nullptr) && pItem->IsKindOf(rtClass) )
		{
			aIDs.Add( new SString( strID ) ) ;
		}
		Composition *	pComp = ESLTypeCast<Composition>( pItem ) ;
		if ( pComp == nullptr )
		{
			S3DSubCompositionSerializer *	pSubComp =
				ESLTypeCast<S3DSubCompositionSerializer>( pItem ) ;
			if ( pSubComp != nullptr )
			{
				pComp = pSubComp->GetSubComposition() ;
			}
		}
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemIDsAs( aIDs, rtClass, strID ) ;
		}
	}
}

// アイテム列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Composition::EnumerateItemsAs
	( SSystem::SPointerArray<S3DSceneComposer::ItemSerializer>& aItems,
									const ESLRuntimeClass& rtClass ) const
{
	for ( size_t i = 0; i < m_ssoaItems.GetLength(); i ++ )
	{
		ItemSerializer *	pItem = m_ssoaItems.GetAt( i ) ;
		if ( (pItem != nullptr) && pItem->IsKindOf(rtClass) )
		{
			aItems.Add( pItem ) ;
		}
		Composition *	pComp = ESLTypeCast<Composition>( pItem ) ;
		if ( pComp == nullptr )
		{
			S3DSubCompositionSerializer *	pSubComp =
				ESLTypeCast<S3DSubCompositionSerializer>( pItem ) ;
			if ( pSubComp != nullptr )
			{
				pComp = pSubComp->GetSubComposition() ;
			}
		}
		if ( pComp != nullptr )
		{
			pComp->EnumerateItemsAs( aItems, rtClass ) ;
		}
	}
}

// ユーザーデータ
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposer::ScriptObject& S3DSceneComposer::Composition::GetScriptInstance( void ) const
{
	return	m_instance ;
}

void S3DSceneComposer::Composition::SetScriptInstance( const S3DSceneComposer::ScriptObject& instance )
{
	m_instance = instance ;
}

const Rosetta::RSSmartPtr& S3DSceneComposer::Composition::GetUserInstance( void ) const
{
	return	m_instance.GetRosetta() ;
}

void S3DSceneComposer::Composition::SetUserInstance( const Rosetta::RSSmartPtr& ptrInstance )
{
	m_instance = ScriptObject( ptrInstance ) ;
}

const Loquaty::LObjPtr& S3DSceneComposer::Composition::GetLoquatyInstance( void ) const
{
	return	m_instance.GetLoquatyObj() ;
}

void S3DSceneComposer::Composition::SetLoquatyInstance( const Loquaty::LObjPtr& pObj )
{
	m_instance = ScriptObject( pObj ) ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Composition::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneComposition" ;
}



//////////////////////////////////////////////////////////////////////////////
// インポート情報
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::ImportEntry, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ImportEntry::ImportEntry
	( S3DCompositionManager * pManager, S3DSceneComposer * pComposer )
{
	ESLAssert( pManager != nullptr ) ;
//	ESLAssert( pComposer != nullptr ) ;
	m_pManager = pManager ;
	m_pComposer = pComposer ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ImportEntry::~ImportEntry( void )
{
	UnloadComposer() ;
}

// アンロード
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ImportEntry::UnloadComposer( void )
{
	ESLAssert( m_pManager != nullptr ) ;
//	ESLAssert( m_pComposer != nullptr ) ;
	if ( m_pComposer != nullptr )
	{
		m_pManager->UnloadComposer( m_pComposer ) ;
		m_pComposer = nullptr ;
	}
}

// リロード
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ImportEntry::ReloadComposer( S3DSceneComposer * pMaster )
{
	UnloadComposer() ;
	//
	SSmartPointer<SFileInterface>
			pFile = pMaster->OpenAssetFile( m_strSrcFile ) ;
	if ( pFile == nullptr )
	{
		return	sglErrFailed ;
	}
	ESLAssert( m_pManager != nullptr ) ;
	ESLAssert( m_pComposer == nullptr ) ;
	m_pComposer = m_pManager->LoadComposer( m_strSrcFile, pFile ) ;
	//
	return	(m_pComposer != nullptr) ? sglErrSuccess : sglErrFailed ;
}



//////////////////////////////////////////////////////////////////////////////
// バリアント
//////////////////////////////////////////////////////////////////////////////

// バリアント構築
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Variant::Variant( void )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
}

S3DSceneComposer::Variant::Variant( const Variant& val )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( val.m_pData, val.m_nBytes, val.m_type ) ;
}

S3DSceneComposer::Variant::Variant( const S3DDMatrix& mat3, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &mat3, sizeof(S3DDMatrix), type ) ;
}

S3DSceneComposer::Variant::Variant( const S3DDVector& vec3, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &vec3, sizeof(S3DDVector), type ) ;
}

S3DSceneComposer::Variant::Variant( const S3DDQuaternion& q, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	S3DDMatrix	mat ;
	q.ToMatrix( mat ) ;
	SetData( &mat, sizeof(S3DDMatrix), type ) ;
}

S3DSceneComposer::Variant::Variant( const SGLPalette& rgb, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	S3DDVector	vec = VectorFromColor( rgb ) ;
	SetData( &vec, sizeof(S3DDVector), type ) ;
}

S3DSceneComposer::Variant::Variant( bool b, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &b, sizeof(bool), type ) ;
}

S3DSceneComposer::Variant::Variant( int32_t n, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &n, sizeof(int32_t), type ) ;
}

S3DSceneComposer::Variant::Variant( double s, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &s, sizeof(double), type ) ;
}

S3DSceneComposer::Variant::Variant( const wchar_t * pwszCmd, ParameterType type )
	: m_type( type ), m_pData( nullptr ), m_nBytes( 0 )
{
	if ( pwszCmd != nullptr )
	{
		size_t	nStrLen = SString::GetLength( pwszCmd ) ;
		SetData( pwszCmd, (nStrLen + 1) * sizeof(wchar_t), type ) ;
	}
	else
	{
		wchar_t	wchNull = 0 ;
		SetData( &wchNull, sizeof(wchar_t), type ) ;
	}
}

S3DSceneComposer::Variant::Variant( const PoseInstance& pose )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &pose, sizeof(PoseInstance), typePose ) ;
}

S3DSceneComposer::Variant::Variant( const S4DDMatrix& mat4, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &mat4, sizeof(S4DDMatrix), type ) ;
}

S3DSceneComposer::Variant::Variant( const S4DDVector& vec4, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &vec4, sizeof(S4DDVector), type ) ;
}

S3DSceneComposer::Variant::Variant( const S2DDVector& vec2, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( &vec2, sizeof(S2DDVector), type ) ;
}

S3DSceneComposer::Variant::Variant( const void * pData, size_t nBytes, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetData( pData, nBytes, type ) ;
}

S3DSceneComposer::Variant::Variant( Rosetta::RSObject * pObj, ParameterType type )
	: m_type( typeInvalid ), m_pData( nullptr ), m_nBytes( 0 )
{
	SetFromRosetta( pObj, type ) ;
}

// 消滅
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Variant::~Variant( void )
{
	Clear() ;
}

// データ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Variant::SetData( const void * pData, size_t nBytes, ParameterType type )
{
	eslCopyMemory( PutBuffer( nBytes, type ), pData, nBytes ) ;
}

bool S3DSceneComposer::Variant::SetFromRosetta( Rosetta::RSObject * pObj, ParameterType type )
{
	if ( pObj == nullptr )
	{
		Clear() ;
		return	true ;
	}
	pObj = pObj->GetEntityObject() ;
	if ( pObj == nullptr )
	{
		Clear() ;
		return	true ;
	}
	RSStructuredPointerClass *	pStructType =
		ESLTypeCast<RSStructuredPointerClass>( pObj->GetRSClass() ) ;
	if ( pStructType != nullptr )
	{
		RSStructuredPointer *	pStruct =
				ESLTypeCast<RSStructuredPointer>( pObj ) ;
		if ( pStruct != nullptr )
		{
			const wchar_t *	pwszClassName = pStructType->GetRSClassName() ;
			if ( (SString::Compare( pwszClassName, L"Vector3D4" ) == 0)
				|| (SString::Compare( pwszClassName, L"Vector3D" ) == 0) )
			{
				S3DDVector	vSrc( 0, 0, 0 ) ;
				S3DVector *	pvBuf =
					(S3DVector*) pStruct->GetPointer( sizeof(S3DVector) ) ;
				if ( pvBuf != nullptr )
				{
					vSrc = *pvBuf ;
				}
				if ( (type != S3DSceneComposer::typeDirection)
					&& (type != S3DSceneComposer::typeZoom)
					&& (type != S3DSceneComposer::typeColor) )
				{
					type = S3DSceneComposer::typePosition ;
				}
				SetData( &vSrc, sizeof(S3DDVector), type ) ;
				return	true ;
			}
			else if ( SString::Compare( pwszClassName, L"Quaternion" ) == 0 )
			{
				S3DDMatrix	matSrc( 1, 1, 1 ) ;
				S3DQuaternion *	pqBuf =
					(S3DQuaternion*) pStruct->GetPointer( sizeof(S3DQuaternion) ) ;
				if ( pqBuf != nullptr )
				{
					S3DMatrix	matf ;
					pqBuf->ToMatrix( matf ) ;
					matSrc = matf ;
				}
				SetData( &matSrc, sizeof(S3DDMatrix), S3DSceneComposer::typeRotation ) ;
				return	true ;
			}
			else if ( SString::Compare( pwszClassName, L"Matrix3D" ) == 0 )
			{
				S3DDMatrix	matSrc( 1, 1, 1 ) ;
				SGL3DMatrix<float32_t,3> *	pmatBuf =
					(SGL3DMatrix<float32_t,3>*)
						pStruct->GetPointer( sizeof(SGL3DMatrix<float32_t,3>) ) ;
				if ( pmatBuf != nullptr )
				{
					for ( int i = 0; i < 3; i ++ )
					{
						matSrc.m[i][0] = pmatBuf->m[i][0] ;
						matSrc.m[i][1] = pmatBuf->m[i][1] ;
						matSrc.m[i][2] = pmatBuf->m[i][2] ;
					}
				}
				if ( type != S3DSceneComposer::typeRotation )
				{
					type = S3DSceneComposer::typeMatrix ;
				}
				SetData( &matSrc, sizeof(S3DDMatrix), type ) ;
				return	true ;
			}
			else if ( SString::Compare( pwszClassName, L"RGBColor" ) == 0 )
			{
				S3DDVector		vSrc( 0, 0, 0 ) ;
				SGLPalette *	ppBuf =
					(SGLPalette*) pStruct->GetPointer( sizeof(SGLPalette) ) ;
				if ( ppBuf != nullptr )
				{
					vSrc = VectorFromColor( *ppBuf ) ;
				}
				if ( (type != S3DSceneComposer::typePosition)
					&& (type != S3DSceneComposer::typeDirection)
					&& (type != S3DSceneComposer::typeZoom) )
				{
					type = S3DSceneComposer::typeColor ;
				}
				SetData( &vSrc, sizeof(S3DDVector), type ) ;
				return	true ;
			}
			else if ( SString::Compare( pwszClassName, L"Vector2D" ) == 0 )
			{
				S2DDVector	vSrc2( 0, 0 ) ;
				S2DVector *	pvBuf =
					(S2DVector*) pStruct->GetPointer( sizeof(S2DVector) ) ;
				if ( pvBuf != nullptr )
				{
					vSrc2 = *pvBuf ;
				}
				SetData( &vSrc2, sizeof(S2DDVector), S3DSceneComposer::typeVector2 ) ;
				return	true ;
			}
			else if ( SString::Compare( pwszClassName, L"Matrix4D" ) == 0 )
			{
				S4DDMatrix	matSrc4( 1, 1, 1, 1 ) ;
				S4DMatrix *	pmatBuf =
					(S4DMatrix*) pStruct->GetPointer( sizeof(S4DMatrix) ) ;
				if ( pmatBuf != nullptr )
				{
					matSrc4 = *pmatBuf ;
				}
				SetData( &matSrc4, sizeof(S4DDMatrix), S3DSceneComposer::typeMatrix4 ) ;
				return	true ;
			}
		}
	}
	else if ( type == S3DSceneComposer::typeScalar )
	{
		double	num = 0.0 ;
		pObj->AsRealNumber( num ) ;
		SetData( &num, sizeof(double), S3DSceneComposer::typeScalar ) ;
		return	true ;
	}
	else if ( type == S3DSceneComposer::typeInteger )
	{
		int64_t	num = 0 ;
		pObj->AsInteger( num ) ;
		//
		int32_t	n32 = (int32_t) num ;
		SetData( &n32, sizeof(int32_t), S3DSceneComposer::typeInteger ) ;
		return	true ;
	}
	else if ( type == S3DSceneComposer::typeBoolean )
	{
		bool	b = pObj->AsBoolean() ;
		SetData( &b, sizeof(bool), S3DSceneComposer::typeBoolean ) ;
		return	true ;
	}
	else  if ( pObj->IsIntegerType() )
	{
		RSInteger *	pIntObj = ESLTypeCast<RSInteger>( pObj ) ;
		if ( (pIntObj != nullptr)
			&& (pIntObj->m_intType == RSInteger::typeBoolean) )
		{
			bool	b = pObj->AsBoolean() ;
			SetData( &b, sizeof(bool), S3DSceneComposer::typeBoolean ) ;
		}
		else
		{
			int64_t	num = 0 ;
			pObj->AsInteger( num ) ;
			//
			int32_t	n32 = (int32_t) num ;
			SetData( &n32, sizeof(int32_t), S3DSceneComposer::typeInteger ) ;
		}
		return	true ;
	}
	else if ( pObj->IsFloatType() )
	{
		double	num = 0.0 ;
		pObj->AsRealNumber( num ) ;
		SetData( &num, sizeof(double), S3DSceneComposer::typeScalar ) ;
		return	true ;
	}
	else if ( pObj->IsStringType() )
	{
		if ( (type != typeSelector)
			&& (type != typeCommand) )
		{
			type = S3DSceneComposer::typeCommand ;
		}
		SString	str ;
		pObj->AsString( str ) ;
		//
		if ( !str.IsEmpty() )
		{
			size_t	nStrLen = str.GetLength() ;
			SetData( (const wchar_t*) str, (nStrLen + 1) * sizeof(wchar_t), type ) ;
		}
		else
		{
			wchar_t	wchNull = 0 ;
			SetData( &wchNull, sizeof(wchar_t), type ) ;
		}
		return	true ;
	}
	Clear() ;
	return	false ;
}

// バッファ確保
//////////////////////////////////////////////////////////////////////////////
uint8_t * S3DSceneComposer::Variant::PutBuffer( size_t nBytes, ParameterType type )
{
	if ( nBytes > m_nBytes )
	{
		if ( nBytes <= sizeof(m_buf) )
		{
			m_pData = m_buf ;
		}
		else
		{
			m_pData = (uint8_t*) esl_realloc( m_pData, nBytes ) ;
		}
	}
	m_type = type ;
	m_nBytes = nBytes ;
	return	m_pData ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposer::Variant&
	S3DSceneComposer::Variant::operator =
			( const S3DSceneComposer::Variant& val )
{
	SetData( val.m_pData, val.m_nBytes, val.m_type ) ;
	return	*this ;
}

// クリア
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Variant::Clear( void )
{
	if ( (m_pData != nullptr) && (m_nBytes > sizeof(m_buf)) )
	{
		esl_free( m_pData ) ;
	}
	m_type = S3DSceneComposer::typeInvalid ;
	m_pData = nullptr ;
	m_nBytes = 0 ;
}

// 空判定
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Variant::IsEmpty( void ) const
{
	return	(m_type == S3DSceneComposer::typeInvalid)
			|| (m_pData == nullptr) || (m_nBytes == 0) ;
}

// 型取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ParameterType S3DSceneComposer::Variant::GetType( void ) const
{
	return	m_type ;
}

// データ取得
//////////////////////////////////////////////////////////////////////////////
const uint8_t * S3DSceneComposer::Variant::GetData( void ) const
{
	return	m_pData ;
}

size_t S3DSceneComposer::Variant::GetDataBytes( void ) const
{
	return	m_nBytes ;
}

const S3DDMatrix& S3DSceneComposer::Variant::GetMatrix( void ) const
{
	ESLAssert( (m_type == S3DSceneComposer::typeMatrix)
				|| (m_type == S3DSceneComposer::typeRotation) ) ;
	ESLAssert( m_nBytes == sizeof(S3DDMatrix) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((S3DDMatrix*) m_pData) ;
}

const S3DDVector& S3DSceneComposer::Variant::GetVector( void ) const
{
	ESLAssert( (m_type == S3DSceneComposer::typePosition)
				|| (m_type == S3DSceneComposer::typeDirection)
				|| (m_type == S3DSceneComposer::typeZoom)
				|| (m_type == S3DSceneComposer::typeColor) ) ;
	ESLAssert( m_nBytes == sizeof(S3DDVector) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((S3DDVector*) m_pData) ;
}

double S3DSceneComposer::Variant::GetScalar( void ) const
{
	ESLAssert( m_type == S3DSceneComposer::typeScalar ) ;
	ESLAssert( m_nBytes == sizeof(double) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((double*) m_pData) ;
}

int32_t S3DSceneComposer::Variant::GetInteger( void ) const
{
	ESLAssert( m_type == S3DSceneComposer::typeInteger ) ;
	ESLAssert( m_nBytes == sizeof(int32_t) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((int32_t*) m_pData) ;
}

bool S3DSceneComposer::Variant::GetBoolean( void ) const
{
	ESLAssert( m_type == S3DSceneComposer::typeBoolean ) ;
	ESLAssert( m_nBytes == sizeof(bool) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((bool*) m_pData) ;
}

const wchar_t * S3DSceneComposer::Variant::GetString( void ) const
{
	ESLAssert( (m_type == S3DSceneComposer::typeSelector)
				|| (m_type == S3DSceneComposer::typeCommand) ) ;
	return	(const wchar_t *) m_pData ;
}

const S3DSceneComposer::PoseInstance& S3DSceneComposer::Variant::GetPose( void ) const
{
	ESLAssert( m_type == S3DSceneComposer::typePose ) ;
	ESLAssert( m_nBytes == sizeof(PoseInstance) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((PoseInstance*) m_pData) ;
}

const S4DDMatrix& S3DSceneComposer::Variant::GetMatrix4( void ) const
{
	ESLAssert( m_type == S3DSceneComposer::typeMatrix4 ) ;
	ESLAssert( m_nBytes == sizeof(S4DDMatrix) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((S4DDMatrix*) m_pData) ;
}

const S4DDVector& S3DSceneComposer::Variant::GetVector4( void ) const
{
	ESLAssert( m_type == S3DSceneComposer::typeVector4 ) ;
	ESLAssert( m_nBytes == sizeof(S4DDVector) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((S4DDVector*) m_pData) ;
}

const S2DDVector& S3DSceneComposer::Variant::GetVector2( void ) const
{
	ESLAssert( m_type == S3DSceneComposer::typeVector2 ) ;
	ESLAssert( m_nBytes == sizeof(S2DDVector) ) ;
	ESLAssert( m_pData != nullptr ) ;
	return	*((S2DDVector*) m_pData) ;
}

bool S3DSceneComposer::Variant::GetToRosetta( Rosetta::RSObject * pObj ) const
{
	if ( pObj == nullptr )
	{
		return	IsEmpty() ;
	}
	pObj = pObj->GetEntityObject() ;
	if ( pObj == nullptr )
	{
		return	IsEmpty() ;
	}
	RSStructuredPointerClass *	pStructType =
		ESLTypeCast<RSStructuredPointerClass>( pObj->GetRSClass() ) ;
	if ( pStructType != nullptr )
	{
		RSStructuredPointer *	pStruct =
				ESLTypeCast<RSStructuredPointer>( pObj ) ;
		if ( pStruct != nullptr )
		{
			const wchar_t *	pwszClassName = pStructType->GetRSClassName() ;
			if ( (SString::Compare( pwszClassName, L"Vector3D4" ) == 0)
				|| (SString::Compare( pwszClassName, L"Vector3D" ) == 0) )
			{
				if ( (m_type != S3DSceneComposer::typePosition)
					&& (m_type != S3DSceneComposer::typeDirection)
					&& (m_type != S3DSceneComposer::typeZoom)
					&& (m_type != S3DSceneComposer::typeColor) )
				{
					return	false ;
				}
				S3DVector *	pvBuf =
					(S3DVector*) pStruct->GetPointer( sizeof(S3DVector) ) ;
				if ( pvBuf != nullptr )
				{
					*pvBuf = GetVector() ;
					return	true ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"Quaternion" ) == 0 )
			{
				if ( (m_type != S3DSceneComposer::typeMatrix)
					&& (m_type != S3DSceneComposer::typeRotation) )
				{
					return	false ;
				}
				S3DDMatrix	matSrc( 1, 1, 1 ) ;
				S3DQuaternion *	pqBuf =
					(S3DQuaternion*) pStruct->GetPointer( sizeof(S3DQuaternion) ) ;
				if ( pqBuf != nullptr )
				{
					S3DMatrix	matf = GetMatrix() ;
					pqBuf->FromMatrix( matf ) ;
					return	true ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"Matrix3D" ) == 0 )
			{
				if ( (m_type != S3DSceneComposer::typeMatrix)
					&& (m_type != S3DSceneComposer::typeRotation) )
				{
					return	false ;
				}
				SGL3DMatrix<float32_t,3> *	pmatBuf =
					(SGL3DMatrix<float32_t,3>*)
						pStruct->GetPointer( sizeof(SGL3DMatrix<float32_t,3>) ) ;
				if ( pmatBuf != nullptr )
				{
					S3DMatrix	matf = GetMatrix() ;
					for ( int i = 0; i < 3; i ++ )
					{
						pmatBuf->m[i][0] = matf.m[i][0] ;
						pmatBuf->m[i][1] = matf.m[i][1] ;
						pmatBuf->m[i][2] = matf.m[i][2] ;
					}
					return	true ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"RGBColor" ) == 0 )
			{
				if ( (m_type != S3DSceneComposer::typePosition)
					&& (m_type != S3DSceneComposer::typeDirection)
					&& (m_type != S3DSceneComposer::typeZoom)
					&& (m_type != S3DSceneComposer::typeColor) )
				{
					return	false ;
				}
				SGLPalette *	ppBuf =
					(SGLPalette*) pStruct->GetPointer( sizeof(SGLPalette) ) ;
				if ( ppBuf != nullptr )
				{
					*ppBuf = ColorFromVector( GetVector() ) ;
					return	true ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"Vector2D" ) == 0 )
			{
				if ( m_type != S3DSceneComposer::typeVector2 )
				{
					return	false ;
				}
				S2DVector *	pvBuf =
					(S2DVector*) pStruct->GetPointer( sizeof(S2DVector) ) ;
				if ( pvBuf != nullptr )
				{
					*pvBuf = GetVector2() ;
					return	true ;
				}
			}
			else if ( SString::Compare( pwszClassName, L"Matrix4D" ) == 0 )
			{
				if ( m_type != S3DSceneComposer::typeMatrix4 )
				{
					return	false ;
				}
				S4DMatrix *	pmatBuf =
					(S4DMatrix*) pStruct->GetPointer( sizeof(S4DMatrix) ) ;
				if ( pmatBuf != nullptr )
				{
					*pmatBuf = GetMatrix4() ;
					return	true ;
				}
			}
		}
	}
	else if ( m_type == S3DSceneComposer::typeScalar )
	{
		pObj->SetNumberAs( GetScalar() ) ;
		return	true ;
	}
	else if ( m_type == S3DSceneComposer::typeInteger )
	{
		pObj->SetIntegerAs( GetInteger() ) ;
		return	true ;
	}
	else if ( m_type == S3DSceneComposer::typeBoolean )
	{
		pObj->SetIntegerAs( GetBoolean() ) ;
		return	true ;
	}
	if ( (m_type == S3DSceneComposer::typeSelector)
		|| (m_type == S3DSceneComposer::typeCommand) )
	{
		pObj->SetStringAs( GetString() ) ;
		return	true ;
	}
	return	false ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SString S3DSceneComposer::Variant::Serialize( void ) const
{
	SString	strValue ;
	switch ( m_type )
	{
	case	S3DSceneComposer::typeMatrix:
	case	S3DSceneComposer::typeRotation:
		{
			S3DDMatrix	mat = GetMatrix() ;
			FormatMatrix( strValue, mat ) ;
		}
		break ;
	case	S3DSceneComposer::typePosition:
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		{
			S3DDVector	vec = GetVector() ;
			FormatVector( strValue, vec ) ;
		}
		break ;
	case	S3DSceneComposer::typeScalar:
		{
			double	s = GetScalar() ;
			FormatScalar( strValue, s ) ;
		}
		break ;
	case	S3DSceneComposer::typeInteger:
		{
			int32_t	n = GetInteger() ;
			FormatInteger( strValue, n ) ;
		}
		break ;
	case	S3DSceneComposer::typeBoolean:
		{
			bool	b = GetBoolean() ;
			FormatBoolean( strValue, b ) ;
		}
		break ;
	case	S3DSceneComposer::typeMatrix4:
		{
			S4DDMatrix	mat4 = GetMatrix4() ;
			FormatVectorX( strValue, &(mat4.m[0][0]), 16 ) ;
		}
		break ;
	case	S3DSceneComposer::typeVector4:
		{
			S4DDVector	vec = GetVector4() ;
			double	vec4[4] =
			{
				vec.x, vec.y, vec.z, vec.w
			} ;
			FormatVectorX( strValue, vec4, 4 ) ;
		}
		break ;
	case	S3DSceneComposer::typeVector2:
		{
			S2DDVector	vec = GetVector2() ;
			double	vec2[2] =
			{
				vec.x, vec.y
			} ;
			FormatVectorX( strValue, vec2, 2 ) ;
		}
		break ;
	case	S3DSceneComposer::typeSelector:
	case	S3DSceneComposer::typeCommand:
		strValue = GetString() ;
		break ;

	case	S3DSceneComposer::typeBinary:
		Charset::EncodeBase64( strValue, m_pData, m_nBytes ) ;
		break ;

	case	S3DSceneComposer::typePose:
	default:
		break ;
	}
	return	strValue ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Variant::Deserialize( const SSystem::SString& strValue )
{
	switch ( m_type )
	{
	case	S3DSceneComposer::typeMatrix:
	case	S3DSceneComposer::typeRotation:
		{
			S3DDMatrix	mat ;
			if ( ParseMatrix( mat, strValue ) )
			{
				SetData( &mat, sizeof(S3DDMatrix), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typePosition:
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		{
			S3DDVector	vec ;
			if ( ParseVector( vec, strValue ) )
			{
				SetData( &vec, sizeof(S3DDVector), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeScalar:
		{
			double	s ;
			if ( ParseScalar( s, strValue ) )
			{
				SetData( &s, sizeof(double), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeInteger:
		{
			int32_t	n ;
			if ( ParseInteger( n, strValue ) )
			{
				SetData( &n, sizeof(int32_t), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeBoolean:
		{
			bool	b ;
			if ( ParseBoolean( b, strValue ) )
			{
				SetData( &b, sizeof(bool), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeMatrix4:
		{
			S4DDMatrix	mat4 ;
			if ( ParseVectorX( &(mat4.m[0][0]), 16, strValue ) )
			{
				SetData( &mat4, sizeof(S4DDMatrix), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeVector4:
		{
			double	vec4[4] ;
			if ( ParseVectorX( vec4, 4, strValue ) )
			{
				S4DDVector	vec( vec4[0], vec4[1], vec4[2], vec4[3] ) ;
				SetData( &vec, sizeof(S4DDVector), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeVector2:
		{
			double	vec2[2] ;
			if ( ParseVectorX( vec2, 2, strValue ) )
			{
				S2DDVector	vec( vec2[0], vec2[1] ) ;
				SetData( &vec, sizeof(S2DDVector), m_type ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeSelector:
	case	S3DSceneComposer::typeCommand:
		if ( !strValue.IsEmpty() )
		{
			size_t	nStrLen = strValue.GetLength() ;
			SetData( (const wchar_t*) strValue, (nStrLen + 1) * sizeof(wchar_t), m_type ) ;
		}
		else
		{
			wchar_t	wchNull = 0 ;
			SetData( &wchNull, sizeof(wchar_t), m_type ) ;
		}
		return	true ;

	case	S3DSceneComposer::typeBinary:
		{
			SArray<uint8_t>	aBinary ;
			Charset::DecodeBase64( aBinary, strValue, (ssize_t) strValue.GetLength() ) ;
			SetData( aBinary.GetConstArray(), aBinary.GetLength(), m_type ) ;
		}
		return	true ;

	case	S3DSceneComposer::typePose:
	default:
		break ;
	}
	return	false ;
}

// 解釈
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Variant::ParseMatrix( S3DDMatrix& mat, const SSystem::SString& strValue )
{
	return	Parameter::ParseMatrix( mat, strValue ) ;
}

bool S3DSceneComposer::Variant::ParseVector( S3DDVector& vec, const SSystem::SString& strValue )
{
	return	Parameter::ParseVector( vec, strValue ) ;
}

bool S3DSceneComposer::Variant::ParseScalar( double& s, const SSystem::SString& strValue )
{
	return	Parameter::ParseScalar( s, strValue ) ;
}

bool S3DSceneComposer::Variant::ParseInteger( int32_t& n, const SSystem::SString& strValue )
{
	return	Parameter::ParseInteger( n, strValue ) ;
}

bool S3DSceneComposer::Variant::ParseBoolean( bool& b, const SSystem::SString& strValue )
{
	return	Parameter::ParseBoolean( b, strValue ) ;
}

bool S3DSceneComposer::Variant::ParseVectorX( double * pVec, size_t nCount, const SSystem::SString& strValue )
{
	return	Parameter::ParseVectorX( pVec, nCount, strValue ) ;
}

// フォーマット
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Variant::FormatMatrix( SSystem::SString& strValue, const S3DDMatrix& mat )
{
	Parameter::FormatMatrix( strValue, mat ) ;
}

void S3DSceneComposer::Variant::FormatVector( SSystem::SString& strValue, const S3DDVector& vec )
{
	Parameter::FormatVector( strValue, vec ) ;
}

void S3DSceneComposer::Variant::FormatScalar( SSystem::SString& strValue, double s )
{
	Parameter::FormatScalar( strValue, s ) ;
}

void S3DSceneComposer::Variant::FormatInteger( SSystem::SString& strValue, int32_t n )
{
	Parameter::FormatInteger( strValue, n ) ;
}

void S3DSceneComposer::Variant::FormatBoolean( SSystem::SString& strValue, bool b )
{
	Parameter::FormatBoolean( strValue, b ) ;
}

void S3DSceneComposer::Variant::FormatVectorX( SSystem::SString& strValue, const double * pVec, size_t nCount )
{
	Parameter::FormatVectorX( strValue, pVec, nCount ) ;
}

// S3DDVector -> SGLPalette 変換
//////////////////////////////////////////////////////////////////////////////
SGLPalette S3DSceneComposer::Variant::ColorFromVector( const S3DDVector& vec )
{
	return	Parameter::ColorFromVector( vec ) ;
}

// SGLPalette -> S3DDVector 変換
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSceneComposer::Variant::VectorFromColor( const SGLPalette& rgb )
{
	return	Parameter::VectorFromColor( rgb ) ;
}



//////////////////////////////////////////////////////////////////////////////
// パラメータ・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::Parameter, SObject )

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::Parameter::GetMatrixParameter( size_t i ) const
{
	return	S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
}

S3DDVector S3DSceneComposer::Parameter::GetVectorParameter( size_t i ) const
{
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DSceneComposer::Parameter::GetScalarParameter( size_t i ) const
{
	return	0.0 ;
}

int32_t S3DSceneComposer::Parameter::GetIntegerParameter( size_t i ) const
{
	return	0 ;
}

bool S3DSceneComposer::Parameter::GetBooleanParameter( size_t i ) const
{
	return	false ;
}

const wchar_t * S3DSceneComposer::Parameter::GetCommandParameter( size_t i ) const
{
	return	nullptr ;
}

S3DSceneComposer::PoseInstance S3DSceneComposer::Parameter::GetPoseParameter( size_t i ) const
{
	return	PoseInstance() ;
}

size_t S3DSceneComposer::Parameter::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	return	0 ;
}

void S3DSceneComposer::Parameter::GetParameter
	( SSystem::SString& strValue,
		S3DSceneComposer::ParameterType type, size_t i )
{
	switch ( type )
	{
	case	S3DSceneComposer::typeMatrix:
	case	S3DSceneComposer::typeRotation:
		{
			S3DDMatrix	mat = GetMatrixParameter( i ) ;
			FormatMatrix( strValue, mat ) ;
		}
		break ;
	case	S3DSceneComposer::typePosition:
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		{
			S3DDVector	pos = GetVectorParameter( i ) ;
			FormatVector( strValue, pos ) ;
		}
		break ;
	case	S3DSceneComposer::typeScalar:
		{
			double	s = GetScalarParameter( i ) ;
			FormatScalar( strValue, s ) ;
		}
		break ;
	case	S3DSceneComposer::typeInteger:
		{
			int32_t	n = GetIntegerParameter( i ) ;
			FormatInteger( strValue, n ) ;
		}
		break ;
	case	S3DSceneComposer::typeBoolean:
		{
			bool	b = GetBooleanParameter( i ) ;
			FormatBoolean( strValue, b ) ;
		}
		break ;
	case	S3DSceneComposer::typeMatrix4:
		{
			double	mat4[16] ;
			GetBinaryParameter( mat4, sizeof(mat4), i ) ;
			FormatVectorX( strValue, mat4, 16 ) ;
		}
		break ;
	case	S3DSceneComposer::typeVector4:
		{
			double	vec4[4] ;
			GetBinaryParameter( vec4, sizeof(vec4), i ) ;
			FormatVectorX( strValue, vec4, 4 ) ;
		}
		break ;
	case	S3DSceneComposer::typeVector2:
		{
			double	vec2[2] ;
			GetBinaryParameter( vec2, sizeof(vec2), i ) ;
			FormatVectorX( strValue, vec2, 2 ) ;
		}
		break ;
	case	S3DSceneComposer::typeSelector:
	case	S3DSceneComposer::typeCommand:
	case	S3DSceneComposer::typePose:
	case	S3DSceneComposer::typeBinary:
		strValue = GetCommandParameter( i ) ;
		break ;
	default:
		strValue.FreeArray() ;
		break ;
	}
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Parameter::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
}

void S3DSceneComposer::Parameter::SetVectorParameter( size_t i, const S3DDVector& vec )
{
}

void S3DSceneComposer::Parameter::SetScalarParameter( size_t i, double s )
{
}

void S3DSceneComposer::Parameter::SetIntegerParameter( size_t i, int32_t n )
{
}

void S3DSceneComposer::Parameter::SetBooleanParameter( size_t i, bool b )
{
}

void S3DSceneComposer::Parameter::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
}

void S3DSceneComposer::Parameter::SetPoseParameter
		( size_t i, const S3DSceneComposer::PoseInstance& pose )
{
}

size_t S3DSceneComposer::Parameter::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	return	0 ;
}

bool S3DSceneComposer::Parameter::SetParameter
	( S3DSceneComposer::ParameterType type,
		size_t i, const SSystem::SString& strValue )
{
	switch ( type )
	{
	case	S3DSceneComposer::typeMatrix:
	case	S3DSceneComposer::typeRotation:
		{
			S3DDMatrix	mat ;
			if ( ParseMatrix( mat, strValue ) )
			{
				SetMatrixParameter( i, mat ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typePosition:
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		{
			S3DDVector	pos ;
			if ( ParseVector( pos, strValue ) )
			{
				SetVectorParameter( i, pos ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeScalar:
		{
			double	s ;
			if ( ParseScalar( s, strValue ) )
			{
				SetScalarParameter( i, s ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeInteger:
		{
			int32_t	n ;
			if ( ParseInteger( n, strValue ) )
			{
				SetIntegerParameter( i, n ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeBoolean:
		{
			bool	b ;
			if ( ParseBoolean( b, strValue ) )
			{
				SetBooleanParameter( i, b ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeMatrix4:
		{
			double	mat4[16] ;
			if ( ParseVectorX( mat4, 16, strValue ) )
			{
				SetBinaryParameter( i, mat4, sizeof(mat4) ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeVector4:
		{
			double	vec4[4] ;
			if ( ParseVectorX( vec4, 4, strValue ) )
			{
				SetBinaryParameter( i, vec4, sizeof(vec4) ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeVector2:
		{
			double	vec2[2] ;
			if ( ParseVectorX( vec2, 2, strValue ) )
			{
				SetBinaryParameter( i, vec2, sizeof(vec2) ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeSelector:
	case	S3DSceneComposer::typeCommand:
	case	S3DSceneComposer::typePose:
	case	S3DSceneComposer::typeBinary:
		SetCommandParameter( i, strValue ) ;
		return	true ;
	default:
		break ;
	}
	return	false ;
}

bool S3DSceneComposer::Parameter::SetVariant
		( size_t iParam, const S3DSceneComposer::Variant& val )
{
	switch ( val.GetType() )
	{
	case	S3DSceneComposer::typeMatrix:
	case	S3DSceneComposer::typeRotation:
		SetMatrixParameter( iParam, val.GetMatrix() ) ;
		return	true ;

	case	S3DSceneComposer::typePosition:
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		SetVectorParameter( iParam, val.GetVector() ) ;
		return	true ;

	case	S3DSceneComposer::typeScalar:
		SetScalarParameter( iParam, val.GetScalar() ) ;
		return	true ;

	case	S3DSceneComposer::typeInteger:
		SetIntegerParameter( iParam, val.GetInteger() ) ;
		return	true ;

	case	S3DSceneComposer::typeBoolean:
		SetBooleanParameter( iParam, val.GetBoolean() ) ;
		return	true ;

	case	S3DSceneComposer::typeMatrix4:
	case	S3DSceneComposer::typeVector4:
	case	S3DSceneComposer::typeVector2:
	case	S3DSceneComposer::typeBinary:
		SetBinaryParameter( iParam, val.GetData(), val.GetDataBytes() ) ;
		return	true ;

	case	S3DSceneComposer::typePose:
		SetPoseParameter( iParam, val.GetPose() ) ;
		return	true ;

	case	S3DSceneComposer::typeSelector:
	case	S3DSceneComposer::typeCommand:
		SetCommandParameter( iParam, val.GetString() ) ;
		return	true ;

	default:
		break ;
	}
	return	false ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Parameter::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	return	false ;
}

// バイナリヘッダ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Parameter::GetBinaryParameterHeader
		( S3DSceneComposer::BinaryHeader& hdr, size_t i ) const
{
	if ( GetBinaryParameter
		( &hdr, sizeof(S3DSceneComposer::BinaryHeader), i )
						== sizeof(S3DSceneComposer::BinaryHeader) )
	{
		return	true ;
	}
	return	false ;
}

// インスタンス・パラメータ
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Parameter::GetInstancingParameter
	( SSystem::SArray<BinaryInstancingEntry>& aInstancing, size_t i ) const
{
	size_t	nDataBytes = GetBinaryParameter( nullptr, 0, i ) ;
	S3DSceneComposer::BinaryHeader *
		pbhdr = (S3DSceneComposer::BinaryHeader*) esl_malloc( nDataBytes ) ;
	GetBinaryParameter( pbhdr, nDataBytes, i ) ;
	//
	bool	flagSuccess = false ;
	if ( pbhdr->nType == binaryInstancing )
	{
		S3DSceneComposer::BinaryInstancingData *
			pbid = (S3DSceneComposer::BinaryInstancingData*) pbhdr->GetBodyPtr() ;
		aInstancing.AddArray( pbid->entries, pbid->count ) ;
		flagSuccess = true ;
	}
	esl_free( pbhdr ) ;
	return	flagSuccess ;
}

void S3DSceneComposer::Parameter::SetInstancingParameter
	( size_t i, const BinaryInstancingEntry * pInstancing, size_t nCount )
{
	size_t	nDataBytes = sizeof(S3DSceneComposer::BinaryHeader)
				+ sizeof(S3DSceneComposer::BinaryInstancingData)
				+ sizeof(S3DSceneComposer::BinaryInstancingEntry) * (nCount - 1) ;
	//
	S3DSceneComposer::BinaryHeader *
		pbhdr = (S3DSceneComposer::BinaryHeader*) esl_malloc( nDataBytes ) ;
	pbhdr->nType = S3DSceneComposer::binaryInstancing ;
	pbhdr->nSubType = 0 ;
	pbhdr->nBodyBytes = (uint32_t) (nDataBytes - sizeof(S3DSceneComposer::BinaryHeader)) ;
	pbhdr->nReserved = 0 ;
	//
	S3DSceneComposer::BinaryInstancingData *
		pbid = (S3DSceneComposer::BinaryInstancingData*) pbhdr->GetBodyPtr() ;
	pbid->count = (uint32_t) nCount ;
	//
	eslCopyMemory
		( pbid->entries, pInstancing,
			nCount * sizeof(S3DSceneComposer::BinaryInstancingEntry) ) ;
	//
	SetBinaryParameter( i, pbhdr, nDataBytes ) ;
	//
	esl_free( pbhdr ) ;
}

// パラメータ解釈
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Parameter::ParseMatrix( S3DDMatrix& mat, const SSystem::SString& strValue )
{
	SStringParser	sparsValue ;
	sparsValue.AttachString( strValue ) ;
	//
	double	m[9] ;
	if ( sparsValue.ParseNumberArray( &m[0], 9 ) < 9 )
	{
		return	false ;
	}
	mat.m[0][0] = m[0] ;
	mat.m[0][1] = m[1] ;
	mat.m[0][2] = m[2] ;
	mat.m[1][0] = m[3] ;
	mat.m[1][1] = m[4] ;
	mat.m[1][2] = m[5] ;
	mat.m[2][0] = m[6] ;
	mat.m[2][1] = m[7] ;
	mat.m[2][2] = m[8] ;
	return	true ;
}

bool S3DSceneComposer::Parameter::ParseVector( S3DDVector& vec, const SSystem::SString& strValue )
{
	SStringParser	sparsValue ;
	sparsValue.AttachString( strValue ) ;
	//
	double	v[3] ;
	if ( sparsValue.ParseNumberArray( &v[0], 3 ) < 3 )
	{
		return	false ;
	}
	vec.x = v[0] ;
	vec.y = v[1] ;
	vec.z = v[2] ;
	return	true ;
}

bool S3DSceneComposer::Parameter::ParseScalar( double& s, const SSystem::SString& strValue )
{
	SStringParser	sparsValue ;
	sparsValue.AttachString( strValue ) ;
	//
	int	type = sparsValue.IsNextNumber() ;
	if ( type == SStringParser::numberInvalid )
	{
		return	false ;
	}
	s = sparsValue.NextRealNumber( type ) ;
	return	true ;
}

bool S3DSceneComposer::Parameter::ParseInteger( int32_t& n, const SSystem::SString& strValue )
{
	bool	fError = false ;
	n = (int32_t) strValue.AsInteger( 10, true, &fError ) ;
	return	!fError ;
}

bool S3DSceneComposer::Parameter::ParseBoolean( bool& b, const SSystem::SString& strValue )
{
	if ( strValue == L"true" )
	{
		b = true ;
		return	true ;
	}
	if ( strValue == L"false" )
	{
		b = false ;
		return	true ;
	}
	b = false ;
	return	false ;
}

bool S3DSceneComposer::Parameter::ParseVectorX
	( double * pVec, size_t nCount, const SSystem::SString& strValue )
{
	SStringParser	sparsValue ;
	sparsValue.AttachString( strValue ) ;
	return	(sparsValue.ParseNumberArray( pVec, nCount ) == nCount) ;
}

// パラメータフォーマット
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Parameter::FormatMatrix( SSystem::SString& strValue, const S3DDMatrix& mat )
{
	strValue.Format
		( L"%f %f %f %f %f %f %f %f %f",
			mat.m[0][0], mat.m[0][1], mat.m[0][2],
			mat.m[1][0], mat.m[1][1], mat.m[1][2],
			mat.m[2][0], mat.m[2][1], mat.m[2][2] ) ;
}

void S3DSceneComposer::Parameter::FormatVector( SSystem::SString& strValue, const S3DDVector& vec )
{
	strValue.Format( L"%f %f %f", vec.x, vec.y, vec.z ) ;
}

void S3DSceneComposer::Parameter::FormatScalar( SSystem::SString& strValue, double s )
{
	strValue.FromReal( s ) ;
}

void S3DSceneComposer::Parameter::FormatInteger( SSystem::SString& strValue, int32_t n )
{
	strValue.FromInteger( n ) ;
}

void S3DSceneComposer::Parameter::FormatBoolean( SSystem::SString& strValue, bool b )
{
	if ( b )
	{
		strValue = L"true" ;
	}
	else
	{
		strValue = L"false" ;
	}
}

void S3DSceneComposer::Parameter::FormatVectorX( SSystem::SString& strValue, const double * pVec, size_t nCount )
{
	SString	strElement ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( i != 0 )
		{
			strValue += L" " ;
		}
		strElement.FromReal( pVec[i] ) ;
		strValue += strElement ;
	}
}

// S3DDVector -> SGLPalette 変換
//////////////////////////////////////////////////////////////////////////////
SGLPalette S3DSceneComposer::Parameter::ColorFromVector( const S3DDVector& vec )
{
	SGLPalette	rgb ;
	rgb.argb.Red =
		(uint8_t) esl_clampi
			( eslRoundR32ToInt( (float32_t) vec.x ), 0, 0xFF ) ;
	rgb.argb.Green =
		(uint8_t) esl_clampi
			( eslRoundR32ToInt( (float32_t) vec.y ), 0, 0xFF ) ;
	rgb.argb.Blue =
		(uint8_t) esl_clampi
			( eslRoundR32ToInt( (float32_t) vec.z ), 0, 0xFF ) ;
	return	rgb ;
}

// SGLPalette -> S3DDVector 変換
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSceneComposer::Parameter::VectorFromColor( const SGLPalette& rgb )
{
	return	S3DDVector( rgb.argb.Red, rgb.argb.Green, rgb.argb.Blue ) ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Parameter::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneParameter" ;
}



//////////////////////////////////////////////////////////////////////////////
// アイテム生成エントリ
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::ItemCreator, SObject )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::ESLItemCreator, ItemCreator )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ESLItemCreator::ESLItemCreator( const ItemClassDescriptor* pDesc )
{
	ESLAssert( pDesc != nullptr ) ;
	m_pRuntimeClass = pDesc->pRuntimeClass ;
}

// アイテム生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Parameter * S3DSceneComposer::ESLItemCreator::NewItem( void )
{
	ESLAssert( m_pRuntimeClass->pfnNewObject != nullptr ) ;
	if ( m_pRuntimeClass->pfnNewObject != nullptr )
	{
		return	ESLSmartCast<S3DSceneComposer::Parameter>
									( (m_pRuntimeClass->pfnNewObject)() ) ;
	}
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// シーケンサ
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::Sequencer::m_aiInterpolateMethods[6] =
{
	{ L"signal", methodSignal},
	{ L"selector", methodSelector },
	{ L"linear", methodLinear },
	{ L"spline", methodSpline },
	{ L"bezier", methodBezier },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::Sequencer, Parameter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer::Sequencer( void )
{
	m_methodInterpolate = methodLinear ;
}

S3DSceneComposer::Sequencer::Sequencer( const Sequencer& seq )
	: m_methodInterpolate( seq.m_methodInterpolate ),
		m_arrKeyFrames( seq.m_arrKeyFrames )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer::~Sequencer( void )
{
}

// 補完方法取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer::InterpolateMethod
	S3DSceneComposer::Sequencer::GetInterpolateMethod( void ) const
{
	return	m_methodInterpolate ;
}

// 補完方法設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Sequencer::SetInterpolateMethod
	( S3DSceneComposer::Sequencer::InterpolateMethod method )
{
	m_methodInterpolate = method ;
}

// キーフレーム範囲取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Sequencer::GetKeyFrameRange( int32_t& iFirst, int32_t& iEnd ) const
{
	size_t	nCount = m_arrKeyFrames.GetLength() ;
	if ( nCount == 0 )
	{
		iFirst = 0 ;
		iEnd = 0 ;
		return	false ;
	}
	iFirst = m_arrKeyFrames.At(0).iFrame ;
	iEnd = m_arrKeyFrames.At(nCount - 1).iFrame ;
	return	true ;
}

// キーフレームパラメータ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Sequencer::GetKeyFrameParameter
		( size_t i, S3DSceneComposer::KeyFrameParam& kfp ) const
{
	KeyFrameParam *	pkfp = m_arrKeyFrames.GetAt( i ) ;
	if ( pkfp == nullptr )
	{
		return	false ;
	}
	kfp = *pkfp ;
	return	true ;
}

// キーフレームパラメータ設定
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Sequencer::SetKeyFrameParameter
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	KeyFrameParam *	pkfp = m_arrKeyFrames.GetAt( i ) ;
	if ( pkfp == nullptr )
	{
		return	false ;
	}
	*pkfp = kfp ;
	return	true ;
}

// キーフレーム個数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::Sequencer::GetKeyFrameCount( void ) const
{
	return	m_arrKeyFrames.GetLength() ;
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Sequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	m_arrKeyFrames.InsertAt( i, kfp ) ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Sequencer::RemoveKeyFrame( size_t i )
{
	m_arrKeyFrames.RemoveAt( i ) ;
}

// キーフレームから各フレームの値を計算する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Sequencer::UpdateAllFrameValues( void )
{
}

// キーフレーム順を正規化する（後ろのキーフレームが前に来ないように）
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Sequencer::NormalizeKeyFrameOrder( void )
{
	KeyFrameParam *	pkfp = m_arrKeyFrames.GetArray() ;
	const size_t	nLength = m_arrKeyFrames.GetLength() ;
	bool			flagModified = false ;
	for ( size_t i = 1; i < nLength; i ++ )
	{
		KeyFrameParam&	kfpLast = pkfp[i - 1] ;
		KeyFrameParam&	kfpCur = pkfp[i] ;
		if ( kfpLast.iFrame > kfpCur.iFrame )
		{
			SwapKeyFrame( i - 1, i ) ;
			flagModified = true ;
		}
	}
	return	flagModified ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Sequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	m_arrKeyFrames.Swap( i1, i2 ) ;
}

// キーフレーム挿入指標検索
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::Sequencer::OrderKeyFrameIndex( int32_t iFrame ) const
{
	const KeyFrameParam *	pkfp = m_arrKeyFrames.GetConstArray() ;
	const ssize_t	nLength = (ssize_t) m_arrKeyFrames.GetLength() ;
	ssize_t			iFirst = 0 ;
	ssize_t			iEnd = nLength - 1 ;
	while ( iFirst <= iEnd )
	{
		size_t	iMiddle = (iFirst + iEnd) >> 1 ;
		if ( pkfp[iMiddle].iFrame > iFrame )
		{
			iEnd = (ssize_t) iMiddle - 1 ;
		}
		else if ( pkfp[iMiddle].iFrame < iFrame )
		{
			iFirst = (ssize_t) iMiddle + 1 ;
		}
		else
		{
			iFirst = (ssize_t) iMiddle ;
			break ;
		}
	}
	return	(size_t) iFirst ;
}

// キーフレーム検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSceneComposer::Sequencer::FindKeyFrame( int32_t iFrame ) const
{
	size_t			i = OrderKeyFrameIndex( iFrame ) ;
	KeyFrameParam *	pkfp = m_arrKeyFrames.GetAt( i ) ;
	if ( pkfp != nullptr )
	{
		if ( pkfp->iFrame == iFrame )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// 区間取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::Sequencer::GetKeyFrameProgress( double& t, int32_t iFrame ) const
{
	size_t			i = OrderKeyFrameIndex( iFrame ) ;
	KeyFrameParam *	pkfp1 = m_arrKeyFrames.GetAt( i ) ;
	if ( pkfp1 == nullptr )
	{
		t = 0.0 ;
		if ( m_arrKeyFrames.GetLength() == 0 )
		{
			return	0 ;
		}
		return	m_arrKeyFrames.GetLength() - 1 ;
	}
	if ( pkfp1->iFrame == iFrame )
	{
		t = 0.0 ;
		return	i ;
	}
	KeyFrameParam *	pkfp0 = m_arrKeyFrames.GetAt( i - 1 ) ;
	if ( pkfp0 == nullptr )
	{
		t = 0.0 ;
		return	i ;
	}
	double	ts = (double) (iFrame - pkfp0->iFrame)
							/ (pkfp1->iFrame - pkfp0->iFrame) ;
	double	tr = 1.0 - ts ;
	//
	double	b[4] ;
	b[0] = 0.0 ;
	b[1] =       pkfp0->speedOut * (1.0 / 3.0) ;
	b[2] = 1.0 - pkfp1->speedIn * (1.0 / 3.0) ;
	b[3] = 1.0 ;
	//
	double	p ;
	p =  b[1] * (3.0 * ts * tr * tr) ;
	p += b[2] * (3.0 * ts * ts * tr) ;
	p +=        (ts * ts * ts) ;
	//
	t = p ;
	return	i - 1 ;
}

// シーケンサー解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Sequencer::ParseSequencer
	( const SSystem::SXMLDocument& xmlTag,
			S3DSceneComposer::ParameterType type )
{
	bool	flagSequence = false ;
	m_methodInterpolate =
		(InterpolateMethod) xmlTag.GetAttrSymbolizedIntegerAs
			( L"interpolation", m_aiInterpolateMethods, m_methodInterpolate ) ;
	for ( size_t j = 0; j < xmlTag.GetElementsCount(); j ++ )
	{
		SXMLDocument *	pxmlKey = xmlTag.GetElementAt( j ) ;
		if ( pxmlKey == nullptr )
		{
			continue ;
		}
		if ( pxmlKey->GetTag() == L"keyframe" )
		{
			KeyFrameParam	kfp ;
			kfp.iFrame = (int32_t) pxmlKey->GetAttrIntegerAs( L"frame" ) ;
			kfp.nFlags =
				(uint32_t) pxmlKey->GetAttrComplexIntegerAs
					( L"flags", &S3DSceneComposer::m_aiKeyFrameFlags[0] ) ;
			kfp.speedIn = (float32_t) pxmlKey->GetAttrRealAs( L"in_speed" ) ;
			kfp.speedOut = (float32_t) pxmlKey->GetAttrRealAs( L"out_speed" ) ;
			//
			size_t	k = OrderKeyFrameIndex( kfp.iFrame ) ;
			InsertKeyFrame( k, kfp ) ;
			//
			const SString *	pstrValue = pxmlKey->GetAttributeAs( L"value" ) ;
			if ( pstrValue != nullptr )
			{
				if ( m_methodInterpolate == methodBezier )
				{
					SetParameter( type, k * 3, *pstrValue ) ;
					//
					const SString *	pstrHandle0 =
							pxmlKey->GetAttributeAs( L"handle0" ) ;
					if ( (k > 0) && (pstrHandle0 != nullptr) )
					{
						SetParameter( type, k * 3 - 1, *pstrHandle0 ) ;
					}
					const SString *	pstrHandle1 =
							pxmlKey->GetAttributeAs( L"handle1" ) ;
					if ( pstrHandle1 != nullptr )
					{
						SetParameter( type, k * 3 + 1, *pstrHandle1 ) ;
					}
				}
				else
				{
					SetParameter( type, k, *pstrValue ) ;
				}
			}
		}
		else if ( pxmlKey->GetTag() == L"sequence" )
		{
			ParseFrameSequence( *pxmlKey, type ) ;
			flagSequence = true ;
		}
		else if ( pxmlKey->GetTag() == L"default" )
		{
			const SString *	pstrValue = pxmlKey->GetAttributeAs( L"value" ) ;
			if ( pstrValue != nullptr )
			{
				SetParameter( type, (size_t) indexDefault, *pstrValue ) ;
			}
		}
	}
	if ( !flagSequence )
	{
		UpdateAllFrameValues() ;
	}
	return	sglErrSuccess ;
}

// アニメーションフレーム解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Sequencer::ParseFrameSequence
	( const SSystem::SXMLDocument& xmlSeq,
			S3DSceneComposer::ParameterType type )
{
	return	sglErrFailed ;
}

// シーケンサー保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Sequencer::FormatSequencer
	( SSystem::SXMLDocument& xmlTag,
			S3DSceneComposer::ParameterType type, uint32_t nFlags )
{
	xmlTag.SetAttrSymbolizedIntegerAs
		( L"interpolation", m_aiInterpolateMethods, m_methodInterpolate ) ;
	//
	SXMLDocument *	pxmlDefault = new SXMLDocument ;
	pxmlDefault->SetTag( L"default" ) ;
	xmlTag.AddElement( pxmlDefault ) ;
	{
		SString	strValue ;
		GetParameter( strValue, type, (size_t) indexDefault ) ;
		pxmlDefault->SetAttributeAs( L"value", strValue ) ;
	}
	const KeyFrameParam *	pkfp = m_arrKeyFrames.GetConstArray() ;
	const size_t	nCount = m_arrKeyFrames.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlKey = new SXMLDocument ;
		pxmlKey->SetTag( L"keyframe" ) ;
		xmlTag.AddElement( pxmlKey ) ;
		//
		pxmlKey->SetAttrIntegerAs( L"frame", pkfp[i].iFrame ) ;
		pxmlKey->SetAttrComplexIntegerAs
			( L"flags", &S3DSceneComposer::m_aiKeyFrameFlags[0], pkfp[i].nFlags ) ;
		pxmlKey->SetAttrRealAs( L"in_speed", pkfp[i].speedIn ) ;
		pxmlKey->SetAttrRealAs( L"out_speed", pkfp[i].speedOut ) ;
		//
		SString	strValue ;
		if ( m_methodInterpolate == methodBezier )
		{
			GetParameter( strValue, type, i * 3 ) ;
			pxmlKey->SetAttributeAs( L"value", strValue ) ;
			//
			if ( i > 0 )
			{
				GetParameter( strValue, type, i * 3 - 1 ) ;
				pxmlKey->SetAttributeAs( L"handle0", strValue ) ;
			}
			GetParameter( strValue, type, i * 3 + 1 ) ;
			pxmlKey->SetAttributeAs( L"handle1", strValue ) ;
		}
		else
		{
			GetParameter( strValue, type, i ) ;
			pxmlKey->SetAttributeAs( L"value", strValue ) ;
		}
	}
	if ( nFlags & S3DSceneComposer::formatOptWithFrameSequence )
	{
		SXMLDocument *	pxmlSeq = new SXMLDocument ;
		pxmlSeq->SetTag( L"sequence" ) ;
		if ( !FormatFrameSequence( *pxmlSeq, type ) )
		{
			xmlTag.AddElement( pxmlSeq ) ;
		}
		else
		{
			delete	pxmlSeq ;
		}
	}
	return	sglErrSuccess ;
}

// アニメーションフレーム保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Sequencer::FormatFrameSequence
	( SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	return	sglErrFailed ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::Sequencer::GetFrameMatrix( double fpFrame )
{
	return	S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
}

S3DDVector S3DSceneComposer::Sequencer::GetFrameVector( double fpFrame )
{
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DSceneComposer::Sequencer::GetFrameScalar( double fpFrame )
{
	return	0.0 ;
}

int32_t S3DSceneComposer::Sequencer::GetFrameInteger( double fpFrame )
{
	return	0 ;
}

bool S3DSceneComposer::Sequencer::GetFrameBoolean( double fpFrame )
{
	return	false ;
}

const wchar_t * S3DSceneComposer::Sequencer::GetFrameCommand
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	return	nullptr ;
}

S3DSceneComposer::PoseInstance
	S3DSceneComposer::Sequencer::GetFramePoseInstance( double fpFrame )
{
	return	PoseInstance() ;
}

size_t S3DSceneComposer::Sequencer::GetFrameBinary
	( void * pDst, size_t nBufBytes, double fpFrame ) const
{
	return	0 ;
}

// フレームシーケンス作成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Sequencer::CreateFrameSequence
						( size_t iFirstFrame, size_t nDuration )
{
	return	sglErrFailed ;
}

// フレーム値設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Sequencer::SetFrameMatrix( size_t iFrame, const S3DDMatrix& mat )
{
	return	sglErrFailed ;
}

SGLError S3DSceneComposer::Sequencer::SetFrameVector( size_t iFrame, const S3DDVector& vec )
{
	return	sglErrFailed ;
}

SGLError S3DSceneComposer::Sequencer::SetFrameScalar( size_t iFrame, double s )
{
	return	sglErrFailed ;
}

SGLError S3DSceneComposer::Sequencer::SetFrameInteger( size_t iFrame, int32_t n )
{
	return	sglErrFailed ;
}

SGLError S3DSceneComposer::Sequencer::SetFrameBoolean( size_t iFrame, bool b )
{
	return	sglErrFailed ;
}

size_t S3DSceneComposer::Sequencer::SetFrameBinary
	( size_t iFrame, const void * pSrc, size_t nBufBytes )
{
	return	0 ;
}

// シーケンサー生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::Sequencer::NewParameterSequencer
					( S3DSceneComposer::ParameterType type )
{
	Sequencer *	pSeq = nullptr ;
	switch ( type )
	{
	case	S3DSceneComposer::typeMatrix:
		pSeq = new MatrixSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodLinear ) ;
		break ;
	case	S3DSceneComposer::typePosition:
		pSeq = new VectorSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodSpline ) ;
		break ;
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		pSeq = new VectorSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodLinear ) ;
		break ;
	case	S3DSceneComposer::typeRotation:
		pSeq = new RotationSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodLinear ) ;
		break ;
	case	S3DSceneComposer::typeScalar:
		pSeq = new ScalarSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodLinear ) ;
		break ;
	case	S3DSceneComposer::typeInteger:
		pSeq = new IntegerSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodSelector ) ;
		break ;
	case	S3DSceneComposer::typeBoolean:
		pSeq = new BooleanSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodSelector ) ;
		break ;
	case	S3DSceneComposer::typeSelector:
		pSeq = new CommandSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodSelector ) ;
		break ;
	case	S3DSceneComposer::typeCommand:
		pSeq = new CommandSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodSignal ) ;
		break ;
	case	S3DSceneComposer::typePose:
		pSeq = new PoseSequencer ;
		pSeq->SetInterpolateMethod( Sequencer::methodLinear ) ;
		break ;
	case	S3DSceneComposer::typeMatrix4:
		pSeq = new VectorXSequencer( 16 ) ;
		break ;
	case	S3DSceneComposer::typeVector4:
		pSeq = new VectorXSequencer( 4 ) ;
		break ;
	case	S3DSceneComposer::typeVector2:
		pSeq = new VectorXSequencer( 2 ) ;
		break ;
	default:
		break ;
	}
	return	pSeq ;
}

// シーケンサー型照合
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Sequencer::VerifyParameterSequencer
	( S3DSceneComposer::ParameterType type, S3DSceneComposer::Sequencer * pSeq )
{
	VectorXSequencer *	pVecXSeq ;
	switch ( type )
	{
	case	S3DSceneComposer::typeMatrix:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(MatrixSequencer) ) ;

	case	S3DSceneComposer::typePosition:
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(VectorSequencer) ) ;

	case	S3DSceneComposer::typeRotation:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(RotationSequencer) ) ;

	case	S3DSceneComposer::typeScalar:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(ScalarSequencer) ) ;

	case	S3DSceneComposer::typeInteger:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(IntegerSequencer) ) ;

	case	S3DSceneComposer::typeBoolean:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(BooleanSequencer) ) ;

	case	S3DSceneComposer::typeSelector:
	case	S3DSceneComposer::typeCommand:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(CommandSequencer) ) ;

	case	S3DSceneComposer::typePose:
		return	pSeq->IsKindOf( ESL_RUNTIME_CLASS(PoseSequencer) ) ;

	case	S3DSceneComposer::typeMatrix4:
		pVecXSeq = ESLTypeCast<VectorXSequencer>( pSeq ) ;
		return	(pVecXSeq != nullptr) && (pVecXSeq-> GetDimension() == 16) ;

	case	S3DSceneComposer::typeVector4:
		pVecXSeq = ESLTypeCast<VectorXSequencer>( pSeq ) ;
		return	(pVecXSeq != nullptr) && (pVecXSeq-> GetDimension() == 4) ;

	case	S3DSceneComposer::typeVector2:
		pVecXSeq = ESLTypeCast<VectorXSequencer>( pSeq ) ;
		return	(pVecXSeq != nullptr) && (pVecXSeq-> GetDimension() == 2) ;

	default:
		break ;
	}
	return	false ;
}

// 作成時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Sequencer::OnSequencerAttached
			( S3DSceneComposer::ParameterProperty * pItem )
{
}

// リソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::Sequencer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
			ItemSerializer * pItem, uint32_t nFlags )
{
	return	0 ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Sequencer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneSequencer" ;
}



//////////////////////////////////////////////////////////////////////////////
// 行列シーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::MatrixSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::MatrixSequencer::MatrixSequencer( void )
	: m_matDefault( 1, 0, 0,  0, 1, 0,  0, 0, 1 )
{
	m_iFirstFrame = 0 ;
	m_fEachFrames = false ;
}

S3DSceneComposer::MatrixSequencer::MatrixSequencer( const MatrixSequencer& seq )
	: Sequencer( seq ),
		m_matDefault( seq.m_matDefault ),
		m_arrValues( seq.m_arrValues ),
		m_arrFrames( seq.m_arrFrames ),
		m_iFirstFrame( seq.m_iFirstFrame ),
		m_fEachFrames( seq.m_fEachFrames )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::MatrixSequencer::~MatrixSequencer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::MatrixSequencer::GetMatrixParameter( size_t i ) const
{
	S3DDMatrix *	pm = m_arrValues.GetAt( i ) ;
	if ( pm != nullptr )
	{
		return	*pm ;
	}
	return	m_matDefault ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::MatrixSequencer::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	S3DDMatrix *	pm = m_arrValues.GetAt( i ) ;
	if ( pm != nullptr )
	{
		*pm = mat ;
		m_fEachFrames = false ;
	}
	else
	{
		m_matDefault = mat ;
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::MatrixSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.InsertAt( i, matI ) ;
	m_fEachFrames = false ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::MatrixSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.RemoveAt( i ) ;
	m_fEachFrames = false ;
}

// キーフレームから各フレームの値を計算する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::MatrixSequencer::UpdateAllFrameValues( void )
{
	const KeyFrameParam *	pkfp = m_arrKeyFrames.GetConstArray() ;
	const size_t	nLength = m_arrKeyFrames.GetLength() ;
	if ( nLength <= 1 )
	{
		m_fEachFrames = false ;
		return ;
	}
	NormalizeKeyFrameOrder() ;
	//
	const S3DDMatrix *	pKeyMatrix = m_arrValues.GetConstArray() ;
	ESLAssert( m_arrValues.GetLength() == nLength ) ;
	//
	ESLAssert( pkfp[nLength - 1].iFrame >= pkfp[0].iFrame ) ;
	m_arrFrames.SetLength
		( pkfp[nLength - 1].iFrame - pkfp[0].iFrame + 1 ) ;
	//
	S3DDMatrix *	pFrameMat = m_arrFrames.GetArray() ;
	size_t			iDstFrame = 0 ;
	for ( size_t i = 1; i < nLength; i ++ )
	{
		const KeyFrameParam *	pkfp0 = pkfp + (i - 1) ;
		const KeyFrameParam *	pkfp1 = pkfp + i ;
		//
		SGLBezierCurves<double>	bzFrame ;
		bzFrame.SetLine
			( 0.0, 1.0, pkfp0->speedOut, pkfp1->speedIn ) ;
		//
		while ( (int32_t) iDstFrame + pkfp[0].iFrame <= pkfp1->iFrame )
		{
			double	t = 1.0 ;
			if ( pkfp1->iFrame > pkfp0->iFrame )
			{
				int	iFrame = (int) iDstFrame + pkfp[0].iFrame ;
				t = (double) (iFrame - pkfp0->iFrame)
								/ (pkfp1->iFrame - pkfp0->iFrame) ;
				t = bzFrame.PointAt( t ) ;
			}
			pFrameMat[iDstFrame ++] =
				pKeyMatrix[i - 1] * (1.0 - t) + pKeyMatrix[i] * t ;
		}
	}
	m_arrFrames.FinishArray() ;
	m_iFirstFrame = pkfp[0].iFrame ;
	m_fEachFrames = true ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::MatrixSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
}

// アニメーションフレーム解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::MatrixSequencer::ParseFrameSequence
	( const SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	size_t	nDuration = (size_t) xmlSeq.GetAttrIntegerAs( L"duration", 0 ) ;
	m_iFirstFrame = (size_t) xmlSeq.GetAttrIntegerAs( L"first_frame", 0 ) ;
	if ( nDuration == 0 )
	{
		m_fEachFrames = false ;
		return	sglErrSuccess ;
	}
	m_arrFrames.SetLength( nDuration ) ;
	//
	S3DDMatrix *	pFrames = m_arrFrames.GetArray() ;
	SString *		pstrFrames = xmlSeq.GetTextElement() ;
	SStringParser	sparsFrames ;
	sparsFrames.AttachString( *pstrFrames ) ;
	//
	size_t	iDst = 0 ;
	SString	strFrame ;
	while ( (iDst < nDuration) && sparsFrames.PassSpace() )
	{
		sparsFrames.NextEnclosedString( strFrame, L'/' ) ;
		ParseMatrix( pFrames[iDst], strFrame ) ;
		iDst ++ ;
	}
	m_arrFrames.FinishArray() ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// アニメーションフレーム保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::MatrixSequencer::FormatFrameSequence
	( SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	if ( !m_fEachFrames )
	{
		return	sglErrSuccess ;
	}
	xmlSeq.SetAttrIntegerAs( L"duration", m_arrFrames.GetLength() ) ;
	xmlSeq.SetAttrIntegerAs( L"first_frame", m_iFirstFrame ) ;
	//
	SString				strFrames ;
	SString				strValue ;
	const S3DDMatrix *	pmatFrames = m_arrFrames.GetConstArray() ;
	for ( size_t i = 0; i < m_arrFrames.GetLength(); i ++ )
	{
		FormatMatrix( strValue, pmatFrames[i] ) ;
		strFrames += strValue ;
		strFrames += L" / " ;
	}
	xmlSeq.SetTextElement( strFrames ) ;
	return	sglErrSuccess ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::MatrixSequencer::GetFrameMatrix( double fpFrame )
{
	int	iFrame = (int) eslRoundR64ToLInt( fpFrame ) ;
	if ( m_fEachFrames )
	{
		ESLAssert( m_arrFrames.GetLength() >= 1 ) ;
		iFrame = esl_clampi
					( iFrame - (int) m_iFirstFrame, 0,
							(int) m_arrFrames.GetLength() - 1 ) ;
		return	m_arrFrames.At( (size_t) iFrame ) ;
	}
	double			t ;
	size_t			i = GetKeyFrameProgress( t, iFrame ) ;
	S3DDMatrix *	pm0 = m_arrValues.GetAt( i ) ;
	S3DDMatrix *	pm1 = m_arrValues.GetAt( i + 1 ) ;
	if ( (pm1 == nullptr) || (t == 0.0) )
	{
		if ( pm0 != nullptr )
		{
			return	*pm0 ;
		}
		return	m_matDefault ;
	}
	ESLAssert( pm0 != nullptr ) ;
	return	*pm0 * (1.0 - t) + *pm1 * t ;
}

// フレームシーケンス作成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::MatrixSequencer::CreateFrameSequence
							( size_t iFirstFrame, size_t nDuration )
{
	m_arrFrames.SetLength( nDuration ) ;
	m_iFirstFrame = iFirstFrame ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// フレーム値設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::MatrixSequencer::SetFrameMatrix( size_t iFrame, const S3DDMatrix& mat )
{
	S3DDMatrix *	pMatrix = m_arrFrames.GetAt( iFrame - m_iFirstFrame ) ;
	if ( pMatrix == nullptr )
	{
		return	sglErrFailed ;
	}
	*pMatrix = mat ;
	return	sglErrSuccess ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::MatrixSequencer::DuplicateSequencer( void )
{
	return	new MatrixSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 回転シーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::RotationSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::RotationSequencer::RotationSequencer( void )
	: m_qDefault( 1, 0, 0, 0 )
{
	m_iFirstFrame = 0 ;
	m_fEachFrames = false ;
}

S3DSceneComposer::RotationSequencer::RotationSequencer( const RotationSequencer& seq )
	: Sequencer( seq ),
		m_qDefault( seq.m_qDefault ),
		m_arrValues( seq.m_arrValues ),
		m_arrFrames( seq.m_arrFrames ),
		m_iFirstFrame( seq.m_iFirstFrame ),
		m_fEachFrames( seq.m_fEachFrames )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::RotationSequencer::~RotationSequencer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::RotationSequencer::GetMatrixParameter( size_t i ) const
{
	S3DDQuaternion *	pq = m_arrValues.GetAt( i ) ;
	S3DDMatrix	mat ;
	if ( pq != nullptr )
	{
		pq->ToMatrix( mat ) ;
	}
	else
	{
		m_qDefault.ToMatrix( mat ) ;
	}
	return	mat ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::RotationSequencer::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	S3DDQuaternion *	pq = m_arrValues.GetAt( i ) ;
	if ( pq != nullptr )
	{
		pq->FromMatrix( mat ) ;
		m_fEachFrames = false ;
	}
	else
	{
		m_qDefault.FromMatrix( mat ) ;
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::RotationSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	S3DDQuaternion	q( 1, 0, 0, 0 ) ;
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.InsertAt( i, q ) ;
	m_fEachFrames = false ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::RotationSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.RemoveAt( i ) ;
	m_fEachFrames = false ;
}

// キーフレームから各フレームの値を計算する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::RotationSequencer::UpdateAllFrameValues( void )
{
	const KeyFrameParam *	pkfp = m_arrKeyFrames.GetConstArray() ;
	const size_t	nLength = m_arrKeyFrames.GetLength() ;
	if ( nLength <= 1 )
	{
		m_fEachFrames = false ;
		return ;
	}
	NormalizeKeyFrameOrder() ;
	//
	const S3DDQuaternion *	pKeyMatrix = m_arrValues.GetConstArray() ;
	ESLAssert( m_arrValues.GetLength() == nLength ) ;
	//
	ESLAssert( pkfp[nLength - 1].iFrame >= pkfp[0].iFrame ) ;
	m_arrFrames.SetLength
		( pkfp[nLength - 1].iFrame - pkfp[0].iFrame + 1 ) ;
	//
	S3DDQuaternion *	pFrameMat = m_arrFrames.GetArray() ;
	size_t				iDstFrame = 0 ;
	for ( size_t i = 1; i < nLength; i ++ )
	{
		const KeyFrameParam *	pkfp0 = pkfp + (i - 1) ;
		const KeyFrameParam *	pkfp1 = pkfp + i ;
		//
		SGLBezierCurves<double>	bzFrame ;
		bzFrame.SetLine
			( 0.0, 1.0, pkfp0->speedOut, pkfp1->speedIn ) ;
		//
		while ( (int32_t) iDstFrame + pkfp[0].iFrame <= pkfp1->iFrame )
		{
			double	t = 1.0 ;
			if ( pkfp1->iFrame > pkfp0->iFrame )
			{
				int	iFrame = (int) iDstFrame + pkfp[0].iFrame ;
				t = (double) (iFrame - pkfp0->iFrame)
								/ (pkfp1->iFrame - pkfp0->iFrame) ;
				t = bzFrame.PointAt( t ) ;
			}
			S3DDQuaternion	q ;
			q.Slerp( pKeyMatrix[i - 1], pKeyMatrix[i], t ) ;
			//
			pFrameMat[iDstFrame ++] = q ;
		}
	}
	m_arrFrames.FinishArray() ;
	//
	m_iFirstFrame = pkfp[0].iFrame ;
	m_fEachFrames = true ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::RotationSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
}

// アニメーションフレーム解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::RotationSequencer::ParseFrameSequence
	( const SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	size_t	nDuration = (size_t) xmlSeq.GetAttrIntegerAs( L"duration", 0 ) ;
	m_iFirstFrame = (size_t) xmlSeq.GetAttrIntegerAs( L"first_frame", 0 ) ;
	if ( nDuration == 0 )
	{
		m_fEachFrames = false ;
		return	sglErrSuccess ;
	}
	m_arrFrames.SetLength( nDuration ) ;
	//
	S3DDQuaternion *	pFrames = m_arrFrames.GetArray() ;
	SString *			pstrFrames = xmlSeq.GetTextElement() ;
	SStringParser		sparsFrames ;
	sparsFrames.AttachString( *pstrFrames ) ;
	//
	size_t	iDst = 0 ;
	SString	strFrame ;
	while ( (iDst < nDuration) && sparsFrames.PassSpace() )
	{
		S3DDMatrix	mat ;
		sparsFrames.NextEnclosedString( strFrame, L'/' ) ;
		ParseMatrix( mat, strFrame ) ;
		pFrames[iDst].FromMatrix( mat ) ;
		iDst ++ ;
	}
	m_arrFrames.FinishArray() ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// アニメーションフレーム保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::RotationSequencer::FormatFrameSequence
	( SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	if ( !m_fEachFrames )
	{
		return	sglErrSuccess ;
	}
	xmlSeq.SetAttrIntegerAs( L"duration", m_arrFrames.GetLength() ) ;
	xmlSeq.SetAttrIntegerAs( L"first_frame", m_iFirstFrame ) ;
	//
	SString					strFrames ;
	SString					strValue ;
	const S3DDQuaternion *	pqtFrames = m_arrFrames.GetConstArray() ;
	for ( size_t i = 0; i < m_arrFrames.GetLength(); i ++ )
	{
		S3DDMatrix	mat ;
		pqtFrames[i].ToMatrix( mat ) ;
		FormatMatrix( strValue, mat ) ;
		strFrames += strValue ;
		strFrames += L" / " ;
	}
	xmlSeq.SetTextElement( strFrames ) ;
	return	sglErrSuccess ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::RotationSequencer::GetFrameMatrix( double fpFrame )
{
	if ( m_fEachFrames )
	{
		ESLAssert( m_arrFrames.GetLength() >= 1 ) ;
		int		iFrame0 = (int) floor( fpFrame ) ;
		int		iFrame1 = iFrame0 + 1 ;
		double	fpDecimal = fpFrame - iFrame0 ;
		iFrame0 = esl_clampi
					( iFrame0 - (int) m_iFirstFrame, 0,
							(int) m_arrFrames.GetLength() - 1 ) ;
		iFrame1 = esl_clampi
					( iFrame1 - (int) m_iFirstFrame, 0,
							(int) m_arrFrames.GetLength() - 1 ) ;
		//
		S3DDQuaternion	qt0 = m_arrFrames.At( (size_t) iFrame0 ) ;
		S3DDQuaternion	qt1 = m_arrFrames.At( (size_t) iFrame1 ) ;
		S3DDQuaternion	qtTemp = qt0 * (1.0 - fpDecimal) + qt1 * fpDecimal ;
		qtTemp.Normalize() ;
		//
		S3DDMatrix	matTemp ;
		qtTemp.ToMatrix( matTemp ) ;
		return	matTemp ;
	}
	const int			iFrame = (int) eslRoundR64ToLInt( fpFrame ) ;
	double				t ;
	size_t				i = GetKeyFrameProgress( t, iFrame ) ;
	S3DDQuaternion *	pq0 = m_arrValues.GetAt( i ) ;
	S3DDQuaternion *	pq1 = m_arrValues.GetAt( i + 1 ) ;
	if ( (pq1 == nullptr) || (t == 0.0) )
	{
		S3DDMatrix	mat ;
		if ( pq0 != nullptr )
		{
			pq0->ToMatrix( mat ) ;
		}
		else
		{
			m_qDefault.ToMatrix( mat ) ;
		}
		return	mat ;
	}
	ESLAssert( pq0 != nullptr ) ;
	S3DDQuaternion	q ;
	q.Slerp( *pq0, *pq1, t ) ;
	//
	S3DDMatrix	mat ;
	q.ToMatrix( mat ) ;
	return	mat ;
}

// フレームシーケンス作成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::RotationSequencer::CreateFrameSequence
							( size_t iFirstFrame, size_t nDuration )
{
	m_arrFrames.SetLength( nDuration ) ;
	m_iFirstFrame = iFirstFrame ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// フレーム値設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::RotationSequencer::SetFrameMatrix( size_t iFrame, const S3DDMatrix& mat )
{
	S3DDQuaternion *
		pQuaternion = m_arrFrames.GetAt( iFrame - m_iFirstFrame ) ;
	if ( pQuaternion == nullptr )
	{
		return	sglErrFailed ;
	}
	pQuaternion->FromMatrix( mat ) ;
	return	sglErrSuccess ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::RotationSequencer::DuplicateSequencer( void )
{
	return	new RotationSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 座標シーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::VectorSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::VectorSequencer::VectorSequencer( void )
	: m_vDefault( 0, 0, 0 )
{
	m_iFirstFrame = 0 ;
	m_fEachFrames = false ;
}

S3DSceneComposer::VectorSequencer::VectorSequencer( const VectorSequencer& seq )
	: Sequencer( seq ),
		m_vDefault( seq.m_vDefault ),
		m_arrValues( seq.m_arrValues ),
		m_arrFrames( seq.m_arrFrames ),
		m_iFirstFrame( seq.m_iFirstFrame ),
		m_fEachFrames( seq.m_fEachFrames )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::VectorSequencer::~VectorSequencer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSceneComposer::VectorSequencer::GetVectorParameter( size_t i ) const
{
	S3DDVector *	pv = m_arrValues.GetAt( i ) ;
	if ( pv != nullptr )
	{
		return	*pv ;
	}
	return	m_vDefault ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorSequencer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	S3DDVector *	pv = m_arrValues.GetAt( i ) ;
	if ( pv != nullptr )
	{
		*pv = vec ;
		m_fEachFrames = false ;
	}
	else
	{
		m_vDefault = vec ;
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	S3DDVector	v( 0, 0, 0 ) ;
	size_t	iv = i ;
	size_t	iStep = 1 ;
	if ( m_methodInterpolate == methodBezier )
	{
		iv = i * 3 ;
		iStep = 3 ;
	}
	if ( iv < m_arrValues.GetLength() )
	{
		v = m_arrValues.At( iv ) ;
	}
	Sequencer::InsertKeyFrame( i, kfp ) ;
	//
	if ( m_methodInterpolate == methodBezier )
	{
		if ( (iv >= 3) && (iv < m_arrValues.GetLength()) )
		{
			SGLBezierCurves<S3DDVector>	bzCurve ;
			bzCurve.SetAt( 0, m_arrValues.At( iv - 3 ) ) ;
			bzCurve.SetAt( 1, m_arrValues.At( iv - 2 ) ) ;
			bzCurve.SetAt( 2, m_arrValues.At( iv - 1 ) ) ;
			bzCurve.SetAt( 3, v ) ;
			//
			SGLBezierCurves<S3DDVector>	bzPrev, bzNext ;
			bzCurve.DivideBezier( 0.5, bzPrev, bzNext ) ;
			//
			m_arrValues.SetAt( iv - 3, bzPrev.At(0) ) ;
			m_arrValues.SetAt( iv - 2, bzPrev.At(1) ) ;
			m_arrValues.SetAt( iv - 1, bzPrev.At(2) ) ;
			m_arrValues.SetAt( iv, bzPrev.At(3) ) ;
			//
			m_arrValues.InsertAt( iv + 1, bzNext.At(1) ) ;
			m_arrValues.InsertAt( iv + 2, bzNext.At(2) ) ;
			m_arrValues.InsertAt( iv + 3, bzNext.At(3) ) ;
		}
		else
		{
			m_arrValues.InsertAt( iv, v ) ;
			m_arrValues.InsertAt( iv, v ) ;
			m_arrValues.InsertAt( iv, v ) ;
		}
	}
	else
	{
		if ( iv + 1 < m_arrValues.GetLength() )
		{
			v += (m_arrValues.At( iv + 1 ) - v) * 0.5 ;
		}
		m_arrValues.InsertAt( i, v ) ;
	}
	m_fEachFrames = false ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	//
	if ( m_methodInterpolate == methodBezier )
	{
		m_arrValues.Remove( i * 3, 3 ) ;
	}
	else
	{
		m_arrValues.RemoveAt( i ) ;
	}
	m_fEachFrames = false ;
}

// キーフレームから各フレームの値を計算する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorSequencer::UpdateAllFrameValues( void )
{
	const KeyFrameParam *	pkfp = m_arrKeyFrames.GetConstArray() ;
	const size_t	nLength = m_arrKeyFrames.GetLength() ;
	if ( nLength <= 1 )
	{
		m_fEachFrames = false ;
		return ;
	}
	NormalizeKeyFrameOrder() ;
	//
//	S3DDVector *	pKeyPos = m_arrValues.GetArray() ;
	if ( m_methodInterpolate == methodBezier )
	{
		ESLAssert( m_arrValues.GetLength() == nLength * 3 ) ;
	}
	else
	{
		ESLAssert( m_arrValues.GetLength() == nLength ) ;
	}
	//
	ESLAssert( pkfp[nLength - 1].iFrame >= pkfp[0].iFrame ) ;
	m_arrFrames.SetLength
		( pkfp[nLength - 1].iFrame - pkfp[0].iFrame + 1 ) ;
	//
	UpdateFrameValues( 0, nLength - 1 ) ;
	/*
	S3DSplineCurves	spline ;
	SArray<double>	aT ;
	double *		pT = aT.GetArray( nLength ) ;
	S3DDVector *	pFramePos = m_arrFrames.GetArray() ;
	size_t			iDstFrame = 0 ;
	size_t			iStartOfSpline = 1 ;
	size_t			iEndOfSpline = 0 ;
	for ( size_t i = 1; i < nLength; i ++ )
	{
		if ( (m_methodInterpolate == methodSpline) && (i > iEndOfSpline) )
		{
			size_t	nCtrlPoints = 0 ;
			while ( pkfp[i - 1].iFrame == pkfp[i].iFrame )
			{
				if ( ++ i >= nLength )
				{
					break ;
				}
			}
			iStartOfSpline = i ;
			pT[nCtrlPoints ++] = pkfp[i - 1].iFrame ;
			for ( size_t j = i; j < nLength; j ++ )
			{
				if ( pkfp[j].iFrame == pT[nCtrlPoints - 1] )
				{
					break ;
				}
				pT[nCtrlPoints ++] = pkfp[j].iFrame ;
				if ( pkfp[j].nFlags & keyframeCorner )
				{
					break ;
				}
			}
			spline.CreateSpline
				( m_arrValues.GetArray() + (i - 1), pT, nCtrlPoints ) ;
			iEndOfSpline = nCtrlPoints + i - 2 ;
		}
		KeyFrameParam *	pkfp0 = pkfp + (i - 1) ;
		KeyFrameParam *	pkfp1 = pkfp + i ;
		//
		SGLBezierCurves<double>	bzFrame ;
		bzFrame.SetLine
			( 0.0, 1.0, pkfp0->speedOut, pkfp1->speedIn ) ;
		//
		while ( (int32_t) iDstFrame + pkfp[0].iFrame <= pkfp1->iFrame )
		{
			double	t = 1.0 ;
			if ( pkfp1->iFrame > pkfp0->iFrame )
			{
				int	iFrame = (int) iDstFrame + pkfp[0].iFrame ;
				t = (double) (iFrame - pkfp0->iFrame)
									/ (pkfp1->iFrame - pkfp0->iFrame) ;
				t = bzFrame.PointAt( t ) ;
			}
			if ( m_methodInterpolate == methodSpline )
			{
				pFramePos[iDstFrame ++] =
					spline.InterpolateAt
						( i - iStartOfSpline,
							pkfp0->iFrame * (1.0 - t) + pkfp1->iFrame * t ) ;
			}
			else
			{
				S3DDVector	v =
						pKeyPos[i - 1] * (1.0 - t) + pKeyPos[i] * t ;
				pFramePos[iDstFrame ++] = v ;
			}
		}
	}
	ESLAssert( iDstFrame == m_arrFrames.GetLength() ) ;
	*/
	m_iFirstFrame = pkfp[0].iFrame ;
	m_fEachFrames = true ;
}

void S3DSceneComposer::VectorSequencer::UpdateFrameValues( size_t iFirstKey, size_t iEndKey )
{
	const KeyFrameParam *		pkfp = m_arrKeyFrames.GetConstArray() ;
	const S3DDVector *			pKeyPos = m_arrValues.GetConstArray() ;
	S3DSplineCurves				spline ;
	SGLBezierCurves<double>		bzFrame ;
	SGLBezierCurves<S3DDVector>	bzCurve ;
	SArray<double>	aT ;
	double *		pT = aT.GetArray( iEndKey - iFirstKey + 1 ) ;
	S3DDVector *	pFramePos = m_arrFrames.GetArray() ;
	size_t			iDstFrame = 0 ;
	size_t			iStartOfSpline = 1 ;
	size_t			iEndOfSpline = 0 ;
	for ( size_t i = iFirstKey + 1; i <= iEndKey; i ++ )
	{
		if ( (m_methodInterpolate == methodSpline) && (i > iEndOfSpline) )
		{
			size_t	nCtrlPoints = 0 ;
			while ( pkfp[i - 1].iFrame == pkfp[i].iFrame )
			{
				if ( ++ i >= iEndKey + 1 )
				{
					break ;
				}
			}
			iStartOfSpline = i ;
			pT[nCtrlPoints ++] = pkfp[i - 1].iFrame ;
			for ( size_t j = i; j <= iEndKey; j ++ )
			{
				if ( pkfp[j].iFrame == pT[nCtrlPoints - 1] )
				{
					break ;
				}
				pT[nCtrlPoints ++] = pkfp[j].iFrame ;
				if ( pkfp[j].nFlags & keyframeCorner )
				{
					break ;
				}
			}
			ESLAssert( nCtrlPoints <= aT.GetLength() ) ;
			spline.CreateSpline( pKeyPos + (i - 1), pT, nCtrlPoints ) ;
			iEndOfSpline = nCtrlPoints + i - 2 ;
		}
		const KeyFrameParam *	pkfp0 = pkfp + (i - 1) ;
		const KeyFrameParam *	pkfp1 = pkfp + i ;
		//
		bzFrame.SetLine
			( 0.0, 1.0, pkfp0->speedOut, pkfp1->speedIn ) ;
		//
		if ( m_methodInterpolate == methodBezier )
		{
			size_t	j = i * 3 ;
			bzCurve.SetAt( 0, pKeyPos[j - 3] ) ;
			bzCurve.SetAt( 1, pKeyPos[j - 2] ) ;
			bzCurve.SetAt( 2, pKeyPos[j - 1] ) ;
			bzCurve.SetAt( 3, pKeyPos[j] ) ;
		}
		//
		while ( (int32_t) iDstFrame + pkfp[0].iFrame <= pkfp1->iFrame )
		{
			double	t = 1.0 ;
			if ( pkfp1->iFrame > pkfp0->iFrame )
			{
				int	iFrame = (int) iDstFrame + pkfp[0].iFrame ;
				t = (double) (iFrame - pkfp0->iFrame)
									/ (pkfp1->iFrame - pkfp0->iFrame) ;
				t = bzFrame.PointAt( t ) ;
			}
			if ( m_methodInterpolate == methodSpline )
			{
				pFramePos[iDstFrame ++] =
					spline.InterpolateAt
						( i - iStartOfSpline,
							pkfp0->iFrame * (1.0 - t) + pkfp1->iFrame * t ) ;
			}
			else if ( m_methodInterpolate == methodBezier )
			{
				pFramePos[iDstFrame ++] = bzCurve.PointAt( t, 0 ) ;
			}
			else
			{
				S3DDVector	v =
						pKeyPos[i - 1] * (1.0 - t) + pKeyPos[i] * t ;
				pFramePos[iDstFrame ++] = v ;
			}
		}
	}
	aT.FinishArray() ;
	m_arrFrames.FinishArray() ;
	ESLAssert( iDstFrame <= m_arrFrames.GetLength() ) ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
}

// アニメーションフレーム解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::VectorSequencer::ParseFrameSequence
	( const SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	size_t	nDuration = (size_t) xmlSeq.GetAttrIntegerAs( L"duration", 0 ) ;
	m_iFirstFrame = (size_t) xmlSeq.GetAttrIntegerAs( L"first_frame", 0 ) ;
	if ( nDuration == 0 )
	{
		m_fEachFrames = false ;
		return	sglErrSuccess ;
	}
	m_arrFrames.SetLength( nDuration ) ;
	//
	S3DDVector *	pFrames = m_arrFrames.GetArray() ;
	SString *		pstrFrames = xmlSeq.GetTextElement() ;
	SStringParser	sparsFrames ;
	sparsFrames.AttachString( *pstrFrames ) ;
	//
	size_t	iDst = 0 ;
	SString	strFrame ;
	while ( (iDst < nDuration) && sparsFrames.PassSpace() )
	{
		sparsFrames.NextEnclosedString( strFrame, L'/' ) ;
		ParseVector( pFrames[iDst], strFrame ) ;
		iDst ++ ;
	}
	m_arrFrames.FinishArray() ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// アニメーションフレーム保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::VectorSequencer::FormatFrameSequence
	( SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	if ( !m_fEachFrames )
	{
		return	sglErrSuccess ;
	}
	xmlSeq.SetAttrIntegerAs( L"duration", m_arrFrames.GetLength() ) ;
	xmlSeq.SetAttrIntegerAs( L"first_frame", m_iFirstFrame ) ;
	//
	SString				strFrames ;
	SString				strValue ;
	const S3DDVector *	pvFrames = m_arrFrames.GetConstArray() ;
	for ( size_t i = 0; i < m_arrFrames.GetLength(); i ++ )
	{
		FormatVector( strValue, pvFrames[i] ) ;
		strFrames += strValue ;
		strFrames += L" / " ;
	}
	xmlSeq.SetTextElement( strFrames ) ;
	return	sglErrSuccess ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSceneComposer::VectorSequencer::GetFrameVector( double fpFrame )
{
	if ( !m_fEachFrames )
	{
		UpdateAllFrameValues() ;
		if ( !m_fEachFrames )
		{
			return	m_vDefault ;
		}
	}
	ESLAssert( m_arrFrames.GetLength() >= 1 ) ;
	int		iFrame0 = (int) floor( fpFrame ) ;
	int		iFrame1 = iFrame0 + 1 ;
	double	fpDecimal = fpFrame - iFrame0 ;
	iFrame0 = esl_clampi
				( iFrame0 - (int) m_iFirstFrame, 0,
						(int) m_arrFrames.GetLength() - 1 ) ;
	iFrame1 = esl_clampi
				( iFrame1 - (int) m_iFirstFrame, 0,
						(int) m_arrFrames.GetLength() - 1 ) ;
	S3DDVector	v0 = m_arrFrames.At( (size_t) iFrame0 ) ;
	S3DDVector	v1 = m_arrFrames.At( (size_t) iFrame1 ) ;
	return	v0 * (1.0 - fpDecimal) + v1 * fpDecimal ;
}

// 補完方法を変更
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorSequencer::ChangeInterpolateMethod
		( S3DSceneComposer::Sequencer::InterpolateMethod itpl )
{
	if ( m_methodInterpolate == itpl )
	{
		return ;
	}
	size_t	nKeyFrames = m_arrKeyFrames.GetLength() ;
	if ( m_methodInterpolate == methodBezier )
	{
		if ( (nKeyFrames > 0)
			&& (m_arrValues.GetLength() > (nKeyFrames - 1) * 3) )
		{
			S3DDVector *	pvValues = m_arrValues.GetArray() ;
			for ( size_t i = 0, j = 0; i < nKeyFrames; i ++, j += 3 )
			{
				pvValues[i] = pvValues[j] ;
			}
			m_arrValues.FinishArray() ;
			m_arrValues.SetLength( nKeyFrames ) ;
		}
	}
	else if ( itpl == methodBezier )
	{
		if ( m_arrValues.GetLength() >= nKeyFrames )
		{
			S3DDVector *	pvValues = m_arrValues.GetArray( nKeyFrames ) ;
			SGLBezierCurves<S3DDVector>	bzCurves ;
			if ( nKeyFrames >= 3 )
			{
				bzCurves.SetCurveUnsmoothSpeed
					( pvValues[0], pvValues[1], pvValues[2],
										1.0, 1.0, 1.0, 1.0 ) ;
				for ( size_t i = 3; i < nKeyFrames; i ++ )
				{
					bzCurves.AddCurveUnsmoothSpeed( pvValues[i], 1.0, 1.0 ) ;
				}
			}
			else if ( nKeyFrames == 2 )
			{
				bzCurves.SetLine( pvValues[0], pvValues[1], 1.0, 1.0 ) ;
			}
			else if ( nKeyFrames == 1 )
			{
				bzCurves.SetLine( pvValues[0], pvValues[0], 1.0, 1.0 ) ;
				bzCurves.SetLength( 3 ) ;
			}
			m_arrValues.FinishArray() ;
			//
			m_arrValues.SetLength( bzCurves.GetLength() ) ;
			eslCopyMemory
				( m_arrValues.GetArray(),
					bzCurves.GetConstArray(),
					bzCurves.GetLength() * sizeof(S3DDVector) ) ;
			m_arrValues.FinishArray() ;
			//
			if ( nKeyFrames > 0 )
			{
				m_arrValues.Add( pvValues[nKeyFrames - 1] ) ;
				m_arrValues.Add( pvValues[nKeyFrames - 1] ) ;
			}
		}
	}
	m_methodInterpolate = itpl ;
	m_fEachFrames = false ;
}

// フレームシーケンス作成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::VectorSequencer::CreateFrameSequence
							( size_t iFirstFrame, size_t nDuration )
{
	m_arrFrames.SetLength( nDuration ) ;
	m_iFirstFrame = iFirstFrame ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// フレーム値設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::VectorSequencer::SetFrameVector( size_t iFrame, const S3DDVector& vec )
{
	S3DDVector *	pVector = m_arrFrames.GetAt( iFrame - m_iFirstFrame ) ;
	if ( pVector == nullptr )
	{
		return	sglErrFailed ;
	}
	*pVector = vec ;
	return	sglErrSuccess ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::VectorSequencer::DuplicateSequencer( void )
{
	return	new VectorSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// スカラシーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ScalarSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ScalarSequencer::ScalarSequencer( void )
{
	m_fpDefault = 0.0 ;
	m_iFirstFrame = 0 ;
	m_fEachFrames = false ;
}

S3DSceneComposer::ScalarSequencer::ScalarSequencer( const ScalarSequencer& seq )
	: Sequencer( seq ),
		m_fpDefault( seq.m_fpDefault ),
		m_arrValues( seq.m_arrValues ),
		m_arrFrames( seq.m_arrFrames ),
		m_iFirstFrame( seq.m_iFirstFrame ),
		m_fEachFrames( seq.m_fEachFrames )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ScalarSequencer::~ScalarSequencer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::ScalarSequencer::GetScalarParameter( size_t i ) const
{
	double *	ps = m_arrValues.GetAt( i ) ;
	if ( ps != nullptr )
	{
		return	*ps ;
	}
	return	m_fpDefault ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ScalarSequencer::SetScalarParameter( size_t i, double s )
{
	double *	ps = m_arrValues.GetAt( i ) ;
	if ( ps != nullptr )
	{
		*ps = s ;
		m_fEachFrames = false ;
	}
	else
	{
		m_fpDefault = s ;
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ScalarSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.InsertAt( i, 0.0 ) ;
	m_fEachFrames = false ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ScalarSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.RemoveAt( i ) ;
	m_fEachFrames = false ;
}

// キーフレームから各フレームの値を計算する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ScalarSequencer::UpdateAllFrameValues( void )
{
	const KeyFrameParam *	pkfp = m_arrKeyFrames.GetConstArray() ;
	const size_t	nLength = m_arrKeyFrames.GetLength() ;
	if ( nLength <= 1 )
	{
		m_fEachFrames = false ;
		return ;
	}
	NormalizeKeyFrameOrder() ;
	//
	const double *	pKeyPos = m_arrValues.GetConstArray() ;
	ESLAssert( m_arrValues.GetLength() == nLength ) ;
	//
	ESLAssert( pkfp[nLength - 1].iFrame >= pkfp[0].iFrame ) ;
	m_arrFrames.SetLength
		( pkfp[nLength - 1].iFrame - pkfp[0].iFrame + 1 ) ;
	//
	SGLSplineCurves	spline ;
	SArray<double>	aT ;
	double *		pT = aT.GetArray( nLength ) ;
	double *		pFramePos = m_arrFrames.GetArray() ;
	size_t			iDstFrame = 0 ;
	size_t			iStartOfSpline = 1 ;
	size_t			iEndOfSpline = 0 ;
	for ( size_t i = 1; i < nLength; i ++ )
	{
		if ( (m_methodInterpolate == methodSpline) && (i > iEndOfSpline) )
		{
			size_t	nCtrlPoints = 0 ;
			while ( pkfp[i - 1].iFrame == pkfp[i].iFrame )
			{
				if ( ++ i >= nLength )
				{
					break ;
				}
			}
			iStartOfSpline = i ;
			pT[nCtrlPoints ++] = pkfp[i - 1].iFrame ;
			for ( size_t j = i; j < nLength; j ++ )
			{
				if ( pkfp[j].iFrame == pT[nCtrlPoints - 1] )
				{
					break ;
				}
				pT[nCtrlPoints ++] = pkfp[j].iFrame ;
				if ( pkfp[j].nFlags & keyframeCorner )
				{
					break ;
				}
			}
			spline.CreateSpline
				( m_arrValues.GetConstArray() + (i - 1), pT, nCtrlPoints ) ;
			iEndOfSpline = nCtrlPoints + i - 2 ;
		}
		const KeyFrameParam *	pkfp0 = pkfp + (i - 1) ;
		const KeyFrameParam *	pkfp1 = pkfp + i ;
		//
		SGLBezierCurves<double>	bzFrame ;
		bzFrame.SetLine
			( 0.0, 1.0, pkfp0->speedOut, pkfp1->speedIn ) ;
		//
		while ( (int32_t) iDstFrame + pkfp[0].iFrame <= pkfp1->iFrame )
		{
			double	t = 1.0 ;
			if ( pkfp1->iFrame > pkfp0->iFrame )
			{
				int	iFrame = (int) iDstFrame + pkfp[0].iFrame ;
				t = (double) (iFrame - pkfp0->iFrame)
								/ (pkfp1->iFrame - pkfp0->iFrame) ;
				t = bzFrame.PointAt( t ) ;
			}
			if ( m_methodInterpolate == methodSpline )
			{
				pFramePos[iDstFrame ++] =
					spline.InterpolateAt
						( i - iStartOfSpline,
							pkfp0->iFrame * (1.0 - t) + pkfp1->iFrame * t ) ;
			}
			else
			{
				pFramePos[iDstFrame ++] =
						pKeyPos[i - 1] * (1.0 - t) + pKeyPos[i] * t ;
			}
		}
	}
	aT.FinishArray() ;
	m_arrFrames.FinishArray() ;
	//
	m_iFirstFrame = pkfp[0].iFrame ;
	m_fEachFrames = true ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ScalarSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
}

// アニメーションフレーム解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ScalarSequencer::ParseFrameSequence
	( const SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	size_t	nDuration = (size_t) xmlSeq.GetAttrIntegerAs( L"duration", 0 ) ;
	m_iFirstFrame = (size_t) xmlSeq.GetAttrIntegerAs( L"first_frame", 0 ) ;
	if ( nDuration == 0 )
	{
		m_fEachFrames = false ;
		return	sglErrSuccess ;
	}
	m_arrFrames.SetLength( nDuration ) ;
	//
	double *		pFrames = m_arrFrames.GetArray() ;
	SString *		pstrFrames = xmlSeq.GetTextElement() ;
	SStringParser	sparsFrames ;
	sparsFrames.AttachString( *pstrFrames ) ;
	//
	size_t	iDst = 0 ;
	SString	strFrame ;
	while ( (iDst < nDuration) && sparsFrames.PassSpace() )
	{
		sparsFrames.NextEnclosedString( strFrame, L'/' ) ;
		ParseScalar( pFrames[iDst], strFrame ) ;
		iDst ++ ;
	}
	m_arrFrames.FinishArray() ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// アニメーションフレーム保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ScalarSequencer::FormatFrameSequence
	( SSystem::SXMLDocument& xmlSeq, S3DSceneComposer::ParameterType type )
{
	if ( !m_fEachFrames )
	{
		return	sglErrSuccess ;
	}
	xmlSeq.SetAttrIntegerAs( L"duration", m_arrFrames.GetLength() ) ;
	xmlSeq.SetAttrIntegerAs( L"first_frame", m_iFirstFrame ) ;
	//
	SString			strFrames ;
	SString			strValue ;
	const double *	pfpFrames = m_arrFrames.GetConstArray() ;
	for ( size_t i = 0; i < m_arrFrames.GetLength(); i ++ )
	{
		FormatScalar( strValue, pfpFrames[i] ) ;
		strFrames += strValue ;
		strFrames += L" / " ;
	}
	xmlSeq.SetTextElement( strFrames ) ;
	return	sglErrSuccess ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::ScalarSequencer::GetFrameScalar( double fpFrame )
{
	if ( !m_fEachFrames )
	{
		UpdateAllFrameValues() ;
		if ( !m_fEachFrames )
		{
			return	m_fpDefault ;
		}
	}
	ESLAssert( m_arrFrames.GetLength() >= 1 ) ;
	int		iFrame0 = (int) floor( fpFrame ) ;
	int		iFrame1 = iFrame0 + 1 ;
	double	fpDecimal = fpFrame - iFrame0 ;
	iFrame0 = esl_clampi
				( iFrame0 - (int) m_iFirstFrame, 0,
						(int) m_arrFrames.GetLength() - 1 ) ;
	iFrame1 = esl_clampi
				( iFrame1 - (int) m_iFirstFrame, 0,
						(int) m_arrFrames.GetLength() - 1 ) ;
	double	s0 = m_arrFrames.At( (size_t) iFrame0 ) ;
	double	s1 = m_arrFrames.At( (size_t) iFrame1 ) ;
	return	s0 * (1.0 - fpDecimal) + s1 * fpDecimal ;
}

// フレームシーケンス作成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ScalarSequencer::CreateFrameSequence
					( size_t iFirstFrame, size_t nDuration )
{
	m_arrFrames.SetLength( nDuration ) ;
	m_iFirstFrame = iFirstFrame ;
	m_fEachFrames = true ;
	return	sglErrSuccess ;
}

// フレーム値設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ScalarSequencer::SetFrameScalar( size_t iFrame, double s )
{
	double *	pScalar = m_arrFrames.GetAt( iFrame - m_iFirstFrame ) ;
	if ( pScalar == nullptr )
	{
		return	sglErrFailed ;
	}
	*pScalar = s ;
	return	sglErrSuccess ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::ScalarSequencer::DuplicateSequencer( void )
{
	return	new ScalarSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 整数シーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::IntegerSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::IntegerSequencer::IntegerSequencer( void )
{
	m_nDefault = 0 ;
}

S3DSceneComposer::IntegerSequencer::IntegerSequencer( const IntegerSequencer& seq )
	: Sequencer( seq ),
		m_nDefault( seq.m_nDefault ),
		m_arrValues( seq.m_arrValues )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::IntegerSequencer::~IntegerSequencer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DSceneComposer::IntegerSequencer::GetIntegerParameter( size_t i ) const
{
	int32_t *	pn = m_arrValues.GetAt( i ) ;
	if ( pn != nullptr )
	{
		return	*pn ;
	}
	return	m_nDefault ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::IntegerSequencer::SetIntegerParameter( size_t i, int32_t n )
{
	int32_t *	pn = m_arrValues.GetAt( i ) ;
	if ( pn != nullptr )
	{
		*pn = n ;
	}
	else
	{
		m_nDefault = n ;
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::IntegerSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.InsertAt( i, 0 ) ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::IntegerSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.RemoveAt( i ) ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::IntegerSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DSceneComposer::IntegerSequencer::GetFrameInteger( double fpFrame )
{
	double	t ;
	int		iFrame = (int) floor( fpFrame ) ;
	size_t	i = GetKeyFrameProgress( t, iFrame ) ;
	int32_t *	pn0 = m_arrValues.GetAt( i ) ;
	if ( pn0 != nullptr )
	{
		return	*pn0 ;
	}
	return	m_nDefault ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::IntegerSequencer::DuplicateSequencer( void )
{
	return	new IntegerSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ブーリアンシーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::BooleanSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::BooleanSequencer::BooleanSequencer( void )
{
	m_fDefault = false ;
}

S3DSceneComposer::BooleanSequencer::BooleanSequencer( const BooleanSequencer& seq )
	: Sequencer( seq ),
		m_fDefault( seq.m_fDefault ),
		m_arrValues( seq.m_arrValues )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::BooleanSequencer::~BooleanSequencer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::BooleanSequencer::GetBooleanParameter( size_t i ) const
{
	bool *	pb = m_arrValues.GetAt( i ) ;
	if ( pb != nullptr )
	{
		return	*pb ;
	}
	return	m_fDefault ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::BooleanSequencer::SetBooleanParameter( size_t i, bool b )
{
	bool *	pb = m_arrValues.GetAt( i ) ;
	if ( pb != nullptr )
	{
		*pb = b ;
	}
	else
	{
		m_fDefault = b ;
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::BooleanSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.InsertAt( i, false ) ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::BooleanSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.RemoveAt( i ) ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::BooleanSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::BooleanSequencer::GetFrameBoolean( double fpFrame )
{
	double	t ;
	int		iFrame = (int) floor( fpFrame ) ;
	size_t	i = GetKeyFrameProgress( t, iFrame ) ;
	bool *	pb0 = m_arrValues.GetAt( i ) ;
	if ( pb0 != nullptr )
	{
		return	*pb0 ;
	}
	return	m_fDefault ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::BooleanSequencer::DuplicateSequencer( void )
{
	return	new BooleanSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// N次元ベクタシーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::VectorXSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::VectorXSequencer::VectorXSequencer( size_t nDim )
{
	ESLAssert( nDim != 0 ) ;
	m_nDimension = nDim ;
	m_arrDefault.SetLength( nDim ) ;
}

S3DSceneComposer::VectorXSequencer::VectorXSequencer( const VectorXSequencer& seq )
	: Sequencer( seq ),
		m_arrDefault( seq.m_arrDefault ),
		m_arrValues( seq.m_arrValues ),
		m_nDimension( seq.m_nDimension )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::VectorXSequencer::~VectorXSequencer( void )
{
}

// 次元数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::VectorXSequencer::GetDimension( void ) const
{
	return	m_nDimension ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::VectorXSequencer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	if ( nBufBytes != m_nDimension * sizeof(double) )
	{
		return	0 ;
	}
	size_t	nKeyFrames = m_arrValues.GetLength() / m_nDimension ;
	if ( i < nKeyFrames )
	{
		eslCopyMemory
			( pDst, m_arrValues.GetAt( i * m_nDimension ), nBufBytes ) ;
	}
	else
	{
		ESLAssert( m_arrDefault.GetLength() == m_nDimension ) ;
		eslCopyMemory
			( pDst, m_arrDefault.GetAt(0), nBufBytes ) ;
	}
	return	nBufBytes ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::VectorXSequencer::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	if ( nBufBytes != m_nDimension * sizeof(double) )
	{
		return	0 ;
	}
	size_t	nKeyFrames = m_arrValues.GetLength() / m_nDimension ;
	if ( i < nKeyFrames )
	{
		eslCopyMemory
			( m_arrValues.GetAt( i * m_nDimension ), pSrc, nBufBytes ) ;
	}
	else
	{
		ESLAssert( m_arrDefault.GetLength() == m_nDimension ) ;
		eslCopyMemory
			( m_arrDefault.GetAt(0), pSrc, nBufBytes ) ;
	}
	return	nBufBytes ;
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorXSequencer::InsertKeyFrame
		( size_t i, const KeyFrameParam& kfp )
{
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.Insert( i * m_nDimension, m_nDimension ) ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorXSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.Remove( i * m_nDimension, m_nDimension ) ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::VectorXSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	for ( size_t i = 0; i < m_nDimension; i ++ )
	{
		m_arrValues.Swap( i1 * m_nDimension + i, i2 * m_nDimension + i ) ;
	}
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::VectorXSequencer::GetFrameBinary
	( void * pDst, size_t nBufBytes, double fpFrame ) const
{
	if ( nBufBytes != m_nDimension * sizeof(double) )
	{
		return	0 ;
	}
	double		t ;
	size_t		i = GetKeyFrameProgress
						( t, eslRoundR32ToInt( (float32_t) fpFrame ) ) ;
	double *	pv0 = m_arrValues.GetAt( i * m_nDimension ) ;
	double *	pv1 = m_arrValues.GetAt( (i + 1) * m_nDimension ) ;
	if ( (pv1 == nullptr) || (t == 0.0) )
	{
		if ( pv0 != nullptr )
		{
			eslCopyMemory( pDst, pv0, nBufBytes ) ;
			return	nBufBytes ;
		}
		ESLAssert( m_arrDefault.GetLength() == m_nDimension ) ;
		eslCopyMemory
			( pDst, m_arrDefault.GetAt(0), nBufBytes ) ;
		return	nBufBytes ;
	}
	ESLAssert( pv0 != nullptr ) ;
	size_t	nDim = m_nDimension ;
	for ( size_t i = 0; i < nDim; i ++ )
	{
		((double*)pDst)[i] = pv0[i] * (1.0 - t) + pv1[i] * t ;
	}
	return	nBufBytes ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::VectorXSequencer::DuplicateSequencer( void )
{
	return	new VectorXSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// コマンドシーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::CommandSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::CommandSequencer::CommandSequencer( void )
{
	m_iLastKeyFrame = -1 ;
}

S3DSceneComposer::CommandSequencer::CommandSequencer( const CommandSequencer& seq )
	: Sequencer( seq ),
		m_arrValues( seq.m_arrValues ),
		m_strDefault( seq.m_strDefault ),
		m_iLastKeyFrame( seq.m_iLastKeyFrame ),
		m_strLastFrameCmd( seq.m_strLastFrameCmd )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::CommandSequencer::~CommandSequencer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::CommandSequencer::GetCommandParameter( size_t i ) const
{
	SString *	pstr = m_arrValues.GetAt( i ) ;
	if ( pstr != nullptr )
	{
		return	*pstr ;
	}
	return	m_strDefault ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommandSequencer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	SString *	pstr = m_arrValues.GetAt( i ) ;
	if ( pstr != nullptr )
	{
		*pstr = pwszCmd ;
	}
	else
	{
		m_strDefault = pwszCmd ;
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommandSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.InsertAt( i, new SString() ) ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommandSequencer::RemoveKeyFrame( size_t i )
{
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.RemoveAt( i ) ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommandSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::CommandSequencer::GetFrameCommand
					( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	double	t ;
	int		iFrame = (int) floor( fpFrame ) ;
	size_t	iKeyFrame = GetKeyFrameProgress( t, iFrame ) ;
	//
	if ( m_methodInterpolate == methodSelector )
	{
		SString *	pstr = m_arrValues.GetAt( iKeyFrame ) ;
		if ( pstr != nullptr )
		{
			return	*pstr ;
		}
		return	m_strDefault ;
	}
	else
	{
		KeyFrameParam	kfp ;
		if ( GetKeyFrameParameter( iKeyFrame, kfp ) )
		{
			if ( iFrame < kfp.iFrame )
			{
				iKeyFrame -- ;
			}
		}
		if ( (seek == seekStream) || (seek == seekStreamPaused) )
		{
			m_strLastFrameCmd = L"" ;
			for ( ssize_t i = m_iLastKeyFrame + 1;
								i <= (ssize_t) iKeyFrame; i ++ )
			{
				SString *	pstr = m_arrValues.GetAt( (size_t) i ) ;
				if ( pstr != nullptr )
				{
					if ( !m_strLastFrameCmd.IsEmpty()
						&& (m_strLastFrameCmd.GetLastAt(0) != L'\n') )
					{
						m_strLastFrameCmd += '\n' ;
					}
					m_strLastFrameCmd += *pstr ;
				}
			}
			m_iLastKeyFrame = (ssize_t) iKeyFrame ;
			return	m_strLastFrameCmd ;
		}
		else
		{
			m_iLastKeyFrame = (ssize_t) iKeyFrame ;
			//
			KeyFrameParam	kfp ;
			if ( GetKeyFrameParameter( iKeyFrame, kfp )
				&& (kfp.iFrame == iFrame) )
			{
				SString *	pstr = m_arrValues.GetAt( iKeyFrame ) ;
				if ( pstr != nullptr )
				{
					return	*pstr ;
				}
			}
			return	nullptr ;
		}
	}
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::CommandSequencer::DuplicateSequencer( void )
{
	return	new CommandSequencer( *this ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ポーズシーケンサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer::PoseSequencer, Sequencer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::PoseSequencer::PoseSequencer( void )
{
	m_poseDefault.pPoseBase = nullptr ;
	m_poseDefault.pPoseTarget = nullptr ;
	m_poseDefault.fpTransition = 0.0 ;
}

S3DSceneComposer::PoseSequencer::PoseSequencer( const PoseSequencer& seq )
	: Sequencer( seq ),
		m_arrValues( seq.m_arrValues ),
		m_libPose( seq.m_libPose )
{
	m_poseDefault.pPoseBase = nullptr ;
	m_poseDefault.pPoseTarget = nullptr ;
	m_poseDefault.fpTransition = 0.0 ;
	//
	m_arrPoses.SetLength( m_arrValues.GetLength() ) ;
	for ( size_t i = 0; i < m_arrValues.GetLength(); i ++ )
	{
		SString *	pstrPose = m_arrValues.GetAt( i ) ;
		if ( pstrPose != nullptr )
		{
			ParsePoseString( m_arrPoses.At(i), *pstrPose ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::PoseSequencer::~PoseSequencer( void )
{
}

// 参照ライブラリを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::SetReferenceLibrary( S3DModelPoseLibrary * pLib )
{
	m_libPose.SetParentLibrary( pLib ) ;
}

// テンポラリポーズライブラリを取得
//////////////////////////////////////////////////////////////////////////////
const S3DModelPoseLibrary&
	S3DSceneComposer::PoseSequencer::GetTemporaryPose( void ) const
{
	return	m_libPose ;
}

// シーケンサー解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::PoseSequencer::ParseSequencer
	( const SSystem::SXMLDocument& xmlTag,
				S3DSceneComposer::ParameterType type )
{
	SXMLDocument *	pxmlLib = xmlTag.GetElementTagAs( L"pose_stock" ) ;
	if ( pxmlLib != nullptr )
	{
		m_libPose.ParseLibraryXML( *pxmlLib ) ;
	}
	SGLError	err = Sequencer::ParseSequencer( xmlTag, type ) ;
	CleanupTemporaryPose() ;
	return	err ;
}

// シーケンサー保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::PoseSequencer::FormatSequencer
	( SSystem::SXMLDocument& xmlTag,
			S3DSceneComposer::ParameterType type, uint32_t nFlags )
{
	Sequencer::FormatSequencer( xmlTag, type, nFlags ) ;
	//
	SXMLDocument *	pxmlLib = xmlTag.CreateElementTagAs( L"pose_stock" ) ;
	if ( pxmlLib != nullptr )
	{
		CleanupTemporaryPose() ;
		m_libPose.FormatLibraryXML( *pxmlLib ) ;
	}
	return	sglErrSuccess ;
}

// 使用されていないテンポラリポーズを削除する
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::CleanupTemporaryPose( void )
{
	for ( size_t iPose = 0; iPose < m_libPose.GetPoseCount(); iPose ++ )
	{
		S3DModelPose *	pPose = m_libPose.GetPoseAt( iPose ) ;
		if ( pPose == nullptr )
		{
			continue ;
		}
		bool	flagFoundPose = false ;
		for ( size_t i = 0; i < m_arrPoses.GetLength(); i ++ )
		{
			const PoseInstance&	pi = m_arrPoses.At(i) ;
			if ( (pi.pPoseBase == pPose)
				&& (pi.pPoseTarget == pPose) )
			{
				flagFoundPose = true ;
				break ;
			}
		}
		if ( !flagFoundPose )
		{
			const wchar_t *	pwszPoseID = m_libPose.GetPoseIdentityAt( iPose ) ;
			for ( size_t i = 0; i < m_arrValues.GetLength(); i ++ )
			{
				const SString *	pstrPose = m_arrValues.GetAt(i) ;
				if ( (pstrPose != nullptr)
					&& (*pstrPose == pwszPoseID) )
				{
					flagFoundPose = true ;
					break ;
				}
			}
		}
		if ( !flagFoundPose )
		{
			m_libPose.RemovePoseAt( iPose -- ) ;
		}
	}
}

// ポーズ文字列解釈 <pose-identity>[,<animation-time>]
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::ParsePoseInstance
	( S3DSceneComposer::PoseInstance& pose, const wchar_t * pwszPose ) const
{
	pose.pPoseBase = nullptr ;
	pose.pPoseTarget = nullptr ;
	pose.secBasePose = 0.0 ;
	pose.secTargetPose = 0.0 ;
	pose.fpTransition = 0.0 ;
	//
	if ( pwszPose != nullptr )
	{
		SStringParser	sparsParam = pwszPose ;
		wchar_t	wchClosed ;
		pose.pPoseBase =
			m_libPose.GetPoseAs
				( sparsParam.GetEnclosedString( L',', 0, &wchClosed ) ) ;
		if ( wchClosed == L',' )
		{
			pose.secBasePose = sparsParam.NextRealNumber() ;
		}
	}
}

SSystem::SString S3DSceneComposer::PoseSequencer::ParsePoseUsage
						( double& secPose, const wchar_t * pwszPose )
{
	SString	strPoseID ;
	secPose = 0.0 ;
	//
	if ( pwszPose != nullptr )
	{
		SStringParser	sparsParam = pwszPose ;
		wchar_t	wchClosed =
					sparsParam.NextEnclosedString( strPoseID, L',' ) ;
		if ( wchClosed == L',' )
		{
			secPose = sparsParam.NextRealNumber() ;
		}
	}
	return	strPoseID ;
}

// ポーズ解釈（完全版） <pose0-id>[,<time0>[,<pose1-id>,<time>,<trans>]]
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::ParsePoseString
	( S3DSceneComposer::PoseInstance& pose, const wchar_t * pwszPose ) const
{
	pose = PoseInstance() ;
	//
	SString	strPoseID ;
	do
	{
		SStringParser	sparsParam = pwszPose ;
		wchar_t	wchClosed =
					sparsParam.NextEnclosedString( strPoseID, L',' ) ;
		pose.pPoseBase = m_libPose.GetPoseAs( strPoseID ) ;
		if ( wchClosed != L',' )
		{
			break ;
		}
		pose.secBasePose = sparsParam.NextRealNumber() ;
		if ( sparsParam.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		wchClosed = sparsParam.NextEnclosedString( strPoseID, L',' ) ;
		pose.pPoseTarget = m_libPose.GetPoseAs( strPoseID ) ;
		if ( wchClosed != L',' )
		{
			break ;
		}
		pose.secTargetPose = sparsParam.NextRealNumber() ;
		if ( sparsParam.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		pose.fpTransition = sparsParam.NextRealNumber() ;
	}
	while ( false ) ;
}

// ポーズ文字列書式化
//////////////////////////////////////////////////////////////////////////////
SSystem::SString
	S3DSceneComposer::PoseSequencer::FormatPoseString
					( const S3DSceneComposer::PoseInstance& pose ) const
{
	SString	strPoseID ;
	if ( GetPoseIdentityOf( strPoseID, pose.pPoseBase ) )
	{
		SString	strTargetPoseID ;
		if ( (pose.pPoseTarget != nullptr)
			&& GetPoseIdentityOf( strTargetPoseID, pose.pPoseTarget ) )
		{
			SString	strPoseSpec ;
			strPoseSpec.Format
				( L"%s,%.3f,%s,%.3f,%.3f",
					(const wchar_t*) strPoseID, pose.secBasePose,
					(const wchar_t*) strTargetPoseID, pose.secTargetPose,
					pose.fpTransition ) ;
			return	strPoseSpec ;
		}
		else if ( pose.secBasePose != 0.0 )
		{
			SString	strPoseSpec ;
			strPoseSpec.Format
				( L"%s,%.3f", (const wchar_t*) strPoseID, pose.secBasePose ) ;
			return	strPoseSpec ;
		}
		else
		{
			return	strPoseID ;
		}
	}
	return	SString() ;
}

// ユニークな新しいポーズIDを生成する
//////////////////////////////////////////////////////////////////////////////
SSystem::SString S3DSceneComposer::PoseSequencer::NewUniquePoseID( void ) const
{
	SakuraCL::SCLRandomizer	randomizer ;
	randomizer.InitializeSeed() ;
	for ( ; ; )
	{
		SString	strPoseID ;
		strPoseID.Format( L"$<%08X>", randomizer.Randomize() ) ;
		if ( m_libPose.GetPoseAs( strPoseID ) == nullptr )
		{
			return	strPoseID ;
		}
	}
}

// テンポラリポーズ生成
//////////////////////////////////////////////////////////////////////////////
S3DModelPose * S3DSceneComposer::PoseSequencer::NewTemporaryPose( SSystem::SString& strID )
{
	strID = NewUniquePoseID() ;
	//
	S3DModelPose *	pPose = new S3DModelPose ;
	m_libPose.AddPoseAs( strID, pPose ) ;
	//
	return	pPose ;
}

// テンポラリポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose *
	S3DSceneComposer::PoseSequencer::GetTemporaryPoseIdentityAs( const wchar_t * pwszPoseID ) const
{
	return	m_libPose.GetPoseAs( pwszPoseID, true ) ;
}

// ポーズID取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::PoseSequencer::GetPoseIdentityOf
	( SSystem::SString& strPoseID, S3DModelPose * pPose ) const
{
	if ( GetTemporaryPoseIdentityOf( strPoseID, pPose ) )
	{
		return	true ;
	}
	const wchar_t *	pwszID = m_libPose.GetPoseIdentityOf( pPose ) ;
	if ( pwszID != nullptr )
	{
		strPoseID = pwszID ;
		return	true ;
	}
	return	false ;
}

// テンポラリポーズID取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::PoseSequencer::GetTemporaryPoseIdentityOf
	( SSystem::SString& strPoseID, S3DModelPose * pPose ) const
{
	const wchar_t *	pwszID = m_libPose.GetPoseIdentityOf( pPose, true ) ;
	if ( pwszID != nullptr )
	{
		strPoseID = pwszID ;
		return	true ;
	}
	return	false ;
}

// テンポラリポーズ判定
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::PoseSequencer::IsTemporaryPose( S3DModelPose * pPose ) const
{
	const wchar_t *	pwszID = m_libPose.GetPoseIdentityOf( pPose, true ) ;
	return	(pwszID != nullptr) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::PoseSequencer::GetCommandParameter( size_t i ) const
{
	SString *	pstr = m_arrValues.GetAt( i ) ;
	if ( pstr != nullptr )
	{
		return	*pstr ;
	}
	return	nullptr ;
}

S3DSceneComposer::PoseInstance
	S3DSceneComposer::PoseSequencer::GetPoseParameter( size_t i ) const
{
	PoseInstance *	pPose = m_arrPoses.GetAt( i ) ;
	if ( pPose != nullptr )
	{
		return	*pPose ;
	}
	return	m_poseDefault ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	SString *	pstr = m_arrValues.GetAt( i ) ;
	if ( pstr != nullptr )
	{
		if ( *pstr != pwszCmd )
		{
			// <pose-id> 形式の場合には単純に置き換える
			// <pose-id>,<time> 形式の場合には <pose-id> が一時ポーズでない場合には
			// 単純に置き換える
			// それ以外の場合には、新規一時ポーズを作成して置き換える
			//
			PoseInstance	pose ;
			ParsePoseString( pose, pwszCmd ) ;
			//
			if ( (pose.pPoseTarget == nullptr)
				&& ((pose.secBasePose == 0.0)
					&& !IsTemporaryPose(pose.pPoseBase)) )
			{
				double	secPose ;
				m_libPose.RemovePoseAs( ParsePoseUsage( secPose, *pstr ) ) ;
				//
				m_arrPoses.SetAt( i, pose ) ;
				//
				*pstr = pwszCmd ;
			}
			else
			{
				SetPoseParameter( i, pose ) ;
			}
		}
	}
	else
	{
		ParsePoseInstance( m_poseDefault, pwszCmd ) ;
	}
}

void S3DSceneComposer::PoseSequencer::SetPoseParameter
		( size_t i, const S3DSceneComposer::PoseInstance& pose )
{
	if ( i == (size_t) Sequencer::indexDefault )
	{
		m_poseDefault = pose ;
		return ;
	}
	if ( pose.pPoseBase != nullptr )
	{
		S3DModelPoseLibrary *	pRefLib = m_libPose.GetParentLibrary() ;
		SString	strPoseID ;
		if ( pRefLib != nullptr )
		{
			strPoseID = pRefLib->GetPoseIdentityOf( pose.pPoseBase ) ;
		}
		if ( !strPoseID.IsEmpty() && (pose.pPoseTarget == nullptr) )
		{
			// ポーズ参照
			// ※<pose-id> 又は <pose-id>,<time> 形式で一時ポーズでない場合に限り
			SString *	pstr = m_arrValues.GetAt( i ) ;
			if ( pstr != nullptr )
			{
				double	secPose ;
				m_libPose.RemovePoseAs( ParsePoseUsage( secPose, *pstr ) ) ;
			}
			if ( pose.secBasePose != 0.0 )
			{
				SString *	pstrPoseUsage = new SString ;
				pstrPoseUsage->Format
					( L"%s,%.3f", (const wchar_t*) strPoseID,
											pose.secBasePose ) ;
				m_arrValues.SetAt( i, pstrPoseUsage ) ;
			}
			else if ( pstr != nullptr )
			{
				*pstr = strPoseID ;
			}
			else
			{
				m_arrValues.SetAt( i, new SString( strPoseID ) ) ;
			}
			m_arrPoses.SetAt( i, pose ) ;
		}
		else
		{
			// ポーズ複製保持
			S3DModelPose *	pPose = nullptr ;
			SString *		pstr = m_arrValues.GetAt( i ) ;
			if ( pstr != nullptr )
			{
				// 一時ポーズの場合、取得
				double	secPose ;
				strPoseID = ParsePoseUsage( secPose, *pstr ) ;
				pPose = m_libPose.GetPoseAs( strPoseID, true ) ;
			}
			if ( pPose == nullptr )
			{
				// 一時ポーズでない場合、新規作成
				pPose = NewTemporaryPose( strPoseID ) ;
				//
				PoseInstance	poseTemp ;
				poseTemp.pPoseBase = pPose ;
				m_arrPoses.SetAt( i, poseTemp ) ;
			}
			//
			*pPose = *(pose.pPoseBase) ;
			//
			if ( pose.pPoseTarget != nullptr )
			{
				pPose->PrepareBlendPoseTarget( *(pose.pPoseTarget) ) ;
				pPose->AllocateStillPoseBuffer() ;
				pPose->BlendAnimationFrame
					( 0.0, *(pose.pPoseTarget),
						pose.secTargetPose, pose.fpTransition ) ;
			}
			if ( pstr != nullptr )
			{
				*pstr = strPoseID ;
			}
			else
			{
				m_arrValues.SetAt( i, new SString( strPoseID ) ) ;
			}
		}
	}
	else
	{
		SString *	pstr = m_arrValues.GetAt( i ) ;
		if ( pstr != nullptr )
		{
			*pstr = L"" ;
			m_arrPoses.SetAt( i, pose ) ;
		}
		else
		{
			m_poseDefault = pose ;
		}
	}
}

// キーフレーム挿入
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::InsertKeyFrame
		( size_t i, const S3DSceneComposer::KeyFrameParam& kfp )
{
	Sequencer::InsertKeyFrame( i, kfp ) ;
	m_arrValues.InsertAt( i, new SString() ) ;
	m_arrPoses.InsertAt( i, PoseInstance() ) ;
}

// キーフレーム削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::RemoveKeyFrame( size_t i )
{
	SString *	pstr = m_arrValues.GetAt( i ) ;
	if ( pstr != nullptr )
	{
		size_t	nRef = 0 ;
		for ( size_t j = 0; j < m_arrValues.GetLength(); j ++ )
		{
			if ( i != j )
			{
				SString *	pstrAnother = m_arrValues.GetAt( j ) ;
				if ( (pstrAnother != nullptr)
					&& (*pstrAnother == *pstr) )
				{
					nRef ++ ;
					break ;
				}
			}
		}
		if ( nRef == 0 )
		{
			m_libPose.RemovePoseAs( *pstr ) ;
		}
	}
	Sequencer::RemoveKeyFrame( i ) ;
	m_arrValues.RemoveAt( i ) ;
	m_arrPoses.RemoveAt( i ) ;
}

// キーフレームの順番を入れ替える
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::SwapKeyFrame( size_t i1, size_t i2 )
{
	Sequencer::SwapKeyFrame( i1, i2 ) ;
	m_arrValues.Swap( i1, i2 ) ;
	m_arrPoses.Swap( i1, i2 ) ;
}

// 補完パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::PoseInstance
	S3DSceneComposer::PoseSequencer::GetFramePoseInstance( double fpFrame )
{
	double	t ;
	int		iFrame = (int) floor( fpFrame ) ;
	size_t	iKeyFrame = GetKeyFrameProgress( t, iFrame ) ;
	//
	PoseInstance *	piBasePose = m_arrPoses.GetAt( iKeyFrame ) ;
	PoseInstance *	piTargetPose = m_arrPoses.GetAt( iKeyFrame + 1 ) ;
	KeyFrameParam *	pkfp0 = m_arrKeyFrames.GetAt( iKeyFrame ) ;
	KeyFrameParam *	pkfp1 = m_arrKeyFrames.GetAt( iKeyFrame + 1 ) ;
	//
	PoseInstance	pose ;
	if ( piBasePose && piBasePose->pPoseBase
		&& piTargetPose && piTargetPose->pPoseBase && pkfp0 && pkfp1 )
	{
		if ( piBasePose->pPoseBase == piTargetPose->pPoseBase )
		{
			pose.pPoseBase = piBasePose->pPoseBase ;
			pose.pPoseTarget = nullptr ;
			pose.secBasePose =
					(piTargetPose->secBasePose
						- piBasePose->secBasePose) * t
									+ piBasePose->secBasePose ;
			pose.fpTransition = 0.0 ;
		}
		else if ( t == 0.0 )
		{
			pose.pPoseBase = piBasePose->pPoseBase ;
			pose.pPoseTarget = nullptr ;
			pose.secBasePose = piBasePose->secBasePose ;
			pose.secTargetPose = 0.0 ;
			pose.fpTransition = 0.0 ;
		}
		else
		{
			pose.pPoseBase = piBasePose->pPoseBase ;
			pose.pPoseTarget = piTargetPose->pPoseBase ;
			pose.secBasePose = piBasePose->secBasePose ;
			pose.secTargetPose = piTargetPose->secBasePose ;
			pose.fpTransition = t ;
		}
	}
	else
	{
		pose.pPoseBase = (piBasePose ? piBasePose->pPoseBase : nullptr) ;
		pose.pPoseTarget = (piTargetPose ? piTargetPose->pPoseBase : nullptr) ;
		pose.secBasePose = (piBasePose ? piBasePose->secBasePose : 0.0) ;
		pose.secTargetPose = (piTargetPose ? piTargetPose->secBasePose : 0.0) ;
		pose.fpTransition = t ;
	}
	return	pose ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::PoseSequencer::DuplicateSequencer( void )
{
	return	new PoseSequencer( *this ) ;
}

// 作成時の処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::PoseSequencer::OnSequencerAttached
					( S3DSceneComposer::ParameterProperty * pItem )
{
	S3DModelPoseLibrary *	pPoseLib = pItem->GetPoseLibraryChain() ;
	if ( pPoseLib != nullptr )
	{
		m_libPose.SetParentLibrary( pPoseLib ) ;
	}
}

// リソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::PoseSequencer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
			ItemSerializer * pItem, uint32_t nFlags )
{
	if ( nFlags & updateRefResource )
	{
		m_libPose.ResetAllPoseTarget( false ) ;
		//
		S3DModelPoseLibrary *	pPoseLib = pItem->GetPoseLibraryChain() ;
		for ( size_t i = 0; i < m_arrPoses.GetLength(); i ++ )
		{
			PoseInstance *	pPose = m_arrPoses.GetAt( i ) ;
			ESLAssert( pPose != nullptr ) ;
			if ( pPose == nullptr )
			{
				continue ;
			}
			if ( (pPose->pPoseBase != nullptr)
				&& (m_libPose.FindPosePtr( pPose->pPoseBase ) < 0) )
			{
				SString *	pstrPose = m_arrValues.GetAt( i ) ;
				if ( pstrPose != nullptr )
				{
					ParsePoseInstance( *pPose, *pstrPose ) ;
				}
			}
		}
	}
	return	0 ;
}



//////////////////////////////////////////////////////////////////////////////
// パラメータ・プロパティ情報
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ParameterProperty, Parameter )

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ParameterProperty::GetParameterCategoryName( size_t iCategory ) const
{
	return	nullptr ;
}

// ポーズライブラリ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary * S3DSceneComposer::ParameterProperty::GetPoseLibraryChain( void )
{
	return	nullptr ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::ParameterProperty::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	ParameterType	type = GetParameterType( i ) ;
	switch ( type )
	{
	case	typeMatrix:
	case	typeRotation:
		if ( nBufBytes == sizeof(S3DDMatrix) )
		{
			*((S3DDMatrix*)pDst) = GetMatrixParameter( i ) ;
			return	nBufBytes ;
		}
		break ;
	case	typePosition:
	case	typeDirection:
	case	typeZoom:
	case	typeColor:
		if ( nBufBytes == sizeof(S3DDVector) )
		{
			*((S3DDVector*)pDst) = GetVectorParameter( i ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeScalar:
		if ( nBufBytes == sizeof(double) )
		{
			*((double*)pDst) = GetScalarParameter( i ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeInteger:
		if ( nBufBytes == sizeof(int32_t) )
		{
			*((int32_t*)pDst) = GetIntegerParameter( i ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeBoolean:
		if ( nBufBytes == sizeof(bool) )
		{
			*((bool*)pDst) = GetBooleanParameter( i ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeSelector:
	case	typeCommand:
		{
			const wchar_t *	pwszCmd = GetCommandParameter( i ) ;
			size_t	nStrLen = SString::GetLength( pwszCmd ) ;
			size_t	nBufLen = nBufBytes / sizeof(wchar_t) ;
			size_t	nCopyLen = (size_t) esl_min( (int) nStrLen, (int) nBufLen ) ;
			eslCopyMemory( pDst, pwszCmd, nCopyLen * sizeof(wchar_t) ) ;
			if ( nBufLen > 0 )
			{
				size_t	j = esl_min( (int) nCopyLen, (int) nBufLen - 1 ) ;
				((wchar_t*)pDst)[j] = 0 ;
				return	(j + 1) * sizeof(wchar_t) ;
			}
		}
		break ;
	case	typePose:
		if ( nBufBytes == sizeof(PoseInstance) )
		{
			*((PoseInstance*)pDst) = GetPoseParameter( i ) ;
			return	nBufBytes ;
		}
		break ;
	default:
		break ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::ParameterProperty::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	ParameterType	type = GetParameterType( i ) ;
	switch ( type )
	{
	case	typeMatrix:
	case	typeRotation:
		if ( nBufBytes == sizeof(S3DDMatrix) )
		{
			SetMatrixParameter( i, *((const S3DDMatrix*)pSrc) ) ;
			return	nBufBytes ;
		}
		break ;
	case	typePosition:
	case	typeDirection:
	case	typeZoom:
	case	typeColor:
		if ( nBufBytes == sizeof(S3DDVector) )
		{
			SetVectorParameter( i, *((const S3DDVector*)pSrc) ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeScalar:
		if ( nBufBytes == sizeof(double) )
		{
			SetScalarParameter( i, *((const double*)pSrc) ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeInteger:
		if ( nBufBytes == sizeof(int32_t) )
		{
			SetIntegerParameter( i, *((const int32_t*)pSrc) ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeBoolean:
		if ( nBufBytes == sizeof(bool) )
		{
			SetBooleanParameter( i, *((const bool*)pSrc) ) ;
			return	nBufBytes ;
		}
		break ;
	case	typeSelector:
	case	typeCommand:
		{
			const wchar_t *	pwszCmd = (const wchar_t*) pSrc ;
			size_t	nBufLen = nBufBytes / sizeof(wchar_t) ;
			for ( size_t j = 0; j < nBufLen; j ++ )
			{
				if ( pwszCmd[j] == 0 )
				{
					SetCommandParameter( i, pwszCmd ) ;
					return	(j + 1) * sizeof(wchar_t) ;
				}
			}
		}
		break ;
	case	typePose:
		if ( nBufBytes == sizeof(PoseInstance) )
		{
			SetPoseParameter( i, *((const PoseInstance*)pSrc) ) ;
			return	nBufBytes ;
		}
		break ;
	default:
		break ;
	}
	return	0 ;
}

// バリアントとして取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Variant
	S3DSceneComposer::ParameterProperty::GetVariant( size_t iParam ) const
{
	ParameterType	type = GetParameterType( iParam ) ;
	switch ( type )
	{
	case	typeMatrix:
	case	typeRotation:
		return	Variant( GetMatrixParameter( iParam ), type ) ;
		break ;

	case	typePosition:
	case	typeDirection:
	case	typeZoom:
	case	typeColor:
		return	Variant( GetVectorParameter( iParam ), type ) ;

	case	typeScalar:
		return	Variant( GetScalarParameter( iParam ), type ) ;

	case	typeInteger:
		return	Variant( GetIntegerParameter( iParam ), type ) ;

	case	typeBoolean:
		return	Variant( GetBooleanParameter( iParam ), type ) ;

	case	typeSelector:
	case	typeCommand:
		return	Variant( GetCommandParameter( iParam ), type ) ;

	case	typePose:
		return	Variant( GetPoseParameter( iParam ) ) ;

	case	typeBinary:
	case	typeMatrix4:
	case	typeVector4:
	case	typeVector2:
		{
			Variant	varTemp ;
			size_t	nBufBytes = GetBinaryParameter( nullptr, 0, iParam ) ;
			GetBinaryParameter( varTemp.PutBuffer( nBufBytes, type ), nBufBytes, iParam ) ;
			return	varTemp ;
		}
		break ;

	default:
		break ;
	}
	return	Variant() ;
}

// シーケンスパラメータをプロパティに設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ParameterProperty::SetFrameSequenceParameter
	( S3DSceneComposer::ParameterType type, size_t i,
		S3DSceneComposer::Sequencer * pSeq,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	switch ( type )
	{
	case	typeMatrix:
	case	typeRotation:
		SetMatrixParameter( i, pSeq->GetFrameMatrix( fpFrame ) ) ;
		break ;
	case	typePosition:
	case	typeDirection:
	case	typeZoom:
	case	typeColor:
		SetVectorParameter( i, pSeq->GetFrameVector( fpFrame ) ) ;
		break ;
	case	typeScalar:
		SetScalarParameter( i, pSeq->GetFrameScalar( fpFrame ) ) ;
		break ;
	case	typeInteger:
		SetIntegerParameter( i, pSeq->GetFrameInteger( fpFrame ) ) ;
		break ;
	case	typeBoolean:
		SetBooleanParameter( i, pSeq->GetFrameBoolean( fpFrame ) ) ;
		break ;
	case	typeSelector:
	case	typeCommand:
		{
			const wchar_t *	pwszCmd =
						pSeq->GetFrameCommand( fpFrame, seek ) ;
			SetCommandParameter( i, pwszCmd ) ;
		}
		break ;
	case	typePose:
		{
			PoseInstance	pose =
				pSeq->GetFramePoseInstance( fpFrame ) ;
			SetPoseParameter( i, pose ) ;
		}
		break ;
	case	typeMatrix4:
	case	typeVector4:
	case	typeVector2:
		{
			double	buf[16] ;
			int		nDim = 0 ;
			switch ( type )
			{
			case	typeMatrix4:
				nDim = 16 ;
				break ;
			case	typeVector4:
				nDim = 4 ;
				break ;
			case	typeVector2:
				nDim = 2 ;
				break ;
			default:
				break ;
			}
			if ( pSeq->GetFrameBinary
				( buf, sizeof(double) * nDim, fpFrame )
									== sizeof(double) * nDim )
			{
				SetBinaryParameter( i, buf, sizeof(double) * nDim ) ;
			}
		}
		break ;
	default:
		break ;
	}
}

// ポーズ文字列解釈
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ParameterProperty::ParsePoseString
	( PoseInstance& pose, const wchar_t * pwszPose ) const
{
	pose.pPoseBase = nullptr ;
	pose.pPoseTarget = nullptr ;
	pose.secBasePose = 0.0 ;
	pose.secTargetPose = 0.0 ;
	pose.fpTransition = 0.0 ;
	//
	if ( (pwszPose != nullptr) && (pwszPose[0] != 0) )
	do
	{
		SStringParser	sparsParam = pwszPose ;
		wchar_t	wchClosed ;
		SString	strPose ;
		wchClosed = sparsParam.NextEnclosedString( strPose, L',', 0 ) ;
		pose.pPoseBase = GetPoseIdentityAs( strPose ) ;
		if ( wchClosed != L',' )
		{
			break ;
		}
		pose.secBasePose = sparsParam.NextRealNumber() ;
		//
		if ( sparsParam.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		wchClosed = sparsParam.NextEnclosedString( strPose, L',', 0 ) ;
		pose.pPoseTarget = GetPoseIdentityAs( strPose ) ;
		if ( wchClosed != L',' )
		{
			break ;
		}
		pose.secTargetPose = sparsParam.NextRealNumber() ;
		//
		if ( sparsParam.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		pose.fpTransition = sparsParam.NextRealNumber() ;
	}
	while ( false ) ;
}

// ポーズ文字列書式化
//////////////////////////////////////////////////////////////////////////////
SSystem::SString
		S3DSceneComposer::ParameterProperty::FormatPoseString( const PoseInstance& pose ) const
{
	SString	strPoseID ;
	if ( GetPoseIdentityOf( strPoseID, pose.pPoseBase ) )
	{
		SString	strTargetPoseID ;
		if ( (pose.pPoseTarget != nullptr)
			&& GetPoseIdentityOf( strTargetPoseID, pose.pPoseTarget ) )
		{
			SString	strPoseSpec ;
			strPoseSpec.Format
				( L"%s,%.3f,%s,%.3f,%.3f",
					(const wchar_t*) strPoseID, pose.secBasePose,
					(const wchar_t*) strTargetPoseID, pose.secTargetPose,
					pose.fpTransition ) ;
			return	strPoseSpec ;
		}
		else if ( pose.secBasePose != 0.0 )
		{
			SString	strPoseSpec ;
			strPoseSpec.Format
				( L"%s,%.3f", (const wchar_t*) strPoseID, pose.secBasePose ) ;
			return	strPoseSpec ;
		}
		else
		{
			return	strPoseID ;
		}
	}
	return	SString() ;
}

// ポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose *
	S3DSceneComposer::ParameterProperty::GetPoseIdentityAs( const wchar_t * pwszPoseID ) const
{
	return	nullptr ;
}

// ポーズID取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ParameterProperty::GetPoseIdentityOf
	( SSystem::SString& strPoseID, S3DModelPose * pPose ) const
{
	return	false ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ParameterProperty::ParseParameterProperties
	( const SSystem::SXMLDocument& xmlParam )
{
	for ( size_t i = 0; i < xmlParam.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlParam = xmlParam.GetElementAt( i ) ;
		if ( pxmlParam == nullptr )
		{
			continue ;
		}
		ssize_t	iParam = FindParameterID( pxmlParam->GetTag() ) ;
		if ( iParam < 0 )
		{
			continue ;
		}
		const SString *	pstrValue = pxmlParam->GetAttributeAs( L"value" ) ;
		if ( pstrValue != nullptr )
		{
			S3DSceneComposer::ParameterType	type = GetParameterType( iParam ) ;
			SetParameter( type, (size_t) iParam, *pstrValue ) ;
		}
	}
	return	sglErrSuccess ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ParameterProperty::FormatParameterProperties
	( SSystem::SXMLDocument& xmlParam )
{
	const size_t	nParamCount = GetParameterCount() ;
	for ( size_t iParam = 0; iParam < nParamCount; iParam ++ )
	{
		if ( GetParameterAttributes( iParam ) & attrNoSerializeFlags )
		{
			continue ;
		}
		SXMLDocument *	pxmlParam = new SXMLDocument ;
		pxmlParam->SetTag( GetParameterID( iParam ) ) ;
		xmlParam.AddElement( pxmlParam ) ;
		//
		SString	strValue ;
		GetParameter( strValue, GetParameterType( iParam ), iParam ) ;
		pxmlParam->SetAttributeAs( L"value", strValue ) ;
	}
	return	sglErrSuccess ;
}

// パラメータの一時保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ParameterProperty::SaveParameterEntryAt
		( S3DSceneComposer::ParameterEntryStorage& storage, size_t i )
{
	ESLAssert( i < GetParameterCount() ) ;
	GetParameter( storage.m_strValue, GetParameterType( i ), i ) ;
	Sequencer *	pSeq = GetParameterSequencer( i ) ;
	if ( pSeq != nullptr )
	{
		storage.m_pSequencer = pSeq->DuplicateSequencer() ;
	}
	return	sglErrSuccess ;
}

// 一時保存パラメータの復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ParameterProperty::RestoreParameterEntryAt
	( size_t i, ParameterEntryStorage& storage )
{
	ESLAssert( i < GetParameterCount() ) ;
	SetParameter( GetParameterType( i ), i, storage.m_strValue ) ;
	//
	Sequencer *	pSeq = GetParameterSequencer( i ) ;
	if ( storage.m_pSequencer != nullptr )
	{
		SetParameterSequencer( i, storage.m_pSequencer.Detach() ) ;
	}
	return	sglErrSuccess ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ParameterProperty::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneProperty" ;
}



//////////////////////////////////////////////////////////////////////////////
// アイテム・コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::Controller, ParameterProperty )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Controller::Controller( const wchar_t * pwszClassID )
	: m_pwszClassID( pwszClassID ),
		m_pOwnerItem( nullptr ),
		m_flagsBehavior( 0 ), m_flagsEventClasses( 0 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Controller::~Controller( void )
{
}

// パラメータ数宣言
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::PrepareParameterEntryCount( size_t nCount )
{
	m_arrParamClass.SetLimit( nCount ) ;
}

// パラメータ追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::Controller::AddParameterEntry
		( const S3DSceneComposer::ParamEntry& paramEntry )
{
	size_t	iParam = m_arrParamClass.Add( paramEntry ) ;
	return	iParam ;
}

size_t S3DSceneComposer::Controller::AddParameterEntry
	( const wchar_t * id,
		S3DSceneComposer::ParameterType type,
		uint32_t attr, const wchar_t * name, const wchar_t * desc,
		double minRange, double maxRange )
{
	S3DSceneComposer::ParamEntry	entry ;
	entry.id = id ;
	entry.type = type ;
	entry.attr = attr ;
	entry.name = name ;
	entry.desc = desc ;
	entry.minRange = minRange ;
	entry.maxRange = maxRange ;
	return	m_arrParamClass.Add( entry ) ;
}

// パラメータエントリ編集
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ParamEntry&
	S3DSceneComposer::Controller::ParameterEntryAt( size_t iParam )
{
	return	m_arrParamClass.At( iParam ) ;
}

// パラメータ削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::RemoveAllParameterEntries( void )
{
	m_arrParamClass.RemoveAll() ;
	m_arrUnknownProp.RemoveAll() ;
}

void S3DSceneComposer::Controller::ChopParameterEntryLastAt( size_t iParam )
{
	if ( iParam < m_arrParamClass.GetLength() )
	{
		m_arrParamClass.SetLength( iParam + 1 ) ;
	}
	if ( iParam < m_arrSequencers.GetLength() )
	{
		m_arrSequencers.SetLength( iParam + 1 ) ;
	}
}

// 所有アイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::Controller::GetOwnerItem( void ) const
{
	return	m_pOwnerItem ;
}

// コンポジション取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
	S3DSceneComposer::Controller::GetComposition( void ) const
{
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetComposition() ;
	}
	return	nullptr ;
}

// コンポーザー取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer * S3DSceneComposer::Controller::GetComposer( void ) const
{
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetComposer() ;
	}
	return	nullptr ;
}

// コンポーザー・マネージャー取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionManager * S3DSceneComposer::Controller::GetManager( void ) const
{
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetManager() ;
	}
	return	nullptr ;
}

// エディター取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionEditorInterface *
	S3DSceneComposer::Controller::GetEditor( void ) const
{
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetEditor() ;
	}
	return	nullptr ;
}

// アイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::Controller::GetSceneItemAs( const wchar_t * pwszID ) const
{
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	pComp->GetSceneItemAs( pwszID ) ;
	}
	return	nullptr ;
}

S3DSceneComposer::SpaceSerializer *
	S3DSceneComposer::Controller::GetSceneSpaceAs( const wchar_t * pwszID ) const
{
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	ESLTypeCast<SpaceSerializer>
					( pComp->GetSceneItemAs( pwszID ) ) ;
	}
	return	nullptr ;
}

S3DScene::Space *
	S3DSceneComposer::Controller::GetReferenceSpaceAs( const wchar_t * pwszPath ) const
{
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetReferenceSpaceAs( pwszPath ) ;
	}
	return	nullptr ;
}

// オーナーアイテムのプライマリモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface *
	S3DSceneComposer::Controller::GetOwnerItemPrimaryModel( void )
{
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetItemPrimaryModel() ;
	}
	return	nullptr ;
}

// 読み込み処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Controller::ParseController
	( S3DSceneComposer& composer,
		Composition& composition,
		const SSystem::SXMLDocument& xmlTag )
{
	m_strControlID = xmlTag.GetAttrStringAs( L"id" ) ;
	m_arrUnknownProp.RemoveAll() ;
	//
	SGLError	errResult = sglErrSuccess ;
	for ( size_t i = 0; i < xmlTag.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlParam = xmlTag.GetElementAt( i ) ;
		if ( pxmlParam == nullptr )
		{
			continue ;
		}
		//
		// パラメータ
		//
		ssize_t	iParam = FindParameterID( pxmlParam->GetTag() ) ;
		if ( iParam < 0 )
		{
			m_arrUnknownProp.Add( new SXMLDocument( *pxmlParam ) ) ;
			continue ;
		}
		if ( (GetParameterAttributes( iParam ) & attrNoSerializeFlags)
			|| !IsParameterValidation( iParam ) )
		{
			continue ;
		}
		SXMLDocument *	pxmlSeq = pxmlParam->GetElementTagAs( L"sequencer" ) ;
		if ( pxmlSeq != nullptr )
		{
			Sequencer *	pSeq = CreateParameterSequencer( iParam ) ;
			if ( pSeq == nullptr )
			{
				continue ;
			}
			ParameterType	type = GetParameterType( iParam ) ;
			//
			const SString *	pstrValue = pxmlParam->GetAttributeAs( L"value" ) ;
			if ( pstrValue != nullptr )
			{
				pSeq->SetParameter
					( type, (size_t) Sequencer::indexDefault, *pstrValue ) ;
				SetParameter( type, (size_t) iParam, *pstrValue ) ;
			}
			SGLError	err = pSeq->ParseSequencer( *pxmlSeq, type ) ;
			if ( err && !errResult )
			{
				errResult = err ;
			}
		}
		else
		{
			const SString *	pstrValue = pxmlParam->GetAttributeAs( L"value" ) ;
			if ( pstrValue != nullptr )
			{
				ParameterType	type = GetParameterType( iParam ) ;
				SetParameter( type, (size_t) iParam, *pstrValue ) ;
			}
		}
	}
	return	errResult ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::Controller::FormatController
	( S3DSceneComposer& composer,
		SSystem::SXMLDocument& xmlTag, uint32_t nFlags )
{
	xmlTag.SetTag( GetControllerClass() ) ;
	xmlTag.SetAttributeAs( L"id", m_strControlID ) ;
	//
	const size_t		nParamCount = m_arrParamClass.GetLength() ;
	const ParamEntry *	pParamEntries = m_arrParamClass.GetConstArray() ;
	for ( size_t iParam = 0; iParam < nParamCount; iParam ++ )
	{
		const ParamEntry&	pe = pParamEntries[iParam] ;
		if ( (pe.attr & attrNoSerializeFlags)
			|| !IsParameterValidation( iParam ) )
		{
			continue ;
		}
		SXMLDocument *	pxmlParam = new SXMLDocument ;
		pxmlParam->SetTag( pe.id ) ;
		xmlTag.AddElement( pxmlParam ) ;
		//
		Sequencer *	pSeq = m_arrSequencers.GetAt( iParam ) ;
		if ( pSeq != nullptr )
		{
			SString	strValue ;
			pSeq->GetParameter
				( strValue, pe.type, (size_t) Sequencer::indexDefault ) ;
			pxmlParam->SetAttributeAs( L"value", strValue ) ;
			//
			SXMLDocument *	pxmlSeq = new SXMLDocument ;
			pxmlSeq->SetTag( L"sequencer" ) ;
			pxmlParam->AddElement( pxmlSeq ) ;
			//
			pSeq->FormatSequencer( *pxmlSeq, pe.type, nFlags ) ;
		}
		else
		{
			SString	strValue ;
			GetParameter( strValue, pe.type, iParam ) ;
			pxmlParam->SetAttributeAs( L"value", strValue ) ;
		}
	}
	for ( size_t i = 0; i < m_arrUnknownProp.GetLength(); i ++ )
	{
		SXMLDocument *	pxmlProp = m_arrUnknownProp.GetAt( i ) ;
		ESLAssert( pxmlProp != nullptr ) ;
		if ( pxmlProp != nullptr )
		{
			xmlTag.AddElement( new SXMLDocument( *pxmlProp ) ) ;
		}
	}
	return	sglErrSuccess ;
}

// フレームを適用
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::SetFrameParameters( double fpFrame, SeekMethod seek )
{
	const ParamEntry *	pEntries = m_arrParamClass.GetConstArray() ;
	Sequencer *const*	ppSeq = m_arrSequencers.GetConstArray() ;
	const size_t		nCount = m_arrSequencers.GetLength() ;
	ESLAssert( nCount <= m_arrParamClass.GetLength() ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Sequencer *	pSeq = ppSeq[i] ;
		if ( pSeq == nullptr )
		{
			continue ;
		}
		if ( ((seek == seekStream) || ((seek == seekStreamPaused)))
			&& (pSeq->GetKeyFrameCount() == 0) )
		{
			continue ;
		}
		switch ( pEntries[i].type )
		{
		case	typeMatrix:
		case	typeRotation:
			{
				S3DDMatrix	mat = pSeq->GetFrameMatrix( fpFrame ) ;
				SetMatrixParameter( i, mat ) ;
			}
			break ;
		case	typePosition:
		case	typeDirection:
		case	typeZoom:
		case	typeColor:
			{
				S3DDVector	vec = pSeq->GetFrameVector( fpFrame ) ;
				SetVectorParameter( i, vec ) ;
			}
			break ;
		case	typeScalar:
			SetScalarParameter( i, pSeq->GetFrameScalar( fpFrame ) ) ;
			break ;
		case	typeInteger:
			SetIntegerParameter( i, pSeq->GetFrameInteger( fpFrame ) ) ;
			break ;
		case	typeBoolean:
			SetBooleanParameter( i, pSeq->GetFrameBoolean( fpFrame ) ) ;
			break ;
		case	typeSelector:
		case	typeCommand:
			SetCommandParameter( i, pSeq->GetFrameCommand( fpFrame, seek ) ) ;
			break ;
		case	typePose:
			SetPoseParameter( i, pSeq->GetFramePoseInstance( fpFrame ) ) ;
			break ;
		case	typeMatrix4:
		case	typeVector4:
		case	typeVector2:
			{
				double	buf[16] ;
				int		nDim = 0 ;
				switch ( pEntries[i].type )
				{
				case	typeMatrix4:
					nDim = 16 ;
					break ;
				case	typeVector4:
					nDim = 4 ;
					break ;
				case	typeVector2:
					nDim = 2 ;
					break ;
				default:
					break ;
				}
				if ( pSeq->GetFrameBinary
					( buf, sizeof(double) * nDim, fpFrame )
										== sizeof(double) * nDim )
				{
					SetBinaryParameter( i, buf, sizeof(double) * nDim ) ;
				}
			}
			break ;
		default:
			break ;
		}
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::Controller::GetMatrixParameter( size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetMatrixParameter( Sequencer::indexDefault ) ;
	}
	return	S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
}

S3DDVector S3DSceneComposer::Controller::GetVectorParameter( size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetVectorParameter( Sequencer::indexDefault ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DSceneComposer::Controller::GetScalarParameter( size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetScalarParameter( Sequencer::indexDefault ) ;
	}
	return	0.0 ;
}

int32_t S3DSceneComposer::Controller::GetIntegerParameter( size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetIntegerParameter( Sequencer::indexDefault ) ;
	}
	return	0 ;
}

bool S3DSceneComposer::Controller::GetBooleanParameter( size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetBooleanParameter( Sequencer::indexDefault ) ;
	}
	return	false ;
}

const wchar_t * S3DSceneComposer::Controller::GetCommandParameter( size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetCommandParameter( Sequencer::indexDefault ) ;
	}
	return	nullptr ;
}

S3DSceneComposer::PoseInstance
		S3DSceneComposer::Controller::GetPoseParameter( size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetPoseParameter( Sequencer::indexDefault ) ;
	}
	return	PoseInstance() ;
}

size_t S3DSceneComposer::Controller::GetBinaryParameter
		( void * pDst, size_t nBufBytes, size_t i ) const
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->GetBinaryParameter
					( pDst, nBufBytes, Sequencer::indexDefault ) ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		pSeq->SetMatrixParameter( Sequencer::indexDefault, mat ) ;
	}
}

void S3DSceneComposer::Controller::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		pSeq->SetVectorParameter( Sequencer::indexDefault, vec ) ;
	}
}

void S3DSceneComposer::Controller::SetScalarParameter( size_t i, double s )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		pSeq->SetScalarParameter( Sequencer::indexDefault, s ) ;
	}
}

void S3DSceneComposer::Controller::SetIntegerParameter( size_t i, int32_t n )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		pSeq->SetIntegerParameter( Sequencer::indexDefault, n ) ;
	}
}

void S3DSceneComposer::Controller::SetBooleanParameter( size_t i, bool b )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		pSeq->SetBooleanParameter( Sequencer::indexDefault, b ) ;
	}
}

void S3DSceneComposer::Controller::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		pSeq->SetCommandParameter( Sequencer::indexDefault, pwszCmd ) ;
	}
}

void S3DSceneComposer::Controller::SetPoseParameter
		( size_t i, const S3DSceneComposer::PoseInstance& pose )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		pSeq->SetPoseParameter( Sequencer::indexDefault, pose ) ;
	}
}

size_t S3DSceneComposer::Controller::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq != nullptr )
	{
		return	pSeq->SetBinaryParameter
					( Sequencer::indexDefault, pSrc, nBufBytes ) ;
	}
	return	0 ;
}

// コントローラークラスID取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Controller::GetControllerClass( void ) const
{
	return	m_pwszClassID ;
}

// アイテムID取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Controller::GetItemIdentity( void ) const
{
	return	m_strControlID ;
}

// アイテムID設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::SetItemIdentity( const wchar_t * pwszID )
{
	m_strControlID = pwszID ;
}

// パラメータ総数
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::Controller::GetParameterCount( void ) const
{
	return	m_arrParamClass.GetLength() ;
}

// パラメータ識別名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Controller::GetParameterID( size_t i ) const
{
	ParamEntry *	pe = m_arrParamClass.GetAt( i ) ;
	if ( pe != nullptr )
	{
		return	pe->id ;
	}
	return	nullptr ;
}

// パラメータ表示名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Controller::GetParameterFriendlyName( size_t i ) const
{
	ParamEntry *	pe = m_arrParamClass.GetAt( i ) ;
	if ( pe != nullptr )
	{
		return	pe->name ;
	}
	return	nullptr ;
}

// パラメータ説明
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Controller::GetParameterDescription( size_t i ) const
{
	ParamEntry *	pe = m_arrParamClass.GetAt( i ) ;
	if ( pe != nullptr )
	{
		return	pe->desc ;
	}
	return	nullptr ;
}

// パラメータ指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSceneComposer::Controller::FindParameterID( const wchar_t * pwszID ) const
{
	const size_t		nCount = m_arrParamClass.GetLength() ;
	const ParamEntry *	pEntries = m_arrParamClass.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( SString::Compare( pEntries[i].id, pwszID ) == 0 )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// パラメータ型
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ParameterType
	S3DSceneComposer::Controller::GetParameterType( size_t i ) const
{
	ParamEntry *	pe = m_arrParamClass.GetAt( i ) ;
	if ( pe != nullptr )
	{
		return	pe->type ;
	}
	return	typeInvalid ;
}

// パラメータ属性
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::Controller::GetParameterAttributes( size_t i ) const
{
	ParamEntry *	pe = m_arrParamClass.GetAt( i ) ;
	if ( pe != nullptr )
	{
		return	pe->attr ;
	}
	return	0 ;
}

// パラメータ有効範囲
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Controller::GetParameterScalarRange
	( size_t i, double& fpMin, double& fpMax ) const
{
	ParamEntry *	pe = m_arrParamClass.GetAt( i ) ;
	if ( pe != nullptr )
	{
		fpMin = pe->minRange ;
		fpMax = pe->maxRange ;
		return	(pe->attr & attrUIScalarSlider) != 0 ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Controller::IsParameterValidation( size_t i ) const
{
	return	true ;
}

// パラメータ・シーケンサ取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer * S3DSceneComposer::Controller::GetParameterSequencer( size_t i ) const
{
	return	m_arrSequencers.GetAt( i ) ;
}

S3DSceneComposer::Sequencer * S3DSceneComposer::Controller::CreateParameterSequencer( size_t i )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq == nullptr )
	{
		ParamEntry *	pe = m_arrParamClass.GetAt( i ) ;
		if ( pe == nullptr )
		{
			return	nullptr ;
		}
		pSeq = NewParameterSequencer( pe->type ) ;
		m_arrSequencers.SetAt( i, pSeq ) ;
		pSeq->OnSequencerAttached( this ) ;
	}
	return	pSeq ;
}

S3DSceneComposer::Sequencer * S3DSceneComposer::Controller::NewParameterSequencer( ParameterType type ) const
{
	return	Sequencer::NewParameterSequencer( type ) ;
}

// パラメータ・シーケンサ削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::RemoveParameterSequencer( size_t i )
{
	if ( GetParameterType( i ) != typeInvalid )
	{
		m_arrSequencers.SetAt( i, nullptr ) ;
	}
}

// パラメータ・シーケンサ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::SetParameterSequencer( size_t i, Sequencer * pSeq )
{
	ESLAssert( pSeq != nullptr ) ;
	if ( Sequencer::VerifyParameterSequencer( GetParameterType( i ), pSeq ) )
	{
		m_arrSequencers.SetAt( i, pSeq ) ;
		pSeq->OnSequencerAttached( this ) ;
	}
	else
	{
		delete	pSeq ;
	}
}

// ポーズライブラリ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary * S3DSceneComposer::Controller::GetPoseLibraryChain( void )
{
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetPoseLibraryChain() ;
	}
	return	nullptr ;
}

// ポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose *
	S3DSceneComposer::Controller::GetPoseIdentityAs( const wchar_t * pwszPoseID ) const
{
	for ( size_t i = 0; i < m_arrSequencers.GetLength(); i ++ )
	{
		PoseSequencer *	pPoseSeq =
			ESLTypeCast<PoseSequencer>( m_arrSequencers.GetAt( i ) ) ;
		if ( pPoseSeq != nullptr )
		{
			S3DModelPose *	pPose =
				pPoseSeq->GetTemporaryPoseIdentityAs( pwszPoseID ) ;
			if ( pPose != nullptr )
			{
				return	pPose ;
			}
		}
	}
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetPoseIdentityAs( pwszPoseID ) ;
	}
	return	nullptr ;
}

// ポーズID取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::Controller::GetPoseIdentityOf
	( SSystem::SString& strPoseID, S3DModelPose * pPose ) const
{
	if ( pPose == nullptr )
	{
		return	false ;
	}
	for ( size_t i = 0; i < m_arrSequencers.GetLength(); i ++ )
	{
		PoseSequencer *	pPoseSeq =
			ESLTypeCast<PoseSequencer>( m_arrSequencers.GetAt( i ) ) ;
		if ( pPoseSeq != nullptr )
		{
			if ( pPoseSeq->GetTemporaryPoseIdentityOf( strPoseID, pPose ) )
			{
				return	true ;
			}
		}
	}
	if ( m_pOwnerItem != nullptr )
	{
		return	m_pOwnerItem->GetPoseIdentityOf( strPoseID, pPose ) ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::Controller::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
			S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t			nResFlags = 0 ;
	Sequencer *const*	ppSeq = m_arrSequencers.GetConstArray() ;
	const size_t		nCount = m_arrSequencers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Sequencer *	pSeq = ppSeq[i] ;
		if ( pSeq != nullptr )
		{
			nResFlags |=
				pSeq->UpdatePropertyReference( comp, pItem, nFlags ) ;
		}
	}
	return	nResFlags ;
}

// レンダリング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnSetupSceneSettings
	( S3DScene& scene, S3DSceneComposer::ItemSerializer * pItem,
		const SGLSize& sizeFrame, SGLSecondaryViewProducer * psvp )
{
}

// レンダリング後始末
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnShoutdownSceneSettings
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem,
		SGLSecondaryViewProducer * psvp )
{
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
		double fpFrame, S3DSceneComposer::SeekMethod seek )
{
}

// 動作フラグ（無効化フラグ）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::EnableController( bool flagEnable )
{
	if ( flagEnable )
	{
		m_flagsBehavior &= ~behaviorDisabled ;
	}
	else
	{
		m_flagsBehavior |= behaviorDisabled ;
	}
}

// 規定のレンダリングデバイス設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnSetRenderDevice( S3DRenderDevice * pDevice )
{
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnPrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
}

// レンダリングイベント
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem,
			S3DSceneComposer::ItemSerializer * pItem )
{
}

// 当たり判定追加
//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::RenderCollision
	( const S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, S3DCollision& render )
{
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::RenderModel
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::BeforeRenderModel
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		ItemSerializer * pItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::AfterRenderModel
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		ItemSerializer * pItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
}

// アイテムに追加された
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::OnAddController
			( S3DSceneComposer::ItemSerializer * pItem )
{
}

// アイテムから分離される
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Controller::BeforeDetachController
			( S3DSceneComposer::ItemSerializer * pItem )
{
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::Controller::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneController" ;
}



//////////////////////////////////////////////////////////////////////////////
// アイテム・シリアライザ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ItemSerializer, ParameterProperty )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer::ItemSerializer
		( const wchar_t * pwszClassID,
			const S3DSceneComposer::ParamSetClass * pClass )
	: m_pwszClassID( pwszClassID ), m_ppsClass( pClass )
{
}

// 読み込み処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ItemSerializer::ParseItem
	( S3DSceneComposer& composer,
		S3DSceneComposer::Composition& composition,
		S3DScene::Space& space,
		const SSystem::SXMLDocument& xmlTag )
{
	m_strRefStyle = xmlTag.GetAttrStringAs( L"ref_style" ) ;
	if ( !m_strRefStyle.IsEmpty() )
	{
		SXMLDocument *
			pxmlStyle = composer.GetPartsStyleAs( m_strRefStyle ) ;
		if ( pxmlStyle == nullptr )
		{
			composer.OutputError
				( SString(L"定義されないスタイル参照：") + m_strRefStyle ) ;
			return	sglErrFailed ;
		}
		SGLError	err =
			ParseStyle
				( composer, composition, space, *pxmlStyle ) ;
		if ( err )
		{
			return	err ;
		}
		return	ParseStyle( composer, composition, space, xmlTag ) ;
	}
	else
	{
		return	ParseStyle( composer, composition, space, xmlTag ) ;
	}
}

SGLError S3DSceneComposer::ItemSerializer::ParseStyle
	( S3DSceneComposer& composer,
		S3DSceneComposer::Composition& composition,
		S3DScene::Space& space,
		const SSystem::SXMLDocument& xmlTag )
{
	m_arrUnknownProp.RemoveAll() ;
	//
	SGLError	errResult = sglErrSuccess ;
	for ( size_t i = 0; i < xmlTag.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlParam = xmlTag.GetElementAt( i ) ;
		if ( pxmlParam == nullptr )
		{
			continue ;
		}
		if ( pxmlParam->GetTag() == L"controllers" )
		{
			SGLError	err =
				ParseControllersTag
					( composer, composition, space, *pxmlParam ) ;
			if ( err )
			{
				errResult = err ;
			}
			continue ;
		}
		//
		// パラメータ
		//
		ssize_t	iParam = FindParameterID( pxmlParam->GetTag() ) ;
		if ( iParam < 0 )
		{
			SGLError	err =
				ParseNonParameterTag
					( composer, composition, space, *pxmlParam ) ;
			if ( err )
			{
				errResult = err ;
			}
			continue ;
		}
		const uint32_t	attrFlags = GetParameterAttributes( iParam ) ;
		if ( (attrFlags & attrNoSerializeFlags)
			|| !IsParameterValidation( iParam ) )
		{
			continue ;
		}
		ParameterType	type = GetParameterType( iParam ) ;
		const SString *	pstrValue = pxmlParam->GetAttributeAs( L"value" ) ;
		if ( pstrValue != nullptr )
		{
			SetParameter( type, iParam, *pstrValue ) ;
		}
		//
		// シーケンサ
		//
		SXMLDocument *	pxmlSeq = pxmlParam->GetElementTagAs( L"sequencer" ) ;
		if ( pxmlSeq == nullptr )
		{
			continue ;
		}
		Sequencer *	pSeq = CreateParameterSequencer( iParam ) ;
		if ( pSeq == nullptr )
		{
			continue ;
		}
		pSeq->ParseSequencer( *pxmlSeq, type ) ;
	}
	return	errResult ;
}

SGLError S3DSceneComposer::ItemSerializer::ParseNonParameterTag
	( S3DSceneComposer& composer,
		S3DSceneComposer::Composition& composition,
		S3DScene::Space& space,
		const SSystem::SXMLDocument& xmlTag )
{
	m_arrUnknownProp.Add( new SXMLDocument( xmlTag ) ) ;
	return	sglErrSuccess ;
}

SGLError S3DSceneComposer::ItemSerializer::ParseControllersTag
	( S3DSceneComposer& composer,
		Composition& composition,
		S3DScene::Space& space,
		const SSystem::SXMLDocument& xmlTag )
{
	SGLError	errResult = sglErrSuccess ;
	for ( size_t i = 0; i < xmlTag.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlCtrl = xmlTag.GetElementAt( i ) ;
		if ( pxmlCtrl == nullptr )
		{
			continue ;
		}
		Controller *
			pController =
				composer.CreateController( pxmlCtrl->GetTag() ) ;
		if ( pController == nullptr )
		{
			continue ;
		}
		size_t	iController = AddController( pController ) ;
		//
		SGLError	err =
			pController->ParseController
				( composer, composition, *pxmlCtrl ) ;
		if ( err )
		{
			RemoveControllerAt( iController ) ;
			errResult = err ;
			continue ;
		}
	}
	return	sglErrSuccess ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ItemSerializer::FormatItem
	( S3DSceneComposer& composer,
			SSystem::SXMLDocument& xmlTag, uint32_t nFlags )
{
	size_t	nParamCount = GetParameterCount() ;
	for ( size_t i = 0; i < nParamCount; i ++ )
	{
		//
		// パラメータ
		//
		if ( (GetParameterAttributes( i ) & attrNoSerializeFlags)
			|| !IsParameterValidation( i ) )
		{
			continue ;
		}
		const wchar_t *	pwszID = GetParameterID( i ) ;
		ParameterType	type = GetParameterType( i ) ;
		ESLAssert( pwszID && pwszID[0] ) ;
		//
		SXMLDocument *	pxmlParam = new SXMLDocument ;
		pxmlParam->SetTag( pwszID ) ;
		xmlTag.AddElement( pxmlParam ) ;
		//
		SString	strValue ;
		GetParameter( strValue, type, i ) ;
		pxmlParam->SetAttributeAs( L"value", strValue ) ;
		//
		// シーケンサ
		//
		Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
		if ( pSeq == nullptr )
		{
			continue ;
		}
		SXMLDocument *	pxmlSeq = new SXMLDocument ;
		pxmlSeq->SetTag( L"sequencer" ) ;
		pxmlParam->AddElement( pxmlSeq ) ;
		//
		pSeq->FormatSequencer( *pxmlSeq, type, nFlags ) ;
	}
	SXMLDocument *	pxmlCtrlTag =
						xmlTag.CreateElementTagAs( L"controllers" ) ;
	size_t	nCtrlCount = GetControllerCount() ;
	for ( size_t i = 0; i < nCtrlCount; i ++ )
	{
		//
		// コントローラー
		//
		Controller *	pController = GetControllerAt( i ) ;
		if ( pController == nullptr )
		{
			continue ;
		}
		const wchar_t *	pwszType = pController->GetControllerClass() ;
		ESLAssert( pwszType && pwszType[0] ) ;
		//
		SXMLDocument *	pxmlCtrl = new SXMLDocument ;
		pxmlCtrl->SetTag( pwszType ) ;
		pxmlCtrlTag->AddElement( pxmlCtrl ) ;
		//
		pController->FormatController( composer, *pxmlCtrl, nFlags ) ;
	}
	for ( size_t i = 0; i < m_arrUnknownProp.GetLength(); i ++ )
	{
		SXMLDocument *	pxmlProp = m_arrUnknownProp.GetAt( i ) ;
		ESLAssert( pxmlProp != nullptr ) ;
		if ( pxmlProp != nullptr )
		{
			xmlTag.AddElement( new SXMLDocument( *pxmlProp ) ) ;
		}
	}
	xmlTag.SetAttributeAs( L"id", m_strItemID ) ;
	xmlTag.SetTag( m_pwszClassID ) ;
	return	sglErrSuccess ;
}

// フレームの初期化処理（デフォルトは0フレームへのシーク）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::InitializeFrameParameters( void )
{
	SetFrameParameters( 0.0, seekJumpReset ) ;
}

// フレームを適用
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::SetFrameParameters
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	size_t	nCount = GetParameterCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
		if ( pSeq == nullptr )
		{
			continue ;
		}
		ParameterType	type = GetParameterType( i ) ;
		SetFrameSequenceParameter( type, i, pSeq, fpFrame, seek ) ;
	}
	CallControllerSetFrameParameters( fpFrame, seek ) ;
}

// スタイル参照アイテムか？
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemSerializer::GetStyleReference( void ) const
{
	if ( !m_strRefStyle.IsEmpty() )
	{
		return	m_strRefStyle ;
	}
	return	nullptr ;
}

// アイテムクラス取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemSerializer::GetItemClass( void ) const
{
	return	m_pwszClassID ;
}

// アイテムID取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemSerializer::GetItemIdentity( void ) const
{
	return	m_strItemID ;
}

// アイテムID設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::SetItemIdentity( const wchar_t * pwszID )
{
	m_strItemID = pwszID ;
}

// パラメータエントリ取得
//////////////////////////////////////////////////////////////////////////////
const S3DSceneComposer::ParamEntry *
	S3DSceneComposer::ItemSerializer::GetParameterEntryAt( size_t i ) const
{
	return	GetParameterEntryAt( m_ppsClass, i ) ;
}

const S3DSceneComposer::ParamEntry *
	S3DSceneComposer::ItemSerializer::GetParameterEntryAt
			( const S3DSceneComposer::ParamSetClass * ppsClass, size_t i )
{
	if ( ppsClass == nullptr )
	{
		return	nullptr ;
	}
	size_t	nParentCount = GetParameterCount( ppsClass->pParent ) ;
	if ( i < nParentCount )
	{
		return	GetParameterEntryAt( ppsClass->pParent, i ) ;
	}
	else if ( i - nParentCount < ppsClass->nCount )
	{
		return	&(ppsClass->pEntries[i - nParentCount]) ;
	}
	return	nullptr ;
}

// パラメータ総数
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::ItemSerializer::GetParameterCount( void ) const
{
	return	GetParameterCount( m_ppsClass ) ;
}

size_t S3DSceneComposer::ItemSerializer::GetParameterCount
	( const S3DSceneComposer::ParamSetClass * ppsClass )
{
	size_t	nCount = 0 ;
	while ( ppsClass != nullptr )
	{
		nCount += ppsClass->nCount ;
		ppsClass = ppsClass->pParent ;
	}
	return	nCount ;
}

// パラメータ識別名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemSerializer::GetParameterID( size_t i ) const
{
	const ParamEntry *	pe = GetParameterEntryAt( i ) ;
	return	pe ? pe->id : nullptr ;
}

// パラメータ表示名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemSerializer::GetParameterFriendlyName( size_t i ) const
{
	const ParamEntry *	pe = GetParameterEntryAt( i ) ;
	return	pe ? pe->name : nullptr ;
}

// パラメータ説明
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemSerializer::GetParameterDescription( size_t i ) const
{
	const ParamEntry *	pe = GetParameterEntryAt( i ) ;
	return	pe ? pe->desc : nullptr ;
}

// パラメータ指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSceneComposer::ItemSerializer::FindParameterID( const wchar_t * pwszID ) const
{
	return	FindParameterID( m_ppsClass, pwszID ) ;
}

ssize_t S3DSceneComposer::ItemSerializer::FindParameterID
	( const S3DSceneComposer::ParamSetClass * ppsClass, const wchar_t * pwszID )
{
	if ( ppsClass == nullptr )
	{
		return	-1 ;
	}
	for ( size_t i = 0; i < ppsClass->nCount; i ++ )
	{
		if ( SString::Compare( ppsClass->pEntries[i].id, pwszID ) == 0 )
		{
			return	(ssize_t) (GetParameterCount(ppsClass->pParent) + i) ;
		}
	}
	return	FindParameterID( ppsClass->pParent, pwszID ) ;
}

// パラメータ型
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ParameterType
	S3DSceneComposer::ItemSerializer::GetParameterType( size_t i ) const
{
	const ParamEntry *	pe = GetParameterEntryAt( i ) ;
	return	pe ? pe->type : typeInvalid ;
}

// パラメータ属性
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::ItemSerializer::GetParameterAttributes( size_t i ) const
{
	const ParamEntry *	pe = GetParameterEntryAt( i ) ;
	return	pe ? pe->attr : 0 ;
}

// パラメータ有効範囲
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ItemSerializer::GetParameterScalarRange
	( size_t i, double& fpMin, double& fpMax ) const
{
	const ParamEntry *	pe = GetParameterEntryAt( i ) ;
	if ( pe != nullptr )
	{
		fpMin = pe->minRange ;
		fpMax = pe->maxRange ;
		return	(pe->attr & attrUIScalarSlider) != 0 ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ItemSerializer::IsParameterValidation( size_t i ) const
{
	return	true ;
}

// パラメータ・シーケンサ取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Sequencer *
	S3DSceneComposer::ItemSerializer::GetParameterSequencer( size_t i ) const
{
	return	m_arrSequencers.GetAt( i ) ;
}

S3DSceneComposer::Sequencer *
	S3DSceneComposer::ItemSerializer::CreateParameterSequencer( size_t i )
{
	Sequencer *	pSeq = m_arrSequencers.GetAt( i ) ;
	if ( pSeq == nullptr )
	{
		pSeq = NewParameterSequencer( GetParameterType( i ) ) ;
		if ( pSeq != nullptr )
		{
			m_arrSequencers.SetAt( i, pSeq ) ;
			pSeq->OnSequencerAttached( this ) ;
		}
	}
	return	pSeq ;
}

S3DSceneComposer::Sequencer *
	S3DSceneComposer::ItemSerializer::NewParameterSequencer
					( S3DSceneComposer::ParameterType type ) const
{
	return	Sequencer::NewParameterSequencer( type ) ;
}

// パラメータ・シーケンサ削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::RemoveParameterSequencer( size_t i )
{
	if ( GetParameterType( i ) != typeInvalid )
	{
		m_arrSequencers.SetAt( i, nullptr ) ;
	}
}

// パラメータ・シーケンサ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::SetParameterSequencer( size_t i, Sequencer * pSeq )
{
	ESLAssert( pSeq != nullptr ) ;
	if ( Sequencer::VerifyParameterSequencer( GetParameterType( i ), pSeq ) )
	{
		m_arrSequencers.SetAt( i, pSeq ) ;
		pSeq->OnSequencerAttached( this ) ;
	}
	else
	{
		delete	pSeq ;
	}
}

// ポーズライブラリ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary * S3DSceneComposer::ItemSerializer::GetPoseLibraryChain( void )
{
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pCompoer = pComp->GetSceneComposer() ;
		if ( pCompoer != nullptr )
		{
			return	&(pCompoer->Assets().PoseLibrary()) ;
		}
	}
	return	nullptr ;
}

// コントローラー数
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::ItemSerializer::GetControllerCount( void ) const
{
	return	m_arrControllers.GetLength() ;
}

// コントローラー指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSceneComposer::ItemSerializer::FindControllerID( const wchar_t * pwszID ) const
{
	Controller *const*	ppControlers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControlers[i] ;
		if ( (pController != nullptr)
			&& (SString::Compare( pController->GetItemIdentity(), pwszID ) == 0) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

ssize_t S3DSceneComposer::ItemSerializer::FindController
			( S3DSceneComposer::Controller * pContoroller ) const
{
	return	m_arrControllers.FindPtr( pContoroller ) ;
}

ssize_t S3DSceneComposer::ItemSerializer::FindControllerClassOf
	( const ESLRuntimeClass& rtClass, size_t iFirst ) const
{
	Controller *const*	ppControlers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = iFirst; i < nCount; i ++ )
	{
		Controller *	pController = ppControlers[i] ;
		if ( pController->IsKindOf( rtClass ) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// コントローラー取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Controller *
	S3DSceneComposer::ItemSerializer::GetControllerAt( size_t i ) const
{
	return	m_arrControllers.GetAt( i ) ;
}

S3DSceneComposer::Controller *
	S3DSceneComposer::ItemSerializer::GetControllerAs( const wchar_t * pwszID ) const
{
	Controller *const*	ppControlers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControlers[i] ;
		if ( (pController != nullptr)
			&& (SString::Compare( pController->GetItemIdentity(), pwszID ) == 0) )
		{
			return	pController ;
		}
	}
	return	nullptr ;
}

// コントローラー追加
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::ItemSerializer::AddController( S3DSceneComposer::Controller * pContoroller )
{
	SSmartLock<SCriticalSection>	lock( &m_csCtrlSync ) ;
	pContoroller->m_pOwnerItem = this ;
	size_t	iCtrl = m_arrControllers.Add( pContoroller ) ;
	OnAddController( pContoroller ) ;
	m_flagsCtrlBehavior
			|= pContoroller->GetControllerBehaviorFlags() ;
	m_flagsCtrlEventClasses
			|= pContoroller->GetBehaviorRenderEventClasses() ;
	return	iCtrl ;
}

size_t S3DSceneComposer::ItemSerializer::InsertController( size_t i, Controller * pContoroller )
{
	SSmartLock<SCriticalSection>	lock( &m_csCtrlSync ) ;
	pContoroller->m_pOwnerItem = this ;
	if ( i >= m_arrControllers.GetLength() )
	{
		i = m_arrControllers.GetLength() ;
	}
	m_arrControllers.InsertAt( i, pContoroller ) ;
	OnAddController( pContoroller ) ;
	m_flagsCtrlBehavior
			|= pContoroller->GetControllerBehaviorFlags() ;
	m_flagsCtrlEventClasses
			|= pContoroller->GetBehaviorRenderEventClasses() ;
	return	i ;
}

// コントローラー削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ItemSerializer::RemoveControllerAt( size_t i )
{
	SSmartLock<SCriticalSection>	lock( &m_csCtrlSync ) ;
	if ( i >= m_arrControllers.GetLength() )
	{
		return	sglErrFailed ;
	}
	Controller *	pController = m_arrControllers.GetAt( i ) ;
	if ( pController != nullptr )
	{
		BeforeDetachController( pController ) ;
	}
	m_arrControllers.RemoveAt( i ) ;
	return	sglErrSuccess ;
}

SGLError S3DSceneComposer::ItemSerializer::RemoveControllerAs( const wchar_t * pwszID )
{
	SSmartLock<SCriticalSection>	lock( &m_csCtrlSync ) ;
	return	RemoveControllerAt( (size_t) FindControllerID( pwszID ) ) ;
}

S3DSceneComposer::Controller *
	S3DSceneComposer::ItemSerializer::DetachControllerAt( size_t i )
{
	SSmartLock<SCriticalSection>	lock( &m_csCtrlSync ) ;
	Controller *	pController = m_arrControllers.DetachAt( i ) ;
	if ( pController != nullptr )
	{
		BeforeDetachController( pController ) ;
		pController->m_pOwnerItem = nullptr ;
	}
	return	pController ;
}

S3DSceneComposer::Controller * S3DSceneComposer::ItemSerializer::DetachControllerAs( const wchar_t * pwszID )
{
	SSmartLock<SCriticalSection>	lock( &m_csCtrlSync ) ;
	Controller *	pController =
			m_arrControllers.DetachAt( (size_t) FindControllerID( pwszID ) ) ;
	if ( pController != nullptr )
	{
		BeforeDetachController( pController ) ;
		pController->m_pOwnerItem = nullptr ;
	}
	return	pController ;
}

// コントローラー操作排他操作
//（OnTimer は複数スレッドで実行される可能性があるため Lock 状態で実行される）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::LockController( void ) const
{
	m_csCtrlSync.Lock() ;
}

void S3DSceneComposer::ItemSerializer::UnlockController( void ) const
{
	m_csCtrlSync.Unlock() ;
}

atomic_int_t S3DSceneComposer::ItemSerializer::UnlockControllerAll( void ) const
{
	return	m_csCtrlSync.UnlockAll() ;
}

void S3DSceneComposer::ItemSerializer::RelockController( atomic_int_t nLock ) const
{
	m_csCtrlSync.Relock( nLock ) ;
}

const SSystem::SCriticalSection *
	S3DSceneComposer::ItemSerializer::GetControllerLocker( void ) const
{
	return	&m_csCtrlSync ;
}

// コントローラー遅延追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::DelayAddController
	( S3DSceneComposer::Controller * pController )
{
	m_csCtrlSync.Lock() ;
	m_arrDelayAddCtrls.Add( pController ) ;
	m_csCtrlSync.Unlock() ;
}

// コントローラー遅延削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::DelayRemoveControllerAt( size_t iCtrl )
{
	const size_t *	pIndexes ;
	size_t			nCount ;
	bool			flagAddLast = true ;
	m_csCtrlSync.Lock() ;
	pIndexes = m_aDelayRemoveCtrls.GetConstArray() ;
	nCount = m_aDelayRemoveCtrls.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( iCtrl < pIndexes[i] )
		{
			flagAddLast = false ;
			m_aDelayRemoveCtrls.InsertAt( i, iCtrl ) ;
			break ;
		}
		if ( iCtrl == pIndexes[i] )
		{
			flagAddLast = false ;
			break ;
		}
	}
	if ( flagAddLast )
	{
		m_aDelayRemoveCtrls.Add( iCtrl ) ;
	}
	m_csCtrlSync.Unlock() ;
}

// コントローラー遅延処理確定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CommitDelayControllers( void )
{
	if ( m_aDelayRemoveCtrls.GetLength() > 0 )
	{
		m_csCtrlSync.Lock() ;
		const size_t *	pRemoveIndexes = m_aDelayRemoveCtrls.GetConstArray() ;
		size_t			nRemoveCount = m_aDelayRemoveCtrls.GetLength() ;
		for ( size_t i = 0; i < nRemoveCount; i ++ )
		{
			RemoveControllerAt( pRemoveIndexes[i] - i ) ;
		}
		m_aDelayRemoveCtrls.RemoveAll() ;
		m_csCtrlSync.Unlock() ;
	}
	if ( m_arrDelayAddCtrls.GetLength() > 0 )
	{
		m_csCtrlSync.Lock() ;
		while ( m_arrDelayAddCtrls.GetLength() > 0 )
		{
			Controller *	pCtrl = m_arrDelayAddCtrls.DetachAt( 0 ) ;
			AddController( pCtrl ) ;
		}
		m_csCtrlSync.Unlock() ;
	}
}

// コントローラー通知
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnAddController
	( S3DSceneComposer::Controller * pController )
{
	pController->OnAddController( this ) ;
}

void S3DSceneComposer::ItemSerializer::BeforeDetachController
	( S3DSceneComposer::Controller * pController )
{
	pController->BeforeDetachController( this ) ;
}

// コントローラーID正規化
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::NormalizeControllerID( SSystem::SString& strID ) const
{
	if ( FindControllerID( strID ) >= 0 )
	{
		SString	strBaseID = strID ;
		while ( (strBaseID.GetLength() > 1)
			&& (strBaseID.GetLastAt(0) >= L'0')
			&& (strBaseID.GetLastAt(0) <= L'9') )
		{
			strBaseID.ChopRight(1) ;
		}
		for ( int i = 1; i < 0x10000; i ++ )
		{
			strID = strBaseID ;
			strID += SString( i ) ;
			if ( FindControllerID( strID ) < 0 )
			{
				break ;
			}
		}
	}
}

// Rosetta インスタンス取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneScriptInstance *
	S3DSceneComposer::ItemSerializer::GetScriptInstanceOf
		( const wchar_t * pwszClass ) const
{
	Controller *const*	ppControlers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DSceneScriptInstance *	pController =
				ESLTypeCast<S3DSceneScriptInstance>( ppControlers[i] ) ;
		if ( (pController != nullptr)
			&& (pController->GetScirptClassName() == pwszClass) )
		{
			return	pController ;
		}
	}
	return	nullptr ;
}

Rosetta::RSObject *
	S3DSceneComposer::ItemSerializer::GetRosettaInstanceOf
		( const wchar_t * pwszClass ) const
{
	S3DSceneScriptInstance *	pCtrl = GetScriptInstanceOf( pwszClass ) ;
	return	(pCtrl != nullptr) ? pCtrl->GetRSInstance() : nullptr ;
}

Loquaty::LObjPtr
	S3DSceneComposer::ItemSerializer::GetLoquatyInstanceOf
							( const wchar_t * pwszClass ) const
{
	S3DSceneScriptInstance *	pCtrl = GetScriptInstanceOf( pwszClass ) ;
	return	(pCtrl != nullptr) ? pCtrl->GetLoquatyInstance() : LObjPtr() ;
}

uint8_t * S3DSceneComposer::ItemSerializer::GetScriptStructInstanceOf
		( const wchar_t * pwszStruct, size_t nStructBytes ) const
{
	S3DSceneScriptInstance *	pCtrl = GetScriptInstanceOf( pwszStruct ) ;
	if ( pCtrl != nullptr )
	{
		Loquaty::LObjPtr	pLObj = pCtrl->GetLoquatyInstance() ;
		if ( pLObj != nullptr )
		{
			LPtr<LPointerObj>	pPtrObj( pLObj->GetBufferPoiner() ) ;
			if ( pPtrObj != nullptr )
			{
				return	pPtrObj->GetPointer( 0, nStructBytes ) ;
			}
			return	nullptr ;
		}
		Rosetta::RSSmartPtr	pObj( pCtrl->GetRSInstance() ) ;
		RSStructuredPointer *	pStruct =
				ESLTypeCast<RSStructuredPointer>( pObj.GetEntity().Ptr() ) ;
		if ( pStruct != nullptr )
		{
			return	pStruct->GetPointer( nStructBytes ) ;
		}
	}
	return	nullptr ;
}

// 親空間取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::ItemSerializer::GetParentSpaceItem( void ) const
{
	return	nullptr ;
}

// コンポジション取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
	S3DSceneComposer::ItemSerializer::GetComposition( void ) const
{
	return	nullptr ;
}

// コンポーザー取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer * S3DSceneComposer::ItemSerializer::GetComposer( void ) const
{
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	pComp->GetSceneComposer() ;
	}
	return	nullptr ;
}

// コンポーザー・マネージャー取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionManager * S3DSceneComposer::ItemSerializer::GetManager( void ) const
{
	S3DSceneComposer *	pComp = GetComposer() ;
	if ( pComp != nullptr )
	{
		return	pComp->GetManager() ;
	}
	return	nullptr ;
}

// シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DSceneComposer::ItemSerializer::GetScene( void ) const
{
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	pComp->GetScene() ;
	}
	return	nullptr ;
}

// エディター取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionEditorInterface *
	S3DSceneComposer::ItemSerializer::GetEditor( void ) const
{
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	pComp->GetEditor() ;
	}
	return	nullptr ;
}

// デバッグ用（エディタ出力）
void S3DSceneComposer::ItemSerializer::OutputError
	( const wchar_t * pwszErrMsg,
		const wchar_t * pwszSrcFile,
		int nLineNum, const wchar_t * pwszSrcLine )
{
	S3DCompositionEditorInterface *	pEditor = GetEditor() ;
	if ( pEditor != nullptr )
	{
		pEditor->OutputError( pwszErrMsg, pwszSrcFile, nLineNum, pwszSrcLine ) ;
	}
}

void S3DSceneComposer::ItemSerializer::OutputTraceLog( const wchar_t * pwszLogMsg )
{
	S3DCompositionEditorInterface *	pEditor = GetEditor() ;
	if ( pEditor != nullptr )
	{
		pEditor->OutputTraceLog( pwszLogMsg ) ;
	}
}

// アイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::ItemSerializer::GetSceneItemAs( const wchar_t * pwszID ) const
{
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	pComp->GetSceneItemAs( pwszID ) ;
	}
	return	nullptr ;
}

S3DSceneComposer::SpaceSerializer *
	S3DSceneComposer::ItemSerializer::GetSceneSpaceAs( const wchar_t * pwszID ) const
{
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		return	ESLTypeCast<SpaceSerializer>
					( pComp->GetSceneItemAs( pwszID ) ) ;
	}
	return	nullptr ;
}

S3DScene::Space *
	S3DSceneComposer::ItemSerializer::GetReferenceSpaceAs( const wchar_t * pwszPath ) const
{
	if ( (pwszPath == nullptr) || (pwszPath[0] == 0) )
	{
		return	nullptr ;
	}
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		ssize_t	iSepItem = -1 ;
		for ( size_t i = 0; pwszPath[i]; i ++ )
		{
			if ( pwszPath[i] == L'\\' )
			{
				iSepItem = (ssize_t) i ;
				break ;
			}
		}
		if ( iSepItem < 0 )
		{
			SpaceSerializer *
				pSpace = ESLTypeCast<SpaceSerializer>
							( pComp->GetSceneItemAs( pwszPath ) ) ;
			if ( pSpace != nullptr )
			{
				return	pSpace->GetSceneSpace() ;
			}
		}
		else
		{
			S3DScene::ModelItem *
				pItem = ESLTypeCast<S3DScene::ModelItem>
							( pComp->GetSceneItemAs
									( SString( pwszPath, iSepItem ) ) ) ;
			if ( pItem != nullptr )
			{
				S3DModelBuffer *	pModel =
					ESLTypeCast<S3DModelBuffer>( pItem->GetModel() ) ;
				if ( pModel != nullptr )
				{
					S3DModelBoneSpace *	pBone =
							pModel->GetBonePropertyAs
									( pwszPath + (iSepItem + 1) ) ;
					return	pBone ;
				}
			}
		}
	}
	return	nullptr ;
}

// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::GetItemLinkTransformation
		( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const
{
	matLink.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	vLinkPos = S3DDVector( 0, 0, 0 ) ;
}

// 空間行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const
{
	matGlobal.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	vGlobalPos = S3DDVector( 0, 0, 0 ) ;
}

// 空間行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::GetTransformationFrom
	( S3DDMatrix& matOffset, S3DDVector& vOffset,
						const ItemSerializer * pItem ) const
{
	S3DDMatrix	matdItem( 1, 1, 1 ) ;
	S3DDVector	vdItem( 0, 0, 0 ) ;
	if ( pItem != nullptr )
	{
		pItem->GetGlobalTransformation( matdItem, vdItem ) ;
	}
	GetTransformationFrom( matOffset, vOffset, matdItem, vdItem ) ;
}

void S3DSceneComposer::ItemSerializer::GetTransformationFrom
	( S3DDMatrix& matOffset, S3DDVector& vOffset,
		const S3DDMatrix& matAnother, const S3DDVector& vAnother ) const
{
	GetGlobalTransformation( matOffset, vOffset ) ;
	//
	S3DDMatrix	matdITarget = matOffset.Inverse() ;
	matOffset = matdITarget * matAnother ;
	vOffset = matdITarget * (vAnother - vOffset) ;
}

// 空間色効果取得（乗算色α要素に不透明度取得）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::GetGlobalColorEffect( S3DColor& clrEffect ) const
{
	clrEffect.rgbMul = 0xFFFFFFFF ;
	clrEffect.rgbAdd = 0 ;
}

// 子アイテム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::ItemSerializer::GetChildItemCount( void ) const
{
	return	0 ;
}

// 子アイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::ItemSerializer::GetChildItemAt( size_t i ) const
{
	return	nullptr ;
}

// 子アイテム検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSceneComposer::ItemSerializer::FindChildItem
					( S3DSceneComposer::ItemSerializer * pItem ) const
{
	size_t	nCount = GetChildItemCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( GetChildItemAt( i ) == pItem )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// ポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose * S3DSceneComposer::ItemSerializer::GetPoseIdentityAs( const wchar_t * pwszPoseID ) const
{
	for ( size_t i = 0; i < m_arrSequencers.GetLength(); i ++ )
	{
		PoseSequencer *	pPoseSeq =
			ESLTypeCast<PoseSequencer>( m_arrSequencers.GetAt( i ) ) ;
		if ( pPoseSeq != nullptr )
		{
			S3DModelPose *	pPose =
				pPoseSeq->GetTemporaryPoseIdentityAs( pwszPoseID ) ;
			if ( pPose != nullptr )
			{
				return	pPose ;
			}
		}
	}
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pComposer = pComp->GetSceneComposer() ;
		if ( pComposer != nullptr )
		{
			return	pComposer->GetAssets().
						GetPoseLibrary().GetPoseAs( pwszPoseID ) ;
		}
	}
	return	nullptr ;
}

// ポーズID取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ItemSerializer::GetPoseIdentityOf
	( SSystem::SString& strPoseID, S3DModelPose * pPose ) const
{
	if ( pPose == nullptr )
	{
		return	false ;
	}
	for ( size_t i = 0; i < m_arrSequencers.GetLength(); i ++ )
	{
		PoseSequencer *	pPoseSeq =
			ESLTypeCast<PoseSequencer>( m_arrSequencers.GetAt( i ) ) ;
		if ( pPoseSeq != nullptr )
		{
			if ( pPoseSeq->GetTemporaryPoseIdentityOf( strPoseID, pPose ) )
			{
				return	true ;
			}
		}
	}
	Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pComposer = pComp->GetSceneComposer() ;
		if ( pComposer != nullptr )
		{
			const wchar_t *	pwszID =
				pComposer->GetAssets().
						GetPoseLibrary().GetPoseIdentityOf( pPose ) ;
			if ( pwszID != nullptr )
			{
				strPoseID = pwszID ;
				return	true ;
			}
		}
	}
	return	false ;
}

// アイテムのプライマリモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface *
	S3DSceneComposer::ItemSerializer::GetItemPrimaryModel( void )
{
	return	nullptr ;
}

// アイテムのコリジョンバッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DCollider * S3DSceneComposer::ItemSerializer::GetItemPrimaryCollider( void )
{
	return	nullptr ;
}

// コンポジションツリーに追加された
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnAttachedCompositionTree
	( S3DSceneComposer::Composition& comp )
{
	UpdatePropertyReference( comp, updateRefAll ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::ItemSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t			nResFlags = 0 ;
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCtrlCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCtrlCount; i ++ )
	{
		Controller *	pController = ppControllers[i] ;
		if ( pController != nullptr )
		{
			nResFlags |=
				pController->UpdatePropertyReference( comp, this, nFlags ) ;
		}
	}
	Sequencer *const*	ppSeq = m_arrSequencers.GetConstArray() ;
	const size_t		nSeqCount = m_arrSequencers.GetLength() ;
	for ( size_t i = 0; i < nSeqCount; i ++ )
	{
		Sequencer *	pSeq = ppSeq[i] ;
		if ( pSeq != nullptr )
		{
			nResFlags |=
				pSeq->UpdatePropertyReference( comp, this, nFlags ) ;
		}
	}
	return	nResFlags ;
}

// レンダリング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnSetupSceneSettings
	( S3DScene& scene,
		const SGLSize& sizeFrame, SGLSecondaryViewProducer * psvp )
{
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControllers[i] ;
		if ( pController != nullptr )
		{
			pController->OnSetupSceneSettings( scene, this, sizeFrame, psvp ) ;
		}
	}
}

// レンダリング後始末
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnShoutdownSceneSettings
	( S3DScene& scene, SGLSecondaryViewProducer * psvp )
{
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControllers[i] ;
		if ( pController != nullptr )
		{
			pController->OnShoutdownSceneSettings( scene, this, psvp ) ;
		}
	}
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControllers[i] ;
		if ( pController != nullptr )
		{
			pController->OnExtendNotify
				( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
		}
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnTimer
				( S3DScene& scene, uint32_t msecPast )
{
	CommitDelayControllers() ;
	//
	m_csCtrlSync.Lock() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	m_flagsCtrlBehavior = 0 ;
	m_flagsCtrlEventClasses = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControllers[i] ;
		if ( (pController != nullptr)
			&& !pController->IsControllerDisabled() )
		{
			pController->OnTimer( scene, this, msecPast ) ;
			//
			m_flagsCtrlBehavior
					|= pController->GetControllerBehaviorFlags() ;
			m_flagsCtrlEventClasses
					|= pController->GetBehaviorRenderEventClasses() ;
		}
	}
	m_csCtrlSync.Unlock() ;
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	CallControllerOnUpdateFrame( fpFrame, seek ) ;
}

// 規定のレンダリングデバイス設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnSetRenderDevice( S3DRenderDevice * pDevice )
{
	CallControllerOnSetRenderDevice( pDevice ) ;
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnPrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	CallControllerOnPrepareToRender( pDevice, nFlags ) ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	CallControllerOnRenderEvent( scene, clsItem ) ;
}

// 当たり判定追加
//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnItemRenderCollision
	( const S3DScene& scene, S3DCollision& render )
{
	CallControllerRenderCollision( scene, render ) ;
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::OnItemRenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	CallControllerRenderModel
		( scene, scene.GetCurrentRenderingPhase(), render, flagsExclusion ) ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::BeforeItemRenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	CallControllerBeforeRenderModel
		( scene, scene.GetCurrentRenderingPhase(), render, flagsExclusion ) ;
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::AfterItemRenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	CallControllerAfterRenderModel
		( scene, scene.GetCurrentRenderingPhase(), render, flagsExclusion ) ;
}

// Controller の SetFrameParameters 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerSetFrameParameters
	( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	CommitDelayControllers() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControllers[i] ;
		if ( (pController != nullptr)
			&& !pController->IsControllerDisabled() )
		{
			pController->SetFrameParameters( fpFrame, seek ) ;
		}
	}
}

// Controller の OnUpdateFrame 呼び出し処理と挙動フラグの更新
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerOnUpdateFrame
	( double fpFrame, S3DSceneComposer::SeekMethod seek,
		uint32_t flagsNeesdBehavior, uint32_t flagsNegativeBehavior )
{
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	m_flagsCtrlBehavior = 0 ;
	m_flagsCtrlEventClasses = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pController = ppControllers[i] ;
		if ( pController != nullptr )
		{
			uint32_t	flagsBehavior =
							pController->GetControllerBehaviorFlags() ;
			if ( (flagsBehavior & flagsNeesdBehavior)
				|| (~flagsBehavior & flagsNegativeBehavior) )
			{
				if ( !pController->IsControllerDisabled() )
				{
					pController->OnUpdateFrame( this, fpFrame, seek ) ;
				}
				flagsBehavior = pController->GetControllerBehaviorFlags() ;
			}
			m_flagsCtrlBehavior |= flagsBehavior ;
			m_flagsCtrlEventClasses
					|= pController->GetBehaviorRenderEventClasses() ;
		}
	}
}

// Controller の OnSetRenderDevice 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerOnSetRenderDevice
	( S3DRenderDevice * pDevice )
{
	CommitDelayControllers() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pCtrl = ppControllers[i] ;
		if ( (pCtrl != nullptr) && !pCtrl->IsControllerDisabled() )
		{
			pCtrl->OnSetRenderDevice( pDevice ) ;
		}
	}
}

// Controller の OnPrepareToRender 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerOnPrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	CommitDelayControllers() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pCtrl = ppControllers[i] ;
		if ( (pCtrl != nullptr) && !pCtrl->IsControllerDisabled() )
		{
			pCtrl->OnPrepareToRender( pDevice, nFlags ) ;
		}
	}
}

// Controller の OnRenderEvent 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerOnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( !(m_flagsCtrlBehavior & behaviorRenderEvent) )
	{
		return ;
	}
	CommitDelayControllers() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pCtrl = ppControllers[i] ;
		if ( (pCtrl != nullptr)
			&& !pCtrl->IsControllerDisabled()
			&& (pCtrl->GetControllerBehaviorFlags() & behaviorRenderEvent) )
		{
			pCtrl->OnRenderEvent( scene, clsItem, this ) ;
		}
	}
}

// Controller の RenderCollision 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerRenderCollision
	( const S3DScene& scene, S3DCollision& render )
{
	if ( !(m_flagsCtrlBehavior & behaviorCollision) )
	{
		return ;
	}
	CommitDelayControllers() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pCtrl = ppControllers[i] ;
		if ( (pCtrl != nullptr)
			&& !pCtrl->IsControllerDisabled()
			&& (pCtrl->GetControllerBehaviorFlags() & behaviorCollision) )
		{
			pCtrl->RenderCollision( scene, this, render ) ;
		}
	}
}

// Controller の RenderModel 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerRenderModel
	( const S3DScene& scene, S3DScene::ItemClass clsItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( !(m_flagsCtrlBehavior & behaviorRender) )
	{
		return ;
	}
	CommitDelayControllers() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pCtrl = ppControllers[i] ;
		if ( (pCtrl != nullptr)
			&& !pCtrl->IsControllerDisabled()
			&& (pCtrl->GetControllerBehaviorFlags() & behaviorRender) )
		{
			pCtrl->RenderModel
				( scene, clsItem, this, render, flagsExclusion ) ;
		}
	}
}

// Controller の BeforeRenderModel 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerBeforeRenderModel
	( const S3DScene& scene, S3DScene::ItemClass clsItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( !(m_flagsCtrlBehavior & behaviorRenderContext) )
	{
		return ;
	}
	CommitDelayControllers() ;
	//
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pCtrl = ppControllers[i] ;
		if ( (pCtrl != nullptr)
			&& !pCtrl->IsControllerDisabled()
			&& (pCtrl->GetControllerBehaviorFlags() & behaviorRenderContext) )
		{
			pCtrl->BeforeRenderModel
				( scene, clsItem, this, render, flagsExclusion ) ;
		}
	}
}

// Controller の AfterRenderModel 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemSerializer::CallControllerAfterRenderModel
	( const S3DScene& scene, S3DScene::ItemClass clsItem,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( !(m_flagsCtrlBehavior & behaviorRenderContext) )
	{
		return ;
	}
	Controller *const*	ppControllers = m_arrControllers.GetConstArray() ;
	const size_t		nCount = m_arrControllers.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Controller *	pCtrl = ppControllers[i] ;
		if ( (pCtrl != nullptr)
			&& !pCtrl->IsControllerDisabled()
			&& (pCtrl->GetControllerBehaviorFlags() & behaviorRenderContext) )
		{
			pCtrl->AfterRenderModel
				( scene, clsItem, this, render, flagsExclusion ) ;
		}
	}
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneItem" ;
}



//////////////////////////////////////////////////////////////////////////////
// 空間・シリアライザ（共通基底）
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSceneComposer::CommonSerializer::m_paramEntries
			[S3DSceneComposer::CommonSerializer::paramCount] =
{
	{ L"position",
		S3DSceneComposer::typePosition,
		S3DSceneComposer::attrNoLocalTransform,
		L"位置", L"変換行列の内、平行移動成分を表現します" },
	{ L"rotation",
		S3DSceneComposer::typeRotation, 0,
		L"回転", L"変換行列の内、回転成分を四元数で表現します" },
	{ L"zoom",
		S3DSceneComposer::typeZoom, 0,
		L"拡大", L"変換行列の内、拡大率を表現します" },
	{ L"transparency",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrUIScalarSlider,
		L"透明度", L"描画透明度を指定します。0.0が不透明、1.0で完全に透明になります。",
		0.0, 1.0 },
	{ L"color_mul",
		S3DSceneComposer::typeColor,	0,
		L"色乗算", nullptr },
	{ L"color_add",
		S3DSceneComposer::typeColor,	0,	L"色加算", nullptr },
	{ L"visible",
		S3DSceneComposer::typeBoolean,	0,	L"可視状態", nullptr },
	{ L"force_toon",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"強制トゥーンシェーダー", nullptr },
	{ L"force_border",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"強制縁取り", nullptr },
	{ L"free_toon",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"強制トゥーンシェーダー無効化", nullptr },
	{ L"free_border",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"強制縁取り無効化", nullptr },
	{ L"use_collision",
		S3DSceneComposer::typeBoolean,	0,
		L"当たり判定", L"当たり判定を有効にします。" },
	{ L"global_space",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"大域空間", L"変換行列を常に大域空間への写像とします。" },
	{ L"camera_shift",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"カメラ座標相対", L"カメラ位置に対して空間座標を平行移動します" },
	{ L"camera_space",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"カメラ空間", L"カメラ姿勢に対して追従します" },
	{ L"hide_near",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"カメラから近い場合非表示", nullptr },
	{ L"hide_far",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"カメラから遠い場合非表示", nullptr },
} ;

const S3DSceneComposer::ParamSetClass
	S3DSceneComposer::CommonSerializer::m_pscClass =
{
	nullptr,
	S3DSceneComposer::CommonSerializer::paramCount,
	&S3DSceneComposer::CommonSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::CommonSerializer, ItemSerializer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::CommonSerializer::CommonSerializer
		( const wchar_t * pwszClassID,
			const S3DSceneComposer::ParamSetClass * pClass )
	: ItemSerializer( pwszClassID, pClass ),
		m_matSpaceRotation( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
		m_vSpaceZoom( 1, 1, 1 )
{
}

// 回転
//////////////////////////////////////////////////////////////////////////////
const S3DDMatrix& S3DSceneComposer::CommonSerializer::GetItemRotation( void ) const
{
	return	m_matSpaceRotation ;
}

void S3DSceneComposer::CommonSerializer::SetItemRotation( const S3DDMatrix& matRot )
{
	m_matSpaceRotation = matRot ;
	UpdateSpaceMatrix() ;
}

// 拡大
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DSceneComposer::CommonSerializer::GetItemZoom( void ) const
{
	return	m_vSpaceZoom ;
}

void S3DSceneComposer::CommonSerializer::SetItemZoom( const S3DDVector& vZoom )
{
	m_vSpaceZoom = vZoom ;
	UpdateSpaceMatrix() ;
}

// 位置
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DSceneComposer::CommonSerializer::GetItemPosition( void ) const
{
	return	GetSpaceInfo().m_vCenter ;
}

// 透明度
//////////////////////////////////////////////////////////////////////////////
unsigned int S3DSceneComposer::CommonSerializer::GetItemTransparency( void ) const
{
	return	GetSpaceInfo().m_nTransparency ;
}

// 色効果
//////////////////////////////////////////////////////////////////////////////
const S3DColor& S3DSceneComposer::CommonSerializer::GetItemColorEffect( void ) const
{
	return	GetSpaceInfo().m_colorEffect ;
}

// 変更フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::CommonSerializer::IsModifiedItemMatrix( void ) const
{
	return	(GetSpaceInfo().m_flagsModified & S3DScene::spaceElementTransformation) != 0 ;
}

bool S3DSceneComposer::CommonSerializer::IsModifiedItemPosition( void ) const
{
	return	(GetSpaceInfo().m_flagsModified & S3DScene::spaceElementPosition) != 0 ;
}

bool S3DSceneComposer::CommonSerializer::IsModifiedItemTransparency( void ) const
{
	return	(GetSpaceInfo().m_flagsModified & S3DScene::spaceElementTransparency) != 0 ;
}

bool S3DSceneComposer::CommonSerializer::IsModifiedItemColorAdd( void ) const
{
	return	(GetSpaceInfo().m_flagsModified & S3DScene::spaceElementColorAdd) != 0 ;
}

bool S3DSceneComposer::CommonSerializer::IsModifiedItemColorMul( void ) const
{
	return	(GetSpaceInfo().m_flagsModified & S3DScene::spaceElementColorMul) != 0 ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSceneComposer::CommonSerializer::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
		return	m_matSpaceRotation ;
	}
	return	S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
}

S3DDVector S3DSceneComposer::CommonSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
		return	m_vSpaceZoom ;
	case	paramPosition:
		return	GetSpaceInfo().m_vCenter ;
	case	paramColorMul:
		return	VectorFromColor( GetSpaceInfo().m_colorEffect.rgbMul ) ;
	case	paramColorAdd:
		return	VectorFromColor( GetSpaceInfo().m_colorEffect.rgbAdd ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DSceneComposer::CommonSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramTransparency:
		return	(double) GetSpaceInfo().m_nTransparency * (1.0 / 256.0) ;
	}
	return	0.0 ;
}

bool S3DSceneComposer::CommonSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramVisible:
		return	GetVisibleParameter() ;
	case	paramForceToon:
		return	((GetSpaceInfo().m_flagsShader & S3DScene::shaderForceToon) != 0) ;
	case	paramForceBorder:
		return	((GetSpaceInfo().m_flagsShader & S3DScene::shaderForceBorder) != 0) ;
	case	paramFreeToon:
		return	((GetSpaceInfo().m_flagsShader & S3DScene::shaderFreeToon) != 0) ;
	case	paramFreeBorder:
		return	((GetSpaceInfo().m_flagsShader & S3DScene::shaderFreeBorder) != 0) ;
	case	paramUseCollision:
		return	((GetBehaviorFlags() & S3DScene::itemCollision) != 0) ;
	case	paramGlobalSpace:
		return	((GetBehaviorFlags() & S3DScene::itemGlobalSpace) != 0) ;
	case	paramCameraShift:
		return	((GetBehaviorFlags() & S3DScene::itemCameraShift) != 0) ;
	case	paramCameraSpace:
		return	((GetBehaviorFlags() & S3DScene::itemCameraSpace) != 0) ;
	case	paramHideNear:
		return	((GetBehaviorFlags() & S3DScene::itemHideNear) != 0) ;
	case	paramHideFar:
		return	((GetBehaviorFlags() & S3DScene::itemHideFar) != 0) ;
	}
	return	false ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommonSerializer::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramRotation:
		m_matSpaceRotation = mat ;
		UpdateSpaceMatrix() ;
		break ;
	}
}

void S3DSceneComposer::CommonSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramZoom:
		m_vSpaceZoom = vec ;
		UpdateSpaceMatrix() ;
		break ;
	case	paramPosition:
		{
			S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
			spinf.m_vCenter = vec ;
			spinf.m_flagsModified |= S3DScene::spaceElementPosition ;
		}
		break ;
	case	paramColorMul:
		{
			S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
			spinf.m_colorEffect.rgbMul = ColorFromVector( vec ) ;
			spinf.m_flagsModified |= S3DScene::spaceElementColorMul ;
		}
		break ;
	case	paramColorAdd:
		{
			S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
			spinf.m_colorEffect.rgbAdd = ColorFromVector( vec ) ;
			spinf.m_flagsModified |= S3DScene::spaceElementColorAdd ;
		}
		break ;
	}
}

void S3DSceneComposer::CommonSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramTransparency:
		{
			S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
			spinf.m_nTransparency =
				(unsigned int) esl_clampi
					( eslRoundR32ToInt
						( (float32_t) (s * (256.0 / 1.0)) ), 0, 0x100 ) ;
			spinf.m_flagsModified |= S3DScene::spaceElementTransparency ;
		}
		break ;
	}
}

void S3DSceneComposer::CommonSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramVisible:
		SetVisibleParameter( b ) ;
		break ;
	case	paramForceToon:
		SpaceParameter().m_flagsShader =
			(SpaceParameter().m_flagsShader & ~S3DScene::shaderForceToon)
				| (b ? S3DScene::shaderForceToon : 0) ;
		break ;
	case	paramForceBorder:
		SpaceParameter().m_flagsShader =
			(SpaceParameter().m_flagsShader & ~S3DScene::shaderForceBorder)
				| (b ? S3DScene::shaderForceBorder : 0) ;
		break ;
	case	paramFreeToon:
		SpaceParameter().m_flagsShader =
			(SpaceParameter().m_flagsShader & ~S3DScene::shaderFreeToon)
				| (b ? S3DScene::shaderFreeToon : 0) ;
		break ;
	case	paramFreeBorder:
		SpaceParameter().m_flagsShader =
			(SpaceParameter().m_flagsShader & ~S3DScene::shaderFreeBorder)
				| (b ? S3DScene::shaderFreeBorder : 0) ;
		break ;
	case	paramUseCollision:
		SetBehaviorFlags
			( (GetBehaviorFlags() & ~S3DScene::itemCollision)
							| (b ? S3DScene::itemCollision : 0) ) ;
		break ;
	case	paramGlobalSpace:
		SetBehaviorFlags
			( (GetBehaviorFlags() & ~S3DScene::itemGlobalSpace)
							| (b ? S3DScene::itemGlobalSpace : 0) ) ;
		break ;
	case	paramCameraShift:
		SetBehaviorFlags
			( (GetBehaviorFlags() & ~S3DScene::itemCameraShift)
							| (b ? S3DScene::itemCameraShift : 0) ) ;
		break ;
	case	paramCameraSpace:
		SetBehaviorFlags
			( (GetBehaviorFlags() & ~S3DScene::itemCameraSpace)
							| (b ? S3DScene::itemCameraSpace : 0) ) ;
		break ;
	case	paramHideNear:
		SetBehaviorFlags
			( (GetBehaviorFlags() & ~S3DScene::itemHideNear)
							| (b ? S3DScene::itemHideNear : 0) ) ;
		break ;
	case	paramHideFar:
		SetBehaviorFlags
			( (GetBehaviorFlags() & ~S3DScene::itemHideFar)
							| (b ? S3DScene::itemHideFar : 0) ) ;
		break ;
	}
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::CommonSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	if ( iCategory == 0 )
	{
		return	L"基本設定" ;
	}
	return	ItemSerializer::GetParameterCategoryName( iCategory ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::CommonSerializer::IsParameterValidation( size_t i ) const
{
	return	true ;
}

// フレームを適用
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommonSerializer::SetFrameParameters
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
	spinf.m_flagsModified = 0 ;

	ItemSerializer::SetFrameParameters( fpFrame, seek ) ;
}

// 行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommonSerializer::SetItemMatrix( const S3DDMatrix& matrix )
{
	m_vSpaceZoom.x = 0.0 ;
	m_vSpaceZoom.y = 0.0 ;
	m_vSpaceZoom.z = 0.0 ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		m_vSpaceZoom.x += matrix.m[i][0] * matrix.m[i][0] ;
		m_vSpaceZoom.y += matrix.m[i][1] * matrix.m[i][1] ;
		m_vSpaceZoom.z += matrix.m[i][2] * matrix.m[i][2] ;
	}
	m_vSpaceZoom.x = sqrt( m_vSpaceZoom.x ) ;
	m_vSpaceZoom.y = sqrt( m_vSpaceZoom.y ) ;
	m_vSpaceZoom.z = sqrt( m_vSpaceZoom.z ) ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		if ( m_vSpaceZoom.x != 0.0 )
		{
			m_matSpaceRotation.m[i][0] = matrix.m[i][0] / m_vSpaceZoom.x ;
		}
		if ( m_vSpaceZoom.y != 0.0 )
		{
			m_matSpaceRotation.m[i][1] = matrix.m[i][1] / m_vSpaceZoom.y ;
		}
		if ( m_vSpaceZoom.z != 0.0 )
		{
			m_matSpaceRotation.m[i][2] = matrix.m[i][2] / m_vSpaceZoom.z ;
		}
	}
	S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
	spinf.m_matTransformation = matrix ;
	spinf.m_flagsModified |= S3DScene::spaceElementTransformation ;
}

// 座標設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommonSerializer::SetItemPositioin( const S3DDVector& vPos )
{
	S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
	spinf.m_vCenter = vPos ;
	spinf.m_flagsModified |= S3DScene::spaceElementPosition ;
}

// 変換行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CommonSerializer::UpdateSpaceMatrix( void )
{
	S3DScene::SpaceInfo&	spinf = SpaceParameter() ;
	spinf.m_matTransformation = m_matSpaceRotation ;
	spinf.m_matTransformation.MagnifyByVector( m_vSpaceZoom ) ;
	spinf.m_flagsModified |= S3DScene::spaceElementTransformation ;
}



//////////////////////////////////////////////////////////////////////////////
// 空間・シリアライザ
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSceneComposer::SpaceSerializer::m_paramEntries
		[S3DSceneComposer::SpaceSerializer::paramSpaceCount] =
{
	{ L"space_ignore",
			S3DSceneComposer::typeBoolean,	0,	L"処理無効", nullptr },
	{ L"layered_space",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"レイヤード描画",
			L"空間内の子アイテムを別パスで空間レイヤーに描画します" },
	{ L"layered_priority",
			S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant1,
			L"描画優先度",
			L"優先度の高い順に描画されます（優先度が大きい方が奥）" },
	{ L"layered_param",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1,
			L"描画パラメータ",
			L"レイヤー描画に使用するパラメータ（透明度 0.0～1.0）" },
	{ L"layered_refl_map",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrStringEnumeration,
			L"反射マップ",
			L"レイヤー描画に使用する反射用環境マップソース" },
	{ L"layered_refr_map",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrCategory1
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrStringEnumeration,
			L"屈折マップ",
			L"レイヤー描画に使用する屈折用環境マップソース" },
	{ L"layered_field",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"field 描画",
			L"field クラス子アイテムをこの空間レイヤーに描画します" },
	{ L"layered_static_item1",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"static_item1 描画",
			L"static_item1 クラス子アイテムをこの空間レイヤーに描画します" },
	{ L"layered_static_item2",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"static_item2 描画",
			L"static_item2 クラス子アイテムをこの空間レイヤーに描画します" },
	{ L"layered_dynamic_item1",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"dynamic_item1 描画",
			L"dynamic_item1 クラス子アイテムをこの空間レイヤーに描画します" },
	{ L"layered_dynamic_item2",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"dynamic_item2 描画",
			L"dynamic_item2 クラス子アイテムをこの空間レイヤーに描画します" },
	{ L"layered_dynamic_item3",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"dynamic_item3 描画",
			L"dynamic_item3 クラス子アイテムをこの空間レイヤーに描画します" },
	{ L"layered_effect_item",
			S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrCategory1,
			L"effect_item 描画",
			L"effect_item クラス子アイテムをこの空間レイヤーに描画します" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DSceneComposer::SpaceSerializer::m_pscClass =
{
	&S3DSceneComposer::CommonSerializer::m_pscClass,
	S3DSceneComposer::SpaceSerializer::paramSpaceCount,
	&S3DSceneComposer::SpaceSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::SpaceSerializer::m_aiEnvMapSource[3] =
{
	{ L"default", S3DScene::envmapSourceDefault },
	{ L"viewport", S3DScene::envmapSourceViewport },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2_CAST
	( SakuraGL::S3DSceneComposer::SpaceSerializer, Space, CommonSerializer, m_pSpace )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DSceneComposer::SpaceSerializer, space )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::SpaceSerializer::SpaceSerializer( S3DScene::Space * pSpace )
	: CommonSerializer
		( m_ItemClassDescriptor.pwszClassID,
			&S3DSceneComposer::SpaceSerializer::m_pscClass )
{
	m_pSpace = (pSpace != nullptr) ? pSpace : (S3DScene::Space*) this ;
	m_pSpace->AttachItemListener( this ) ;
}

S3DSceneComposer::SpaceSerializer::SpaceSerializer
		( const wchar_t * pwszClassID,
			const ParamSetClass * pClass, S3DScene::Space * pSpace )
	: CommonSerializer( pwszClassID, pClass )
{
	m_pSpace = (pSpace != nullptr) ? pSpace : (S3DScene::Space*) this ;
	m_pSpace->AttachItemListener( this ) ;
}

// S3DScene::Space 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::AttachSpace( S3DScene::Space * pSpace )
{
	if ( m_pSpace != nullptr )
	{
		m_pSpace->AttachItemListener( nullptr ) ;
	}
	m_pSpace = pSpace ;
	if ( m_pSpace != nullptr )
	{
		m_pSpace->AttachItemListener( this ) ;
	}
}

// 所有リソース解放
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::ReleaseAllAssets( void )
{
	RemoveAllItems() ;
	RemoveAllChildren() ;
	m_arrItemAssets.RemoveAll() ;
}

// 親空間取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::SpaceSerializer::GetParentSpaceItem( void ) const
{
	S3DScene::Space *	pSpace = m_pSpace ;
	if ( pSpace != nullptr )
	{
		return	ESLTypeCast<ItemSerializer>( pSpace->GetParentSpace() ) ;
	}
	return	nullptr ;
}

// コンポジション検索
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
		S3DSceneComposer::SpaceSerializer::GetComposition( void ) const
{
	S3DScene::Space *	pSpace = m_pSpace ;
	while ( pSpace != nullptr )
	{
		Composition *	pComp = ESLTypeCast<Composition>( pSpace ) ;
		if ( pComp != nullptr )
		{
			return	pComp ;
		}
		pSpace = pSpace->GetParentSpace() ;
	}
	return	nullptr ;
}

// シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DSceneComposer::SpaceSerializer::GetScene( void ) const
{
	return	ItemSerializer::GetScene() ;
}

// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::GetItemLinkTransformation
		( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const
{
	S3DScene::Space *	pSpace = m_pSpace ;
	if ( pSpace != nullptr )
	{
		Space *	pParent = pSpace->GetParentSpace() ;
		if ( pParent != nullptr )
		{
			pParent->CalcGlobalTransformation( matLink, vLinkPos ) ;
		}
		else
		{
			matLink.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
			vLinkPos = S3DDVector( 0, 0, 0 ) ;
		}
	}
	else
	{
		CommonSerializer::GetItemLinkTransformation( matLink, vLinkPos ) ;
	}
}

// 空間行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const
{
	S3DScene::Space *	pSpace = m_pSpace ;
	if ( pSpace != nullptr )
	{
		pSpace->CalcGlobalTransformation( matGlobal, vGlobalPos ) ;
	}
	else
	{
		CommonSerializer::GetGlobalTransformation( matGlobal, vGlobalPos ) ;
	}
}

// 空間色効果取得（乗算色α要素に不透明度取得）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::GetGlobalColorEffect( S3DColor& clrEffect ) const
{
	S3DScene::Space *	pSpace = m_pSpace ;
	if ( pSpace != nullptr )
	{
		pSpace->CalcGlobalColorEffect( clrEffect ) ;
	}
	else
	{
		CommonSerializer::GetGlobalColorEffect( clrEffect ) ;
	}
}

// 子アイテム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::SpaceSerializer::GetChildItemCount( void ) const
{
	S3DScene::Space *	pSpace = m_pSpace ;
	if ( pSpace != nullptr )
	{
		return	pSpace->GetChildrenCount() + pSpace->GetItemCount() ;
	}
	return	0 ;
}

// 子アイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::SpaceSerializer::GetChildItemAt( size_t i ) const
{
	S3DScene::Space *	pSpace = m_pSpace ;
	if ( pSpace != nullptr )
	{
		size_t	nChildren = pSpace->GetChildrenCount() ;
		if ( i < nChildren )
		{
			return	ESLTypeCast<ItemSerializer>
							( pSpace->GetChildAt( i ) ) ;
		}
		else
		{
			return	ESLTypeCast<ItemSerializer>
							( pSpace->GetItemAt( i - nChildren ) ) ;
		}
	}
	return	nullptr ;
}

// アイテムID設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::SetItemIdentity( const wchar_t * pwszID )
{
	CommonSerializer::SetItemIdentity( pwszID ) ;
	if ( m_pSpace != nullptr )
	{
		m_pSpace->AttachSpaceIDString( GetItemIdentity() ) ;
	}
}

// 読み込み処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::SpaceSerializer::ParseNonParameterTag
	( S3DSceneComposer& composer,
		S3DSceneComposer::Composition& composition,
		S3DScene::Space& space,
		const SSystem::SXMLDocument& xmlTag )
{
	if ( xmlTag.GetTag() == L"children" )
	{
		m_arrItemAssets.RemoveAll() ;
		RemoveAllChildren() ;
		//
		return	ParseSpaceChildren
				( composer, composition, *this,
					IsSubCompositionToParseChildren(), xmlTag ) ;
	}
	return	sglErrFailed ;
}

SGLError S3DSceneComposer::SpaceSerializer::ParseSpaceChildren
	( S3DSceneComposer& composer,
		S3DSceneComposer::Composition& composition,
		S3DSceneComposer::SpaceSerializer& space,
		bool flagSubComposition,
		const SSystem::SXMLDocument& xmlChildren )
{
	SGLError	errResult = sglErrSuccess ;
	for ( size_t i = 0; i < xmlChildren.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlChild = xmlChildren.GetElementAt( i ) ;
		if ( pxmlChild == nullptr )
		{
			continue ;
		}
		const SString *	pstrID = pxmlChild->GetAttributeAs( L"id" ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		if ( flagSubComposition && (pstrID->GetAt(0) == L'@') )
		{
			continue ;
		}
		ItemSerializer *
			pItem = composer.CreateSceneItem( pxmlChild->GetTag() ) ;
		if ( pItem == nullptr )
		{
			continue ;
		}
		composition.AddSpaceChild( space, pItem, *pstrID ) ;
		//
		SGLError	err =
			pItem->ParseItem
				( composer, composition, space, *pxmlChild ) ;
		if ( err )
		{
//			composition.DetachSpaceChild( space, pItem ) ;
//			delete	pItem ;
			errResult = err ;
			continue ;
		}
	}
	return	errResult ;
}

bool S3DSceneComposer::SpaceSerializer::IsSubCompositionToParseChildren( void ) const
{
	return	false ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::SpaceSerializer::FormatItem
	( S3DSceneComposer& composer,
		SSystem::SXMLDocument& xmlTag, uint32_t nFlags )
{
	SGLError	err =
		CommonSerializer::FormatItem( composer, xmlTag, nFlags ) ;
	//
	SXMLDocument *	pxmlChildren = xmlTag.CreateElementTagAs( L"children" ) ;
	FormatSpaceChildren( composer, *this, *pxmlChildren, nFlags ) ;
	//
	return	err ;
}

SGLError S3DSceneComposer::SpaceSerializer::FormatSpaceChildren
	( S3DSceneComposer& composer,
		SpaceSerializer& space,
		SSystem::SXMLDocument& xmlChildren, uint32_t nFlags )
{
	SGLError	errResult = sglErrSuccess ;
	size_t	nCount = space.m_pSpace->GetChildrenCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pChild =
			ESLTypeCast<ItemSerializer>( space.m_pSpace->GetChildAt( i ) ) ;
		if ( pChild != nullptr )
		{
			SXMLDocument *	pxmlTag = new SXMLDocument ;
			SGLError	err = pChild->FormatItem( composer, *pxmlTag, nFlags ) ;
			if ( err )
			{
				errResult = err ;
				delete	pxmlTag ;
			}
			else
			{
				xmlChildren.AddElement( pxmlTag ) ;
			}
		}
	}
	nCount = space.m_pSpace->GetItemCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pItem =
			ESLTypeCast<ItemSerializer>( space.m_pSpace->GetItemAt( i ) ) ;
		if ( pItem != nullptr )
		{
			SXMLDocument *	pxmlTag = new SXMLDocument ;
			SGLError	err = pItem->FormatItem( composer, *pxmlTag, nFlags ) ;
			if ( err )
			{
				errResult = err ;
				delete	pxmlTag ;
			}
			else
			{
				xmlChildren.AddElement( pxmlTag ) ;
			}
		}
	}
	return	errResult ;
}

// フレーム反映
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::SetAllItemsFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	if ( ((seek == seekStream) || (seek == seekStreamPaused))
		&& (m_flagsSpaceBehavior & S3DScene::itemIgnore) )
	{
		// itemIgnore から復帰判定
		Sequencer *	pSeq = m_arrSequencers.GetAt( paramIgnore ) ;
		if ( pSeq == nullptr )
		{
			return ;
		}
		bool	flagIgnore = pSeq->GetFrameBoolean( fpFrame ) ;
		if ( flagIgnore )
		{
			return ;
		}
		m_flagsSpaceBehavior &= ~S3DScene::itemIgnore ;
	}
	//
	// 所有アイテムのフレーム反映
	//
	for ( size_t i = 0; i < m_arrItemAssets.GetLength(); i ++ )
	{
		ItemSerializer *	pItem = m_arrItemAssets.GetAt( i ) ;
		if ( pItem != nullptr )
		{
			pItem->SetFrameParameters( fpFrame, seek ) ;
		}
	}
	//
	// 自分自身のフレーム反映
	//
	SetFrameParameters( fpFrame, seek ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSceneComposer::SpaceSerializer::GetVectorParameter( size_t i ) const
{
	S3DDVector	vTemp ;
	switch ( i )
	{
	case	paramPosition:
		return	GetLocalSpacePosition( vTemp ) ;
	}
	return	CommonSerializer::GetVectorParameter( i ) ;
}

double S3DSceneComposer::SpaceSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLayeredParam:
		return	GetLayeredDrawParameter() ;
	}
	return	CommonSerializer::GetScalarParameter( i ) ;
}

int32_t S3DSceneComposer::SpaceSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLayeredPriority:
		return	GetDrawingPriority() ;
	}
	return	CommonSerializer::GetIntegerParameter( i ) ;
}

bool S3DSceneComposer::SpaceSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramIgnore:
		return	((GetBehaviorFlags() & S3DScene::itemIgnore) != 0) ;
	case	paramSetLayerSace:
		return	(m_maskSpaceClasses & (1 << S3DScene::classLayeredSpace)) != 0 ;
	case	paramLayeredField:
		return	(m_maskLayeredClasses & (1 << S3DScene::classField)) != 0 ;
	case	paramLayeredStaticItem1:
		return	(m_maskLayeredClasses & (1 << S3DScene::classStaticItem1)) != 0 ;
	case	paramLayeredStaticItem2:
		return	(m_maskLayeredClasses & (1 << S3DScene::classStaticItem2)) != 0 ;
	case	paramLayeredDynamicItem1:
		return	(m_maskLayeredClasses & (1 << S3DScene::classDynamicItem1)) != 0 ;
	case	paramLayeredDynamicItem2:
		return	(m_maskLayeredClasses & (1 << S3DScene::classDynamicItem2)) != 0 ;
	case	paramLayeredDynamicItem3:
		return	(m_maskLayeredClasses & (1 << S3DScene::classDynamicItem3)) != 0 ;
	case	paramLayeredEffectItem:
		return	(m_maskLayeredClasses & (1 << S3DScene::classEffectItem)) != 0 ;
	}
	return	CommonSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DSceneComposer::SpaceSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLayeredReflMap:
		return	SXMLDocument::GetSymbolAsIntegerOf
					( m_aiEnvMapSource, m_emsReflectionSource ) ;
	case	paramLayeredRefrMap:
		return	SXMLDocument::GetSymbolAsIntegerOf
					( m_aiEnvMapSource, m_emsRefractionSource ) ;
	}
	return	CommonSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramPosition:
		SetLocalSpacePosition( vec ) ;
		return ;
	}
	return	CommonSerializer::SetVectorParameter( i, vec ) ;
}

void S3DSceneComposer::SpaceSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramLayeredParam:
		SetLayeredDrawParameter( (float32_t) s ) ;
		return ;
	}
	return	CommonSerializer::SetScalarParameter( i, s ) ;
}

void S3DSceneComposer::SpaceSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramLayeredPriority:
		SetDrawingPriority( n ) ;
		return ;
	}
	return	CommonSerializer::SetIntegerParameter( i, n ) ;
}

void S3DSceneComposer::SpaceSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramIgnore:
		SetBehaviorFlags
			( (GetBehaviorFlags() & ~S3DScene::itemIgnore)
							| (b ? S3DScene::itemIgnore : 0) ) ;
		return ;
	case	paramSetLayerSace:
		m_maskSpaceClasses =
			(m_maskSpaceClasses & ~(1 << S3DScene::classLayeredSpace))
							| (b ? (1 << S3DScene::classLayeredSpace) : 0) ;
		return ;
	case	paramLayeredField:
		m_maskLayeredClasses =
			(m_maskLayeredClasses & ~(1 << S3DScene::classField))
							| (b ? (1 << S3DScene::classField) : 0) ;
		return ;
	case	paramLayeredStaticItem1:
		m_maskLayeredClasses =
			(m_maskLayeredClasses & ~(1 << S3DScene::classStaticItem1))
							| (b ? (1 << S3DScene::classStaticItem1) : 0) ;
		return ;
	case	paramLayeredStaticItem2:
		m_maskLayeredClasses =
			(m_maskLayeredClasses & ~(1 << S3DScene::classStaticItem2))
							| (b ? (1 << S3DScene::classStaticItem2) : 0) ;
		return ;
	case	paramLayeredDynamicItem1:
		m_maskLayeredClasses =
			(m_maskLayeredClasses & ~(1 << S3DScene::classDynamicItem1))
							| (b ? (1 << S3DScene::classDynamicItem1) : 0) ;
		return ;
	case	paramLayeredDynamicItem2:
		m_maskLayeredClasses =
			(m_maskLayeredClasses & ~(1 << S3DScene::classDynamicItem2))
							| (b ? (1 << S3DScene::classDynamicItem2) : 0) ;
		return ;
	case	paramLayeredDynamicItem3:
		m_maskLayeredClasses =
			(m_maskLayeredClasses & ~(1 << S3DScene::classDynamicItem3))
							| (b ? (1 << S3DScene::classDynamicItem3) : 0) ;
		return ;
	case	paramLayeredEffectItem:
		m_maskLayeredClasses =
			(m_maskLayeredClasses & ~(1 << S3DScene::classEffectItem))
							| (b ? (1 << S3DScene::classEffectItem) : 0) ;
		return ;
	}
	CommonSerializer::SetBooleanParameter( i, b ) ;
}

void S3DSceneComposer::SpaceSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramLayeredReflMap:
		m_emsReflectionSource =
			(S3DScene::EnvironmentMappingSource)
				SXMLDocument::GetIntegerAsSymbolOf
					( m_aiEnvMapSource, pwszCmd, m_emsReflectionSource ) ;
		return ;
	case	paramLayeredRefrMap:
		m_emsRefractionSource =
			(S3DScene::EnvironmentMappingSource)
				SXMLDocument::GetIntegerAsSymbolOf
					( m_aiEnvMapSource, pwszCmd, m_emsRefractionSource ) ;
		return ;
	}
	CommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::SpaceSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramLayeredReflMap:
	case	paramLayeredRefrMap:
		for ( j = 0; m_aiEnvMapSource[j].pszSymbol; j ++ )
		{
			aStrSet.Add( new SString( m_aiEnvMapSource[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	CommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	S3DSceneComposer::SpaceSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"レイヤード描画" ;
	}
	return	CommonSerializer::GetParameterCategoryName( iCategory ) ;
}

// S3DScene::SpaceInfo 取得
//////////////////////////////////////////////////////////////////////////////
const S3DScene::SpaceInfo&
	S3DSceneComposer::SpaceSerializer::GetSpaceInfo( void ) const
{
	return	*this ;
}

S3DScene::SpaceInfo& S3DSceneComposer::SpaceSerializer::SpaceParameter( void )
{
	return	*this ;
}

// 表示フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::SpaceSerializer::GetVisibleParameter( void ) const
{
	return	!(m_flagsSpaceBehavior & S3DScene::itemSpaceHidden) ;
}

// 表示フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::SetVisibleParameter( bool b )
{
	if ( !b )
	{
		ModifyBehaviorFlags( S3DScene::itemSpaceHidden, 0 ) ;
	}
	else
	{
		ModifyBehaviorFlags( 0, S3DScene::itemSpaceHidden ) ;
	}
}

// 動作フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::SpaceSerializer::GetBehaviorFlags( void ) const
{
	return	m_flagsSpaceBehavior ;
}

// 動作フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::SetBehaviorFlags( uint32_t nFlags )
{
	m_flagsSpaceBehavior = nFlags ;
}

// 変換行列更新
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::UpdateSpaceMatrix( void )
{
	S3DDMatrix	matLocal = m_matSpaceRotation ;
	matLocal.MagnifyByVector( m_vSpaceZoom ) ;
	SetLocalTransformation( matLocal ) ;
}

// 無効化状態
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::SpaceSerializer::IsIgnoreParameter( void ) const
{
	return	((m_flagsSpaceBehavior & S3DScene::itemIgnore) != 0) ;
}

void S3DSceneComposer::SpaceSerializer::SetIgnoreParameter( bool flagIgnore )
{
	m_flagsSpaceBehavior = (m_flagsSpaceBehavior & ~S3DScene::itemIgnore)
								| (flagIgnore ? S3DScene::itemIgnore : 0) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::SpaceSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		CommonSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	size_t	nCount = m_pSpace->GetChildrenCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pChild =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetChildAt( i ) ) ;
		if ( pChild != nullptr )
		{
			nResFlags |= pChild->UpdatePropertyReference( comp, nFlags ) ;
		}
	}
	nCount = m_pSpace->GetItemCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pItem =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetItemAt( i ) ) ;
		if ( pItem != nullptr )
		{
			nResFlags |= pItem->UpdatePropertyReference( comp, nFlags ) ;
		}
	}
	return	nResFlags ;
}

// レンダリング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::OnSetupSceneSettings
	( S3DScene& scene,
		const SGLSize& sizeFrame, SGLSecondaryViewProducer * psvp )
{
	CommonSerializer::OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
	//
	size_t	nCount = m_pSpace->GetChildrenCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pChild =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetChildAt( i ) ) ;
		if ( pChild != nullptr )
		{
			pChild->OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
		}
	}
	nCount = m_pSpace->GetItemCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pItem =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetItemAt( i ) ) ;
		if ( pItem != nullptr )
		{
			pItem->OnSetupSceneSettings( scene, sizeFrame, psvp ) ;
		}
	}
}

// レンダリング後始末
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::OnShoutdownSceneSettings
	( S3DScene& scene, SGLSecondaryViewProducer * psvp )
{
	size_t	nCount = m_pSpace->GetChildrenCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pChild =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetChildAt( i ) ) ;
		if ( pChild != nullptr )
		{
			pChild->OnShoutdownSceneSettings( scene, psvp ) ;
		}
	}
	nCount = m_pSpace->GetItemCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pItem =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetItemAt( i ) ) ;
		if ( pItem != nullptr )
		{
			pItem->OnShoutdownSceneSettings( scene, psvp ) ;
		}
	}
	CommonSerializer::OnShoutdownSceneSettings( scene, psvp ) ;
}

// 拡張的な処理の通知
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::OnExtendNotify
	( const wchar_t * pwszCmd, const wchar_t * pwszParam,
		const void * pExParam, size_t nExParamBytes )
{
	CommonSerializer::OnExtendNotify
		( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
	//
	size_t	nCount = m_pSpace->GetChildrenCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pChild =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetChildAt( i ) ) ;
		if ( pChild != nullptr )
		{
			pChild->OnExtendNotify
				( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
		}
	}
	nCount = m_pSpace->GetItemCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ItemSerializer *	pItem =
			ESLTypeCast<ItemSerializer>( m_pSpace->GetItemAt( i ) ) ;
		if ( pItem != nullptr )
		{
			pItem->OnExtendNotify
				( pwszCmd, pwszParam, pExParam, nExParamBytes ) ;
		}
	}
}

// タイマ処理 Space::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	Space::OnTimer( scene, msecPast ) ;
	ItemSerializer::OnTimer( scene, msecPast ) ;
	//
	// OnPreRenderFrame 呼び出しが必要か判定
	//
	if ( m_flagsCtrlBehavior & (behaviorRenderEvent | behaviorRender) )
	{
		m_maskSpaceClasses |= m_flagsCtrlEventClasses ;
	}
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	if ( !(m_flagsSpaceBehavior & S3DScene::itemIgnore) )
	{
		for ( size_t i = 0; i < m_arrItemAssets.GetLength(); i ++ )
		{
			ItemSerializer *	pItem = m_arrItemAssets.GetAt( i ) ;
			if ( pItem != nullptr )
			{
				pItem->OnUpdateFrame( fpFrame, seek ) ;
			}
		}
		CommonSerializer::OnUpdateFrame( fpFrame, seek ) ;
//		CallControllerOnUpdateFrame
//			( fpFrame, seek, 0, behaviorAlwaysActive ) ;
	}
	else
	{
		CallControllerOnUpdateFrame
			( fpFrame, seek, behaviorAlwaysActive, 0 ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::SpaceSerializer::OnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	Space::OnRenderEvent( scene, clsItem ) ;
	//
	CallControllerOnRenderEvent( scene, clsItem ) ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::SpaceSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneSpace" ;
}




//////////////////////////////////////////////////////////////////////////////
// アイテム・基底シリアライザ
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSceneComposer::ItemCommonSerializer::m_paramEntries
		[S3DSceneComposer::ItemCommonSerializer::paramItemCount] =
{
	{ L"item_class",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration, L"アイテムクラス", nullptr },
	{ L"render_priority",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant,
		L"描画優先度", L"0～15の描画順優先度（数値が小さい順に描画）" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DSceneComposer::ItemCommonSerializer::m_pscClass =
{
	&S3DSceneComposer::CommonSerializer::m_pscClass,
	S3DSceneComposer::ItemCommonSerializer::paramItemCount,
	&S3DSceneComposer::ItemCommonSerializer::m_paramEntries[0]
} ;

const wchar_t *	S3DSceneComposer::ItemCommonSerializer::m_pwszItemClassIDs[S3DScene::classCount] =
{
	L"camera", L"light",
	L"prerender", L"prerender2", L"prerender_frame",
	L"begin_frame",
	L"backscape", L"scape", L"field",
	L"static_item1", L"static_item2",
	L"dynamic_item1", L"dynamic_item2", L"dynamic_item3", L"effect_item",
	L"layered_space", L"layered1", L"layered2",
	L"effect1", L"effect2", L"effect3",
	L"end_frame",
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ItemCommonSerializer, CommonSerializer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemCommonSerializer::ItemCommonSerializer
		( const wchar_t * pwszClassID,
			const ParamSetClass * pClass, S3DScene::Item * pItem )
	: CommonSerializer( pwszClassID, pClass ), m_pItem( pItem )
{
	if ( pItem != nullptr )
	{
		pItem->AttachItemListener( this ) ;
	}
}

// アイテム関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::AttachSceneItem( S3DScene::Item * pItem )
{
	if ( m_pItem != nullptr )
	{
		m_pItem->AttachItemListener( nullptr ) ;
	}
	m_pItem = pItem ;
	if ( m_pItem != nullptr )
	{
		m_pItem->AttachItemListener( this ) ;
	}
}

// 親空間取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::ItemCommonSerializer::GetParentSpaceItem( void ) const
{
	if ( m_pItem != nullptr )
	{
		SpaceSerializer *	pSpace =
			ESLTypeCast<SpaceSerializer>( m_pItem->GetParentSpace() ) ;
		if ( pSpace != nullptr )
		{
			return	pSpace ;
		}
	}
	return	nullptr ;
}

// コンポジション検索
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
	S3DSceneComposer::ItemCommonSerializer::GetComposition( void ) const
{
	if ( m_pItem != nullptr )
	{
		SpaceSerializer *	pSpace =
			ESLTypeCast<SpaceSerializer>( m_pItem->GetParentSpace() ) ;
		if ( pSpace != nullptr )
		{
			return	pSpace->GetComposition() ;
		}
	}
	return	nullptr ;
}

// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::GetItemLinkTransformation
		( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const
{
	if ( m_pItem != nullptr )
	{
		m_pItem->CalcItemLinkTransformation( matLink, vLinkPos ) ;
	}
	else
	{
		CommonSerializer::GetItemLinkTransformation( matLink, vLinkPos ) ;
	}
}

// 空間行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::GetGlobalTransformation
				( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const
{
	if ( m_pItem != nullptr )
	{
		m_pItem->CalcGlobalTransformation( matGlobal, vGlobalPos ) ;
	}
	else
	{
		CommonSerializer::GetGlobalTransformation( matGlobal, vGlobalPos ) ;
	}
}

// 空間色効果取得（乗算色α要素に不透明度取得）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::GetGlobalColorEffect( S3DColor& clrEffect ) const
{
	if ( m_pItem != nullptr )
	{
		m_pItem->CalcGlobalColorEffect( clrEffect ) ;
	}
	else
	{
		CommonSerializer::GetGlobalColorEffect( clrEffect ) ;
	}
}

// アイテムID設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::SetItemIdentity( const wchar_t * pwszID )
{
	CommonSerializer::SetItemIdentity( pwszID ) ;
	if ( m_pItem != nullptr )
	{
		m_pItem->m_pwszID = GetItemIdentity() ;
	}
}

// S3DScene::SpaceInfo 取得
//////////////////////////////////////////////////////////////////////////////
const S3DScene::SpaceInfo&
	S3DSceneComposer::ItemCommonSerializer::GetSpaceInfo( void ) const
{
	ESLAssert( m_pItem != nullptr ) ;
	return	m_pItem->m_space ;
}

S3DScene::SpaceInfo&
	S3DSceneComposer::ItemCommonSerializer::SpaceParameter( void )
{
	ESLAssert( m_pItem != nullptr ) ;
	return	m_pItem->m_space ;
}

// 表示フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ItemCommonSerializer::GetVisibleParameter( void ) const
{
	ESLAssert( m_pItem != nullptr ) ;
	return	(m_pItem->m_flagsBehavior & S3DScene::itemVisible) != 0 ;
}

// 表示フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::SetVisibleParameter( bool b )
{
	ESLAssert( m_pItem != nullptr ) ;
	if ( b )
	{
		m_pItem->m_flagsBehavior |= S3DScene::itemVisible ;
	}
	else
	{
		m_pItem->m_flagsBehavior &= ~S3DScene::itemVisible ;
	}
}

// 動作フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::ItemCommonSerializer::GetBehaviorFlags( void ) const
{
	return	m_pItem->m_flagsBehavior ;
}

// 動作フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::SetBehaviorFlags( uint32_t nFlags )
{
	m_pItem->m_flagsBehavior = nFlags ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DSceneComposer::ItemCommonSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramItemPriority:
		return	(int32_t) m_pItem->m_nRenderPriority ;
	}
	return	CommonSerializer::GetIntegerParameter( i ) ;
}

const wchar_t * S3DSceneComposer::ItemCommonSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramItemClass:
		return	m_pwszItemClassIDs[m_pItem->m_classItem] ;
	}
	return	CommonSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramItemPriority:
		m_pItem->m_nRenderPriority = (uint32_t) n ;
		return ;
	}
	CommonSerializer::SetIntegerParameter( i, n ) ;
}

void S3DSceneComposer::ItemCommonSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramItemClass:
		if ( (m_pItem != nullptr)
			&& SString::Compare
				( pwszCmd, m_pwszItemClassIDs[m_pItem->m_classItem] ) != 0 )
		{
			for ( int j = 0; j < S3DScene::classCount; j ++ )
			{
				if ( SString::Compare( pwszCmd, m_pwszItemClassIDs[j] ) == 0 )
				{
					m_pItem->m_classItem = (S3DScene::ItemClass) j ;
					break ;
				}
			}
		}
		return ;
	}
	CommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ItemCommonSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	SPointerArray<ItemSerializer>	aItems ;
	size_t	j ;
	switch ( i )
	{
	case	paramItemClass:
		for ( j = 0; j < S3DScene::classCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszItemClassIDs[j] ) ) ;
		}
		return	true ;
	}
	return	CommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ItemCommonSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
		return	false ;
	}
	return	CommonSerializer::IsParameterValidation( i ) ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	CommonSerializer::OnTimer( scene, msecPast ) ;
	//
	if ( m_pItem != nullptr )
	{
		if ( m_flagsCtrlBehavior
			& (behaviorRenderEvent | behaviorRender | behaviorRenderContext) )
		{
			m_pItem->m_maskClasses |= m_flagsCtrlEventClasses ;
		}
	}
}

// フレーム更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemCommonSerializer::OnUpdateFrame( double fpFrame, SeekMethod seek )
{
	CommonSerializer::OnUpdateFrame( fpFrame, seek ) ;
	//
	if ( m_pItem != nullptr )
	{
		if ( m_flagsCtrlBehavior
			& (behaviorRenderEvent | behaviorRender | behaviorRenderContext) )
		{
			m_pItem->m_maskClasses |= m_flagsCtrlEventClasses ;
		}
		if ( m_flagsCtrlBehavior & behaviorOnTimer )
		{
			m_pItem->m_flagsBehavior |= S3DScene::itemTimer ;
		}
	}
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::ItemCommonSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneCommon" ;
}



//////////////////////////////////////////////////////////////////////////////
// アイテム・シリアライザ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSceneComposer::ItemBasicSerializer, Item, ItemCommonSerializer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemBasicSerializer::ItemBasicSerializer
		( const wchar_t * pwszClassID, const ParamSetClass * pClass )
	: ItemCommonSerializer( pwszClassID, pClass, nullptr )
{
	AttachSceneItem( (S3DScene::Item*) this ) ;
}

// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemBasicSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	Item::OnTimer( scene, msecPast ) ;
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;
}

// アイテム空間変換行列（アイテム自身を含まないグローバル変換）を計算
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemBasicSerializer::GetItemLinkTransformation
		( S3DDMatrix& matLink, S3DDVector& vLinkPos ) const
{
	Item::CalcItemLinkTransformation( matLink, vLinkPos ) ;
}

// 空間行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ItemBasicSerializer::GetGlobalTransformation
	( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const
{
	Item::CalcGlobalTransformation( matGlobal, vGlobalPos ) ;
}

// シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DSceneComposer::ItemBasicSerializer::GetScene( void ) const
{
	return	ItemCommonSerializer::GetScene() ;
}



//////////////////////////////////////////////////////////////////////////////
// 光源アイテム・シリアライザ
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSceneComposer::LightSerializer::m_paramEntries
		[S3DSceneComposer::LightSerializer::paramLightCount] =
{
	{ L"light_type",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrDynamicValidation,
		L"光源タイプ", nullptr },
	{ L"light_color",
		S3DSceneComposer::typeColor,
		S3DSceneComposer::attrCategory1,
		L"光源色", nullptr },
	{ L"light_brightness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"光源輝度", nullptr, 0.0, 1.0 },
	{ L"light_direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,
		L"光源向き", nullptr },
	{ L"light_attenuation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"距離減衰次元", nullptr },
	{ L"light_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"スポット範囲角",
		L"スポットライトの範囲角[deg]", 0.0, 180.0 },
	{ L"light_gradation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"スポットぼかし角",
		L"スポットライトのぼかし幅[deg]", 0.0, 180.0 },
	{ L"light_shadowmap",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,
		L"シャドウマッピング有効", nullptr },
	{ L"shadowmap_width",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant2,
		L"フレーム幅", nullptr },
	{ L"shadowmap_height",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant2,
		L"フレーム高", nullptr },
	{ L"shadowmap_pixel_density",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"ピクセル密度", L"投影空間距離／ピクセル（シャドウマップ１ピクセルのサイズ）" },
	{ L"shadowmap_distance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"投影距離", nullptr },
	{ L"shadowmap_adj_by_camera",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant2,
		L"適応投影有効", L"カメラ注視点距離に応じて投影距離を変化させる" },
	{ L"shadowmap_camera_std_distance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"適応投影基準距離",
		L"適応投影時の基準となるカメラ注視点距離（この距離の場合、投影距離、最小ｚ、最大ｚが1.0倍となる）" },
	{ L"shadowmap_pers_near",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"投影最小ｚ", nullptr },
	{ L"shadowmap_pers_far",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"投影最大ｚ", nullptr },
	{ L"shadowmap_error_precision",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"誤差精度", nullptr },
	{ L"shadowmap_error_sub_precision",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"誤差精度２", nullptr },
	{ L"shadowmap_filter_gauss",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"ガウス", L"深度を処理するガウス係数" },
	{ L"shadowmap_cascade_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant2,
		L"カスケード最大数",
		L"追加的に行うシャドウマッピングの数（実際に使用されるシャドウマッピングの数はこの数＋１となる）" },
	{ L"shadowmap_cascade_ratio",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"カスケード比", L"カスケードシャドウマッピングの2段目以降のピクセル密度に対する比率" },
	{ L"shadowmap_cascade_length",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"カスケード距離比",
		L"カスケードシャドウマッピングの2段目以降の注視点距離の1段目との比" },
	{ L"shadowmap_cascade_length_offset",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"カスケード距離オフセット",
		L"カスケードシャドウマッピングの2段目以降の注視点距離オフセット（カメラ注視点までとの距離によらない）" },
	{ L"shadowmap_cascade_depth",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"カスケード投影深度比",
		L"カスケードシャドウマッピングの2段目以降の投影深度の比率" },
	{ L"shadowmap_cascade_distance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant2,
		L"カスケード投影距離オフセット",
		L"カスケードシャドウマッピングの2段目以降の投影距離のオフセット" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DSceneComposer::LightSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemCommonSerializer::m_pscClass,
	S3DSceneComposer::LightSerializer::paramLightCount,
	&S3DSceneComposer::LightSerializer::m_paramEntries[0]
} ;

const wchar_t *	S3DSceneComposer::LightSerializer::m_pwszLightTypeIDs
						[S3DSceneComposer::LightSerializer::typeLightCount] =
{
	L"ambient", L"ambient_mul", L"vector", L"point", L"spot", L"fog",
} ;

const uint32_t	S3DSceneComposer::LightSerializer::m_nLightTypeFlag
					[S3DSceneComposer::LightSerializer::typeLightCount] =
{
	lightTypeAmbient, lightTypeAmbientMul, lightTypeVector,
	lightTypePoint, lightTypeSpot, lightTypeFog,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSceneComposer::LightSerializer, Light, ItemCommonSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DSceneComposer::LightSerializer, light )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::LightSerializer::LightSerializer( void )
	: ItemCommonSerializer
		( m_ItemClassDescriptor.pwszClassID,
			&S3DSceneComposer::LightSerializer::m_pscClass, nullptr )
{
	AttachSceneItem( (S3DScene::Light*) this ) ;
	//
	m_typeLight = typeLightVector ;
	m_rgbColor = 0xFFFFFF ;
	m_flagShadowmapping = false ;
	m_degLightAngle = 30.0 ;
	m_degLightGradation = 30.0 ;
	//
	m_smpShadowMap.nFlags |= S3DScene::shadowPersDistance ;
}

// 光源タイプ
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::LightSerializer::LightTypeIndex
		S3DSceneComposer::LightSerializer::GetLightType( void ) const
{
	return	(LightTypeIndex) m_typeLight ;
}

void S3DSceneComposer::LightSerializer::SetLightType
		( S3DSceneComposer::LightSerializer::LightTypeIndex type )
{
	m_typeLight = type ;
	m_light.typeLight = m_nLightTypeFlag[m_typeLight] ;
	if ( m_flagShadowmapping )
	{
		m_light.typeLight |= lightShadowMapping ;
	}
}

// 光源色
//////////////////////////////////////////////////////////////////////////////
SGLPalette S3DSceneComposer::LightSerializer::GetLightColor( void ) const
{
	return	m_rgbColor ;
}

void S3DSceneComposer::LightSerializer::SetLightColor( const SGLPalette& rgbColor )
{
	m_rgbColor = rgbColor ;
	if ( m_typeLight == typeLightAmbient )
	{
		m_light.rgbColor =
			rgbColor * esl_fmin( esl_fmax( m_light.fpBrightness, 0.0 ), 1.0 ) ;
	}
	else
	{
		m_light.rgbColor = rgbColor ;
	}
}

// 輝度
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::LightSerializer::GetLightBrightness( void ) const
{
	return	m_light.fpBrightness ;
}

void S3DSceneComposer::LightSerializer::SetLightBrightness( double fpBrightness )
{
	m_light.fpBrightness = (float32_t) fpBrightness ;
	//
	if ( m_typeLight == typeLightAmbient )
	{
		m_light.rgbColor =
			m_rgbColor * esl_fmin( esl_fmax( m_light.fpBrightness, 0.0 ), 1.0 ) ;
	}
}

// 点光源減衰力 x : (1/r-x)
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::LightSerializer::GetAttenuationPower( void ) const
{
	return	m_light.fpAttenuationPower ;
}

void S3DSceneComposer::LightSerializer::SetAttenuationPower( double fpAttenuation )
{
	m_light.fpAttenuationPower = (float32_t) fpAttenuation ;
}

// 光源位置
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DSceneComposer::LightSerializer::GetLightPosition( void ) const
{
	return	m_space.m_vCenter ;
}

void S3DSceneComposer::LightSerializer::SetLightPosition( const S3DDVector& vPos )
{
	m_space.m_vCenter = vPos ;
}

// 光源向き
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DSceneComposer::LightSerializer::GetLightDirection( void ) const
{
	return	m_light.vecDirection ;
}

void S3DSceneComposer::LightSerializer::SetLightDirection( const S3DVector& vDir )
{
	m_light.vecDirection = vDir ;
}

// スポットライト範囲角 [deg]
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::LightSerializer::GetLightAngle( void ) const
{
	return	m_degLightAngle ;
}

void S3DSceneComposer::LightSerializer::SetLightAngle( double degAngle )
{
	double	g ;
	m_degLightAngle = degAngle ;
	g = esl_fmax( m_degLightAngle - m_degLightGradation, 0.0 ) ;
	m_light.fpAngle = (float32_t) cos( degAngle * PI / 180.0 ) ;
	m_light.fpGradation =
		(float32_t) (cos( g * PI / 180.0 )
						- cos(m_degLightAngle * PI / 180)) ;
}

// スポットライトぼかし角 [deg]
//////////////////////////////////////////////////////////////////////////////
double S3DSceneComposer::LightSerializer::GetLightGradation( void ) const
{
	return	m_degLightGradation ;
}

void S3DSceneComposer::LightSerializer::SetLightGradation( double degGradation )
{
	double	g ;
	m_degLightGradation = degGradation ;
	g = esl_fmax( m_degLightAngle - m_degLightGradation, 0.0 ) ;
	m_light.fpGradation =
		(float32_t) (cos( g * PI / 180.0 )
						- cos(m_degLightAngle * PI / 180)) ;
}

// シャドウマッピング有効
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::LightSerializer::IsEnabledShadowMapping( void ) const
{
	return	m_flagShadowmapping ;
}

void S3DSceneComposer::LightSerializer::EnableShadowMapping( bool fShadowmap )
{
	m_flagShadowmapping = fShadowmap ;
	m_light.typeLight = m_nLightTypeFlag[m_typeLight] ;
	if ( m_flagShadowmapping )
	{
		m_light.typeLight |= lightShadowMapping ;
	}
}

// シャドウマッピングパラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::LightSerializer::GetShadowMappingParam
	( S3DScene::ShadowMapParam& param ) const
{
	param = m_smpShadowMap ;
}

void S3DSceneComposer::LightSerializer::SetShadowMappingParam
	( const S3DScene::ShadowMapParam& param )
{
	m_smpShadowMap = param ;
	m_smpShadowMap.nFlags |= S3DScene::shadowPersDistance ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::LightSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
	case	paramZoom:
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramItemClass:
		return	false ;
	case	paramLightBrightness:
		return	(m_typeLight == typeLightAmbient)
				|| (m_typeLight == typeLightVector)
				|| (m_typeLight == typeLightPoint)
				|| (m_typeLight == typeLightSpot)
				|| (m_typeLight == typeLightFog) ;
	case	paramLightDirection:
		return	(m_typeLight == typeLightVector)
				|| (m_typeLight == typeLightSpot)
				|| (m_typeLight == typeLightFog) ;
	case	paramLightAttenuationPower:
		return	(m_typeLight == typeLightPoint)
				|| (m_typeLight == typeLightSpot) ;
	case	paramLightAngle:
	case	paramLightGradation:
		return	(m_typeLight == typeLightSpot) ;
	case	paramLightShadowMapping:
	case	paramShadowMapWidth:
	case	paramShadowMapHeight:
	case	paramShadowPersNear:
	case	paramShadowPersFar:
	case	paramShadowErrorPrecision:
	case	paramShadowErrorSubPrecision:
	case	paramShadowFilterGauss:
	case	paramShadowCascadeCount:
		return	(m_typeLight == typeLightVector)
				|| (m_typeLight == typeLightPoint)
				|| (m_typeLight == typeLightSpot) ;
	case	paramShadowPixelDensity:
	case	paramShadowPersDistance:
	case	paramShadowAdjustByCamera:
	case	paramShadowCameraStdDistance:
	case	paramShadowCascadeDensity:
	case	paramShadowCascadeTargetLengthRatio:
	case	paramShadowCascadeTargetLengthOffset:
	case	paramShadowCascadeDepth:
	case	paramShadowCascadeDistanceOffsetStep:
		return	(m_typeLight == typeLightVector) ;
	}
	return	ItemCommonSerializer::IsParameterValidation( i ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSceneComposer::LightSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightColor:
		return	VectorFromColor( GetLightColor() ) ;
	case	paramLightDirection:
		return	S3DDVector( m_light.vecDirection ) ;
	}
	return	ItemCommonSerializer::GetVectorParameter( i ) ;
}

double S3DSceneComposer::LightSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightBrightness:
		return	GetLightBrightness() ;
	case	paramLightAttenuationPower:
		return	m_light.fpAttenuationPower ;
	case	paramLightAngle:
		return	m_degLightAngle ;
	case	paramLightGradation:
		return	m_degLightGradation ;
	case	paramShadowPixelDensity:
		return	m_smpShadowMap.fpPixelDensity ;
	case	paramShadowPersDistance:
		return	m_smpShadowMap.zPersDistance ;
	case	paramShadowCameraStdDistance:
		return	m_smpShadowMap.zCameraStdDistance ;
	case	paramShadowPersNear:
		return	m_smpShadowMap.zPersNear ;
	case	paramShadowPersFar:
		return	m_smpShadowMap.zPersFar ;
	case	paramShadowErrorPrecision:
		return	m_smpShadowMap.zErrorPrecision ;
	case	paramShadowErrorSubPrecision:
		return	m_smpShadowMap.zErrorSubPrecision ;
	case	paramShadowFilterGauss:
		return	m_smpShadowMap.fpFilterGauss ;
	case	paramShadowCascadeDensity:
		return	m_smpShadowMap.zCascadeDensityStep ;
	case	paramShadowCascadeTargetLengthRatio:
		return	m_smpShadowMap.zCascadeTargetLengthStep ;
	case	paramShadowCascadeTargetLengthOffset:
		return	m_smpShadowMap.zCascadeTargetLengthOffset ;
	case	paramShadowCascadeDepth:
		return	m_smpShadowMap.zCascadeDepthStep ;
	case	paramShadowCascadeDistanceOffsetStep:
		return	m_smpShadowMap.zCascadeDistanceOffsetStep ;
	}
	return	ItemCommonSerializer::GetScalarParameter( i ) ;
}

int32_t S3DSceneComposer::LightSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightType:
		return	GetLightType() ;
	case	paramShadowMapWidth:
		return	m_smpShadowMap.sizeMapping.w ;
	case	paramShadowMapHeight:
		return	m_smpShadowMap.sizeMapping.h ;
	case	paramShadowCascadeCount:
		return	m_smpShadowMap.nCascadeMaxCount ;
	}
	return	ItemCommonSerializer::GetIntegerParameter( i ) ;
}

bool S3DSceneComposer::LightSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightShadowMapping:
		return	m_flagShadowmapping ;
	case	paramShadowAdjustByCamera:
		return	(m_smpShadowMap.nFlags & S3DScene::shadowAdjustByCamera) != 0 ;
	}
	return	ItemCommonSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DSceneComposer::LightSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightType:
		return	m_pwszLightTypeIDs[m_typeLight] ;
	}
	return	ItemCommonSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::LightSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramLightColor:
		SetLightColor( ColorFromVector( vec ) ) ;
		return ;
	case	paramLightDirection:
		m_light.vecDirection = vec ;
		return ;
	}
	ItemCommonSerializer::SetVectorParameter( i, vec ) ;
}

void S3DSceneComposer::LightSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramLightBrightness:
		SetLightBrightness( s ) ;
		return ;
	case	paramLightAttenuationPower:
		m_light.fpAttenuationPower = (float32_t) s ;
		return ;
	case	paramLightAngle:
		SetLightAngle( s ) ;
		return ;
	case	paramLightGradation:
		SetLightGradation( s ) ;
		return ;
	case	paramShadowPixelDensity:
		m_smpShadowMap.fpPixelDensity = (float32_t) s ;
		return ;
	case	paramShadowPersDistance:
		m_smpShadowMap.zPersDistance = (float32_t) s ;
		return ;
	case	paramShadowCameraStdDistance:
		m_smpShadowMap.zCameraStdDistance = (float32_t) s ;
		return ;
	case	paramShadowPersNear:
		m_smpShadowMap.zPersNear = (float32_t) s ;
		return ;
	case	paramShadowPersFar:
		m_smpShadowMap.zPersFar = (float32_t) s ;
		return ;
	case	paramShadowErrorPrecision:
		m_smpShadowMap.zErrorPrecision = (float32_t) s ;
		return ;
	case	paramShadowErrorSubPrecision:
		m_smpShadowMap.zErrorSubPrecision = (float32_t) s ;
		return ;
	case	paramShadowFilterGauss:
		m_smpShadowMap.fpFilterGauss = (float32_t) s ;
		return ;
	case	paramShadowCascadeDensity:
		m_smpShadowMap.zCascadeDensityStep = (float32_t) s ;
		return ;
	case	paramShadowCascadeTargetLengthRatio:
		m_smpShadowMap.zCascadeTargetLengthStep = (float32_t) s ;
		return ;
	case	paramShadowCascadeTargetLengthOffset:
		m_smpShadowMap.zCascadeTargetLengthOffset = (float32_t) s ;
		return ;
	case	paramShadowCascadeDepth:
		m_smpShadowMap.zCascadeDepthStep = (float32_t) s ;
		return ;
	case	paramShadowCascadeDistanceOffsetStep:
		m_smpShadowMap.zCascadeDistanceOffsetStep = (float32_t) s ;
		return ;
	}
	ItemCommonSerializer::SetScalarParameter( i, s ) ;
}

void S3DSceneComposer::LightSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramLightType:
		SetLightType( (LightTypeIndex) n ) ;
		return ;
	case	paramShadowMapWidth:
		m_smpShadowMap.sizeMapping.w = n ;
		return ;
	case	paramShadowMapHeight:
		m_smpShadowMap.sizeMapping.h = n ;
		return ;
	case	paramShadowCascadeCount:
		m_smpShadowMap.nCascadeMaxCount = n ;
		return ;
	}
	return	ItemCommonSerializer::SetIntegerParameter( i, n ) ;
}

void S3DSceneComposer::LightSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramLightShadowMapping:
		EnableShadowMapping( b ) ;
		return ;
	case	paramShadowAdjustByCamera:
		m_smpShadowMap.nFlags =
			S3DScene::shadowPersDistance
				| (b ? S3DScene::shadowAdjustByCamera : 0) ;
		return ;
	}
	ItemCommonSerializer::SetBooleanParameter( i, b ) ;
}

void S3DSceneComposer::LightSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	size_t	j ;
	switch ( i )
	{
	case	paramLightType:
		for ( j = 0; j < typeLightCount; j ++ )
		{
			if ( SString::Compare( pwszCmd, m_pwszLightTypeIDs[j] ) == 0 )
			{
				m_typeLight = (int) j ;
				break ;
			}
		}
		m_light.typeLight = m_nLightTypeFlag[m_typeLight] ;
		if ( m_flagShadowmapping )
		{
			m_light.typeLight |= lightShadowMapping ;
		}
		return ;
	}
	ItemCommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::LightSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramLightType:
		for ( j = 0; j < typeLightCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszLightTypeIDs[j] ) ) ;
		}
		return	true ;
	}
	return	ItemCommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::LightSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"光源設定" ;
	case	2:
		return	L"シャドウマッピング設定" ;
	}
	return	ItemCommonSerializer::GetParameterCategoryName( iCategory ) ;
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::LightSerializer::OnPrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	ItemCommonSerializer::OnPrepareToRender( pDevice, nFlags ) ;

	if ( m_light.typeLight & lightShadowMapping )
	{
		const S3DScene::ShadowMapParam &	smp = m_smpShadowMap ;
		size_t	nCascadeMaxCount = smp.nCascadeMaxCount + 1 ;
		if ( nCascadeMaxCount > Light::maxCascadeShadowmap )
		{
			nCascadeMaxCount = Light::maxCascadeShadowmap ;
		}
		m_imgShadowColor.CreateImage
			( smp.sizeMapping.w, smp.sizeMapping.h,
				formatImageDefaultRGBA, 32,
				SGLImageObject::bufferForRenderTarget
					| SGLImageObject::bufferTextureArray
					| SGLImageObject::bufferOnDeviceOnly,
				nCascadeMaxCount ) ;
		m_imgShadowDepth.CreateImage
			( smp.sizeMapping.w, smp.sizeMapping.h,
				formatImageDepth, 32,
				SGLImageObject::bufferForRenderTarget
					| SGLImageObject::bufferTextureArray
					| SGLImageObject::bufferOnDeviceOnly,
				nCascadeMaxCount ) ;
		m_imgShadowColor.SetImageIdentity( L"shadow map color" ) ;
		m_imgShadowDepth.SetImageIdentity( L"shadow map depth" ) ;
		//
		pDevice->CommitDeviceImage( &(m_imgShadowColor), 1 ) ;
		pDevice->CommitDeviceImage( &(m_imgShadowDepth), 1 ) ;
	}
}

// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::LightSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	Light::OnTimer( scene, msecPast ) ;
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::LightSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneLight" ;
}



//////////////////////////////////////////////////////////////////////////////
// カメラアイテム・シリアライザ
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSceneComposer::CameraSerializer::m_paramEntries
		[S3DSceneComposer::CameraSerializer::paramCameraCount] =
{
	{ L"camera_target",
		S3DSceneComposer::typePosition,
		S3DSceneComposer::attrNoLocalTransform,
		L"注視点", nullptr },
	{ L"camera_top",
		S3DSceneComposer::typeDirection, 0,
		L"上基底", nullptr },
	{ L"camera_hfov",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrUIScalarSlider,
		L"水平視野角", L"カメラ水平視野角[deg]", 0.0, 170.0 },
	{ L"force_camera_fov",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant,
		L"視野角使用", nullptr },
	{ L"camera_left_ear",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrConstant1,
		L"左耳向き", nullptr },
	{ L"camera_right_ear",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrConstant1,
		L"右耳向き", nullptr },
	{ L"camera_ear_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"可聴角", L"可聴範囲角[deg]", 0.0, 180.0 },
	{ L"camera_back_volume",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"背後音量", L"範囲外音量", 0.0, 1.0 },
} ;

const S3DSceneComposer::ParamSetClass
	S3DSceneComposer::CameraSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemCommonSerializer::m_pscClass,
	S3DSceneComposer::CameraSerializer::paramCameraCount,
	&S3DSceneComposer::CameraSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSceneComposer::CameraSerializer, Camera, ItemCommonSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DSceneComposer::CameraSerializer, camera )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::CameraSerializer::CameraSerializer( void )
	: ItemCommonSerializer
		( m_ItemClassDescriptor.pwszClassID,
			&S3DSceneComposer::CameraSerializer::m_pscClass, nullptr )
{
	AttachSceneItem( (S3DScene::Camera*) this ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::CameraSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
	case	paramZoom:
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramVisible:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramCameraShift:
	case	paramCameraSpace:
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
		return	false ;
	}
	return	ItemCommonSerializer::IsParameterValidation( i ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSceneComposer::CameraSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
		return	GetCameraPosition() ;
	case	paramCameraTarget:
		return	GetCameraTarget() ;
	case	paramCameraTop:
		return	GetCameraTop() ;
	case	paramCameraLeftEar:
		return	m_vLeftEarDir ;
	case	paramCameraRightEar:
		return	m_vRightEarDir ;
	}
	return	ItemCommonSerializer::GetVectorParameter( i ) ;
}

double S3DSceneComposer::CameraSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCameraHFOV:
		return	m_degHorzFOV ;
	case	paramCameraEarAngle:
		return	acos( m_cosSoundLow ) * 180.0 / PI ;
	case	paramCameraBackVolume:
		return	m_fpLowVolume ;
	}
	return	ItemCommonSerializer::GetScalarParameter( i ) ;
}

bool S3DSceneComposer::CameraSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCameraForceHFOV:
		return	(m_nCameraFlags & cameraForceHFOV) != 0 ;
	}
	return	ItemCommonSerializer::GetBooleanParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CameraSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramPosition:
		SetCameraPosition( vec ) ;
		return ;
	case	paramCameraTarget:
		SetCameraTarget( vec ) ;
		return ;
	case	paramCameraTop:
		SetCameraTop( vec ) ;
		return ;
	case	paramCameraLeftEar:
		m_vLeftEarDir = vec ;
		return ;
	case	paramCameraRightEar:
		m_vRightEarDir = vec ;
		return ;
	}
	ItemCommonSerializer::SetVectorParameter( i, vec ) ;
}

void S3DSceneComposer::CameraSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramCameraHFOV:
		m_degHorzFOV = s ;
		return ;
	case	paramCameraEarAngle:
		m_cosSoundLow = cos( s * PI / 180.0 ) ;
		return ;
	case	paramCameraBackVolume:
		m_fpLowVolume = s ;
		return ;
	}
	ItemCommonSerializer::SetScalarParameter( i, s ) ;
}

void S3DSceneComposer::CameraSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramCameraForceHFOV:
		m_nCameraFlags = (m_nCameraFlags & ~cameraForceHFOV)
										| (b ? cameraForceHFOV : 0) ;
		return ;
	}
	ItemCommonSerializer::SetBooleanParameter( i, b ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::CameraSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"カメラ設定" ;
	case	1:
		return	L"音声カメラ設定" ;
	}
	return	ItemCommonSerializer::GetParameterCategoryName( iCategory ) ;
}

// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::CameraSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	Camera::OnTimer( scene, msecPast ) ;
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::CameraSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneCamera" ;
}



//////////////////////////////////////////////////////////////////////////////
// モデルアイテム・シリアライザ
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSceneComposer::ModelSerializer::m_paramEntries
			[S3DSceneComposer::ModelSerializer::paramModelCount] =
{
	{ L"model",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration, L"モデル", nullptr },
	{ L"collider_model",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration, L"コリジョン", nullptr },
	{ L"collider_flag",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant
		| S3DSceneComposer::attrFlagSetInteger,
		L"衝突フラグ",
		L"当たり判定の対象を判別するためのビット集合を指定する。\n"
		L"システム既定値として 0x01 が形状、0x02 が移動障壁として定義済み。\n"
		L"0x04 は攻撃当たり判定として、0x08 はイベント発生判定として推奨。\n"
		L"0x10～0x80 は未定義の予約領域で、0x0100 以上がユーザー領域である。" },
} ;

const S3DSceneComposer::ParamSetClass	S3DSceneComposer::ModelSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemCommonSerializer::m_pscClass,
	S3DSceneComposer::ModelSerializer::paramModelCount,
	&S3DSceneComposer::ModelSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DSceneComposer::ModelSerializer, ModelItem, ItemCommonSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DSceneComposer::ModelSerializer, model )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ModelSerializer::ModelSerializer( void )
	: ItemCommonSerializer
		( m_ItemClassDescriptor.pwszClassID,
			&S3DSceneComposer::ModelSerializer::m_pscClass, nullptr )
{
	AttachSceneItem( (S3DScene::ModelItem*) this ) ;
}

// モデル
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ModelSerializer::SetModel( const wchar_t * pwszModelID )
{
	if ( m_strModelID != pwszModelID )
	{
		m_strModelID = pwszModelID ;
		UpdateModel() ;
	}
}

void S3DSceneComposer::ModelSerializer::UpdateModel( void )
{
	if ( m_strModelID.IsEmpty() )
	{
		AttachModel( nullptr ) ;
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
		if ( pSceneComp != nullptr )
		{
			AttachModel( pSceneComp->Assets().GetModelAs( m_strModelID ) ) ;
		}
	}
}

const wchar_t * S3DSceneComposer::ModelSerializer::GetModelID( void ) const
{
	return	m_strModelID ;
}

// 衝突判定モデル
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ModelSerializer::SetCollision( const wchar_t * pwszModelID )
{
	if ( m_strCollisionID != pwszModelID )
	{
		m_strCollisionID = pwszModelID ;
		UpdateCollision() ;
	}
}

void S3DSceneComposer::ModelSerializer::UpdateCollision( void )
{
	if ( m_strCollisionID.IsEmpty() )
	{
		AttachCollisionModel( nullptr ) ;
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
		if ( pSceneComp != nullptr )
		{
			AttachCollisionModel
				( pSceneComp->Assets().GetModelAs( m_strCollisionID ) ) ;
		}
	}
}

const wchar_t * S3DSceneComposer::ModelSerializer::GetCollisionID( void ) const
{
	return	m_strCollisionID ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ModelSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramForceToon:
	case	paramForceBorder:
		return	true ;
	}
	return	ItemCommonSerializer::IsParameterValidation( i ) ;
}

// ポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose *
	S3DSceneComposer::ModelSerializer::GetPoseIdentityAs( const wchar_t * pwszPoseID ) const
{
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != nullptr )
	{
		S3DModelPose *	pPose = pModel->GetPoseLibrary().GetPoseAs( pwszPoseID ) ;
		if ( pPose != nullptr )
		{
			return	pPose ;
		}
	}
	return	ItemCommonSerializer::GetPoseIdentityAs( pwszPoseID ) ;
}

// ポーズID取得
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ModelSerializer::GetPoseIdentityOf
	( SSystem::SString& strPoseID, S3DModelPose * pPose ) const
{
	if ( pPose == nullptr )
	{
		return	false ;
	}
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != nullptr )
	{
		const wchar_t *	pwszID =
			pModel->GetPoseLibrary().GetPoseIdentityOf( pPose ) ;
		if ( pwszID != nullptr )
		{
			strPoseID = pwszID ;
			return	true ;
		}
	}
	return	ItemCommonSerializer::GetPoseIdentityOf( strPoseID, pPose ) ;
}

// アイテムのプライマリモデル取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface *
	S3DSceneComposer::ModelSerializer::GetItemPrimaryModel( void )
{
	return	m_pModel ;
}

// アイテムのコリジョンバッファ取得
//////////////////////////////////////////////////////////////////////////////
S3DCollider * S3DSceneComposer::ModelSerializer::GetItemPrimaryCollider( void )
{
	if ( m_flagCollision )
	{
		return	&m_collisionMesh ;
	}
	return	nullptr ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DSceneComposer::ModelSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColliderFlags:
		return	(int32_t) GetColliderUserFlags() ;
	}
	return	ItemCommonSerializer::GetIntegerParameter( i ) ;
}

const wchar_t * S3DSceneComposer::ModelSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramModel:
		return	m_strModelID ;
	case	paramCollision:
		return	m_strCollisionID ;
	}
	return	ItemCommonSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ModelSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramColliderFlags:
		SetColliderUserFlags( (uint32_t) n ) ;
		return ;
	}
	ItemCommonSerializer::SetIntegerParameter( i, n ) ;
}

void S3DSceneComposer::ModelSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramModel:
		SetModel( pwszCmd ) ;
		return ;
	case	paramCollision:
		SetCollision( pwszCmd ) ;
		return ;
	}
	ItemCommonSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ModelSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramModel:
	case	paramCollision:
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != nullptr )
		{
			S3DSceneComposer *	pSceneComp = pComp->GetSceneComposer() ;
			if ( pSceneComp != nullptr )
			{
				pSceneComp->Assets().EnumerateResourceIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DModelBuffer) ) ;
			}
		}
		return	true ;
	}
	return	ItemCommonSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ModelSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ModelItem::OnTimer( scene, msecPast ) ;
	ItemCommonSerializer::OnTimer( scene, msecPast ) ;
}

// ポーズライブラリ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary * S3DSceneComposer::ModelSerializer::GetPoseLibraryChain( void )
{
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != nullptr )
	{
		return	&(pModel->GetPoseLibrary()) ;
	}
	return	ItemCommonSerializer::GetPoseLibraryChain() ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSceneComposer::ModelSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	if ( nFlags & updateRefResource )
	{
		UpdateModel() ;
		UpdateCollision() ;
	}
	uint32_t	nResFlags =
		ItemCommonSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	return	nResFlags ;
}

// モデルデータ関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ModelSerializer::AttachModel
	( S3DVertexBufferInterface * pModel )
{
	m_pModel = pModel ;
}

void S3DSceneComposer::ModelSerializer::AttachCollisionModel
	( S3DVertexBufferInterface * pColModel, bool fBuildCollision )
{
	m_pCollision = pColModel ;
	//
	if ( fBuildCollision && (pColModel != nullptr) )
	{
		BuildCollisionMesh() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// コライダー・コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ColliderController, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DSceneComposer::ColliderController, collider_controller )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ColliderController::ColliderController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID )
{
	m_flagsBehavior |= behaviorCollision ;
	//
	m_typeMarker = S3DModelData::MarkerInfo::typeInvalid ;
	m_maskCollider = 1 << S3DCollision::colliderHit ;
	//
	AddParameterEntry
		( L"marker_type", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"対象マーカー",
			L"コリジョン処理に追加するマーカーの種別を指定します" ) ;
	AddParameterEntry
		( L"marker_type", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant,
			L"対象マーカー・リードID",
			L"コリジョン処理に追加するマーカーの先頭IDを指定します。\n"
			L"空文字列は全マーカーが対象になります。" ) ;
	AddParameterEntry
		( L"collider_flags", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrFlagSetInteger,
			L"衝突フラグ",
			L"当たり判定の対象を判別するためのビット集合を指定する。\n"
			L"（※マーカーにコライダ指標が定義されている場合にはそちらを優先）\n"
			L"システム既定値として 0x01 が形状、0x02 が移動障壁として定義済み。\n"
			L"0x04 は当たり判定（敵）、0x08 は（敵）攻撃当たり判定、0x10 はイベント発生判定として推奨。\n"
			L"0x10～0x80 は未定義の予約領域で、0x0100 以上がユーザー領域である。" ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ColliderController::~ColliderController( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DSceneComposer::ColliderController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColliderMask:
		return	(int32_t) m_maskCollider ;
	}
	return	0 ;
}

const wchar_t * S3DSceneComposer::ColliderController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLeadMarkerID:
		return	m_strLeadMarkerID ;
	case	paramColliderMask:
		if ( m_typeMarker != S3DModelData::MarkerInfo::typeInvalid )
		{
			return	S3DModelData::MarkerInfo::m_pwszTypeTags[m_typeMarker] ;
		}
		return	L"every_marker" ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ColliderController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramColliderMask:
		m_maskCollider = (uint32_t) n ;
		return ;
	}
}

void S3DSceneComposer::ColliderController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramLeadMarkerID:
		m_strLeadMarkerID = pwszCmd ;
		return ;
	case	paramColliderMask:
		{
			m_typeMarker = S3DModelData::MarkerInfo::typeInvalid ;
			for ( int j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
			{
				if ( SString::Compare
					( pwszCmd, S3DModelData::MarkerInfo::m_pwszTypeTags[j] ) == 0 )
				{
					m_typeMarker = (S3DModelData::MarkerInfo::Type) j ;
					break ;
				}
			}
		}
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSceneComposer::ColliderController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramMarkerType:
		{
			for ( int j = 0; j < S3DModelData::MarkerInfo::typeCount; j ++ )
			{
				aStrSet.Add( new SString(S3DModelData::MarkerInfo::m_pwszTypeTags[j]) ) ;
			}
			aStrSet.Add( new SString(L"every_marker") ) ;
		}
		return	true ;
	}
	return	false ;
}

// 当たり判定追加
//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ColliderController::RenderCollision
	( const S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, S3DCollision& render )
{
	S3DModelBuffer *	pModel =
			ESLTypeCast<S3DModelBuffer>( pItem->GetItemPrimaryModel() ) ;
	if ( pModel != nullptr )
	{
		uint32_t	maskSave = render.GetUserClassesMask() ;
		render.SetUserClassesMask( m_maskCollider ) ;
		pModel->AddAllMarkerForCollision
			( render, m_typeMarker, m_strLeadMarkerID ) ;
		render.SetUserClassesMask( maskSave ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// リソース・プラグイン
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSceneComposer::ResourcePlugin, ESLObject )



//////////////////////////////////////////////////////////////////////////////
// シーン・コンポーザー
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::m_aiKeyFrameFlags[2] =
{
	{ L"corner", S3DSceneComposer::keyframeCorner },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneComposer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::S3DSceneComposer( S3DCompositionManager * pManager )
{
	m_pManager = pManager ;
	m_nRefCount = 1 ;
	m_assets.AttachSceneComposer( this ) ;
	m_pRsrcFileOpener = nullptr ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::~S3DSceneComposer( void )
{
	Release() ;
}

// マネージャ取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionManager * S3DSceneComposer::GetManager( void ) const
{
	return	m_pManager ;
}

// 参照カウンタ
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::AddReference( void )
{
	m_nRefCount ++ ;
}

bool S3DSceneComposer::ReleaseRef( void )
{
	return	((-- m_nRefCount) <= 0) ;
}

// ファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::LoadComposeFile( const wchar_t * pwszFileName )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile
				( pwszFileName, SFileOpener::shareRead ) ;
	if ( pFile == nullptr )
	{
		return	sglErrFailed ;
	}
	SString	strRsrcBaseDir = SString( pwszFileName ).GetFileDirectoryPart() ;
	if ( (strRsrcBaseDir.GetLastAt(0) == L'\\')
		|| (strRsrcBaseDir.GetLastAt(0) == L'/') )
	{
		strRsrcBaseDir.ChopRight( 1 ) ;
	}
	return	ReadComposeFile( *pFile, strRsrcBaseDir ) ;
}

SGLError S3DSceneComposer::ReadComposeFile
	( SSystem::SFileInterface& file, const wchar_t * pwszBaseDir )
{
	SGLError		err = sglErrFailed ;
	SFileOpener *	pLastOpener = m_pRsrcFileOpener ;
	//
	ERISA::SGLArchiveFile	arcfile ;
	int64_t	fpPos = file.GetPosition() ;
	if ( !arcfile.OpenArchive( &file ) )
	{
		if ( arcfile.DescendFile( L"__default.xmlprs" ) )
		{
			return	sglErrFailed ;
		}
		SParserErrorTracer	perr ;
		SXMLDocument	xmlDoc ;
		SError	errXML = xmlDoc.ReadDocument( arcfile, perr ) ;
		if ( errXML )
		{
			return	(SGLError) errXML ;
		}
		arcfile.AscendFile() ;
		//
		m_pRsrcFileOpener = &arcfile ;
		m_pManager->VM().AddScriptFileOpener( &arcfile ) ;
		//
		err = ParseComposeFile( xmlDoc, nullptr ) ;
		//
		m_pManager->VM().DetachScriptFileOpener( &arcfile ) ;
		arcfile.Close() ;
	}
	else
	{
		file.Seek( fpPos ) ;
		//
		SParserErrorTracer	perr ;
		SXMLDocument	xmlDoc ;
		SError	errXML = xmlDoc.ReadDocument( file, perr ) ;
		if ( errXML )
		{
			return	(SGLError) errXML ;
		}
		if ( m_pRsrcFileOpener == nullptr )
		{
			m_pRsrcFileOpener = &file ;
		}
		m_pManager->VM().AddScriptFileOpener( &file ) ;
		//
		err = ParseComposeFile( xmlDoc, pwszBaseDir ) ;
		//
		m_pManager->VM().DetachScriptFileOpener( &file ) ;
	}
	m_pRsrcFileOpener = pLastOpener ;
	return	err ;
}

SGLError S3DSceneComposer::ParseComposeFile
	( const SXMLDocument& xmlDoc, const wchar_t * pwszBaseDir )
{
	Release() ;
	//
	const SXMLDocument *	pxmlScene = nullptr ;
	if ( xmlDoc.GetType() == SXMLDocument::typeRoot )
	{
		pxmlScene = xmlDoc.GetElementTagAs( L"scene" ) ;
	}
	else if ( xmlDoc.GetTag() == L"scene" )
	{
		pxmlScene = &xmlDoc ;
	}
	if ( pxmlScene == nullptr )
	{
		ESLTrace( "not found <scene> tag.\n" ) ;
		return	sglErrFailed ;
	}
	m_strRsrcBaseDir =
		pxmlScene->GetAttrStringAs( L"assets_base_dir", m_strRsrcBaseDir ) ;
	if ( pwszBaseDir != nullptr )
	{
		m_strRsrcBaseDir =
			SString(pwszBaseDir).OffsetFilePath( m_strRsrcBaseDir ) ;
	}
	m_strRsrcBaseDir.NormalizeFilePath() ;
	//
	// インポート
	//
	SXMLDocument *	pxmlImports = pxmlScene->GetElementTagAs( L"imports" ) ;
	if ( pxmlImports != nullptr )
	{
		for ( size_t i = 0; i < pxmlImports->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlImport = pxmlImports->GetElementAt( i ) ;
			if ( (pxmlImport == nullptr)
				|| (pxmlImport->GetTag() != L"import") )
			{
				continue ;
			}
			SString	strSrcFile = pxmlImport->GetAttrStringAs( L"src" ) ;
			if ( strSrcFile.IsEmpty() )
			{
				continue ;
			}
			AddImportComposer( strSrcFile ) ;
		}
	}
	//
	// スタイル
	//
	SXMLDocument *	pxmlStyles = pxmlScene->GetElementTagAs( L"styles" ) ;
	if ( pxmlStyles != nullptr )
	{
		for ( size_t i = 0; i < pxmlStyles->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlStyle = pxmlStyles->GetElementAt( i ) ;
			if ( pxmlStyle == nullptr )
			{
				continue ;
			}
			const SString *	pstrStyleID =
						pxmlStyle->GetAttributeAs( L"style_id" ) ;
			if ( pstrStyleID != nullptr )
			{
				m_ssoaParts.SetAs
					( *pstrStyleID, new SXMLDocument(*pxmlStyle) ) ;
			}
		}
	}
	//
	// コンポジション
	//
	SXMLDocument *	pxmlComps = pxmlScene->GetElementTagAs( L"compositions" ) ;
	if ( pxmlComps != nullptr )
	{
		m_ssoaCompositions.RemoveAll() ;
		for ( size_t i = 0; i < pxmlComps->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlComp = pxmlComps->GetElementAt( i ) ;
			if ( (pxmlComp == nullptr)
				|| (pxmlComp->GetTag() != L"composition") )
			{
				continue ;
			}
			CompositionInfo *	pci = new CompositionInfo ;
			if ( !pci->ParseComposition( *pxmlComp ) )
			{
				SString	strID = pxmlComp->GetAttrStringAs( L"id" ) ;
				m_ssoaCompositions.Add( strID, pci ) ;
			}
			else
			{
				delete	pci ;
			}
		}
	}
	//
	// 資源
	//
	SXMLDocument *	pxmlAssets = pxmlScene->GetElementTagAs( L"assets" ) ;
	if ( pxmlAssets != nullptr )
	{
		ParseImportAssets( m_assets, *pxmlAssets, nullptr ) ;
	}
	return	sglErrSuccess ;
}

// ファイル書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::SaveComposeFile( const wchar_t * pwszFileName )
{
	SSmartPointer<SFileInterface>
		pFile = SFileOpener::DefaultNewOpenFile
					( pwszFileName, SFileOpener::modeCreate ) ;
	if ( pFile == nullptr )
	{
		return	sglErrFailed ;
	}
	SString	strRsrcBaseDir = SString( pwszFileName ).GetFileDirectoryPart() ;
	if ( (strRsrcBaseDir.GetLastAt(0) == L'\\')
		|| (strRsrcBaseDir.GetLastAt(0) == L'/') )
	{
		strRsrcBaseDir.ChopRight( 1 ) ;
	}
	strRsrcBaseDir.NormalizeFilePath() ;
	return	WriteComposeFile( *pFile, strRsrcBaseDir ) ;
}

SGLError S3DSceneComposer::ExportComposeFile( const wchar_t * pwszFileName )
{
	SXMLDocument	xmlDoc ;
	SStrSortArray<const ResourceContainer*>	ssaExports ;
	SGLError	err = FormatComposeFile( xmlDoc, nullptr, &ssaExports ) ;
	if ( err )
	{
		return	err ;
	}
	SSmartPointer<SFileInterface>
		pFile = SFileOpener::DefaultNewOpenFile
					( pwszFileName, SFileOpener::modeCreate ) ;
	if ( pFile == nullptr )
	{
		return	sglErrFailed ;
	}
	ERISA::SGLArchiveFile::SDirectory		dirRoot ;
	SStrSortArray<const ResourceContainer*>	ssaRootFiles ;
	//
	// __default.xmlprs エントリ作成
	//
	ERISA::SGLArchiveFile::FILE_ENTRY_EX	feEntry ;
	eslFillMemory( &feEntry, 0, sizeof(feEntry) ) ;
	feEntry.nEncodeType = ERISA::SGLArchiveFile::encodeRaw ;
	//
	DATE_TIME	dtFile ;
	CurrentLocalDate( dtFile ) ;
	feEntry.ftFileTime.nYear = dtFile.nYear ;
	feEntry.ftFileTime.nMonth = (uint8_t) dtFile.nMonth ;
	feEntry.ftFileTime.nDay = (uint8_t) dtFile.nDay ;
	feEntry.ftFileTime.nWeek = (uint8_t) dtFile.nWeek ;
	feEntry.ftFileTime.nHour = (uint8_t) dtFile.nHour ;
	feEntry.ftFileTime.nMinute = (uint8_t) dtFile.nMinute ;
	feEntry.ftFileTime.nSecond = (uint8_t) dtFile.nSecond ;
	//
	dirRoot.AddFileEntry( L"__default.xmlprs", feEntry ) ;
	//
	// それ以外のルートファイルの収集
	//
	CollectDirectory( dirRoot, ssaRootFiles, ssaExports, L"" ) ;
	//
	// NOA ファイルを作成開始
	//
	ERISA::SGLArchiveFile	arcfile ;
	if ( arcfile.OpenArchive( pFile, false, SFileOpener::modeCreate, &dirRoot ) )
	{
		return	sglErrFailed ;
	}
	//
	// __default.xmlprs 書き出し
	//
	if ( arcfile.DescendFile( L"__default.xmlprs" ) )
	{
		return	sglErrFailed ;
	}
	xmlDoc.WriteDocument( arcfile ) ;
	arcfile.AscendFile() ;
	//
	// ルートディレクトリから順に出力
	//
	WriteFileIntoArchive( arcfile, ssaRootFiles ) ;
	ExportSubDirectory( arcfile, ssaExports, L"" ) ;
	//
	return	sglErrSuccess ;
}

void S3DSceneComposer::ExportSubDirectory
	( ERISA::SGLArchiveFile& arcfile,
		SSystem::SStrSortArray<const ResourceContainer*>& ssaFiles,
		const wchar_t * pwszDirPath )
{
	SString	strDirPath = pwszDirPath ;
	for ( size_t i = 0; i < ssaFiles.GetLength(); i ++ )
	{
		const SString *				pstrPath = ssaFiles.GetTagAt( i ) ;
		const ResourceContainer**	ppRsrc = ssaFiles.GetAt( i ) ;
		if ( (pstrPath == nullptr) || (ppRsrc == nullptr) || (*ppRsrc == nullptr) )
		{
			continue ;
		}
		if ( pstrPath->CompareLeftNoCase( strDirPath ) != 0 )
		{
			continue ;
		}
		SString	strRelPath = pstrPath->Middle( strDirPath.GetLength() ) ;
		ssize_t	iDir = strRelPath.Find( L'\\' ) ;
		if ( iDir <= 0 )
		{
			continue ;
		}
		SString	strDirName = strRelPath.Left( (size_t) iDir ) ;
		SString	strSubDirPath = strDirPath + strDirName + L"\\" ;
		//
		ERISA::SGLArchiveFile::SDirectory		dirEntries ;
		SStrSortArray<const ResourceContainer*>	ssaSubDirFiles ;
		CollectDirectory( dirEntries, ssaSubDirFiles, ssaFiles, strSubDirPath ) ;
		if ( arcfile.DescendDirectory( strDirName, &dirEntries ) )
		{
			SString	strMsg ;
			strMsg.Format( L"ディレクトリへの書き出しに失敗しました \'%s\'",
										(const wchar_t*) strSubDirPath ) ;
			OutputError( strMsg ) ;
			continue ;
		}
		WriteFileIntoArchive( arcfile, ssaSubDirFiles ) ;
		ExportSubDirectory( arcfile, ssaFiles, strSubDirPath ) ;
		arcfile.AscendDirectory() ;
	}
}

void S3DSceneComposer::CollectDirectory
	( ERISA::SGLArchiveFile::SDirectory& dirEntries,
		SSystem::SStrSortArray<const ResourceContainer*>& ssaCollected,
		SSystem::SStrSortArray<const ResourceContainer*>& ssaFiles,
		const wchar_t * pwszFileDir )
{
	SString	strFileDir = pwszFileDir ;
	for ( size_t i = 0; i < ssaFiles.GetLength(); i ++ )
	{
		const SString *				pstrPath = ssaFiles.GetTagAt( i ) ;
		const ResourceContainer**	ppRsrc = ssaFiles.GetAt( i ) ;
		if ( (pstrPath == nullptr) || (ppRsrc == nullptr) || (*ppRsrc == nullptr) )
		{
			continue ;
		}
		if ( pstrPath->CompareLeftNoCase( strFileDir ) != 0 )
		{
			continue ;
		}
		ERISA::SGLArchiveFile::FILE_ENTRY_EX	feEntry ;
		eslFillMemory( &feEntry, 0, sizeof(feEntry) ) ;
		feEntry.nEncodeType = ERISA::SGLArchiveFile::encodeRaw ;
		//
		DATE_TIME	dtFile ;
		CurrentLocalDate( dtFile ) ;
		//
		SString	strRelPath = pstrPath->Middle( strFileDir.GetLength() ) ;
		ssize_t	iDir = strRelPath.Find( L'\\' ) ;
		if ( iDir == 0 )
		{
			continue ;
		}
		SString	strFileName = strRelPath ;
		if ( iDir >= 0 )
		{
			SString	strDirName = strRelPath.Left( (size_t) iDir ) ;
			if ( dirEntries.GetFileInfoAs( strDirName ) != nullptr )
			{
				continue ;
			}
			strFileName = strDirName ;
		}
		else
		{
			SString				strRsrcPath = (*ppRsrc)->GetSourceFile() ;
			SFileOpener::State	state ;
			if ( !SFile::QueryFileState( strRsrcPath, state ) )
			{
				if ( state.bitAttributes & SFileOpener::fieldFileSize )
				{
					feEntry.nBytes = state.nFileSize ;
				}
				if ( state.bitAttributes & SFileOpener::fieldModifiedTime )
				{
					dtFile = state.dtModified ;
				}
			}
			ssaCollected.Add( strRelPath, *ppRsrc ) ;
			ssaFiles.RemoveAt( i -- ) ;
		}
		feEntry.ftFileTime.nYear = dtFile.nYear ;
		feEntry.ftFileTime.nMonth = (uint8_t) dtFile.nMonth ;
		feEntry.ftFileTime.nDay = (uint8_t) dtFile.nDay ;
		feEntry.ftFileTime.nWeek = (uint8_t) dtFile.nWeek ;
		feEntry.ftFileTime.nHour = (uint8_t) dtFile.nHour ;
		feEntry.ftFileTime.nMinute = (uint8_t) dtFile.nMinute ;
		feEntry.ftFileTime.nSecond = (uint8_t) dtFile.nSecond ;
		//
		dirEntries.AddFileEntry( strFileName, feEntry ) ;
	}
}

void S3DSceneComposer::WriteFileIntoArchive
	( ERISA::SGLArchiveFile& arcfile,
		const SSystem::SStrSortArray<const ResourceContainer*>& ssaFiles )
{
	const size_t	nBufSize = 0x10000 ;
	SArray<uint8_t>	bufTemp ;
	uint8_t *		pbytBuf = bufTemp.GetArray( nBufSize ) ;
	//
	for ( size_t i = 0; i < ssaFiles.GetLength(); i ++ )
	{
		const SString *				pstrFileName = ssaFiles.GetTagAt( i ) ;
		const ResourceContainer**	ppRsrc = ssaFiles.GetAt( i ) ;
		if ( (pstrFileName == nullptr) || (ppRsrc == nullptr) || (*ppRsrc == nullptr) )
		{
			continue ;
		}
		SString	strRsrcPath = (*ppRsrc)->GetSourceFile() ;
		SFile	file ;
		if ( file.Open( strRsrcPath, SFileOpener::shareRead ) )
		{
			SString	strMsg ;
			strMsg.Format( L"ファイルを開けません \'%s\'", (const wchar_t*) strRsrcPath ) ;
			OutputError( strMsg ) ;
			continue ;
		}
		if ( arcfile.DescendFile( *pstrFileName ) )
		{
			SString	strMsg ;
			strMsg.Format( L"ファイルを書き込めません （ソース \'%s\'）", (const wchar_t*) strRsrcPath ) ;
			OutputError( strMsg ) ;
			continue ;
		}
		for ( ; ; )
		{
			size_t	nReadBytes = file.Read( pbytBuf, nBufSize ) ;
			if ( nReadBytes == 0 )
			{
				break ;
			}
			arcfile.Write( pbytBuf, nReadBytes ) ;
		}
		arcfile.AscendFile() ;
	}
	//
	bufTemp.FinishArray() ;
}

SGLError S3DSceneComposer::WriteComposeFile
	( SSystem::SFileInterface& file, const wchar_t * pwszBaseDir )
{
	SXMLDocument	xmlDoc ;
	SGLError	err = FormatComposeFile( xmlDoc, pwszBaseDir ) ;
	if ( err )
	{
		return	err ;
	}
	return	(SGLError) xmlDoc.WriteDocument( file ) ;
}

SGLError S3DSceneComposer::FormatComposeFile
	( SSystem::SXMLDocument& xmlDoc,
		const wchar_t * pwszBaseDir,
		SSystem::SStrSortArray<const ResourceContainer*> * pssaExport )
{
	xmlDoc.SetTag( L"scene" ) ;
	//
	if ( pssaExport == nullptr )
	{
		SString	strBaseDir = m_strRsrcBaseDir ;
		strBaseDir.NormalizeFilePath() ;
		if ( pwszBaseDir != nullptr )
		{
			strBaseDir = SString(pwszBaseDir).RelativeFilePath( strBaseDir ) ;
		}
		if ( !strBaseDir.IsEmpty() )
		{
			xmlDoc.SetAttributeAs( L"assets_base_dir", strBaseDir ) ;
		}
	}
	//
	// インポートファイル
	//
	SXMLDocument *	pxmlImports = xmlDoc.CreateElementTagAs( L"imports" ) ;
	//
	for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
	{
		ImportEntry *	pie = m_imports.GetAt( i ) ;
		ESLAssert( pie != nullptr ) ;
		//
		SString	strAbsPath = m_strRsrcBaseDir.OffsetFilePath( pie->GetSourceFile() ) ;
		SString	strImportPath =
				m_strRsrcBaseDir.RelativeFilePath( strAbsPath.NormalizeFilePath() ) ;
		//
		SXMLDocument *	pxmlImport = new SXMLDocument ;
		pxmlImport->SetTag( L"import" ) ;
		pxmlImport->SetAttributeAs( L"src", strImportPath ) ;
		pxmlImports->AddElement( pxmlImport ) ;
	}
	//
	// リソース
	//
	SXMLDocument *	pxmlAssets = xmlDoc.CreateElementTagAs( L"assets" ) ;
	//
	for ( size_t i = 0; i < m_assets.m_ssoaResources.GetLength(); i ++ )
	{
		const SString *	pstrID = m_assets.m_ssoaResources.GetTagAt( i ) ;
		ResourceContainer *	prc = m_assets.m_ssoaResources.GetAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		ESLAssert( prc != nullptr ) ;
		if ( (pstrID != nullptr)
			&& (prc != nullptr) && !prc->GetType().IsEmpty() )
		{
			SXMLDocument *	pxmlRsrc = new SXMLDocument ;
			prc->SaveProceduralOption() ;
			FormatResource( *pxmlRsrc, *pstrID, *prc, pssaExport ) ;
			pxmlAssets->AddElement( pxmlRsrc ) ;
		}
	}
	//
	// スタイル
	//
	SXMLDocument *	pxmlStyles = xmlDoc.CreateElementTagAs( L"styles" ) ;
	//
	for ( size_t i = 0; i < m_ssoaParts.GetLength(); i ++ )
	{
		const SString *	pstrID = m_ssoaParts.GetTagAt( i ) ;
		SXMLDocument *	pxmlStyle = m_ssoaParts.GetAt( i ) ;
		if ( pstrID && pxmlStyle )
		{
			pxmlStyle->SetAttributeAs( L"style_id", *pstrID ) ;
			pxmlStyles->AddElement( new SXMLDocument( *pxmlStyle ) ) ;
		}
	}
	//
	// コンポジション
	//
	SXMLDocument *	pxmlComps = xmlDoc.CreateElementTagAs( L"compositions" ) ;
	for ( size_t i = 0; i < m_ssoaCompositions.GetLength(); i ++ )
	{
		const SString *	pstrID = m_ssoaCompositions.GetTagAt( i ) ;
		CompositionInfo *	pci = m_ssoaCompositions.GetAt( i ) ;
		if ( pstrID && pci )
		{
			SXMLDocument *	pxmlComp = new SXMLDocument ;
			pci->FormatComposition( *pxmlComp ) ;
			//
			pxmlComp->SetTag( L"composition" ) ;
			pxmlComp->SetAttributeAs( L"id", *pstrID ) ;
			pxmlComps->AddElement( pxmlComp ) ;
		}
	}
	return	sglErrSuccess ;
}

// リソース解放
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::Release( void )
{
	m_assets.Release() ;
	m_ssoaParts.RemoveAll() ;
	m_ssoaCompositions.RemoveAll() ;
	m_imports.RemoveAll() ;
}

// アセットファイル読み込みファイルオープナー
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileOpener * S3DSceneComposer::GetResourceFileOpener( void ) const
{
	return	m_pRsrcFileOpener ;
}

void S3DSceneComposer::AttachResourceFileOpener( SSystem::SFileOpener * pOpener )
{
	m_pRsrcFileOpener = pOpener ;
}

// アセットファイル読み込みベースディレクトリ
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSceneComposer::GetResourceBaseDirectory( void ) const
{
	return	m_strRsrcBaseDir ;
}

void S3DSceneComposer::SetResourceBaseDirectory( const wchar_t * pwszBaseDir )
{
	m_strRsrcBaseDir = pwszBaseDir ;
}

// リソースをデバイス用にロードする
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::PrepareToRenderDevice
				( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	if ( nFlags & prepareImportAssets )
	{
		for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
		{
			ImportEntry *	pie = m_imports.GetAt( i ) ;
			if ( pie->GetComposer() != nullptr )
			{
				pie->GetComposer()->PrepareToRenderDevice( pDevice, nFlags ) ;
			}
		}
	}
	bool	flagMakeCompressedTexture = false ;
	if ( nFlags & prepareCompressedTexture )
	{
		S3DRenderDevice::Features	features ;
		if ( !pDevice->GetDeviceFeatures( features ) )
		{
			flagMakeCompressedTexture =
				((features.flagsFeatures[0]
					& S3DRenderDevice::feature0_CompressionS3TC) != 0) ;
		}
	}
	size_t	nCount = m_assets.GetResourceCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ResourceContainer *	prc = m_assets.GetResourceAt( i ) ;
		if ( prc == nullptr )
		{
			continue ;
		}
		if ( prc->IsResourceType( resourceTypeImage ) )
		{
			SGLImageObject *	pImage =
				ESLTypeCast<SGLImageObject>( prc->GetReference() ) ;
			if ( pImage != nullptr )
			{
				pDevice->CommitDeviceImage( pImage, 1 ) ;
			}
		}
		else if ( prc->IsResourceType( resourceTypeModel ) )
		{
			S3DModelBuffer *	pModel =
				ESLTypeCast<S3DModelBuffer>( prc->GetReference() ) ;
			if ( pModel != nullptr )
			{
				pModel->CommitToDevice( pDevice, 1 ) ;
			}
		}
		else if ( prc->IsResourceType( resourceTypeShaderDef ) )
		{
			UserShader *	pUserShader =
				ESLTypeCast<UserShader>( prc->GetReference() ) ;
			if ( pUserShader != nullptr )
			{
				pUserShader->LoadShaderFor( pDevice ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// デバイスリソースを解放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::UnprepareToRenderDevice
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	size_t	nCount = m_assets.GetResourceCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ResourceContainer *	prc = m_assets.GetResourceAt( i ) ;
		if ( prc == nullptr )
		{
			continue ;
		}
		if ( prc->IsResourceType( resourceTypeImage ) )
		{
			SGLImageObject *	pImage =
				ESLTypeCast<SGLImageObject>( prc->GetReference() ) ;
			if ( pImage != nullptr )
			{
				pDevice->ReleaseDeviceImage( pImage, 1 ) ;
			}
		}
		else if ( prc->IsResourceType( resourceTypeModel ) )
		{
			S3DModelBuffer *	pModel =
				ESLTypeCast<S3DModelBuffer>( prc->GetReference() ) ;
			if ( pModel != nullptr )
			{
				pModel->ReleaseForDevice( pDevice, 1 ) ;
			}
		}
		else if ( prc->IsResourceType( resourceTypeShaderDef ) )
		{
			UserShader *	pUserShader =
				ESLTypeCast<UserShader>( prc->GetReference() ) ;
			if ( pUserShader != nullptr )
			{
//				pUserShader->ReleaseAllShaderRef() ;
//				pDevice->RemoveCustomShaderAs( strRsrcID ) ;
			}
		}
	}
	if ( nFlags & unprepareImportAssets )
	{
		for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
		{
			ImportEntry *	pie = m_imports.GetAt( i ) ;
			if ( pie->GetComposer() != nullptr )
			{
				pie->GetComposer()->UnprepareToRenderDevice( pDevice, nFlags ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// コンポジション取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::CompositionInfo *
	S3DSceneComposer::GetCompositionAs
			( const wchar_t * pwszID, bool flagWithImport ) const
{
	S3DSceneComposer::CompositionInfo *
			pci = m_ssoaCompositions.GetAs( pwszID ) ;
	if ( (pci == nullptr) && flagWithImport )
	{
		for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
		{
			ImportEntry *	pie = m_imports.GetAt( i ) ;
			if ( pie == nullptr )
			{
				continue ;
			}
			S3DSceneComposer *	pCompoer = pie->GetComposer() ;
			if ( pCompoer != nullptr )
			{
				pci = pCompoer->GetCompositionAs( pwszID, false ) ;
				if ( pci != nullptr )
				{
					break ;
				}
			}
		}
	}
	return	pci ;
}

S3DSceneComposer::CompositionInfo *
	S3DSceneComposer::GetCompositionAt( size_t i ) const
{
	return	m_ssoaCompositions.GetAt( i ) ;
}

// コンポジション数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::GetCompositionCount( void ) const
{
	return	m_ssoaCompositions.GetLength() ;
}

// コンポジション名取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString * S3DSceneComposer::GetCompositionID( size_t i ) const
{
	return	m_ssoaCompositions.GetTagAt( i ) ;
}

// コンポジション検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DSceneComposer::FindCompositionOf
		( const S3DSceneComposer::CompositionInfo * pciComp ) const
{
	return	m_ssoaCompositions.FindPtr
				( (S3DSceneComposer::CompositionInfo*) pciComp ) ;
}

// コンポジション追加
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::AddCompositionAs
	( const wchar_t * pwszID, S3DSceneComposer::CompositionInfo * pciComp )
{
	m_ssoaCompositions.Add( pwszID, pciComp ) ;
}

// コンポジション削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::RemoveCompositionAs( const wchar_t * pwszID )
{
	m_ssoaCompositions.RemoveAs( pwszID ) ;
}

// コンポジションID列挙
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::EnumerateCompositionIDs
	( SObjectArray<SString>& aIDs, bool flagWithImport ) const
{
	for ( size_t i = 0; i < m_ssoaCompositions.GetLength(); i ++ )
	{
		const SString *	pstrTag = m_ssoaCompositions.GetTagAt( i ) ;
		if ( pstrTag != nullptr )
		{
			aIDs.Add( new SString( *pstrTag ) ) ;
		}
	}
	if ( flagWithImport )
	{
		for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
		{
			ImportEntry *	pie = m_imports.GetAt( i ) ;
			if ( pie == nullptr )
			{
				continue ;
			}
			S3DSceneComposer *	pCompoer = pie->GetComposer() ;
			if ( pCompoer != nullptr )
			{
				pCompoer->EnumerateCompositionIDs( aIDs, false ) ;
			}
		}
	}
}

// 表示用コンポジション生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Composition *
	S3DSceneComposer::CreateComposition
		( const S3DSceneComposer::CompositionInfo& ci,
					Composition * pOwner, bool flagAsSubComp )
{
	if ( pOwner != nullptr )
	{
		Composition *	pNextOwner = pOwner ;
		do
		{
			if ( pNextOwner->GetCompositionInfo() == &ci )
			{
				return	nullptr ;
			}
			pNextOwner = pNextOwner->GetOwnerComposition() ;
		}
		while ( pNextOwner != nullptr ) ;
	}
	Composition *	pComp = new Composition( this, &ci, pOwner, flagAsSubComp ) ;
	pComp->ParseItem
		( *this, *pComp, *pComp, ci.GetComposition() ) ;
	pComp->UpdatePropertyReference( *pComp, updateRefItem ) ;
	pComp->InitializeFrameParameters() ;
	return	pComp ;
}

// コンポーザー・インポート
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::AddImportComposer( const wchar_t * pwszFilePath )
{
	SString	strFullPath = m_strRsrcBaseDir.OffsetFilePath( pwszFilePath ) ;
	strFullPath.NormalizeFilePath() ;
	//
	S3DSceneComposer *	pComposer = nullptr ;
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( strFullPath ) ;
	if ( pFile != nullptr )
	{
		pComposer = m_pManager->LoadComposer( strFullPath, pFile ) ;
	}
	//
	ImportEntry *	pie = new ImportEntry( m_pManager, pComposer ) ;
	pie->SetSourceFile( strFullPath ) ;
	m_imports.Add( pie ) ;
	//
	if ( pComposer != nullptr )
	{
		m_assets.AddReferenceAssets( &(pComposer->m_assets) ) ;
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrFailed ;
	}
}

// 全インポート・コンポーザーを読み込み直す
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ReloadAllImportComposers( void )
{
	for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
	{
		ImportEntry *	pie = m_imports.GetAt( i ) ;
		if ( pie == nullptr )
		{
			continue ;
		}
		S3DSceneComposer *	pComposer = pie->GetComposer() ;
		if ( pComposer != nullptr )
		{
			m_assets.DetachReferenceAssets( &(pComposer->m_assets) ) ;
		}
		pie->UnloadComposer() ;
	}
	for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
	{
		ImportEntry *	pie = m_imports.GetAt( i ) ;
		if ( pie == nullptr )
		{
			continue ;
		}
		pie->ReloadComposer( this ) ;
		//
		S3DSceneComposer *	pComposer = pie->GetComposer() ;
		if ( pComposer != nullptr )
		{
			m_assets.AddReferenceAssets( &(pComposer->m_assets) ) ;
		}
	}
	return	sglErrSuccess ;
}

// インポート数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSceneComposer::GetImportComposerCount( void ) const
{
	return	m_imports.GetLength() ;
}

// インポート情報取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ImportEntry *
	S3DSceneComposer::GetImportComposerAt( size_t i ) const
{
	return	m_imports.GetAt( i ) ;
}

// インポートコンポーザー削除
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::RemoveImportComposerAt( size_t i )
{
	ImportEntry *	pie = m_imports.GetAt( i ) ;
	if ( pie != nullptr )
	{
		S3DSceneComposer *	pComposer = pie->GetComposer() ;
		if ( pComposer != nullptr )
		{
			m_assets.DetachReferenceAssets( &(pComposer->m_assets) ) ;
		}
		pie->UnloadComposer() ;
	}
	m_imports.RemoveAt( i ) ;
}

// リソース追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::AddAssetResource
	( const wchar_t * pwszID, ESLObject * pRsrc,
		const wchar_t * pwszType,
		const wchar_t * pwszSrcFile,
		const wchar_t * pwszPath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	return	AddResourceAs
				( m_assets,
					pwszID, pRsrc, pwszType,
					pwszSrcFile, pwszPath, pxmlOptions ) ;
}

// プロシージャルリソース追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::AddProceduralResource
	( const wchar_t * pwszID,
		S3DSceneComposer::ResourceProcedure * pProc,
		const wchar_t * pwszType,
		const wchar_t * pwszPath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SGLError	err =
		m_assets.RegisterResource
			( pwszID, nullptr, pwszType, nullptr, pwszPath, pxmlOptions ) ;
	if ( err )
	{
		return	err ;
	}
	ResourceContainer *	prc = m_assets.GetResourceContainerAs( pwszID ) ;
	ESLAssert( prc != nullptr ) ;
	if ( prc == nullptr )
	{
		return	sglErrFailed ;
	}
	pProc->AttachSceneComposer( this ) ;
	if ( pxmlOptions != nullptr )
	{
		pProc->ParseParameter( *pxmlOptions ) ;
	}
	prc->SetProcedure( pProc ) ;
	prc->CreateProceduralRsrc( m_assets ) ;
	//
	return	sglErrSuccess ;
}

// 全スクリプトを読み込み直す
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::ReloadAllScripts( bool flagReloadImportScripts )
{
	for ( size_t i = 0; i < m_imports.GetLength(); i ++ )
	{
		ImportEntry *	pie = m_imports.GetAt( i ) ;
		if ( (pie != nullptr)
			&& (pie->GetComposer() != nullptr) )
		{
			if ( flagReloadImportScripts
				|| (pie->GetManager() != m_pManager) )
			{
				pie->GetComposer()->ReloadAllScripts( flagReloadImportScripts ) ;
			}
		}
	}
	for ( size_t i = 0; i < m_assets.GetResourceCount(); i ++ )
	{
		ResourceContainer *	prc = m_assets.GetResourceAt( i ) ;
		if ( (prc == nullptr)
			|| (prc->GetProcedure() != nullptr)
			|| !prc->IsResourceType(resourceTypeScript) )
		{
			continue ;
		}
		SString	strRsrcID = m_assets.GetResourceIdentityAt( i ) ;
		ESLObject *	pObj =
			LoadAssetResourceObject
				( m_assets,
					prc->GetType(), strRsrcID,
					prc->GetSourceFile(),
					&(prc->GetOptions()) ) ;
		m_assets.ReloadResource( prc, pObj ) ;
	}
}

// アセットフォーマット
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::FormatResource
	( SSystem::SXMLDocument& xmlRsrc,
		const wchar_t * pwszID,
		const S3DSceneComposer::ResourceContainer& rcRsrc,
		SSystem::SStrSortArray<const ResourceContainer*> * pssaExport )
{
	SString	strRsrcPath = rcRsrc.GetSourceFile() ;
	SString	strSourceFile =
		m_strRsrcBaseDir.RelativeFilePath( strRsrcPath.NormalizeFilePath() ) ;
	//
	xmlRsrc.SetTag( rcRsrc.GetType() ) ;
	xmlRsrc.SetAttributeAs( L"id", pwszID ) ;
	if ( !rcRsrc.IsFolder() && (rcRsrc.GetProcedure() == nullptr) )
	{
		if ( pssaExport != nullptr )
		{
			if ( (strSourceFile.Find( L":" ) >= 0)
				&& (strSourceFile.Find( L"\\\\" ) >= 0)
				&& (strSourceFile.Find( L"..\\" ) >= 0) )
			{
				SString	strErrMsg ;
				strErrMsg.Format( L"リソース \'%s\' のパスが不正です（\'%s\'）",
									pwszID, (const wchar_t*) strSourceFile ) ;
				OutputError( strErrMsg ) ;
				//
				strSourceFile = SString(strSourceFile.GetFileNamePart()) ;
			}
			strSourceFile.Replace( L'/', L'\\' ) ;
			pssaExport->Add( strSourceFile, &rcRsrc ) ;
		}
		xmlRsrc.SetAttributeAs( L"src", strSourceFile ) ;
	}
	xmlRsrc.SetAttributeAs( L"path", rcRsrc.GetTreePath() ) ;
	//
	if ( !rcRsrc.GetOptions().IsEmpty() )
	{
		SXMLDocument *	pxmlOptions = new SXMLDocument( rcRsrc.GetOptions() ) ;
		pxmlOptions->SetTag( L"options" ) ;
		xmlRsrc.AddElement( pxmlOptions ) ;
	}
	return	sglErrSuccess ;
}

// アセット読み込み処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSceneComposer::ParseImportAssets
	( ResourceAssets& assets,
		const SXMLDocument& xmlAssets, const wchar_t * pwszBasePath )
{
	SGLError	errResult = sglErrSuccess ;
	size_t	i ;
	for ( i = 0; i < xmlAssets.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlAssets.GetElementAt( i ) ;
		if ( pxmlTag == nullptr )
		{
			continue ;
		}
		SGLError	err =
			LoadAssetFile( assets, *pxmlTag, pwszBasePath ) ;
		if ( err )
		{
			errResult = err ;
		}
	}
	SPointerArray<ResourceContainer>	aPendingRsrc ;
	for ( i = 0; i < assets.GetResourceCount(); i ++ )
	{
		ResourceContainer *	prc = assets.GetResourceAt( i ) ;
		if ( prc && prc->IsPendingProceduralRsrc() )
		{
			if ( !prc->CreateProceduralRsrc( assets ) )
			{
				aPendingRsrc.Add( prc ) ;
			}
		}
	}
	size_t	nNest = 0 ;
	while ( (aPendingRsrc.GetLength() > 0) && (nNest < 10) )
	{
		assets.MaterialLibrary().
			UpdateAllTextureReference( assets.GetTextureLibrary() ) ;
		//
		for ( i = 0; i < aPendingRsrc.GetLength(); i ++ )
		{
			ResourceContainer *	prc = aPendingRsrc.GetAt( i ) ;
			if ( (prc != nullptr)
				&& prc->CreateProceduralRsrc( assets ) )
			{
				aPendingRsrc.SetAt( i, nullptr ) ;
			}
		}
		aPendingRsrc.TrimEmpty() ;
		nNest ++ ;
	}
	//
	assets.UpdateAllResourceReference() ;
	return	errResult ;
}

SGLError S3DSceneComposer::ImportAssets
	( S3DSceneComposer::ResourceAssets& assets, const wchar_t * pwszAssetsFile )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszAssetsFile ) ;
	if ( pFile == nullptr )
	{
		return	sglErrFailed ;
	}
	SParserErrorTracer	perr ;
	SXMLDocument	xmlDoc ;
	SError	err = xmlDoc.ReadDocument( *pFile, perr ) ;
	if ( err )
	{
		return	(SGLError) err ;
	}
	SXMLDocument *	pxmlScene = xmlDoc.GetElementTagAs( L"scene" ) ;
	if ( pxmlScene == nullptr )
	{
		ESLTrace( "not found <scene> tag for import assets.\n" ) ;
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlAssets = pxmlScene->GetElementTagAs( L"assets" ) ;
	if ( pxmlAssets == nullptr )
	{
		ESLTrace( "not found <scene><assets> tag for import assets.\n" ) ;
		return	sglErrFailed ;
	}
	return	ParseImportAssets( assets, *pxmlAssets, pwszAssetsFile ) ;
}

SGLError S3DSceneComposer::LoadAssetFile
	( ResourceAssets& assets,
		const SSystem::SXMLDocument& xmlTag, const wchar_t * pwszBaseTreePath )
{
	SString			strBasePath = pwszBaseTreePath ;
	const SString&	strType = xmlTag.GetTag() ;
	SString			strID = xmlTag.GetAttrStringAs( L"id" ) ;
	SString			strSrcFile = xmlTag.GetAttrStringAs( L"src" ) ;
	SString			strSrc = m_strRsrcBaseDir.OffsetFilePath( strSrcFile ) ;
	SString			strPath =
						strBasePath.OffsetFilePath
							( xmlTag.GetAttrStringAs( L"path" ) ) ;
	SXMLDocument *	pxmlOptions = xmlTag.GetElementTagAs( L"options" ) ;
	//
	ESLObject *	pRsrc = nullptr ;
	if ( !strSrcFile.IsEmpty() )
	{
		strSrc.NormalizeFilePath() ;
		pRsrc = LoadAssetResourceObject
			( assets, strType, strID, strSrcFile, pxmlOptions ) ;
	}
	//
	return	AddResourceAs
				( assets, strID, pRsrc, strType, strSrc, strPath, pxmlOptions ) ;
}

SGLError S3DSceneComposer::LoadAssetFile
	( ResourceAssets& assets,
		const wchar_t *	pwszTypeID,
		const wchar_t *	pwszRsrcID,
		const wchar_t *	pwszFilePath,
		const wchar_t * pwszTreePath,
		const SXMLDocument * pxmlOptions )
{
	ESLObject *	pRsrc =
		LoadAssetResourceObject
			( assets, pwszTypeID, pwszRsrcID, pwszFilePath, pxmlOptions ) ;
	//
	return	AddResourceAs
				( assets, pwszRsrcID, pRsrc, pwszTypeID,
						pwszFilePath, pwszTreePath, pxmlOptions ) ;
}

SGLError S3DSceneComposer::AddResourceAs
	( ResourceAssets& assets,
		const wchar_t * pwszID, ESLObject * pRsrc,
		const wchar_t * pwszType,
		const wchar_t * pwszSrcFile,
		const wchar_t * pwszPath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SGLError	err =
		assets.AddResourceAs
				( pwszID, pRsrc, pwszType,
						pwszSrcFile, pwszPath, pxmlOptions ) ;
	if ( !err && (pxmlOptions != nullptr) && (m_pManager != nullptr) )
	{
		ResourceContainer *	prc = assets.GetResourceContainerAs( pwszID ) ;
		SString *	pstrProcID = pxmlOptions->GetAttributeAs( L"proc_id" ) ;
		if ( (prc != nullptr) && (pstrProcID != nullptr) )
		{
			ResourceProcedure *	pProc =
					m_pManager->CreateResourceProc( *pstrProcID ) ;
			if ( pProc != nullptr )
			{
				pProc->AttachSceneComposer( this ) ;
				pProc->ParseParameter( *pxmlOptions ) ;
				prc->SetProcedure( pProc ) ;
			}
		}
	}
	return	err ;
}

ESLObject * S3DSceneComposer::LoadAssetResourceObject
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t *	pwszRsrcID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	if ( (pwszFilePath == nullptr) || (pwszFilePath[0] == 0) )
	{
		return	nullptr ;
	}
	ResourceType	typeRsrc = GetResourceType( pwszTypeID ) ;
	switch ( typeRsrc )
	{
	case	resourceTypeImage:
		return	LoadImageAsset
					( assets, pwszTypeID, pwszRsrcID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeAudio:
		return	LoadAudioAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeMovie:
		return	LoadMovieAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeModel:
		return	(S3DRenderBufferInterface*)
				LoadModelAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypePose:
		return	LoadPoseLibraryAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeMaterial:
		return	LoadMaterialLibraryAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeShaderDef:
		return	LoadUserShaderAsset
					( assets, pwszTypeID, pwszRsrcID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeBasicForm:
		return	LoadBasicFormAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeScript:
		return	LoadRosettaScriptAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeAntirrhinum:
		return	LoadAntirrhinumScriptAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeBinary:
		return	(SFileOpener*)
				LoadBinaryAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeLoquaty:
		return	LoadLoquatyScriptAsset
					( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;

	case	resourceTypeFolder:
		break ;

	default:
		return	LoadExtendAssetFile( assets, pwszTypeID, pwszFilePath, pxmlOptions ) ;
	}
	return	nullptr ;
}

S3DModelBuffer * S3DSceneComposer::LoadModelAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	S3DModelBuffer *	pModel = new S3DModelBuffer ;
	SGLError	err = pModel->ReadModel( pFile ) ;
	if ( err )
	{
		OutputError( SString( pwszFilePath ) + L" : 読み込みに失敗しました" ) ;
		delete	pModel ;
		return	nullptr ;
	}
	return	pModel ;
}

SGLImage * S3DSceneComposer::LoadImageAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t *	pwszRsrcID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	//
	LoadImageOption	opt ;
	if ( pxmlOptions != nullptr )
	{
		opt.ParseOptions( *pxmlOptions ) ;
	}
	//
	size_t	iExt = 0 ;
	for ( size_t i = 0; pwszFilePath[i] != 0; i ++ )
	{
		if ( pwszFilePath[i] == L'.' )
		{
			iExt = i + 1 ;
		}
	}
	SGLImage *	pImage = new SGLImage ;
	SGLError	err = sglErrSuccess ;
	SGLImageDecoderInterface *	pDecoder =
		SGLImageDecoderManager::FindDecoder( pwszFilePath + iExt ) ;
	if ( pDecoder != nullptr )
	{
		err = pDecoder->ReadImage( *pImage, *pFile ) ;
		if ( err )
		{
			pFile->Seek( 0 ) ;
			err = pImage->ReadImage( pFile ) ;
		}
	}
	else
	{
		err = pImage->ReadImage( pFile ) ;
	}
	if ( err )
	{
		OutputError( SString( pwszFilePath ) + L" : 読み込みに失敗しました" ) ;
		delete	pImage ;
		return	nullptr ;
	}
	//
	if ( opt.format & formatImageFlagS3TC )
	{
		bool	flagSupportedS3TC = false ;
		//
		QuickLock() ;
		SGLAbstractWindow *	pWindow = SGLAbstractWindow::GetDefaultWindow() ;
		while ( pWindow != nullptr )
		{
			S3DRenderDevice *	pDevice = pWindow->GetRenderDevice() ;
			if ( pDevice != nullptr )
			{
				S3DRenderDevice::Features	features ;
				if ( !pDevice->GetDeviceFeatures( features ) )
				{
					if ( features.flagsFeatures[0]
							& S3DRenderDevice::feature0_CompressionS3TC )
					{
						flagSupportedS3TC = true ;
						break ;
					}
				}
			}
			pWindow = pWindow->EnumerateNextWindow() ;
		}
		QuickUnlock() ;
		//
		if ( !flagSupportedS3TC )
		{
			opt.format &= ~formatImageFlagS3TC ;
		}
	}
	pImage->NormalizeFormat
		( opt.format, opt.depth,
			opt.nNormalizeFlags, opt.width, opt.height ) ;
	//
	if ( (opt.argbBackColor.ui32 != 0)
		&& (pImage->GetImageFormat() & formatImageFlagAlpha) )
	{
		pImage->BlendImageBackgroundColor( opt.argbBackColor ) ;
	}
	pImage->NormalizeToTexture( opt.nTextureFlags ) ;
	//
	if ( opt.nTextureFlags & SGLImageObject::bufferForMipmapTexture )
	{
		pImage->NormalizeToMipmapTexture() ;
	}
	pImage->SetImageIdentity( pwszRsrcID ) ;
	//
	return	pImage ;
}

SGLAudioPlayer * S3DSceneComposer::LoadAudioAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SFileInterface *	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SString	strFilePath = pwszFilePath ;
	SFile *	pRawFile = ESLTypeCast<SFile>( pFile ) ;
	if ( pRawFile != nullptr )
	{
		strFilePath = pRawFile->GetFilePath() ;
	}
	//
	LoadAudioOption	opt ;
	if ( pxmlOptions != nullptr )
	{
		opt.ParseOptions( *pxmlOptions ) ;
	}
	//
	SGLAudioPlayer *	pAudio = new SGLAudioPlayer ;
	SGLError	err = pAudio->Create( pFile, true, opt.nOpenFlags ) ;
	if ( err )
	{
		err = pAudio->Open( strFilePath, opt.nOpenFlags ) ;
		if ( err )
		{
			OutputError( SString( pwszFilePath ) + L" : 読み込みに失敗しました" ) ;
			delete	pAudio ;
			return	nullptr ;
		}
	}
	pAudio->SetVolumeLineMask( 1 << opt.iVolumeLine ) ;
	return	pAudio ;
}

SGLMediaPlayer * S3DSceneComposer::LoadMovieAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SFileInterface *	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SString	strFilePath = pwszFilePath ;
	SFile *	pRawFile = ESLTypeCast<SFile>( pFile ) ;
	if ( pRawFile != nullptr )
	{
		strFilePath = pRawFile->GetFilePath() ;
	}
	//
	LoadMovieOption	opt ;
	if ( pxmlOptions != nullptr )
	{
		opt.ParseOptions( *pxmlOptions ) ;
	}
	//
	SGLMediaPlayer *	pMovie = new SGLMediaPlayer ;
	SGLError	err = pMovie->Create( pFile, true, opt.nOpenFlags ) ;
	if ( err )
	{
		err = pMovie->Open( strFilePath, opt.nOpenFlags ) ;
		if ( err )
		{
			OutputError( SString( pwszFilePath ) + L" : 読み込みに失敗しました" ) ;
			delete	pMovie ;
			return	nullptr ;
		}
	}
	pMovie->SetAudioVolumeLine( opt.iVolumeLine ) ;
	return	pMovie ;
}

S3DModelPoseLibrary * S3DSceneComposer::LoadPoseLibraryAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	S3DModelPoseLibrary *	pPoseLib = new S3DModelPoseLibrary ;
	SGLError	err = pPoseLib->ReadLibrary( *pFile ) ;
	pFile = nullptr ;
	if ( err )
	{
		SXMLDocument	xmlDoc ;
		pFile->Seek( 0 ) ;
		xmlDoc.ReadDocument( *pFile, xmlDoc ) ;
		//
		SXMLDocument *	pxmlLib = xmlDoc.GetElementTagAs( L"library" ) ;
		if ( pxmlLib == nullptr )
		{
			OutputError( SString( pwszFilePath ) + L" : <library> タグが見つかりません" ) ;
			delete	pPoseLib ;
			return	nullptr ;
		}
		err = pPoseLib->ParseLibraryXML( *pxmlLib ) ;
		if ( err )
		{
			OutputError( SString( pwszFilePath ) + L" : 読み込みに失敗しました" ) ;
			delete	pPoseLib ;
			return	nullptr ;
		}
	}
	return	pPoseLib ;
}

S3DMaterialLibrary * S3DSceneComposer::LoadMaterialLibraryAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SXMLDocument	xmlDoc ;
	pFile->Seek( 0 ) ;
	xmlDoc.ReadDocument( *pFile, xmlDoc ) ;
	//
	SXMLDocument *	pxmlLib = xmlDoc.GetElementTagAs( L"materials" ) ;
	if ( pxmlLib == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : <materials> タグが見つかりません" ) ;
		return	nullptr ;
	}
	S3DMaterialLibrary *	pMaterialLib = new S3DMaterialLibrary ;
	SGLError	err =
		pMaterialLib->ParseXML
			( *pxmlLib, assets.GetTextureLibrary() ) ;
	if ( err )
	{
		OutputError( SString( pwszFilePath ) + L" : 読み込みに失敗しました" ) ;
		delete	pMaterialLib ;
		return	nullptr ;
	}
	return	pMaterialLib ;
}

S3DSceneComposer::UserShader * S3DSceneComposer::LoadUserShaderAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t *	pwszRsrcID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SXMLDocument	xmlDoc ;
	pFile->Seek( 0 ) ;
	xmlDoc.ReadDocument( *pFile, xmlDoc ) ;
	//
	SXMLDocument *	pxmlShader = xmlDoc.GetElementTagAs( L"shader" ) ;
	if ( pxmlShader == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : <shader> タグが見つかりません" ) ;
		return	nullptr ;
	}
	UserShader *	pShader = new UserShader( pwszRsrcID ) ;
	pShader->ParseDescriptor( *pxmlShader ) ;
	return	pShader ;
}

SGLBasicFormParser * S3DSceneComposer::LoadBasicFormAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SGLBasicFormParser *	pBasicForm = new SGLBasicFormParser ;
	pBasicForm->AttachResourceProducer( &(assets.TextureLibrary()) ) ;
	if ( pBasicForm->ReadForm( *pFile ) )
	{
		OutputError( SString( pwszFilePath ) + L" : 読み込みに失敗しました" ) ;
	}
	return	pBasicForm ;
}

SStringParser * S3DSceneComposer::LoadRosettaScriptAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SString	strFileName = SString(pwszFilePath).GetFileNamePart() ;
	if ( m_pManager->VM().GetLoadedScriptAs( strFileName ) != nullptr )
	{
		return	nullptr ;
	}
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SStringParser *	psparsSrc = new SStringParser ;
	psparsSrc->ReadTextFile( *pFile ) ;
	//
	RSVirtualMachine&	vm = m_pManager->VM() ;
	RSContext	context( &vm ) ;
	vm.AddScriptFileOpener( pFile ) ;
	vm.AddScriptSource( context, *psparsSrc, pwszFilePath ) ;
	vm.DetachScriptFileOpener( pFile ) ;
	//
	if ( context.IsException() )
	{
		OutputRosettaException( context ) ;
	}
	return	psparsSrc ;
}

AntirrhinumGL::AGLModulePtr *
	S3DSceneComposer::LoadAntirrhinumScriptAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	AntirrhinumGL::AGLModule *
		pModule = assets.AntirrhinumManager().LoadScript( pwszFilePath ) ;
	return	new AntirrhinumGL::AGLModulePtr
					( pModule, &assets.AntirrhinumManager() ) ;
}

SSystem::SByteBuffer * S3DSceneComposer::LoadBinaryAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SSmartPointer<SFileInterface>	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SByteBuffer *	pBinBuf = new SByteBuffer ;
	pBinBuf->ReadFromFile( *pFile ) ;
	return	pBinBuf ;
}

ESLObject * S3DSceneComposer::LoadLoquatyScriptAsset
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	SFileInterface *	pFile = OpenAssetFile( pwszFilePath ) ;
	if ( pFile == nullptr )
	{
		OutputError( SString( pwszFilePath ) + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	SString					strScriptFile = m_strRsrcBaseDir.OffsetFilePath( pwszFilePath ) ;
	LSceneManagerCurrent	manCurrent( m_pManager ) ;
	LVirtualMachine *	vm = m_pManager->LoquatyVM() ;
	LGLS4File			lfile( pFile ) ;
	LSourceFilePtr		pSource = vm->SourceProducer().
									GetSourceFile( strScriptFile, &lfile ) ;
	if ( pSource != nullptr )
	{
		return	new LSourceFilePtrObj( pSource ) ;
	}
	pSource = vm->SourceProducer().GetSourceFile
					( SString( strScriptFile.GetFileNamePart() ), &lfile ) ;
	if ( pSource != nullptr )
	{
		return	new LSourceFilePtrObj( pSource ) ;
	}
	pSource = vm->SourceProducer().LoadSourceFile( strScriptFile, &lfile ) ;
	if ( pSource == nullptr )
	{
		OutputError( strScriptFile + L" : 開けませんでした" ) ;
		return	nullptr ;
	}
	LSceneCompiler	compiler( *this, *vm ) ;
	compiler.DoCompile( pSource.get() ) ;
	return	new LSourceFilePtrObj( pSource ) ;
}

ESLObject * S3DSceneComposer::LoadExtendAssetFile
	( ResourceAssets& assets,
		const wchar_t * pwszTypeID,
		const wchar_t * pwszFilePath,
		const SSystem::SXMLDocument * pxmlOptions )
{
	OutputError( SString(pwszTypeID) + L" : 未定義のアセット種別です" ) ;
	return	nullptr ;
}

// アニメーション画像／テクスチャオプション
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::LoadImageOption::LoadImageOption( void )
	: argbBackColor( 0 )
{
	nTextureFlags = 0 ;
	nNormalizeFlags = 0 ;
	format = 0 ;
	depth = 0 ;
	width = 0 ;
	height = 0 ;
}

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::LoadImageOption::m_aiTextureFlags[10] =
{
	{ L"mipmap", SGLImageObject::bufferForMipmapTexture },
	{ L"3d_texture", SGLImageObject::bufferTexture3D },
	{ L"compressed", SGLImageObject::bufferCompressedTexture },
	{ L"tiling", SGLImageObject::bufferSampleTiling },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::LoadImageOption::m_aiNormalizeFlags[10] =
{
	{ L"merge_animation", SGLImageObject::formatMergeAnimation },
	{ L"straight_pixel", SGLImageObject::formatStraightPixel },
	{ L"no_dithering", SGLImageObject::formatNoDithering },
	{ nullptr, 0 },
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::LoadImageOption::m_aiFormatFlags[10] =
{
	{ L"argb", formatImageARGB },
	{ L"rgb", formatImageRGB },
	{ L"grayscale", formatImageGray },
	{ L"s3tc_dxt1_rgb", formatImageRGB_S3TC_DXT1 },
	{ L"s3tc_dxt1_rgba", formatImageRGBA_S3TC_DXT1 },
	{ nullptr, 0 },
} ;

void S3DSceneComposer::LoadImageOption::ParseOptions( const SSystem::SXMLDocument& xmlOptions )
{
	nTextureFlags = (uint32_t)
		xmlOptions.GetAttrComplexIntegerAs
			( L"texture_flags", m_aiTextureFlags, 0 ) ;
	nNormalizeFlags = (uint32_t)
		xmlOptions.GetAttrComplexIntegerAs
			( L"normalize_flags", m_aiNormalizeFlags, 0 ) ;
	format = (uint32_t)
		xmlOptions.GetAttrSymbolizedIntegerAs
			( L"format", m_aiFormatFlags, 0 ) ;
	depth = (uint32_t) xmlOptions.GetAttrIntegerAs( L"depth", 0 ) ;
	width = (uint32_t) xmlOptions.GetAttrIntegerAs( L"width", 0 ) ;
	height = (uint32_t) xmlOptions.GetAttrIntegerAs( L"height", 0 ) ;
	argbBackColor.ui32 = (uint32_t) xmlOptions.GetAttrHexIntegerAs( L"back_color", 0 ) ;
}

void S3DSceneComposer::LoadImageOption::FormatOptions( SSystem::SXMLDocument& xmlOptions )
{
	xmlOptions.SetAttrComplexIntegerAs
		( L"texture_flags", m_aiTextureFlags, nTextureFlags ) ;
	xmlOptions.SetAttrComplexIntegerAs
		( L"normalize_flags", m_aiNormalizeFlags, nNormalizeFlags ) ;
	xmlOptions.SetAttrSymbolizedIntegerAs
		( L"format", m_aiFormatFlags, format ) ;
	xmlOptions.SetAttrIntegerAs( L"depth", depth ) ;
	xmlOptions.SetAttrIntegerAs( L"width", width ) ;
	xmlOptions.SetAttrIntegerAs( L"height", height ) ;
	xmlOptions.SetAttrHexIntegerAs( L"back_color", argbBackColor.ui32 ) ;
}

// オーディオオプション
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::LoadAudioOption::LoadAudioOption( void )
{
	nOpenFlags = SGLAudioPlayerInterface::modeOpenAutoStatic ;
}

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::LoadAudioOption::m_aiOpenFlags[10] =
{
	{ L"static", SGLAudioPlayerInterface::modeOpenStatic },
	{ L"auto_static", SGLAudioPlayerInterface::modeOpenAutoStatic },
	{ L"dynamic", SGLAudioPlayerInterface::modeOpenDynamicOnMemory },
	{ nullptr, 0 },
} ;

void S3DSceneComposer::LoadAudioOption::ParseOptions( const SSystem::SXMLDocument& xmlOptions )
{
	nOpenFlags = xmlOptions.GetAttrSymbolizedIntegerAs
					( L"load", m_aiOpenFlags, nOpenFlags ) ;
	iVolumeLine = (size_t) xmlOptions.GetAttrIntegerAs( L"volume_line", SGLAudioPlayer::lineMusic ) ;
}

void S3DSceneComposer::LoadAudioOption::FormatOptions( SSystem::SXMLDocument& xmlOptions )
{
	xmlOptions.SetAttrSymbolizedIntegerAs
					( L"load", m_aiOpenFlags, nOpenFlags ) ;
	xmlOptions.SetAttrIntegerAs( L"volume_line", iVolumeLine ) ;
}

// 動画オプション
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::LoadMovieOption::LoadMovieOption( void )
{
	nOpenFlags = SGLAudioPlayerInterface::modeOpenDynamicRead ;
}

const SSystem::SXMLDocument::AttrInteger
	S3DSceneComposer::LoadMovieOption::m_aiOpenFlags[10] =
{
	{ L"static", SGLAudioPlayerInterface::modeOpenStatic },
	{ L"auto_static", SGLAudioPlayerInterface::modeOpenAutoStatic },
	{ L"dynamic", SGLAudioPlayerInterface::modeOpenDynamicRead },
	{ nullptr, 0 },
} ;

void S3DSceneComposer::LoadMovieOption::ParseOptions( const SSystem::SXMLDocument& xmlOptions )
{
	nOpenFlags = xmlOptions.GetAttrSymbolizedIntegerAs
					( L"load", m_aiOpenFlags, nOpenFlags ) ;
	iVolumeLine = (size_t) xmlOptions.GetAttrIntegerAs( L"volume_line", SGLAudioPlayer::lineComposition ) ;
}

void S3DSceneComposer::LoadMovieOption::FormatOptions( SSystem::SXMLDocument& xmlOptions )
{
	xmlOptions.SetAttrSymbolizedIntegerAs
					( L"load", m_aiOpenFlags, nOpenFlags ) ;
	xmlOptions.SetAttrIntegerAs( L"volume_line", iVolumeLine ) ;
}

// S3DSceneComposer 生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer * S3DSceneComposer::NewSceneComposer( void ) const
{
	return	m_pManager->NewSceneComposer() ;
}

// リソースファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface *
	S3DSceneComposer::OpenAssetFile( const wchar_t * pwszFile )
{
	if ( m_pRsrcFileOpener != nullptr )
	{
		SFileInterface *	pFile =
			m_pRsrcFileOpener->NewOpenFile
					( pwszFile, SFileOpener::shareRead ) ;
		if ( pFile != nullptr )
		{
			return	pFile ;
		}
	}
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile
			( m_strRsrcBaseDir.OffsetFilePath( pwszFile ), SFileOpener::shareRead ) ;
	if ( pFile != nullptr )
	{
		return	pFile ;
	}
	pFile = SFileOpener::DefaultNewOpenFile( pwszFile, SFileOpener::shareRead ) ;
	if ( pFile != nullptr )
	{
		return	pFile ;
	}
	if ( m_pRsrcFileOpener != nullptr )
	{
		SString	strFileName = SString( pwszFile ).GetFileNamePart() ;
		pFile = m_pRsrcFileOpener->NewOpenFile
					( strFileName, SFileOpener::shareRead ) ;
		if ( pFile != nullptr )
		{
			return	pFile ;
		}
	}
	SString	strFileName = SString( pwszFile ).GetFileNamePart() ;
	return	SFileOpener::DefaultNewOpenFile( strFileName, SFileOpener::shareRead ) ;
}

// アイテム生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DSceneComposer::CreateSceneItem( const wchar_t * pwszType )
{
	LSceneComposerCurrent	current( this ) ;
	return	m_pManager->CreateSceneItem( pwszType ) ;
}

// コントローラー生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Controller *
	S3DSceneComposer::CreateController( const wchar_t * pwszClass )
{
	LSceneComposerCurrent	current( this ) ;
	return	m_pManager->CreateController( pwszClass ) ;
}

// エディタ・インターフェース取得
//////////////////////////////////////////////////////////////////////////////
S3DCompositionEditorInterface * S3DSceneComposer::GetEditorInterface( void )
{
	return	nullptr ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::OutputError
	( const wchar_t * pwszErrMsg,
		const wchar_t * pwszSrcFile,
		int nLineNum, const wchar_t * pwszSrcLine )
{
	SArray<char>	bufErrMsg ;
	SArray<char>	bufSrcFile ;
	SArray<char>	bufSrcLine ;
	//
	if ( pwszSrcFile != nullptr )
	{
		ESLTrace( "%s(%d):error:%s\n",
			SString(pwszSrcFile).EncodeDefaultTo(bufSrcFile), nLineNum,
			SString(pwszErrMsg).EncodeDefaultTo(bufErrMsg) ) ;
	}
	else
	{
		ESLTrace( "error:%s\n",
			SString(pwszErrMsg).EncodeDefaultTo(bufErrMsg) ) ;
	}
	if ( pwszSrcLine != nullptr )
	{
		ESLTrace( ">>%s\n",
			SString(pwszSrcLine).EncodeDefaultTo(bufSrcLine) ) ;
	}
}

// デバッグ用ログ出力
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::OutputTraceLog( const wchar_t * pwszLogMsg )
{
	SArray<char>	bufLogMsg ;
	Trace( SString(pwszLogMsg).EncodeDefaultTo(bufLogMsg) ) ;
}

// Rosetta スクリプトの例外を出力
//////////////////////////////////////////////////////////////////////////////
void S3DSceneComposer::OutputRosettaException( Rosetta::RSContext& context )
{
	RSObject *	pException = context.GetException() ;
	if ( pException != nullptr )
	{
		SString	strErr, strSrcPath, strSrcLine ;
		size_t	iSrcIndex, iSrcLine ;
		if ( !pException->AsString( strErr ) )
		{
			strErr = pException->GetTypeName() ;
		}
		context.GetExceptionPositionInfo
			( strSrcPath, strSrcLine, iSrcIndex, iSrcLine ) ;
		//
		if ( strSrcPath.IsEmpty() )
		{
			OutputError( strErr, nullptr, 0, nullptr ) ;
		}
		else
		{
			OutputError( strErr, strSrcPath, (int) iSrcLine, strSrcLine ) ;
		}
		context.ClearException() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// コンポーザー・マネージャー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCompositionManager, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCompositionManager::S3DCompositionManager( void )
{
	m_pParent = nullptr ;
	m_pLoquatyVM = nullptr ;
	m_pDynamicPlugin = nullptr ;
	m_random.InitializeSeed() ;
	m_flagScene = false ;

	// アイテムクラス登録
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSceneComposer::SpaceSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSceneComposer::LightSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSceneComposer::CameraSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSceneComposer::ModelSerializer) ) ;
	//
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCameraRelativeSpace) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DDynamicCamera) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DDynamicModelSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMultiModelSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMultiInstanceSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DVRControllerModelSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DParticleSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DParticleShootSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBillboardSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DStringBillboardSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSubCompositionSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSubCompInstanceSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSoundItemSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DLensFlareSerializer) ) ;
	//
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasSerializer) ) ;
	//
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBulletItemSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBatteryItemSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMeshBufferItemSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DIndirectMeshBuilderSerializer) ) ;
	//
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMeshEditorSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBonePhysMaterialSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBoneSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMarkerItemSerializer) ) ;
	//
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCubeEnvironmentMapSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DDelayLightSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSSGlobalIlluminationSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DGlowEffectSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DDepthOfFieldEffectSerializer) ) ;
	AddItemDescriptor
		( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCurtainEffectSerializer) ) ;

	// コントローラークラス登録
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSceneScriptInstance) ) ;
	AddControllerCreator
			( L"rosetta_instance",
				new S3DSceneComposer::ESLItemCreator
					( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSceneScriptInstance) ) ) ;	// 互換性のため
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCompositionPlayerController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DQuickVisibleFlagController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCameraRelativeController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCameraWiggleController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCameraOffsetController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCameraHMDPosition) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCameraRelationController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSpaceReferenceController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DDynamicModelSerializer::PoseTrack) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DDynamicModelSerializer::IKBoneTrack) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DInertialPoseController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DLODModelInstanceController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DLODModelShaderController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DModelPartialRenderController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DInstanceEntryController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DOPhysicsInstancingController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCyclicInstanceController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCyclicMatrixController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DInstanceRotationController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBulletDrawController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBulletRenderController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBulletParticleController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBulletSoundController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBulletSubCompositionController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DPolyhedronMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DStandardMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DModelRefMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCascadePlaneMeshController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DStandardShaderController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DUserShaderController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DShadowDepthFuncController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSimpleWaterShaderController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMeshBezierPathController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DThunderMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DTreeMeshBuilderController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMorphMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DNormalMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DDisplacementMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DMirrorMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DArrayMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBooleanMeshController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DSimpleMeshReductionController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DBakedMeshController) ) ;
	//
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasImageController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasTextController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasBasicFormController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasAudioTrackController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasMovieTrackController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasCompositionTrackController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasBillboardController) ) ;
	AddControllerDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DCanvasBlurEffectController) ) ;

	// プロシージャルリソース登録
	AddResourceProcDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DResourceAtlasTextureProc) ) ;
	AddResourceProcDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DRsrc3DTextureBuilderProc) ) ;
	AddResourceProcDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DRsrcImageRepeaterProc) ) ;
	AddResourceProcDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DRsrcImagePerlinProc) ) ;
	AddResourceProcDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DRsrcImageNormalInverseProc) ) ;
	AddResourceProcDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DRsrcImageBumpNormalProc) ) ;
	AddResourceProcDescriptor
			( S3D_COMPOSER_ITEM_DESCRIPTOR(S3DResourceCompositionBakerProc) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCompositionManager::~S3DCompositionManager( void )
{
	Release() ;
}

// VM の初期化
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::InitializeVM
	( Rosetta::RSVirtualMachine * pRefVM,
		ECSSakura2::StandardVM * pSakura2VM, bool flagDebugMode,
		Loquaty::LVirtualMachine * pRefLoquatyVM )
{
	if ( m_pLoquatyVM == nullptr )
	{
		m_pLoquatyVM = new Loquaty::LVirtualMachine ;
		if ( pRefLoquatyVM == nullptr )
		{
			m_pLoquatyVM->Initialize() ;
		}
		else
		{
			m_pLoquatyVM->InitializeRef( *pRefLoquatyVM ) ;
		}
		Loquaty::SetEntisGLS4FileSystem() ;
		Loquaty::AddEnvironmentIncludePath( m_pLoquatyVM ) ;
		Loquaty::DefineEntisGLS4NativeFunctions( m_pLoquatyVM ) ;
		if ( pRefLoquatyVM == nullptr )
		{
			Loquaty::LCompiler	compiler( *m_pLoquatyVM ) ;
			compiler.IncludeScript( L"entisgls4.lqs" ) ;
		}
	}
	if ( pRefVM == nullptr )
	{
		m_vmRosetta.Initialize() ;
	}
	else
	{
		m_vmRosetta.InitializeRefVM( *pRefVM ) ;
	}
	if ( pSakura2VM != nullptr )
	{
		m_vmRosetta.AttachSakura2VM( pSakura2VM ) ;
	}
	m_vmRosetta.SetDebug( flagDebugMode ) ;
	//
	m_pContext = new Rosetta::RSContext( &m_vmRosetta ) ;
}

void S3DCompositionManager::InitializeRef
	( S3DCompositionManager * pParent, bool flagDebugMode )
{
	m_pParent = pParent ;
	//
	InitializeVM
		( &(pParent->m_vmRosetta),
			pParent->m_vmRosetta.GetSakura2VM(),
			flagDebugMode, pParent->m_pLoquatyVM ) ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::Release( void )
{
	while ( m_ssoaComposer.GetLength() > 0 )
	{
		m_ssoaComposer.RemoveAt( m_ssoaComposer.GetLength() - 1 ) ;
	}
	ReleaseVM() ;
}

// VM のみ解放
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::ReleaseVM( void )
{
	m_vmRosetta.Release() ;
	m_pContext = nullptr ;
	m_pParent = nullptr ;

	if ( m_pDynamicPlugin != nullptr )
	{
		ssize_t	i = m_aPlugins.FindPtr( m_pDynamicPlugin ) ;
		if ( i >= 0 )
		{
			m_aPlugins.RemoveAt( (size_t) i ) ;
		}
		m_pDynamicPlugin = nullptr ;
	}
	ReleaseTempItemCreator( m_ssaItemCreators ) ;
	ReleaseTempItemCreator( m_ssaCtrlCreators ) ;
	ReleaseTempItemCreator( m_ssaRsrcProcCreators ) ;

	if ( m_pLoquatyVM != nullptr )
	{
		m_pLoquatyVM->ReleaseRef() ;
		m_pLoquatyVM = nullptr ;
	}
}

// Rosetta スクリプトを Sakura2 コードへコンパイル
//////////////////////////////////////////////////////////////////////////////
SSystem::SError S3DCompositionManager::CompileRosettaToSakura2
	( bool flagFlatPointer,
		bool flagCompileToNative,
		SSystem::SParserErrorLogger& perr )
{
	return	m_vmRosetta.CompileToSakura2
					( flagFlatPointer, flagCompileToNative, perr ) ;
}

// 数式を Rosetta VM で評価
//////////////////////////////////////////////////////////////////////////////
Rosetta::RSSmartPtr
	S3DCompositionManager::ScriptExpression::EvalExpressionAsRosetta
			( S3DSceneDebugTracer * pTracer )
{
	RSSmartPtr	pObj
		( m_exprRosetta.PerformExpression( nullptr, pTracer ),
									m_exprRosetta.GetContext() ) ;
	m_exprRosetta.ClearException() ;
	return	pObj ;
}

// 数式を Loquaty VM で評価
//////////////////////////////////////////////////////////////////////////////
Loquaty::LValue
	S3DCompositionManager::ScriptExpression::EvalExpressionAsLoquaty
			( S3DSceneDebugTracer * pTracer )
{
	LPtr<LTaskObj>	pLTask = m_context.GetLoquaty() ;
	if ( pLTask == nullptr )
	{
		return	LValue() ;
	}
	return	m_evalLoquaty.EvaluateValue( pLTask, LObjPtr() ) ;
}

// 数式を Rosetta として事前処理
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::MakeExpressionAsRosetta
	( ScriptExpression& expr,
		const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer )
{
	RSVirtualMachine *		pVM = &m_vmRosetta ;
	expr.m_exprRosetta.PrepareContext( pVM ) ;
	expr.m_exprRosetta.SetSourceCode( pVM, pwszExpr ) ;
}

// 数式を Rosetta VM で評価
//////////////////////////////////////////////////////////////////////////////
Rosetta::RSSmartPtr S3DCompositionManager::EvalExpressionAsRosetta
	( const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer )
{
	RSVirtualMachine *		pVM = &m_vmRosetta ;
	Rosetta::RSExpression	exprRosetta ;
	exprRosetta.PrepareContext( pVM ) ;
	exprRosetta.SetSourceCode( pVM, pwszExpr ) ;

	RSSmartPtr	pObj
		( exprRosetta.PerformExpression( nullptr, pTracer ),
									exprRosetta.GetContext() ) ;
	exprRosetta.ClearException() ;
	return	pObj ;
}

// 数式を Loquaty として事前処理
//////////////////////////////////////////////////////////////////////////////
bool S3DCompositionManager::MakeExpressionAsLoquaty
	( ScriptExpression& expr,
		const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer )
{
	if ( m_pLoquatyVM == nullptr )
	{
		return	false ;
	}
	expr.m_context.SetLoquaty( m_pLoquatyVM->new_Task() ) ;
	expr.m_evalLoquaty.AttachVM( *m_pLoquatyVM ) ;
	if ( expr.m_evalLoquaty.MakeExpression( pwszExpr, nullptr ) )
	{
		return	true ;
	}
	if ( pTracer != nullptr )
	{
		SStringParser	ss ;
		pTracer->OutputError( ss, expr.m_evalLoquaty.GetErrorMessages() ) ;
	}
	return	false ;
}

// 数式を Loquaty VM で評価
//////////////////////////////////////////////////////////////////////////////
Loquaty::LValue S3DCompositionManager::EvalExpressionAsLoquaty
	( const wchar_t * pwszExpr, S3DSceneDebugTracer * pTracer )
{
	if ( m_pLoquatyVM == nullptr )
	{
		return	LValue() ;
	}
	LInstantEvaluator	eval( *m_pLoquatyVM ) ;
	if ( eval.MakeExpression( pwszExpr, nullptr ) )
	{
		LPtr<LTaskObj>	pLTask = m_pLoquatyVM->new_Task() ;
		return	eval.EvaluateValue( pLTask, LObjPtr() ) ;
	}
	else
	{
		if ( pTracer != nullptr )
		{
			SStringParser	ss ;
			pTracer->OutputError( ss, eval.GetErrorMessages() ) ;
		}
	}
	return	LValue() ;
}

// 数式を Loquaty VM で評価（参照）
//////////////////////////////////////////////////////////////////////////////
Loquaty::LValue S3DCompositionManager::EvalRefExpressionAsLoquaty( const wchar_t * pwszExpr )
{
	if ( m_pLoquatyVM == nullptr )
	{
		return	LValue() ;
	}
	LStringParser	spars = pwszExpr ;
	LContext		context( *m_pLoquatyVM ) ;
	return	LDebugger::EvaluateExpr
					( context, spars, LDebugger::evalReference ) ;
}

// プラグイン追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddPlugin( S3DSceneComposerPluginInstance * pPlugin )
{
	m_aPlugins.Add( pPlugin ) ;
}

// プラグイン数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DCompositionManager::GetPluginCount( void ) const
{
	return	m_aPlugins.GetLength() ;
}

// プラグイン取得
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposerPluginInstance * S3DCompositionManager::GetPluginAt( size_t iPlugin ) const
{
	return	m_aPlugins.GetAt( iPlugin ) ;
}

// プラグイン OnStartScene 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::OnStartScene( const S3DSceneComposerPluginSceneInfo& scpsi )
{
	m_flagScene = true ;
	m_scpsiScene = scpsi ;

	for ( size_t i = 0; i < m_aPlugins.GetLength(); i ++ )
	{
		S3DSceneComposerPluginInstance *	pPlugin = m_aPlugins.GetAt(i) ;
		if ( pPlugin != nullptr )
		{
			pPlugin->OnStartScene( scpsi ) ;
		}
	}
}

// プラグイン OnEndScene 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::OnEndScene( void )
{
	for ( size_t i = 0; i < m_aPlugins.GetLength(); i ++ )
	{
		S3DSceneComposerPluginInstance *	pPlugin = m_aPlugins.GetAt(i) ;
		if ( pPlugin != nullptr )
		{
			pPlugin->OnEndScene() ;
		}
	}
	m_flagScene = false ;
}

// プラグイン OnStartComposition 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::OnStartComposition( S3DSceneComposer::Composition * pComposition )
{
	for ( size_t i = 0; i < m_aPlugins.GetLength(); i ++ )
	{
		S3DSceneComposerPluginInstance *	pPlugin = m_aPlugins.GetAt(i) ;
		if ( pPlugin != nullptr )
		{
			pPlugin->OnStartComposition( pComposition ) ;
		}
	}
}

// シーン情報取得
//////////////////////////////////////////////////////////////////////////////
bool S3DCompositionManager::GetSceneInfo( S3DSceneComposerPluginSceneInfo& scpsi ) const
{
	if ( m_flagScene )
	{
		scpsi = m_scpsiScene ;
	}
	return	m_flagScene ;
}

// コンポーザー取得（参照カウンタ不変）
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer * S3DCompositionManager::GetComposer( const wchar_t * pwszFileName ) const
{
	SString	strFileName = NormalizeComposerFileName( pwszFileName ) ;
	return	m_ssoaComposer.GetAs( strFileName ) ;
}

// コンポーザー登録（参照カウンタ不変）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCompositionManager::AddComposer
	( const wchar_t * pwszFileName, S3DSceneComposer * pComposer )
{
	SString	strFileName = NormalizeComposerFileName( pwszFileName ) ;
	//
	if ( m_ssoaComposer.GetAs( strFileName ) != nullptr )
	{
		pComposer->ReleaseRef() ;
		return	sglErrFailed ;
	}
	m_ssoaComposer.Add( strFileName, pComposer ) ;
	return	sglErrSuccess ;
}

// コンポーザー読み込み
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer * S3DCompositionManager::LoadComposer
	( const wchar_t * pwszFileName, SSystem::SFileInterface * pFile )
{
	SString				strFileName = NormalizeComposerFileName( pwszFileName ) ;
	S3DSceneComposer *	pComposer = m_ssoaComposer.GetAs( strFileName ) ;
	if ( pComposer != nullptr )
	{
		pComposer->AddReference() ;
		return	pComposer ;
	}
	if ( m_pParent != nullptr )
	{
		pComposer = m_pParent->GetComposer( pwszFileName ) ;
		if ( pComposer != nullptr )
		{
			pComposer->AddReference() ;
			return	pComposer ;
		}
	}
	SSmartPointer<SFileInterface>	pTempFile ;
	if ( pFile == nullptr )
	{
		pTempFile = SFileOpener::DefaultNewOpenFile
						( pwszFileName, SFileOpener::shareRead ) ;
		pFile = pTempFile ;
		if ( pFile == nullptr )
		{
			return	nullptr ;
		}
	}
	SString	strBaseDir = SString(pwszFileName).GetFileDirectoryPart() ;
	if ( (strBaseDir.GetLastAt(0) == L'\\')
		|| (strBaseDir.GetLastAt(0) == L'/') )
	{
		strBaseDir.ChopRight( 1 ) ;
	}
	pComposer = NewSceneComposer() ;
	if ( pComposer->ReadComposeFile( *pFile, strBaseDir ) )
	{
		delete	pComposer ;
		return	nullptr ;
	}
	m_ssoaComposer.Add( strFileName, pComposer ) ;
	return	pComposer ;
}

// コンポーザー解放（参照カウンタ減少）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DCompositionManager::UnloadComposer( S3DSceneComposer * pComposer )
{
	ssize_t	i = m_ssoaComposer.FindPtr( pComposer ) ;
	if ( i < 0 )
	{
		if ( m_pParent != nullptr )
		{
			return	m_pParent->UnloadComposer( pComposer ) ;
		}
		return	sglErrFailed ;
	}
	if ( pComposer->ReleaseRef() )
	{
		ESLVerify( m_ssoaComposer.DetachAt( (size_t) i ) == pComposer ) ;
		delete	pComposer ;
	}
	return	sglErrSuccess ;
}

// ファイル名判定用正規化（ディレクトリを含まない小文字ファイル名）
//////////////////////////////////////////////////////////////////////////////
SSystem::SString
	S3DCompositionManager::
		NormalizeComposerFileName( const wchar_t * pwszFileName )
{
	SString	strFilePath = pwszFileName ;
	SString	strFileName = strFilePath.GetFileNamePart() ;
	strFileName.MakeLower() ;
	return	strFileName ;
}

// ESLItemCreator 以外の ItemCreator を削除
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::ReleaseTempItemCreator
	( SSystem::SStrSortObjectArray
		<S3DSceneComposer::ItemCreator>& ssaCreators )
{
	for ( size_t i = 0; i < ssaCreators.GetLength(); i ++ )
	{
		if ( ESLTypeCast<S3DSceneComposer::ESLItemCreator>
									( ssaCreators.GetAt(i) ) == nullptr )
		{
			ssaCreators.RemoveAt( i -- ) ;
		}
	}
}

// アイテム・タイプ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddItemCreator
		( const wchar_t * pwszType,
				S3DSceneComposer::ItemCreator * pCreator )
{
	m_ssaItemCreators.SetAs( pwszType, pCreator ) ;
}

// コントローラー・タイプ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddControllerCreator
		( const wchar_t * pwszType,
				S3DSceneComposer::ItemCreator * pCreator )
{
	m_ssaCtrlCreators.SetAs( pwszType, pCreator ) ;
}

// リソースプロシージャ・タイプ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddResourceProcCreator
		( const wchar_t * pwszType,
				S3DSceneComposer::ItemCreator * pCreator )
{
	m_ssaRsrcProcCreators.SetAs( pwszType, pCreator ) ;
}

// プラグイン・メニュ項目を追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddDynamicPluginDescriptor
	( S3DSceneComposerDynamicPlugin::Descriptor* pDesc )
{
	if ( m_pDynamicPlugin == nullptr )
	{
		m_pDynamicPlugin = new S3DSceneComposerDynamicPlugin ;
		m_pDynamicPlugin->InitPlugin( this ) ;
		m_aPlugins.Add( m_pDynamicPlugin ) ;
	}
	m_pDynamicPlugin->AddDescriptor( pDesc ) ;
}

// アイテムディスクリプタ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddItemDescriptor
		( const S3DSceneComposer::ItemClassDescriptor* pDesc )
{
	ESLAssert( pDesc != nullptr ) ;
	ESLAssert( pDesc->pwszClassID != nullptr ) ;
	AddItemCreator( pDesc->pwszClassID, new S3DSceneComposer::ESLItemCreator( pDesc ) ) ;
}

// コントローラーディスクリプタ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddControllerDescriptor
		( const S3DSceneComposer::ItemClassDescriptor* pDesc )
{
	ESLAssert( pDesc != nullptr ) ;
	ESLAssert( pDesc->pwszClassID != nullptr ) ;
	AddControllerCreator( pDesc->pwszClassID, new S3DSceneComposer::ESLItemCreator( pDesc ) ) ;
}

// リソースプロシージャディスクリプタ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCompositionManager::AddResourceProcDescriptor
		( const S3DSceneComposer::ItemClassDescriptor* pDesc )
{
	ESLAssert( pDesc != nullptr ) ;
	ESLAssert( pDesc->pwszClassID != nullptr ) ;
	AddResourceProcCreator( pDesc->pwszClassID, new S3DSceneComposer::ESLItemCreator( pDesc ) ) ;
}

// S3DSceneComposer 生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer * S3DCompositionManager::NewSceneComposer( void )
{
	return	new S3DSceneComposer( this ) ;
}

// アイテム生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ItemSerializer *
	S3DCompositionManager::CreateSceneItem( const wchar_t * pwszType )
{
	S3DSceneComposer::ItemCreator *	pCreator = m_ssaItemCreators.GetAs( pwszType ) ;
	if ( pCreator != nullptr )
	{
		LSceneManagerCurrent	current( this ) ;
		return	ESLSmartCast<S3DSceneComposer::ItemSerializer>( pCreator->NewItem() ) ;
	}
	return	nullptr ;
}

// コントローラー生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::Controller *
	S3DCompositionManager::CreateController( const wchar_t * pwszClass )
{
	S3DSceneComposer::ItemCreator *	pCreator = m_ssaCtrlCreators.GetAs( pwszClass ) ;
	if ( pCreator != nullptr )
	{
		LSceneManagerCurrent	current( this ) ;
		return	ESLSmartCast<S3DSceneComposer::Controller>( pCreator->NewItem() ) ;
	}
	return	nullptr ;
}

// リソースプロシージャ生成
//////////////////////////////////////////////////////////////////////////////
S3DSceneComposer::ResourceProcedure *
	S3DCompositionManager::CreateResourceProc( const wchar_t * pwszClass )
{
	S3DSceneComposer::ItemCreator *	pCreator = m_ssaRsrcProcCreators.GetAs( pwszClass ) ;
	if ( pCreator != nullptr )
	{
		LSceneManagerCurrent	current( this ) ;
		return	ESLSmartCast<S3DSceneComposer::ResourceProcedure>( pCreator->NewItem() ) ;
	}
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// コンポーザー・エディター・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCompositionEditorInterface, ESLObject )

// プロパティ編集（UI表示反映・オートキーフレーム有効）
//////////////////////////////////////////////////////////////////////////////
bool S3DCompositionEditorInterface::EditProperty
	( S3DSceneComposer::Composition * pComp,
		S3DSceneComposer::ParameterProperty * pProp,
		S3DSceneComposer::ParameterType type,
		size_t iParam, const SSystem::SString& strValue,
		bool flagAddUndo, const wchar_t * pwszUndoText )
{
	switch ( type )
	{
	case	S3DSceneComposer::typeMatrix:
	case	S3DSceneComposer::typeRotation:
		{
			S3DDMatrix	mat ;
			if ( S3DSceneComposer::Parameter::ParseMatrix( mat, strValue ) )
			{
				EditMatrixProperty( pComp, pProp, iParam, mat, flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typePosition:
	case	S3DSceneComposer::typeDirection:
	case	S3DSceneComposer::typeZoom:
	case	S3DSceneComposer::typeColor:
		{
			S3DDVector	pos ;
			if ( S3DSceneComposer::Parameter::ParseVector( pos, strValue ) )
			{
				EditVectorProperty( pComp, pProp, iParam, pos, flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeScalar:
		{
			double	s ;
			if ( S3DSceneComposer::Parameter::ParseScalar( s, strValue ) )
			{
				EditScalarProperty( pComp, pProp, iParam, s, flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeInteger:
		{
			int32_t	n ;
			if ( S3DSceneComposer::Parameter::ParseInteger( n, strValue ) )
			{
				EditIntegerProperty( pComp, pProp, iParam, n, flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeBoolean:
		{
			bool	b ;
			if ( S3DSceneComposer::Parameter::ParseBoolean( b, strValue ) )
			{
				EditBooleanProperty( pComp, pProp, iParam, b, flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeMatrix4:
		{
			double	mat4[16] ;
			if ( S3DSceneComposer::Parameter::ParseVectorX( mat4, 16, strValue ) )
			{
				EditBinaryProperty
					( pComp, pProp, iParam, mat4, sizeof(mat4), flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeVector4:
		{
			double	vec4[4] ;
			if ( S3DSceneComposer::Parameter::ParseVectorX( vec4, 4, strValue ) )
			{
				EditBinaryProperty
					( pComp, pProp, iParam, vec4, sizeof(vec4), flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeVector2:
		{
			double	vec2[2] ;
			if ( S3DSceneComposer::Parameter::ParseVectorX( vec2, 2, strValue ) )
			{
				EditBinaryProperty
					( pComp, pProp, iParam, vec2, sizeof(vec2), flagAddUndo, pwszUndoText ) ;
				return	true ;
			}
		}
		break ;
	case	S3DSceneComposer::typeSelector:
	case	S3DSceneComposer::typeCommand:
	case	S3DSceneComposer::typePose:
	case	S3DSceneComposer::typeBinary:
		EditCommandProperty( pComp, pProp, iParam, strValue, flagAddUndo, pwszUndoText ) ;
		return	true ;
	default:
		break ;
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// エラー出力
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSceneDebugTracer, SParserErrorInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSceneDebugTracer::S3DSceneDebugTracer( S3DCompositionEditorInterface * pEditor )
	: m_pEditor( pEditor )
{
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DSceneDebugTracer::AttachCompositionEditor( S3DCompositionEditorInterface * pEditor )
{
	m_pEditor = pEditor ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void S3DSceneDebugTracer::OutputError
	( const SStringParser& ss, const wchar_t * pszError )
{
	if ( m_pEditor == nullptr )
	{
		return ;
	}
	SString	strLine ;
	size_t	nColNum = ss.GetIndex() ;
	size_t	nLineNum = ss.GetLineCharIndexOf( strLine, nColNum ) ;

	m_pEditor->OutputError
		( pszError, ss.GetFilePath(), (int) nLineNum, strLine ) ;

	SString	strMsg ;
	if ( strLine.IsEmpty() )
	{
		strMsg.Format( L"error:%s\n", pszError ) ;
	}
	else
	{
		strMsg.Format
			( L"error:%s\n%s(%d行 %d桁):%s\n",
				pszError, ss.GetFilePath(), nLineNum, nColNum, strLine.GetConstArray() ) ;
	}
	m_pEditor->OutputTraceLog( strMsg ) ;
}

// 警告出力
//////////////////////////////////////////////////////////////////////////////
void S3DSceneDebugTracer::OutputWarning
	( const SStringParser& ss, const wchar_t * pszWarning )
{
	if ( m_pEditor == nullptr )
	{
		return ;
	}
	SString	strLine ;
	size_t	nColNum = ss.GetIndex() ;
	size_t	nLineNum = ss.GetLineCharIndexOf( strLine, nColNum ) ;

	SString	strMsg ;
	if ( strLine.IsEmpty() )
	{
		strMsg.Format( L"%s\n", pszWarning ) ;
	}
	else
	{
		strMsg.Format
			( L"%s\n%s(%d行 %d桁):%s\n",
				pszWarning, ss.GetFilePath(), nLineNum, nColNum, strLine.GetConstArray() ) ;
	}
	m_pEditor->OutputTraceLog( strMsg ) ;
}

