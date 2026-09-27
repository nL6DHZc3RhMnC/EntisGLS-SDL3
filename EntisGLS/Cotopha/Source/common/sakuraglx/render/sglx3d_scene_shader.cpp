
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
//#include <sakuragl/sgl_opengl_context.h>
//#include <sakuragl/sgl_opengl_custom_shader.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// シェーダー共通処理
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	S3DCommonShaderController::m_aiTargetClass[16] =
{
	{ L"allways", -1 },
	{ L"backscape", S3DScene::classBackscape },
	{ L"scape", S3DScene::classScape },
	{ L"field", S3DScene::classField },
	{ L"static_item1", S3DScene::classStaticItem1 },
	{ L"static_item2", S3DScene::classStaticItem2 },
	{ L"dynamic_item1", S3DScene::classDynamicItem1 },
	{ L"dynamic_item2", S3DScene::classDynamicItem2 },
	{ L"dynamic_item3", S3DScene::classDynamicItem3 },
	{ L"effect_item", S3DScene::classEffectItem },
	{ L"layered_space", S3DScene::classLayeredSpace },
	{ L"layered_item1", S3DScene::classLayeredItem1 },
	{ L"layered_item2", S3DScene::classLayeredItem2 },
	{ L"effect1", S3DScene::classEffect1 },
	{ L"effect2", S3DScene::classEffect2 },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DCommonShaderController, Controller )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCommonShaderController::S3DCommonShaderController
		( const wchar_t * pwszClassID, int iClassTargetParam )
	: Controller( pwszClassID ), m_flagExceptShadow( true ),
		m_iClassTargetParam( iClassTargetParam ), m_clsTarget( -1 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorRenderContext ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCommonShaderController::~S3DCommonShaderController( void )
{
}

// ターゲットクラス・パラメータ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCommonShaderController::AddTargetClassParameterEntry( void )
{
	m_iClassTargetParam =
		(int) AddParameterEntry
			( L"target_class", S3DSceneComposer::typeSelector,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration
				| S3DSceneComposer::attrUIOnlyEnumeration,
				L"対象クラス", nullptr ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCommonShaderController::GetCommandParameter( size_t i ) const
{
	if ( i == m_iClassTargetParam )
	{
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiTargetClass, m_clsTarget ) ;
	}
	return	Controller::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCommonShaderController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	if ( i == m_iClassTargetParam )
	{
		m_clsTarget =
			(int) SXMLDocument::GetIntegerAsSymbolOf
						( m_aiTargetClass, pwszCmd, m_clsTarget ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DCommonShaderController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( i == m_iClassTargetParam )
	{
		for ( int j = 0; m_aiTargetClass[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString(m_aiTargetClass[j].pszSymbol) ) ;
		}
		return	true ;
	}
	return	false ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void S3DCommonShaderController::BeforeRenderModel
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	if ( m_flagExceptShadow
		&& (scene.GetCurrentRenderingStage() == S3DScene::renderingShadow) )
	{
		return ;
	}
	if ( (m_clsTarget != -1) && (clsItem != m_clsTarget) )
	{
		return ;
	}
	UpdateShader( scene, clsItem, render ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 標準シェーダー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DStandardShaderController::m_pwszShadingType
	[S3DStandardShaderController::shadingCount] =
{
	L"no_shade", L"gouraud", L"phong",
} ;

const uint64_t	S3DStandardShaderController::m_nShadingFlag
	[S3DStandardShaderController::shadingCount] =
{
	shadingMethodNothing,
	shadingMethodGouraud,
	shadingMethodPhong,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStandardShaderController, S3DCommonShaderController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DStandardShaderController, standard_shader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStandardShaderController::S3DStandardShaderController( void )
	: S3DCommonShaderController( m_ItemClassDescriptor.pwszClassID ),
		m_shading( shadingPhong )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorRenderContext ;
	//
	AddParameterEntry
		( L"shading_type", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"シェーディング", nullptr ) ;
	AddTargetClassParameterEntry() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DStandardShaderController::~S3DStandardShaderController( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DStandardShaderController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramShaderType:
		return	m_pwszShadingType[m_shading] ;
	}
	return	S3DCommonShaderController::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DStandardShaderController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	int	j ;
	switch ( i )
	{
	case	paramShaderType:
		for ( j = 0; j < shadingCount; j ++ )
		{
			if ( SString::Compare( pwszCmd, m_pwszShadingType[j] ) == 0 )
			{
				m_shading = (ShadingType) j ;
				break ;
			}
		}
		return ;
	}
	S3DCommonShaderController::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DStandardShaderController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	int	j ;
	switch ( i )
	{
	case	paramShaderType:
		for ( j = 0; j < shadingCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszShadingType[j] ) ) ;
		}
		return	true ;
	}
	return	S3DCommonShaderController::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DStandardShaderController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"標準シェーダー" ;
	}
	return	nullptr ;
}

// シェーダー設定
//////////////////////////////////////////////////////////////////////////////
void S3DStandardShaderController::UpdateShader
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DRenderContextInterface& render )
{
	render.SetShadingFlag
		( (render.GetShadingFlag() & ~shadingMethodMask)
								| m_nShadingFlag[m_shading] ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ユーザー・シェーダー・シリアライザ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DUserShaderSerializer::S3DUserShaderSerializer( size_t iFirstParam )
	: m_iShaderParamFirst( iFirstParam ),
		m_pUserShader( nullptr ),
		m_pDevice( nullptr ), m_pShader( nullptr ), m_secTimer( 0.0 )
{
}

// シェーダ―・プロパティ更新
//////////////////////////////////////////////////////////////////////////////
void S3DUserShaderSerializer::UpdateShaderProperty
	( S3DSceneComposer::Controller& prop,
		S3DSceneComposer * pComposer,
		const wchar_t * pwszShaderID,
		S3DSceneComposer::UserShader * pUserShader,
		uint32_t nAttrCategoryFlag, bool flagSwitchShader )
{
	m_strUserShaderID = pwszShaderID ;
	m_pUserShader = pUserShader ;
	m_pDevice = nullptr ;
	m_pShader = nullptr ;
	//
	prop.ChopParameterEntryLastAt( m_iShaderParamFirst - 1 ) ;
	if ( flagSwitchShader )
	{
		m_elements.RemoveAll() ;
	}
	m_uniformIndex.RemoveAll() ;
	if ( (pComposer == nullptr) || (m_pUserShader == nullptr) )
	{
		return ;
	}
	size_t	iElement = 0 ;
	for ( size_t i = 0; i < m_pUserShader->m_uniforms.GetLength(); i ++ )
	{
		m_uniformIndex.Add( iElement ) ;
		//
		S3DRenderDevice::UniformDescriptor *
				pDesc = m_pUserShader->m_uniforms.GetAt( i ) ;
		ESLAssert( pDesc != nullptr ) ;
		if ( pDesc == nullptr )
		{
			continue ;
		}
		SString	strNum ;
		for ( size_t j = 0; j < pDesc->m_count; j ++ )
		{
			//
			// 要素作成
			//
			UniformElement *	pue = new UniformElement ;
			pue->m_typeUniform = pDesc->m_type ;
			//
			if ( pDesc->m_count == 1 )
			{
				pue->m_idParam = L"ufid_" ;
				pue->m_idParam += pDesc->m_name ;
				pue->m_idDispName = pDesc->m_name ;
			}
			else
			{
				strNum.FromInteger( j ) ;
				//
				pue->m_idParam = L"ufeid_" ;
				pue->m_idParam += pDesc->m_name ;
				pue->m_idParam += L"_" ;
				pue->m_idParam += strNum ;
				//
				pue->m_idDispName = pDesc->m_name ;
				pue->m_idDispName += L"[" ;
				pue->m_idDispName += strNum ;
				pue->m_idDispName += L"]" ;
			}
			pue->m_iUniform = i ;
			pue->m_iIndex = j ;
			//
			// プロパティ追加
			//
			S3DSceneComposer::ParameterType
							type = S3DSceneComposer::typeInvalid ;
			const wchar_t *	pwszDispName = pue->m_idDispName ;
			const wchar_t *	pwszComment = nullptr ;
			uint32_t		nAttrFlags = nAttrCategoryFlag ;//S3DSceneComposer::attrCategory1 ;
			double			minRange = 0.0, maxRange = 1.0 ;
			//
			const SString *	pstrSubType =
							pDesc->m_xmlDesc.GetAttributeAs( L"sub_type" ) ;
			if ( pstrSubType != nullptr )
			{
				if ( *pstrSubType == L"position" )
				{
					type = S3DSceneComposer::typePosition ;
				}
				else if ( *pstrSubType == L"direction" )
				{
					type = S3DSceneComposer::typeDirection ;
				}
				else if ( *pstrSubType == L"zoom" )
				{
					type = S3DSceneComposer::typeZoom ;
				}
				else if ( *pstrSubType == L"rotation" )
				{
					type = S3DSceneComposer::typeRotation ;
				}
				else if ( *pstrSubType == L"color" )
				{
					type = S3DSceneComposer::typeColor ;
				}
				else if ( *pstrSubType == L"timer" )
				{
					pue->m_typeSpecial = typeTimerElement ;
				}
			}
			if ( type == S3DSceneComposer::typeInvalid )
			{
				const SXMLDocument *	pxmlSelector = nullptr ;
				const SXMLDocument *	pxmlRange = nullptr ;
				switch ( pDesc->m_type )
				{
				case	S3DCustomShader::uniformInt:
					type = S3DSceneComposer::typeInteger ;
					//
					pxmlSelector = pDesc->m_xmlDesc.GetElementTagAs( L"selector" ) ;
					if ( pxmlSelector != nullptr )
					{
						type = S3DSceneComposer::typeSelector ;
						nAttrFlags |= S3DSceneComposer::attrStringEnumeration
									| S3DSceneComposer::attrUIOnlyEnumeration ;
						//
						for ( size_t iOpt = 0;
							iOpt < pxmlSelector->GetElementsCount(); iOpt ++ )
						{
							const SXMLDocument *
								pxmlEntry = pxmlSelector->GetElementAt( iOpt ) ;
							if ( (pxmlEntry == nullptr)
								|| (pxmlEntry->GetTag() != L"enum") )
							{
								continue ;
							}
							const SString *	pstrName = pxmlEntry->GetAttributeAs( L"name" ) ;
							if ( pstrName != nullptr )
							{
								SString *	pstrNameBuf = new SString( *pstrName ) ;
								SXMLDocument::AttrInteger	ai ;
								ai.nValue = pxmlEntry->GetAttrIntegerAs( L"num" ) ;
								ai.pszSymbol = *pstrNameBuf ;
								pue->m_aStrIntPairs.Add( ai ) ;
								pue->m_aStrBuffers.Add( pstrNameBuf ) ;
							}
						}
						SXMLDocument::AttrInteger	aiNull ;
						aiNull.nValue = 0 ;
						aiNull.pszSymbol = nullptr ;
						pue->m_aStrIntPairs.Add( aiNull ) ;
					}
					break ;

				case	S3DCustomShader::uniformFloat:
					type = S3DSceneComposer::typeScalar ;
					//
					pxmlRange = pDesc->m_xmlDesc.GetElementTagAs( L"range" ) ;
					if ( pxmlRange != nullptr )
					{
						nAttrFlags |= S3DSceneComposer::attrUIScalarSlider ;
						minRange = pxmlRange->GetAttrRealAs( L"min", 0.0 ) ;
						maxRange = pxmlRange->GetAttrRealAs( L"max", 0.0 ) ;
					}
					break ;

				case	S3DCustomShader::uniformVector2D:
					type = S3DSceneComposer::typeVector2 ;
					break ;

				case	S3DCustomShader::uniformVector3D:
					type = S3DSceneComposer::typePosition ;
					break ;

				case	S3DCustomShader::uniformVector4D:
					type = S3DSceneComposer::typeVector4 ;
					break ;

				case	S3DCustomShader::uniformMatrix2x2:
					type = S3DSceneComposer::typeMatrix ;
					break ;

				case	S3DCustomShader::uniformMatrix3x3:
					type = S3DSceneComposer::typeMatrix ;
					break ;

				case	S3DCustomShader::uniformMatrix4x4:
					type = S3DSceneComposer::typeMatrix4 ;
					break ;

				case	S3DCustomShader::uniformTexture:
				case	S3DCustomShader::uniformImageRead:
				case	S3DCustomShader::uniformImageWrite:
				case	S3DCustomShader::uniformImageReadWrite:
					type = S3DSceneComposer::typeSelector ;
					nAttrFlags |= S3DSceneComposer::attrStringEnumeration ;
					break ;

				default:
					break ;
				}
			}
			if ( type == S3DSceneComposer::typeInvalid )
			{
				delete	pue ;
				continue ;
			}
			//
			// 要素追加
			//
			pue->m_typeProp = type ;
			//
			bool	flagInitValue = false ;
			if ( !flagSwitchShader )
			{
				UniformElement *	pueOld = m_elements.GetAt( iElement ) ;
				if ( (pueOld != nullptr)
					&& (pueOld->m_typeUniform == pue->m_typeUniform)
					&& (pueOld->m_idDispName == pue->m_idDispName)
					&& (pueOld->m_iIndex == pue->m_iIndex) )
				{
					pue->m_value = pueOld->m_value ;
					pue->m_idTexture = pueOld->m_idTexture ;
					flagInitValue = true ;
				}
				m_elements.SetAt( iElement, pue ) ;
			}
			else
			{
				m_elements.Add( pue ) ;
			}
			iElement ++ ;
			//
			const SString *	pstrFriendlyName = pDesc->m_xmlDesc.GetTextElementAs( L"name" ) ;
			if ( pstrFriendlyName != nullptr )
			{
				pwszDispName = *pstrFriendlyName ;
			}
			const SString *	pstrDesc = pDesc->m_xmlDesc.GetTextElementAs( L"desc" ) ;
			if ( pstrDesc != nullptr )
			{
				pwszComment = *pstrDesc ;
			}
			prop.AddParameterEntry
				( pue->m_idParam, type,
					nAttrFlags, pwszDispName,
					pwszComment, minRange, maxRange ) ;
			//
			// 初期値設定
			//
			const SString *	pstrDefault = pDesc->m_xmlDesc.GetTextElementAs( L"default" ) ;
			if ( flagInitValue )
			{
				if ( S3DCustomShader::IsUniformTypeTexture(pue->m_typeUniform) )
				{
					UpdateTextureReference( pComposer, pue ) ;
				}
			}
			else if ( pstrDefault != nullptr )
			{
				switch ( type )
				{
				case	S3DSceneComposer::typeMatrix:
				case	S3DSceneComposer::typeRotation:
					S3DSceneComposer::Parameter::ParseMatrix
						( pue->m_value.Matrix3D(), *pstrDefault ) ;
					break ;
				case	S3DSceneComposer::typePosition:
				case	S3DSceneComposer::typeDirection:
				case	S3DSceneComposer::typeZoom:
				case	S3DSceneComposer::typeColor:
					S3DSceneComposer::Parameter::ParseVector
						( pue->m_value.Vector3D(), *pstrDefault ) ;
					break ;
				case	S3DSceneComposer::typeScalar:
					S3DSceneComposer::Parameter::ParseScalar
						( pue->m_value.Float64(), *pstrDefault ) ;
					break ;
				case	S3DSceneComposer::typeInteger:
					S3DSceneComposer::Parameter::ParseInteger
						( pue->m_value.Int32(), *pstrDefault ) ;
					break ;
				case	S3DSceneComposer::typeMatrix4:
					S3DSceneComposer::Parameter::ParseVectorX
						( &(pue->m_value.Matrix4D().m[0][0]), 16, *pstrDefault ) ;
					break ;
				case	S3DSceneComposer::typeVector4:
					S3DSceneComposer::Parameter::ParseVectorX
						( &(pue->m_value.Vector4D().x), 4, *pstrDefault ) ;
					break ;
				case	S3DSceneComposer::typeVector2:
					S3DSceneComposer::Parameter::ParseVectorX
						( &(pue->m_value.Vector2D().x), 2, *pstrDefault ) ;
					break ;
				default:
					break ;
				}
			}
			else
			{
				switch ( type )
				{
				case	S3DSceneComposer::typeMatrix:
				case	S3DSceneComposer::typeRotation:
					pue->m_value.Matrix3D() = S3DDMatrix( 1, 1, 1 ) ;
					break ;
				case	S3DSceneComposer::typePosition:
					pue->m_value.Vector3D() = S3DDVector( 0, 0, 0 ) ;
					break ;
				case	S3DSceneComposer::typeDirection:
					pue->m_value.Vector3D() = S3DDVector( 0, 0, 1 ) ;
					break ;
				case	S3DSceneComposer::typeZoom:
					pue->m_value.Vector3D() = S3DDVector( 1, 1, 1 ) ;
					break ;
				case	S3DSceneComposer::typeColor:
					pue->m_value.Vector3D() = S3DDVector( 1, 1, 1 ) ;
					break ;
				case	S3DSceneComposer::typeScalar:
					pue->m_value.Float64() = 0.0 ;
					break ;
				case	S3DSceneComposer::typeInteger:
					pue->m_value.Int32() = 0 ;
					break ;
				case	S3DSceneComposer::typeMatrix4:
					pue->m_value.Matrix4D() = S4DDMatrix( 1, 1, 1, 1 ) ;
					break ;
				case	S3DSceneComposer::typeVector4:
					pue->m_value.Vector4D() = S4DDVector( 0, 0, 0, 1 ) ;
					break ;
				case	S3DSceneComposer::typeVector2:
					pue->m_value.Vector2D() = S2DDVector( 0, 0 ) ;
					break ;
				default:
					break ;
				}
			}
		}
	}
	m_elements.SetLength( iElement ) ;
}

// テクスチャ参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DUserShaderSerializer::UpdateTextureReference
	( S3DSceneComposer * pComposer, UniformElement * pue )
{
	if ( pComposer && pue )
	{
		pue->m_pTexture =
			pComposer->GetAssets().GetImageAs( pue->m_idTexture ) ;
	}
}

// パラメータ保存
//////////////////////////////////////////////////////////////////////////////
void S3DUserShaderSerializer::SaveShaderParameters
	( S3DSceneComposer::ParameterProperty& prop,
		SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	const size_t	nParamCount = prop.GetParameterCount() ;
	for ( size_t iParam = m_iShaderParamFirst; iParam < nParamCount; iParam ++ )
	{
		const wchar_t *	pwszID = prop.GetParameterID( iParam ) ;
		if ( pwszID == nullptr )
		{
			continue ;
		}
		S3DSceneComposer::ParameterEntryStorage *
				ppes = new S3DSceneComposer::ParameterEntryStorage ;
		prop.SaveParameterEntryAt( *ppes, iParam ) ;
		ssoaParam.SetAs( pwszID, ppes ) ;
	}
}

// パラメータ復帰
//////////////////////////////////////////////////////////////////////////////
void S3DUserShaderSerializer::ResotreShaderParameters
	( S3DSceneComposer::ParameterProperty& prop,
		const SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	for ( size_t i = 0; i < ssoaParam.GetLength(); i ++ )
	{
		const SString *	pstrTag = ssoaParam.GetTagAt( i ) ;
		ESLAssert( pstrTag != nullptr ) ;
		//
		ssize_t	iParam = prop.FindParameterID( *pstrTag ) ;
		if ( iParam < 0 )
		{
			continue ;
		}
		S3DSceneComposer::ParameterEntryStorage *
								ppes = ssoaParam.GetAt( i ) ;
		if ( ppes != nullptr )
		{
			prop.RestoreParameterEntryAt( iParam, *ppes ) ;
		}
	}
}

// シェーダー設定
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderSerializer::AttachShaderParamTo
	( S3DRenderContextInterface& render, S3DSceneComposer * pComposer )
{
	if ( m_pUserShader == nullptr )
	{
		return ;
	}
	S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
	if ( pDevice == nullptr )
	{
		return ;
	}
	if ( (m_pShader == nullptr) || (m_pDevice != pDevice) )
	{
		SString				strErrMsg, strErrSrc ;
		S3DCustomShader *	pShader =
					m_pUserShader->LoadShaderFor( pDevice, &strErrMsg, &strErrSrc ) ;
		render.AttachCustomShader( pShader ) ;
		if ( pShader == nullptr )
		{
			if ( !strErrMsg.IsEmpty() && (pComposer != nullptr) )
			{
				SString	strErr ;
				strErr.Format( L"failed to compile shader \'%s\'",
									(const wchar_t*) m_strUserShaderID ) ;
				pComposer->OutputError( strErr ) ;
				pComposer->OutputTraceLog
					( SString(L"\n--------error--------\n") + strErrMsg
							+ L"--------source--------\n" + strErrSrc
							+ L"--------end of source--------\n" ) ;
			}
			return ;
		}
		m_pDevice = pDevice ;
		m_pShader = pShader ;
	}
	else
	{
		render.AttachCustomShader( m_pShader ) ;
	}
	for ( size_t i = 0; i < m_uniformIndex.GetLength(); i ++ )
	{
		const size_t		iElement = m_uniformIndex.At(i) ;
		UniformElement *	pue = m_elements.GetAt( iElement ) ;
		ESLAssert( pue != nullptr ) ;
		if ( pue == nullptr )
		{
			continue ;
		}
		ESLAssert( pue->m_iIndex == 0 ) ;
		S3DRenderDevice::UniformDescriptor *
				pDesc = m_pUserShader->m_uniforms.GetAt( pue->m_iUniform ) ;
		ESLAssert( pDesc != nullptr ) ;
		if ( pDesc == nullptr )
		{
			continue ;
		}
		ESLAssert( pue->m_typeUniform == pDesc->m_type ) ;
		if ( pDesc->m_count == 1 )
		{
			float32_t	fpValue ;
			S2DVector	v2dValue ;
			S3DVector	v3dValue ;
			S4DVector	v4dValue ;
			S3DMatrix	mat3Value ;
			S4DMatrix	mat4Value ;
			//
			switch ( pue->m_typeUniform )
			{
			case	S3DCustomShader::uniformInt:
				render.SetCustomShaderUniformInt
					( pDesc->m_name, &(pue->m_value.Int32()), 1 ) ;
				break ;
			case	S3DCustomShader::uniformFloat:
				fpValue = (float32_t) pue->m_value.Float64() ;
				if ( pue->m_typeSpecial == typeTimerElement )
				{
					fpValue += (float32_t) m_secTimer ;
				}
				render.SetCustomShaderUniformFloat
					( pDesc->m_name, &fpValue, 1 ) ;
				break ;
			case	S3DCustomShader::uniformVector2D:
				v2dValue = pue->m_value.Vector2D() ;
				render.SetCustomShaderUniformVector2D
					( pDesc->m_name, &v2dValue, 1 ) ;
				break ;
			case	S3DCustomShader::uniformVector3D:
				v3dValue = pue->m_value.Vector3D() ;
				render.SetCustomShaderUniformVector3D
					( pDesc->m_name, &v3dValue, 1 ) ;
				break ;
			case	S3DCustomShader::uniformVector4D:
				v4dValue = pue->m_value.Vector4D() ;
				render.SetCustomShaderUniformVector4D
					( pDesc->m_name, &v4dValue, 1 ) ;
				break ;
			case	S3DCustomShader::uniformMatrix2x2:
				break ;
			case	S3DCustomShader::uniformMatrix3x3:
				mat3Value = pue->m_value.Matrix3D() ;
				render.SetCustomShaderUniformMatrix3x3
					( pDesc->m_name, &mat3Value, 1 ) ;
				break ;
			case	S3DCustomShader::uniformMatrix4x4:
				mat4Value = pue->m_value.Matrix4D() ;
				render.SetCustomShaderUniformMatrix4x4
					( pDesc->m_name, &mat4Value, 1 ) ;
				break ;
			case	S3DCustomShader::uniformTexture:
			case	S3DCustomShader::uniformImageRead:
			case	S3DCustomShader::uniformImageWrite:
			case	S3DCustomShader::uniformImageReadWrite:
				render.SetCustomShaderUniformTexture
					( pDesc->m_name, pue->m_pTexture ) ;
				break ;
			default:
				break ;
			}
		}
		else
		{
			SArray<int32_t>		aInt ;
			SArray<float32_t>	aFloat ;
			SArray<S2DVector>	aVec2 ;
			SArray<S3DVector>	aVec3 ;
			SArray<S4DVector>	aVec4 ;
			SArray<S3DMatrix>	aMat3 ;
			SArray<S4DMatrix>	aMat4 ;
			size_t				j ;
			//
			switch ( pue->m_typeUniform )
			{
			case	S3DCustomShader::uniformInt:
				for ( j = 0; j < pDesc->m_count; j ++ )
				{
					pue = m_elements.GetAt( m_uniformIndex.At(iElement + j) ) ;
					ESLAssert( pue != nullptr ) ;
					if ( pue != nullptr )
					{
						aInt.Add( pue->m_value.Int32() ) ;
					}
				}
				render.SetCustomShaderUniformInt
					( pDesc->m_name, aInt.GetConstArray(), aInt.GetLength() ) ;
				break ;
			case	S3DCustomShader::uniformFloat:
				for ( j = 0; j < pDesc->m_count; j ++ )
				{
					pue = m_elements.GetAt( m_uniformIndex.At(iElement + j) ) ;
					ESLAssert( pue != nullptr ) ;
					if ( pue != nullptr )
					{
						aFloat.Add( (float32_t) pue->m_value.Float64() ) ;
					}
				}
				render.SetCustomShaderUniformFloat
					( pDesc->m_name, aFloat.GetConstArray(), aFloat.GetLength() ) ;
				break ;
			case	S3DCustomShader::uniformVector2D:
				for ( j = 0; j < pDesc->m_count; j ++ )
				{
					pue = m_elements.GetAt( m_uniformIndex.At(iElement + j) ) ;
					ESLAssert( pue != nullptr ) ;
					if ( pue != nullptr )
					{
						aVec2.Add( pue->m_value.Vector2D() ) ;
					}
				}
				render.SetCustomShaderUniformVector2D
					( pDesc->m_name, aVec2.GetConstArray(), aVec2.GetLength() ) ;
				break ;
			case	S3DCustomShader::uniformVector3D:
				for ( j = 0; j < pDesc->m_count; j ++ )
				{
					pue = m_elements.GetAt( m_uniformIndex.At(iElement + j) ) ;
					ESLAssert( pue != nullptr ) ;
					if ( pue != nullptr )
					{
						aVec3.Add( pue->m_value.Vector3D() ) ;
					}
				}
				render.SetCustomShaderUniformVector3D
					( pDesc->m_name, aVec3.GetConstArray(), aVec3.GetLength() ) ;
				break ;
			case	S3DCustomShader::uniformVector4D:
				for ( j = 0; j < pDesc->m_count; j ++ )
				{
					pue = m_elements.GetAt( m_uniformIndex.At(iElement + j) ) ;
					ESLAssert( pue != nullptr ) ;
					if ( pue != nullptr )
					{
						aVec4.Add( pue->m_value.Vector4D() ) ;
					}
				}
				render.SetCustomShaderUniformVector4D
					( pDesc->m_name, aVec4.GetConstArray(), aVec4.GetLength() ) ;
				break ;
			case	S3DCustomShader::uniformMatrix2x2:
				break ;
			case	S3DCustomShader::uniformMatrix3x3:
				for ( j = 0; j < pDesc->m_count; j ++ )
				{
					pue = m_elements.GetAt( m_uniformIndex.At(iElement + j) ) ;
					ESLAssert( pue != nullptr ) ;
					if ( pue != nullptr )
					{
						aMat3.Add( pue->m_value.Matrix3D() ) ;
					}
				}
				render.SetCustomShaderUniformMatrix3x3
					( pDesc->m_name, aMat3.GetConstArray(), aMat3.GetLength() ) ;
				break ;
			case	S3DCustomShader::uniformMatrix4x4:
				for ( j = 0; j < pDesc->m_count; j ++ )
				{
					pue = m_elements.GetAt( m_uniformIndex.At(iElement + j) ) ;
					ESLAssert( pue != nullptr ) ;
					if ( pue != nullptr )
					{
						aMat4.Add( pue->m_value.Matrix4D() ) ;
					}
				}
				render.SetCustomShaderUniformMatrix4x4
					( pDesc->m_name, aMat4.GetConstArray(), aMat4.GetLength() ) ;
				break ;
			case	S3DCustomShader::uniformTexture:
			case	S3DCustomShader::uniformImageRead:
			case	S3DCustomShader::uniformImageWrite:
			case	S3DCustomShader::uniformImageReadWrite:
				render.SetCustomShaderUniformTexture
					( pDesc->m_name, pue->m_pTexture ) ;
				break ;
			default:
				break ;
			}
		}
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderSerializer::OnTimer( double secPast )
{
	m_secTimer += secPast ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DUserShaderSerializer::GetMatrixParameter( size_t iParam ) const
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		return	pue->m_value.Matrix3D() ;
	}
	return	S3DDMatrix( 0, 0, 0 ) ;
}

S3DDVector S3DUserShaderSerializer::GetVectorParameter( size_t iParam ) const
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		if ( pue->m_typeProp == S3DSceneComposer::typeColor )
		{
			return	pue->m_value.Vector3D() * 255.0 ;
		}
		return	pue->m_value.Vector3D() ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DUserShaderSerializer::GetScalarParameter( size_t iParam ) const
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		return	pue->m_value.Float64() ;
	}
	return	0.0 ;
}

int32_t S3DUserShaderSerializer::GetIntegerParameter( size_t iParam ) const
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		return	pue->m_value.Int32() ;
	}
	return	0 ;
}

const wchar_t * S3DUserShaderSerializer::GetCommandParameter( size_t iParam ) const
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		if ( pue->m_typeUniform == S3DCustomShader::uniformInt )
		{
			SXMLDocument::AttrInteger *	paiPair = 
				pue->m_aStrIntPairs.GetAt( (size_t) pue->m_value.Int32() ) ;
			if ( paiPair != nullptr )
			{
				return	paiPair->pszSymbol ;
			}
		}
		else if ( S3DCustomShader::IsUniformTypeTexture(pue->m_typeUniform) )
		{
			return	pue->m_idTexture ;
		}
	}
	return	nullptr ;
}

