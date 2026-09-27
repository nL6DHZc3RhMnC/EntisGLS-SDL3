
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sglx3d_render.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 乱数範囲
//////////////////////////////////////////////////////////////////////////////

S3DParticleGenerator::NumberRange::NumberRange( double num, double r )
	: fpNumber( (float32_t) num), fpRange( (float32_t) r)
{
}

S3DParticleGenerator::NumberRange::NumberRange
		( const S3DParticleGenerator::NumberRange& nr )
	: fpNumber(nr.fpNumber), fpRange(nr.fpRange)
{
}

const S3DParticleGenerator::NumberRange&
	S3DParticleGenerator::NumberRange::operator =
			( const S3DParticleGenerator::NumberRange& nr )
{
	fpNumber = nr.fpNumber ;
	fpRange = nr.fpRange ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// 生成パラメータ
//////////////////////////////////////////////////////////////////////////////

S3DParticleGenerator::GenerationParam::GenerationParam
		( const S3DParticleGenerator::GenerationParam& gp )
	: vPosition(gp.vPosition), vOffset(gp.vOffset),
		fpRadius(gp.fpRadius),
		vDirection(gp.vDirection), fpAngle(gp.fpAngle),
		nrVelocity(gp.nrVelocity), nGenPerSec(gp.nGenPerSec)
{
}

const S3DParticleGenerator::GenerationParam&
	S3DParticleGenerator::GenerationParam::operator =
		( const S3DParticleGenerator::GenerationParam& gp )
{
	vPosition = gp.vPosition ;
	vOffset = gp.vOffset ;
	fpRadius = gp.fpRadius ;
	vDirection = gp.vDirection ;
	fpAngle = gp.fpAngle ;
	nrVelocity = gp.nrVelocity ;
	nGenPerSec = gp.nGenPerSec ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// 媒質パラメータ
//////////////////////////////////////////////////////////////////////////////

S3DParticleGenerator::FieldParam::FieldParam
		( const S3DParticleGenerator::FieldParam& fp )
	: vGravity(fp.vGravity),
		vStream(fp.vStream), fpAttenuation(fp.fpAttenuation)
{
}

const S3DParticleGenerator::FieldParam&
	S3DParticleGenerator::FieldParam::operator =
		( const S3DParticleGenerator::FieldParam& fp )
{
	vGravity = fp.vGravity ;
	vStream = fp.vStream ;
	fpAttenuation = fp.fpAttenuation ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// 揺らぎパラメータ
//////////////////////////////////////////////////////////////////////////////

S3DParticleGenerator::FlickeringParam::FlickeringParam
		( const S3DParticleGenerator::FlickeringParam& fp )
	: nrAmplitude(fp.nrAmplitude), nrCycle(fp.nrCycle)
{
}

const S3DParticleGenerator::FlickeringParam&
	S3DParticleGenerator::FlickeringParam::operator =
		( const S3DParticleGenerator::FlickeringParam& fp )
{
	nrAmplitude = fp.nrAmplitude ;
	nrCycle = fp.nrCycle ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// 粒子パラメータ
//////////////////////////////////////////////////////////////////////////////

S3DParticleGenerator::ParticleParam::ParticleParam( void )
	: msecFadein(0), msecFadeout(0), msecDuration(10000),
		fpObliquity(0.0), fpZoomIn(1.0), fpZoomOut(1.0)
{
}

S3DParticleGenerator::ParticleParam::ParticleParam
	( const S3DParticleGenerator::ParticleParam& pp )
	: msecFadein(pp.msecFadein), msecFadeout(pp.msecFadeout),
		msecDuration(pp.msecDuration),
		nrRotation(pp.nrRotation), fpObliquity(pp.fpObliquity),
		nrZoom(pp.nrZoom), fpZoomIn(pp.fpZoomIn), fpZoomOut(pp.fpZoomOut)
{
	fpFlickering[0] = pp.fpFlickering[0] ;
	fpFlickering[1] = pp.fpFlickering[1] ;
}

const S3DParticleGenerator::ParticleParam&
	S3DParticleGenerator::ParticleParam::operator =
		( const S3DParticleGenerator::ParticleParam& pp )
{
	msecFadein = pp.msecFadein ;
	msecFadeout = pp.msecFadeout ;
	msecDuration = pp.msecDuration ;
	nrRotation = pp.nrRotation ;
	fpObliquity = pp.fpObliquity ;
	nrZoom = pp.nrZoom ;
	fpZoomIn = pp.fpZoomIn ;
	fpZoomOut = pp.fpZoomOut ;
	fpFlickering[0] = pp.fpFlickering[0] ;
	fpFlickering[1] = pp.fpFlickering[1] ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// 粒子インスタンス
//////////////////////////////////////////////////////////////////////////////

void SakuraGL::S3DParticleGenerator::ParticleInstance::CalcPosition( S3DVector& vPos ) const
{
	vPos = vPosition ;
	//
	if ( fiFlickering[0].secCycle > 0 )
	{
		vPos += fiFlickering[0].vAmplitude
			* sin( (msecLife * 0.001 + fiFlickering[0].secPhase)
						/ fiFlickering[0].secCycle * (2.0 * PI) ) ;
	}
	if ( fiFlickering[1].secCycle > 0 )
	{
		vPos += fiFlickering[1].vAmplitude
			* sin( (msecLife * 0.001 + fiFlickering[1].secPhase)
						/ fiFlickering[1].secCycle * (2.0 * PI) ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 粒子効果オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DParticleGenerator::ParticleEffector, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleGenerator::ParticleEffector::ParticleEffector( void )
{
}

S3DParticleGenerator::ParticleEffector::ParticleEffector
	( const S3DParticleGenerator::ParticleEffector& pe )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleGenerator::ParticleEffector::~ParticleEffector( void )
{
}


// 吸引オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DParticleGenerator::ParticleAbsorber, ParticleEffector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleGenerator::ParticleAbsorber::ParticleAbsorber( void )
{
	m_rAbsob = 1 ;
	m_fpGravity = 0 ;
	m_fpStream = 0 ;
}

S3DParticleGenerator::ParticleAbsorber::ParticleAbsorber
		( const S3DVector& vPos, double r, double g, double str )
	: m_vPos( vPos ), m_rAbsob( r ), m_fpGravity( g ), m_fpStream( str )
{
}

// 効果
//////////////////////////////////////////////////////////////////////////////
S3DParticleGenerator::ParticleEffectResult
	S3DParticleGenerator::ParticleAbsorber::EffectParticle
		( const S3DParticleGenerator& pg,
			S3DParticleGenerator::ParticleInstance& pi, size_t msecTime )
{
	S3DVector	vDelta = pi.vPosition - m_vPos ;
	if ( vDelta.Absolute() <= m_rAbsob )
	{
		return	effectRemove ;
	}
	vDelta.Normalize() ;
	//
	S3DVector	vSpeed = pi.vSpeed ;
	double		secTime = msecTime * 0.001 ;
	if ( m_fpGravity > 0.0 )
	{
		vSpeed -= vDelta * (m_fpGravity * secTime) ;
	}
	const FieldParam&	fp = pg.GetFieldParam() ;
	S3DVector	vStream = vDelta * m_fpStream ;
	vSpeed -= vStream ;
	vSpeed *= (float32_t) pow( 1.0 - fp.fpAttenuation, secTime) ;
	vSpeed += vStream ;
	pi.vSpeed = vSpeed ;
	return	effectContiue ;
}


//////////////////////////////////////////////////////////////////////////////
// 抽象パーティクル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DParticleGenerator, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleGenerator::S3DParticleGenerator( void )
{
	m_random.InitializeSeed() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleGenerator::~S3DParticleGenerator( void )
{
}

// 生成パラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::SetGenerationParam
	( const S3DParticleGenerator::GenerationParam& gp )
{
	m_gparam = gp ;
}

// 媒質パラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::SetFieldParam
	( const S3DParticleGenerator::FieldParam& fp )
{
	m_fparam = fp ;
}

// 粒子パラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::SetParticleParam
	( const S3DParticleGenerator::ParticleParam& pp )
{
	m_pparam = pp ;
}

// 疑似乱数の種を設定
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::SetRandomSeed( uint32_t nSeed )
{
	m_random.InitializeSeedBy( nSeed ) ;
}

// パーティクル取得
//////////////////////////////////////////////////////////////////////////////
const S3DParticleGenerator::ParticleInstance *const *
	S3DParticleGenerator::GetParticleArray( size_t& nCount ) const
{
	nCount = m_particles.GetLength() ;
	return	m_particles.GetConstArray() ;
}

// 効果オブジェクト追加
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::AddEffector
		( S3DParticleGenerator::ParticleEffector * pEffector )
{
	m_effectors.Add( pEffector ) ;
}

// 効果オブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::RemoveEffector
		( S3DParticleGenerator::ParticleEffector * pEffector )
{
	ssize_t	i = m_effectors.FindPtr( pEffector ) ;
	if ( i >= 0 )
	{
		m_effectors.RemoveAt( (size_t) i ) ;
	}
	else
	{
		delete	pEffector ;
	}
}

// 時間を進める
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::AdvanceTime( size_t msecTime )
{
	size_t	nGenCount = msecTime * m_gparam.nGenPerSec ;
	size_t	nDecCount = nGenCount % 1000 ;
	nGenCount /= 1000 ;
	if ( nDecCount > m_random.QuickRandomize(1000) )
	{
		nGenCount ++ ;
	}
	GenerateParticle( nGenCount ) ;
	AdvanceParticle( msecTime ) ;
}

// 乱数生成
//////////////////////////////////////////////////////////////////////////////
float32_t S3DParticleGenerator::RandomizeNumber( double fpRange )
{
	return	(float32_t) (m_random.QuickRandomize(0x100000)
											* fpRange / 0x100000) ;
}

float32_t S3DParticleGenerator::RandomizeNumber
		( const S3DParticleGenerator::NumberRange& nr )
{
	return	(float32_t) (nr.fpNumber
							+ m_random.QuickRandomize(0x100000)
										* nr.fpRange / 0x100000) ;
}

// 粒子生成
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::GenerateParticle( size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ParticleInstance *	p = new ParticleInstance ;
		m_particles.Add( p ) ;
		//
		GenerateParticlePosition( *p ) ;
		//
		p->msecLife = 0 ;
		p->speedRotation = RandomizeNumber( m_pparam.nrRotation ) ;
		p->degRotation = 0 ;
		p->axisRotation.x = 0 ;
		p->axisRotation.y = 0 ;
		p->axisRotation.z = 1 ;
		//
		if ( m_pparam.fpObliquity > 1.0e-8 )
		{
			S3DMatrix	mat( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			double	rad = RandomizeNumber(360) * PI / 180.0 ;
			mat.RevolveOnZ( sin(rad), cos(rad) ) ;
			rad = RandomizeNumber(m_pparam.fpObliquity) * PI / 180.0 ;
			mat.RevolveOnY( sin(rad), cos(rad) ) ;
			mat.RevolveVector( p->axisRotation ) ;
		}
		//
		p->fpZoom = RandomizeNumber( m_pparam.nrZoom ) ;
		//
		for ( size_t j = 0; j < 2; j ++ )
		{
			p->fiFlickering[j].vAmplitude.x = 0 ;
			p->fiFlickering[j].vAmplitude.y = 0 ;
			p->fiFlickering[j].vAmplitude.z = 0 ;
			p->fiFlickering[j].secCycle = 0 ;
			p->fiFlickering[j].secPhase = 0 ;
			//
			double	amp =
				RandomizeNumber( m_pparam.fpFlickering[j].nrAmplitude ) ;
			if ( amp >= 1.0e-5 )
			{
				S3DMatrix	mat( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
				double	rad = RandomizeNumber(360) * PI / 180.0 ;
				mat.RevolveForAngle( p->vSpeed ) ;
				mat.RevolveOnZ( sin(rad), cos(rad) ) ;
				p->fiFlickering[j].vAmplitude.x = (float32_t) amp ;
				mat.RevolveVector( p->fiFlickering[j].vAmplitude ) ;
				//
				p->fiFlickering[j].secCycle =
					RandomizeNumber( m_pparam.fpFlickering[j].nrCycle ) ;
				p->fiFlickering[j].secPhase =
					RandomizeNumber( p->fiFlickering[j].secCycle ) ;
			}
		}
	}
}

// 生成座標
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::GenerateParticlePosition
			( S3DParticleGenerator::ParticleInstance& pi )
{
	S3DMatrix	mat( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DVector	vDir( 0, 0, 1 ) ;
	double	rad ;
	mat.RevolveForAngle( m_gparam.vDirection ) ;
	rad = RandomizeNumber( 360 ) * PI / 180.0 ;
	mat.RevolveOnZ( sin(rad), cos(rad) ) ;
	rad = RandomizeNumber( m_gparam.fpAngle ) * PI / 180.0 ;
	mat.RevolveOnX( sin(rad), cos(rad) ) ;
	mat.RevolveVector( vDir ) ;
	//
	pi.vPosition = m_gparam.vPosition ;
	pi.vPosition += m_gparam.vOffset * RandomizeNumber( 1.0 ) ;
	pi.vPosition += vDir * RandomizeNumber( m_gparam.fpRadius ) ;
	pi.vSpeed = vDir * RandomizeNumber( m_gparam.nrVelocity ) ;
}

// 粒子挙動
//////////////////////////////////////////////////////////////////////////////
void S3DParticleGenerator::AdvanceParticle( size_t msecTime )
{
	size_t				nParticles = m_particles.GetLength() ;
	ParticleInstance **	ppParticles = m_particles.GetArray() ;
	size_t				nEffectors = m_effectors.GetLength() ;
	ParticleEffector *const*
						ppEffectors = m_effectors.GetConstArray() ;
	//
	for ( size_t i = 0; i < nParticles; i ++ )
	{
		ParticleInstance *	p = ppParticles[i] ;
		if ( p == NULL )
		{
			continue ;
		}
		if ( nEffectors > 0 )
		{
			for ( size_t j = 0; j < nEffectors; j ++ )
			{
				ParticleEffector *	pEffector = ppEffectors[j] ;
				ESLAssert( pEffector != NULL ) ;
				ParticleEffectResult
					per = pEffector->EffectParticle( *this, *p, msecTime ) ;
				if ( per == effectRemove )
				{
					delete	p ;
					ppParticles[i] = NULL ;
					p = NULL ;
					break ;
				}
			}
			if ( p == NULL )
			{
				continue ;
			}
		}
		ParticleEffectResult	per = MoveParticle( *p, msecTime ) ;
		if ( per == effectRemove )
		{
			delete	p ;
			ppParticles[i] = NULL ;
			p = NULL ;
		}
	}
	m_particles.FinishArray() ;
	m_particles.TrimEmpty() ;
}

// 粒子挙動
//////////////////////////////////////////////////////////////////////////////
S3DParticleGenerator::ParticleEffectResult
	S3DParticleGenerator::MoveParticle
		(  S3DParticleGenerator::ParticleInstance& pi, size_t msecTime )
{
	pi.msecLife += msecTime ;
	if ( pi.msecLife >= m_pparam.msecDuration )
	{
		return	effectRemove ;
	}
	S3DVector	vSpeed = pi.vSpeed ;
	double		secTime = msecTime * 0.001 ;
	vSpeed += m_fparam.vGravity * secTime ;
	vSpeed -= m_fparam.vStream ;
	vSpeed *= (float32_t) pow( 1.0 - m_fparam.fpAttenuation, secTime) ;
	vSpeed += m_fparam.vStream ;
	pi.vSpeed = vSpeed ;
	pi.vPosition += vSpeed * secTime ;
	return	effectContiue ;
}


//////////////////////////////////////////////////////////////////////////////
// パーティクル・アイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DParticleBillboardItem, BillboardItem, S3DParticleGenerator )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleBillboardItem::S3DParticleBillboardItem( void )
{
	m_flagsBehavior |= S3DScene::itemTimer | S3DScene::itemOwnerBehavior ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DParticleBillboardItem::~S3DParticleBillboardItem( void )
{
}

// パーティクル画像設定
//////////////////////////////////////////////////////////////////////////////
void S3DParticleBillboardItem::SetBillboardParam
	( const S3DMeshShaper::BillboardParam& bp,
		uint32_t flagsBillboard, uint32_t flagsZBuf, uint32_t fxAnimeSpeed )
{
	m_ipImage.bp = bp ;
	m_ipImage.nBillboardFlags = flagsBillboard ;
	m_ipImage.nZBufFlags = flagsZBuf ;
	m_ipImage.fxAnimeSpeed = fxAnimeSpeed ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DParticleBillboardItem::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	AdvanceParticle( msecPast ) ;
	//
	if ( !(m_ipImage.nBillboardFlags & flagLoopAnimation) )
	{
		if ( (m_ipImage.bp.pImage != NULL)
			&& (m_ipImage.bp.pImage->GetFrameCount() >= 2) )
		{
			size_t	msecDuration =
						(size_t) (m_ipImage.bp.pImage->GetTotalTime()
									* m_ipImage.fxAnimeSpeed / 0x10000) ;
			size_t	nCount = m_particles.GetLength() ;
			ParticleInstance **
					ppParticles = m_particles.GetArray() ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				ParticleInstance *	p = ppParticles[i] ;
				if ( p != NULL )
				{
					if ( p->msecLife >= msecDuration )
					{
						delete	p ;
						ppParticles[i] = NULL ;
					}
				}
			}
			m_particles.FinishArray() ;
		}
	}
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DParticleBillboardItem::OnUpdateBehavior( S3DScene& scene )
{
	if ( m_ipImage.bp.pImage == NULL )
	{
		return ;
	}
	size_t	msecDuration =
				(size_t) (m_ipImage.bp.pImage->GetTotalTime()
							* m_ipImage.fxAnimeSpeed / 0x10000) ;
	size_t	nFrames = m_ipImage.bp.pImage->GetFrameCount() ;
	//
	m_bp = m_ipImage.bp ;
	m_flagsZBuf = m_ipImage.nZBufFlags ;
	//
	m_bufPoints.RemoveAll() ;
	m_bufFrames.RemoveAll() ;
	m_bufColors.RemoveAll() ;
	m_bufZooms.RemoveAll() ;
	//
	m_particles.TrimEmpty() ;
	//
	size_t			nCount = m_particles.GetLength() ;
	ParticleInstance *const*
					ppParticles = m_particles.GetConstArray() ;
	S3DVector4 *	pvPoints = m_bufPoints.GetArray( nCount ) ;
	size_t *		pFrames = m_bufFrames.GetArray( nCount ) ;
	S3DColor *		pColors = m_bufColors.GetArray( nCount ) ;
	float32_t *		pZooms = m_bufZooms.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ParticleInstance *	p = ppParticles[i] ;
		ESLAssert( p != NULL ) ;
		p->CalcPosition( *pvPoints ) ;
		//
		if ( (nFrames >= 2) && (msecDuration > 0) )
		{
			*pFrames =
				(p->msecLife % msecDuration) * nFrames / msecDuration ;
		}
		else
		{
			*pFrames = 0 ;
		}
		float32_t	fpZoom = p->fpZoom ;
		pColors->rgbMul = 0xFFFFFFFF ;
		pColors->rgbAdd = 0 ;
		if ( p->msecLife < m_pparam.msecFadein )
		{
			pColors->rgbMul.argb.Alpha =
				(uint8_t) esl_clampi
					( (int) (0xFF * p->msecLife / m_pparam.msecFadein), 0, 0xFF ) ;
			//
			float32_t	t = (float32_t) p->msecLife
								/ (float32_t) m_pparam.msecFadein ;
			*pZooms = m_pparam.fpZoomIn + (fpZoom - m_pparam.fpZoomIn) * t ;
		}
		else if ( p->msecLife
					> m_pparam.msecDuration - m_pparam.msecFadeout )
		{
			pColors->rgbMul.argb.Alpha =
				(uint8_t) esl_clampi
					( (int) (0xFF * (m_pparam.msecDuration - p->msecLife)
											/ m_pparam.msecFadeout), 0, 0xFF ) ;
			//
			float32_t	t = (float32_t) (m_pparam.msecDuration - p->msecLife)
											/ (float32_t) m_pparam.msecFadeout ;
			*pZooms = m_pparam.fpZoomOut + (fpZoom - m_pparam.fpZoomOut) * t ;
		}
		else
		{
			*pZooms = fpZoom ;
		}
		pvPoints ++ ;
		pFrames ++ ;
		pColors ++ ;
		pZooms ++ ;
	}
	m_bufPoints.FinishArray() ;
	m_bufFrames.FinishArray() ;
	m_bufColors.FinishArray() ;
	m_bufZooms.FinishArray() ;
}

