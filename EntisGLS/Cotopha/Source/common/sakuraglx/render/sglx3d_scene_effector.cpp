
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_composer.h>
#include <sakuraglx/render/sglx3d_scene_effector.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;



//////////////////////////////////////////////////////////////////////////////
// 環境マッピングターゲット
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DCubeEnvironmentMapSerializer::m_paramEntries
		[S3DCubeEnvironmentMapSerializer::paramEnvMapCount] =
{
	{ L"cubemap_width",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"解像度", L"環境マッピング用にレンダリングする画像サイズ" },
	{ L"update_envmap",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1,
		L"更新", L"環境マッピング用に再レンダリングします" },
	{ L"envmap_field",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"field 描画", L"field クラスをレンダリング対象に追加します" },
	{ L"envmap_static_item1",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"static_item1 描画", L"static_item1 クラスをレンダリング対象に追加します" },
	{ L"envmap_static_item2",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"static_item2 描画", L"static_item2 クラスをレンダリング対象に追加します" },
	{ L"envmap_dynamic_item1",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"dynamic_item1 描画", L"dynamic_item1 クラスをレンダリング対象に追加します" },
	{ L"envmap_dynamic_item2",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"dynamic_item2 描画", L"dynamic_item2 クラスをレンダリング対象に追加します" },
	{ L"envmap_dynamic_item3",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"dynamic_item3 描画", L"dynamic_item3 クラスをレンダリング対象に追加します" },
	{ L"envmap_effect_item",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"effect_item 描画", L"effect_item クラスをレンダリング対象に追加します" },
	{ L"envmap_layered_item",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"layered_item 描画", L"layered_item クラスをレンダリング対象に追加します" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DCubeEnvironmentMapSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	paramEnvMapCount,
	&S3DCubeEnvironmentMapSerializer::m_paramEntries[0],
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DCubeEnvironmentMapSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM
	( SakuraGL::S3DCubeEnvironmentMapSerializer, envmap_target )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCubeEnvironmentMapSerializer::S3DCubeEnvironmentMapSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DCubeEnvironmentMapSerializer::m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	//
	m_maskTargetClasses = S3DScene::classBitsAllScape
						| S3DScene::classBitField
						| S3DScene::classBitsAllStaticItem
						| S3DScene::classBitsAllDynamicItem ;
	m_sizeImage.w = 256 ;
	m_sizeImage.h = 256 ;
	m_flagEnabled = true ;
	m_flagUpdate = true ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCubeEnvironmentMapSerializer::~S3DCubeEnvironmentMapSerializer( void )
{
}

// 画像サイズ
//////////////////////////////////////////////////////////////////////////////
void S3DCubeEnvironmentMapSerializer::SetImageSize( const SGLSize& size )
{
	m_sizeImage = size ;
}

// 描画ターゲット
//////////////////////////////////////////////////////////////////////////////
void S3DCubeEnvironmentMapSerializer::SetTargetClasses( uint32_t maskClasses )
{
	m_maskTargetClasses = maskClasses ;
}

// 有効・無効化
//////////////////////////////////////////////////////////////////////////////
void S3DCubeEnvironmentMapSerializer::EnableEnvMap( bool flagEnable )
{
	m_flagEnabled = flagEnable ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DCubeEnvironmentMapSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramImageWidth:
		return	m_sizeImage.w ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

bool S3DCubeEnvironmentMapSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramUpdateFlag:
		return	m_flagUpdate ;
	case	paramTargetField:
		return	(m_maskTargetClasses & S3DScene::classBitField) != 0 ;
	case	paramTargetStaticItem1:
		return	(m_maskTargetClasses & S3DScene::classBitStaticItem1) != 0 ;
	case	paramTargetStaticItem2:
		return	(m_maskTargetClasses & S3DScene::classBitStaticItem2) != 0 ;
	case	paramTargetDynamicItem1:
		return	(m_maskTargetClasses & S3DScene::classBitDynamicItem1) != 0 ;
	case	paramTargetDynamicItem2:
		return	(m_maskTargetClasses & S3DScene::classBitDynamicItem2) != 0 ;
	case	paramTargetDynamicItem3:
		return	(m_maskTargetClasses & S3DScene::classBitDynamicItem3) != 0 ;
	case	paramTargetEffectItem:
		return	(m_maskTargetClasses & S3DScene::classBitEffectItem) != 0 ;
	case	paramTargetLayeredItem:
		return	(m_maskTargetClasses & S3DScene::classBitLayeredItem1) != 0 ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCubeEnvironmentMapSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramImageWidth:
		m_sizeImage.w = n ;
		m_sizeImage.h = n ;
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DCubeEnvironmentMapSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramUpdateFlag:
		m_flagUpdate = b ;
		return ;
	case	paramTargetField:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitField)
				| (b ? S3DScene::classBitField : 0) ;
		return ;
	case	paramTargetStaticItem1:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitStaticItem1)
				| (b ? S3DScene::classBitStaticItem1 : 0) ;
		return ;
	case	paramTargetStaticItem2:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitStaticItem2)
				| (b ? S3DScene::classBitStaticItem2 : 0) ;
		return ;
	case	paramTargetDynamicItem1:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitDynamicItem1)
				| (b ? S3DScene::classBitDynamicItem1 : 0) ;
		return ;
	case	paramTargetDynamicItem2:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitDynamicItem2)
				| (b ? S3DScene::classBitDynamicItem2 : 0) ;
		return ;
	case	paramTargetDynamicItem3:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitDynamicItem3)
				| (b ? S3DScene::classBitDynamicItem3 : 0) ;
		return ;
	case	paramTargetEffectItem:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitEffectItem)
				| (b ? S3DScene::classBitEffectItem : 0) ;
		return ;
	case	paramTargetLayeredItem:
		m_maskTargetClasses =
			(m_maskTargetClasses & ~S3DScene::classBitLayeredItem1)
				| (b ? S3DScene::classBitLayeredItem1 : 0) ;
		return ;
	}
	return	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DCubeEnvironmentMapSerializer::IsParameterValidation( size_t i ) const
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
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCubeEnvironmentMapSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"環境マップ" ;
	case	2:
		return	L"環境マップ反映対象" ;
	}
	return	NULL ;
}