size_t S3DUserShaderSerializer::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t iParam ) const
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		if ( pDst == nullptr )
		{
			switch ( pue->m_typeProp )
			{
			case	S3DSceneComposer::typeMatrix4:
				return	sizeof(S4DDMatrix) ;
			case	S3DSceneComposer::typeVector4:
				return	sizeof(S4DDVector) ;
			case	S3DSceneComposer::typeVector2:
				return	sizeof(S2DDVector) ;
			default:
				return	0 ;
			}
		}
		else
		{
			switch ( pue->m_typeProp )
			{
			case	S3DSceneComposer::typeMatrix4:
				if ( nBufBytes == sizeof(S4DDMatrix) )
				{
					*((S4DDMatrix*)pDst) = pue->m_value.Matrix4D() ;
					return	sizeof(S4DDMatrix) ;
				}
				break ;
			case	S3DSceneComposer::typeVector4:
				if ( nBufBytes == sizeof(S4DDVector) )
				{
					*((S4DDVector*)pDst) = pue->m_value.Vector4D() ;
					return	sizeof(S4DDVector) ;
				}
				break ;
			case	S3DSceneComposer::typeVector2:
				if ( nBufBytes == sizeof(S2DDVector) )
				{
					*((S2DDVector*)pDst) = pue->m_value.Vector2D() ;
					return	sizeof(S2DDVector) ;
				}
				break ;
			default:
				break ;
			}
		}
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderSerializer::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		pue->m_value.Matrix3D() = mat ;
	}
}

