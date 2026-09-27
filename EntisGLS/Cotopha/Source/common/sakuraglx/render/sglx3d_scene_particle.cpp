
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_item.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// パーティクル発生源
//////////////////////////////////////////////////////////////////////////////

// RenderTarget クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DParticleSerializer::RenderTarget, SObject )

// S3DParticleSerializer クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DParticleSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DParticleSerializer, particle_emitter )

const S3DSceneComposer::ParamEntry
	S3DParticleSerializer::m_paramEntries
		[S3DParticleSerializer::paramParticleCount] =
{
	{ L"render_target",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration,	L"描画アイテム", NULL },
	{ L"use_duration",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,	L"寿命有効", NULL },
	{ L"life_duration",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"寿命", NULL },
	{ L"fadein_duration",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"フェードイン長", NULL },
	{ L"fadeout_duration",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"フェードアウト長", NULL },
	{ L"particle_zoom",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"粒子サイズ（拡大率）", NULL },
	{ L"particle_zoom_indefinition",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"粒子サイズ（揺らぎ率）", NULL },
	{ L"particle_scale_end",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"消滅時拡大率（生成時比）", NULL },
	{ L"animation_speed",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"画像アニメ速度", NULL },
	{ L"rotation_type",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrConstant1,	L"粒子回転指定", NULL },
	{ L"rotation_speed",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,	L"粒子回転速度", NULL },
	{ L"use_face_dir",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,	L"粒子表示向き使用", NULL },
	{ L"face_dir",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,	L"粒子表示向き", NULL },
	{ L"local_space_particle",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,	L"粒子局所座標モード", NULL },
	{ L"blur_frames",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"ブラーフレーム数",
		L"ブラー表示フレーム数。\n0 はブラー無し、最大 4 フレームまで" },
	{ L"blur_supplement",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"ブラー補完距離",
		L"ブラー表示補完距離。\nブラー表示する時に、表示粒子を挿入する間隔" },
	{ L"use_move_aspect",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,	L"移動方向伸長有効", NULL },
	{ L"base_aspect_speed",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"移動方向伸長ベース速度",
		L"粒子の移動方向へ伸長します。ただし、ベース速度以下の場合には縮小はしません。" },
	{ L"emission_direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory2,	L"放出向き", NULL },
	{ L"emission_base_speed",
		S3DSceneComposer::typePosition,
		S3DSceneComposer::attrCategory2,	L"放出加算速度", NULL },
	{ L"emission_min_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,
		L"放出最小角",
		L"放出方向に対して、最小角～（最小角＋範囲角）[deg] の範囲に生成します" },
	{ L"emission_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,
		L"放出範囲角",
		L"放出方向に対して、最小角～（最小角＋範囲角）[deg] の範囲に生成します" },
	{ L"emission_count",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,	L"秒間放出数", NULL },
	{ L"emission_speed",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,	L"放出速度", NULL },
	{ L"emission_indefinition",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,	L"放出速度（揺らぎ率）", NULL },
	{ L"emission_scale",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory2,	L"放出領域サイズ", NULL },
	{ L"emission_shape",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrCategory2,	L"放出領域形状", NULL },
	{ L"acceleration_direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory3,	L"空間加速度向き", NULL },
	{ L"acceleration_velocity",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory3,	L"空間加速度", NULL },
	{ L"stream_direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory3,	L"空間流速向き", NULL },
	{ L"stream_velocity",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory3,	L"空間流速", NULL },
	{ L"attenuation",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory3,	L"秒間減速率", NULL },
	{ L"with_collision",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory3,	L"当たり判定有効", NULL },
	{ L"with_extinction",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory3,	L"衝突時消滅", NULL },
	{ L"reaction",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory3,	L"衝突反発係数", NULL },
	{ L"with_absorption",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrCategory3,	L"吸収", NULL },
	{ L"absorb_accel",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory3,	L"吸収加速度", NULL },
	{ L"absorb_ex_accel",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory3,
		L"領域外吸収加速比",
		L"放出領域外で追加的な引力を発生させる比率（距離に比例した加速：遠距離になるほど加速度が強い）" },
	{ L"absorb_radius",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory3,	L"吸収半径", NULL },
} ;

const S3DSceneComposer::ParamSetClass
	S3DParticleSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemBasicSerializer::m_pscClass,
	S3DParticleSerializer::paramParticleCount,
	&S3DParticleSerializer::m_paramEntries[0]
} ;

const wchar_t *	S3DParticleSerializer::m_pwszEmissionTypeIDs
							[S3DParticleSerializer::shapeCount] =
{
	L"sphere", L"disc"
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleSerializer::S3DParticleSerializer( void )
	: ItemBasicSerializer( m_ItemClassDescriptor.pwszClassID, &m_pscClass ),
		m_vParamAccel( 0, 1, 0 ), m_fpParamAccel( 9.8 ),
		m_vParamStream( 0, 0, 1 ), m_fpParamStream( 0.0 ),
		m_idNxtParticle( 1 )
{
	m_classItem = S3DScene::classPreRender ; 
	m_flagsBehavior |= S3DScene::itemTimer ;
	//
	m_randomizer.InitializeSeed() ;
	//
	m_emission.vDirection = S3DVector( 0, -1, 0 ) ;
	m_emission.vBaseSpeed = S3DVector( 0, 0, 0 ) ;
	m_emission.degAngle = 30.0f ;
	m_emission.countPerSec = 100.0f ;
	m_emission.fpSpeed = 10.0f ;
	m_emission.fpIndefinition = 0.5f ;
	m_emission.fpAreaScale = 1.0f ;
	m_emission.shapeType = shapeSphere ;
	//
	m_physics.nFlags = 0 ;
	m_physics.vAcceleration = S3DVector( 0, 9.8, 0 ) ;
	m_physics.vStream = S3DVector( 0, 0, 0 ) ;
	m_physics.fpAttenuation = 0.99f ;
	m_physics.fpReaction = 0.5f ;
	//
	m_particle.nFlags = 0 ;
	m_particle.secDuration = 1.0f ;
	m_particle.secFadeout = 0.5f ;
	m_particle.fpSizeScale = 1.0f ;
	m_particle.fpSizeIndefinition = 0.0f ;
	m_particle.fpSizeScaleEnd = 1.0f ;
	m_particle.fpAnimationSpeed = 1.0f ;
	m_particle.dpsRotationSpeed = 0.0f ;
}

// 出力ターゲット設定
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::AttachRenderTarget
	( S3DParticleSerializer::RenderTarget * pTarget, const wchar_t * pwszID )
{
	m_refRenderTarget.SetReference( pTarget ) ;
	m_strRenderTarget = pwszID ;
}

S3DParticleSerializer::RenderTarget *
		S3DParticleSerializer::GetRenderTarget( void ) const
{
	return	m_refRenderTarget.GetReference() ;
}

// 放出パラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::SetEmissionParameter
		( const S3DParticleSerializer::EmissionParam& param )
{
	m_emission = param ;
}

const S3DParticleSerializer::EmissionParam&
	S3DParticleSerializer::GetEmissionParameter( void ) const
{
	return	m_emission ;
}

// 物理パラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::SetPhysicParameter
		( const S3DParticleSerializer::PhysicsParam& param )
{
	m_physics = param ;
}

const S3DParticleSerializer::PhysicsParam&
		S3DParticleSerializer::GetPhysicsParameter( void ) const
{
	return	m_physics ;
}

// 粒子パラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::SetParticleParameter
		( const S3DParticleSerializer::ParticleParam& param )
{
	m_particle = param ;
}

const S3DParticleSerializer::ParticleParam&
		S3DParticleSerializer::GetParticleParameter( void ) const
{
	return	m_particle ;
}

// 粒子生成
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::GenerateParticles( double fpCount )
{
	if ( fpCount <= 0.0 )
	{
		return ;
	}
	S3DDMatrix	matdItem ;
	S3DDVector	vdItem ;
	CalcGlobalTransformation( matdItem, vdItem ) ;
	//
	S3DMatrix	matItem = matdItem ;
	S3DVector	vDirection = matItem * m_emission.vDirection ;
	S3DVector	vBaseSpeed = matItem * m_emission.vBaseSpeed ;
	S3DVector	vFaceDir = matItem * m_particle.vDefFaceDir ;
	//
	size_t	nCount = GenerateCount( fpCount ) ;
	GenerateParticlesByParam
		( nCount, vdItem, &vDirection, &vBaseSpeed, &vFaceDir ) ;
}

// 生成数計算
//////////////////////////////////////////////////////////////////////////////
size_t S3DParticleSerializer::GenerateCount( double fpCount )
{
	fpCount = fabs( fpCount ) ;
	size_t	nCount = (size_t) floor( fpCount ) ;
	double	fpDecimal = fpCount - nCount ;
	if ( m_randomizer.QuickRandomFloat( 1.0f ) < fpDecimal )
	{
		nCount ++ ;
	}
	return	nCount ;
}

// 粒子生成（大域座標・方向・速度指定）
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::GenerateParticlesByParam
	( size_t nCount, const S3DDVector& vEmittionPos,
		const S3DVector * pvDir, const S3DVector * pvSpeed,
		const S3DVector * pvFaceDir,
		const S3DColor * pColor,
		float32_t fpZoom, float32_t fpLifeSpeed,
		float32_t fpSpeedScale, float32_t fpEmissionSpeed )
{
	if ( nCount == 0 )
	{
		return ;
	}
	m_csLock.Lock() ;
	//
	size_t	iBase = m_aParticles.GetLength() ;
	m_aParticles.SetLength( iBase + nCount ) ;
	//
	S3DDMatrix	matdTarget( 1, 1, 1 ) ;
	S3DDVector	vdTarget( 0, 0, 0 ) ;
	//
	S3DMatrix	matItem = matdTarget ;
	S3DVector	vItem = matdTarget * vEmittionPos + vdTarget ;
	S3DVector	vDirection = m_emission.vDirection ;
	S3DVector	vBaseSpeed = m_emission.vBaseSpeed ;
	S3DVector	vFaceDir( 0, 0, 1 ) ;
	S3DColor	clrParticle( 0xFFFFFFFF, 0 ) ;
	if ( pvDir != NULL )
	{
		vDirection = *pvDir ;
	}
	if ( pvSpeed != NULL )
	{
		vBaseSpeed = *pvSpeed ;
	}
	if ( pvFaceDir != NULL )
	{
		vFaceDir = *pvFaceDir ;
	}
	if ( pColor != NULL )
	{
		clrParticle = *pColor ;
	}
	else
	{
		GetGlobalColorEffect( clrParticle ) ;
	}
	vDirection = matItem * vDirection ;
	vBaseSpeed = matItem * vBaseSpeed ;
	vFaceDir = matItem * vFaceDir ;
	//
	S3DMatrix	matDirection( 1, 1, 1 ) ;
	matDirection.RevolveForAngle( vDirection ) ;
	//
	const float32_t	deg2rad = (float32_t) (PI / 180.0) ;
	const double	radAngle = m_emission.degAngle * deg2rad ;
	const double	radMinAngle = m_emission.degMinAngle * deg2rad ;
	//
	Particle *	pParticles = m_aParticles.GetAt( iBase ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Particle&	p = pParticles[i] ;
		S3DVector	vPos, vSpeed ;
		if ( m_emission.shapeType == shapeSphere )
		{
			double	radX = m_randomizer.QuickRandomDouble( radAngle ) + radMinAngle ;
			double	radY = m_randomizer.QuickRandomDouble( PI ) ;
			double	y = sin( radX ) ;
			double	z = cos( radX ) ;
			vPos.x = (float32_t) (y * cos( radY )) ;
			vPos.y = (float32_t) (y * sin( radY )) ;
			vPos.z = (float32_t) z ;
			vSpeed = vPos ;
		}
		else // if ( m_emittion.shapeType == shapeDisc )
		{
			double	r = m_randomizer.QuickRandomDouble( 1.0 ) ;
			double	rad = m_randomizer.QuickRandomDouble( PI ) ;
			vPos.x = (float32_t) (r * cos( rad )) ;
			vPos.y = (float32_t) (r * sin( rad )) ;
			vPos.z = 0.0f ;
			//
			double	radX = m_randomizer.QuickRandomDouble( radAngle ) + radMinAngle ;
			double	radY = m_randomizer.QuickRandomDouble( PI ) ;
			double	y = sin( radX ) ;
			double	z = cos( radX ) ;
			vSpeed.x = (float32_t) (y * cos( radY )) ;
			vSpeed.y = (float32_t) (y * sin( radY )) ;
			vSpeed.z = (float32_t) z ;
		}
		vPos *= m_emission.fpAreaScale * fpZoom ;
		vSpeed *= (m_emission.fpSpeed * fpSpeedScale + fpEmissionSpeed)
						* (1.0f - m_randomizer.QuickRandomFloat
										( m_emission.fpIndefinition )) ;
		p.vPos = matDirection * vPos + vItem ;
		p.vSpeed = matDirection * vSpeed
				+ vBaseSpeed * (1.0f - m_randomizer.QuickRandomFloat
											( m_emission.fpIndefinition )) ;
		//
		p.vRotate.x = 0.0f ;
		p.vRotate.y = 0.0f ;
		p.vRotate.z = 0.0f ;
		p.vRotateSpeed.x = 0.0f ;
		p.vRotateSpeed.y = 0.0f ;
		p.vRotateSpeed.z = 0.0f ;
		p.vFaceDir = vFaceDir ;
		p.clrParticle = clrParticle ;
		p.fpZoom = m_particle.fpSizeScale
						* (1.0f - m_randomizer.QuickRandomFloat
										( m_particle.fpSizeIndefinition )) ;
		p.fpZoom *= fpZoom ;
		p.secLife = 0.0f ;
		p.secAnimation = 0.0f ;
		p.fpLifeSpeed = fpLifeSpeed ;
		p.nIdentity = m_idNxtParticle ++ ;
		p.nLastPosCount = 0 ;
		//
		if ( m_idNxtParticle == 0 )
		{
			m_idNxtParticle = 1 ;
		}
		if ( m_particle.nFlags & particleRotation3D )
		{
			p.vRotate.x = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotate.y = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotate.z = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotateSpeed.x =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
			p.vRotateSpeed.y =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
			p.vRotateSpeed.z =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
		}
		else if ( m_particle.nFlags & particleRotation )
		{
			p.vRotate.z = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotateSpeed.z =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
		}
	}
	m_csLock.Unlock() ;
}

void S3DParticleSerializer::GenerateParticlesWithParam
	( size_t nCount,
		const S3DDVector& vEmittionPos,
		const S3DParticleSerializer::EmissionParam& paramEmission,
		const S3DColor * pColor,
		float32_t fpZoom, float32_t fpLifeSpeed,
		float32_t fpSpeedScale, float32_t fpEmissionSpeed )
{
	if ( nCount == 0 )
	{
		return ;
	}
	m_csLock.Lock() ;
	//
	size_t	iBase = m_aParticles.GetLength() ;
	m_aParticles.SetLength( iBase + nCount ) ;
	//
	S3DDMatrix	matdTarget( 1, 1, 1 ) ;
	S3DDVector	vdTarget( 0, 0, 0 ) ;
	//
	S3DMatrix	matItem = matdTarget ;
	S3DVector	vItem = matdTarget * vEmittionPos + vdTarget ;
	S3DVector	vDirection = paramEmission.vDirection ;
	S3DVector	vBaseSpeed = paramEmission.vBaseSpeed ;
	S3DVector	vFaceDir( 0, 0, 1 ) ;
	S3DColor	clrParticle( 0xFFFFFFFF, 0 ) ;
	//
	vDirection = matItem * vDirection ;
	vBaseSpeed = matItem * vBaseSpeed ;
	vFaceDir = matItem * vFaceDir ;
	//
	if ( pColor != NULL )
	{
		clrParticle = *pColor ;
	}
	else
	{
		GetGlobalColorEffect( clrParticle ) ;
	}
	//
	S3DMatrix	matDirection( 1, 1, 1 ) ;
	matDirection.RevolveForAngle( vDirection ) ;
	//
	const float32_t	deg2rad = (float32_t) (PI / 180.0) ;
	const double	radAngle = paramEmission.degAngle * deg2rad ;
	const double	radMinAngle = paramEmission.degMinAngle * deg2rad ;
	//
	Particle *	pParticles = m_aParticles.GetAt( iBase ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Particle&	p = pParticles[i] ;
		S3DVector	vPos, vSpeed ;
		if ( paramEmission.shapeType == shapeSphere )
		{
			double	radX = m_randomizer.QuickRandomDouble( radAngle ) + radMinAngle ;
			double	radY = m_randomizer.QuickRandomDouble( PI ) ;
			double	y = sin( radX ) ;
			double	z = cos( radX ) ;
			vPos.x = (float32_t) (y * cos( radY )) ;
			vPos.y = (float32_t) (y * sin( radY )) ;
			vPos.z = (float32_t) z ;
			vSpeed = vPos ;
		}
		else // if ( paramEmission.shapeType == shapeDisc )
		{
			double	r = m_randomizer.QuickRandomDouble( 1.0 ) ;
			double	rad = m_randomizer.QuickRandomDouble( PI ) ;
			vPos.x = (float32_t) (r * cos( rad )) ;
			vPos.y = (float32_t) (r * sin( rad )) ;
			vPos.z = 0.0f ;
			//
			double	radX = m_randomizer.QuickRandomDouble( radAngle ) + radMinAngle ;
			double	radY = m_randomizer.QuickRandomDouble( PI ) ;
			double	y = sin( radX ) ;
			double	z = cos( radX ) ;
			vSpeed.x = (float32_t) (y * cos( radY )) ;
			vSpeed.y = (float32_t) (y * sin( radY )) ;
			vSpeed.z = (float32_t) z ;
		}
		vPos *= paramEmission.fpAreaScale * fpZoom ;
		vSpeed *= (paramEmission.fpSpeed * fpSpeedScale + fpEmissionSpeed)
						* (1.0f - m_randomizer.QuickRandomFloat
										( paramEmission.fpIndefinition )) ;
		p.vPos = matDirection * vPos + vItem ;
		p.vSpeed = matDirection * vSpeed
					+ vBaseSpeed * (1.0f - m_randomizer.QuickRandomFloat
											( m_emission.fpIndefinition )) ;
		//
		p.vRotate.x = 0.0f ;
		p.vRotate.y = 0.0f ;
		p.vRotate.z = 0.0f ;
		p.vRotateSpeed.x = 0.0f ;
		p.vRotateSpeed.y = 0.0f ;
		p.vRotateSpeed.z = 0.0f ;
		p.vFaceDir = vFaceDir ;
		p.clrParticle = clrParticle ;
		p.fpZoom = m_particle.fpSizeScale
						* (1.0f - m_randomizer.QuickRandomFloat
										( m_particle.fpSizeIndefinition )) ;
		p.fpZoom *= fpZoom ;
		p.secLife = 0.0f ;
		p.secAnimation = 0.0f ;
		p.fpLifeSpeed = fpLifeSpeed ;
		p.nIdentity = m_idNxtParticle ++ ;
		p.nLastPosCount = 0 ;
		//
		if ( m_idNxtParticle == 0 )
		{
			m_idNxtParticle = 1 ;
		}
		if ( m_particle.nFlags & particleRotation3D )
		{
			p.vRotate.x = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotate.y = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotate.z = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotateSpeed.x =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
			p.vRotateSpeed.y =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
			p.vRotateSpeed.z =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
		}
		else if ( m_particle.nFlags & particleRotation )
		{
			p.vRotate.z = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotateSpeed.z =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
		}
	}
	m_csLock.Unlock() ;
}

void S3DParticleSerializer::GenerateParticlesByParams
	( size_t nCount,
		const S3DDVector * pvEmittionPoss,
		const S3DVector * pvDirs,
		const S3DVector * pvSpeeds,
		const S3DVector * pvFaceDirs,
		const S3DColor * pColors,
		const float32_t * pfpZooms,
		const float32_t * pfpLifeSpeeds )
{
	if ( nCount == 0 )
	{
		return ;
	}
	m_csLock.Lock() ;
	//
	size_t	iBase = m_aParticles.GetLength() ;
	m_aParticles.SetLength( iBase + nCount ) ;
	//
	S3DDMatrix	matdTarget( 1, 1, 1 ) ;
	S3DDVector	vdTarget( 0, 0, 0 ) ;
	//
	S3DMatrix	matItem = matdTarget ;
	S3DVector	vDirection = m_emission.vDirection ;
	S3DVector	vBaseSpeed = m_emission.vBaseSpeed ;
	S3DVector	vFaceDir( 0, 0, 1 ) ;
	S3DColor	clrParticle( 0xFFFFFFFF, 0 ) ;
	vDirection = matItem * vDirection ;
	vBaseSpeed = matItem * vBaseSpeed ;
	vFaceDir = matItem * vFaceDir ;
	//
	if ( pColors == nullptr )
	{
		GetGlobalColorEffect( clrParticle ) ;
	}
	//
	S3DMatrix	matI( 1, 1, 1 ) ;
	S3DMatrix	matDirection( 1, 1, 1 ) ;
	matDirection.RevolveForAngle( vDirection ) ;
	//
	const float32_t	deg2rad = (float32_t) (PI / 180.0) ;
	const double	radAngle = m_emission.degAngle * deg2rad ;
	const double	radMinAngle = m_emission.degMinAngle * deg2rad ;
	//
	Particle *	pParticles = m_aParticles.GetAt( iBase ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Particle&	p = pParticles[i] ;
		S3DVector	vItem = matdTarget * pvEmittionPoss[i] + vdTarget ;
		if ( pvDirs != NULL )
		{
			vDirection = pvDirs[i] ;
			matDirection = matI ;
			matDirection.RevolveForAngle( vDirection ) ;
		}
		if ( pvSpeeds != NULL )
		{
			vBaseSpeed = pvSpeeds[i] ;
		}
		if ( pvFaceDirs != NULL )
		{
			vFaceDir = pvFaceDirs[i] ;
		}
		if ( pColors != NULL )
		{
			clrParticle = pColors[i] ;
		}
		p.vPos = vItem ;
		p.vSpeed = vBaseSpeed ;
		//
		p.vRotate.x = 0.0f ;
		p.vRotate.y = 0.0f ;
		p.vRotate.z = 0.0f ;
		p.vRotateSpeed.x = 0.0f ;
		p.vRotateSpeed.y = 0.0f ;
		p.vRotateSpeed.z = 0.0f ;
		p.vFaceDir = vFaceDir ;
		p.clrParticle = clrParticle ;
		p.fpZoom = m_particle.fpSizeScale
						* (1.0f - m_randomizer.QuickRandomFloat
										( m_particle.fpSizeIndefinition )) ;
		if ( pfpZooms != NULL )
		{
			p.fpZoom *= pfpZooms[i] ;
		}
		p.secLife = 0.0f ;
		p.secAnimation = 0.0f ;
		if ( pfpLifeSpeeds != NULL )
		{
			p.fpLifeSpeed = pfpLifeSpeeds[i] ;
		}
		else
		{
			p.fpLifeSpeed = 1.0f ;
		}
		p.nIdentity = m_idNxtParticle ++ ;
		p.nLastPosCount = 0 ;
		//
		if ( m_idNxtParticle == 0 )
		{
			m_idNxtParticle = 1 ;
		}
		if ( m_particle.nFlags & particleRotation3D )
		{
			p.vRotate.x = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotate.y = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotate.z = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotateSpeed.x =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
			p.vRotateSpeed.y =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
			p.vRotateSpeed.z =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
		}
		else if ( m_particle.nFlags & particleRotation )
		{
			p.vRotate.z = (float32_t) m_randomizer.QuickRandomDouble( PI ) ;
			p.vRotateSpeed.z =
				(float32_t) m_randomizer.QuickRandomDouble
								( m_particle.dpsRotationSpeed ) * deg2rad ;
		}
	}
	m_csLock.Unlock() ;
}

// 粒子生成（発生形状メッシュ指定）
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::GenerateParticlesOnMesh
	( S3DVertexBufferInterface * pVBO,
		const S3DDMatrix& matVBO,
		const S3DDVector& vGlobalPos,
		size_t nCount, uint32_t nFlags,
		const S3DColor& clrMul, const S3DColor& clrAdd,
		float32_t fpZoomRate, float32_t fpLifeSpeed,
		float32_t fpSpeedScale, float32_t fpEmissionSpeed,
		const size_t * pMeshIndexes, size_t nMeshCount )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	// メッシュ頂点数取得
	//
	const size_t	nVBMeshTotalCount = pVBO->GetMeshCount() ;
	size_t *		pMeshVertexCount = NULL ;
	size_t			nTotalVertexCount = 0 ;
	if ( (pMeshIndexes != NULL) && (nMeshCount > 0) )
	{
		pMeshVertexCount = m_bufTempVerCount.GetArray( nMeshCount ) ;
		//
		for ( size_t i = 0; i < nMeshCount; i ++ )
		{
			S3DVertexBufferInterface::MeshInfo	info ;
			if ( !pVBO->GetMeshInfoAt( info, pMeshIndexes[i], 0 ) )
			{
				nTotalVertexCount += info.countVertex ;
			}
			else
			{
				info.countVertex = 0 ;
			}
			pMeshVertexCount[i] = info.countVertex ;
		}
		m_bufTempVerCount.FinishArray() ;
	}
	else
	{
		size_t *	pMeshRefIndex ;
		pMeshVertexCount = m_bufTempVerCount.GetArray( nVBMeshTotalCount ) ;
		pMeshRefIndex = m_bufTempMeshIndex.GetArray( nVBMeshTotalCount ) ;
		pMeshIndexes = pMeshRefIndex ;
		nMeshCount = nVBMeshTotalCount ;
		//
		for ( size_t i = 0; i < nVBMeshTotalCount; i ++ )
		{
			S3DVertexBufferInterface::MeshInfo	info ;
			pMeshRefIndex[i] = i ;
			if ( !pVBO->GetMeshInfoAt( info, i, 0 ) )
			{
				nTotalVertexCount += info.countVertex ;
			}
			else
			{
				info.countVertex = 0 ;
			}
			pMeshVertexCount[i] = info.countVertex ;
		}
		m_bufTempVerCount.FinishArray() ;
		m_bufTempMeshIndex.FinishArray() ;
	}
	if ( nTotalVertexCount == 0 )
	{
		return ;
	}
	//
	// 生成パラメータ計算
	//
	S3DDVector *	pvEmittionPos = m_bufTempPos.GetArray( nCount ) ;
	S3DVector *		pvGenDirs = m_bufTempDir.GetArray( nCount ) ;
	S3DVector *		pvGenSpeeds = m_bufTempSpeed.GetArray( nCount ) ;
	S3DColor *		pGenColors = (nFlags & flagWithoutColor) ?
									NULL : m_bufColors.GetArray( nCount ) ;
	float32_t *		pfpZooms = m_bufZooms.GetArray( nCount ) ;
	float32_t *		pfpLifeSpeeds = m_bufTempLifeSpeed.GetArray( nCount ) ;
	size_t			nNextMeshCount = 0 ;
	size_t			nNextAccVertex = 0 ;
	ssize_t			iNextMesh = -1 ;
	//
	S3DMaterial *		pMaterial = NULL ;
	SGLImageObject *	pTexture = NULL ;
	SGLImageInfo		infTexture ;
	const uint8_t *		pbytTexture = NULL ;
	SGLSize				sizeTexture( 0, 0 ) ;
	SGLSize				sizeTextureMask( 1, 1 ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		while ( (i >= nNextMeshCount)
				&& ((size_t) iNextMesh + 1 < nMeshCount) )
		{
			nNextAccVertex += pMeshVertexCount[++ iNextMesh] ;
			nNextMeshCount = nCount * nNextAccVertex / nTotalVertexCount ;
		}
		//
		// 頂点情報をサンプリング
		//
		size_t	iVertex =
			(size_t) m_randomizer.QuickRandomize
						( (uint32_t) pMeshVertexCount[iNextMesh] ) ;
		//
		S3DVertexBufferInterface::MeshInfo	info ;
		eslFillMemory
			( &info, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
		//
		S3DVector4	vVertex, vNormal ;
		S2DVector	vUVMap ;
		S3DColor	vColor ;
		info.pvVertex = &vVertex ;
		info.pvNormal = &vNormal ;
		info.pvUVMap = &vUVMap ;
		info.pColor = &vColor ;
		//
		if ( pVBO->GetMeshInfoAt
			( info, pMeshIndexes[iNextMesh], 1, iVertex,
				S3DVertexBufferInterface::flagMeshTransformed ) )
		{
			continue ;
		}
		//
		// 座標・向き・速度決定
		//
		pvEmittionPos[i] = matVBO * S3DDVector(vVertex) + vGlobalPos ;
		pvGenDirs[i] = matVBO * S3DDVector(vNormal) ;
		pvGenSpeeds[i] = pvGenDirs[i] ;
		pvGenSpeeds[i] *=
			(m_emission.fpSpeed * fpSpeedScale + fpEmissionSpeed)
						* (1.0f - m_randomizer.QuickRandomFloat
										( m_emission.fpIndefinition )) ;
		pfpZooms[i] = fpZoomRate ;
		pfpLifeSpeeds[i] = fpLifeSpeed ;
		//
		// 色情報取得
		//
		if ( pGenColors == NULL )
		{
			continue ;
		}
		uint64_t	flagShading =
			(info.pMaterial != NULL)
				? info.pMaterial->m_attrSurface.flagsShading : 0 ;
		uint32_t	nAlpha = 0xFF ;
		if ( flagShading & shadingVertexAlpha )
		{
			nAlpha = vColor.rgbMul.argb.Alpha ;
		}
		else
		{
			vColor.rgbMul.argb.Alpha = 0xFF ;
		}
		if ( pMaterial != info.pMaterial )
		{
			//
			// マテリアル変更→テクスチャ取得
			//
			if ( pbytTexture != NULL )
			{
				pTexture->UnlockBuffer( SGLImageObject::lockRead ) ;
			}
			pMaterial = info.pMaterial ;
			pTexture = NULL ;
			pbytTexture = NULL ;
			//
			if ( (flagShading & shadingTextureMapping)
					&& (nFlags & flagWithColorTexture)
					&& (pMaterial != nullptr) )
			{
				pTexture = pMaterial->GetTexture
						( pMaterial->FindTextureTypeOf
								( S3DMaterial::textureDiffusion ) ) ;
				if ( pTexture != NULL )
				{
					pbytTexture =
						pTexture->LockBuffer
							( infTexture, SGLImageObject::lockRead ) ;
					sizeTexture.w = (int32_t) infTexture.width ;
					sizeTexture.h = (int32_t) infTexture.height ;
					//
					if ( flagShading & shadingTextureTiling )
					{
						sizeTextureMask.w =
							(int32_t) sglNormalizeScalePowerBy2
										( (uint32_t) sizeTexture.w ) - 1 ;
						sizeTextureMask.h =
							(int32_t) sglNormalizeScalePowerBy2
										( (uint32_t) sizeTexture.h ) - 1 ;
						//
						if ( sizeTextureMask.w + 1 > sizeTexture.w )
						{
							sizeTextureMask.w >>= 1 ;
						}
						if ( sizeTextureMask.h + 1 > sizeTexture.h )
						{
							sizeTextureMask.h >>= 1 ;
						}
					}
				}
			}
		}
		if ( (pTexture != NULL) && (infTexture.depth == 32) )
		{
			//
			// テクスチャサンプリング
			//
			int	xTexture, yTexture ;
			if ( !(flagShading & shadingNormalizedUVScale) )
			{
				xTexture = esl_roundfi( vUVMap.x ) ;
				yTexture = esl_roundfi( vUVMap.y ) ;
			}
			else
			{
				xTexture = esl_roundfi( vUVMap.x * (float32_t) sizeTexture.w ) ;
				yTexture = esl_roundfi( vUVMap.y * (float32_t) sizeTexture.h ) ;
			}
			if ( flagShading & shadingTextureTiling )
			{
				xTexture &= sizeTextureMask.w ;
				yTexture &= sizeTextureMask.h ;
			}
			else
			{
				xTexture = esl_clampi( xTexture, 0, sizeTexture.w - 1 ) ;
				yTexture = esl_clampi( yTexture, 0, sizeTexture.h - 1 ) ;
			}
			SGLPalette	rgba =
				*((SGLPalette*)(pbytTexture
							+ (xTexture * 4
								+ yTexture * infTexture.pitchLine))) ;
			uint32_t	nTexAlpha = rgba.argb.Alpha ;
			rgba = vColor * rgba ;
			//
			SGLPalette	rgbMul = clrMul * rgba ;
			SGLPalette	rgbAdd = clrAdd * rgba ;
			//
			if ( infTexture.format & formatImageFlagAlpha )
			{
				nAlpha = (nTexAlpha * (nAlpha + 1)) >> 8 ;
			}
			rgbMul.argb.Alpha = (uint8_t) esl_clampi( (int) nAlpha, 0, 0xFF ) ;
			//
			vColor.rgbMul = rgbMul ;
			vColor.rgbAdd = rgbAdd ;
		}
		else if ( pMaterial != nullptr )
		{
			vColor = clrMul * pMaterial->m_attrSurface.colorBase + clrAdd ;
			vColor.rgbMul.argb.Alpha = (uint8_t) esl_clampi( (int) nAlpha, 0, 0xFF ) ;
		}
		pGenColors[i] = vColor ;
	}
	if ( pbytTexture != NULL )
	{
		ESLAssert( pTexture != NULL ) ;
		pTexture->UnlockBuffer( SGLImageObject::lockRead ) ;
	}
	//
	// パーティクル生成
	//
	GenerateParticlesByParams
		( nCount, pvEmittionPos, pvGenDirs, pvGenSpeeds,
					NULL, pGenColors, pfpZooms, pfpLifeSpeeds ) ;
	//
	m_bufTempPos.FinishArray() ;
	m_bufTempDir.FinishArray() ;
	m_bufTempSpeed.FinishArray() ;
	m_bufColors.FinishArray() ;
	m_bufZooms.FinishArray() ;
	m_bufTempLifeSpeed.FinishArray() ;
}

// 粒子消去
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::ClearAllParticles( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_aParticles.RemoveAll() ;
}

void S3DParticleSerializer::FadeoutAllParticles( void )
{
	if ( !(m_particle.nFlags & particleDuration)
				|| (m_particle.secFadeout <= 0.0) )
	{
		ClearAllParticles() ;
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;

	Particle *		pParticles = m_aParticles.GetArray() ;
	const size_t	nCount = m_aParticles.GetLength() ;
	const float32_t	secFadeout = esl_fmaxf( m_particle.secDuration
											- m_particle.secFadeout, 0.0f ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pParticles[i].secLife < secFadeout )
		{
			pParticles[i].secLife = secFadeout ;
		}
	}
	m_aParticles.FinishArray() ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DParticleSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramFaceDir:
		return	S3DDVector( m_particle.vDefFaceDir ) ;

	case	paramEmissionDirection:
		return	S3DDVector( m_emission.vDirection ) ;

	case	paramEmissionBaseSpeed:
		return	S3DDVector( m_emission.vBaseSpeed ) ;

	case	paramAccelerationDirection:
		return	m_vParamAccel ;

	case	paramStreamDirection:
		return	m_vParamStream ;
	}
	return	ItemBasicSerializer::GetVectorParameter( i ) ;
}

double S3DParticleSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDuration:
		return	m_particle.secDuration ;

	case	paramFadein:
		return	m_particle.secFadein ;

	case	paramFadeout:
		return	m_particle.secFadeout ;

	case	paramParticleZoom:
		return	m_particle.fpSizeScale ;

	case	paramParticleZoomIndefinition:
		return	m_particle.fpSizeIndefinition ;

	case	paramParticleZoomEnd:
		return	m_particle.fpSizeScaleEnd ;

	case	paramAnimationSpeed:
		return	m_particle.fpAnimationSpeed ;

	case	paramRotationSpeed:
		return	m_particle.dpsRotationSpeed ;

	case	paramBlurSupplement:
		return	m_particle.fpBlurSupplement ;

	case	paramBaseAspectSpeed:
		return	m_particle.fpAspectSpeed ;

	case	paramEmissionMinAngle:
		return	m_emission.degMinAngle ;

	case	paramEmissionAngle:
		return	m_emission.degAngle ;

	case	paramEmissionCount:
		return	m_emission.countPerSec ;

	case	paramEmissionSpeed:
		return	m_emission.fpSpeed ;

	case	paramEmissionSpeedIndefinition:
		return	m_emission.fpIndefinition ;

	case	paramEmissionScale:
		return	m_emission.fpAreaScale ;

	case	paramAccelerationVelocity:
		return	m_fpParamAccel ;

	case	paramStreamVelocity:
		return	m_fpParamStream ;

	case	paramAttenuation:
		return	m_physics.fpAttenuation ;

	case	paramReaction:
		return	m_physics.fpReaction ;

	case	paramAbsorbAccel:
		return	m_physics.fpAbsorbAccel ;

	case	paramAbsorbExAccel:
		return	m_physics.fpAbsorbExAccel ;

	case	paramAbsorbRadius:
		return	m_physics.fpAbsorbRadius ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

int32_t S3DParticleSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBlurFrame:
		return	m_particle.nBlurFrames ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

bool S3DParticleSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramUseDuration:
		return	((m_particle.nFlags & particleDuration) != 0) ;

	case	paramUseFaceDir:
		return	((m_particle.nFlags & particleFaceDir) != 0) ;

	case	paramLocalSpace:
		return	((m_particle.nFlags & particleLocalSpace) != 0) ;

	case	paramMoveAspect:
		return	((m_particle.nFlags & particleMoveAspect) != 0) ;

	case	paramWithCollision:
		return	((m_physics.nFlags & physicsCollision) != 0) ;

	case	paramWithExtinction:
		return	((m_physics.nFlags & physicsExtinction) != 0) ;

	case	paramWithAbsorption:
		return	((m_physics.nFlags & physicsAbsorption) != 0) ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DParticleSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRenderItem:
		return	m_strRenderTarget ;

	case	paramRotationType:
		if ( m_particle.nFlags & particleRotation3D )
		{
			return	L"rotation_3d" ;
		}
		else if ( m_particle.nFlags & particleRotation )
		{
			return	L"rotation_2d" ;
		}
		return	L"no_rotation" ;

	case	paramEmissionShape:
		return	m_pwszEmissionTypeIDs[m_emission.shapeType] ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramFaceDir:
		m_particle.vDefFaceDir = vec ;
		return ;

	case	paramEmissionDirection:
		m_emission.vDirection = vec ;
		return ;

	case	paramEmissionBaseSpeed:
		m_emission.vBaseSpeed = vec ;
		return ;

	case	paramAccelerationDirection:
		m_vParamAccel = vec ;
		m_physics.vAcceleration = m_vParamAccel * m_fpParamAccel ;
		return ;

	case	paramStreamDirection:
		m_vParamStream= vec ;
		m_physics.vStream = m_vParamStream * m_fpParamStream ;
		return ;
	}
	ItemBasicSerializer::SetVectorParameter( i, vec ) ;
}