// レンダリング後始末
//////////////////////////////////////////////////////////////////////////////
void S3DCubeEnvironmentMapSerializer::OnShoutdownSceneSettings
	( S3DScene& scene, SGLSecondaryViewProducer * psvp )
{
	scene.AttachDynamicEnvironmentTarget( NULL, NULL ) ;
	scene.EnableDynamicEnvironment( false ) ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DCubeEnvironmentMapSerializer::OnUpdateBehavior( S3DScene& scene )
{
	if ( m_flagEnabled )
	{
		if ( m_imgCubemap.GetImageSize() != m_sizeImage )
		{
			m_imgCubemap.CreateImage
				( (uint32_t) m_sizeImage.w,
					(uint32_t) m_sizeImage.h,
					formatImageDefaultRGBA, 32,
					SGLImageObject::bufferCubeMapTexture
					| SGLImageObject::bufferOnDeviceOnly ) ;
		}
		if ( m_imgZBuffer.GetImageSize() != m_sizeImage )
		{
			m_imgZBuffer.CreateImage
				( (uint32_t) m_sizeImage.w,
					(uint32_t) m_sizeImage.h,
					formatImageDepth, 32,
					SGLImageObject::bufferOnDeviceOnly ) ;
		}
		S3DDMatrix	matItem ;
		S3DDVector	vPos ;
		GetGlobalTransformation( matItem, vPos ) ;
		//
		S3DScene::DynamicEnvironment	denv ;
		denv.vCenterPos = vPos ;
		denv.maskTargetClasses =
				m_maskTargetClasses
				| S3DScene::classBitBackscape | S3DScene::classBitScape ;
		//
		scene.AttachDynamicEnvironmentTarget( &m_imgCubemap, &m_imgZBuffer ) ;
		scene.SetDynamicEnvironment( denv, true ) ;
		if ( m_flagUpdate )
		{
			scene.SetUpdateDynamicEnvironment() ;
		}
		scene.EnableDynamicEnvironment( true ) ;
	}
	else
	{
		scene.AttachDynamicEnvironmentTarget( NULL, NULL ) ;
		scene.EnableDynamicEnvironment( false ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// エフェクター共通
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DCommonEffectorItemSerializer::m_paramEntries
			[S3DCommonEffectorItemSerializer::paramEffectorCount] =
{
	{ L"effector_class",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrDynamicValidation,
		L"効果パスクラス", L"効果を実行するパスを指定します" },
	{ L"effector_priority",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"効果順序", L"効果を実行する順序を指定します（数値の小さいものから順に実行）" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DCommonEffectorItemSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemBasicSerializer::m_pscClass,
	S3DCommonEffectorItemSerializer::paramEffectorCount,
	&S3DCommonEffectorItemSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DCommonEffectorItemSerializer::m_aiEffectorClass
			[S3DCommonEffectorItemSerializer::effectorClassCount+1] =
{
	{ L"above", S3DCommonEffectorItemSerializer::effectorGlobalAbove },
	{ L"below", S3DCommonEffectorItemSerializer::effectorGlobalBelow },
	{ L"local_space", S3DCommonEffectorItemSerializer::effectorLocalSpace },
	{ NULL, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DCommonEffectorItemSerializer, ItemBasicSerializer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCommonEffectorItemSerializer::S3DCommonEffectorItemSerializer
	( const wchar_t * pwszClassID,
		const S3DSceneComposer::ParamSetClass * pClass )
	: ItemBasicSerializer( pwszClassID, pClass )
{
	m_classItem = S3DScene::classPreRender ;
	m_pEffector = NULL ;
	m_clsEffector = effectorGlobalAbove ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCommonEffectorItemSerializer::~S3DCommonEffectorItemSerializer( void )
{
}

// エフェクタ関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DCommonEffectorItemSerializer::AttachSceneEffector( S3DScene::Effector * pEffector )
{
	m_pEffector = pEffector ;
}

// 効果クラス
//////////////////////////////////////////////////////////////////////////////
S3DCommonEffectorItemSerializer::EffectorClass
	S3DCommonEffectorItemSerializer::GetEffectorClass( void ) const
{
	return	m_clsEffector ;
}

void S3DCommonEffectorItemSerializer::SetEffectorClass
		( S3DCommonEffectorItemSerializer::EffectorClass clsEffector )
{
	m_clsEffector = clsEffector ;
}

// 効果優先度
//////////////////////////////////////////////////////////////////////////////
int S3DCommonEffectorItemSerializer::GetEffectorPriority( void ) const
{
	return	m_pEffector ? m_pEffector->GetDrawingPriority() : 0 ;
}

void S3DCommonEffectorItemSerializer::SetEffectorPriority( int nPriority )
{
	if ( m_pEffector != NULL )
	{
		m_pEffector->SetDrawingPriority( nPriority ) ;
	}
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DCommonEffectorItemSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
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
	case	paramGlobalSpace:
	case	paramCameraShift:
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DCommonEffectorItemSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramEffectorPriority:
		return	(int32_t) GetEffectorPriority() ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

const wchar_t * S3DCommonEffectorItemSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramEffectorClass:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiEffectorClass, m_clsEffector ) ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCommonEffectorItemSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramEffectorPriority:
		SetEffectorPriority( (int) n ) ;
		return ;
	}
	return	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DCommonEffectorItemSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramEffectorClass:
		m_clsEffector =
			(EffectorClass) SXMLDocument::GetIntegerAsSymbolOf
							( m_aiEffectorClass, pwszCmd, m_clsEffector ) ;
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DCommonEffectorItemSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramEffectorClass:
		for ( j = 0; m_aiEffectorClass[j].pszSymbol; j ++ )
		{
			aStrSet.Add( new SString( m_aiEffectorClass[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCommonEffectorItemSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"効果設定" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DCommonEffectorItemSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( (clsItem == S3DScene::classPreRender)
		&& (m_pEffector != NULL) && GetVisibleParameter() )
	{
		S3DScene::Space *	pSpace ;
		switch ( m_clsEffector )
		{
		case	effectorGlobalAbove:
			scene.AddTemporaryEffect( m_pEffector ) ;
			break ;
		case	effectorGlobalBelow:
			scene.AddTemporaryPostEffect( m_pEffector ) ;
			break ;
		case	effectorLocalSpace:
			pSpace = GetParentSpace() ;
			if ( pSpace != NULL )
			{
				pSpace->AddTemporaryEffect( m_pEffector ) ;
			}
			break ;
		default:
			break ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// 遅延シェーダー光源
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DDelayLightSerializer::m_paramEntries
			[S3DDelayLightSerializer::paramLightCount] =
{
	{ L"light_type",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrDynamicValidation,
		L"光源タイプ", NULL },
	{ L"write_emission",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"発光出力", L"発光フレームバッファにも書き出す" },
	{ L"cutoff_distance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"影響消失距離",
		L"カメラからの距離がこれ以上の時に遅延シェーダーは省略されます" },
	{ L"fadeout_distance",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"影響消失フェード",
		L"カメラからの距離が影響消失距離以内でも、"
		L"影響消失距離からフェード距離の区間では遅延シェーダーの影響は補完フェードします" },
	{ L"light_diffusion",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"拡散反射", NULL, 0.0, 1.0 },
	{ L"light_specular",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"鏡面反射", NULL, 0.0, 1.0 },
	{ L"air_scattering",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"大気散乱効果", NULL, 0.0, 1.0 },
	{ L"air_unit_length",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"散乱単位距離", NULL },
	{ L"light_color",
		S3DSceneComposer::typeColor,
		S3DSceneComposer::attrCategory1,
		L"光源色", NULL },
	{ L"light_brightness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"光源輝度", NULL, 0.0, 1.0 },
	{ L"light_direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,
		L"光源向き", NULL },
	{ L"light_attenuation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"距離減衰次元", NULL, 0.0, 2.0 },
	{ L"light_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"スポット範囲角", NULL, 0.0, 180.0 },
	{ L"light_gradation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"スポットぼかし角", NULL, 0.0, 180.0 },
} ;

const S3DSceneComposer::ParamSetClass
	S3DDelayLightSerializer::m_pscClass =
{
	&S3DCommonEffectorItemSerializer::m_pscClass,
	S3DDelayLightSerializer::paramLightCount,
	&S3DDelayLightSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DDelayLightSerializer::m_aiLightTypes[3] =
{
	{ L"point", lightTypePoint },
	{ L"spot", lightTypeSpot },
	{ NULL, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DDelayLightSerializer, S3DCommonEffectorItemSerializer, Effector )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DDelayLightSerializer, delay_light )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDelayLightSerializer::S3DDelayLightSerializer( void )
	: S3DCommonEffectorItemSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DDelayLightSerializer::m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	//
	SetDrawingPriority( 10 ) ;
	AttachSceneEffector( this ) ;
	//
	m_fpDiffusion = 1.0 ;
	m_fpSpecular = 1.0 ;
	m_fpAirScattering = 0.1 ;
	u_fpAirUnitLength = 10.0 ;
	//
	m_light.typeLight = lightTypePoint ;
	m_degLightAngle = 30.0 ;
	m_degLightGradation = 30.0 ;
	//
	m_flagWriteEmission = false ;
	m_fpCutOffDistance = 10000.0 ;
	m_fpFadeOutDistance = 100.0 ;
	m_fpDistanceFade = 1.0 ;
	//
	m_pDevice = NULL ;
	m_pRender = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DDelayLightSerializer::~S3DDelayLightSerializer( void )
{
	delete	m_pRender ;
	m_pRender = NULL ;
}

// 拡散反射適用度
//////////////////////////////////////////////////////////////////////////////
double S3DDelayLightSerializer::GetDiffusion( void ) const
{
	return	m_fpDiffusion ;
}

void S3DDelayLightSerializer::SetDiffusion( double fpDiffusion )
{
	m_fpDiffusion = fpDiffusion ;
}

// 鏡面反射適用度
//////////////////////////////////////////////////////////////////////////////
double S3DDelayLightSerializer::GetSpecular( void ) const
{
	return	m_fpSpecular ;
}

void S3DDelayLightSerializer::SetSpecular( double fpDiffusion )
{
	m_fpSpecular = fpDiffusion ;
}

// 大気散乱効果度
//////////////////////////////////////////////////////////////////////////////
double S3DDelayLightSerializer::GetAirScattering( void ) const
{
	return	m_fpAirScattering ;
}

double S3DDelayLightSerializer::GetAirScatteringUnit( void ) const
{
	return	u_fpAirUnitLength ;
}

void S3DDelayLightSerializer::SetAirScattering
		( double fpAirScattering, double fpAirUnitLength )
{
	m_fpAirScattering = fpAirScattering ;
	u_fpAirUnitLength = fpAirUnitLength ;
}

// 光源タイプ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DDelayLightSerializer::GetLightType( void ) const
{
	return	m_light.typeLight ;
}

void S3DDelayLightSerializer::SetLightType( uint32_t type )
{
	m_light.typeLight = type ;
}

// 光源色
//////////////////////////////////////////////////////////////////////////////
SGLPalette S3DDelayLightSerializer::GetLightColor( void ) const
{
	return	m_light.rgbColor ;
}

void S3DDelayLightSerializer::SetLightColor( const SGLPalette& rgbColor )
{
	m_light.rgbColor = rgbColor ;
}

// 輝度
//////////////////////////////////////////////////////////////////////////////
double S3DDelayLightSerializer::GetLightBrightness( void ) const
{
	return	m_light.fpBrightness ;
}

void S3DDelayLightSerializer::SetLightBrightness( double fpBrightness )
{
	m_light.fpBrightness = (float32_t) fpBrightness ;
}

// 点光源減衰力 x : (1/r^x)
//////////////////////////////////////////////////////////////////////////////
double S3DDelayLightSerializer::GetAttenuationPower( void ) const
{
	return	m_light.fpAttenuationPower ;
}

void S3DDelayLightSerializer::SetAttenuationPower( double fpAttenuation )
{
	m_light.fpAttenuationPower = (float32_t) fpAttenuation ;
}

// 光源位置
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DDelayLightSerializer::GetLightPosition( void ) const
{
	return	m_space.m_vCenter ;
}

void S3DDelayLightSerializer::SetLightPosition( const S3DDVector& vPos )
{
	m_space.m_vCenter = vPos ;
}

// 光源向き
//////////////////////////////////////////////////////////////////////////////
const S3DVector& S3DDelayLightSerializer::GetLightDirection( void ) const
{
	return	m_light.vecDirection ;
}

void S3DDelayLightSerializer::SetLightDirection( const S3DVector& vDir )
{
	m_light.vecDirection = vDir ;
}

// スポットライト範囲角 [deg]
//////////////////////////////////////////////////////////////////////////////
double S3DDelayLightSerializer::GetLightAngle( void ) const
{
	return	m_degLightAngle ;
}

void S3DDelayLightSerializer::SetLightAngle( double degAngle )
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
double S3DDelayLightSerializer::GetLightGradation( void ) const
{
	return	m_degLightGradation ;
}

void S3DDelayLightSerializer::SetLightGradation( double degGradation )
{
	double	g ;
	m_degLightGradation = degGradation ;
	g = esl_fmax( m_degLightAngle - m_degLightGradation, 0.0 ) ;
	m_light.fpGradation =
		(float32_t) (cos( g * PI / 180.0 )
						- cos(m_degLightAngle * PI / 180)) ;
}

// 発光出力
//////////////////////////////////////////////////////////////////////////////
bool S3DDelayLightSerializer::IsEnabledWriteEmission( void ) const
{
	return	m_flagWriteEmission ;
}

void S3DDelayLightSerializer::EnableWriteEmission( bool fEmission )
{
	m_flagWriteEmission = fEmission ;
}

// 影響消失距離
//////////////////////////////////////////////////////////////////////////////
double S3DDelayLightSerializer::GetCutOffDistance( void ) const
{
	return	m_fpCutOffDistance ;
}

double S3DDelayLightSerializer::GetFadeOutDistance( void ) const
{
	return	m_fpFadeOutDistance ;
}

void S3DDelayLightSerializer::SetCutOffDistance( double fpCutOff )
{
	m_fpCutOffDistance = fpCutOff ;
}

void S3DDelayLightSerializer::SetFadeOutDistance( double fpFadeOut )
{
	m_fpFadeOutDistance = fpFadeOut ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DDelayLightSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
		return	true ;
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
		return	(m_light.typeLight == lightTypeVector)
				| (m_light.typeLight == lightTypePoint)
				| (m_light.typeLight == lightTypeSpot) ;
	case	paramLightDirection:
		return	(m_light.typeLight == lightTypeVector)
				| (m_light.typeLight == lightTypeSpot) ;
	case	paramLightAttenuationPower:
		return	(m_light.typeLight == lightTypePoint)
				| (m_light.typeLight == lightTypeSpot) ;
	case	paramLightAngle:
	case	paramLightGradation:
		return	(m_light.typeLight == lightTypeSpot) ;
	}
	return	S3DCommonEffectorItemSerializer::IsParameterValidation( i ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DDelayLightSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightColor:
		return	VectorFromColor( m_light.rgbColor ) ;
	case	paramLightDirection:
		return	S3DDVector( m_light.vecDirection ) ;
	}
	return	S3DCommonEffectorItemSerializer::GetVectorParameter( i ) ;
}

double S3DDelayLightSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCutOffDistance:
		return	m_fpCutOffDistance ;
	case	paramFadeOutDistance:
		return	m_fpFadeOutDistance ;
	case	paramLightDiffusion:
		return	m_fpDiffusion ;
	case	paramLightSpecular:
		return	m_fpSpecular ;
	case	paramLightAirScattering:
		return	m_fpAirScattering ;
	case	paramLightAirUnitLength:
		return	u_fpAirUnitLength ;
	case	paramLightBrightness:
		return	m_light.fpBrightness ;
	case	paramLightAttenuationPower:
		return	m_light.fpAttenuationPower ;
	case	paramLightAngle:
		return	m_degLightAngle ;
	case	paramLightGradation:
		return	m_degLightGradation ;
	}
	return	S3DCommonEffectorItemSerializer::GetScalarParameter( i ) ;
}

int32_t S3DDelayLightSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightType:
		return	GetLightType() ;
	}
	return	S3DCommonEffectorItemSerializer::GetIntegerParameter( i ) ;
}

bool S3DDelayLightSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramWriteEmission:
		return	m_flagWriteEmission ;
	}
	return	S3DCommonEffectorItemSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DDelayLightSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLightType:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiLightTypes, GetLightType() ) ;
	}
	return	S3DCommonEffectorItemSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DDelayLightSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramLightColor:
		m_light.rgbColor = ColorFromVector( vec ) ;
		return ;
	case	paramLightDirection:
		m_light.vecDirection = vec ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetVectorParameter( i, vec ) ;
}

void S3DDelayLightSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramCutOffDistance:
		m_fpCutOffDistance = s ;
		return ;
	case	paramFadeOutDistance:
		m_fpFadeOutDistance = s ;
		return ;
	case	paramLightDiffusion:
		m_fpDiffusion = s ;
		return ;
	case	paramLightSpecular:
		m_fpSpecular = s ;
		return ;
	case	paramLightAirScattering:
		m_fpAirScattering = s ;
		return ;
	case	paramLightAirUnitLength:
		u_fpAirUnitLength = s ;
		return ;
	case	paramLightBrightness:
		m_light.fpBrightness = (float32_t) s ;
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
	}
	S3DCommonEffectorItemSerializer::SetScalarParameter( i, s ) ;
}

void S3DDelayLightSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramLightType:
		SetLightType( n ) ;
		return ;
	}
	return	S3DCommonEffectorItemSerializer::SetIntegerParameter( i, n ) ;
}

void S3DDelayLightSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramWriteEmission:
		EnableWriteEmission( b ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetBooleanParameter( i, b ) ;
}

void S3DDelayLightSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramLightType:
		m_light.typeLight =
			(uint32_t) SXMLDocument::GetIntegerAsSymbolOf
							( m_aiLightTypes, pwszCmd, m_light.typeLight ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DDelayLightSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	size_t	j ;
	switch ( i )
	{
	case	paramLightType:
		for ( j = 0; m_aiLightTypes[j].pszSymbol; j ++ )
		{
			aStrSet.Add( new SString( m_aiLightTypes[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	S3DCommonEffectorItemSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DDelayLightSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"光源設定" ;
	}
	return	S3DCommonEffectorItemSerializer::GetParameterCategoryName( iCategory ) ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DDelayLightSerializer::OnUpdateBehavior( S3DScene& scene )
{
	if ( m_flagsBehavior & S3DScene::itemVisible )
	{
		S3DDMatrix	matLight ;
		S3DDVector	vLight ;
		CalcGlobalTransformation( matLight, vLight ) ;
		//
		scene.AddLightDazzlement( matLight, vLight, m_light ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DDelayLightSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( clsItem == S3DScene::classPreRender )
	{
		S3DScene::Camera *	pCamera = scene.GetCurrentCamera() ;
		if ( pCamera != NULL )
		{
			S3DDMatrix	matSpace ;
			S3DDVector	vSpace ;
			GetGlobalTransformation( matSpace, vSpace ) ;
			//
			S3DDMatrix	matCameraSpace ;
			S3DDVector	vCamera ;
			pCamera->CalcGlobalTransformation( matCameraSpace, vCamera ) ;
			//
			double	fpDistance = (vCamera - vSpace).Absolute() ;
			double	fpFadeOut = m_fpCutOffDistance - m_fpFadeOutDistance ;
			if ( fpDistance >= m_fpCutOffDistance )
			{
				m_fpDistanceFade = 0.0 ;
				return ;
			}
			else if ( fpDistance > fpFadeOut )
			{
				m_fpDistanceFade =
					1.0 - (fpDistance - fpFadeOut) / m_fpFadeOutDistance ;
			}
			else
			{
				m_fpDistanceFade = 1.0 ;
			}
		}
	}
	return	S3DCommonEffectorItemSerializer::OnItemRenderEvent( scene, clsItem ) ;
}

// 要求カラーバッファ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DDelayLightSerializer::GetRequiredColorBufferMask( void ) const
{
	return	(1 << S3DScene::renderTargetNormal)
			| (1 << S3DScene::renderTargetDiffusion)
			| (1 << S3DScene::renderTargetSpecular) ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void S3DDelayLightSerializer::DrawEffect
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
#if	!defined(__COTOPHA__)
	if ( (m_fpDistanceFade == 0.0) || (m_light.fpBrightness <= 0.0) )
	{
		return ;
	}
	if ( nMultiImages <= S3DScene::renderTargetDiffusion )
	{
		return ;
	}
	S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
	if ( pDevice == NULL )
	{
		return ;
	}
	S3DCustomShader *	pDLShader =
			pDevice->GetDefaultShaderProgramAs
				( S3DRenderDevice::DefaultShaderId::DelayLight ) ;
	S3DDelayLightShaderInterface *
		pdlsiLight = ESLTypeCast<S3DDelayLightShaderInterface>( pDLShader ) ;
	if ( pdlsiLight == NULL )
	{
		return ;
	}
	//
	// 空間行列・カメラ行列取得
	//
	S3DDMatrix	matItem, matCamera ;
	S3DDVector	vItem, vCamera ;
	GetGlobalTransformation( matItem, vItem ) ;
	matCamera = scene.GetCurrentCameraTransformation( vCamera ) ;
	//
	// 光源情報
	//
	S3DLightEntry	light = m_light ;
	light.vecPosition = matCamera * vItem + vCamera ;
	light.vecDirection =
			matCamera * (matItem * S3DDVector(light.vecDirection)) ;
	light.fpBrightness *= (float32_t) scene.GetLightSensitivity() ;
	//
	// ※フレームバッファへの描画はｙ座標を反転する
	light.vecPosition.y = - light.vecPosition.y ;
	light.vecDirection.y = - light.vecDirection.y ;
	//
	// バッファを準備
	//
	SGLImageObject*const*	pLastMultiTarget ;
	size_t					nLastMultiTargets ;
	pLastMultiTarget = render.GetMultiTargetImages( nLastMultiTargets ) ;
	//
	SSystem::SPointerArray<SGLImageObject>	aSaveMultiTargets ;
	aSaveMultiTargets.AddArray( pLastMultiTarget, nLastMultiTargets ) ;
	//
	SGLImageObject *	pMultiTargets[S3DScene::renderTargetCount] ;
	for ( size_t i = 0; (i < nMultiImages)
						&& (i < S3DScene::renderTargetCount); i ++ )
	{
		pMultiTargets[i] = ppImage[i] ;
	}
	//
	if ( m_flagWriteEmission
		&& (nMultiImages >= S3DScene::renderTargetEmission + 1) )
	{
		render.AttachMultiTargetImages
				( ppImage + 1, S3DScene::renderTargetEmission ) ;
	}
	else
	{
		render.AttachMultiTargetImages( NULL, 0 ) ;
	}
	//
	// シェーダーにパラメータ設定
	//
	S4DMatrix	matPers = scene.GetCurrentPerspective() ;
	//
	pdlsiLight->SetLightParam( light ) ;
	pdlsiLight->SetLightApplication
		( (float32_t) m_fpDiffusion, (float32_t) m_fpSpecular ) ;
	pdlsiLight->SetAirScattering
		( (float32_t) m_fpAirScattering, (float32_t) u_fpAirUnitLength ) ;
	pdlsiLight->SetPerspective( matPers ) ;
	pdlsiLight->SetSourceBuffer( pMultiTargets, nMultiImages, pDepth ) ;
	//
	// シェーダーを設定して描画
	//
	S3DCustomShader *	pOldShader = render.GetCustomShader() ;
	render.AttachCustomShader( pDLShader ) ;
	//
	SGLPaintParam	pp ;
	pp.nTransparency =
		(uint32_t) esl_clampi( (int) esl_lroundfi
						( (1.0 - m_fpDistanceFade) * 0x100 ), 0, 0x100 ) ;
	render.DrawImage( pp, ppImage[S3DScene::renderTargetDiffusion], NULL ) ;
	render.Finish() ;
	//
	render.AttachMultiTargetImages
		( aSaveMultiTargets.GetConstArray(), aSaveMultiTargets.GetLength() ) ;
	render.AttachCustomShader( pOldShader ) ;
#endif
}



//////////////////////////////////////////////////////////////////////////////
// 大域照明効果アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DSSGlobalIlluminationSerializer::m_paramEntries
				[S3DSSGlobalIlluminationSerializer::paramGICount] =
{
	{ L"write_emission",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory1,
		L"発光出力", L"発光フレームバッファにも書き出す" },
	{ L"ao_reach",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"AO到達距離", NULL },
	{ L"ao_sampling_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"AOサンプリング数", NULL },
	{ L"gi_sampling_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"GIサンプリング数", NULL },
	{ L"sampling_scale",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration,
		L"サンプリング比", NULL },
	{ L"gi_luminousness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"拡散反射輝度", NULL, 0.0, 1.0 },
	{ L"ao_blend_ratio",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"AO適用度", NULL, 0.0, 1.0 },
	{ L"ao_shade_color",
		S3DSceneComposer::typeColor,
		S3DSceneComposer::attrCategory1,
		L"AO影色", NULL },
	{ L"gi_blend_ratio",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"GI適用度", NULL, 0.0, 1.0 },
	{ L"make_panorama",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"パノラマ画像生成",
		L"AO/GI 処理用にパノラマ画像をレンダリングする" },
	{ L"panorama_size",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant2,
		L"パノラマ画像サイズ",
		L"3N×N のパノラマ画像が生成されます" },
	{ L"panorama_ref_backscape",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"backscape 反映",
		L"パノラマ画像に backspace を描画する" },
	{ L"panorama_ref_scape",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"scape 反映",
		L"パノラマ画像に space を描画する" },
	{ L"panorama_ref_field",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"field 反映",
		L"パノラマ画像に field を描画する" },
	{ L"panorama_ref_static_item1",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"static_item1 反映",
		L"パノラマ画像に static_item1 を描画する" },
	{ L"panorama_ref_static_item2",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"static_item2 反映",
		L"パノラマ画像に static_item2 を描画する" },
	{ L"panorama_ref_dynamic_item1",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"dynamic_item1 反映",
		L"パノラマ画像に dynamic_item1 を描画する" },
	{ L"panorama_ref_dynamic_item2",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"dynamic_item2 反映",
		L"パノラマ画像に dynamic_item2 を描画する" },
	{ L"panorama_ref_dynamic_item3",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"dynamic_item3 反映",
		L"パノラマ画像に dynamic_item3 を描画する" },
	{ L"panorama_ref_effect_item",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"effect_item 反映",
		L"パノラマ画像に effect_item を描画する" },
	{ L"panorama_ref_layered_space",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"layered_space 反映",
		L"パノラマ画像に layered_space を描画する" },
	{ L"panorama_ref_layered1",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"layered1 反映",
		L"パノラマ画像に layered1 を描画する" },
	{ L"panorama_ref_layered2",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory2,
		L"layered2 反映",
		L"パノラマ画像に layered2 を描画する" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DSSGlobalIlluminationSerializer::m_pscClass =
{
	&S3DCommonEffectorItemSerializer::m_pscClass,
	S3DSSGlobalIlluminationSerializer::paramGICount,
	&S3DSSGlobalIlluminationSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DSSGlobalIlluminationSerializer, S3DCommonEffectorItemSerializer, Effector )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DSSGlobalIlluminationSerializer, ssgi_effect )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSSGlobalIlluminationSerializer::S3DSSGlobalIlluminationSerializer( void )
	: S3DCommonEffectorItemSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DSSGlobalIlluminationSerializer::m_pscClass )
{
	m_maskClasses |= (1 << S3DScene::classPreRenderFrame) ;
	SetDrawingPriority( 0 ) ;
	AttachSceneEffector( this ) ;
	//
	m_fpAOReachDistance = 200.0 ;
	m_nAOSamplingCount = 32 ;
	m_nGISamplingCount = 16 ;
	m_nSamplingScale = 1 ;
	m_fpLuminousness = 0.1 ;
	m_fpAOBlendRatio = 0.5 ;
	m_fpGIBlendRatio = 0.5 ;
	m_rgbAOShadeColor = 0 ;
	m_flagWriteEmission = false ;
	m_flag3WayPanorama = false ;
	m_classes3WayPanorama = S3DScene::classBitsAllScape
								| S3DScene::classBitsItems ;
	m_n3WayPanoramaSize = 256 ;
	//
	m_pDevice = NULL ;
	m_pRender = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSSGlobalIlluminationSerializer::~S3DSSGlobalIlluminationSerializer( void )
{
	delete	m_pRender ;
	m_pRender = NULL ;
}

// AO 到達距離
//////////////////////////////////////////////////////////////////////////////
double S3DSSGlobalIlluminationSerializer::GetAOReachDistance( void ) const
{
	return	m_fpAOReachDistance ;
}

void S3DSSGlobalIlluminationSerializer::SetAOReachDistance( double fpReach )
{
	m_fpAOReachDistance = fpReach ;
}

// AO サンプリング数
//////////////////////////////////////////////////////////////////////////////
size_t S3DSSGlobalIlluminationSerializer::GetAOSamplingCount( void ) const
{
	return	m_nAOSamplingCount ;
}

void S3DSSGlobalIlluminationSerializer::SetAOSamplingCount( size_t nCount )
{
	m_nAOSamplingCount = nCount ;
}

// GI サンプリング数
//////////////////////////////////////////////////////////////////////////////
size_t S3DSSGlobalIlluminationSerializer::GetGISamplingCount( void ) const
{
	return	m_nGISamplingCount ;
}

void S3DSSGlobalIlluminationSerializer::SetGISamplingCount( size_t nCount )
{
	m_nGISamplingCount = nCount ;
}

// サンプリングスケール
//////////////////////////////////////////////////////////////////////////////
size_t S3DSSGlobalIlluminationSerializer::GetSamplingScale( void ) const
{
	return	m_nSamplingScale ;
}

void S3DSSGlobalIlluminationSerializer::SetSamplingScale( size_t nScale )
{
	m_nSamplingScale = nScale ;
}

// 拡散反射輝度
//////////////////////////////////////////////////////////////////////////////
double S3DSSGlobalIlluminationSerializer::GetDiffusionLuminousness( void ) const
{
	return	m_fpLuminousness ;
}

void S3DSSGlobalIlluminationSerializer::SetDiffusionLuminousness( double fpLuminousness )
{
	m_fpLuminousness = fpLuminousness ;
}

// 適用度
//////////////////////////////////////////////////////////////////////////////
double S3DSSGlobalIlluminationSerializer::GetAOBlendRatio( void ) const
{
	return	m_fpAOBlendRatio ;
}

double S3DSSGlobalIlluminationSerializer::GetGIBlendRatio( void ) const
{
	return	m_fpGIBlendRatio ;
}

void S3DSSGlobalIlluminationSerializer::SetAOBlendRatio( double fpBlend )
{
	m_fpAOBlendRatio = fpBlend ;
}

void S3DSSGlobalIlluminationSerializer::SetGIBlendRatio( double fpBlend )
{
	m_fpGIBlendRatio = fpBlend ;
}

// AO 影色
//////////////////////////////////////////////////////////////////////////////
const SGLPalette& S3DSSGlobalIlluminationSerializer::GetAOShadeColor( void ) const
{
	return	m_rgbAOShadeColor ;
}

void S3DSSGlobalIlluminationSerializer::SetAOShadeColor( const SGLPalette& rgbColor )
{
	m_rgbAOShadeColor = rgbColor ;
}

// 発光出力
//////////////////////////////////////////////////////////////////////////////
bool S3DSSGlobalIlluminationSerializer::IsEnabledWriteEmission( void ) const
{
	return	m_flagWriteEmission ;
}

void S3DSSGlobalIlluminationSerializer::EnableWriteEmission( bool fEmission )
{
	m_flagWriteEmission = fEmission ;
}

// 3面パノラマ画像有効化
//////////////////////////////////////////////////////////////////////////////
bool S3DSSGlobalIlluminationSerializer::IsEnabled3WayPanorama( void ) const
{
	return	m_flag3WayPanorama ;
}

void S3DSSGlobalIlluminationSerializer::Enable3WayPanorama( bool fEnable )
{
	m_flag3WayPanorama = fEnable ;
}

// 3面パノラマ画像サイズ設定
//////////////////////////////////////////////////////////////////////////////
int S3DSSGlobalIlluminationSerializer::Get3WayPanoramaSize( void ) const
{
	return	m_n3WayPanoramaSize ;
}

void S3DSSGlobalIlluminationSerializer::Set3WayPanoramaSize( int nSize )
{
	m_n3WayPanoramaSize = nSize ;
}

// 3面パノラマ描画対象
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSSGlobalIlluminationSerializer::Get3WayPanoramaClassesMask( void ) const
{
	return	m_classes3WayPanorama ;
}

void S3DSSGlobalIlluminationSerializer::Set3WayPanoramaClassesMask( uint32_t maskClasses )
{
	m_classes3WayPanorama = maskClasses ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSSGlobalIlluminationSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAOShadeColor:
		return	VectorFromColor( m_rgbAOShadeColor ) ;
	}
	return	S3DCommonEffectorItemSerializer::GetVectorParameter( i ) ;
}

double S3DSSGlobalIlluminationSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAOReachDistance:
		return	m_fpAOReachDistance ;
	case	paramLuminousness:
		return	m_fpLuminousness ;
	case	paramAOBlendRatio:
		return	m_fpAOBlendRatio ;
	case	paramGIBlendRatio:
		return	m_fpGIBlendRatio ;
	}
	return	S3DCommonEffectorItemSerializer::GetScalarParameter( i ) ;
}

int32_t S3DSSGlobalIlluminationSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAOSamplingCount:
		return	(int32_t) m_nAOSamplingCount ;
	case	paramGISamplingCount:
		return	(int32_t) m_nGISamplingCount ;
	case	paramSamplingScale:
		return	(int32_t) m_nSamplingScale ;
	case	paramPanoramaImageSize:
		return	m_n3WayPanoramaSize ;
	}
	return	S3DCommonEffectorItemSerializer::GetIntegerParameter( i ) ;
}

bool S3DSSGlobalIlluminationSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramWriteEmission:
		return	m_flagWriteEmission ;
	case	paramMakePanorama:
		return	m_flag3WayPanorama ;
	case	paramRefRenderBackscape:
		return	(m_classes3WayPanorama & S3DScene::classBitBackscape) != 0 ;
	case	paramRefRenderScape:
		return	(m_classes3WayPanorama & S3DScene::classBitScape) != 0 ;
	case	paramRefRenderField:
		return	(m_classes3WayPanorama & S3DScene::classBitField) != 0 ;
	case	paramRefRenderStaticItem1:
		return	(m_classes3WayPanorama & S3DScene::classBitStaticItem1) != 0 ;
	case	paramRefRenderStaticItem2:
		return	(m_classes3WayPanorama & S3DScene::classBitStaticItem2) != 0 ;
	case	paramRefRenderDynamicItem1:
		return	(m_classes3WayPanorama & S3DScene::classBitDynamicItem1) != 0 ;
	case	paramRefRenderDynamicItem2:
		return	(m_classes3WayPanorama & S3DScene::classBitDynamicItem2) != 0 ;
	case	paramRefRenderDynamicItem3:
		return	(m_classes3WayPanorama & S3DScene::classBitDynamicItem3) != 0 ;
	case	paramRefRenderEffectItem:
		return	(m_classes3WayPanorama & S3DScene::classBitEffectItem) != 0 ;
	case	paramRefRenderLayerdSpace:
		return	(m_classes3WayPanorama & S3DScene::classBitLayeredSpace) != 0 ;
	case	paramRefRenderLayerdItem1:
		return	(m_classes3WayPanorama & S3DScene::classBitLayeredItem1) != 0 ;
	case	paramRefRenderLayerdItem2:
		return	(m_classes3WayPanorama & S3DScene::classBitLayeredItem2) != 0 ;
	}
	return	S3DCommonEffectorItemSerializer::GetBooleanParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DSSGlobalIlluminationSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramAOShadeColor:
		m_rgbAOShadeColor = ColorFromVector( vec ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetVectorParameter( i, vec ) ;
}

void S3DSSGlobalIlluminationSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramAOReachDistance:
		SetAOReachDistance( s ) ;
		return ;
	case	paramLuminousness:
		SetDiffusionLuminousness( s ) ;
		return ;
	case	paramAOBlendRatio:
		SetAOBlendRatio( s ) ;
		return ;
	case	paramGIBlendRatio:
		SetGIBlendRatio( s ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetScalarParameter( i, s ) ;
}

void S3DSSGlobalIlluminationSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramAOSamplingCount:
		SetAOSamplingCount( (size_t) n ) ;
		return ;
	case	paramGISamplingCount:
		SetGISamplingCount( (size_t) n ) ;
		return ;
	case	paramSamplingScale:
		SetSamplingScale( (size_t) n ) ;
		return ;
	case	paramPanoramaImageSize:
		Set3WayPanoramaSize( (int) n ) ;
		return ;
	}
	return	S3DCommonEffectorItemSerializer::SetIntegerParameter( i, n ) ;
}

void S3DSSGlobalIlluminationSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramWriteEmission:
		EnableWriteEmission( b ) ;
		return ;
	case	paramMakePanorama:
		Enable3WayPanorama( b ) ;
		return ;
	case	paramRefRenderBackscape:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitBackscape)
								| (b ? S3DScene::classBitBackscape : 0) ;
		return ;
	case	paramRefRenderScape:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitScape)
								| (b ? S3DScene::classBitScape : 0) ;
		return ;
	case	paramRefRenderField:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitField)
								| (b ? S3DScene::classBitField : 0) ;
		return ;
	case	paramRefRenderStaticItem1:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitStaticItem1)
								| (b ? S3DScene::classBitStaticItem1 : 0) ;
		return ;
	case	paramRefRenderStaticItem2:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitStaticItem2)
								| (b ? S3DScene::classBitStaticItem2 : 0) ;
		return ;
	case	paramRefRenderDynamicItem1:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitDynamicItem1)
								| (b ? S3DScene::classBitDynamicItem1 : 0) ;
		return ;
	case	paramRefRenderDynamicItem2:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitDynamicItem2)
								| (b ? S3DScene::classBitDynamicItem2 : 0) ;
		return ;
	case	paramRefRenderDynamicItem3:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitDynamicItem3)
								| (b ? S3DScene::classBitDynamicItem3 : 0) ;
		return ;
	case	paramRefRenderEffectItem:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitEffectItem)
								| (b ? S3DScene::classBitEffectItem : 0) ;
		return ;
	case	paramRefRenderLayerdSpace:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitLayeredSpace)
								| (b ? S3DScene::classBitLayeredSpace : 0) ;
		return ;
	case	paramRefRenderLayerdItem1:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitLayeredItem1)
								| (b ? S3DScene::classBitLayeredItem1 : 0) ;
		return ;
	case	paramRefRenderLayerdItem2:
		m_classes3WayPanorama =
			(m_classes3WayPanorama & ~S3DScene::classBitLayeredItem2)
								| (b ? S3DScene::classBitLayeredItem2 : 0) ;
		return ;
	}
	return	S3DCommonEffectorItemSerializer::SetBooleanParameter( i, b ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DSSGlobalIlluminationSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramSamplingScale:
		aStrSet.Add( new SString( L"1" ) ) ;
		aStrSet.Add( new SString( L"2" ) ) ;
		return	true ;
	}
	return	S3DCommonEffectorItemSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DSSGlobalIlluminationSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"大域照明設定" ;
	case	2:
		return	L"GI用パノラマ画像" ;
	}
	return	S3DCommonEffectorItemSerializer::GetParameterCategoryName( iCategory ) ;
}


// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DSSGlobalIlluminationSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( clsItem == S3DScene::classPreRenderFrame )
	{
		if ( (m_fpAOBlendRatio > 0.004) || (m_fpGIBlendRatio > 0.004) )
		{
			if ( m_flag3WayPanorama )
			{
				Render3WayPanoramaBuffer( scene ) ;
			}
			S3DCommonEffectorItemSerializer::OnItemRenderEvent( scene, clsItem ) ;
		}
	}
	else
	{
		if ( (m_fpAOBlendRatio > 0.004) || (m_fpGIBlendRatio > 0.004) )
		{
			S3DCommonEffectorItemSerializer::OnItemRenderEvent( scene, clsItem ) ;
		}
	}
}

// ３面パノラマレンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DSSGlobalIlluminationSerializer::Render3WayPanoramaBuffer( S3DScene& scene )
{
	//
	// バッファ確保
	//
	SGLSize	sizeBuf( m_n3WayPanoramaSize * 3, m_n3WayPanoramaSize ) ;
	for ( int i = 0; i < 2; i ++ )
	{
		if ( m_image3WayPanorama[i].GetImageSize() != sizeBuf )
		{
			m_image3WayPanorama[i].CreateImage
				( sizeBuf.w, sizeBuf.h, formatImageABGR, 32,
					SGLImageObject::bufferForRenderTarget
						| SGLImageObject::bufferNonPowerOf2
						| SGLImageObject::bufferOnDeviceOnly ) ;
		}
	}
	if ( m_depth3WayPanorama.GetImageSize() != sizeBuf )
	{
		m_depth3WayPanorama.CreateImage
			( sizeBuf.w, sizeBuf.h, formatImageDepth, 32,
				SGLImageObject::bufferForRenderTarget
					| SGLImageObject::bufferNonPowerOf2
					| SGLImageObject::bufferOnDeviceOnly ) ;
	}
	//
	// カメラ情報取得
	//
	S3DDVector	vCameraPos( 0, 0, 0 ) ;
	S3DDVector	vCameraTarget( 0, 0, 1 ) ;
	S3DDVector	vCameraTop( 0, -1, 0 ) ;
	//
	S3DScene::Camera *	pCurCamera = scene.GetCurrentCamera() ;
	if ( pCurCamera != NULL )
	{
		vCameraPos = pCurCamera->GetCameraPosition() ;
		vCameraTarget = pCurCamera->GetCameraTarget() ;
		vCameraTop = pCurCamera->GetCameraTop() ;
		//
		S3DDVector	vDeltaTarget = vCameraTarget - vCameraPos ;
		S3DDVector	vTempCross = vDeltaTarget * vCameraTop ;
		S3DDVector	vTempTop = - vDeltaTarget * vTempCross ;
		//
		vCameraTop = vTempTop ;
		vCameraTop.Normalize() ;
	}
	//
	// パノラマ３分割描画
	//
	if ( m_pRender == NULL )
	{
		m_pRender = new S3DRenderContext ;
	}
	SGLImageObject *	pMultiTargets[1] =
	{
		&m_image3WayPanorama[1],
	} ;
	m_pRender->AttachTargetImage
		( &m_image3WayPanorama[0], &m_depth3WayPanorama, NULL ) ;
	m_pRender->AttachMultiTargetImages( &pMultiTargets[0], 1 ) ;
	scene.SetShadingConfig( *m_pRender, true ) ;
	m_pRender->Begin3DRenderer() ;
	m_pRender->FillClearTarget( 0 ) ;
	m_pRender->End3DRenderer() ;
	m_pRender->Finish() ;
	//
	S3DScene::ProjectionParam	projParam ;
	scene.GetProjection( projParam ) ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		//
		// レンダリング設定
		//
		SGLImageRect	rectView
			( i * m_n3WayPanoramaSize, 0,
				m_n3WayPanoramaSize, m_n3WayPanoramaSize ) ;
		//
		m_pRender->AttachTargetImage
			( &m_image3WayPanorama[0], &m_depth3WayPanorama, &rectView ) ;
		m_pRender->AttachMultiTargetImages( &pMultiTargets[0], 1 ) ;
		//
		S3DVector	vScreen ;
		vScreen.x = (float32_t) (m_n3WayPanoramaSize / 2
									+ i * m_n3WayPanoramaSize) ;
		vScreen.y = (float32_t) (m_n3WayPanoramaSize / 2) ;
		vScreen.z = (float32_t) (m_n3WayPanoramaSize / 2)
									* 0.57735026918962576450914878050195f ;
		m_pRender->SetProjectionScreen( vScreen, 1.0 ) ;
		m_pRender->SetZClipRange( projParam.zNear, projParam.zFar ) ;
		//
		m_pRender->GetPerspectiveMatrix( m_mat3WayPanoramaPers ) ;
		//
		// カメラ設定
		//
		S3DDMatrix	matRotCamera ;
		double		radCameraRot = (i - 1) * PI * (-120.0 / 180.0) ;
		matRotCamera.RotationOnVectorOf
			( vCameraTop, sin(radCameraRot), cos(radCameraRot) ) ;
		//
		m_camraPanorama.SetCameraPosition( vCameraPos ) ;
		m_camraPanorama.SetCameraTop( vCameraTop ) ;
		m_camraPanorama.SetCameraTarget
			( vCameraPos + matRotCamera * (vCameraTarget - vCameraPos) ) ;
		//
		// レンダリング
		//
		uint64_t	flagsExclusion = shadingNoReflectObject ;
		if ( i == 1 )
		{
			flagsExclusion |= shadingNoZBuffer | shadingZBufferNoWrite ;
		}
		scene.RenderSceneTemporary
			( *m_pRender, &m_camraPanorama, false,
				flagsExclusion, m_classes3WayPanorama ) ;
	}
}