void S3DUserShaderSerializer::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		if ( pue->m_typeUniform == S3DCustomShader::uniformVector4D )
		{
			pue->m_value.Vector4D() = S4DDVector( vec, 1 ) ;
		}
		else if ( pue->m_typeProp == S3DSceneComposer::typeColor )
		{
			pue->m_value.Vector3D() = vec * (1.0 / 255.0) ;
		}
		else
		{
			pue->m_value.Vector3D() = vec ;
		}
	}
}

void S3DUserShaderSerializer::SetScalarParameter( size_t iParam, double s )
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		pue->m_value.Float64() = s ;
	}
}

void S3DUserShaderSerializer::SetIntegerParameter( size_t iParam, int32_t n )
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		pue->m_value.Int32() = n ;
	}
}

void S3DUserShaderSerializer::SetCommandParameter
	( S3DSceneComposer * pComposer, size_t iParam, const wchar_t * pwszCmd )
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		if ( pue->m_typeUniform == S3DCustomShader::uniformInt )
		{
			if ( pue->m_aStrIntPairs.GetLength() >= 2 )
			{
				pue->m_value.Int32() =
					(int32_t) SXMLDocument::GetIntegerAsSymbolOf
						( pue->m_aStrIntPairs.GetConstArray(),
								pwszCmd, pue->m_value.Int32() ) ;
				return ;
			}
		}
		else if ( S3DCustomShader::IsUniformTypeTexture(pue->m_typeUniform) )
		{
			if ( pue->m_idTexture != pwszCmd )
			{
				pue->m_idTexture = pwszCmd ;
				UpdateTextureReference( pComposer, pue ) ;
			}
			return ;
		}
	}
}