void S3DParticleSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramDuration:
		m_particle.secDuration = (float32_t) s ;
		return ;

	case	paramFadein:
		m_particle.secFadein = (float32_t) s ;
		return ;

	case	paramFadeout:
		m_particle.secFadeout = (float32_t) s ;
		return ;

	case	paramParticleZoom:
		m_particle.fpSizeScale = (float32_t) s ;
		return ;

	case	paramParticleZoomIndefinition:
		m_particle.fpSizeIndefinition = (float32_t) s ;
		return ;

	case	paramParticleZoomEnd:
		m_particle.fpSizeScaleEnd = (float32_t) s ;
		return ;

	case	paramAnimationSpeed:
		m_particle.fpAnimationSpeed = (float32_t) s ;
		return ;

	case	paramRotationSpeed:
		m_particle.dpsRotationSpeed = (float32_t) s ;
		return ;

	case	paramBlurSupplement:
		m_particle.fpBlurSupplement = (float32_t) s ;
		return ;

	case	paramBaseAspectSpeed:
		m_particle.fpAspectSpeed = (float32_t) s ;
		return ;

	case	paramEmissionMinAngle:
		m_emission.degMinAngle = (float32_t) s ;
		return ;

	case	paramEmissionAngle:
		m_emission.degAngle = (float32_t) s ;
		return ;

	case	paramEmissionCount:
		m_emission.countPerSec = (float32_t) s ;
		return ;

	case	paramEmissionSpeed:
		m_emission.fpSpeed = (float32_t) s ;
		return ;

	case	paramEmissionSpeedIndefinition:
		m_emission.fpIndefinition = (float32_t) s ;
		return ;

	case	paramEmissionScale:
		m_emission.fpAreaScale = (float32_t) s ;
		return ;

	case	paramAccelerationVelocity:
		m_fpParamAccel = s ;
		m_physics.vAcceleration = m_vParamAccel * m_fpParamAccel ;
		return ;

	case	paramStreamVelocity:
		m_fpParamStream = s ;
		m_physics.vStream = m_vParamStream * m_fpParamStream ;
		return ;

	case	paramAttenuation:
		m_physics.fpAttenuation = (float32_t) s ;
		return ;

	case	paramReaction:
		m_physics.fpReaction = (float32_t) s ;
		return ;

	case	paramAbsorbAccel:
		m_physics.fpAbsorbAccel = (float32_t) s ;
		return ;

	case	paramAbsorbExAccel:
		m_physics.fpAbsorbExAccel = (float32_t) s ;
		return ;

	case	paramAbsorbRadius:
		m_physics.fpAbsorbRadius = (float32_t) s ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DParticleSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramBlurFrame:
		m_particle.nBlurFrames = (uint32_t) esl_max( n, 0 ) ;
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DParticleSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramUseDuration:
		m_particle.nFlags &= ~particleDuration ;
		m_particle.nFlags |= b ? particleDuration : 0 ;
		return ;

	case	paramUseFaceDir:
		m_particle.nFlags &= ~particleFaceDir ;
		m_particle.nFlags |= b ? particleFaceDir : 0 ;
		return ;

	case	paramLocalSpace:
		m_particle.nFlags &= ~particleLocalSpace ;
		m_particle.nFlags |= b ? particleLocalSpace : 0 ;
		return ;

	case	paramMoveAspect:
		m_particle.nFlags &= ~particleMoveAspect ;
		m_particle.nFlags |= b ? particleMoveAspect : 0 ;
		return ;

	case	paramWithCollision:
		m_physics.nFlags &= ~physicsCollision ;
		m_physics.nFlags |= b ? physicsCollision : 0 ;
		return ;

	case	paramWithExtinction:
		m_physics.nFlags &= ~physicsExtinction ;
		m_physics.nFlags |= b ? physicsExtinction : 0 ;
		return ;

	case	paramWithAbsorption:
		m_physics.nFlags &= ~physicsAbsorption ;
		m_physics.nFlags |= b ? physicsAbsorption : 0 ;
		return ;
	}
	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