// 要求カラーバッファ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DSSGlobalIlluminationSerializer::GetRequiredColorBufferMask( void ) const
{
	if ( (m_fpAOBlendRatio <= 0.0) && (m_fpGIBlendRatio <= 0.0) )
	{
		return	0 ;
	}
	return	(1 << S3DScene::renderTargetComposed)
			| (1 << S3DScene::renderTargetEmission)
			| (1 << S3DScene::renderTargetNormal)
			| (1 << S3DScene::renderTargetAmbient)
			| (1 << S3DScene::renderTargetDiffusion)
			| (1 << S3DScene::renderTargetSpecular) ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void S3DSSGlobalIlluminationSerializer::DrawEffect
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
#if	!defined(__COTOPHA__)
	if ( (m_fpAOBlendRatio <= 0.0) && (m_fpGIBlendRatio <= 0.0) )
	{
		return ;
	}
	if ( nMultiImages <= S3DScene::renderTargetSpecular )
	{
		return ;
	}
	S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
	if ( pDevice == NULL )
	{
		return ;
	}
	S3DCustomShader *	pSSGISampler =
			pDevice->GetDefaultShaderProgramAs
				( S3DRenderDevice::DefaultShaderId::SSGISampler ) ;
	S3DSSGISamplerInterface *
		pssgiSampler = ESLTypeCast<S3DSSGISamplerInterface>( pSSGISampler ) ;
	if ( pssgiSampler == NULL )
	{
		return ;
	}
	S3DCustomShader *	pSSGIComposer =
			pDevice->GetDefaultShaderProgramAs
				( S3DRenderDevice::DefaultShaderId::SSGIComposer ) ;
	S3DSSGIComposerInterface *
		pssgiComposer = ESLTypeCast<S3DSSGIComposerInterface>( pSSGIComposer ) ;
	if ( pssgiComposer == NULL )
	{
		return ;
	}
	//
	// 作業バッファ取得
	//
	SGLImageObject *	pGISampleBuf = scene.GetTemporaryRenderBuffer( 0 ) ;
	if ( pGISampleBuf == NULL )
	{
		return ;
	}
	//
	// レンダラ準備
	//
	if ( (pDevice != m_pDevice) || (m_pRender == NULL) )
	{
		delete	m_pRender ;
		m_pDevice = pDevice ;
		m_pRender = pDevice->NewRenderer() ;
	}
	if ( m_pRender == NULL )
	{
		return ;
	}
	//
	// バッファを準備
	//
	SGLImageObject*const*	pLastMultiTarget ;
	size_t					nLastMultiTargets ;
	pLastMultiTarget = render.GetMultiTargetImages( nLastMultiTargets ) ;
	//
	SSystem::SPointerArray<SGLImageObject>	aSaveMultiTargets ;
	aSaveMultiTargets.AddArray( pLastMultiTarget, nLastMultiTargets ) ;
	//
	if ( m_flagWriteEmission
		&& (nMultiImages >= S3DScene::renderTargetEmission + 1) )
	{
		render.AttachMultiTargetImages
				( ppImage + 1, S3DScene::renderTargetEmission ) ;
	}
	else
	{
		render.AttachMultiTargetImages( NULL, 0 ) ;
	}
	//
	// サンプリングシェーダーにパラメータ設定
	//
	S4DMatrix	matPers = scene.GetCurrentPerspective() ;
	//
	pssgiSampler->SetAOReachDistance
				( (float32_t) m_fpAOReachDistance, 1.0f ) ;
	pssgiSampler->SetAOSamplingCount
		( (size_t) esl_min( (int) m_nAOSamplingCount, 200 ) ) ;
	if ( m_fpGIBlendRatio > 0.0 )
	{
		pssgiSampler->SetGISamplingCount
			( (size_t) esl_min( (int) m_nGISamplingCount, 200 ) ) ;
	}
	else
	{
		pssgiSampler->SetGISamplingCount( 0 ) ;
	}
	pssgiSampler->SetGILuminousness( (float32_t) m_fpLuminousness ) ;
	pssgiSampler->SetPerspective( matPers ) ;
	pssgiSampler->SetSourceBuffer( ppImage, nMultiImages, pDepth ) ;
	//
	pssgiSampler->Enable3WayPanoramaBuffer( m_flag3WayPanorama ) ;
	if ( m_flag3WayPanorama )
	{
		SGLImageObject*	pPanoramaBuf[2]=
		{
			&m_image3WayPanorama[0],
			&m_image3WayPanorama[1],
		} ;
		pssgiSampler->Set3WayPerspective( m_mat3WayPanoramaPers ) ;
		pssgiSampler->Set3WayPanoramaBuffer
				( &pPanoramaBuf[0], 2, &m_depth3WayPanorama ) ;
	}
	//
	// サンプリングシェーダーを設定して描画
	//
	SGLSize			sizeImage = pGISampleBuf->GetImageSize() ;
	SGLImageRect	rectView ;
	rectView.x = 0 ;
	rectView.y = 0 ;
	rectView.w = sizeImage.w ;
	rectView.h = sizeImage.h ;
	//
	SGLPaintParam	pp ;
	SGLAffine		affine( 1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f ) ;
	if ( m_nSamplingScale > 1 )
	{
		rectView.w /= (int32_t) m_nSamplingScale ;
		rectView.h /= (int32_t) m_nSamplingScale ;
		//
		affine.a11 = (float32_t) rectView.w / (float32_t) sizeImage.w ;
		affine.a12 = 0.0f ;
		affine.a13 = 0.0f ;
		affine.a21 = 0.0f ;
		affine.a22 = (float32_t) rectView.h / (float32_t) sizeImage.h ;
		affine.a23 = 0.0f ;
		pp.pAffine = &affine ;
	}
	m_pRender->AttachCustomShader( pSSGISampler ) ;
	m_pRender->AttachTargetImage( pGISampleBuf, NULL, NULL ) ;
	m_pRender->FillClearTarget( 0 ) ;
	m_pRender->DrawImage
		( pp, ppImage[S3DScene::renderTargetComposed], NULL ) ;
	m_pRender->Finish() ;
	m_pRender->DetachTargetImage() ;
	m_pRender->AttachCustomShader( NULL ) ;
	//
	// 描画シェーダーにパラメータ設定
	//
	if ( m_nSamplingScale > 1 )
	{
		affine.a11 = (float32_t) sizeImage.w / (float32_t) rectView.w ;
		affine.a22 = (float32_t) sizeImage.h / (float32_t) rectView.h ;
		pp.pAffine = &affine ;
	}
	S2DVector	vSamplingScale( affine.a11, affine.a22 ) ;
	pssgiComposer->SetSamplingScale( vSamplingScale ) ;
	pssgiComposer->SetBlendRatio
		( (float32_t) m_fpAOBlendRatio, (float32_t) m_fpGIBlendRatio ) ;
	pssgiComposer->SetAOShadeColor( m_rgbAOShadeColor ) ;
	pssgiComposer->SetSourceBuffer( ppImage, nMultiImages ) ;
	//
	// 描画シェーダーで GI 反映
	//
	S3DCustomShader *	pOldShader = render.GetCustomShader() ;
	render.AttachCustomShader( pSSGIComposer ) ;
	//
	render.DrawImage( pp, pGISampleBuf, &rectView ) ;
	render.Finish() ;
	//
	render.AttachMultiTargetImages
		( aSaveMultiTargets.GetConstArray(), aSaveMultiTargets.GetLength() ) ;
	render.AttachCustomShader( pOldShader ) ;
#endif
}