size_t S3DUserShaderSerializer::SetBinaryParameter
	( size_t iParam, const void * pSrc, size_t nBufBytes )
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue != nullptr )
	{
		switch ( pue->m_typeProp )
		{
		case	S3DSceneComposer::typeMatrix4:
			if ( nBufBytes == sizeof(S4DDMatrix) )
			{
				pue->m_value.Matrix4D() = *((S4DDMatrix*)pSrc) ;
				return	sizeof(S4DDMatrix) ;
			}
			break ;
		case	S3DSceneComposer::typeVector4:
			if ( nBufBytes == sizeof(S4DDVector) )
			{
				pue->m_value.Vector4D() = *((S4DDVector*)pSrc) ;
				return	sizeof(S4DDVector) ;
			}
			break ;
		case	S3DSceneComposer::typeVector2:
			if ( nBufBytes == sizeof(S2DDVector) )
			{
				pue->m_value.Vector2D() = *((S2DDVector*)pSrc) ;
				return	sizeof(S2DDVector) ;
			}
			break ;
		default:
			break ;
		}
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////
bool S3DUserShaderSerializer::EnumerateStringSet
	( S3DSceneComposer * pComposer,
		size_t iParam, SSystem::SStringArray& aStrSet )
{
	UniformElement *	pue = m_elements.GetAt( iParam - m_iShaderParamFirst ) ;
	if ( pue == nullptr )
	{
		return	false ;
	}
	if ( pue->m_typeUniform == S3DCustomShader::uniformInt )
	{
		for ( size_t j = 0; j < pue->m_aStrIntPairs.GetLength(); j ++ )
		{
			SXMLDocument::AttrInteger *	pai = pue->m_aStrIntPairs.GetAt( j ) ;
			if ( pai && pai->pszSymbol )
			{
				aStrSet.Add( new SString(pai->pszSymbol) ) ;
			}
		}
		return	(pue->m_aStrIntPairs.GetLength() >= 2) ;
	}
	else if ( S3DCustomShader::IsUniformTypeTexture(pue->m_typeUniform) )
	{
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateTextureStringSet( aStrSet ) ;
			return	true ;
		}
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////
// ユーザー・シェーダー
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DUserShaderController, S3DCommonShaderController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DUserShaderController, user_shader )

// 構築関数
//////////////////////////////////////////////////////////////////////////
S3DUserShaderController::S3DUserShaderController( void )
	: S3DCommonShaderController( m_ItemClassDescriptor.pwszClassID ),
		S3DUserShaderSerializer( paramFirstUniform )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorRenderContext
						| S3DSceneComposer::behaviorOnTimer ;
	//
	AddParameterEntry
		( L"shader_id", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"シェーダ―", nullptr ) ;
	AddTargetClassParameterEntry() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
S3DUserShaderController::~S3DUserShaderController( void )
{
}

// シェーダ―更新
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderController::UpdateShaderReference( bool flagSwitchShader )
{
	S3DSceneComposer *				pComposer = GetComposer() ;
	S3DSceneComposer::UserShader *	pUserShader = nullptr ;
	if ( (pComposer != nullptr) && !m_strPropShaderID.IsEmpty() )
	{
		pUserShader = pComposer->GetAssets().GetUserShaderAs( m_strPropShaderID ) ;
	}
	UpdateShaderProperty
		( *this, pComposer, m_strPropShaderID,
			pUserShader, S3DSceneComposer::attrCategory1, flagSwitchShader ) ;
}

// パラメータ保存
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderController::SaveShaderParameters
	( SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	S3DUserShaderSerializer::SaveShaderParameters( *this, ssoaParam ) ;
}

// パラメータ復帰
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderController::ResotreShaderParameters
	( const SSystem::SStrSortObjectArray
			<S3DSceneComposer::ParameterEntryStorage>& ssoaParam )
{
	S3DUserShaderSerializer::ResotreShaderParameters( *this, ssoaParam ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DUserShaderController::GetMatrixParameter( size_t iParam ) const
{
	return	S3DUserShaderSerializer::GetMatrixParameter( iParam ) ;
}

S3DDVector S3DUserShaderController::GetVectorParameter( size_t iParam ) const
{
	return	S3DUserShaderSerializer::GetVectorParameter( iParam ) ;
}

double S3DUserShaderController::GetScalarParameter( size_t iParam ) const
{
	return	S3DUserShaderSerializer::GetScalarParameter( iParam ) ;
}

int32_t S3DUserShaderController::GetIntegerParameter( size_t iParam ) const
{
	return	S3DUserShaderSerializer::GetIntegerParameter( iParam ) ;
}

const wchar_t * S3DUserShaderController::GetCommandParameter( size_t iParam ) const
{
	if ( iParam == paramShaderID )
	{
		return	m_strPropShaderID ;
	}
	else if ( iParam == paramTargetClass )
	{
		return	S3DCommonShaderController::GetCommandParameter( iParam ) ;
	}
	return	S3DUserShaderSerializer::GetCommandParameter( iParam ) ;
}

size_t S3DUserShaderController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t iParam ) const
{
	return	S3DUserShaderSerializer::GetBinaryParameter( pDst, nBufBytes, iParam ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderController::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	S3DUserShaderSerializer::SetMatrixParameter( iParam, mat ) ;
}

void S3DUserShaderController::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	S3DUserShaderSerializer::SetVectorParameter( iParam, vec ) ;
}

void S3DUserShaderController::SetScalarParameter( size_t iParam, double s )
{
	S3DUserShaderSerializer::SetScalarParameter( iParam, s ) ;
}

void S3DUserShaderController::SetIntegerParameter( size_t iParam, int32_t n )
{
	S3DUserShaderSerializer::SetIntegerParameter( iParam, n ) ;
}

void S3DUserShaderController::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	if ( iParam == paramShaderID )
	{
		if ( m_strPropShaderID != pwszCmd )
		{
			m_strPropShaderID = pwszCmd ;
			UpdateShaderReference( true ) ;
		}
		return ;
	}
	else if ( iParam == paramTargetClass )
	{
		S3DCommonShaderController::SetCommandParameter( iParam, pwszCmd ) ;
		return ;
	}
	S3DUserShaderSerializer::SetCommandParameter( GetComposer(), iParam, pwszCmd ) ;
}

size_t S3DUserShaderController::SetBinaryParameter
	( size_t iParam, const void * pSrc, size_t nBufBytes )
{
	return	S3DUserShaderSerializer::SetBinaryParameter( iParam, pSrc, nBufBytes ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////
bool S3DUserShaderController::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	if ( iParam == paramShaderID )
	{
		S3DSceneComposer *	pComposer = GetComposer() ;
		if ( pComposer != nullptr )
		{
			pComposer->GetAssets().EnumerateResourceIDsAs
				( aStrSet, S3DSceneComposer::resourceTypeShaderDef ) ;
		}
		return	true ;
	}
	else if ( iParam == paramTargetClass )
	{
		return	S3DCommonShaderController::EnumerateStringSet( iParam, aStrSet ) ;
	}
	return	S3DUserShaderSerializer::EnumerateStringSet( GetComposer(), iParam, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////
const wchar_t * S3DUserShaderController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
	default:
		return	L"基本設定" ;
	case	1:
		return	L"シェーダ―設定" ;
	}
	return	nullptr ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////
uint32_t S3DUserShaderController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefResource )
	{
		SStrSortObjectArray<S3DSceneComposer::ParameterEntryStorage>	ssoaParam ;
		SaveShaderParameters( ssoaParam ) ;
		//
		UpdateShaderReference( false ) ;
		//
		ResotreShaderParameters( ssoaParam ) ;
	}
	return	nResFlags ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;

	S3DUserShaderSerializer::OnTimer( (double) msecPast / 1000.0 ) ;
}

// シェーダー設定
//////////////////////////////////////////////////////////////////////////
void S3DUserShaderController::UpdateShader
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DRenderContextInterface& render )
{
	AttachShaderParamTo( render, GetComposer() ) ;
}