void S3DParticleSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramRenderItem:
		if ( m_strRenderTarget != pwszCmd )
		{
			AttachRenderTarget
				( ESLTypeCast<RenderTarget>
					( GetSceneItemAs( pwszCmd ) ), pwszCmd ) ;
		}
		return ;

	case	paramRotationType:
		m_particle.nFlags &= ~(particleRotation | particleRotation3D) ;
		if ( SString::Compare( pwszCmd, L"rotation_3d" ) == 0 )
		{
			m_particle.nFlags |= particleRotation3D ;
		}
		else if ( SString::Compare( pwszCmd, L"rotation_2d" ) == 0 )
		{
			m_particle.nFlags |= particleRotation ;
		}
		return ;

	case	paramEmissionShape:
		for ( int i = 0; i < shapeCount; i ++ )
		{
			if ( SString::Compare( pwszCmd, m_pwszEmissionTypeIDs[i] ) == 0 )
			{
				m_emission.shapeType = (EmissionShape) i ;
				break ;
			}
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DParticleSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramRenderItem:
		pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(RenderTarget) ) ;
		}
		return	true ;

	case	paramRotationType:
		aStrSet.Add( new SString( L"no_rotation" ) ) ;
		aStrSet.Add( new SString( L"rotation_2d" ) ) ;
		aStrSet.Add( new SString( L"rotation_3d" ) ) ;
		return	true ;

	case	paramEmissionShape:
		for ( int i = 0; i < shapeCount; i ++ )
		{
			aStrSet.Add( new SString( m_pwszEmissionTypeIDs[i] ) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DParticleSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"粒子設定" ;
	case	2:
		return	L"放出設定" ;
	case	3:
		return	L"物理設定" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DParticleSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
	case	paramZoom:
//	case	paramColorMul:
//	case	paramColorAdd:
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

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DParticleSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( (nFlags & S3DSceneComposer::updateRefItem)
		&& !m_strRenderTarget.IsEmpty() )
	{
		RenderTarget *	pTarget =
			ESLTypeCast<RenderTarget>
				( GetSceneItemAs( m_strRenderTarget ) ) ;
		AttachRenderTarget( pTarget, m_strRenderTarget ) ;
		//
		if ( pTarget == NULL )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	ItemBasicSerializer::OnItemRenderEvent( scene, clsItem ) ;
	//
	if ( clsItem == S3DScene::classPreRender )
	{
		AddParticleToTarget( scene ) ;
	}
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	m_csLock.Lock() ;
	//
	Particle *		pParticles = m_aParticles.GetArray() ;
	const size_t	nCount = m_aParticles.GetLength() ;
	size_t			iDst = 0 ;
	float32_t		secDuration = 0.0 ;
	float32_t		secFadeout = 0.0 ;
	float32_t		secPast = (float32_t) msecPast / 1000.0f ;
	S3DVector		vAcceleration = m_physics.vAcceleration * secPast ;
	float32_t		fpAttenuation =
						(float32_t) pow
							( (double) (1.0 - m_physics.fpAttenuation),
														(double) secPast ) ;
	float32_t		secAnimSpeed = 1.0f ;
	RenderTarget *	pRenderTarget = m_refRenderTarget.GetReference() ;
	if ( pRenderTarget != NULL )
	{
		double	secLength = 0.0 ;
		if ( pRenderTarget->GetTargetAnimationLength( secLength ) )
		{
			secDuration = (float32_t) secLength ;
			secAnimSpeed = m_particle.fpAnimationSpeed ;
			secDuration /= secAnimSpeed ;
		}
	}
	if ( m_particle.nFlags & particleDuration )
	{
		secDuration = m_particle.secDuration ;
		secFadeout = m_particle.secFadeout ;
	}
	S3DVector	vParticleSpace( 0, 0, 0 ) ;
	if ( !(m_particle.nFlags & particleLocalSpace)
				&& (m_physics.nFlags & physicsAbsorption) )
	{
		S3DDMatrix	matdParticleSpace ;
		S3DDVector	vdParticleSpace ;
		GetGlobalTransformation( matdParticleSpace, vdParticleSpace ) ;
		vParticleSpace = vdParticleSpace ;
	}
	//
	S3DCollisionResult	rsHit ;
	S3DDVector			vPos0, vPos1 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Particle&	pSrc = pParticles[i] ;
		float32_t	secLifePast = secPast * pSrc.fpLifeSpeed ;
		pSrc.secLife += secLifePast ;
		pSrc.secAnimation += secLifePast * secAnimSpeed ;
		if ( pSrc.secLife >= secDuration )
		{
			continue ;
		}
		Particle&	pDst = pParticles[iDst ++] ;
		pDst = pSrc ;
		pDst.vLastPos[3] = pSrc.vLastPos[2] ;
		pDst.vLastPos[2] = pSrc.vLastPos[1] ;
		pDst.vLastPos[1] = pSrc.vLastPos[0] ;
		pDst.vLastPos[0] = pSrc.vPos ;
		pDst.nLastPosCount = esl_min( pSrc.nLastPosCount + 1, 4 ) ;
		//
		S3DVector	vSpeed = pDst.vSpeed ;
		vSpeed += vAcceleration ;
		vSpeed -= m_physics.vStream ;
		if ( m_physics.nFlags & physicsAbsorption )
		{
			S3DVector	vAbsorbDir = (vParticleSpace - pSrc.vPos).Normalized() ;
			vSpeed += vAbsorbDir * (m_physics.fpAbsorbAccel * secPast) ;
		}
		vSpeed *= fpAttenuation ;
		vSpeed += m_physics.vStream ;
		//
		pDst.vPos += vSpeed * secPast ;
		pDst.vSpeed = vSpeed ;
		pDst.vRotateSpeed *= fpAttenuation ;
		pDst.vRotate += pDst.vRotateSpeed ;
		//
		if ( m_physics.nFlags & physicsAbsorption )
		{
			S3DVector	vAbsorbDelta = vParticleSpace - pDst.vPos ;
			double		rAbsorbDelta = vAbsorbDelta.Absolute() ;
			if ( rAbsorbDelta <= m_physics.fpAbsorbRadius )
			{
				iDst -- ;
				continue ;
			}
			S3DVector	vMoveDelta = pDst.vLastPos[0] - pDst.vPos ;
			float32_t	d = (float32_t) vMoveDelta.Absolute() ;
			if ( d > 1.0e-5 )
			{
				float32_t	t = vMoveDelta.InnerProduct( vAbsorbDelta ) / d ;
				if ( (t >= 0.0) && (t < d) )
				{
					S3DVector	vt = vMoveDelta * (t / d) + pDst.vPos ;
					if ( (vParticleSpace - vt).Absolute() < m_physics.fpAbsorbRadius )
					{
						iDst -- ;
						continue ;
					}
				}
			}
			if ( rAbsorbDelta > m_emission.fpAreaScale )
			{
				pDst.vSpeed += vAbsorbDelta * m_physics.fpAbsorbExAccel ;
			}
		}
		if ( m_physics.nFlags & physicsCollision )
		{
			vPos0 = pDst.vLastPos[0] ;
			vPos1 = pDst.vPos ;
			rsHit.fpDistance = (float32_t) (vPos1 - vPos0).Absolute() ;
			if ( scene.IsSegmentCrossing( vPos0, vPos1, 0.0001f, rsHit ) )
			{
				if ( m_physics.nFlags & physicsExtinction )
				{
					iDst -- ;
					continue ;
				}
				rsHit.vNormal.Normalize() ;
				vSpeed = vSpeed - rsHit.vNormal
							* (vSpeed.InnerProduct( rsHit.vNormal )
									* (1.0f + m_physics.fpReaction)) ;
				pDst.vPos = rsHit.vHitGlobal ;
				pDst.vSpeed = vSpeed ;
			}
		}
	}
	m_aParticles.FinishArray() ;
	m_aParticles.SetLength( iDst ) ;
	//
	GenerateParticles( secPast * m_emission.countPerSec ) ;
	//
	m_csLock.Unlock() ;
	//
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;
}

// パーティクルをターゲットに出力
//////////////////////////////////////////////////////////////////////////////
void S3DParticleSerializer::AddParticleToTarget( const S3DScene& scene )
{
	RenderTarget *	pRenderTarget = m_refRenderTarget.GetReference() ;
	if ( (pRenderTarget != nullptr) && (m_aParticles.GetLength() > 0) )
	{
		SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
		//
		const bool		flagIndexedParticle = pRenderTarget->IsUsingIndexedParticles() ;
		const double	framesPerSec = 60.0 ;
		/*
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			const S3DSceneComposer::CompositionInfo *	pci = pComp->GetCompositionInfo() ;
			if ( pci != NULL )
			{
				framesPerSec = pci->FrameIndexFromSecond( 1.0 ) ;
			}
		}
		*/
		//
		// バッファ確保
		//
		size_t	nPointSupply = 1 ;
		size_t	nMaxBlurSupply = 1 ;
		size_t	nBlurFrames =
					(size_t) esl_min( (int) m_particle.nBlurFrames, 4 ) ;
		size_t	nMaxFrameSupply = 0 ;
		if ( flagIndexedParticle )
		{
			nPointSupply = 5 ;
		}
		else if ( nBlurFrames > 0 )
		{
			nMaxBlurSupply = 32 ;
			nPointSupply = 32 ;
			nMaxFrameSupply =
					esl_max( 1, (int) (nMaxBlurSupply / nBlurFrames) ) ;
		}
		const Particle *	pParticls = m_aParticles.GetConstArray() ;
		const size_t		nCount = m_aParticles.GetLength() ;
		const size_t		nPointMaxCount = nCount * (nPointSupply + 1) ;
		const size_t		nMaxCount = nCount * (nMaxBlurSupply + 1) ;
		S3DVector4 *		pvPoints = m_bufPoints.GetArray( nPointMaxCount ) ;
		size_t *			pFrames = m_bufFrames.GetArray( nMaxCount ) ;
		S3DColor *			pColors = m_bufColors.GetArray( nMaxCount ) ;
		float32_t *			pZooms = NULL ;
		float32_t *			pxAspects = NULL ;
		S4DVector *			pFaceDirs = NULL ;
		ParticleIndex *		pIndexes = NULL ;
		S3DMatrix *			pMatrixs = NULL ;
		if ( flagIndexedParticle )
		{
			pIndexes = m_bufParticleIndex.GetArray( nMaxCount ) ;
			pMatrixs = m_bufFaceMatrixs.GetArray( nMaxCount ) ;
			pZooms = m_bufZooms.GetArray( nMaxCount ) ;
		}
		else
		{
			pZooms = m_bufZooms.GetArray( nMaxCount ) ;
		}
		float32_t	fpSec2Frame = 0.0 ;
		double		secLife = m_particle.secDuration ;
		//
		// 空間変換行列取得
		//
		S3DDMatrix	matdITargetSpace( 1, 1, 1 ) ;
		S3DDVector	vdITargetSpace( 0, 0, 0 ) ;
		pRenderTarget->GetTargetSpaceTransformation
							( matdITargetSpace, vdITargetSpace ) ;
		//
		S3DMatrix	matParticleSpace( 1, 1, 1 ) ;
		S3DVector	vParticleSpace = vdITargetSpace ;
		if ( m_particle.nFlags & particleLocalSpace )
		{
			S3DDMatrix	matdParticleSpace ;
			S3DDVector	vdParticleSpace ;
			GetGlobalTransformation( matdParticleSpace, vdParticleSpace ) ;
			//
			matParticleSpace = matdParticleSpace ;
			matdITargetSpace *= matdParticleSpace ;
			vParticleSpace += S3DVector(vdParticleSpace) ;
		}
		S3DMatrix	matITargetSpace = matdITargetSpace ;
		//
		S3DScene::Camera *	pCamera = scene.GetCurrentCamera() ;
		if ( pCamera == NULL )
		{
			pCamera = scene.GetMainCamera() ;
		}
		S4DVector	vFaceDir( 0, 0, -1, 0 ) ;
		if ( m_particle.nFlags
			& (particleRotation | particleRotation3D
					| particleFaceDir | particleMoveAspect) )
		{
			pFaceDirs = m_bufFaceDirs.GetArray( nMaxCount ) ;
			//
			if ( !(m_particle.nFlags & particleRotation3D) )
			{
				if ( pCamera != NULL )
				{
					S3DDMatrix	matLinkCamera ;
					S3DDVector	vLinkCamera ;
					pCamera->CalcItemLinkTransformation
									( matLinkCamera, vLinkCamera ) ;
					//
					// ※GetCameraPosition, GetCameraTarget は S3DDynamicCamera 効果を考慮しない
					// 　VR HMD 等の効果を反映するには直接値を参照する
					S3DDVector	vCamera =
							matLinkCamera * pCamera->m_space.m_vCenter ;
//							matLinkCamera * pCamera->GetCameraPosition() ;
					S3DDVector	vTarget =
							matLinkCamera * pCamera->m_vTarget ;
//							matLinkCamera * pCamera->GetCameraTarget() ;
					//
					S3DDVector	vViewDir = vTarget - vCamera ;
					vViewDir.Normalize() ;
					//
					vFaceDir = vViewDir ;
					ESLAssert( !vFaceDir.IsNaN() ) ;
					vFaceDir.w = 0.0 ;
				}
				matITargetSpace.RevolveVector( vFaceDir ) ;
			}
			if ( m_particle.nFlags & particleMoveAspect )
			{
				pxAspects = m_bufXAspects.GetArray( nMaxCount ) ;
			}
		}
		//
		// アニメーション情報取得
		//
		double	secLength ;
		size_t	nTotalFrames = 1 ;
		if ( pRenderTarget->GetTargetAnimationLength( secLength ) )
		{
			nTotalFrames = pRenderTarget->GetTargetAnimationFrames() ;
			if ( nTotalFrames == 0 )
			{
				nTotalFrames = 1 ;
			}
			if ( secLength > 0.0 )
			{
				fpSec2Frame = (float32_t) ((nTotalFrames - 1) / secLength) ;
			}
			if ( !(m_particle.nFlags & particleDuration)
				&& (m_particle.fpAnimationSpeed > 0.0) )
			{
				secLife = (secLength / m_particle.fpAnimationSpeed) ;
			}
		}
		ESLAssert( nTotalFrames >= 1 ) ;
		//
		// 全体透明度
		//
		uint32_t	nAlpha = 0xFF ;
		if ( m_space.m_nTransparency > 0 )
		{
			if ( m_space.m_nTransparency >= 0x100 )
			{
				return ;
			}
			nAlpha = nAlpha * (0x100 - m_space.m_nTransparency) / 0x100 ;
			nAlpha = (uint32_t) esl_clampi( (int) nAlpha, 0, 0xFF ) ;
		}
		//
		// 全粒子順次処理
		//
		bool	flagFadein = (m_particle.nFlags & particleDuration)
								&& (m_particle.secFadein > 0.0) ;
		bool	flagFadeout = (m_particle.nFlags & particleDuration)
								&& (m_particle.secFadeout > 0.0) ;
		float32_t	rcpLife = (secLife == 0.0) ? 0.0f : (float32_t) (1.0 / secLife) ;
		float32_t	fpZoomDelta = m_particle.fpSizeScaleEnd - 1.0f ;
		//
		size_t		iDst = 0 ;
		size_t		iPoint = 0 ;
		S4DVector	vTempFaceDir ;
		float32_t	fpTempAspect = 1.0f ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			pvPoints[iPoint] =
				matParticleSpace * pParticls->vPos + vParticleSpace ;
			pFrames[iDst] = (size_t) esl_min
				( (int) nTotalFrames - 1,
					eslRoundR32ToInt( pParticls->secAnimation * fpSec2Frame ) ) ;
			pColors[iDst] = pParticls->clrParticle ;
			pColors[iDst].rgbMul.argb.Alpha =
				(uint8_t) ((((uint32_t) pParticls->clrParticle.rgbMul.argb.Alpha + 1) * nAlpha) >> 8) ;
			//
			if ( flagFadein || flagFadeout )
			{
				float32_t	a = 1.0f ;
				if ( flagFadeout )
				{
					a =  esl_fminf( 1.0f, (m_particle.secDuration
											- pParticls->secLife)
												/ m_particle.secFadeout ) ;
				}
				if ( flagFadein && (pParticls->secLife < m_particle.secFadein) )
				{
					a *= pParticls->secLife / m_particle.secFadein ;
				}
				pColors[iDst].rgbMul.argb.Alpha =
					(uint8_t) eslRoundR32ToInt
						( (float32_t) pColors[iDst].rgbMul.argb.Alpha * a ) ;
			}
			//
			float32_t	t = pParticls->secLife * rcpLife ;
			pZooms[iDst] = pParticls->fpZoom * (1.0f + fpZoomDelta * t) ;
			//
			if ( m_particle.nFlags & particleMoveAspect )
			{
				double	fpSpeed = pParticls->vSpeed.Absolute() ;
				if ( fpSpeed > m_particle.fpAspectSpeed )
				{
					S3DVector	vCross = pParticls->vSpeed * vFaceDir ;
					S3DVector	vNormal = (pParticls->vSpeed * vCross).Normalized() ;
					S3DMatrix	matRot( 1, 1, 1 ) ;
					matRot.RevolveForAngle( vNormal ) ;
					//
					vTempFaceDir = vNormal ;
					vTempFaceDir.w = 0.0f ;
					//
					S3DVector	vRotZ = matRot.Inverse() * pParticls->vSpeed ;
					double		xy2 = sqrt( vRotZ.x * vRotZ.x + vRotZ.y * vRotZ.y ) ;
					if ( xy2 > 1.0e-5 )
					{
						double	cosX = vRotZ.x / xy2 ;
						double	sinY = vRotZ.y / xy2 ;
						double	rad = atan2( sinY, cosX ) ;
						vTempFaceDir.w = (float32_t) (rad + PI * 0.5) ;
					}
					float32_t	yZoom = (float32_t) fpSpeed / m_particle.fpAspectSpeed ;
					pZooms[iDst] *= yZoom ;
					fpTempAspect = 1.0f / yZoom ;
				}
				else
				{
					vTempFaceDir = vFaceDir ;
					fpTempAspect = 1.0f ;
				}
				pFaceDirs[iDst] = vTempFaceDir ;
				pxAspects[iDst] = fpTempAspect ;
			}
			else if ( m_particle.nFlags & particleRotation3D )
			{
				S3DMatrix	matRotate( 1, 1, 1 ) ;
				matRotate.RevolveOnY
					( sin(pParticls->vRotate.y), cos(pParticls->vRotate.y) ) ;
				matRotate.RevolveOnX
					( sin(pParticls->vRotate.x), cos(pParticls->vRotate.x) ) ;
				//
				vTempFaceDir = S4DVector( 0, 0, -1, 0 ) ;
				matRotate.RevolveVector( vTempFaceDir ) ;
				vTempFaceDir.w = pParticls->vRotate.z ;
				//
				pFaceDirs[iDst] = vTempFaceDir ;
			}
			else if ( m_particle.nFlags & particleRotation )
			{
				if ( m_particle.nFlags & particleFaceDir )
				{
					vTempFaceDir = pParticls->vFaceDir ;
				}
				else
				{
					vTempFaceDir = vFaceDir ;
				}
				vTempFaceDir.w = pParticls->vRotate.z ;
				pFaceDirs[iDst] = vTempFaceDir ;
			}
			else if ( m_particle.nFlags & particleFaceDir )
			{
				vTempFaceDir = pParticls->vFaceDir ;
				pFaceDirs[iDst] = vTempFaceDir ;
			}
			if ( flagIndexedParticle )
			{
				float32_t	zy = pZooms[iDst] ;
				float32_t	zx = zy ;
				if ( pxAspects != NULL )
				{
					zx *= pxAspects[iDst] ;
				}
				S3DMatrix	mat3( 1, 1, 1 ) ;
				mat3.RevolveForAngle( vTempFaceDir ) ;
				mat3.RevolveOnZ( sin(vTempFaceDir.w), cos(vTempFaceDir.w) ) ;
				mat3.MagnifyByVector( S3DVector( zx, zy, zy ) ) ;
				pMatrixs[iDst] = mat3 ;
				//
				ParticleIndex&	pi = pIndexes[iDst] ;
				pi.nIndex = iPoint ;
				pi.nBlurCount = pParticls->nLastPosCount + 1 ;
				pi.nIdentity = pParticls->nIdentity ;
				//
				iPoint ++ ;
				for ( size_t j = 0; j < pParticls->nLastPosCount; j ++ )
				{
					pvPoints[iPoint ++] =
						matParticleSpace * pParticls->vLastPos[j] + vParticleSpace ;
				}
			}
			else
			{
				iPoint ++ ;
			}
			iDst ++ ;
			//
			if ( !flagIndexedParticle && (nBlurFrames > 0) )
			{
				//
				// ブラー
				//
				S3DVector	vLastPos = pvPoints[iPoint - 1] ;
				size_t		iFrame = pFrames[iDst - 1] ;
				float32_t	fpZoom = pZooms[iDst - 1] ;
				S3DColor	clrBase = pColors[iDst - 1] ;
				uint32_t	nBaseAlpha = clrBase.rgbMul.argb.Alpha ;
				uint32_t	nLastAlpha = nBaseAlpha ;
				//
				size_t		nCurBlurFrames =
					(size_t) esl_min
						( (int) nBlurFrames, (int) pParticls->nLastPosCount ) ;
				//
				for ( size_t j = 0; j < nCurBlurFrames; j ++ )
				{
					uint32_t	nNextAlpha =
								nBaseAlpha * (uint32_t) (nCurBlurFrames - j - 1)
											/ (uint32_t) nCurBlurFrames ;
					uint32_t	nDeltaAlpha = nLastAlpha - nNextAlpha ;
					//
					S3DVector	vNextPos = matParticleSpace * pParticls->vLastPos[j]
																+ vParticleSpace ;
					S3DVector	vDelta = vNextPos - vLastPos ;
					float32_t	r = (float32_t) vDelta.Absolute() ;
					int			nDiv = eslRoundR32ToInt
										( r / m_particle.fpBlurSupplement ) ;
					nDiv = esl_clampi( nDiv, 1, (int) nMaxFrameSupply ) ;
					//
					for ( int k = 1; k <= nDiv; k ++ )
					{
						double	t = (double) k / nDiv ;
						pvPoints[iPoint] = vLastPos + vDelta * t ;
						pFrames[iDst] = iFrame ;
						clrBase.rgbMul.argb.Alpha = nLastAlpha - nDeltaAlpha * k / nDiv ;
						pColors[iDst] = clrBase ;
						pZooms[iDst] = fpZoom ;
						if ( pFaceDirs != NULL )
						{
							pFaceDirs[iDst] = vTempFaceDir ;
						}
						if ( pxAspects != NULL )
						{
							pxAspects[iDst] = fpTempAspect ;
						}
						iPoint ++ ;
						iDst ++ ;
					}
					vLastPos = vNextPos ;
					nLastAlpha = nNextAlpha ;
				}
			}
			pParticls ++ ;
		}
		//
		// 出力
		//
		ESLAssert( iPoint <= nPointMaxCount ) ;
		ESLAssert( iDst <= nMaxCount ) ;
		if ( flagIndexedParticle )
		{
			pRenderTarget->AddIndexedParticles
				( iDst, pvPoints, pIndexes, pFrames, pColors, pMatrixs ) ;
		}
		else
		{
			pRenderTarget->AddParticles
				( iDst, pvPoints, pFrames, pColors, pZooms, pFaceDirs, pxAspects ) ;
		}
		m_bufPoints.FinishArray() ;
		m_bufFrames.FinishArray() ;
		m_bufColors.FinishArray() ;
		m_bufParticleIndex.FinishArray() ;
		m_bufFaceMatrixs.FinishArray() ;
		m_bufZooms.FinishArray() ;
		m_bufXAspects.FinishArray() ;
		m_bufFaceDirs.FinishArray() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// パーティクル放出アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DParticleShootSerializer::m_paramEntries
		[S3DParticleShootSerializer::paramShootCount] =
{
	{ L"particle_item",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrStringEnumeration,
		L"パーティクル出力先", NULL },
	{ L"gen_count",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"生成数", L"パーティクル生成数 [個/秒]" },
	{ L"count_per_frame",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"生成数毎フレーム", L"パーティクル生成数を [個/frame] に変更" },
	{ L"use_space_scale",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"空間スケール反映", NULL },
	{ L"size_scale",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"粒子サイズ比率", NULL },
	{ L"speed_scale",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"放出速度比", L"出力先パーティクルの放射方向の設定速度に対する比率" },
	{ L"emission_speed",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"放出速度加算", L"放射方向の速度に対する加算" },
	{ L"use_direction",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"放出方向有効", NULL },
	{ L"direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,
		L"放出方向", NULL },
	{ L"use_speed",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"放出速度有効", NULL },
	{ L"speed_dir",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,
		L"放出速度方向", NULL },
	{ L"speed",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"放出速度", L"放出するベース速度" },
	{ L"use_face_dir",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"面の向き指定", NULL },
	{ L"face_dir",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,
		L"面の向き", NULL },
} ;

const S3DSceneComposer::ParamSetClass
	S3DParticleShootSerializer::m_pscClass =
{
	&S3DSceneComposer::ItemBasicSerializer::m_pscClass,
	S3DParticleShootSerializer::paramShootCount,
	&S3DParticleShootSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DParticleShootSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DParticleShootSerializer, particle_shooter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleShootSerializer::S3DParticleShootSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID, &m_pscClass )
{
	m_classItem = S3DScene::classPreRender ; 
	m_flagsBehavior |= S3DScene::itemTimer ;
	//
	m_flagCountPerFrame = false ;
	m_flagSpaceScale = true ;
	m_flagDirection = false ;
	m_flagBaseSpeed = false ;
	m_flagFaceDir = false ;
	m_fpGenCount = 0 ;
	m_fpZoomScale = 1.0 ;
	m_fpEmissionSpeedScale = 1.0 ;
	m_fpEmissionSpeed = 0.0 ;
	m_vDirection = S3DDVector( 0, 0, 1 ) ;
	m_vSpeedDir = S3DDVector( 0, 0, 1 ) ;
	m_fpSpeed = 0.0 ;
	m_vFaceDir = S3DDVector( 0, 0, -1 ) ;
}

// 出力先
//////////////////////////////////////////////////////////////////////////////
void S3DParticleShootSerializer::AttachParticleItem
	( S3DParticleSerializer * pParticle, const wchar_t * pwszID )
{
	m_refParticle.SetReference( (S3DScene::Item*) pParticle ) ;
	m_strParticle = pwszID ;
}

bool S3DParticleShootSerializer::UpdateParticleReference( void )
{
	if ( m_strParticle.IsEmpty() )
	{
		m_refParticle.SetReference( NULL ) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != NULL )
	{
		S3DSceneComposer::ItemSerializer *
				pItem = pComp->GetSceneItemAs( m_strParticle ) ;
		m_refParticle.SetReference( pItem ) ;
		return	(pItem != NULL) ;
	}
	return	false ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DParticleShootSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDirection:
		return	m_vDirection ;
	case	paramBaseSpeedDir:
		return	m_vSpeedDir ;
	case	paramFaceDir:
		return	m_vFaceDir ;
	}
	return	ItemBasicSerializer::GetVectorParameter( i ) ;
}

double S3DParticleShootSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGenCount:
		return	m_fpGenCount ;
	case	paramSizeScale:
		return	m_fpZoomScale ;
	case	paramSpeedScale:
		return	m_fpEmissionSpeedScale ;
	case	paramEmissionSpeed:
		return	m_fpEmissionSpeed ;
	case	paramBaseSpeed:
		return	m_fpSpeed ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

bool S3DParticleShootSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramCountPerFrame:
		return	m_flagCountPerFrame ;
	case	paramUseSpaceScale:
		return	m_flagSpaceScale ;
	case	paramUseDirection:
		return	m_flagDirection ;
	case	paramUseBaseSpeed:
		return	m_flagBaseSpeed ;
	case	paramUseFaceDir:
		return	m_flagFaceDir ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

const wchar_t * S3DParticleShootSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramParticleItem:
		return	m_strParticle ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DParticleShootSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramDirection:
		m_vDirection= vec ;
		return ;
	case	paramBaseSpeedDir:
		m_vSpeedDir = vec ;
		return ;
	case	paramFaceDir:
		m_vFaceDir = vec ;
		return ;
	}
	ItemBasicSerializer::SetVectorParameter( i, vec ) ;
}