//////////////////////////////////////////////////////////////////////////////
// グロー効果アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DGlowEffectSerializer::m_paramEntries
		[S3DGlowEffectSerializer::paramGlowCount] =
{
	{ L"glow_source",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrConstant1,
		L"ソース", L"グロー効果に使用する入力チャネル" },
	{ L"glow_gauss",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"ガウス", L"ぼかし係数", 0.0, 20.0 },
	{ L"gauss_sampling_scale",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"サンプリングスケール",
		L"ぼかし処理でのサンプリングスケール", 1.0, 4.0 },
	{ L"glow_brightness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"輝度", NULL, 0.0, 2.0 },
	{ L"effect_alpha",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"描画適用度", NULL, 0.0, 1.0 },
} ;

const S3DSceneComposer::ParamSetClass
	S3DGlowEffectSerializer::m_pscClass =
{
	&S3DCommonEffectorItemSerializer::m_pscClass,
	S3DGlowEffectSerializer::paramGlowCount,
	&S3DGlowEffectSerializer::m_paramEntries[0]
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DGlowEffectSerializer::m_aiGlowSource[3] =
{
	{ L"composed", S3DRenderDevice::renderTargetComposed },
	{ L"emission", S3DRenderDevice::renderTargetEmission },
	{ NULL, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DGlowEffectSerializer,
		S3DCommonEffectorItemSerializer, EmissiveGlowEffector )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DGlowEffectSerializer, glow_effector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DGlowEffectSerializer::S3DGlowEffectSerializer( void )
	: S3DCommonEffectorItemSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DGlowEffectSerializer::m_pscClass )
{
	SetDrawingPriority( 20 ) ;
	SetEffectorClass( effectorGlobalBelow ) ;
	AttachSceneEffector( this ) ;
	//
	SetAlpha( 1.0 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DGlowEffectSerializer::~S3DGlowEffectSerializer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DGlowEffectSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGauss:
		return	GetGauss() ;
	case	paramSamplingScale:
		return	GetZoom() ;
	case	paramBrighness:
		return	GetBrightness() ;
	case	paramAlpha:
		return	GetAlpha() ;
	}
	return	S3DCommonEffectorItemSerializer::GetScalarParameter( i ) ;
}

const wchar_t * S3DGlowEffectSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGlowSource:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiGlowSource, GetSourceIndex() ) ;
	}
	return	S3DCommonEffectorItemSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DGlowEffectSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramGauss:
		SetGauss( (float32_t) s ) ;
		return ;
	case	paramSamplingScale:
		SetZoom( (float32_t) s ) ;
		return ;
	case	paramBrighness:
		SetBrightness( (float32_t) s ) ;
		return ;
	case	paramAlpha:
		SetAlpha( (float32_t) s ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetScalarParameter( i, s ) ;
}