//////////////////////////////////////////////////////////////////////////////
// ｚバッファ書き込み制御
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DShadowDepthFuncController, Controller, MultiRenderer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DShadowDepthFuncController, depth_func )

const SSystem::SXMLDocument::AttrInteger
	S3DShadowDepthFuncController::m_aiDepthFunc[6] =
{
	{ L"default", S3DRenderContext::depthMaskDefault },
	{ L"write", S3DRenderContext::depthMaskEnable },
	{ L"compare", S3DRenderContext::depthMaskNoWrite },
	{ L"compare_gt", S3DRenderContext::depthMaskNoWriteGT },
	{ L"no_test", S3DRenderContext::depthMaskNoTest },
	{ nullptr, 0 },
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DShadowDepthFuncController::S3DShadowDepthFuncController( void )
	: Controller( m_ItemClassDescriptor.pwszClassID )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorRenderContext ;
	m_depthFunc[0] = S3DRenderContext::depthMaskDefault ;
	m_depthFunc[1] = S3DRenderContext::depthMaskDefault ;
	m_depthFunc[2] = S3DRenderContext::depthMaskDefault ;
	m_depthFunc[3] = S3DRenderContext::depthMaskDefault ;
	m_depthFunc[4] = S3DRenderContext::depthMaskDefault ;
	//
	PrepareParameterEntryCount( paramCount ) ;
	ESLVerify( 	AddParameterEntry
				( L"main_depth", S3DSceneComposer::typeSelector,
					S3DSceneComposer::attrConstant
					| S3DSceneComposer::attrStringEnumeration
					| S3DSceneComposer::attrUIOnlyEnumeration,
					L"メイン深度処理", nullptr ) == paramMainDepthFunc ) ;
	ESLVerify( 	AddParameterEntry
				( L"shadow_depth", S3DSceneComposer::typeSelector,
					S3DSceneComposer::attrConstant
					| S3DSceneComposer::attrStringEnumeration
					| S3DSceneComposer::attrUIOnlyEnumeration,
					L"シャドウ深度処理", nullptr ) == paramShadowDepthFunc ) ;
	ESLVerify( 	AddParameterEntry
				( L"environment_depth", S3DSceneComposer::typeSelector,
					S3DSceneComposer::attrConstant
					| S3DSceneComposer::attrStringEnumeration
					| S3DSceneComposer::attrUIOnlyEnumeration,
					L"環境深度処理", nullptr ) == paramEnvironmentDepthFunc ) ;
	ESLVerify( 	AddParameterEntry
				( L"multi_pass_depth", S3DSceneComposer::typeSelector,
					S3DSceneComposer::attrConstant
					| S3DSceneComposer::attrStringEnumeration
					| S3DSceneComposer::attrUIOnlyEnumeration,
					L"マルチパス深度処理", nullptr ) == paramUserMultiPassDepthFunc ) ;
	ESLVerify( 	AddParameterEntry
				( L"transparent_depth", S3DSceneComposer::typeSelector,
					S3DSceneComposer::attrConstant
					| S3DSceneComposer::attrStringEnumeration
					| S3DSceneComposer::attrUIOnlyEnumeration,
					L"半透明インスタンス深度処理", 
					L"複数インスタンス描画の際に半透明インスタンスの深度処理。\n"
					L"インスタンスは半透明と分離ソートする必要があります。" ) == paramTransparentDepthFunc ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DShadowDepthFuncController::~S3DShadowDepthFuncController( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DShadowDepthFuncController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramMainDepthFunc:
	case	paramShadowDepthFunc:
	case	paramEnvironmentDepthFunc:
	case	paramUserMultiPassDepthFunc:
	case	paramTransparentDepthFunc:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiDepthFunc, m_depthFunc[i] ) ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DShadowDepthFuncController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramMainDepthFunc:
	case	paramShadowDepthFunc:
	case	paramEnvironmentDepthFunc:
	case	paramUserMultiPassDepthFunc:
	case	paramTransparentDepthFunc:
		m_depthFunc[i] =
			(S3DRenderContext::DepthMaskOperation)
				SXMLDocument::GetIntegerAsSymbolOf
					( m_aiDepthFunc, pwszCmd, m_depthFunc[i] ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DShadowDepthFuncController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramMainDepthFunc:
	case	paramShadowDepthFunc:
	case	paramEnvironmentDepthFunc:
	case	paramUserMultiPassDepthFunc:
	case	paramTransparentDepthFunc:
		for ( j = 0; m_aiDepthFunc[j].pszSymbol != nullptr; j ++ )
		{
			aStrSet.Add( new SString(m_aiDepthFunc[j].pszSymbol) ) ;
		}
		return	true ;
	}
	return	false ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void S3DShadowDepthFuncController::BeforeRenderModel
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DSceneComposer::ItemSerializer * pItem,
		S3DRenderContextInterface& render,
		uint64_t flagsExclusion )
{
	const S3DScene::RenderingStage	renderStage = scene.GetCurrentRenderingStage() ;
	size_t	iStage = 0 ;
	switch ( renderStage )
	{
	case	S3DScene::renderingMain:
	default:
		iStage = paramMainDepthFunc ;
		break ;
	case	S3DScene::renderingShadow:
		iStage = paramShadowDepthFunc ;
		break ;
	case	S3DScene::renderingEnvironment:
		iStage = paramEnvironmentDepthFunc ;
		break ;
	case	S3DScene::renderingMultiPass0:
		iStage = paramUserMultiPassDepthFunc ;
		break ;
	}
	if ( m_depthFunc[iStage] != S3DRenderContext::depthMaskDefault )
	{
		render.SetOptionalFeature
			( S3DRenderContext::featureDepthMask,
				(int32_t) m_depthFunc[iStage], nullptr, 0 ) ;
	}
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
S3DItemInstancingSerializer::RenderResult
	S3DShadowDepthFuncController::RenderMultiInstance
		( const S3DScene& scene,
			S3DRenderContextInterface& render,
			const S3DItemInstancingSerializer& instancing,
			S3DItemInstancingSerializer::RenderingType type,
			S3DItemInstancingSerializer::RenderInfo& info )
{
	if ( m_depthFunc[paramTransparentDepthFunc] != S3DRenderContext::depthMaskDefault )
	{
		render.PushTransformation() ;
		render.SetOptionalFeature
			( S3DRenderContext::featureDepthMask,
				(int32_t) m_depthFunc[paramTransparentDepthFunc], nullptr, 0 ) ;
		//
		m_aTempMatrix.RemoveAll() ;
		m_aTempColor.RemoveAll() ;
		m_aTempMatrix.AddArray( info.pInstanceMatrices, info.nInstanceCount ) ;
		m_aTempColor.AddArray( info.pInstanceColors, info.nInstanceCount ) ;
		//
		S4DMatrix *	pMatrices = m_aTempMatrix.GetArray() ;
		S3DColor *	pColors = m_aTempColor.GetArray() ;
		info.pInstanceMatrices = pMatrices ;
		info.pInstanceColors = pColors ;
		//
		for ( size_t iRange = 0; iRange < info.nRangeCount; iRange ++ )
		{
			S3DItemInstancingSerializer::InstanceRange&
									range = info.pInstanceRanges[iRange] ;
			size_t	nCount = range.nCount ;
			if ( nCount == 0 )
			{
				continue ;
			}
			size_t	iBase = range.nIndex ;
			size_t	iLast = nCount ;
			size_t	i = 0 ;
			while ( i + 1 < iLast )
			{
				if ( pColors[iBase + i].rgbMul.argb.Alpha >= 0xFE )
				{
					++ i ;
					continue ;
				}
				do
				{
					if ( pColors[iBase + (-- iLast)].rgbMul.argb.Alpha < 0xFE )
					{
						S4DMatrix	mat4Temp = pMatrices[iBase + iLast] ;
						S3DColor	clrTemp = pColors[iBase + iLast] ;
						pMatrices[iBase + iLast] = pMatrices[iBase + i] ;
						pColors[iBase + iLast] = pColors[iBase + i] ;
						pMatrices[iBase + i] = mat4Temp ;
						pColors[iBase + i] = clrTemp ;
						break ;
					}
				}
				while ( i + 1 < iLast ) ;
			}
			ESLAssert( i < nCount ) ;
			if ( pColors[iBase + i].rgbMul.argb.Alpha >= 0xFE )
			{
				++ i ;
			}
			size_t	nOpaque = i ;
			range.nCount = nOpaque ;
			//
			if ( nCount > nOpaque )
			{
				info.pModel->RenderBufferTo
					( &render, info.flagsExclusion,
						info.iMeshFirst, info.iMeshEnd,
						nCount - nOpaque,
						pMatrices + (iBase + nOpaque),
						pColors + (iBase + nOpaque) ) ;
			}
		}
		//
		render.PopTransformation() ;
	}
	return	S3DItemInstancingSerializer::renderContinue ;
}



//////////////////////////////////////////////////////////////////////////////
// 水面シェーダー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DSimpleWaterShaderController::m_pwszTimeMethod
	[S3DSimpleWaterShaderController::timeMethodCount] =
{
	L"by_timer", L"by_frame", L"by_parameter"
} ;

const wchar_t *	S3DSimpleWaterShaderController::m_pwszParamIDs
	[S3DSimpleWaterShaderController::paramWaveParamCount
		* S3DSimpleWaterShaderController::paramWaveCount] =
{
	L"amplitube0", L"frequency0", L"phase_speed0", L"direction0",
	L"amplitube1", L"frequency1", L"phase_speed1", L"direction1",
	L"amplitube2", L"frequency2", L"phase_speed2", L"direction2",
	L"amplitube3", L"frequency3", L"phase_speed3", L"direction3",
	L"amplitube4", L"frequency4", L"phase_speed4", L"direction4",
	L"amplitube5", L"frequency5", L"phase_speed5", L"direction5",
	L"amplitube6", L"frequency6", L"phase_speed6", L"direction6",
	L"amplitube7", L"frequency7", L"phase_speed7", L"direction7",
} ;

const wchar_t *	S3DSimpleWaterShaderController::m_pwszParamNames
	[S3DSimpleWaterShaderController::paramWaveParamCount
		* S3DSimpleWaterShaderController::paramWaveCount] =
{
	L"振幅[0]", L"周波数[0]", L"位相速度[0]", L"方向[0]",
	L"振幅[1]", L"周波数[1]", L"位相速度[1]", L"方向[1]",
	L"振幅[2]", L"周波数[2]", L"位相速度[2]", L"方向[2]",
	L"振幅[3]", L"周波数[3]", L"位相速度[3]", L"方向[3]",
	L"振幅[4]", L"周波数[4]", L"位相速度[4]", L"方向[4]",
	L"振幅[5]", L"周波数[5]", L"位相速度[5]", L"方向[5]",
	L"振幅[6]", L"周波数[6]", L"位相速度[6]", L"方向[6]",
	L"振幅[7]", L"周波数[7]", L"位相速度[7]", L"方向[7]",
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSimpleWaterShaderController, S3DCommonShaderController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DSimpleWaterShaderController, wave8_shader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSimpleWaterShaderController::S3DSimpleWaterShaderController( void )
	: S3DCommonShaderController( m_ItemClassDescriptor.pwszClassID ),
		m_pDevice( nullptr ),
		m_pShader( nullptr ),
		m_flagSimpleParam( true ),
		m_nRandomSeed( 1 ),
		m_timeMethod( timeByTimer ),
		m_secTime( 0.0 ),
		m_matRotation( 1, 1, 1 ),
		m_fpMasterAmplitube( 1.0 ),
		m_fpMasterScale( 1.0 ),
		m_fpMasterTimeScale( 1.0 ),
		m_fpCascadeFarZ( 300.0 ),
		m_fpCascadePhase1( 0.05 ),
		m_fpCascadeAmplitude1( 0.5 )
{
	m_flagsBehavior |= S3DSceneComposer::behaviorRenderContext
						| S3DSceneComposer::behaviorOnTimer ;
	m_flagExceptShadow = false ;
	//
	DATE_TIME	dtCurrent ;
	SSystem::CurrentLocalDate( dtCurrent ) ;
	m_nRandomSeed = (dtCurrent.nSecond + dtCurrent.nMilliSec) % 100 ;
	//
	PrepareParameterEntryCount
		( paramFirstWave + paramWaveParamCount * paramWaveCount ) ;
	AddTargetClassParameterEntry() ;
	AddParameterEntry
		( L"simple_param", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrDynamicValidation,
			L"シンプルパラメータ", nullptr ) ;
	AddParameterEntry
		( L"simple_param_seed", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"シンプルパラメータ種", nullptr ) ;
	AddParameterEntry
		( L"master_amplitube", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"振幅", L"全体の振幅" ) ;
	AddParameterEntry
		( L"master_scale", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"スケール", L"全体の波長スケール" ) ;
	AddParameterEntry
		( L"master_time_speed", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"波速", L"全体の波速スケール" ) ;
	AddParameterEntry
		( L"cascade_far_z", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"遠方位相距離" ) ;
	AddParameterEntry
		( L"cascade_phase1", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"追加位相スケール" ) ;
	AddParameterEntry
		( L"cascade_amp1", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"追加位相振幅" ) ;
	AddParameterEntry
		( L"rotation", S3DSceneComposer::typeRotation,
			S3DSceneComposer::attrConstant,
			L"回転", nullptr ) ;
	AddParameterEntry
		( L"time_method", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"時間駆動方法", nullptr ) ;
	AddParameterEntry
		( L"time", S3DSceneComposer::typeScalar, 0,
			L"時間", nullptr ) ;
	//
	SetSimpleParameter( m_nRandomSeed ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSimpleWaterShaderController::~S3DSimpleWaterShaderController( void )
{
}

// 簡易パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleWaterShaderController::SetSimpleParameter( uint32_t nGenParam )
{
	ChopParameterEntryLastAt( paramTime ) ;
	WaveSimpleParameter( m_wave, paramWaveCount, nGenParam ) ;
}

void S3DSimpleWaterShaderController::WaveSimpleParameter
	( WaveParameter* pwp, size_t nCount, uint32_t nGenParam )
{
	SakuraCL::SCLRandomizer	rand( nGenParam ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( i < 4 )
		{
			double	rad = (i * 90 + rand.QuickRandomize( 90 )) * PI / 180.0 ;
			pwp[i].fpAmplitude =
				(float32_t) ((rand.QuickRandomize( 70 ) + 30.0f) * 0.01) ;
			pwp[i].fpFrequency =
				(float32_t) (300.0 / (100 + rand.QuickRandomize(200))) ;
			pwp[i].radSpeed =
				(float32_t) (PI * (rand.QuickRandomize(100) * 0.01 + 1.0) * 0.25) ;
			pwp[i].vDirection.x = (float32_t) sin(rad) ;
			pwp[i].vDirection.y = (float32_t) cos(rad) ;
		}
		else
		{
			double	rad = (i * 90 + 45 + rand.QuickRandomize( 90 )) * PI / 180.0 ;
			pwp[i].fpAmplitude =
				(float32_t) (rand.QuickRandomize( 99 ) + 30.0f) / 200.0f ;
			pwp[i].fpFrequency =
				(float32_t) (2.0 * 300.0 / (50 + rand.QuickRandomize(100))) ;
			pwp[i].radSpeed =
				(float32_t) (PI * (rand.QuickRandomize(100) * 0.01 + 1.0) * 0.5) ;
			pwp[i].vDirection.x = (float32_t) sin(rad) ;
			pwp[i].vDirection.y = (float32_t) cos(rad) ;
		}
	}
}

// 詳細パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleWaterShaderController::AddWaveDetailParameters( void )
{
	ChopParameterEntryLastAt( paramTime ) ;
	//
	for ( int i = 0; i < paramWaveCount; i ++ )
	{
		int	j = i * paramWaveParamCount ;
		AddParameterEntry
			( m_pwszParamIDs[j],
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1,
				m_pwszParamNames[j], nullptr ) ;
		AddParameterEntry
			( m_pwszParamIDs[j + 1],
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1,
				m_pwszParamNames[j + 1], nullptr ) ;
		AddParameterEntry
			( m_pwszParamIDs[j + 2],
				S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant1,
				m_pwszParamNames[j + 2], nullptr ) ;
		AddParameterEntry
			( m_pwszParamIDs[j + 3],
				S3DSceneComposer::typeVector2,
				S3DSceneComposer::attrConstant1,
				m_pwszParamNames[j + 3], nullptr ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DSimpleWaterShaderController::GetMatrixParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
		return	m_matRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

double S3DSimpleWaterShaderController::GetScalarParameter( size_t i ) const
{
	size_t	iWave = 0 ;
	if ( i >= paramFirstWave )
	{
		iWave = (i - paramFirstWave) / paramWaveParamCount ;
		i = ((i - paramFirstWave) % paramWaveParamCount) + paramFirstWave ;
		if ( iWave >= paramWaveCount )
		{
			return	0.0 ;
		}
	}
	switch ( i )
	{
	case	paramMasterAmplitude:
		return	m_fpMasterAmplitube ;
	case	paramMasterScale:
		return	m_fpMasterScale ;
	case	paramMasterTimeScale:
		return	m_fpMasterTimeScale ;
	case	paramCascadeFarZ:
		return	m_fpCascadeFarZ ;
	case	paramCascadePhase1:
		return	m_fpCascadePhase1 ;
	case	paramCascadeAmp1:
		return	m_fpCascadeAmplitude1 ;
	case	paramTime:
		return	m_secTime ;
	case	paramAmplitude0:
		return	m_wave[iWave].fpAmplitude ;
	case	paramFrequency0:
		return	m_wave[iWave].fpFrequency ;
	case	paramPhaseSpeed0:
		return	m_wave[iWave].radSpeed ;
	}
	return	0.0 ;
}

int32_t S3DSimpleWaterShaderController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRandomSeed:
		return	m_nRandomSeed ;
	}
	return	0 ;
}

bool S3DSimpleWaterShaderController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramSimpleParam:
		return	m_flagSimpleParam ;
	}
	return	false ;
}

const wchar_t * S3DSimpleWaterShaderController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramTimeMethod:
		return	m_pwszTimeMethod[m_timeMethod] ;
	}
	return	S3DCommonShaderController::GetCommandParameter( i ) ;
}

size_t S3DSimpleWaterShaderController::GetBinaryParameter
	( void * pDst, size_t nBufBytes, size_t i ) const
{
	size_t	iWave = 0 ;
	if ( i >= paramFirstWave )
	{
		iWave = (i - paramFirstWave) / paramWaveParamCount ;
		i = ((i - paramFirstWave) % paramWaveParamCount) + paramFirstWave ;
		if ( iWave >= paramWaveCount )
		{
			return	0 ;
		}
	}
	switch ( i )
	{
	case	paramDirection0:
		if ( pDst == nullptr )
		{
			return	sizeof(S2DDVector) ;
		}
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			*((S2DDVector*)pDst) = m_wave[iWave].vDirection ;
			return	sizeof(S2DDVector) ;
		}
		return	0 ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleWaterShaderController::SetMatrixParameter( size_t i, const S3DDMatrix& mat )
{
	switch ( i )
	{
	case	paramRotation:
		m_matRotation = mat ;
		return ;
	}
}

void S3DSimpleWaterShaderController::SetScalarParameter( size_t i, double s )
{
	size_t	iWave = 0 ;
	if ( i >= paramFirstWave )
	{
		iWave = (i - paramFirstWave) / paramWaveParamCount ;
		i = ((i - paramFirstWave) % paramWaveParamCount) + paramFirstWave ;
		if ( iWave >= paramWaveCount )
		{
			return ;
		}
	}
	switch ( i )
	{
	case	paramMasterAmplitude:
		m_fpMasterAmplitube = s ;
		return ;
	case	paramMasterScale:
		m_fpMasterScale = s ;
		return ;
	case	paramMasterTimeScale:
		m_fpMasterTimeScale = s ;
		return ;
	case	paramCascadeFarZ:
		m_fpCascadeFarZ = s ;
		return ;
	case	paramCascadePhase1:
		m_fpCascadePhase1 = s ;
		return ;
	case	paramCascadeAmp1:
		m_fpCascadeAmplitude1 = s ;
		return ;
	case	paramTime:
		m_secTime = s ;
		return ;
	case	paramAmplitude0:
		m_wave[iWave].fpAmplitude = (float32_t) s ;
		return ;
	case	paramFrequency0:
		m_wave[iWave].fpFrequency = (float32_t) s ;
		return ;
	case	paramPhaseSpeed0:
		m_wave[iWave].radSpeed = (float32_t) s ;
		return ;
	}
}

void S3DSimpleWaterShaderController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramRandomSeed:
		m_nRandomSeed = n ;
		if ( m_flagSimpleParam )
		{
			SetSimpleParameter( m_nRandomSeed ) ;
		}
		return ;
	}
}

void S3DSimpleWaterShaderController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramSimpleParam:
		m_flagSimpleParam = b ;
		if ( m_flagSimpleParam )
		{
			SetSimpleParameter( m_nRandomSeed ) ;
		}
		else
		{
			AddWaveDetailParameters() ;
		}
		return ;
	}
}