void S3DParticleShootSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramGenCount:
		m_fpGenCount = s ;
		return ;
	case	paramSizeScale:
		m_fpZoomScale = s ;
		return ;
	case	paramSpeedScale:
		m_fpEmissionSpeedScale = s ;
		return ;
	case	paramEmissionSpeed:
		m_fpEmissionSpeed = s ;
		return ;
	case	paramBaseSpeed:
		m_fpSpeed = s ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DParticleShootSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramCountPerFrame:
		m_flagCountPerFrame = b ;
		return ;
	case	paramUseSpaceScale:
		m_flagSpaceScale = b ;
		return ;
	case	paramUseDirection:
		m_flagDirection = b ;
		return ;
	case	paramUseBaseSpeed:
		m_flagBaseSpeed = b ;
		return ;
	case	paramUseFaceDir:
		m_flagFaceDir = b ;
		return ;
	}
	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

void S3DParticleShootSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramParticleItem:
		if ( m_strParticle != pwszCmd )
		{
			m_strParticle = pwszCmd ;
			UpdateParticleReference() ;
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DParticleShootSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramParticleItem:
		pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DParticleSerializer) ) ;
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DParticleShootSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramRotation:
	case	paramZoom:
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
const wchar_t * S3DParticleShootSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"放出設定" ;
	}
	return	NULL ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DParticleShootSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;
	//
	if ( m_fpGenCount > 0.0 )
	{
		S3DParticleSerializer *	pParticle =
			ESLTypeCast<S3DParticleSerializer>( m_refParticle.GetReference() ) ;
		if ( pParticle == NULL )
		{
			return ;
		}
		double	fpGenCount = m_fpGenCount ;
		if ( !m_flagCountPerFrame )
		{
			fpGenCount *= msecPast / 1000.0 ;
		}
		size_t	nCount = pParticle->GenerateCount( fpGenCount ) ;
		if ( nCount == 0 )
		{
			return ;
		}
		S3DDMatrix	matdItem ;
		S3DDVector	vdItem ;
		S3DColor	clrEffect ;
		GetGlobalTransformation( matdItem, vdItem ) ;
		GetGlobalColorEffect( clrEffect ) ;
		//
		double	fpSpaceScale = 1.0 ;
		if ( m_flagSpaceScale )
		{
			fpSpaceScale = pow( matdItem.Determinant(), 1.0 / 3.0 ) ;
		}
		//
		S3DVector	vDirection = matdItem * m_vDirection ;
		S3DVector	vBaseSpeed = matdItem * m_vSpeedDir * m_fpSpeed ;
		S3DVector	vFaceDir = matdItem * m_vFaceDir ;
		//
		pParticle->GenerateParticlesByParam
			( nCount, vdItem,
				(m_flagDirection ? &vDirection : NULL),
				(m_flagBaseSpeed ? &vBaseSpeed : NULL),
				(m_flagFaceDir ? &vFaceDir : NULL),
				&clrEffect,
				(float32_t) (m_fpZoomScale * fpSpaceScale), 1.0f,
				(float32_t) (m_fpEmissionSpeedScale * fpSpaceScale),
				(float32_t) (m_fpEmissionSpeed * fpSpaceScale) ) ;
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DParticleShootSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateParticleReference() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}