void S3DGlowEffectSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramGlowSource:
		SetSourceIndex
			( (int) SXMLDocument::GetIntegerAsSymbolOf
					( m_aiGlowSource, pwszCmd, GetSourceIndex() ) ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DGlowEffectSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	int	j ;
	switch ( i )
	{
	case	paramGlowSource:
		for ( j = 0; m_aiGlowSource[j].pszSymbol; j ++ )
		{
			aStrSet.Add( new SString( m_aiGlowSource[j].pszSymbol ) ) ;
		}
		return	true ;
	}
	return	S3DCommonEffectorItemSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DGlowEffectSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"グロー設定" ;
	}
	return	S3DCommonEffectorItemSerializer::GetParameterCategoryName( iCategory ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 擬似被写界深度効果アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DDepthOfFieldEffectSerializer::m_paramEntries
		[S3DDepthOfFieldEffectSerializer::paramDOFCount] =
{
	{ L"blur_gauss",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"ガウス", L"ぼかし係数", 0.0, 20.0 },
	{ L"dof_near_depth",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"手前深度", NULL },
	{ L"dof_far_depth",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"奥深度", NULL },
} ;

const S3DSceneComposer::ParamSetClass
	S3DDepthOfFieldEffectSerializer::m_pscClass =
{
	&S3DCommonEffectorItemSerializer::m_pscClass,
	S3DDepthOfFieldEffectSerializer::paramDOFCount,
	&S3DDepthOfFieldEffectSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DDepthOfFieldEffectSerializer, S3DCommonEffectorItemSerializer, DepthOfFieldEffector )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DDepthOfFieldEffectSerializer, dof_effector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDepthOfFieldEffectSerializer::S3DDepthOfFieldEffectSerializer( void )
	: S3DCommonEffectorItemSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DDepthOfFieldEffectSerializer::m_pscClass )
{
	SetDrawingPriority( 22 ) ;
	SetEffectorClass( effectorGlobalBelow ) ;
	AttachSceneEffector( this ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DDepthOfFieldEffectSerializer::~S3DDepthOfFieldEffectSerializer( void )
{
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DDepthOfFieldEffectSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramPosition:
		return	true ;
	}
	return	S3DCommonEffectorItemSerializer::IsParameterValidation( i ) ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DDepthOfFieldEffectSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGauss:
		return	GetGauss() ;
	case	paramNearDepth:
		return	GetNearDepth() ;
	case	paramFarDepth:
		return	GetFarDepth() ;
	}
	return	S3DCommonEffectorItemSerializer::GetScalarParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DDepthOfFieldEffectSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramGauss:
		SetGauss( (float32_t) s ) ;
		return ;
	case	paramNearDepth:
		SetDepth( (float32_t) s, GetFarDepth() ) ;
		return ;
	case	paramFarDepth:
		SetDepth( GetNearDepth(), (float32_t) s ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetScalarParameter( i, s ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DDepthOfFieldEffectSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"被写界深度" ;
	}
	return	S3DCommonEffectorItemSerializer::GetParameterCategoryName( iCategory ) ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DDepthOfFieldEffectSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( clsItem == S3DScene::classPreRender )
	{
		S3DScene::Camera *	pCamera = scene.GetCurrentCamera() ;
		if ( pCamera != NULL )
		{
			S3DDMatrix	matCamera ;
			S3DDVector	vCameraSpace ;
			pCamera->CalcItemLinkTransformation( matCamera, vCameraSpace ) ;
			//
			S3DDVector	vCamera = pCamera->GetCameraPosition() ;
			S3DDVector	vTarget = pCamera->GetCameraTarget() ;
			//
			vCamera = matCamera * vCamera + vCameraSpace ;
			vTarget = matCamera * vTarget + vCameraSpace ;
			//
			S3DDVector	vDirection = vTarget - vCamera ;
			vDirection.Normalize() ;
			//
			S3DDMatrix	matFocus ;
			S3DDVector	vFocus ;
			CalcGlobalTransformation( matFocus, vFocus ) ;
			//
			SetFocus( (float32_t) fabs
						( vDirection.InnerProduct( vFocus - vCamera ) ) ) ;
		}
	}
	S3DCommonEffectorItemSerializer::OnItemRenderEvent( scene, clsItem ) ;
}




//////////////////////////////////////////////////////////////////////////////
// ぼかし・フラッシュ効果アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DCurtainEffectSerializer::m_paramEntries
			[S3DCurtainEffectSerializer::paramCurtainCount] =
{
	{ L"glow_gauss",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"ガウス", L"ぼかし係数", 0.0, 20.0 },
	{ L"gauss_sampling_scale",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"サンプリングスケール",
		L"ぼかし処理でのサンプリングスケール", 1.0, 4.0 },
	{ L"effect_alpha",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"ぼかし適用度", NULL, 0.0, 1.0 },
	{ L"curtain_color",
		S3DSceneComposer::typeColor,
		S3DSceneComposer::attrCategory1,
		L"重ね色", NULL, 0.0, 1.0 },
	{ L"curtain_alpha",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"重ね色α", NULL, 0.0, 1.0 },
	{ L"curtain_fade",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"重ね色フェード", NULL, 0.0, 1.0 },
} ;

const S3DSceneComposer::ParamSetClass
	S3DCurtainEffectSerializer::m_pscClass =
{
	&S3DCommonEffectorItemSerializer::m_pscClass,
	S3DCurtainEffectSerializer::paramCurtainCount,
	&S3DCurtainEffectSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DCurtainEffectSerializer, S3DCommonEffectorItemSerializer, CurtainEffector )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DCurtainEffectSerializer, shading_effector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DCurtainEffectSerializer::S3DCurtainEffectSerializer( void )
	: S3DCommonEffectorItemSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DCurtainEffectSerializer::m_pscClass ),
		m_argbFlash( 0 )
{
	m_flagsBehavior |= S3DScene::itemTimer ;
	//
	SetDrawingPriority( 20 ) ;
	SetEffectorClass( effectorGlobalBelow ) ;
	AttachSceneEffector( this ) ;
	//
	SetAlpha( 1.0 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DCurtainEffectSerializer::~S3DCurtainEffectSerializer( void )
{
}

// フラッシュ追加
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::AddFlash
	( const SGLPalette& argb, uint32_t msecFadein, uint32_t msecFadeout )
{
	Flash	flash ;
	flash.argbColor = argb ;
	flash.msecPast = 0 ;
	flash.msecFadein = msecFadein ;
	flash.msecFadeout = msecFadeout ;
	//
	m_csSync.Lock() ;
	m_aFlash.Add( flash ) ;
	m_csSync.Unlock() ;
}

// フラッシュ処理中か？
//////////////////////////////////////////////////////////////////////////////
bool S3DCurtainEffectSerializer::IsFlashing( void ) const
{
	return	(m_aFlash.GetLength() > 0) ;
}

// フェード追加
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::AddFade
	( const FadeParam& fp, uint32_t msecDuration, uint32_t msecDelay )
{
	FadeEntry	fe ;
	GetCurrentFadeParam( fe.fp0 ) ;
	fe.fp1 = fp ;
	fe.msecPast = 0 ;
	fe.msecDelay = msecDelay ;
	fe.msecDuration = msecDuration ;
	//
	m_csSync.Lock() ;
	m_aFade.Add( fe ) ;
	m_csSync.Unlock() ;
}

// フェード処理中か？
//////////////////////////////////////////////////////////////////////////////
bool S3DCurtainEffectSerializer::IsFading( void ) const
{
	return	(m_aFade.GetLength() > 0) ;
}

// 全てのフェード処理をキャンセル
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::ClearAllFading( void )
{
	m_csSync.Lock() ;
	m_aFade.RemoveAll() ;
	m_csSync.Unlock() ;
}

// 現在のパラメータ取得
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::GetCurrentFadeParam( FadeParam& fp ) const
{
	fp.maskParam = fadeMaskGauss | fadeMaskBrightness
				| fadeMaskAlpha | fadeMaskCurtain | fadeMaskCurtainAlpha ;
	fp.gauss = GetGauss() ;
	fp.brightness = GetBrightness() ;
	fp.alpha = GetAlpha() ;
	fp.argbCurtain = GetCurtainColor() ;
	fp.aCurtain = GetCurtainAlpha() ;
}

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::SetFadeParam( const FadeParam& fp )
{
	if ( fp.maskParam & fadeMaskGauss )
	{
		SetGauss( fp.gauss ) ;
	}
	if ( fp.maskParam & fadeMaskBrightness )
	{
		SetBrightness( fp.brightness ) ;
	}
	if ( fp.maskParam & fadeMaskAlpha )
	{
		SetAlpha( fp.alpha ) ;
	}
	if ( fp.maskParam & fadeMaskCurtain )
	{
		SetCurtainColor( fp.argbCurtain ) ;
	}
	if ( fp.maskParam & fadeMaskCurtainAlpha )
	{
		SetCurtainAlpha( fp.aCurtain ) ;
	}
}

// フェードパラメータ補完
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::LerpFadeParam
	( FadeParam& fpDst,
		const FadeParam& fp0, const FadeParam& fp1, float32_t t )
{
	uint32_t	uit = (uint32_t) esl_clampi( esl_roundfi( t * 0x100 ), 0, 0x100 ) ;
	fpDst.gauss = fp0.gauss + (fp1.gauss - fp0.gauss) * t ;
	fpDst.brightness = fp0.brightness + (fp1.brightness - fp0.brightness) * t ;
	fpDst.alpha = fp0.alpha + (fp1.alpha - fp0.alpha) * t ;
	fpDst.argbCurtain =
		sglPackedColorAdd
			( sglPackedColorMul( fp0.argbCurtain, 0x100 - uit ),
				sglPackedColorMul( fp1.argbCurtain, uit ) ) ;
	fpDst.aCurtain = (fp0.aCurtain * (0x100 - uit) + fp1.aCurtain * uit) >> 8 ;
	fpDst.maskParam = fp0.maskParam & fp1.maskParam ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	S3DCommonEffectorItemSerializer::OnTimer( scene, msecPast ) ;

	m_csSync.Lock() ;
	uint32_t	argbFlash = 0 ;
	for ( size_t i = 0; i < m_aFlash.GetLength(); i ++ )
	{
		Flash&	flash = m_aFlash.At(i) ;
		flash.msecPast += msecPast ;
		if ( flash.msecPast >= flash.msecFadein + flash.msecFadeout )
		{
			m_aFlash.RemoveAt( i -- ) ;
		}
		else if ( flash.msecPast < flash.msecFadein )
		{
			uint32_t	x = flash.msecPast * 0x100 / flash.msecFadein ;
			SGLPalette	argb = flash.argbColor.imul(x) ;
			argbFlash = sglPackedColorBlend( argbFlash, argb.ui32 ) ;
		}
		else
		{
			uint32_t	x = (flash.msecPast - flash.msecFadein)
										* 0x100 / flash.msecFadeout ;
			ESLAssert( x <= 0x100 ) ;
			SGLPalette		argb = flash.argbColor.imul( 0x100 - x ) ;
			argbFlash = sglPackedColorBlend( argbFlash, argb.ui32 ) ;
		}
		scene.PostSceneUpdate() ;
	}
	m_argbEffectColor = argbFlash ;

	for ( size_t i = 0; i < m_aFade.GetLength(); i ++ )
	{
		FadeEntry&	fe = m_aFade.At(i) ;
		fe.msecPast += msecPast ;
		if ( fe.msecPast >= fe.msecDelay + fe.msecDuration )
		{
			SetFadeParam( fe.fp1 ) ;
			m_aFade.RemoveAt( i -- ) ;
			continue ;
		}
		if ( fe.msecPast > fe.msecDelay )
		{
			uint32_t	dt = fe.msecPast - fe.msecDelay ;
			float32_t	t = (float32_t) dt / (float32_t) fe.msecDuration ;
			FadeParam	fp ;
			LerpFadeParam( fp, fe.fp0, fe.fp1, t ) ;
			SetFadeParam( fp ) ;
		}
		scene.PostSceneUpdate() ;
	}
	m_csSync.Unlock() ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DCurtainEffectSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCurtainColor:
		return	VectorFromColor( GetCurtainColor() ) ;
	}
	return	S3DCommonEffectorItemSerializer::GetVectorParameter( i ) ;
}

double S3DCurtainEffectSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGauss:
		return	GetGauss() ;
	case	paramSamplingScale:
		return	GetZoom() ;
	case	paramAlpha:
		return	GetAlpha() ;
	case	paramCurtainAlpha:
		return	(double) GetCurtainColor().argb.Alpha / 255.0 ;
	case	paramCurtainFadeAlpha:
		return	(double) m_nCurtainAlpha / 0x100 ;
	}
	return	S3DCommonEffectorItemSerializer::GetScalarParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DCurtainEffectSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramCurtainColor:
		{
			SGLPalette	argb = ColorFromVector( vec ) ;
			argb.argb.Alpha = GetCurtainColor().argb.Alpha ;
			SetCurtainColor( argb ) ;
		}
		return ;
	}
	S3DCommonEffectorItemSerializer::SetVectorParameter( i, vec ) ;
}

void S3DCurtainEffectSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramGauss:
		SetGauss( (float32_t) s ) ;
		return ;
	case	paramSamplingScale:
		SetZoom( (float32_t) s ) ;
		return ;
	case	paramAlpha:
		SetAlpha( (float32_t) s ) ;
		return ;
	case	paramCurtainAlpha:
		{
			SGLPalette	argb = GetCurtainColor() ;
			argb.argb.Alpha =
				(uint8_t) esl_clampi( (int) esl_lroundfi( s * 255.0 ), 0, 255 ) ;
			SetCurtainColor( argb ) ;
		}
		return ;
	case	paramCurtainFadeAlpha:
		m_nCurtainAlpha =
			(uint32_t) esl_clampi( (int) esl_lroundfi( s * 256.0 ), 0, 0x100 ) ;
		return ;
	}
	S3DCommonEffectorItemSerializer::SetScalarParameter( i, s ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DCurtainEffectSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"ぼかし効果設定" ;
	}
	return	S3DCommonEffectorItemSerializer::GetParameterCategoryName( iCategory ) ;
}