void S3DSimpleWaterShaderController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	int	j ;
	switch ( i )
	{
	case	paramTimeMethod:
		for ( j = 0; j < timeMethodCount; j ++ )
		{
			if ( SString::Compare( pwszCmd, m_pwszTimeMethod[j] ) == 0 )
			{
				m_timeMethod = (TimeMethod) j ;
				break ;
			}
		}
		return ;
	}
	S3DCommonShaderController::SetCommandParameter( i, pwszCmd ) ;
}

size_t S3DSimpleWaterShaderController::SetBinaryParameter
	( size_t i, const void * pSrc, size_t nBufBytes )
{
	size_t	iWave = 0 ;
	if ( i >= paramFirstWave )
	{
		iWave = (i - paramFirstWave) / paramWaveParamCount ;
		i = ((i - paramFirstWave) % paramWaveParamCount) + paramFirstWave ;
		if ( iWave >= paramWaveCount )
		{
			return	0 ;
		}
	}
	switch ( i )
	{
	case	paramDirection0:
		if ( nBufBytes == sizeof(S2DDVector) )
		{
			m_wave[iWave].vDirection = *((S2DDVector*)pSrc) ;
			return	sizeof(S2DDVector) ;
		}
		return	0 ;
	}
	return	0 ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSimpleWaterShaderController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	int	j ;
	switch ( i )
	{
	case	paramTimeMethod:
		for ( j = 0; j < timeMethodCount; j ++ )
		{
			aStrSet.Add( new SString( m_pwszTimeMethod[j] ) ) ;
		}
		return	true ;
	}
	return	S3DCommonShaderController::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DSimpleWaterShaderController::IsParameterValidation( size_t i ) const
{
	if ( m_flagSimpleParam && (i >= paramFirstWave) )
	{
		return	false ;
	}
	if ( (m_timeMethod != timeByParameter) && (i == paramTime) )
	{
		return	false ;
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSimpleWaterShaderController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"詳細設定" ;
	}
	return	nullptr ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleWaterShaderController::OnTimer
	( S3DScene& scene,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast )
{
	Controller::OnTimer( scene, pItem, msecPast ) ;
	//
	if ( m_timeMethod == timeByTimer )
	{
		m_secTime += (double) msecPast / 1000.0 ;
	}
}

// フレーム（パラメータ）更新後処理
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleWaterShaderController::OnUpdateFrame
	( S3DSceneComposer::ItemSerializer * pItem,
			double fpFrame, S3DSceneComposer::SeekMethod seek )
{
	Controller::OnUpdateFrame( pItem, fpFrame, seek ) ;
	//
	if ( m_timeMethod == timeByFrame )
	{
		S3DSceneComposer::Composition *	pComp = pItem->GetComposition() ;
		if ( pComp != nullptr )
		{
			m_secTime =
				pComp->GetCompositionInfo()->FrameIndexToSecond( fpFrame ) ;
		}
	}
	else if ( m_timeMethod == timeByTimer )
	{
		if ( seek == S3DSceneComposer::seekJumpReset )
		{
			m_secTime = 0.0 ;
		}
	}
}

// シェーダー設定
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleWaterShaderController::UpdateShader
	( const S3DScene& scene,
		S3DScene::ItemClass clsItem,
		S3DRenderContextInterface& render )
{
	S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
	if ( (m_pShader == nullptr) || (m_pDevice != pDevice) )
	{
		if ( pDevice == nullptr )
		{
			return ;
		}
		m_pDevice = pDevice ;
		m_pShader = pDevice->GetDefaultShaderProgramAs
						( S3DRenderDevice::DefaultShaderId::SimpleWater ) ;
	}
	S3DSimpleWaterShaderInterface *
		pWaveShader = ESLTypeCast<S3DSimpleWaterShaderInterface>( m_pShader ) ;
	if ( pWaveShader == nullptr )
	{
		return ;
	}
	S3DSimpleWaterShaderInterface::Parameter	prm ;
	float32_t	sec = (float32_t) (m_secTime * m_fpMasterTimeScale) ;
	float32_t	amp = (float32_t) m_fpMasterAmplitube ;
	float32_t	s = (float32_t) m_fpMasterScale ;
	for ( int i = 0; i < 4; i ++ )
	{
		prm.waterVertex[i].fpAmplitude = m_wave[i].fpAmplitude ;
		prm.waterVertex[i].fpFrequency = m_wave[i].fpFrequency * s ;
		prm.waterVertex[i].radTime = m_wave[i].radSpeed * sec ;
		prm.waterVertex[i].vDirection = m_wave[i].vDirection ;
	}
	for ( int i = 0; i < 4; i ++ )
	{
		prm.waterBump[i].fpAmplitude = m_wave[i+4].fpAmplitude ;
		prm.waterBump[i].fpFrequency = m_wave[i+4].fpFrequency * s ;
		prm.waterBump[i].radTime = m_wave[i+4].radSpeed * sec ;
		prm.waterBump[i].vDirection = m_wave[i+4].vDirection ;
	}
	prm.vLevelAxisX = m_matRotation * S3DDVector( 1, 0, 0 ) ;
	prm.vLevelAxisY = m_matRotation * S3DDVector( 0, 0, 1 ) ;
	prm.fpAmplitude = amp ;
	prm.fpNormalAmp = 1.0f ;
	prm.zCascadeFar = (float32_t) m_fpCascadeFarZ ;
	prm.fpCascadePhase[0] = (float32_t) m_fpCascadePhase1 ;
	prm.fpCascadeAmp[0] = (float32_t) m_fpCascadeAmplitude1 ;
	//
	pWaveShader->LockParameter() ;
	pWaveShader->SetParameter( prm ) ;
	pWaveShader->SetShaderUniformsTo( render ) ;
	pWaveShader->UnlockParameter() ;
	//
	render.AttachCustomShader( m_pShader ) ;
}

