
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene.h>
#include <sakuraglx/render/sglx_model_buffer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 3D シーン・基底タイマ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::Timer, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Timer::Timer( bool flagAutoDelete )
{
	m_flagAutoDelete = flagAutoDelete ;
}

S3DScene::Timer::Timer( const S3DScene::Timer& tm )
{
	m_flagAutoDelete = tm.m_flagAutoDelete ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Timer::OnTimer
	( S3DScene& scene, S3DScene::Space& space, uint32_t msecPast )
{
	return	true ;		// 処理完了
}

// フラッシュ処理
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Timer::OnFlush( S3DScene& scene, S3DScene::Space& space )
{
	return	true ;		// 処理完了
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Timer::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLObject::OnSave( file ) ;
	uint32_t	nFlags = 0 ;
	if ( m_flagAutoDelete )
	{
		nFlags |= 0x01 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Timer::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLObject::OnSave( file ) ;
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	m_flagAutoDelete = ((nFlags & 0x01) != 0) ;
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// 3D シーン・基底アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::Animation, Timer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Animation::Animation
		( bool flagAutoDelete, S3DScene::SpaceInfo * psiTarget )
	: Timer( flagAutoDelete ),
		m_psiTarget( psiTarget ),
		m_msecDuration(0), m_msecCurrent(0),
		m_typeLoop( loopGo ), m_nLoopCount( 1 )
{
}

// 継続時間
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Animation::SetDuration( uint32_t msecDuration )
{
	m_keyDurations.RemoveAll() ;
	m_msecDuration = msecDuration ;
}

void S3DScene::Animation::SetDurations( const uint32_t * pDurations, size_t nDivision )
{
	m_keyDurations.RemoveAll() ;
	m_keyDurations.AddArray( pDurations, nDivision ) ;
	m_msecDuration = 0 ;
	for ( size_t i = 0; i < nDivision; i ++ )
	{
		m_msecDuration += pDurations[i] ;
	}
}

// ループ回数
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Animation::SetLoop( uint32_t nLoop, LoopType typeLoop )
{
	m_typeLoop = typeLoop ;
	m_nLoopCount = nLoop ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Animation::OnTimer
	( S3DScene& scene, S3DScene::Space& space, uint32_t msecPast )
{
	SpaceInfo *	psiTarget = m_psiTarget ;
	if ( psiTarget == nullptr )
	{
		psiTarget = &space ;
	}
	if ( m_msecDuration != 0 )
	{
		m_msecCurrent += msecPast ;
		while ( (m_nLoopCount >= 1) && (m_msecCurrent >= m_msecDuration) )
		{
			if ( m_nLoopCount != 0xFFFFFFFF )
			{
				m_msecCurrent -= m_msecDuration ;
				m_nLoopCount -- ;
			}
			else
			{
				m_msecCurrent %= m_msecDuration ;
			}
		}
	}
	if ( (m_nLoopCount == 0) || (m_msecDuration == 0) )
	{
		if ( m_typeLoop == loopGo )
		{
			OnFrame( scene, *psiTarget, 1.0 ) ;
		}
		else
		{
			OnFrame( scene, *psiTarget, 0.0 ) ;
		}
		return	true ;
	}
	uint32_t	msecCurrent = m_msecCurrent ;
	if ( m_typeLoop == loopGoBack )
	{
		if ( msecCurrent > (m_msecDuration >> 1) )
		{
			if ( msecCurrent < m_msecDuration )
			{
				msecCurrent = (m_msecDuration - msecCurrent) * 2 ;
			}
			else
			{
				msecCurrent = 0 ;
			}
		}
		else
		{
			msecCurrent *= 2 ;
		}
	}
	size_t		nKeyCount = m_keyDurations.GetLength() ;
	if ( nKeyCount > 0 )
	{
		const uint32_t *	pKeyDuration = m_keyDurations.GetConstArray() ;
		for ( size_t i = 0; i < nKeyCount; i ++ )
		{
			uint32_t	nKeyDuration = pKeyDuration[i] ;
			if ( msecCurrent < nKeyDuration )
			{
				OnFrame( scene, *psiTarget,
						(double) i / nKeyCount
							+ (double) msecCurrent
										/ (nKeyDuration * nKeyCount) ) ;
				return	false ;
			}
			msecCurrent -= nKeyDuration ;
		}
	}
	OnFrame( scene, *psiTarget, (double) msecCurrent / m_msecDuration ) ;
	return	false ;
}

// フラッシュ処理
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Animation::OnFlush( S3DScene& scene, S3DScene::Space& space )
{
	SpaceInfo *	psiTarget = m_psiTarget ;
	if ( psiTarget == nullptr )
	{
		psiTarget = &space ;
	}
	if ( m_typeLoop == loopGo )
	{
		OnFrame( scene, *psiTarget, 1.0 ) ;
	}
	else
	{
		OnFrame( scene, *psiTarget, 0.0 ) ;
	}
	return	true ;
}

// アニメーションフレーム処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Animation::OnFrame
	( S3DScene& scene, S3DScene::SpaceInfo& space, double t )
{
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Animation::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Timer::OnSave( file ) ;
	SString		strID ;
	uint32_t	nOffset ;
	if ( m_psiTarget != nullptr )
	{
		SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
		if ( posm == nullptr )
		{
			return	sglErrFailed ;
		}
		Item *	pItem =
			(Item*) (((uint8_t*)m_psiTarget) - offsetof(Item,m_space)) ;
		strID = posm->GetIdentityOf( pItem ) ;
		if ( !strID.IsEmpty() )
		{
			nOffset = offsetof(Item,m_space) ;
		}
	}
	file.WriteString( strID ) ;
	file.Write( &nOffset, sizeof(uint32_t) ) ;
	//
	uint32_t	nCount = (uint32_t) m_keyDurations.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	file.Write( m_keyDurations.GetConstArray(), nCount * sizeof(uint32_t) ) ;
	//
	file.Write( &m_msecDuration, sizeof(uint32_t) ) ;
	file.Write( &m_msecCurrent, sizeof(uint32_t) ) ;
	file.Write( &m_typeLoop, sizeof(uint32_t) ) ;
	file.Write( &m_nLoopCount, sizeof(uint32_t) ) ;
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Animation::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Timer::OnRestore( file ) ;
	SString		strID ;
	uint32_t	nOffset ;
	file.ReadString( strID ) ;
	file.Read( &nOffset, sizeof(uint32_t) ) ;
	if ( !strID.IsEmpty() )
	{
		SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
		if ( posm == nullptr )
		{
			return	sglErrFailed ;
		}
		ESLObject *	pObj = posm->GetObjectOf( strID ) ;
		if ( pObj != nullptr )
		{
			m_psiTarget = (SpaceInfo*) (((uint8_t*)pObj) + nOffset) ;
		}
	}
	//
	uint32_t	nCount ;
	file.Read( &nCount, sizeof(uint32_t) ) ;
	m_keyDurations.SetLength( (size_t) nCount ) ;
	file.Read( m_keyDurations.GetArray(), nCount * sizeof(uint32_t) ) ;
	m_keyDurations.FinishArray() ;
	//
	file.Read( &m_msecDuration, sizeof(uint32_t) ) ;
	file.Read( &m_msecCurrent, sizeof(uint32_t) ) ;
	file.Read( &m_typeLoop, sizeof(uint32_t) ) ;
	file.Read( &m_nLoopCount, sizeof(uint32_t) ) ;
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// 回転行列アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::RotationBezier, Animation )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::RotationBezier::RotationBezier
		( bool flagAutoDelete, S3DScene::SpaceInfo * psiTarget )
	: Animation( flagAutoDelete, psiTarget )
{
}

// 回転設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RotationBezier::SetMatrixTo
	( const S3DScene::SpaceInfo& space,
		const S3DDMatrix& mat, double a0, double a1 )
{
	S3DDQuaternion	q0, q1 ;
	q0.FromMatrix( space.m_matTransformation ) ;
	q1.FromMatrix( mat ) ;
	m_bezier.SetLine( q0, q1, a0, a1 ) ;
}

void S3DScene::RotationBezier::SetMatrixTo
	( const S3DScene::SpaceInfo& space,
		const S3DMatrix& mat, double a0, double a1 )
{
	S3DDQuaternion	q0, q1 ;
	S3DDMatrix		matd = mat ;
	q0.FromMatrix( space.m_matTransformation ) ;
	q1.FromMatrix( matd ) ;
	m_bezier.SetLine( q0, q1, a0, a1 ) ;
}

// アニメーションフレーム処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RotationBezier::OnFrame
	( S3DScene& scene, S3DScene::SpaceInfo& space, double t )
{
	S3DDQuaternion	qt ;
	m_bezier.PointAt( qt, t ) ;
	qt.ToMatrix( space.m_matTransformation ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::RotationBezier::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnSave( file ) ;
	//
	uint32_t	nLength = (uint32_t) m_bezier.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bezier.GetArray(), nLength * sizeof(S3DDQuaternion) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::RotationBezier::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnRestore( file ) ;
	//
	uint32_t	nLength ;
	file.Read( &nLength, sizeof(uint32_t) ) ;
	m_bezier.SetLength( (size_t) nLength ) ;
	file.Read( m_bezier.GetArray(), nLength * sizeof(S3DDQuaternion) ) ;
	m_bezier.FinishArray() ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// 回転角アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::AngleBezier, Animation )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::AngleBezier::AngleBezier
		( bool flagAutoDelete, S3DScene::SpaceInfo * psiTarget )
	: Animation( flagAutoDelete, psiTarget )
{
}

// 回転設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AngleBezier::SetAngleTo
	( const S3DScene::SpaceInfo& space,
		double x, double y, double z, double a0, double a1 )
{
	m_matBase = space.m_matTransformation ;
	m_bezier.SetLine( S3DVector( 0, 0, 0 ), S3DVector( x, y, z ), a0, a1 ) ;
}

// アニメーションフレーム処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AngleBezier::OnFrame
	( S3DScene& scene, S3DScene::SpaceInfo& space, double t )
{
	S3DVector	v ;
	m_bezier.PointAt( v, t ) ;
	//
	S3DDVector	vI( 1, 1, 1 ) ;
	space.m_matTransformation.InitializeMatrix( vI ) ;
	//
	double	rad ;
	rad = v.z * PI / 180.0 ;
	space.m_matTransformation.RevolveOnZ( sin(rad), cos(rad) ) ;
	//
	rad = v.x * PI / 180.0 ;
	space.m_matTransformation.RevolveOnX( sin(rad), cos(rad) ) ;
	//
	rad = v.y * PI / 180.0 ;
	space.m_matTransformation.RevolveOnY( sin(rad), cos(rad) ) ;
	//
	space.m_matTransformation *= m_matBase ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::AngleBezier::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnSave( file ) ;
	//
	file.Write( &m_matBase, sizeof(S3DDMatrix) ) ;
	//
	uint32_t	nLength = (uint32_t) m_bezier.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bezier.GetConstArray(), nLength * sizeof(S3DVector) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::AngleBezier::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnRestore( file ) ;
	//
	file.Read( &m_matBase, sizeof(S3DDMatrix) ) ;
	//
	uint32_t	nLength ;
	file.Read( &nLength, sizeof(uint32_t) ) ;
	m_bezier.SetLength( (size_t) nLength ) ;
	file.Read( m_bezier.GetArray(), nLength * sizeof(S3DVector) ) ;
	m_bezier.FinishArray() ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// 拡大アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::ZoomBezier, Animation )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::ZoomBezier::ZoomBezier
		( bool flagAutoDelete, SpaceInfo * psiTarget )
	: Animation( flagAutoDelete, psiTarget )
{
}

// 回転設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ZoomBezier::SetZoom
	( const S3DScene::SpaceInfo& space,
		const S3DVector & vStart, const S3DVector & vEnd, double a0, double a1 )
{
	m_matBase = space.m_matTransformation ;
	m_bezier.SetLine( vStart, vEnd, a0, a1 ) ;
}

// アニメーションフレーム処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ZoomBezier::OnFrame
	( S3DScene& scene, S3DScene::SpaceInfo& space, double t )
{
	S3DVector	v ;
	m_bezier.PointAt( v, t ) ;
	//
	space.m_matTransformation.InitializeMatrix( S3DDVector( v ) ) ;
	space.m_matTransformation *= m_matBase ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::ZoomBezier::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnSave( file ) ;
	//
	file.Write( &m_matBase, sizeof(S3DDMatrix) ) ;
	//
	uint32_t	nLength = (uint32_t) m_bezier.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bezier.GetConstArray(), nLength * sizeof(S3DVector) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::ZoomBezier::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnRestore( file ) ;
	//
	file.Read( &m_matBase, sizeof(S3DDMatrix) ) ;
	//
	uint32_t	nLength ;
	file.Read( &nLength, sizeof(uint32_t) ) ;
	m_bezier.SetLength( (size_t) nLength ) ;
	file.Read( m_bezier.GetArray(), nLength * sizeof(S3DVector) ) ;
	m_bezier.FinishArray() ;
	//
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// 平行移動アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::MoveBezier, Animation )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::MoveBezier::MoveBezier
		( bool flagAutoDelete, S3DScene::SpaceInfo * psiTarget )
	: Animation( flagAutoDelete, psiTarget )
{
}

// 回転設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::MoveBezier::SetMoveTo
	( const S3DScene::SpaceInfo& space,
		double x, double y, double z, double a0, double a1 )
{
	S3DDVector	v1( x, y, z ) ;
	m_bezier.SetLine( space.m_vCenter, v1, a0, a1 ) ;
}

void S3DScene::MoveBezier::SetMoveTo
	( const S3DScene::SpaceInfo& space,
		const S3DDVector& v, double a0, double a1 )
{
	m_bezier.SetLine( space.m_vCenter, v, a0, a1 ) ;
}

// アニメーションフレーム処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::MoveBezier::OnFrame
	( S3DScene& scene, S3DScene::SpaceInfo& space, double t )
{
	m_bezier.PointAt( space.m_vCenter, t ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::MoveBezier::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnSave( file ) ;
	//
	uint32_t	nLength = (uint32_t) m_bezier.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bezier.GetConstArray(), nLength * sizeof(S3DDVector) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::MoveBezier::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnRestore( file ) ;
	//
	uint32_t	nLength ;
	file.Read( &nLength, sizeof(uint32_t) ) ;
	m_bezier.SetLength( (size_t) nLength ) ;
	file.Read( m_bezier.GetArray(), nLength * sizeof(S3DDVector) ) ;
	m_bezier.FinishArray() ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// 透明度アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::TransparencyBezier, Animation )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::TransparencyBezier::TransparencyBezier
		( bool flagAutoDelete, S3DScene::SpaceInfo * psiTarget )
	: Animation( flagAutoDelete, psiTarget )
{
}

// 回転設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::TransparencyBezier::SetTransparencyTo
	( const S3DScene::SpaceInfo& space,
		unsigned int nTransparency, double a0, double a1 )
{
	m_bezier.SetLine
		( (double) space.m_nTransparency, (double) nTransparency, a0, a1 ) ;
}

void S3DScene::TransparencyBezier::SetAlphaTo
	( const S3DScene::SpaceInfo& space, double alpha, double a0, double a1 )
{
	m_bezier.SetLine
		( (double) space.m_nTransparency, (1.0 - alpha) * 0x100, a0, a1 ) ;
}

// アニメーションフレーム処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::TransparencyBezier::OnFrame
	( S3DScene& scene, S3DScene::SpaceInfo& space, double t )
{
	int	n = eslRoundR32ToInt( (float32_t) m_bezier.PointAt(t) ) ;
	if ( n < 0 )
	{
		n = 0 ;
	}
	else if ( n > 0x100 )
	{
		n = 0x100 ;
	}
	space.m_nTransparency = (unsigned int) n ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::TransparencyBezier::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnSave( file ) ;
	//
	uint32_t	nLength = (uint32_t) m_bezier.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bezier.GetConstArray(), nLength * sizeof(double) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::TransparencyBezier::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnRestore( file ) ;
	//
	uint32_t	nLength ;
	file.Read( &nLength, sizeof(uint32_t) ) ;
	m_bezier.SetLength( (size_t) nLength ) ;
	file.Read( m_bezier.GetArray(), nLength * sizeof(double) ) ;
	m_bezier.FinishArray() ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// 色効果アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::ColorBezier, Animation )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::ColorBezier::ColorBezier
		( bool flagAutoDelete, S3DScene::SpaceInfo * psiTarget )
	: Animation( flagAutoDelete, psiTarget )
{
}

// 回転設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ColorBezier::SetColorTo
	( const S3DScene::SpaceInfo& space,
		const S3DColor& color, double a0, double a1 )
{
	S3DVector	vMul0
		( space.m_colorEffect.rgbMul.argb.Red,
			space.m_colorEffect.rgbMul.argb.Green,
			space.m_colorEffect.rgbMul.argb.Blue ) ;
	S3DVector	vAdd0
		( space.m_colorEffect.rgbAdd.argb.Red,
			space.m_colorEffect.rgbAdd.argb.Green,
			space.m_colorEffect.rgbAdd.argb.Blue ) ;
	S3DVector	vMul1
		( color.rgbMul.argb.Red,
			color.rgbMul.argb.Green,
			color.rgbMul.argb.Blue ) ;
	S3DVector	vAdd1
		( color.rgbAdd.argb.Red,
			color.rgbAdd.argb.Green,
			color.rgbAdd.argb.Blue ) ;
	m_bzMul.SetLine( vMul0, vMul1, a0, a1 ) ;
	m_bzAdd.SetLine( vAdd0, vAdd1, a0, a1 ) ;
}

// アニメーションフレーム処理
//////////////////////////////////////////////////////////////////////////////
static uint8_t ClampToUint8( int n )
{
	if ( ((unsigned int) n) >= 0x100 )
	{
		return	(uint8_t) ~(n >> 31) & 0xFF ;
	}
	return	(uint8_t) n ;
}

void S3DScene::ColorBezier::OnFrame
	( S3DScene& scene, S3DScene::SpaceInfo& space, double t )
{
	S3DVector	vMul, vAdd ;
	m_bzMul.PointAt( vMul, t ) ;
	m_bzAdd.PointAt( vAdd, t ) ;
	space.m_colorEffect.rgbMul.argb.Red   = ClampToUint8( eslRoundR32ToInt( vMul.x ) ) ;
	space.m_colorEffect.rgbMul.argb.Green = ClampToUint8( eslRoundR32ToInt( vMul.y ) ) ;
	space.m_colorEffect.rgbMul.argb.Blue  = ClampToUint8( eslRoundR32ToInt( vMul.z ) ) ;
	space.m_colorEffect.rgbAdd.argb.Red   = ClampToUint8( eslRoundR32ToInt( vAdd.x ) ) ;
	space.m_colorEffect.rgbAdd.argb.Green = ClampToUint8( eslRoundR32ToInt( vAdd.y ) ) ;
	space.m_colorEffect.rgbAdd.argb.Blue  = ClampToUint8( eslRoundR32ToInt( vAdd.z ) ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::ColorBezier::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnSave( file ) ;
	//
	uint32_t	nLength = (uint32_t) m_bzMul.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bzMul.GetConstArray(), nLength * sizeof(S3DVector) ) ;
	//
	nLength = (uint32_t) m_bzAdd.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bzMul.GetConstArray(), nLength * sizeof(S3DVector) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::ColorBezier::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Animation::OnRestore( file ) ;
	//
	uint32_t	nLength ;
	file.Read( &nLength, sizeof(uint32_t) ) ;
	m_bzMul.SetLength( (size_t) nLength ) ;
	file.Read( m_bzMul.GetArray(), nLength * sizeof(S3DVector) ) ;
	m_bzMul.FinishArray() ;
	//
	file.Read( &nLength, sizeof(uint32_t) ) ;
	m_bzAdd.SetLength( (size_t) nLength ) ;
	file.Read( m_bzAdd.GetArray(), nLength * sizeof(S3DVector) ) ;
	m_bzAdd.FinishArray() ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// レイヤー描画パラメータアニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::LayeredParamBezier, Timer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::LayeredParamBezier::LayeredParamBezier( bool flagAutoDelete )
	: Timer( flagAutoDelete )
{
	m_msecDuration = 0 ;
	m_msecCurrent = 0 ;
}

// 継続時間
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredParamBezier::SetDuration( uint32_t msecDuration )
{
	m_msecDuration = msecDuration ;
}

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredParamBezier::SetParameterTo
	( const S3DScene::Space& space, float32_t fpParam, double a0, double a1 )
{
	m_bezier.SetLine
		( (double) space.GetLayeredDrawParameter(),
			(double) fpParam, a0, a1 ) ;
}

// 透明度設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredParamBezier::SetTransparencyTo
	( const S3DScene::Space& space, unsigned int nTransparency, double a0, double a1 )
{
	m_bezier.SetLine
		( (double) space.GetLayeredDrawParameter(),
			(double) nTransparency / 256.0, a0, a1 ) ;
}

void S3DScene::LayeredParamBezier::SetAlphaTo
	( const S3DScene::Space& space, double alpha, double a0, double a1 )
{
	m_bezier.SetLine
		( (double) space.GetLayeredDrawParameter(),
			1.0 - alpha, a0, a1 ) ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::LayeredParamBezier::OnTimer
	( S3DScene& scene, S3DScene::Space& space, uint32_t msecPast )
{
	double	t = 1.0 ;
	m_msecCurrent += msecPast ;
	if ( m_msecCurrent < m_msecDuration )
	{
		t = (double) m_msecCurrent / m_msecDuration ;
	}
	double	p = m_bezier.PointAt( t ) ;
	space.SetLayeredDrawParameter( (float32_t) p ) ;
	return	(m_msecCurrent >= m_msecDuration) ;
}

// フラッシュ処理
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::LayeredParamBezier::OnFlush( S3DScene& scene, S3DScene::Space& space )
{
	double	p = m_bezier.PointAt( 1.0 ) ;
	space.SetLayeredDrawParameter( (float32_t) p ) ;
	return	true ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::LayeredParamBezier::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Timer::OnSave( file ) ;
	//
	file.Write( &m_msecDuration, sizeof(uint32_t) ) ;
	file.Write( &m_msecCurrent, sizeof(uint32_t) ) ;
	//
	uint32_t	nLength = (uint32_t) m_bezier.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write( m_bezier.GetConstArray(), nLength * sizeof(double) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::LayeredParamBezier::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Timer::OnRestore( file ) ;
	//
	file.Read( &m_msecDuration, sizeof(uint32_t) ) ;
	file.Read( &m_msecCurrent, sizeof(uint32_t) ) ;
	//
	uint32_t	nLength = 0 ;
	file.Read( &nLength, sizeof(uint32_t) ) ;
	file.Read( m_bezier.GetArray(nLength), nLength * sizeof(double) ) ;
	m_bezier.FinishArray() ;
	//
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// ローカルシェーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( S3DScene::LocalShader, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::LocalShader::LocalShader( void )
{
	m_pLocalShader = nullptr ;
}

S3DScene::LocalShader::LocalShader( const LocalShader& src )
	: m_pLocalShader( src.m_pLocalShader ),
		m_uniforms( src.m_uniforms )

{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::LocalShader::~LocalShader( void )
{
}

// カスタムシェーダー設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LocalShader::SetCustomShader( S3DCustomShader * pShader )
{
	m_pLocalShader = pShader ;
}

// カスタムシェーダー取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * S3DScene::LocalShader::GetCustomShader( void ) const
{
	return	m_pLocalShader ;
}

// カスタムシェーダーパラメータ
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader::UniformSet&
	S3DScene::LocalShader::ShaderUniformSet( void )
{
	return	m_uniforms ;
}

S3DCustomShader::UniformData *
	S3DScene::LocalShader::GetShaderUniformAs( const wchar_t * pwszName ) const
{
	return	m_uniforms.GetAs( pwszName ) ;
}

void S3DScene::LocalShader::SetShaderUniformDataAs
	( const wchar_t * pwszName,
		S3DCustomShader::UniformType type,
		const void * pData, size_t nLength )
{
	S3DCustomShader::UniformData *	pUniform = m_uniforms.GetAs( pwszName ) ;
	if ( pUniform != nullptr )
	{
		pUniform->SetData( type, pData, nLength ) ;
	}
	else
	{
		pUniform = new S3DCustomShader::UniformData ;
		pUniform->SetData( type, pData, nLength ) ;
		m_uniforms.SetAs( pwszName, pUniform ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 3D シーン・基底アイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::Item, Object )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Item::Item( void )
	: m_matLink( 1, 0, 0,  0, 1, 0,  0, 0, 1 ), m_vLink( 0, 0, 0 )
{
	m_pListener = nullptr ;
	m_pwszID = nullptr ;
	m_classItem = classStaticItem1 ;
	m_nRenderPriority = 4 ;
	m_flagsBehavior = itemVisible ;
	m_maskClasses = 0 ;
}

S3DScene::Item::Item( const S3DScene::Item& src )
	: m_refSpace( src.m_refSpace ),
		m_space( src.m_space ),
		m_matLink( 1, 0, 0,  0, 1, 0,  0, 0, 1 ), m_vLink( 0, 0, 0 ),
		m_pListener( src.m_pListener ),
		m_pwszID( src.m_pwszID ),
		m_classItem( src.m_classItem ),
		m_nRenderPriority( src.m_nRenderPriority ),
		m_flagsBehavior( src.m_flagsBehavior ),
		m_maskClasses( src.m_maskClasses )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Item::~Item( void )
{
}

// 空間コンテナ取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::Space * S3DScene::Item::GetReferenceSpace( void ) const
{
	return	ESLTypeCast<S3DScene::Space>( m_refSpace.GetReference() ) ;
}

S3DScene::Space * S3DScene::Item::GetParentSpace( void ) const
{
	return	ESLTypeCast<S3DScene::Space>( m_refParentSpace.GetReference() ) ;
}

// 親シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DScene::Item::GetScene( void ) const
{
	S3DScene::Space *	pSpace = GetParentSpace() ;
	if ( pSpace != nullptr )
	{
		return	pSpace->GetScene() ;
	}
	return	nullptr ;
}

// ローカル変換行列を取得
//////////////////////////////////////////////////////////////////////////////
const S3DDMatrix&
		S3DScene::Item::GetLocalTransformation( S3DDMatrix& matLocal ) const
{
	return	m_space.m_matTransformation ;
}

// ローカル変換行列を設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::SetLocalTransformation( const S3DDMatrix& matLocal )
{
	m_space.m_matTransformation = matLocal ;
	m_space.m_flagsModified |= spaceElementTransformation ;
}

// ローカル座標を取得
//////////////////////////////////////////////////////////////////////////////
const S3DDVector&
		S3DScene::Item::GetLocalItemPosition( S3DDVector& vLocal ) const
{
	return	m_space.m_vCenter ;
}

// ローカル変換行列を設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::SetLocalItemPosition( const S3DDVector& vLocal )
{
	m_space.m_vCenter = vLocal ;
	m_space.m_flagsModified |= spaceElementPosition ;
}

// アイテム空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::CalcItemLinkTransformation
			( S3DDMatrix& matrix, S3DDVector& pos ) const
{
	S3DScene::Space *	pSpace = GetReferenceSpace() ;
	if ( (pSpace != nullptr) && !(m_flagsBehavior & itemGlobalSpace) )
	{
		pSpace->CalcGlobalTransformation( matrix, pos ) ;
		//
		pos += matrix * m_vLink ;
		matrix *= m_matLink ;
	}
	else
	{
		matrix = S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		pos = S3DDVector( 0, 0, 0 ) ;
	}
}

// グローバル空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::CalcGlobalTransformation
		( S3DDMatrix& matrix, S3DDVector& pos ) const
{
	S3DScene::Space *	pSpace = GetReferenceSpace() ;
	if ( (pSpace != nullptr)
		&& !(m_flagsBehavior & (itemGlobalSpace
								| itemCameraShift | itemCameraSpace)) )
	{
		CalcItemLinkTransformation( matrix, pos ) ;
		pos += matrix * m_space.m_vCenter ;
		matrix *= m_space.m_matTransformation ;
	}
	else
	{
		matrix = m_space.m_matTransformation ;
		pos = m_space.m_vCenter ;
	}
}

// グローバル色効果取得（乗算色α要素に不透明度取得）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::CalcGlobalColorEffect( S3DColor& clrEffect ) const
{
	clrEffect = m_space.m_colorEffect ;
	clrEffect.rgbMul.argb.Alpha =
		(uint8_t) esl_clampi( 0xFF - m_space.m_nTransparency, 0, 0xFF ) ;
	//
	S3DScene::Space *	pSpace = GetReferenceSpace() ;
	if ( pSpace != nullptr )
	{
		S3DColor	clrSpace ;
		pSpace->CalcGlobalColorEffect( clrSpace ) ;
		//
		clrEffect = clrSpace * clrEffect ;
	}
}

// 相対空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::CalcOffsetTransformation
	( S3DDMatrix& matrix, S3DDVector& pos,
					const S3DScene::Space * pStandardSpace ) const
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

void S3DScene::Item::CalcOffsetTransformation
	( S3DDMatrix& matrix, S3DDVector& pos,
			const S3DDMatrix& matStandard,
			const S3DDVector& vStandard ) const
{
	CalcGlobalTransformation( matrix, pos ) ;
	//
	S3DDMatrix	matIStandard ;
	matIStandard.InverseOf( matStandard ) ;
	//
	pos -= vStandard ;
	matIStandard.RevolveVector( pos ) ;
	matrix = matIStandard * matrix ;
}

// 参照空間を変更する（現在の座標・姿勢を維持）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::SwitchReferenceSpace( S3DScene::Space * pRefSpace )
{
	if ( GetReferenceSpace() != pRefSpace )
	{
		// 以前の参照空間 : Ref0
		// 以前の接続変換 : Link0
		// 新しい参照空間 : Ref1
		// 新しい接続変換 : Link1
		// Ref0 * Link0 * Local = Ref1 * Link1 * Local
		// Link1 = Ref1^-1 * Ref0 * Link0
		//
		S3DDMatrix	matLink0 ;
		S3DDVector	vLink0 ;
		CalcItemLinkTransformation( matLink0, vLink0 ) ;
		//
		if ( pRefSpace == nullptr )
		{
			pRefSpace = ESLTypeCast<S3DScene::Space>
							( m_refParentSpace.GetReference() ) ;
		}
		if ( pRefSpace != nullptr )
		{
			S3DDMatrix	matRef1 ;
			S3DDVector	vRef1 ;
			pRefSpace->CalcGlobalTransformation( matRef1, vRef1 ) ;
			//
			S3DDMatrix	matIRef1 ;
			matIRef1.InverseOf( matRef1 ) ;
			//
			vLink0 = matIRef1 * vLink0 - vRef1 ;
			matLink0 = matIRef1 * matLink0 ;
		}
		//
		m_refSpace.SetReference( pRefSpace ) ;
		m_matLink = matLink0 ;
		m_vLink = vLink0 ;
	}
}

// 参照空間を変更する（参照空間に対して正則）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::SetReferenceSpace( S3DScene::Space * pRefSpace )
{
	if ( pRefSpace == nullptr )
	{
		pRefSpace = ESLTypeCast<S3DScene::Space>
						( m_refParentSpace.GetReference() ) ;
	}
	m_refSpace.SetReference( pRefSpace ) ;
	m_matLink.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	m_vLink = S3DDVector( 0, 0, 0 ) ;
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
S3DScene::ItemEventListener *
	S3DScene::Item::AttachItemListener
		( S3DScene::ItemEventListener * pListener )
{
	ItemEventListener *	pLastListener = m_pListener ;
	m_pListener = pListener ;
	return	pLastListener ;
}

S3DScene::ItemEventListener * S3DScene::Item::GetItemListener( void ) const
{
	return	m_pListener ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::OnTimer( S3DScene& scene, uint32_t msecPast )
{
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::OnUpdateBehavior( S3DScene& scene )
{
}

// 規定のレンダリングデバイス設定通知
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::SetRenderDevice( S3DRenderDevice * pDevice )
{
	if ( m_pListener != nullptr )
	{
		m_pListener->OnSetRenderDevice( pDevice ) ;
	}
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::PrepareToRender
			( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	if ( m_pListener != nullptr )
	{
		m_pListener->OnPrepareToRender( pDevice, nFlags ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::OnRenderEvent
		( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( m_pListener != nullptr )
	{
		m_pListener->OnItemRenderEvent( scene, clsItem ) ;
	}
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::RenderCollision
		( const S3DScene& scene, S3DCollision& render )
{
	if ( m_flagsBehavior & itemCollision )
	{
		render.PushTransformation() ;
		if ( m_flagsBehavior
			& (itemGlobalSpace | itemCameraShift | itemCameraSpace) )
		{
			S3DDMatrix	matSpace = m_space.m_matTransformation ;
			S3DDVector	vPos = m_space.m_vCenter ;
			if ( m_flagsBehavior & itemCameraSpace )
			{
				vPos = scene.GetCurrentCameraIMatrix() * vPos ;
				matSpace = scene.GetCurrentCameraIMatrix() * matSpace ;
			}
			if ( m_flagsBehavior & (itemCameraShift | itemCameraSpace) )
			{
				vPos += scene.GetCurrentCameraPosition() ;
			}
			vPos += scene.GetRenderingOffset() ;
			render.SetMatrixTransformation( matSpace, vPos ) ;
		}
		else
		{
			render.AppendMatrixTransformation
				( m_space.m_matTransformation, m_space.m_vCenter ) ;
		}
		RenderLocalCollision( scene, render ) ;
		if ( m_pListener != nullptr )
		{
			m_pListener->OnItemRenderCollision( scene, render ) ;
		}
		render.PopTransformation() ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::RenderModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( m_flagsBehavior & itemVisible )
	{
		render.PushTransformation() ;
		if ( m_flagsBehavior
			& (itemGlobalSpace | itemCameraShift | itemCameraSpace) )
		{
			S3DDMatrix	matSpace = m_space.m_matTransformation ;
			S3DDVector	vPos = m_space.m_vCenter ;
			if ( m_flagsBehavior & itemCameraSpace )
			{
				vPos = scene.GetCurrentCameraIMatrix() * vPos ;
				matSpace = scene.GetCurrentCameraIMatrix() * matSpace ;
			}
			if ( m_flagsBehavior & (itemCameraShift | itemCameraSpace) )
			{
				vPos += scene.GetCurrentCameraPosition() ;
			}
			vPos += scene.GetRenderingOffset() ;
			render.SetMatrixTransformation
				( matSpace, vPos,
					&(m_space.m_colorEffect), m_space.m_nTransparency ) ;
		}
		else
		{
			render.AppendMatrixTransformation
				( m_space.m_matTransformation, m_space.m_vCenter,
					&(m_space.m_colorEffect), m_space.m_nTransparency ) ;
			//
			if ( m_flagsBehavior & itemHideNear )
			{
				if ( scene.IsAheadNearHiddenDistance( render ) )
				{
					render.PopTransformation() ;
					return ;
				}
			}
			if ( m_flagsBehavior & itemHideFar )
			{
				if ( scene.IsBehindFarHiddenDistance( render ) )
				{
					render.PopTransformation() ;
					return ;
				}
			}
		}
		render.SetOptionalFeature
			( RenderContext::featureOrderPriority,
							m_nRenderPriority, nullptr, 0 ) ;
		if ( m_pListener != nullptr )
		{
			m_pListener->BeforeItemRenderModel
						( scene, render, flagsExclusion ) ;
		}
		RenderLocalModel( scene, render, flagsExclusion ) ;
		if ( m_pListener != nullptr )
		{
			m_pListener->AfterItemRenderModel
						( scene, render, flagsExclusion ) ;
		}
		render.PopTransformation() ;
	}
}

// 当たり判定追加（ローカル空間）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::RenderLocalCollision
	( const S3DScene& scene, S3DCollision& render )
{
}

// 表示モデル追加（ローカル空間）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( m_pListener != nullptr )
	{
		m_pListener->OnItemRenderModel( scene, render, flagsExclusion ) ;
	}
}

// レンダリングスレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SSystem::SError S3DScene::Item::Lock( int64_t msecTimeout ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene != nullptr )
	{
		return	pScene->Lock( msecTimeout ) ;
	}
	else
	{
		return	SSystem::Lock( msecTimeout ) ;
	}
}

SSystem::SError S3DScene::Item::Unlock( void ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene != nullptr )
	{
		return	pScene->Unlock() ;
	}
	else
	{
		return	SSystem::Unlock() ;
	}
}

atomic_int_t S3DScene::Item::TestLocked( void ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene != nullptr )
	{
		return	pScene->TestLocked() ;
	}
	else
	{
		return	SSystem::TestLocked() ;
	}
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Item::OnSave( SSystem::SFileInterface& file )
{
	file.Write( &m_space, sizeof(SpaceInfo) ) ;
	uint32_t	classItem = m_classItem ;
	file.Write( &classItem, sizeof(uint32_t) ) ;
	file.Write( &m_flagsBehavior, sizeof(uint32_t) ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Item::OnRestore( SSystem::SFileInterface& file )
{
	file.Read( &m_space, sizeof(SpaceInfo) ) ;
	uint32_t	classItem ;
	file.Read( &classItem, sizeof(uint32_t) ) ;
	m_classItem = (ItemClass) classItem ;
	file.Read( &m_flagsBehavior, sizeof(uint32_t) ) ;
	return	sglErrSuccess ;
}

// 関連付けられた物理演算オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DPhysicsScene::Actor * S3DScene::Item::GetPhysicalActor( void ) const
{
	return	nullptr ;
}

// オーナーオブジェクト取得
//////////////////////////////////////////////////////////////////////////////
ESLObject * S3DScene::Item::GetOwnerObject( void ) const
{
	return	const_cast<S3DScene::Item*>( this ) ;
}

// 座標取得
//////////////////////////////////////////////////////////////////////////////
const S3DDVector&
		S3DScene::Item::GetPhysicsPosition( S3DDVector& vPos ) const
{
	S3DDMatrix	matrix ;
	CalcGlobalTransformation( matrix, vPos ) ;
	return	vPos ;
}

// 座標設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::SetPhysicsPosition( const S3DDVector& vPos )
{
	S3DScene::Space *	pSpace = GetReferenceSpace() ;
	if ( (pSpace != nullptr) && !(m_flagsBehavior & itemGlobalSpace) )
	{
		S3DDMatrix	matrix ;
		S3DDVector	pos ;
		pSpace->CalcGlobalTransformation( matrix, pos ) ;
		//
		pos += matrix * m_vLink ;
		matrix *= m_matLink ;
		//
		S3DDMatrix	matISpace ;
		matISpace.InverseOf( matrix ) ;
		m_space.m_vCenter = matISpace * (vPos - pos) ;
	}
	else
	{
		m_space.m_vCenter = vPos ;
	}
}

// 姿勢取得
//////////////////////////////////////////////////////////////////////////////
const S3DQuaternion&
		S3DScene::Item::GetPhysicsPosture( S3DQuaternion& qPosture ) const
{
	S3DDMatrix	matrix ;
	S3DDVector	pos ;
	CalcGlobalTransformation( matrix, pos ) ;
	//
	S3DMatrix	mat = matrix ;
	qPosture.FromMatrix( mat ) ;
	return	qPosture ;
}

// 姿勢設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::SetPhysicsPosture( const S3DQuaternion& qPosture )
{
	S3DDQuaternion	qd = qPosture ;
	qd.ToMatrix( m_space.m_matTransformation ) ;
	//
	S3DDMatrix	matrix ;
	S3DDVector	pos ;
	CalcItemLinkTransformation( matrix, pos ) ;
	//
	S3DDMatrix	matISpace ;
	matISpace.InverseOf( matrix ) ;
	matISpace *= m_space.m_matTransformation ;
	//
	m_space.m_matTransformation = matISpace ;
}

// レンダリングイベント呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::InvokeRenderSceneEventItems
	( S3DScene& scene, Space* pSpace, ItemClass clsItem )
{
	if ( pSpace->m_maskItemClasses & (1 << clsItem) )
	{
		pSpace->OnRenderEvent( scene, clsItem ) ;
	}
}

// コリジョン呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::InvokeRenderSceneCollision
	( const S3DScene& scene,
		S3DCollision& collision,
		const S3DScene::Space* pSpace, uint32_t maskClasses )
{
	if ( pSpace->m_maskItemClasses & maskClasses )
	{
		pSpace->RenderCollision( scene, collision, maskClasses ) ;
	}
}

// レンダリング呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Item::InvokeRenderSceneItems
	( const S3DScene& scene, S3DRenderContextInterface& render,
		const S3DScene::Space* pSpace,
		uint64_t flagsExclusion, uint32_t optShader )
{
	if ( pSpace->m_maskItemClasses
			& (1 << scene.GetCurrentRenderingPhase()) )
	{
		pSpace->RenderModel
			( scene, render, optShader,
				scene.GetCurrentRenderingPhase(), flagsExclusion ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 3D シーン・基底空間コンテナ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DScene::Space, SGLObject, LocalShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Space::Space( void )
	: m_matLink( 1, 0, 0,  0, 1, 0,  0, 0, 1 ), m_vLink( 0, 0, 0 )
{
	m_pwszID = nullptr ;
	m_flagsBehavior = itemVisible ;
	m_maskItemClasses = 0 ;
	m_flagsSpaceBehavior = 0 ;
	m_maskSpaceClasses = 0 ;
	m_maskLayeredClasses = 0 ;
	m_zLayeredPriority = 0 ;
	m_maskDrawSrcBuffers = 0 ;
	m_fpLayeredParam = 0 ;
	m_emsReflectionSource = envmapSourceDefault ;
	m_emsRefractionSource = envmapSourceDefault ;
	m_pListener = nullptr ;
	m_pLayerEffector = nullptr ;
	m_pLocalShader = nullptr ;
}

S3DScene::Space::Space( const S3DScene::Space& src )
: LocalShader( src ),
	SpaceInfo( src ),
	m_pwszID( src.m_pwszID ),
	m_refParent( src.m_refParent ),
	m_refSpace( src.m_refSpace ),
	m_matLink( src.m_matLink ),
	m_vLink( src.m_vLink ),
	m_flagsBehavior( src.m_flagsBehavior ),
	m_maskItemClasses( src.m_maskItemClasses ),
	m_flagsSpaceBehavior( src.m_flagsSpaceBehavior ),
	m_maskSpaceClasses( src.m_maskSpaceClasses ),
	m_maskLayeredClasses( src.m_maskLayeredClasses ),
	m_zLayeredPriority( src.m_zLayeredPriority ),
	m_maskDrawSrcBuffers( src.m_maskDrawSrcBuffers ),
	m_fpLayeredParam( src.m_fpLayeredParam ),
	m_emsReflectionSource( src.m_emsReflectionSource ),
	m_emsRefractionSource( src.m_emsRefractionSource ),
	m_pListener( src.m_pListener ),
	m_pLayerEffector( src.m_pLayerEffector ),
	m_children( src.m_children ),
	m_items( src.m_items )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Space::~Space( void )
{
	RemoveAllTimers() ;
}

// ID （ポインタの管理は呼び出し側が行う）
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DScene::Space::GetSpaceID( void ) const
{
	return	m_pwszID ;
}

void S3DScene::Space::AttachSpaceIDString( const wchar_t * pwszID )
{
	m_pwszID = pwszID ;
}

// 親空間取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::Space * S3DScene::Space::GetParentSpace( void ) const
{
	return	ESLTypeCast<Space>( m_refParent.GetReference() ) ;
}

// 親シーン取得
//////////////////////////////////////////////////////////////////////////////
S3DScene * S3DScene::Space::GetParentScene( void ) const
{
	return	ESLTypeCast<S3DScene>( m_refParent.GetReference() ) ;
}

S3DScene * S3DScene::Space::GetScene( void ) const
{
	const Space *	pSpace = this ;
	while ( pSpace != nullptr )
	{
		S3DScene *	pScene = pSpace->GetParentScene() ;
		if ( pScene != nullptr )
		{
			return	pScene ;
		}
		pSpace = pSpace->GetParentSpace() ;
	}
	return	nullptr ;
}

// 参照空間取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::Space * S3DScene::Space::GetReferenceSpace( void ) const
{
	return	ESLTypeCast<S3DScene::Space>( m_refSpace.GetReference() ) ;
}

// 親判定
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Space::IsDescendantChild( S3DScene::Space * pChild ) const
{
	if ( pChild == this )
	{
		return	false ;
	}
	Space *	pParent = pChild->GetParentSpace() ;
	while ( pParent != nullptr )
	{
		if ( pParent == this )
		{
			return	true ;
		}
		pParent = pParent->GetParentSpace() ;
	}
	return	false ;
}

// 空間変換行列（アイテム自身を含まないグローバル変換）を計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::CalcSpaceLinkTransformation
		( S3DDMatrix& matrix, S3DDVector& pos ) const
{
	S3DScene::Space *	pSpace = GetReferenceSpace() ;
	if ( (pSpace != nullptr) && !(m_flagsBehavior & itemGlobalSpace) )
	{
		pSpace->CalcGlobalTransformation( matrix, pos ) ;
		//
		pos += matrix * m_vLink ;
		matrix *= m_matLink ;
	}
	else
	{
		matrix = S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		pos = S3DDVector( 0, 0, 0 ) ;
	}
}

// グローバル空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::CalcGlobalTransformation
		( S3DDMatrix& matrix, S3DDVector& pos ) const
{
	S3DScene::Space *	pSpace = GetReferenceSpace() ;
	if ( (pSpace != nullptr)
		&& !(m_flagsBehavior & (itemGlobalSpace
								| itemCameraShift | itemCameraSpace)) )
	{
		CalcSpaceLinkTransformation( matrix, pos ) ;
		pos += matrix * m_vCenter ;
		matrix *= m_matTransformation ;
	}
	else
	{
		matrix = m_matTransformation ;
		pos = m_vCenter ;
	}
}

const S3DDVector&
	S3DScene::Space::CalcGlobalPosition( S3DDVector& pos ) const
{
	S3DDVector	vPos = m_vCenter ;
	if ( !(m_flagsBehavior & itemGlobalSpace) )
	{
		Space *	pSpace = GetReferenceSpace() ;
		while ( pSpace != nullptr )
		{
			vPos = pSpace->m_matTransformation * vPos + pSpace->m_vCenter ;
			if ( pSpace->m_flagsBehavior & itemGlobalSpace )
			{
				break ;
			}
			pSpace = pSpace->GetReferenceSpace() ;
		}
	}
	pos = vPos ;
	return	pos ;
}

// グローバル色効果取得（乗算色α要素に不透明度取得）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::CalcGlobalColorEffect( S3DColor& clrEffect ) const
{
	clrEffect = m_colorEffect ;
	clrEffect.rgbMul.argb.Alpha =
		(uint8_t) esl_clampi( 0xFF - m_nTransparency, 0, 0xFF ) ;

	S3DScene::Space *	pSpace = GetReferenceSpace() ;
	if ( pSpace != nullptr )
	{
		S3DColor	clrParent ;
		pSpace->CalcGlobalColorEffect( clrParent ) ;

		clrEffect = clrParent * clrEffect ;
	}
}

// 相対空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::CalcOffsetTransformation
	( S3DDMatrix& matrix, S3DDVector& pos,
			const S3DScene::Space * pStandardSpace ) const
{
	matrix = m_matTransformation ;
	pos = m_vCenter ;
	//
	Space *	pSpace = ESLTypeCast<Space>( m_refSpace.GetReference() ) ;
	while ( (pSpace != nullptr) && (pSpace != pStandardSpace) )
	{
		matrix = pSpace->m_matTransformation * matrix ;
		pos = pSpace->m_matTransformation * pos + pSpace->m_vCenter ;
		pSpace = ESLTypeCast<Space>( pSpace->m_refSpace.GetReference() ) ;
	}
	if ( pSpace == nullptr )
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

// 参照空間を変更する（現在の座標・姿勢を維持）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::SwitchReferenceSpace( S3DScene::Space * pRefSpace )
{
	if ( GetReferenceSpace() != pRefSpace )
	{
		// 以前の参照空間 : Ref0
		// 以前の接続変換 : Link0
		// 新しい参照空間 : Ref1
		// 新しい接続変換 : Link1
		// Ref0 * Link0 * Local = Ref1 * Link1 * Local
		// Link1 = Ref1^-1 * Ref0 * Link0
		//
		S3DDMatrix	matLink0 ;
		S3DDVector	vLink0 ;
		CalcSpaceLinkTransformation( matLink0, vLink0 ) ;
		//
		if ( pRefSpace == nullptr )
		{
			pRefSpace = ESLTypeCast<S3DScene::Space>
							( m_refParent.GetReference() ) ;
		}
		if ( pRefSpace != nullptr )
		{
			S3DDMatrix	matRef1 ;
			S3DDVector	vRef1 ;
			pRefSpace->CalcGlobalTransformation( matRef1, vRef1 ) ;
			//
			S3DDMatrix	matIRef1 ;
			matIRef1.InverseOf( matRef1 ) ;
			//
			vLink0 = matIRef1 * vLink0 - vRef1 ;
			matLink0 = matIRef1 * matLink0 ;
		}
		//
		m_refSpace.SetReference( pRefSpace ) ;
		m_matLink = matLink0 ;
		m_vLink = vLink0 ;
	}
}

// 参照空間を変更する（参照空間に対して正則）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::SetReferenceSpace( S3DScene::Space * pRefSpace )
{
	if ( pRefSpace == nullptr )
	{
		pRefSpace = ESLTypeCast<S3DScene::Space>
						( m_refParent.GetReference() ) ;
	}
	m_refSpace.SetReference( pRefSpace ) ;
	m_matLink.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	m_vLink = S3DDVector( 0, 0, 0 ) ;
}

// ローカル変換行列を取得
//////////////////////////////////////////////////////////////////////////////
const S3DDMatrix&
	S3DScene::Space::GetLocalTransformation( S3DDMatrix& matLocal ) const
{
	matLocal = m_matTransformation ;
	return	matLocal ;
}

// ローカル変換行列を設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::SetLocalTransformation( const S3DDMatrix& matLocal )
{
	m_matTransformation = matLocal ;
	m_flagsModified |= spaceElementTransformation ;
}

// ローカル座標を取得
//////////////////////////////////////////////////////////////////////////////
const S3DDVector&
		S3DScene::Space::GetLocalSpacePosition( S3DDVector& vLocal ) const
{
	vLocal = m_vCenter ;
	return	vLocal ;
}

// ローカル変換行列を設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::SetLocalSpacePosition( const S3DDVector& vLocal )
{
	m_vCenter = vLocal ;
	m_flagsModified |= spaceElementPosition ;
}

// 作用フラグ変更
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::Space::ModifyBehaviorFlags
	( uint32_t nAddFlags, uint32_t nRemoveFlags )
{
	m_flagsSpaceBehavior =
		(m_flagsSpaceBehavior | nAddFlags) & ~nRemoveFlags ;
	return	m_flagsSpaceBehavior ;
}

// 作用フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::Space::GetBehaviorFlags( void ) const
{
	return	m_flagsSpaceBehavior ;
}

// 空間クラス追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::AddSpaceClass( S3DScene::ItemClass icls )
{
	m_maskSpaceClasses |= (1 << icls) ;
}

// 削除
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::RemoveSpaceClass( S3DScene::ItemClass icls )
{
	m_maskSpaceClasses &= ~(1 << icls) ;
}

// レイヤー描画対象クラス集合
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::Space::GetLayeredSpaceClasses( void ) const
{
	return	m_maskLayeredClasses ;
}

bool S3DScene::Space::IsLayeredSpaceClass( S3DScene::ItemClass icls ) const
{
	return	(m_maskLayeredClasses & (1 << icls)) != 0 ;
}

// レイヤー描画対象変更
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::Space::ModifyLayeredSpaceClasses
	( uint32_t nAddClasses, uint32_t nRemoveClasses )
{
	m_maskLayeredClasses =
		(m_maskLayeredClasses | nAddClasses) & ~nRemoveClasses ;
	return	m_maskLayeredClasses ;
}

void S3DScene::Space::AddLayeredSpaceClass( S3DScene::ItemClass icls )
{
	m_maskLayeredClasses |= (1 << icls) ;
}

void S3DScene::Space::RemoveLayeredSpaceClass( S3DScene::ItemClass icls )
{
	m_maskLayeredClasses &= ~(1 << icls) ;
}

// レイヤー描画優先度
//////////////////////////////////////////////////////////////////////////////
int S3DScene::Space::GetDrawingPriority( void ) const
{
	return	m_zLayeredPriority ;
}

void S3DScene::Space::SetDrawingPriority( int zPriority )
{
	m_zLayeredPriority = zPriority ;
}

// レイヤー描画パラメータ
//////////////////////////////////////////////////////////////////////////////
float32_t S3DScene::Space::GetLayeredDrawParameter( void ) const
{
	return	m_fpLayeredParam ;
}

void S3DScene::Space::SetLayeredDrawParameter( float32_t fpParam )
{
	m_fpLayeredParam = fpParam ;
}

// レイヤー描画環境マッピングソース
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::SetEnvironmentMappingSource
	( S3DScene::EnvironmentMappingSource emsReflection,
		S3DScene::EnvironmentMappingSource emsRefraction )
{
	m_emsReflectionSource = emsReflection ;
	m_emsRefractionSource = emsRefraction ;
}

void S3DScene::Space::GetEnvironmentMappingSource
	( S3DScene::EnvironmentMappingSource& emsReflection,
		S3DScene::EnvironmentMappingSource& emsRefraction ) const
{
	emsReflection = m_emsReflectionSource ;
	emsRefraction = m_emsRefractionSource ;
}

// レイヤー描画一時効果追加（classPreRender で追加）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::AddTemporaryEffect
	( S3DScene::DrawLayerEffector * pEffector )
{
	ESLAssert( m_aTempEffector.FindPtr( pEffector ) < 0 ) ;
	m_aTempEffector.InsertAt
		( S3DScene::OrderToAddEffect
			( m_aTempEffector, pEffector->GetDrawingPriority() ), pEffector ) ;
}

// 要求カラーバッファ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::Space::GetRequiredColorBufferMask( void ) const
{
	uint32_t	maskDrawSrcBufs = m_maskDrawSrcBuffers ;
	for ( size_t i = 0; i < m_aTempEffector.GetLength(); i ++ )
	{
		DrawLayerEffector *	pEffector = m_aTempEffector.GetAt( i ) ;
		if ( pEffector != nullptr )
		{
			maskDrawSrcBufs |= pEffector->GetRequiredColorBufferMask() ;
		}
	}
	if ( m_pLayerEffector != nullptr )
	{
		maskDrawSrcBufs |= m_pLayerEffector->GetRequiredColorBufferMask() ;
	}
	return	maskDrawSrcBufs ;
}

// classLayeredSpace 描画
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::DrawEffect
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
	if ( m_pLayerEffector != nullptr )
	{
		m_pLayerEffector->DrawEffect
			( scene, render, ppImage, nMultiImages, pDepth ) ;
	}
	else if ( (nMultiImages >= 1) && ppImage && ppImage[0] )
	{
		SGLPaintParam	pp ;
		pp.nTransparency =
			eslRoundR32ToInt( (float32_t) (m_fpLayeredParam * 0x100) ) ;
		render.DrawImage( pp, ppImage[0] ) ;
	}
}

// リスナ設定
//////////////////////////////////////////////////////////////////////////////
S3DScene::ItemEventListener *
	S3DScene::Space::AttachItemListener( ItemEventListener * pListener )
{
	ItemEventListener *	pLastListener = m_pListener ;
	m_pListener = pListener ;
	return	pLastListener ;
}

S3DScene::ItemEventListener *
	S3DScene::Space::GetItemListener( void ) const
{
	return	m_pListener ;
}

// レイヤー描画効果設定
//////////////////////////////////////////////////////////////////////////////
S3DScene::DrawLayerEffector *
	S3DScene::Space::AttachDrawLayerEffector( DrawLayerEffector * pEffector )
{
	DrawLayerEffector *	pLastEffector = m_pLayerEffector ;
	m_pLayerEffector = pEffector ;
	return	pLastEffector ;
}

S3DScene::DrawLayerEffector * S3DScene::Space::GetDrawLayerEffector( void ) const
{
	return	m_pLayerEffector ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	size_t	countTimer = m_timers.GetLength() ;
	if ( countTimer > 0 )
	{
		Timer **	ppTimer = m_timers.GetArray() ;
		for ( size_t i = 0; i < countTimer; i ++ )
		{
			Timer *	pTimer = ppTimer[i] ;
			ESLAssert( pTimer != nullptr ) ;
			if ( pTimer->OnTimer( scene, *this, msecPast ) )
			{
				if ( pTimer->m_flagAutoDelete )
				{
					delete	pTimer ;
				}
				ppTimer[i] = nullptr ;
			}
		}
		m_timers.FinishArray() ;
		m_timers.TrimEmpty() ;
		scene.PostSceneUpdate() ;
	}
	size_t			i ;
	size_t			countChildren = m_children.GetLength() ;
	Space*const*	ppChildren = m_children.GetConstArray() ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		if ( !(pChild->m_flagsSpaceBehavior & itemIgnore) )
		{
			// pChild->OnTimer( scene, msecPast ) ;
			scene.AsyncDispatchTimer( pChild, msecPast, nullptr ) ;
		}
	}
	size_t		countItems = m_items.GetLength() ;
	Item*const*	ppItems = m_items.GetConstArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		if ( pItem->m_flagsBehavior & itemTimer )
		{
			pItem->OnTimer( scene, msecPast ) ;
		}
	}
}

// タイマ処理フラッシュ
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::FlushAllTimers( S3DScene& scene )
{
	size_t	countTimer = m_timers.GetLength() ;
	if ( countTimer > 0 )
	{
		Timer **	ppTimer = m_timers.GetArray() ;
		for ( size_t i = 0; i < countTimer; i ++ )
		{
			Timer *	pTimer = ppTimer[i] ;
			ESLAssert( pTimer != nullptr ) ;
			if ( pTimer->OnFlush( scene, *this ) )
			{
				if ( pTimer->m_flagAutoDelete )
				{
					delete	pTimer ;
				}
				ppTimer[i] = nullptr ;
			}
		}
		m_timers.FinishArray() ;
		m_timers.TrimEmpty() ;
	}
}

// タイマ追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::AddTimer( S3DScene::Timer * pTimer )
{
	ESLAssert( pTimer != nullptr ) ;
	ESLAssert( m_timers.FindPtr( pTimer ) < 0 ) ;
	m_timers.Add( pTimer ) ;
}

// タイマ削除
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::RemoveTimer( S3DScene::Timer * pTimer )
{
	ssize_t	i = m_timers.FindPtr( pTimer ) ;
	if ( i >= 0 )
	{
		if ( pTimer->m_flagAutoDelete )
		{
			delete	pTimer ;
		}
		m_timers.RemoveAt( i ) ;
	}
}

void S3DScene::Space::RemoveAllTimers( void )
{
	size_t			countTimer = m_timers.GetLength() ;
	Timer *const*	ppTimer = m_timers.GetConstArray() ;
	for ( size_t i = 0; i < countTimer; i ++ )
	{
		Timer *	pTimer = ppTimer[i] ;
		ESLAssert( pTimer != nullptr ) ;
		if ( pTimer->m_flagAutoDelete )
		{
			delete	pTimer ;
		}
	}
	m_timers.RemoveAll() ;
}

// タイマ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DScene::Space::FindTimerTypeOf
	( const ESLRuntimeClass& rtClass, size_t iFirst ) const
{
	size_t			countTimer = m_timers.GetLength() ;
	Timer *const*	ppTimer = m_timers.GetConstArray() ;
	for ( size_t i = iFirst; i < countTimer; i ++ )
	{
		Timer *	pTimer = ppTimer[i] ;
		ESLAssert( pTimer != nullptr ) ;
		if ( pTimer->IsKindOf( rtClass ) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// タイマ数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DScene::Space::GetTimerCount( void ) const
{
	return	m_timers.GetLength() ;
}

// タイマ取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::Timer * S3DScene::Space::GetTimerAt( size_t iTimer ) const
{
	return	m_timers.GetAt( iTimer ) ;
}

S3DScene::Timer * S3DScene::Space::GetTimerTypeOf
		( const ESLRuntimeClass& rtClass, size_t iFirst ) const
{
	return	m_timers.GetAt( FindTimerTypeOf( rtClass, iFirst ) ) ;
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::OnUpdateBehavior( S3DScene& scene )
{
}

// 規定のレンダリングデバイス設定通知
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::SetRenderDevice( S3DRenderDevice * pDevice )
{
	size_t			i ;
	size_t			countChildren = m_children.GetLength() ;
	Space*const*	ppChildren = m_children.GetConstArray() ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		pChild->SetRenderDevice( pDevice ) ;
	}
	//
	size_t		countItems = m_items.GetLength() ;
	Item*const*	ppItems = m_items.GetConstArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		pItem->SetRenderDevice( pDevice ) ;
	}
	//
	if ( m_pListener != nullptr )
	{
		m_pListener->OnSetRenderDevice( pDevice ) ;
	}
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::PrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	size_t			i ;
	size_t			countChildren = m_children.GetLength() ;
	Space*const*	ppChildren = m_children.GetConstArray() ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		pChild->PrepareToRender( pDevice, nFlags ) ;
	}
	//
	size_t		countItems = m_items.GetLength() ;
	Item*const*	ppItems = m_items.GetConstArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		pItem->PrepareToRender( pDevice, nFlags ) ;
	}
	//
	if ( m_pListener != nullptr )
	{
		m_pListener->OnPrepareToRender( pDevice, nFlags ) ;
	}
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::OnRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( m_flagsSpaceBehavior & (itemSpaceHidden | itemIgnore) )
	{
		return ;
	}
	size_t			i ;
	size_t			countChildren = m_children.GetLength() ;
	Space*const*	ppChildren = m_children.GetConstArray() ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		if ( pChild->m_maskItemClasses & (1 << clsItem) )
		{
			scene.AsyncDispatchRenderEvent( pChild, clsItem, nullptr ) ;
			// pChild->OnRenderEvent( scene, clsItem ) ;
		}
	}
	//
	size_t		countItems = m_items.GetLength() ;
	Item*const*	ppItems = m_items.GetConstArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		if ( (pItem->m_classItem == clsItem)
			|| (pItem->m_maskClasses & (1 << clsItem)) )
		{
			pItem->OnRenderEvent( scene, clsItem ) ;
		}
	}
	//
	if ( m_pListener != nullptr )
	{
		m_pListener->OnItemRenderEvent( scene, clsItem ) ;
	}
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::RenderCollision
	( const S3DScene& scene,
		S3DCollision& render, uint32_t maskClasses ) const
{
	if ( m_flagsSpaceBehavior & itemIgnore )
	{
		return ;
	}
	render.PushTransformation() ;
	if ( m_flagsSpaceBehavior
			& (itemGlobalSpace | itemCameraShift | itemCameraSpace) )
	{
		S3DDMatrix	matSpace = m_matTransformation ;
		S3DDVector	vPos = m_vCenter ;
		if ( m_flagsSpaceBehavior & itemCameraSpace )
		{
			vPos = scene.GetCurrentCameraIMatrix() * vPos ;
			matSpace = scene.GetCurrentCameraIMatrix() * matSpace ;
		}
		if ( m_flagsSpaceBehavior & (itemCameraShift | itemCameraSpace) )
		{
			vPos += scene.GetCurrentCameraPosition() ;
		}
		vPos += scene.GetRenderingOffset() ;
		render.SetMatrixTransformation( matSpace, vPos ) ;
	}
	else
	{
		render.AppendMatrixTransformation( m_matTransformation, m_vCenter ) ;
	}
	//
	size_t	i ;
	size_t	countChildren = m_children.GetLength() ;
	Space**	ppChildren = m_children.GetArray() ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		if ( (pChild->m_flagsBehavior & itemCollision)
			&& (pChild->m_maskItemClasses & maskClasses) )
		{
			pChild->RenderCollision( scene, render, maskClasses ) ;
		}
	}
	//
	size_t	countItems = m_items.GetLength() ;
	Item**	ppItems = m_items.GetArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		if ( (maskClasses & ((1 << pItem->m_classItem) | pItem->m_maskClasses))
			&& (pItem->m_flagsBehavior & itemCollision) )
		{
			render.AttachMeshUserData( pItem ) ;
			pItem->RenderCollision( scene, render ) ;
		}
	}
	//
	if ( m_pListener != nullptr )
	{
		m_pListener->OnItemRenderCollision( scene, render ) ;
	}
	//
	render.PopTransformation() ;
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::RenderModel
	( const S3DScene& scene, S3DRenderContextInterface& render,
		uint32_t optShader,
		S3DScene::ItemClass clsItem, uint64_t flagsExclusion ) const
{
	if ( m_flagsSpaceBehavior & (itemSpaceHidden | itemIgnore) )
	{
		return ;
	}
	//
	// ローカル空間処理
	//
	render.PushTransformation() ;
	if ( m_flagsSpaceBehavior
			& (itemGlobalSpace | itemCameraShift | itemCameraSpace) )
	{
		S3DDMatrix	matSpace = m_matTransformation ;
		S3DDVector	vPos = m_vCenter ;
		if ( m_flagsSpaceBehavior & itemCameraSpace )
		{
			vPos = scene.GetCurrentCameraIMatrix() * vPos ;
			matSpace = scene.GetCurrentCameraIMatrix() * matSpace ;
		}
		if ( m_flagsSpaceBehavior & (itemCameraShift | itemCameraSpace) )
		{
			vPos += scene.GetCurrentCameraPosition() ;
		}
		vPos += scene.GetRenderingOffset() ;
		render.SetMatrixTransformation
			( matSpace, vPos, &(m_colorEffect), m_nTransparency ) ;
	}
	else
	{
		render.AppendMatrixTransformation
				( m_matTransformation, m_vCenter,
					&(m_colorEffect), m_nTransparency ) ;
	}
	if ( m_flagsSpaceBehavior & itemHideNear )
	{
		if ( scene.IsAheadNearHiddenDistance( render ) )
		{
			render.PopTransformation() ;
			return ;
		}
	}
	if ( m_flagsSpaceBehavior & itemHideFar )
	{
		if ( scene.IsBehindFarHiddenDistance( render ) )
		{
			render.PopTransformation() ;
			return ;
		}
	}
	uint64_t	flagsShading = render.GetShadingFlag() ;
	uint64_t	flagsLocalShading =
					EffectedShaderFlags( flagsShading, optShader ) ;
	//
	optShader &= ~(m_flagsShader >> shaderFreeShifter) & shaderForceMask ;
	optShader |= m_flagsShader & shaderForceMask ;
	//
	// シェーダー設定
	//
	S3DCustomShader *	pOldShader = nullptr ;
	bool				fChangeShader = false ;
	if ( m_pLocalShader != nullptr )
	{
		pOldShader = render.GetCustomShader() ;
		if ( pOldShader != m_pLocalShader )
		{
//			render.Flush() ;
			render.AttachCustomShader( m_pLocalShader ) ;
			fChangeShader = true ;
		}
	}
	if ( flagsLocalShading != flagsShading )
	{
		if ( !fChangeShader )
		{
//			render.Flush() ;
			fChangeShader = true ;
		}
		render.SetShadingFlag( flagsLocalShading ) ;
	}
	size_t	i, nCount ;
	nCount = m_uniforms.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DCustomShader::UniformData *	pud = m_uniforms.GetAt( i ) ;
		const SString *					pid = m_uniforms.GetTagAt( i ) ;
		if ( pud && pid )
		{
			render.SetCustomShaderUniform
				( *pid, pud->m_type, pud->m_pData, pud->m_nLength ) ;
		}
	}
	if ( m_pListener != nullptr )
	{
		m_pListener->BeforeItemRenderModel( scene, render, flagsExclusion ) ;
	}
	//
	// 子空間描画処理
	//
	size_t			countChildren = m_children.GetLength() ;
	Space*const*	ppChildren = m_children.GetConstArray() ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		if ( (pChild->m_flagsBehavior & itemVisible)
			&& !(pChild->m_flagsBehavior & (itemSpaceHidden | itemIgnore))
			&& (pChild->m_maskItemClasses & (1 << clsItem)) )
		{
			pChild->RenderModel
				( scene, render, optShader, clsItem, flagsExclusion ) ;
		}
	}
	//
	// 子アイテム描画処理
	//
	size_t		countItems = m_items.GetLength() ;
	Item*const*	ppItems = m_items.GetConstArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		if ( (pItem->m_flagsBehavior & itemVisible)
			&& !(pItem->m_flagsBehavior & itemIgnore) )
		{
			if ( (pItem->m_classItem == clsItem)
				|| (pItem->m_maskClasses & (1 << clsItem)) )
			{
				pItem->RenderModel
					( scene, render, flagsExclusion ) ;
			}
		}
	}
	RenderExtension( scene, render, clsItem, flagsExclusion ) ;
	//
	if ( m_pListener != nullptr )
	{
		m_pListener->OnItemRenderModel( scene, render, flagsExclusion ) ;
		m_pListener->AfterItemRenderModel( scene, render, flagsExclusion ) ;
	}
	//
	if ( fChangeShader )
	{
//		render.Flush() ;
		if ( m_pLocalShader != nullptr )
		{
			render.AttachCustomShader( pOldShader ) ;
		}
	}
	if ( flagsLocalShading != flagsShading )
	{
		render.SetShadingFlag( flagsShading ) ;
	}
	render.PopTransformation() ;
}

// カスタム描画
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::RenderExtension
	( const S3DScene& scene, S3DRenderContextInterface& render,
		S3DScene::ItemClass clsItem, uint64_t flagsExclusion ) const
{
}

// アイテム追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::AddItem( S3DScene::Item * pItem )
{
	ESLAssert( pItem != nullptr ) ;
	if ( pItem != nullptr )
	{
		ESLAssert( m_items.FindPtr( pItem ) < 0 ) ;
		m_items.Add( pItem ) ;
		pItem->m_refParentSpace.SetReference( this ) ;
		pItem->m_refSpace.SetReference( this ) ;
	}
}

void S3DScene::Space::InsertItem( size_t i, S3DScene::Item * pItem )
{
	ESLAssert( pItem != nullptr ) ;
	if ( pItem != nullptr )
	{
		ESLAssert( m_items.FindPtr( pItem ) < 0 ) ;
		m_items.InsertAt( i, pItem ) ;
		pItem->m_refParentSpace.SetReference( this ) ;
		pItem->m_refSpace.SetReference( this ) ;
	}
}

// アイテム削除（分離）
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Space::RemoveItem( S3DScene::Item * pItem )
{
	ssize_t	i = m_items.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_items.RemoveAt( i ) ;
		pItem->m_refParentSpace.ReleaseReference() ;
		if ( pItem->GetReferenceSpace() == this )
		{
			pItem->m_refSpace.ReleaseReference() ;
			return	true ;
		}
	}
	return	false ;
}

// アイテム遅延削除
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::DelayDeleteItem( S3DScene::Item * pItem )
{
	ssize_t	i = m_items.FindPtr( pItem ) ;
	if ( i >= 0 )
	{
		m_items.RemoveAt( i ) ;
		pItem->m_refParentSpace.ReleaseReference() ;
		if ( pItem->GetReferenceSpace() == this )
		{
			pItem->m_refSpace.ReleaseReference() ;
		}
		m_aDelayRemove.Add( pItem ) ;
	}
}

// アイテム全削除（分離）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::RemoveAllItems( void )
{
	size_t			countItems = m_items.GetLength() ;
	Item*const*		ppItems = m_items.GetConstArray() ;
	for ( size_t i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		pItem->m_refParentSpace.ReleaseReference() ;
		if ( pItem->GetReferenceSpace() == this )
		{
			pItem->m_refSpace.ReleaseReference() ;
		}
	}
	m_items.RemoveAll() ;
}

// アイテム数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DScene::Space::GetItemCount( void ) const
{
	return	m_items.GetLength() ;
}

// アイテム取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::Item * S3DScene::Space::GetItemAt( size_t i ) const
{
	return	m_items.GetAt( i ) ;
}

// アイテム検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DScene::Space::FindItemAs( const wchar_t * pwszID ) const
{
	size_t			countItems = m_items.GetLength() ;
	Item*const*		ppItems = m_items.GetConstArray() ;
	for ( size_t i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		ESLAssert( pItem != nullptr ) ;
		if ( pItem->m_pwszID
			&& (SString::Compare( pItem->m_pwszID, pwszID ) == 0) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

S3DScene::Item * S3DScene::Space::GetItemAs( const wchar_t * pwszID ) const
{
	return	m_items.GetAt( (size_t) FindItemAs( pwszID ) ) ;
}

// 子空間追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::AddChild( S3DScene::Space * pChild )
{
	ESLAssert( pChild != nullptr ) ;
	ESLAssert( pChild->m_refParent.GetReference() == nullptr ) ;
	if ( pChild != nullptr )
	{
		ESLAssert( m_children.FindPtr( pChild ) < 0 ) ;
		m_children.Add( pChild ) ;
		pChild->m_refParent.SetReference( this ) ;
		pChild->m_refSpace.SetReference( this ) ;
	}
}

void S3DScene::Space::InsertChild( size_t i, S3DScene::Space * pChild )
{
	ESLAssert( pChild != nullptr ) ;
	ESLAssert( pChild->m_refParent.GetReference() == nullptr ) ;
	if ( pChild != nullptr )
	{
		ESLAssert( m_children.FindPtr( pChild ) < 0 ) ;
		m_children.InsertAt( i, pChild ) ;
		pChild->m_refParent.SetReference( this ) ;
		pChild->m_refSpace.SetReference( this ) ;
	}
}

// 子空間削除
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Space::RemoveChild( S3DScene::Space * pChild )
{
	ssize_t	i = m_children.FindPtr( pChild ) ;
	if ( i >= 0 )
	{
		m_children.RemoveAt( i ) ;
		pChild->m_refParent.ReleaseReference() ;
		if ( pChild->m_refSpace.GetReference() == this )
		{
			pChild->m_refSpace.ReleaseReference() ;
			return	true ;
		}
	}
	return	false ;
}

// 子空間全削除
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Space::RemoveAllChildren( void )
{
	size_t			countChildren = m_children.GetLength() ;
	Space*const*	ppChildren = m_children.GetConstArray() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		pChild->m_refParent.ReleaseReference() ;
		if ( pChild->m_refSpace.GetReference() == this )
		{
			pChild->m_refSpace.ReleaseReference() ;
		}
	}
	m_children.RemoveAll() ;
}

// 子空間数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DScene::Space::GetChildrenCount( void ) const
{
	return	m_children.GetLength() ;
}

// 子空間取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::Space * S3DScene::Space::GetChildAt( size_t i ) const
{
	return	m_children.GetAt( i ) ;
}

// 子空間検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DScene::Space::FindChildAs( const wchar_t * pwszID ) const
{
	size_t			countChildren = m_children.GetLength() ;
	Space*const*	ppChildren = m_children.GetConstArray() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		ESLAssert( pChild != nullptr ) ;
		if ( pChild->m_pwszID
			&& (SString::Compare( pChild->m_pwszID, pwszID ) == 0) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

S3DScene::Space * S3DScene::Space::GetChildAs( const wchar_t * pwszID ) const
{
	return	m_children.GetAt( (size_t) FindChildAs( pwszID ) ) ;
}

// レンダリングスレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SSystem::SError S3DScene::Space::Lock( int64_t msecTimeout ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene != nullptr )
	{
		return	pScene->Lock( msecTimeout ) ;
	}
	else
	{
		return	SSystem::Lock( msecTimeout ) ;
	}
}

SSystem::SError S3DScene::Space::Unlock( void ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene != nullptr )
	{
		return	pScene->Unlock() ;
	}
	else
	{
		return	SSystem::Unlock() ;
	}
}

atomic_int_t S3DScene::Space::TestLocked( void ) const
{
	S3DScene *	pScene = GetScene() ;
	if ( pScene != nullptr )
	{
		return	pScene->TestLocked() ;
	}
	else
	{
		return	SSystem::TestLocked() ;
	}
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Space::OnSave( SSystem::SFileInterface& file )
{
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	file.Write( &m_flagsBehavior, sizeof(uint32_t) ) ;
	file.Write( &m_maskItemClasses, sizeof(uint32_t) ) ;
	file.Write( &m_maskSpaceClasses, sizeof(uint32_t) ) ;
	file.Write( &m_maskLayeredClasses, sizeof(uint32_t) ) ;
	file.Write( &m_zLayeredPriority, sizeof(int32_t) ) ;
	file.Write( &m_maskDrawSrcBuffers, sizeof(uint32_t) ) ;
	file.Write( &m_fpLayeredParam, sizeof(float32_t) ) ;
	file.Write( &m_emsReflectionSource, sizeof(EnvironmentMappingSource) ) ;
	file.Write( &m_emsRefractionSource, sizeof(EnvironmentMappingSource) ) ;
	//
	uint32_t	nCount = (uint32_t) m_children.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		posm->SaveObject( file, m_children.GetAt(i), false ) ;
	}
	nCount = (uint32_t) m_items.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		posm->SaveObject( file, m_items.GetAt(i), false ) ;
	}
	nCount = (uint32_t) m_timers.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		posm->SaveObject( file, m_timers.GetAt(i), false ) ;
	}
	//
	SString	strShaderID ;
	if ( m_pLocalShader != nullptr )
	{
		strShaderID = posm->GetIdentityOf( m_pLocalShader ) ;
	}
	file.WriteString( strShaderID ) ;
	//
	nCount = (uint32_t) m_uniforms.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		const SString *					pid = m_uniforms.GetTagAt( i ) ;
		S3DCustomShader::UniformData *	pud = m_uniforms.GetAt( i ) ;
		ESLAssert( pid && pud ) ;
		if ( !pid || !pud )
		{
			return	sglErrFailed ;
		}
		file.WriteString( *pid ) ;
		//
		uint32_t	nType = (uint32_t) pud->m_type ;
		uint32_t	nLength = (uint32_t) pud->m_nLength ;
		file.Write( &nType, sizeof(uint32_t) ) ;
		file.Write( &nLength, sizeof(uint32_t) ) ;
		file.Write( pud->m_pData, pud->GetDataBytes() ) ;
	}
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Space::OnRestore( SSystem::SFileInterface& file )
{
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( (file.Read( &m_flagsBehavior, sizeof(uint32_t) ) < sizeof(uint32_t))
		|| (file.Read( &m_maskItemClasses, sizeof(uint32_t) ) < sizeof(uint32_t))
		|| (file.Read( &m_maskSpaceClasses, sizeof(uint32_t) ) < sizeof(uint32_t))
		|| (file.Read( &m_maskLayeredClasses, sizeof(uint32_t) ) < sizeof(uint32_t))
		|| (file.Read( &m_zLayeredPriority, sizeof(int32_t) ) < sizeof(int32_t))
		|| (file.Read( &m_maskDrawSrcBuffers, sizeof(uint32_t) ) < sizeof(uint32_t))
		|| (file.Read( &m_fpLayeredParam, sizeof(float32_t) ) < sizeof(float32_t))
		|| (file.Read( &m_emsReflectionSource, sizeof(EnvironmentMappingSource) ) < sizeof(EnvironmentMappingSource))
		|| (file.Read( &m_emsRefractionSource, sizeof(EnvironmentMappingSource) ) < sizeof(EnvironmentMappingSource)) )
	{
		return	sglErrFailed ;
	}
	m_children.RemoveAll() ;
	m_items.RemoveAll() ;
	m_timers.RemoveAll() ;
	m_uniforms.RemoveAll() ;
	//
	uint32_t	nCount ;
	if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		AddChild( SGLSmartCast<Space>( posm->LoadObject( file, false ) ) ) ;
	}
	//
	if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		AddItem( SGLSmartCast<Item>( posm->LoadObject( file, false ) ) ) ;
	}
	//
	if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		AddTimer( SGLSmartCast<Timer>( posm->LoadObject( file, false ) ) ) ;
	}
	//
	SString	strShaderID ;
	file.ReadString( strShaderID ) ;
	m_pLocalShader =
		ESLTypeCast<S3DCustomShader>( posm->GetObjectOf( strShaderID ) ) ;
	//
	if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	for ( uint32_t i = 0; i < nCount; i ++ )
	{
		SString	strID ;
		file.ReadString( strID ) ;
		//
		uint32_t	nType, nLength ;
		file.Read( &nType, sizeof(uint32_t) ) ;
		file.Read( &nLength, sizeof(uint32_t) ) ;
		//
		SArray<uint8_t>	bufData ;
		size_t	nBytes =
			S3DCustomShader::UniformData::GetDataBytes
				( (S3DCustomShader::UniformType) nType, (size_t) nLength ) ;
		file.Read( bufData.GetArray( nBytes ), nBytes ) ;
		bufData.FinishArray() ;
		//
		S3DCustomShader::UniformData *
						pud = new S3DCustomShader::UniformData ;
		pud->SetData
			( (S3DCustomShader::UniformType) nType,
					bufData.GetConstArray(), (size_t) nLength ) ;
		m_uniforms.SetAs( strID, pud ) ;
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// シャドウマッピング・バッファ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::ShadowMapBuffer::ShadowMapBuffer( void )
{
	m_smpShadowMap.nFlags = 0 ;
	m_smpShadowMap.sizeMapping.w = 1024 ;
	m_smpShadowMap.sizeMapping.h = 1024 ;
	m_smpShadowMap.fpPixelDensity = 0.01f ;
	m_smpShadowMap.tanAngleOfView = 1.0f ;
	m_smpShadowMap.zPersDistance = 10.0f ;
	m_smpShadowMap.zCameraStdDistance = 10.0f ;
	m_smpShadowMap.zPersNear = -100.0f ;
	m_smpShadowMap.zPersFar = 300.0f ;
	m_smpShadowMap.zErrorPrecision = -12.0f ;
	m_smpShadowMap.zErrorSubPrecision = -12.0f ;
	m_smpShadowMap.fpFilterGauss = 2.0f ;
	m_smpShadowMap.nCascadeMaxCount = 0 ;
	m_smpShadowMap.zCascadeDensityStep = 3.0f ;
	m_smpShadowMap.zCascadeTargetLengthStep = 2.0f ;
	m_smpShadowMap.zCascadeTargetLengthOffset = 20.0f ;
	m_smpShadowMap.zCascadeDepthStep = 1.0f ;
	m_smpShadowMap.zCascadeDistanceOffsetStep = 0.0f ;
	//
	m_nRenderedShadowmap = 0 ;
}

// シャドウマップ・パラメータ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::ShadowMapBuffer::SetShadowMappingInfo
	( S3DRenderContextInterface& renderForShadowmap,
		S3DShadowMapInfo& smi,
		const ShadowMapParam& smp,
		const S3DLightEntry& light,
		size_t iCascade, const SGLSize& sizeImage,
		const S3DDVector& vCameraTarget, const S3DDVector& vCameraPos )
{
	float32_t	zPersNear = smp.zPersNear ;
	float32_t	zPersFar = smp.zPersFar ;
	double		zPersScreen = sizeImage.w * 0.5 / smp.tanAngleOfView ;
	double		zPersDistance = zPersScreen * smp.fpPixelDensity ;
	double		zPersScale = smp.fpPixelDensity ;
	double		fpAsjustScale = 1.0 ;
	double		degAngleVarX = 0.0, degAngleVarY = 0.0 ;
	S3DDVector	vTarget = vCameraTarget ;

	static const double	s_degAngleVarXY[6][2] =
	{
		{ 0.0, 0.0 }, { 0.0, 90.0 }, { 0.0, -90.0 },
		{ 90.0, 0.0 }, { -90.0, 0.0 }, { 0.0, 180.0 },
	} ;
	switch ( light.typeLight & lightTypeMask )
	{
	case	lightTypeVector:
		if ( smp.nFlags & S3DScene::shadowPersDistance )
		{
			zPersDistance = smp.zPersDistance ;
		}
		if ( smp.nFlags & S3DScene::shadowAdjustByCamera )
		{
			double	r = (vCameraTarget - vCameraPos).Absolute() ;
			fpAsjustScale = sqrt( r / smp.zCameraStdDistance ) ;
			zPersScreen *= fpAsjustScale ;
			zPersDistance *= fpAsjustScale ;
			zPersScale *= fpAsjustScale ;
			zPersNear *= (float32_t) fpAsjustScale ;
			zPersFar *= (float32_t) fpAsjustScale ;
		}
		if ( iCascade > 0 )
		{
			float32_t	fpScale =
				(float32_t) pow( smp.zCascadeDensityStep,
										(float32_t) iCascade ) ;
			zPersScale *= fpScale ;
			//
			fpScale =
				(float32_t) pow( smp.zCascadeDepthStep,
										(float32_t) iCascade ) ;
			zPersDistance *= fpScale ;
			zPersNear *= fpScale ;
			zPersFar *= fpScale ;
			//
			S3DDVector	vCameraView = vCameraTarget - vCameraPos ;
			vTarget += vCameraView
						* (smp.zCascadeTargetLengthStep * iCascade) ;
			vTarget += vCameraView.Normalized()
						* (smp.zCascadeTargetLengthOffset
									* fpAsjustScale * iCascade) ;
			zPersDistance += smp.zCascadeDistanceOffsetStep * iCascade ;
		}
		break ;

	case	lightTypePoint:
	case	lightTypeSpot:
		degAngleVarX = s_degAngleVarXY[iCascade % 6][0] ;
		degAngleVarY = s_degAngleVarXY[iCascade % 6][1] ;
		break ;
	}
	return	smi.SetShadowMappingInfo
				( &renderForShadowmap, light,
					vTarget, sizeImage, zPersScreen,
					zPersDistance, zPersScale,
					zPersNear, zPersFar,
					smp.zErrorPrecision, smp.zErrorSubPrecision,
					degAngleVarX, degAngleVarY ) ;
}


//////////////////////////////////////////////////////////////////////////////
// 光源アイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::Light, Item )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Light::Light( void )
{
	m_classItem = classLight ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Light::~Light( void )
{
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Light::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnSave( file ) ;
	if ( (file.Write
			( &m_light,
				sizeof(S3DLightEntry) ) < sizeof(S3DLightEntry))
		|| (file.Write
			( &m_smpShadowMap,
				sizeof(ShadowMapParam) ) < sizeof(ShadowMapParam)) )
	{
		err = sglErrFailed ;
	}
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Light::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnRestore( file ) ;
	if ( (file.Read
			( &m_light,
				sizeof(S3DLightEntry) ) < sizeof(S3DLightEntry))
		|| (file.Read
			( &m_smpShadowMap,
				sizeof(ShadowMapParam) ) < sizeof(ShadowMapParam)) )
	{
		err = sglErrFailed ;
	}
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// カメラアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::Camera, Item )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Camera::Camera( void )
	: m_nCameraFlags( 0 ),
		m_vTarget( 0, 0, 1 ), m_vTop( 0, -1, 0 ), m_degHorzFOV( 90 ),
		m_vLeftEarDir( -1, 0, 1 ), m_vRightEarDir( 1, 0, 1 ),
		m_cosSoundLow( -1 ), m_fpLowVolume( 0 )
{
	m_classItem = classCamera ;
}

// カメラ視野角から投影スクリーン距離を計算する
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::Camera::CalcProjectionZbyFOV
	( float32_t& zScreen, const SGLSize& sizeView ) const
{
	if ( m_nCameraFlags & Camera::cameraForceHFOV )
	{
		double	tanFOV = tan( m_degHorzFOV * PI / 360.0 ) ;
		zScreen = (float32_t) (sizeView.w * 0.5 / tanFOV) ;
		return	true ;
	}
	return	false ;
}

// カメラ視点
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DScene::Camera::GetCameraPosition( void ) const
{
	return	m_space.m_vCenter ;
}

void S3DScene::Camera::SetCameraPosition( const S3DDVector& vPos )
{
	m_space.m_vCenter = vPos ;
}

// カメラ注視点
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DScene::Camera::GetCameraTarget( void ) const
{
	return	m_vTarget ;
}

void S3DScene::Camera::SetCameraTarget( const S3DDVector& vTarget )
{
	m_vTarget = vTarget ;
}

// カメラ頂点ベクトル
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DScene::Camera::GetCameraTop( void ) const
{
	return	m_vTop ;
}

void S3DScene::Camera::SetCameraTop( const S3DDVector& vTop )
{
	m_vTop = vTop ;
}

// カメラ視野空間への変換行列を計算する
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Camera::CalcCameraMatrix
			( S3DDMatrix& matCamera, S3DDVector& vCamera ) const
{
	S3DDMatrix	matSpace ;
	S3DDVector	vSpace ;
	CalcItemLinkTransformation( matSpace, vSpace ) ;

	vCamera =
		matCamera.CameraAngleOf
				( matSpace * GetCameraTarget() + vSpace,
					matSpace * GetCameraPosition() + vSpace,
					matSpace * GetCameraTop() ) ;
}

// 視差情報を取得する
//////////////////////////////////////////////////////////////////////////////
const S3DScene::StereoParallaxParam *
	S3DScene::Camera::GetParallaxOfView
			( SGLSecondaryViewProducer * psvp ) const
{
	StereoParallaxParam *	pspp = m_psoaViewParallax.GetAs( psvp ) ;
	if ( pspp != nullptr )
	{
		return	pspp ;
	}
	if ( m_nCameraFlags & cameraHasParallax )
	{
		return	&m_sppParallax ;
	}
	return	nullptr ;
}

// 視差情報を設定する
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Camera::SetParallaxOfView
	( SGLSecondaryViewProducer * psvp,
				const S3DScene::StereoParallaxParam& spp )
{
	StereoParallaxParam *	pspp = m_psoaViewParallax.GetAs( psvp ) ;
	if ( pspp == nullptr )
	{
		pspp = new StereoParallaxParam ;
		m_psoaViewParallax.SetAs( psvp, pspp ) ;
	}
	*pspp = spp ;
}

// 視差情報を削除する
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Camera::RemoveParallaxOfView( SGLSecondaryViewProducer * psvp )
{
	m_psoaViewParallax.RemoveAs( psvp ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Camera::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnSave( file ) ;
	if ( (file.Write
			( &m_vTarget,
				sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Write
			( &m_vTop,
				sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Write
			( &m_nCameraFlags, sizeof(uint32_t) ) < sizeof(uint32_t))
		|| (file.Write
			( &m_degHorzFOV, sizeof(double) ) < sizeof(double))
		|| (file.Write
			( &m_sppParallax, sizeof(StereoParallaxParam) )
									< sizeof(StereoParallaxParam))
		|| (file.Write
			( &m_vLeftEarDir, sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Write
			( &m_vRightEarDir, sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Write
			( &m_cosSoundLow, sizeof(double) ) < sizeof(double))
		|| (file.Write
			( &m_fpLowVolume, sizeof(double) ) < sizeof(double)) )
	{
		err = sglErrFailed ;
	}
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::Camera::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnRestore( file ) ;
	if ( (file.Read
			( &m_vTarget,
				sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Read
			( &m_vTop,
				sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Read
			( &m_nCameraFlags, sizeof(uint32_t) ) < sizeof(uint32_t))
		|| (file.Read
			( &m_degHorzFOV, sizeof(double) ) < sizeof(double))
		|| (file.Read
			( &m_sppParallax, sizeof(StereoParallaxParam) )
									< sizeof(StereoParallaxParam))
		|| (file.Read
			( &m_vLeftEarDir, sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Read
			( &m_vRightEarDir, sizeof(S3DDVector) ) < sizeof(S3DDVector))
		|| (file.Read
			( &m_cosSoundLow, sizeof(double) ) < sizeof(double))
		|| (file.Read
			( &m_fpLowVolume, sizeof(double) ) < sizeof(double)) )
	{
		err = sglErrFailed ;
	}
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// モデルデータアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2( SakuraGL::S3DScene::ModelItem, Item, LocalShader )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::ModelItem::ModelItem( void )
{
	m_classItem = classStaticItem1;
	m_pModel = nullptr ;
	m_pCollision = nullptr ;
	m_flagsModelExclusion = 0 ;
	m_flagsModelRequest = 0 ;
	m_iModelViewFirst = 0 ;
	m_iModelViewEnd = -1 ;
	m_flagCollision = false ;
	m_flagLocalBorder = false ;
	m_maskColliderFlags = S3DCollision::colliderShape
							| S3DCollision::colliderBarrier ;
	m_pLocalShader = nullptr ;
}

// モデルデータ関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::AttachModel( S3DVertexBufferInterface * pModel )
{
	m_pModel = pModel ;
	//
	m_flagsBehavior &= ~itemVisible ;
	if ( pModel )
	{
		m_flagsBehavior |= itemVisible ;
	}
}

void S3DScene::ModelItem::AttachCollisionModel
		( S3DVertexBufferInterface * pColModel, bool fBuildCollision )
{
	m_pCollision = pColModel ;
	//
	m_flagsBehavior &= ~itemCollision ;
	if ( pColModel )
	{
		m_flagsBehavior |= itemCollision ;
	}
	if ( fBuildCollision && (pColModel != nullptr) )
	{
		BuildCollisionMesh() ;
	}
}

// 当たり判定モデル構築
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::BuildCollisionMesh( void )
{
	m_collisionMesh.ClearBuffer() ;
	if ( m_pCollision != nullptr )
	{
		m_collisionMesh.AttachMeshUserData( (Item*) this ) ;
		m_collisionMesh.SetSceneClassesMask
							( (1 << m_classItem) | m_maskClasses ) ;
		m_collisionMesh.SetUserClassesMask( m_maskColliderFlags ) ;
		m_pCollision->RenderBufferTo( &m_collisionMesh ) ;
		m_flagCollision = true ;
	}
	else
	{
		m_flagCollision = false ;
	}
}

// 当たり判定モデル解放
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::ReleaseCollisionMesh( void )
{
	m_flagCollision = false ;
	m_collisionMesh.ClearBuffer() ;
}

// 表示除外フラグ（表面属性フラグ）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::SetModelExclusionFlags( uint64_t flagsExclusion )
{
	m_flagsModelExclusion = flagsExclusion ;
}

uint64_t S3DScene::ModelItem::GetModelExclusionFlags( void ) const
{
	return	m_flagsModelExclusion ;
}

// 表示選択フラグ（表面属性フラグ）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::SetModelRequestFlags( uint64_t flagsRequest )
{
	m_flagsModelRequest = flagsRequest ;
}

uint64_t S3DScene::ModelItem::GetModelRequestFlags( void ) const
{
	return	m_flagsModelRequest ;
}

// 表示メッシュ
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::SetViewModelMeshRange( size_t iFirst, ssize_t iEnd )
{
	m_iModelViewFirst = iFirst ;
	m_iModelViewEnd = iEnd ;
}

void S3DScene::ModelItem::GetViewModelMeshRange( size_t& iFirst, ssize_t& iEnd ) const
{
	iFirst = m_iModelViewFirst ;
	iEnd = m_iModelViewEnd ;
}

// 当たり判定ユーザーフラグ集合
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::SetColliderUserFlags( uint32_t nFlags )
{
	m_maskColliderFlags = nFlags ;
}

uint32_t S3DScene::ModelItem::GetColliderUserFlags( void ) const
{
	return	m_maskColliderFlags ;
}

// モデルデータ取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DScene::ModelItem::GetModel( void ) const
{
	return	m_pModel ;
}

// 衝突判定用モデルデータ取得
//////////////////////////////////////////////////////////////////////////////
S3DVertexBufferInterface * S3DScene::ModelItem::GetCollisionModel( void ) const
{
	return	m_flagCollision ?
				(S3DVertexBufferInterface*) &m_collisionMesh
				: m_pCollision ;
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::PrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != nullptr )
	{
		pModel->CommitToDevice( pDevice, 10 ) ;
	}
	else if ( m_pModel != nullptr )
	{
		pDevice->CommitDeviceVertexBuffer( m_pModel ) ;
	}
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::RenderLocalCollision
	( const S3DScene& scene, S3DCollision& render )
{
	if ( m_flagCollision )
	{
		render.SetUserClassesMask( m_maskColliderFlags ) ;
		render.AddColliderObject( &m_collisionMesh, nullptr, 0 ) ;
	}
	else if ( m_pCollision != nullptr )
	{
		render.SetUserClassesMask( m_maskColliderFlags ) ;
		m_pCollision->RenderBufferTo( &render ) ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	ItemClass	clsItem = scene.GetCurrentRenderingPhase() ;
	if ( (m_pModel != nullptr)
		&& ((m_classItem == clsItem)
				|| (m_maskClasses & (1 << clsItem)))
		&& m_pModel->IsModelIntoView( &render ) )
	{
		ShaderSaver	ss ;
		PrepareShaderSettings( scene, render, ss ) ;
		//
		RenderLocalModelWithReq
			( render, m_pModel,
				m_flagsModelRequest,
				flagsExclusion | m_flagsModelExclusion,
				m_iModelViewFirst, m_iModelViewEnd ) ;
		//
		RestoreShaderSettings( scene, render, ss ) ;
	}
	//
	Item::RenderLocalModel( scene, render, flagsExclusion ) ;
}

void S3DScene::ModelItem::RenderLocalModelWithReq
	( S3DRenderContextInterface& render,
		S3DVertexBufferInterface * pModel,
		uint64_t flagsRegFlags, uint64_t flagsExclusion,
		size_t iFirstMesh, ssize_t iEndMesh,
		size_t nInstancingCount,
		const S4DMatrix * pmatInstancing, const S3DColor * pclrInstancing )
{
	if ( flagsRegFlags == 0 )
	{
		pModel->RenderBufferTo
			( &render, flagsExclusion, iFirstMesh, iEndMesh,
				nInstancingCount, pmatInstancing, pclrInstancing ) ;
	}
	else
	{
		size_t	nMeshCount = pModel->GetMeshCount() ;
		if ( iFirstMesh >= nMeshCount )
		{
			return ;
		}
		if ( (iEndMesh < (ssize_t) iFirstMesh)
			|| (iEndMesh >= (ssize_t) nMeshCount) )
		{
			nMeshCount -= iFirstMesh ;
		}
		else
		{
			nMeshCount = (size_t) iEndMesh - iFirstMesh ;
		}
		S3DVertexBufferInterface::MeshInfo	inf ;
		eslFillMemory
			( &inf, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
		size_t	iLast = iFirstMesh ;
		for ( size_t i = 0; i < nMeshCount; i ++ )
		{
			size_t	iMesh = iFirstMesh + i ;
			if ( pModel->GetMeshInfoAt( inf, iMesh, 0 )
				|| (inf.pMaterial == nullptr)
				|| !(inf.pMaterial->m_attrSurface.flagsShading
												& flagsRegFlags) )
			{
				if ( iLast < iMesh )
				{
					pModel->RenderBufferTo
						( &render, flagsExclusion, iLast, (ssize_t) iMesh,
							nInstancingCount, pmatInstancing, pclrInstancing ) ;
				}
				iLast = iMesh + 1 ;
			}
		}
		size_t	iEndMesh = iFirstMesh + nMeshCount ;
		if ( iLast < iEndMesh )
		{
			pModel->RenderBufferTo
				( &render, flagsExclusion, iLast, (ssize_t) iEndMesh,
					nInstancingCount, pmatInstancing, pclrInstancing ) ;
		}
	}
}

// 固有ボーダーパラメータ
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::SetOffsetBorderParameter
	( const S3DScene::OffsetBorderParam& borderParam, bool fEnableLocalBorder )
{
	m_flagLocalBorder = fEnableLocalBorder ;
	m_borderParam = borderParam ;
}

bool S3DScene::ModelItem::GetOffsetBorderParameter
	( S3DScene::OffsetBorderParam& borderParam ) const
{
	borderParam = m_borderParam ;
	return	m_flagLocalBorder ;
}

void S3DScene::ModelItem::EnableLocalBorderParameter( bool fEnable )
{
	m_flagLocalBorder = fEnable ;
}

bool S3DScene::ModelItem::IsEnabledLocalBorderParameter( void ) const
{
	return	m_flagLocalBorder ;
}

// シェーダー設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ModelItem::PrepareShaderSettings
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		S3DScene::ModelItem::ShaderSaver& ss ) const
{
	render.PushTransformation() ;
	//
	ss.flagsShading = render.GetShadingFlag() ;
	ss.flagsLocalShading = m_space.EffectedShaderFlags( ss.flagsShading, 0 ) ;
	if ( m_pLocalShader != nullptr )
	{
		render.AttachCustomShader( m_pLocalShader ) ;
	}
	if ( ss.flagsLocalShading != ss.flagsShading )
	{
		render.SetShadingFlag( ss.flagsLocalShading ) ;
	}
	size_t	i, nCount ;
	nCount = m_uniforms.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		S3DCustomShader::UniformData *	pud = m_uniforms.GetAt( i ) ;
		const SString *					pid = m_uniforms.GetTagAt( i ) ;
		if ( pud && pid )
		{
			render.SetCustomShaderUniform
				( *pid, pud->m_type, pud->m_pData, pud->m_nLength ) ;
		}
	}
	if ( m_flagLocalBorder )
	{
		render.SetOffsetBorderColor( m_borderParam.rgbBorder ) ;
		render.SetOffsetBorderCoefficient
				( m_borderParam.aThickness, m_borderParam.bThickness ) ;
	}
}

void S3DScene::ModelItem::RestoreShaderSettings
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		const S3DScene::ModelItem::ShaderSaver& ss ) const
{
	render.PopTransformation() ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::ModelItem::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnSave( file ) ;
	//
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	SString		strID = posm->GetIdentityOf
							( (S3DRenderBufferInterface*) m_pModel ) ;
	file.WriteString( strID ) ;
	strID = posm->GetIdentityOf
				( (S3DRenderBufferInterface*) m_pCollision ) ;
	file.WriteString( strID ) ;
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::ModelItem::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnRestore( file ) ;
	//
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	SString		strID ;
	file.ReadString( strID ) ;
	m_pModel = ESLTypeCast<S3DVertexBufferInterface>
								( posm->GetObjectOf( strID ) ) ;
	//
	file.ReadString( strID ) ;
	m_pCollision = ESLTypeCast<S3DVertexBufferInterface>
								( posm->GetObjectOf( strID ) ) ;
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// ビルボードアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::BillboardItem, Item )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::BillboardItem::BillboardItem( void )
{
	m_flagsZBuf = 0 ;
}

// 表示画像関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DScene::BillboardItem::AttachImage
	( SGLImageObject * pImage,
		size_t iFrame, uint64_t flagsShading,
		double xCenter, double yCenter,
		double xZoom, double yZoom, double zAngle, double zBias )
{
	m_bp.pImage = pImage ;
	m_bp.vCenter.x = (float32_t) xCenter ;
	m_bp.vCenter.y = (float32_t) yCenter ;
	m_bp.vZoom.x = (float32_t) xZoom ;
	m_bp.vZoom.y = (float32_t) yZoom ;
	m_bp.zAngle = (float32_t) zAngle ;
	m_bp.zBias = (float32_t) zBias ;
	//
	m_flagsZBuf = flagsShading ;
	//
	m_bufPoints.RemoveAll() ;
	m_bufFrames.RemoveAll() ;
	m_bufColors.RemoveAll() ;
	m_bufZooms.RemoveAll() ;
	m_bufXAspects.RemoveAll() ;
	m_bufFaceDirs.RemoveAll() ;
	//
	S3DVector4 *	pvPoints = m_bufPoints.GetArray( 1 ) ;
	size_t *		pFrames = m_bufFrames.GetArray( 1 ) ;
	//
	pvPoints[0].x = 0 ;
	pvPoints[0].y = 0 ;
	pvPoints[0].z = 0 ;
	//
	pFrames[0] = iFrame ;
	//
	m_bufPoints.FinishArray() ;
	m_bufFrames.FinishArray() ;
}

// パーティクル表示設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::BillboardItem::AttachParticle
	( const S3DMeshShaper::BillboardParam& bp,
		uint64_t flagsShading, size_t nCount,
		const S3DVector4 * pvPoints,
		const size_t * pFrames,
		const S3DColor * pColors,
		const float32_t * pZooms,
		const S4DVector * pFaceDirs,
		const float32_t * pxAspect )
{
	m_bp = bp ;
	m_flagsZBuf = flagsShading ;
	//
	m_bufPoints.RemoveAll() ;
	m_bufFrames.RemoveAll() ;
	m_bufColors.RemoveAll() ;
	m_bufZooms.RemoveAll() ;
	m_bufXAspects.RemoveAll() ;
	m_bufFaceDirs.RemoveAll() ;
	//
	#if	defined(__DEBUG__)
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( !pvPoints[i].IsNaN() ) ;
	}
	#endif
	m_bufPoints.AddArray( pvPoints, nCount ) ;
	//
	if ( pFrames != nullptr )
	{
		m_bufFrames.AddArray( pFrames, nCount ) ;
	}
	else
	{
		m_bufFrames.SetLength( nCount ) ;
	}
	if ( pColors != nullptr )
	{
		m_bufColors.AddArray( pColors, nCount ) ;
	}
	if ( pZooms != nullptr )
	{
		m_bufZooms.AddArray( pZooms, nCount ) ;
	}
	if ( pFaceDirs != nullptr )
	{
		#if	defined(__DEBUG__)
		for ( size_t i = 0; i < nCount; i ++ )
		{
			ESLAssert( !pFaceDirs[i].IsNaN() ) ;
		}
		#endif
		m_bufFaceDirs.AddArray( pFaceDirs, nCount ) ;
	}
	if ( pxAspect != nullptr )
	{
		m_bufXAspects.AddArray( pxAspect, nCount ) ;
	}
}

// パーティクル追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::BillboardItem::AddParticle
	( size_t nCount,
		const S3DVector4 * pvPoints, const size_t * pFrames,
		const S3DColor * pColors, const float32_t * pZooms,
		const S4DVector * pFaceDirs, const float32_t * pxAspect )
{
	if ( nCount == 0 )
	{
		return ;
	}
	size_t	nBaseLength = m_bufPoints.GetLength() ;
	//
	ESLAssert( pvPoints != nullptr ) ;
	#if	defined(__DEBUG__)
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( !pvPoints[i].IsNaN() ) ;
	}
	#endif
	m_bufPoints.AddArray( pvPoints, nCount ) ;
	//
	if ( pFrames != nullptr )
	{
		m_bufFrames.SetLength( nBaseLength ) ;
		m_bufFrames.AddArray( pFrames, nCount ) ;
	}
	else
	{
		m_bufFrames.SetLength( nBaseLength + nCount ) ;
	}
	if ( pColors != nullptr )
	{
		if ( m_bufColors.GetLength() < nBaseLength )
		{
			FitColorArray( nBaseLength ) ;
		}
		m_bufColors.AddArray( pColors, nCount ) ;
	}
	else if ( m_bufColors.GetLength() > 0 )
	{
		FitColorArray( nBaseLength + nCount ) ;
	}
	if ( pZooms != nullptr )
	{
		if ( m_bufZooms.GetLength() < nBaseLength )
		{
			FitZoomArray( nBaseLength ) ;
		}
		m_bufZooms.AddArray( pZooms, nCount ) ;
	}
	else if ( m_bufZooms.GetLength() > 0 )
	{
		FitZoomArray( nBaseLength + nCount ) ;
	}
	if ( pxAspect != nullptr )
	{
		if ( m_bufXAspects.GetLength() < nBaseLength )
		{
			FitAspectArray( nBaseLength ) ;
		}
		m_bufXAspects.AddArray( pxAspect, nCount ) ;
	}
	else if ( m_bufXAspects.GetLength() > 0 )
	{
		FitAspectArray( nBaseLength + nCount ) ;
	}
	if ( pFaceDirs != nullptr )
	{
		#if	defined(__DEBUG__)
		for ( size_t i = 0; i < nCount; i ++ )
		{
			ESLAssert( !pFaceDirs[i].IsNaN() ) ;
		}
		#endif
		if ( m_bufFaceDirs.GetLength() < nBaseLength )
		{
			FitFaceDirArray( nBaseLength ) ;
		}
		m_bufFaceDirs.AddArray( pFaceDirs, nCount ) ;
	}
	else if ( m_bufFaceDirs.GetLength() > 0 )
	{
		FitFaceDirArray( nBaseLength + nCount ) ;
	}
}

void S3DScene::BillboardItem::FitColorArray( size_t nLength )
{
	size_t		nLastLen = m_bufColors.GetLength() ;
	S3DColor *	pBufColor = m_bufColors.GetArray( nLength ) ;
	S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
	for ( size_t i = nLastLen; i < nLength; i ++ )
	{
		pBufColor[i] = clrDummy ;
	}
	m_bufColors.FinishArray() ;
}

void S3DScene::BillboardItem::FitZoomArray( size_t nLength )
{
	size_t		nLastLen = m_bufZooms.GetLength() ;
	float32_t *	pBufZoom = m_bufZooms.GetArray( nLength ) ;
	for ( size_t i = nLastLen; i < nLength; i ++ )
	{
		pBufZoom[i] = 1.0f ;
	}
	m_bufZooms.FinishArray() ;
}

void S3DScene::BillboardItem::FitAspectArray( size_t nLength )
{
	size_t		nLastLen = m_bufXAspects.GetLength() ;
	float32_t *	pBufAspect = m_bufXAspects.GetArray( nLength ) ;
	for ( size_t i = nLastLen; i < nLength; i ++ )
	{
		pBufAspect[i] = 1.0f ;
	}
	m_bufXAspects.FinishArray() ;
}

void S3DScene::BillboardItem::FitFaceDirArray( size_t nLength )
{
	size_t		nLastLen = m_bufFaceDirs.GetLength() ;
	S4DVector *	pBufFaceDir = m_bufFaceDirs.GetArray( nLength ) ;
	S4DVector	vDummy( 0, 0, 1.0f, 0.0f ) ;
	for ( size_t i = nLastLen; i < nLength; i ++ )
	{
		pBufFaceDir[i] = vDummy ;
	}
	m_bufFaceDirs.FinishArray() ;
}

// パーティクルリセット
//////////////////////////////////////////////////////////////////////////////
void S3DScene::BillboardItem::ClearAllParticles( void )
{
	m_bufPoints.RemoveAll() ;
	m_bufFrames.RemoveAll() ;
	m_bufColors.RemoveAll() ;
	m_bufZooms.RemoveAll() ;
	m_bufXAspects.RemoveAll() ;
	m_bufFaceDirs.RemoveAll() ;
}

// レンダリングの為のデバイスリソース準備
//////////////////////////////////////////////////////////////////////////////
void S3DScene::BillboardItem::PrepareToRender
	( S3DRenderDevice * pDevice, uint32_t nFlags )
{
	if ( m_bp.pImage != nullptr )
	{
		pDevice->CommitDeviceImage( m_bp.pImage ) ;
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::BillboardItem::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	if ( (m_flagsZBuf & flagsExclusion)
		|| (m_bufPoints.GetLength() == 0) )
	{
		return ;
	}
	S3DDMatrix	matSpace ;
	S3DDVector	vSpace ;
	CalcGlobalTransformation( matSpace, vSpace ) ;
	//
	S3DDMatrix	matISpace ;
	matISpace.InverseOf( matSpace ) ;
	//
	S3DMatrix	matICamera = matISpace * scene.GetCurrentCameraIMatrix() ;
	S3DVector	vCameraPos = scene.GetCurrentCameraPosition() ;
	//
	const S3DColor *	pColors = nullptr ;
	const float32_t *	pZooms = nullptr ;
	const float32_t *	pxAspects = nullptr ;
	const S4DVector *	pFaceDirs = nullptr ;
	if ( m_bufColors.GetLength() >= m_bufPoints.GetLength() )
	{
		pColors = m_bufColors.GetConstArray() ;
	}
	if ( m_bufZooms.GetLength() >= m_bufPoints.GetLength() )
	{
		pZooms = m_bufZooms.GetConstArray() ;
	}
	if ( m_bufXAspects.GetLength() >= m_bufPoints.GetLength() )
	{
		pxAspects = m_bufXAspects.GetConstArray() ;
	}
	if ( m_bufFaceDirs.GetLength() >= m_bufPoints.GetLength() )
	{
		pFaceDirs = m_bufFaceDirs.GetConstArray() ;
	}
	m_meshShaper.AnimationBillboardParticle
		( render, matICamera, vCameraPos,
			m_flagsZBuf, m_bp,
			m_bufPoints.GetLength(),
			m_bufPoints.GetConstArray(),
			m_bufFrames.GetConstArray(),
			pColors, pZooms, pFaceDirs, pxAspects ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::BillboardItem::OnSave( SSystem::SFileInterface& file )
{
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLError	err = Item::OnSave( file ) ;
	//
	SString		strID = posm->GetIdentityOf( m_bp.pImage ) ;
	file.WriteString( strID ) ;
	file.Write( &m_bp, sizeof(S3DMeshShaper::BillboardParam) ) ;
	file.Write( &m_flagsZBuf, sizeof(uint64_t) ) ;
	//
	uint32_t	nCount = (uint32_t) m_bufPoints.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	file.Write( m_bufPoints.GetConstArray(), nCount * sizeof(S3DVector4) ) ;
	//
	nCount = (uint32_t) m_bufFrames.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	file.Write( m_bufFrames.GetConstArray(), nCount * sizeof(size_t) ) ;
	//
	nCount = (uint32_t) m_bufColors.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	file.Write( m_bufColors.GetConstArray(), nCount * sizeof(S3DColor) ) ;
	//
	nCount = (uint32_t) m_bufZooms.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	file.Write( m_bufZooms.GetConstArray(), nCount * sizeof(float32_t) ) ;
	//
	nCount = (uint32_t) m_bufFaceDirs.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	file.Write( m_bufFaceDirs.GetConstArray(), nCount * sizeof(S4DVector) ) ;
	//
	nCount = (uint32_t) m_bufXAspects.GetLength() ;
	file.Write( &nCount, sizeof(uint32_t) ) ;
	file.Write( m_bufXAspects.GetConstArray(), nCount * sizeof(float32_t) ) ;
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::BillboardItem::OnRestore( SSystem::SFileInterface& file )
{
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLError	err = Item::OnRestore( file ) ;
	//
	SString		strID ;
	file.ReadString( strID ) ;
	file.Read( &m_bp, sizeof(S3DMeshShaper::BillboardParam) ) ;
	m_bp.pImage = ESLTypeCast<SGLImageObject>( posm->GetObjectOf( strID ) ) ;
	file.Read( &m_flagsZBuf, sizeof(uint64_t) ) ;
	//
	uint32_t	nCount ;
	m_bufPoints.RemoveAll() ;
	m_bufFrames.RemoveAll() ;
	m_bufColors.RemoveAll() ;
	m_bufZooms.RemoveAll() ;
	m_bufXAspects.RemoveAll() ;
	m_bufFaceDirs.RemoveAll() ;
	//
	file.Read( &nCount, sizeof(uint32_t) ) ;
	file.Read( m_bufPoints.GetArray(nCount), nCount * sizeof(S3DVector4) ) ;
	m_bufPoints.FinishArray() ;
	//
	file.Read( &nCount, sizeof(uint32_t) ) ;
	file.Read( m_bufFrames.GetArray(nCount), nCount * sizeof(size_t) ) ;
	m_bufFrames.FinishArray() ;
	//
	file.Read( &nCount, sizeof(uint32_t) ) ;
	file.Read( m_bufColors.GetArray(nCount), nCount * sizeof(S3DColor) ) ;
	m_bufColors.FinishArray() ;
	//
	file.Read( &nCount, sizeof(uint32_t) ) ;
	file.Read( m_bufZooms.GetArray(nCount), nCount * sizeof(float32_t) ) ;
	m_bufZooms.FinishArray() ;
	//
	file.Read( &nCount, sizeof(uint32_t) ) ;
	file.Read( m_bufFaceDirs.GetArray(nCount), nCount * sizeof(S4DVector) ) ;
	m_bufFaceDirs.FinishArray() ;
	//
	file.Read( &nCount, sizeof(uint32_t) ) ;
	file.Read( m_bufXAspects.GetArray(nCount), nCount * sizeof(float32_t) ) ;
	m_bufXAspects.FinishArray() ;
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// サウンドアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::SoundItem, Item )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::SoundItem::SoundItem( void )
	: m_vDirection( 0, 0, 1 )
{
	m_flagsBehavior |= itemTimer ;
	m_nSoundFlags = 0 ;
	m_fpVolume = 1.0 ;
	m_fpFadeReach = 30.0 ;
	m_fpFadeLatitude = 10.0 ;
	m_fpAttenuationPower = 1.0 ;
	m_fpAttenuationDistance = 1.0 ;
	m_fpConeAngle = 0 ;
	m_fpAngleGradation = -1 ;
	m_fpBaseVolume = 0.1 ;
}

// AudioPlayer 設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::AttachAudioPlayer( SGLAudioPlayerInterface * pPlayer )
{
	m_refPlayer.SetReference( pPlayer ) ;
}

void S3DScene::SoundItem::SetSmartAudioPlayer( SGLAudioPlayerInterface * pPlayer )
{
	m_refPlayer.SetSmartReference( pPlayer ) ;
}

// AudioPlayer 取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * S3DScene::SoundItem::GetAudioPlayer( void ) const
{
	return	m_refPlayer.GetReference() ;
}

// フラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::SoundItem::GetSoundFlags( void ) const
{
	return	m_nSoundFlags ;
}

void S3DScene::SoundItem::SetSoundFlags( uint32_t nFlags )
{
	m_nSoundFlags = nFlags ;
}

// 音量
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::SetVolume( double fpVolume )
{
	m_fpVolume = fpVolume ;
}

double S3DScene::SoundItem::GetVolume( void ) const
{
	return	m_fpVolume ;
}

// 有効距離
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::SetFadeReach( double fpReach, double fpLatitude )
{
	m_fpFadeReach = fpReach ;
	m_fpFadeLatitude = fpLatitude ;
}

double S3DScene::SoundItem::GetFadeReach( void ) const
{
	return	m_fpFadeReach ;
}

double S3DScene::SoundItem::GetFadeLatitude( void ) const
{
	return	m_fpFadeLatitude ;
}

// 距離減衰率
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::SetAttenuationPower
					( double fpPower, double fpDistance )
{
	m_fpAttenuationPower = fpPower ;
	m_fpAttenuationDistance = fpDistance ;
}

double S3DScene::SoundItem::GetAttenuationPower( void ) const
{
	return	m_fpAttenuationPower ;
}

double S3DScene::SoundItem::GetAttenuationDistance( void ) const
{
	return	m_fpAttenuationDistance ;
}

// 指向性
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::SetDirection
	( const S3DDVector& vDir,
		double degCone, double degGrad, double volBase )
{
	m_vDirection = vDir ;
	m_fpConeAngle = cos( degCone * PI / 180.0 ) ;
	m_fpAngleGradation =
			cos( (degCone + degGrad) * PI / 180.0 ) - m_fpConeAngle ;
	m_fpBaseVolume = volBase ;
}

const S3DDVector& S3DScene::SoundItem::GetDirection( void ) const
{
	return	m_vDirection ;
}

double S3DScene::SoundItem::GetDirectionConeAngle( void ) const
{
	return	acos( m_fpConeAngle ) * 180.0 / PI ;
}

double S3DScene::SoundItem::GetDirectionGradation( void ) const
{
	return	acos( m_fpConeAngle + m_fpAngleGradation )
								* 180.0 / PI - GetDirectionConeAngle() ;
}

double S3DScene::SoundItem::GetDirectionBaseVolume( void ) const
{
	return	m_fpBaseVolume ;
}

// 音量反映
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::ApplySoundVolume( S3DScene::Camera * pCamera ) const
{
	SGLAudioPlayerInterface *	pPlayer = m_refPlayer.GetReference() ;
	if ( (pCamera != nullptr) && (pPlayer != nullptr) )
	{
		//
		// 行列計算
		//
		S3DDMatrix	matCamera ;
		S3DDVector	vCamera ;
		S3DScene::CalcCameraTransformation( matCamera, vCamera, pCamera ) ;
		//
		S3DDMatrix	matItem ;
		S3DDVector	vItem ;
		CalcGlobalTransformation( matItem, vItem ) ;
		//
		// 音量反映
		//
		ApplySoundVolume( matCamera, vCamera, matItem, vItem, pCamera, 1.0, pPlayer ) ;
	}
}

void S3DScene::SoundItem::ApplySoundVolume
	( const S3DDMatrix& matCamera, const S3DDVector& vCamera,
		const S3DDMatrix& matItem, const S3DDVector& vItem,
		S3DScene::Camera * pCamera,
		double fpItemVol, SGLAudioPlayerInterface * pPlayer ) const
{
	float32_t	vols[2] ;
	CalcSoundVolume
		( vols, matCamera, vCamera, matItem, vItem, pCamera, fpItemVol ) ;

	ESLAssert( pPlayer != nullptr ) ;
	pPlayer->SetVolume( vols, 2 ) ;
}

// 音量効果計算（このアイテムのパラメータと引数のみを使用）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::CalcSoundVolume
	( float32_t vols[],
		const S3DDMatrix& matCamera,
		const S3DDVector& vCamera,
		const S3DDMatrix& matItem,
		const S3DDVector& vItem,
		S3DScene::Camera * pCamera, double fpItemVol ) const
{
	//
	// 座標変換
	//
	S3DDMatrix	matViewItem = matCamera * matItem ;
	S3DDVector	vViewItem = matCamera * vItem - vCamera ;

	S3DDVector	vDelta = vViewItem ;
	vDelta.Normalize() ;
	//
	// 発生源音量計算
	//
	double	fpVolume = m_fpVolume * fpItemVol ;
	if ( !(m_nSoundFlags & soundEnvironment) )
	{
		if ( m_nSoundFlags & soundDirectional )
		{
			// 指向性音源
			S3DDVector	vDir = matViewItem * m_vDirection ;
			vDir.Normalize() ;
			//
			double	cosDir = - vDelta.InnerProduct( vDir ) ;
			if ( cosDir < m_fpConeAngle )
			{
				if ( cosDir < m_fpConeAngle + m_fpAngleGradation )
				{
					// 範囲外
					fpVolume = m_fpBaseVolume ;
				}
				else
				{
					// ぼかし
					fpVolume -=
						(cosDir - m_fpConeAngle) / m_fpAngleGradation
							* (m_fpVolume * fpItemVol * (1.0 - m_fpBaseVolume)) ;
				}
			}
		}
		//
		// 距離による減衰
		//
		if ( m_fpAttenuationPower > 0.0 )
		{
			double	r = vViewItem.Absolute() ;
			if ( r > 1.0e-8 )
			{
				fpVolume *=
					pow( m_fpAttenuationDistance / r, m_fpAttenuationPower ) ;
			}
		}
	}
	//
	// マイク毎の音量
	//
	if ( !(m_nSoundFlags & soundEnvironment) )
	{
		S3DDVector	vLeftEarDir( -1, 0, 1 ) ;
		S3DDVector	vRightEarDir( 1, 0, 1 ) ;
		double		cosLow = -1 ;
		if ( pCamera != nullptr )
		{
			vLeftEarDir = pCamera->m_vLeftEarDir ;
			vRightEarDir = pCamera->m_vRightEarDir ;
			cosLow = pCamera->m_cosSoundLow ;
		}
		vLeftEarDir.Normalize() ;
		vRightEarDir.Normalize() ;
		//
		double	cosLeft = vLeftEarDir.InnerProduct( vDelta ) ;
		double	cosRight = vRightEarDir.InnerProduct( vDelta ) ;
		double	fpVolScale = fpVolume / (1.0 - cosLow) ;
		double	fpLeftVol =
					(esl_fmax( cosLeft, cosLow ) - cosLow) * fpVolScale ;
		double	fpRightVol =
					(esl_fmax( cosRight, cosLow ) - cosLow) * fpVolScale ;
		//
		vols[0] = esl_fminf( (float32_t) fpLeftVol, 1.0f ) ;
		vols[1] = esl_fminf( (float32_t) fpRightVol, 1.0f ) ;
	}
	else
	{
		vols[0] = esl_fminf( (float32_t) fpVolume, 1.0f ) ;
		vols[1] = esl_fminf( (float32_t) fpVolume, 1.0f ) ;
	}
	if ( m_nSoundFlags & soundFadeReach )
	{
		double	r = vViewItem.Absolute() ;
		if ( r > m_fpFadeReach )
		{
			r -= m_fpFadeReach ;
			if ( r < m_fpFadeLatitude )
			{
				r = 1.0 - r / m_fpFadeLatitude ;
				vols[0] *= (float32_t) r ;
				vols[1] *= (float32_t) r ;
			}
			else
			{
				vols[0] = 0.0f ;
				vols[1] = 0.0f ;
			}
		}
	}
}

void S3DScene::SoundItem::CalcSoundVolume
	( float32_t vols[],
		Camera * pCamera,
		const S3DDMatrix& matItem,
		const S3DDVector& vItem, double fpItemVol ) const
{
	ESLAssert( pCamera != nullptr ) ;

	S3DDMatrix	matCamera ;
	S3DDVector	vCamera ;
	S3DScene::CalcCameraTransformation( matCamera, vCamera, pCamera ) ;

	CalcSoundVolume
		( vols, matCamera, vCamera, matItem, vItem, pCamera, fpItemVol ) ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SoundItem::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	SGLAudioPlayerInterface *	pPlayer = m_refPlayer.GetReference() ;
	Camera *	pCamera = scene.GetSoundCamera() ;
	if ( (pCamera != nullptr) && (pPlayer != nullptr) && pPlayer->IsPlaying() )
	{
		ApplySoundVolume( pCamera ) ;
	}
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::SoundItem::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnSave( file ) ;
	//
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	SString	strPlayerID = posm->GetIdentityOf( m_refPlayer.GetReference() ) ;
	file.WriteString( strPlayerID ) ;
	file.Write( &m_fpVolume, sizeof(m_fpVolume) ) ;
	file.Write( &m_fpAttenuationPower, sizeof(m_fpAttenuationPower) ) ;
	file.Write( &m_vDirection, sizeof(m_vDirection) ) ;
	file.Write( &m_fpConeAngle, sizeof(m_fpConeAngle) ) ;
	file.Write( &m_fpAngleGradation, sizeof(m_fpAngleGradation) ) ;
	file.Write( &m_fpBaseVolume, sizeof(m_fpBaseVolume) ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::SoundItem::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Item::OnRestore( file ) ;
	//
	SGLObjectSavingMapper *	posm = SGLObjectSavingMapper::GetCurrent() ;
	if ( posm == nullptr )
	{
		return	sglErrFailed ;
	}
	SString	strPlayerID ;
	file.ReadString( strPlayerID ) ;
	m_refPlayer.SetReference
		( ESLTypeCast<SGLAudioPlayerInterface>
						( posm->GetObjectOf( strPlayerID ) ) ) ;
	//
	file.Read( &m_fpVolume, sizeof(m_fpVolume) ) ;
	file.Read( &m_fpAttenuationPower, sizeof(m_fpAttenuationPower) ) ;
	file.Read( &m_vDirection, sizeof(m_vDirection) ) ;
	file.Read( &m_fpConeAngle, sizeof(m_fpConeAngle) ) ;
	file.Read( &m_fpAngleGradation, sizeof(m_fpAngleGradation) ) ;
	file.Read( &m_fpBaseVolume, sizeof(m_fpBaseVolume) ) ;
	//
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// 物理演算シーンアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DScene::PhysicsSceneItem, Item, S3DPhysicsScene )




//////////////////////////////////////////////////////////////////////////////
// 抽象効果
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::Effector, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::Effector::Effector( int priority )
	: m_priority( priority )
{
}

// 処理優先度
//////////////////////////////////////////////////////////////////////////////
int S3DScene::Effector::GetDrawingPriority( void ) const
{
	return m_priority ;
}

void S3DScene::Effector::SetDrawingPriority( int priority )
{
	m_priority = priority ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::Effector::OnTimer( S3DScene& scene, uint32_t msecPast )
{
}


// 擬似被写界深度効果
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::DepthOfFieldEffector, Effector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::DepthOfFieldEffector::DepthOfFieldEffector( void )
{
	m_pDevice = nullptr ;
	m_pRender = nullptr ;
	m_gauss = 4.0f ;
	m_zFocus = 1000.0f ;
	m_zNearDepth = 1000.0f ;
	m_zFarDepth = 2000.0f ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::DepthOfFieldEffector::~DepthOfFieldEffector( void )
{
	delete	m_pRender ;
	m_pRender = nullptr ;
}

// ぼかし度合い（ガウスパラメータ）設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::DepthOfFieldEffector::SetGauss( float32_t g )
{
	m_gauss = g ;
}

// ピント距離設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::DepthOfFieldEffector::SetFocus( float32_t zFocus )
{
	m_zFocus = zFocus ;
}

// 被写界深度設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::DepthOfFieldEffector::SetDepth
			( float32_t zNearDepth, float32_t zFarDepth )
{
	m_zNearDepth = zNearDepth ;
	m_zFarDepth = zFarDepth ;
}

// 要求カラーバッファ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::DepthOfFieldEffector::GetRequiredColorBufferMask( void ) const
{
	return	0 ;
}

// 画面効果
//////////////////////////////////////////////////////////////////////////////
void S3DScene::DepthOfFieldEffector::DrawEffect
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
#if	!defined(__COTOPHA__)
	S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
	if ( pDevice == nullptr )
	{
		return ;
	}
	S3DCustomShader *	pBlurShader =
			pDevice->GetDefaultShaderProgramAs
				( S3DRenderDevice::DefaultShaderId::GaussianBlur ) ;
	S3DGaussianBlurShaderInterface *
		pgbiBlur = ESLTypeCast<S3DGaussianBlurShaderInterface>( pBlurShader ) ;
	if ( pgbiBlur == nullptr )
	{
		ESLTrace( "DepthOfFieldEffector: not prepared gaussian blur shader.\n" ) ;
		return ;
	}
	S3DCustomShader *	pDOFShader =
			pDevice->GetDefaultShaderProgramAs
				( S3DRenderDevice::DefaultShaderId::DepthBlender ) ;
	S3DDepthBlenderShaderInterface *
		pdbsiDOF = ESLTypeCast<S3DDepthBlenderShaderInterface>( pDOFShader ) ;
	if ( pdbsiDOF == nullptr )
	{
		ESLTrace( "DepthOfFieldEffector: not prepared depth of field shader.\n" ) ;
		return ;
	}
	//
	// ぼかし処理
	//
	ESLAssert( ppImage != nullptr ) ;
	ESLAssert( nMultiImages >= 1 ) ;
	ESLAssert( ppImage[0] != nullptr ) ;
	ESLAssert( pDepth != nullptr ) ;
	if ( (ppImage[0] == nullptr) || (pDepth == nullptr) )
	{
		return ;
	}
	SGLSize	sizeImage = ppImage[0]->GetImageSize() ;
	SGLSize	sizeLastBufSize = m_imgBuffer[0].GetImageSize() ;
	if ( (sizeLastBufSize.w < sizeImage.w)
		| (sizeLastBufSize.h < sizeImage.h) )
	{
		SGLSize	sizeBufSize( esl_max( sizeImage.w, sizeLastBufSize.w ),
								esl_max( sizeImage.h, sizeLastBufSize.h ) ) ;
		m_imgBuffer[0].CreateImage
			( sizeBufSize.w, sizeBufSize.h,
				formatImageDefaultRGBA, 32,
				SGLImageObject::bufferOnDeviceOnly ) ;
		m_imgBuffer[1].CreateImage
			( sizeBufSize.w, sizeBufSize.h,
				formatImageDefaultRGBA, 32,
				SGLImageObject::bufferOnDeviceOnly ) ;
	}
	S3DRenderContextInterface *	pRender = m_pRender ;
	if ( (pRender == nullptr) || (m_pDevice != pDevice) )
	{
		pRender = pDevice->NewRenderer() ;
		if ( pRender == nullptr )
		{
			ESLTrace( "DepthOfFieldEffector: failed to create renderer.\n" ) ;
			return ;
		}
		delete	m_pRender ;
		m_pDevice = pDevice ;
		m_pRender = pRender ;
	}
	pgbiBlur->SetGauss( m_gauss ) ;
	pgbiBlur->SetDirection( 1, 0 ) ;
	pRender->AttachCustomShader( pBlurShader ) ;
	//
	SGLImageRect	rectView ;
	rectView.x = 0 ;
	rectView.y = 0 ;
	rectView.w = sizeImage.w ;
	rectView.h = sizeImage.h ;
	//
	SGLPaintParam	pp ;
	pRender->AttachTargetImage( &m_imgBuffer[0], nullptr, &rectView ) ;
	pRender->FillClearTarget( 0 ) ;
	pRender->DrawImage( pp, ppImage[0], nullptr ) ;
	pRender->DetachTargetImage( ) ;
	//
	pgbiBlur->SetDirection( 0, 1 ) ;
	pRender->AttachTargetImage( &m_imgBuffer[1], nullptr, &rectView ) ;
	pRender->FillClearTarget( 0 ) ;
	pRender->DrawImage( pp, &m_imgBuffer[0], nullptr ) ;
	pRender->DetachTargetImage( ) ;
	//
	pRender->AttachCustomShader( nullptr ) ;
	//
	// ぼかし合成描画
	//
	S3DCustomShader *	pOldShader = render.GetCustomShader() ;
	S4DMatrix	matPers = scene.GetCurrentPerspective() ;
	//
	pdbsiDOF->SetDepthBuffer( pDepth ) ;
	pdbsiDOF->SetFocusDepth( m_zFocus ) ;
	pdbsiDOF->SetDepthRange( m_zNearDepth, m_zFarDepth ) ;
	pdbsiDOF->SetPersParameter( matPers.m[2][2], matPers.m[2][3] ) ;
	//
	render.AttachCustomShader( pDOFShader ) ;
	//
	render.DrawImage( pp, &m_imgBuffer[1], &rectView ) ;
	//
	render.AttachCustomShader( pOldShader ) ;
#endif
}



//////////////////////////////////////////////////////////////////////////////
// 発光グロー効果
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::EmissiveGlowEffector, Effector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::EmissiveGlowEffector::EmissiveGlowEffector( void )
{
	m_pDevice = nullptr ;
	m_pRender = nullptr ;
	m_iSource = renderTargetEmission ;
	m_gauss = 12.0f ;
	m_zoom = 2.0f ;
	m_brightness = 1.0f ;
	m_alpha = 0.25f ;
	m_drawAdd = true ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::EmissiveGlowEffector::~EmissiveGlowEffector( void )
{
	delete	m_pRender ;
	m_pRender = nullptr ;
}

// ソース
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EmissiveGlowEffector::SetSourceIndex( int iSource )
{
	m_iSource = iSource ;
}

// ぼかし度合い（ガウスパラメータ）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EmissiveGlowEffector::SetGauss( float32_t g )
{
	m_gauss = g ;
}

// ぼかし拡大率（バッファ縮小率）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EmissiveGlowEffector::SetZoom( float32_t z )
{
	m_zoom = z ;
}

// 輝度
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EmissiveGlowEffector::SetBrightness( float32_t b )
{
	m_brightness = b ;
}

// 重ね度合い [0,1]
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EmissiveGlowEffector::SetAlpha( float32_t a )
{
	m_alpha = a ;
}

// 加算描画
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EmissiveGlowEffector::SetDrawAdd( bool a )
{
	m_drawAdd = a ;
}

// 要求カラーバッファ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::EmissiveGlowEffector::GetRequiredColorBufferMask( void ) const
{
	return	(1 << m_iSource) & ~0x0001 ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EmissiveGlowEffector::DrawEffect
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
	if ( (m_alpha <= 0.001) || (m_gauss <= 0.001) )
	{
		return ;
	}
#if	!defined(__COTOPHA__)
	S3DRenderDevice *	pDevice = render.GetRenderDeviceObject() ;
	if ( pDevice == nullptr )
	{
		return ;
	}
	S3DRenderingCapacity	capsRender ;
	render.GetRenderingCapacity( capsRender ) ;
	if ( (m_iSource >= 1)
		&& (!(capsRender.flagsExtensions1
				& S3DRenderingCapacity::extMultiRenderTarget)) )
	{
		return ;
	}
	S3DCustomShader *	pBlurShader =
			pDevice->GetDefaultShaderProgramAs
				( S3DRenderDevice::DefaultShaderId::GaussianBlur ) ;
	S3DGaussianBlurShaderInterface *
		pgbiBlur = ESLTypeCast<S3DGaussianBlurShaderInterface>( pBlurShader ) ;
	if ( pgbiBlur == nullptr )
	{
		ESLTrace( "EmissiveGlowEffector: not prepared gaussian blur shader.\n" ) ;
		return ;
	}
	//
	// ぼかし処理
	//
	ESLAssert( ppImage != nullptr ) ;
	ESLAssert( pDepth != nullptr ) ;
	if ( (nMultiImages <= (size_t) m_iSource)
		|| (ppImage[m_iSource] == nullptr) )
	{
		return ;
	}
	SGLSize	sizeImage = ppImage[m_iSource]->GetImageSize() ;
	SGLSize	sizeImage0
		( eslRoundR32ToInt( (float32_t) sizeImage.w / m_zoom ), sizeImage.h ) ;
	SGLSize	sizeImage1
		( sizeImage0.w, eslRoundR32ToInt( (float32_t) sizeImage0.h / m_zoom ) ) ;
	SGLSize	sizeLastBufSize0 = m_imgBuffer[0].GetImageSize() ;
	SGLSize	sizeLastBufSize1 = m_imgBuffer[1].GetImageSize() ;
	if ( (sizeLastBufSize0.w < sizeImage0.w)
		| (sizeLastBufSize0.h < sizeImage0.h) )
	{
		m_imgBuffer[0].CreateImage
			( esl_max( sizeImage0.w, sizeLastBufSize0.w ),
				esl_max( sizeImage0.h, sizeLastBufSize0.h ),
				formatImageDefaultRGBA, 32,
				SGLImageObject::bufferOnDeviceOnly ) ;
	}
	if ( (sizeLastBufSize1.w < sizeImage1.w)
		| (sizeLastBufSize1.h < sizeImage1.h) )
	{
		m_imgBuffer[1].CreateImage
			( esl_max( sizeImage1.w, sizeLastBufSize1.w ),
				esl_max( sizeImage1.h, sizeLastBufSize1.h ),
				formatImageDefaultRGBA, 32,
				SGLImageObject::bufferOnDeviceOnly ) ;
	}
	S3DRenderContextInterface *	pRender = m_pRender ;
	if ( (pRender == nullptr) || (m_pDevice != pDevice) )
	{
		pRender = pDevice->NewRenderer() ;
		if ( pRender == nullptr )
		{
			ESLTrace( "EmissiveGlowEffector: failed to create renderer.\n" ) ;
			return ;
		}
		delete	m_pRender ;
		m_pDevice = pDevice ;
		m_pRender = pRender ;
	}
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	SGLPoint		ptZero( 0, 0 ) ;
	float32_t		zoomHorz = (float32_t) sizeImage0.w / (float32_t) sizeImage.w ;
	float32_t		zoomVert = (float32_t) sizeImage1.h / (float32_t) sizeImage0.h ;
	float32_t		xOdd = zoomHorz * 0.5f ;
	float32_t		yOdd = zoomVert * 0.5f ;
	//
	pgbiBlur->SetGauss( m_gauss ) ;
	pgbiBlur->SetBrightness( m_brightness ) ;
	pgbiBlur->SetDirection( 1.0 / zoomHorz, 0 ) ;
	pRender->AttachCustomShader( pBlurShader ) ;
	//
	SGLImageRect	rectView0( ptZero, sizeImage0 ) ;
	pp.nFlags = paintSmoothStretch ;
	pp.SetAffine( affine, xOdd, 0, 0, 0, zoomHorz, 1.0 ) ;
	pRender->AttachTargetImage( &m_imgBuffer[0], nullptr, &rectView0 ) ;
	pRender->FillClearTarget( 0 ) ;
	pRender->DrawImage( pp, ppImage[m_iSource], nullptr ) ;
	pRender->DetachTargetImage( ) ;
	//
	SGLImageRect	rectView1( ptZero, sizeImage1 ) ;
	pgbiBlur->SetDirection( 0, 1.0 / zoomVert ) ;
	pp.SetAffine( affine, 0, yOdd, 0, 0, 1.0, zoomVert ) ;
	pRender->AttachTargetImage( &m_imgBuffer[1], nullptr, &rectView1 ) ;
	pRender->FillClearTarget( 0 ) ;
	pRender->DrawImage( pp, &m_imgBuffer[0], nullptr ) ;
	pRender->DetachTargetImage( ) ;
	//
	pRender->AttachCustomShader( nullptr ) ;
	//
	// 描画
	//
	pp.nFlags = paintSmoothStretch ;
	if ( m_drawAdd )
	{
		pp.nFlags |= paintFunctionAdd ;
	}
	pp.SetAffine( affine, 0, 0, 0, 0, 1.0f / zoomHorz, 1.0f / zoomVert ) ;
	pp.nTransparency =
		(uint32_t) esl_clampi
			( eslRoundR32ToInt( (1.0f - m_alpha) * 0x100 ), 0, 0x100 ) ;
	render.DrawImage( pp, &m_imgBuffer[1], &rectView1 ) ;
#endif
}



//////////////////////////////////////////////////////////////////////////////
// ぼかし・フラッシュ効果
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::CurtainEffector, EmissiveGlowEffector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::CurtainEffector::CurtainEffector( void )
	: m_argbCurtain( 0x00FFFFFF ),
		m_nCurtainAlpha( 0 ), m_argbEffectColor( 0 )
{
	SetSourceIndex( renderTargetComposed ) ;
	SetAlpha( 1.0 ) ;
	SetDrawAdd( false ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::CurtainEffector::~CurtainEffector( void )
{
}

// フラッシュ効果色
//////////////////////////////////////////////////////////////////////////////
void S3DScene::CurtainEffector::SetCurtainColor( const SGLPalette& argb )
{
	m_argbCurtain = argb ;
}

const SGLPalette& S3DScene::CurtainEffector::GetCurtainColor( void ) const
{
	return	m_argbCurtain ;
}

// フラッシュ不透明度
//////////////////////////////////////////////////////////////////////////////
void S3DScene::CurtainEffector::SetCurtainAlpha( uint32_t a )
{
	m_nCurtainAlpha = a ;
}

uint32_t S3DScene::CurtainEffector::GetCurtainAlpha( void ) const
{
	return	m_nCurtainAlpha ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::CurtainEffector::DrawEffect
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
	EmissiveGlowEffector::DrawEffect
		( scene, render, ppImage, nMultiImages, pDepth ) ;
	//
	uint32_t	argbCurtain =
		sglPackedColorMul
			( m_argbCurtain,
				(uint32_t) esl_clampi( (int) m_nCurtainAlpha, 0, 0x100 ) ) ;
	argbCurtain =
		sglPackedColorBlend( argbCurtain, m_argbEffectColor.ui32 ) ;
	if ( argbCurtain == 0 )
	{
		return ;
	}
	if ( (nMultiImages == 0) || (ppImage[0] == nullptr) )
	{
		return ;
	}
	SGLSize	sizeImage = ppImage[0]->GetImageSize() ;
	render.FillRectangle
		( -10, -10, sizeImage.w+10, sizeImage.h+10, argbCurtain ) ;
}



//////////////////////////////////////////////////////////////////////////////
// レイヤード効果描画
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::LayeredEffector, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::LayeredEffector::LayeredEffector( void )
{
	m_application = 0.5 ;
}

// 適用度
//////////////////////////////////////////////////////////////////////////////
double S3DScene::LayeredEffector::GetEffectApplication( void ) const
{
	return	m_application ;
}

void S3DScene::LayeredEffector::SetEffectApplication( double apply )
{
	m_application = apply ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredEffector::OnTimer( S3DScene& scene, uint32_t msecPast )
{
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredEffector::DrawLayer
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
	if ( (nMultiImages >= 1) && ppImage && ppImage[0] )
	{
		SGLPaintParam	pp ;
		pp.nTransparency =
			eslRoundR32ToInt( (float32_t) (m_application * 0x100) ) ;
		render.DrawImage( pp, ppImage[0] ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ変形子
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::MeshOperator, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::MeshOperator::MeshOperator( void )
{
}

// 座標変位
//////////////////////////////////////////////////////////////////////////////
void S3DScene::MeshOperator::WarpMesh
	( S3DVector4 * pvDst, S3DColor * pColor,
		const S2DVector * pvSrc, const SGLSize& sizeMesh )
{
	size_t	wMesh = (size_t) sizeMesh.w ;
	size_t	hMesh = (size_t) sizeMesh.h ;
	size_t	i = 0 ;
	for ( size_t y = 0; y <= hMesh; y ++ )
	{
		for ( size_t x = 0; x <= wMesh; x ++, i ++ )
		{
			WarpVertex
				( pvDst + i, pColor + i, pvSrc + i, sizeMesh, i ) ;
		}
	}
}

void S3DScene::MeshOperator::WarpVertex
	( S3DVector4 * pvDst,
		S3DColor * pColor, const S2DVector * pvSrc,
		const SGLSize& sizeMesh, size_t iVertex )
{
}



//////////////////////////////////////////////////////////////////////////////
// メッシュ変形レイヤード効果描画
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene::LayeredMeshEffector, LayeredEffector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::LayeredMeshEffector::LayeredMeshEffector( void )
{
}

// メッシュ分割数設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredMeshEffector::SetMeshDivision( const SGLSize& sizeDivMesh )
{
	m_sizeMesh = sizeDivMesh ;
}

// メッシュ変形子追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredMeshEffector::AddOperator( S3DScene::MeshOperator * pOperator )
{
	m_operators.Add( pOperator ) ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::LayeredMeshEffector::DrawLayer
	( const S3DScene& scene,
		S3DRenderContextInterface& render,
		SGLImageObject*const* ppImage,
		size_t nMultiImages, SGLImageObject * pDepth )
{
	if ( (m_sizeMesh.w == 0)
		|| (m_sizeMesh.h == 0) )
	{
		return ;
	}
	if ( (nMultiImages < 1) || !ppImage || !ppImage[0] )
	{
		return ;
	}
	//
	// メッシュ基本パラメータ
	//
	size_t			wMesh = (size_t) m_sizeMesh.w ;
	size_t			hMesh = (size_t) m_sizeMesh.h ;
	size_t			nVertex = (wMesh + 1) * (hMesh + 1) ;
	S2DVector *		pvSrc = m_vSrcMesh.GetArray( nVertex ) ;
	S3DVector4 *	pvDst = m_vDstMesh.GetArray( nVertex ) ;
	S3DColor *		pColor = m_vDstColor.GetArray( nVertex ) ;
	//
	SGLImageObject *	pLayeredImage = ppImage[0] ;
	SGLSize		sizeImage = pLayeredImage->GetImageSize() ;
	float32_t	xDelta = (float32_t) sizeImage.w / (float32_t) m_sizeMesh.w ;
	float32_t	yDelta = (float32_t) sizeImage.h / (float32_t) m_sizeMesh.h ;
	//
	S3DScene::ProjectionParam	pp ;
	scene.GetProjection( pp ) ;
	//
	float32_t	zMap = pp.vScreen.z * pp.fpZoom ;
	float32_t	yMap = 0.0f ;
	for ( size_t y = 0; y <= hMesh; y ++, yMap += yDelta )
	{
		float32_t	xMap = 0.0f ;
		for ( size_t x = 0; x <= wMesh; x ++, xMap += xDelta )
		{
			pvSrc->x = xMap ;
			pvSrc->y = yMap ;
			//
			pvDst->x = xMap ;
			pvDst->y = yMap ;
			pvDst->z = zMap ;
			pvDst->d = 0.0f ;
			//
			pColor->rgbMul.ui32 = 0xFFFFFFFF ;
			pColor->rgbAdd.ui32 = 0 ;
			//
			pvSrc ++ ;
			pvDst ++ ;
			pColor ++ ;
		}
	}
	m_vSrcMesh.FinishArray() ;
	m_vDstMesh.FinishArray() ;
	m_vDstColor.FinishArray() ;
	//
	// 変形処理
	//
	pvSrc = m_vSrcMesh.GetArray() ;
	pvDst = m_vDstMesh.GetArray() ;
	pColor = m_vDstColor.GetArray() ;
	//
	for ( size_t i = 0; i < m_operators.GetLength(); i ++ )
	{
		MeshOperator *	pmo = m_operators.GetAt( i ) ;
		ESLAssert( pmo != nullptr ) ;
		if ( pmo != nullptr )
		{
			pmo->WarpMesh( pvDst, pColor, pvSrc, m_sizeMesh ) ;
		}
	}
	//
	// 表面属性
	//
	S3DSurfaceAttribute	attr ;
	attr.flagsShading =
			shadingMethodNothing
				| shadingTextureMapping | shadingTextureSmoothing
				| shadingNoZBuffer | shadingVertexAlpha ;
	m_material.SetSurfaceAttribute( attr ) ;
	m_material.SetTexture( pLayeredImage ) ;
	//
	// インデックス生成
	//
	size_t		nPolygons = wMesh * hMesh * 2 ;
	uint32_t *	pIndexedList = m_aIndexList.GetArray( nPolygons * 3 ) ;
	size_t		iLine0 = 0 ;
	for ( size_t y = 0; y < hMesh; y ++ )
	{
		size_t	iLine1 = iLine0 + (wMesh + 1) ;
		for ( size_t x = 0; x < wMesh; x ++ )
		{
			pIndexedList[0] = (uint32_t) (iLine0 + x) ;
			pIndexedList[1] = (uint32_t) (iLine0 + x + 1) ;
			pIndexedList[2] = (uint32_t) (iLine1 + x) ;
			pIndexedList[3] = (uint32_t) (iLine0 + x + 1) ;
			pIndexedList[4] = (uint32_t) (iLine1 + x + 1) ;
			pIndexedList[5] = (uint32_t) (iLine1 + x) ;
			pIndexedList += 6 ;
		}
		iLine0 = iLine1 ;
	}
	m_aIndexList.FinishArray() ;
	//
	// 描画
	//
	unsigned int	nTransparency =
		eslRoundR32ToInt( (float32_t) (m_application * 0x100) ) ;
	render.PushTransformation() ;
	render.SetInverseCameraTransformation( nullptr, nTransparency ) ;
	//
	S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	vScreen( - pp.vScreen.x, - pp.vScreen.y, 0 ) ;
	render.AppendMatrixTransformation( matI, vScreen ) ;
	//
	render.AddIndexedTriangleList
		( &m_material, 0, nPolygons, nVertex,
			pvDst, nullptr, pvSrc, pColor, m_aIndexList.GetConstArray() ) ;
	render.Finish() ;
	//
	m_vSrcMesh.FinishArray() ;
	m_vDstMesh.FinishArray() ;
	m_vDstColor.FinishArray() ;
	//
	render.PopTransformation() ;
}



//////////////////////////////////////////////////////////////////////////////
// 透視変換情報
//////////////////////////////////////////////////////////////////////////////

S3DScene::ProjectionParam::ProjectionParam( void )
: vScreen( 320.0, 240.0, 1000.0 ),
	fpZoom( 1.0 ), fpPixelAspect( 1.0 ),
	zNear( 10.0 ), zFar( 10000.0 )
{
}

const S2DDVector& S3DScene::ProjectionParam::ViewProjectionOf
	( S2DDVector& vProj, const S3DDVector& vPos ) const
{
	if ( vScreen.z != 0.0f )
	{
		double	dy = vScreen.z * fpZoom / vPos.z ;
		double	dx = dy / fpPixelAspect ;
		vProj.x = vPos.x * dx + vScreen.x ;
		vProj.y = vPos.y * dy + vScreen.y ;
	}
	else
	{
		double	dy = fpZoom ;
		double	dx = dy / fpPixelAspect ;
		vProj.x = vPos.x * dx + vScreen.x ;
		vProj.y = vPos.y * dy + vScreen.y ;
	}
	return	vProj ;
}

const S2DDVector& S3DScene::ProjectionParam::ViewProjectionAndScaleOf
	( S2DDVector& vProj, S2DDVector& vScale, const S3DDVector& vPos ) const
{
	if ( vScreen.z != 0.0f )
	{
		double	dy = vScreen.z * fpZoom / vPos.z ;
		double	dx = dy / fpPixelAspect ;
		vScale.x = dx ;
		vScale.y = dy ;
		vProj.x = vPos.x * dx + vScreen.x ;
		vProj.y = vPos.y * dy + vScreen.y ;
	}
	else
	{
		double	dy = fpZoom ;
		double	dx = dy / fpPixelAspect ;
		vScale.x = dx ;
		vScale.y = dy ;
		vProj.x = vPos.x * dx + vScreen.x ;
		vProj.y = vPos.y * dy + vScreen.y ;
	}
	return	vProj ;
}


//////////////////////////////////////////////////////////////////////////////
// 大域疑似フォッグパラメータ
//////////////////////////////////////////////////////////////////////////////

S3DScene::FogParam::FogParam( void )
: rgbFog( 0 ), zNear( 0.0 ), zFar( 10000.0 )
{
}


//////////////////////////////////////////////////////////////////////////////
// 大域環境マッピングパラメータ
//////////////////////////////////////////////////////////////////////////////

S3DScene::EnvMappingParam::EnvMappingParam( void )
: pImage( nullptr ), typeMap( RenderContext::envMappingHemisphere ),
	matMap( 1, 0, 0,  0, 1, 0,  0, 0, 1 )
{
}


//////////////////////////////////////////////////////////////////////////////
// 動的環境マッピング
//////////////////////////////////////////////////////////////////////////////

S3DScene::DynamicEnvironment::DynamicEnvironment( void )
: vCenterPos( 0, 0, 0 ),
	flagsExclusion( shadingNoReflectObject ),
	maskTargetClasses
		( S3DScene::classBitsAllScape | S3DScene::classBitsAllStaticItem )
{
}


//////////////////////////////////////////////////////////////////////////////
// 輪郭線パラメータ
//////////////////////////////////////////////////////////////////////////////

S3DScene::OffsetBorderParam::OffsetBorderParam( void )
: rgbBorder( 0 ), aThickness( 0.0f ), bThickness( 0.0f )
{
}


//////////////////////////////////////////////////////////////////////////////
// 視差パラメータ
//////////////////////////////////////////////////////////////////////////////

S3DScene::ParallaxParam::ParallaxParam( void )
	: matPosture( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
			vParallax( 0, 0, 0 ),
			vScreenDelta( 0, 0, 0 ), fpAspectDelta( 0 )
{
}

S3DScene::ParallaxParam::ParallaxParam( const ParallaxParam& src )
	: matPosture( src.matPosture ),
		vParallax( src.vParallax ),
		vScreenDelta( src.vScreenDelta ),
		fpAspectDelta( src.fpAspectDelta )
{
}

const S3DScene::ParallaxParam&
	S3DScene::ParallaxParam::operator =
				( const S3DScene::ParallaxParam& src )
{
	matPosture = src.matPosture ;
	vParallax = src.vParallax ;
	vScreenDelta = src.vScreenDelta ;
	fpAspectDelta = src.fpAspectDelta ;
	return	*this ;
}

void S3DScene::ParallaxParam::SetParallax
	( double xParallax, double zFocus, double xScreenDelta )
{
	vParallax.x = xParallax ;
	vParallax.y = 0.0 ;
	vParallax.z = 0.0 ;
	//
	S3DDVector	vDir( -xParallax, 0, zFocus ) ;
	matPosture.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	matPosture.RevolveForAngle( vDir ) ;
	//
	vScreenDelta.x = (float32_t) xScreenDelta ;
	vScreenDelta.y = 0.0 ;
	vScreenDelta.z = 0.0 ;
	//
	fpAspectDelta = 0.0f ;
}


//////////////////////////////////////////////////////////////////////////////
// 空間情報
//////////////////////////////////////////////////////////////////////////////

// m_flagsShader enum ShaderFlags 効果フィルタ
//////////////////////////////////////////////////////////////////////////////
uint64_t /*S3DShadingFlags*/
	S3DScene::GetEffectedShaderFlags
		( uint64_t flagsShading /* S3DShadingFlags */,
			uint32_t optShaderFlags /* ShaderFlags */ )
{
	uint64_t	flagsLocalShading =
		flagsShading & ~(shadingMethodToon
							| shadingDrawOffsetBorder
							| shadingEmisiveTarget
							| shadingNoDrawOffsetBorder) ;
	optShaderFlags &=
			~(optShaderFlags >> shaderFreeShifter) & shaderForceMask ;
	if ( optShaderFlags
			& (shaderForceToon | shaderForceBorder
						| shaderForceEmision | shaderNoDrawBorder) )
	{
		if ( optShaderFlags & shaderForceToon )
		{
			flagsLocalShading |= shadingMethodToon ;
		}
		if ( optShaderFlags & shaderForceBorder )
		{
			flagsLocalShading |= shadingDrawOffsetBorder ;
		}
		if ( optShaderFlags & shaderForceEmision )
		{
			flagsLocalShading |= shadingEmisiveTarget ;
		}
		if ( optShaderFlags & shaderNoDrawBorder )
		{
			flagsLocalShading |= shadingNoDrawOffsetBorder ;
		}
	}
	return	flagsLocalShading ;
}



//////////////////////////////////////////////////////////////////////////////
// 非同期処理
//////////////////////////////////////////////////////////////////////////////

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AsyncItemDispatcher::DispatchOnTimer( void )
{
	ESLAssert( m_pSpace != nullptr ) ;
	ESLAssert( m_pScene != nullptr ) ;
	m_pSpace->OnTimer( *m_pScene, m_msecTimer ) ;
}

void S3DScene::AsyncItemDispatcher::DispatchUpdateBehaviorFlags( void )
{
	ESLAssert( m_pSpace != nullptr ) ;
	ESLAssert( m_pScene != nullptr ) ;
	m_pScene->UpdateBehaviorFlags( m_pSpace, m_flagSpaceVisisble ) ;
}

void S3DScene::AsyncItemDispatcher::DispatchOnRenderEvent( void )
{
	ESLAssert( m_pSpace != nullptr ) ;
	ESLAssert( m_pScene != nullptr ) ;
	m_pSpace->OnRenderEvent( *m_pScene, m_clsItem ) ;
}


// SpaceDelayRemoveProc
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DScene::SpaceDelayRemoveProc, ESLObject, SProcedure )

S3DScene::SpaceDelayRemoveProc::SpaceDelayRemoveProc
	( SSystem::SObjectArray<ESLObject>& aDelayRemove )
{
	m_aDelayRemove.MoveArrayFrom( 0, aDelayRemove ) ;
}

void S3DScene::SpaceDelayRemoveProc::Run( void )
{
	m_aDelayRemove.RemoveAll() ;
}



// 非同期処理スレッド数（0 の時、非同期処理無効）
//////////////////////////////////////////////////////////////////////////////
size_t S3DScene::GetAsyncThreadLimit( void ) const
{
	return	m_nAsyncThreadLimit ;
}

void S3DScene::SetAsyncThreadLimit( size_t nCount )
{
	m_nAsyncThreadLimit = nCount ;
}

// 非同期処理開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::BeginAsyncDispatcher( void )
{
	ESLAssert( m_syncItemDispatcher.Sync(0) == errSuccess ) ;
	m_timerAsyncDisp.Reset() ;
	GetUIThreadMutex()->Lock() ;
	//
	ESLAssert( m_nAsyncDispatching == 0 ) ;
	AtomicAdd( &m_nAsyncDispatching, 1 ) ;
	return	sglErrSuccess ;
}

// 非同期処理終了（全処理完了同期）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::EndAsyncDispatcher( void )
{
	FlushDispatchedAsyncProc() ;
	m_syncItemDispatcher.Sync() ;		// 全処理完了
	m_signalNoAsyncThreads.Wait() ;		// 全非同期処理スレッド完了
	m_csDispStock.Lock() ;
	m_csDispStock.Unlock() ;
	//
	AtomicSub( &m_nAsyncDispatching, 1 ) ;
	ESLAssert( m_nAsyncDispatching == 0 ) ;
	GetUIThreadMutex()->Unlock() ;
	//
	m_msecAsyncDispTime = m_timerAsyncDisp.GetRealTime() ;
	return	sglErrSuccess ;
}

// 全処理完了同期
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::FenceAsyncDispatcher( void )
{
	m_syncItemDispatcher.Sync() ;
	return	sglErrSuccess ;
}

// 最後の BeginAsyncDispatcher～EndAsyncDispatcher 区間の経過時間 [ms] を取得
//////////////////////////////////////////////////////////////////////////////
double S3DScene::GetLastAsyncDispatchingDuration( void ) const
{
	return	m_msecAsyncDispTime ;
}

// 同期オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::SyncItemDispatcher * S3DScene::GetSyncDispatcher( void )
{
	SyncItemDispatcher *	pSync = nullptr ;
	m_csDispStock.Lock() ;
	pSync = m_aSyncDispStock.Pop() ;
	m_csDispStock.Unlock() ;
	if ( pSync == nullptr )
	{
		pSync = new SyncItemDispatcher ;
	}
	return	pSync ;
}

// 同期オブジェクト完了待ちと解放
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ReleaseSyncDispatcher( S3DScene::SyncItemDispatcher * pSync )
{
	ESLAssert( pSync != nullptr ) ;
	while ( pSync->Sync(0) == errTimeout )
	{
		AsyncItemDispatcher *	pDisp = nullptr ;
		m_csDispStock.Lock() ;
		size_t	nCount = m_aPendingDispatch.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			AsyncItemDispatcher *	pAsync = m_aPendingDispatch.GetAt( i ) ;
			if ( (pAsync != nullptr)
				&& (pAsync->m_aSyncDisp.FindPtr(pSync) >= 0) )
			{
				pDisp = m_aPendingDispatch.DetachAt( i ) ;
				ESLAssert( pDisp == pAsync ) ;
				break ;
			}
		}
		if ( pDisp == nullptr )
		{
			pDisp = m_aPendingDispatch.Pop() ;
		}
		m_csDispStock.Unlock() ;
		//
		if ( pDisp != nullptr )
		{
			DispatchAsyncProc( pDisp ) ;
		}
		else if ( pSync->Sync(1) == errSuccess )
		{
			break ;
		}
	}
	m_csDispStock.Lock() ;
	m_aSyncDispStock.Push( pSync ) ;
	m_csDispStock.Unlock() ;
}

// 現在のスレッドに同期オブジェクト追加
//////////////////////////////////////////////////////////////////////////////
S3DScene::SyncItemDispatcher * S3DScene::AddCurrentThreadSyncDispatcher( void )
{
	SyncItemDispatcher *	pSync = GetSyncDispatcher() ;
	//
	SThread::IdType	idThread = SThread::GetCurrentId() ;
	m_csDispStock.Lock() ;
	ThreadSyncDispatcher *	ptsd = m_soaAsyncThreads.GetAs( idThread ) ;
	if ( ptsd == nullptr )
	{
		ptsd = new ThreadSyncDispatcher ;
		m_soaAsyncThreads.Add( idThread, ptsd ) ;
	}
	ptsd->Push( pSync ) ;
	m_csDispStock.Unlock() ;
	//
	return	pSync ;
}

// 現在のスレッドの同期オブジェクト完了待ちと解放
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ReleaseCurrentThreadSyncDispatcher( S3DScene::SyncItemDispatcher * pSync )
{
	ReleaseSyncDispatcher( pSync ) ;
	//
	SThread::IdType	idThread = SThread::GetCurrentId() ;
	m_csDispStock.Lock() ;
	ThreadSyncDispatcher *	ptsd = m_soaAsyncThreads.GetAs( idThread ) ;
	ESLAssert( ptsd != nullptr ) ;
	if ( ptsd != nullptr )
	{
		ssize_t	iSync = ptsd->FindPtr( pSync ) ;
		ESLAssert( iSync >= 0 ) ;
		if ( iSync >= 0 )
		{
			ptsd->RemoveAt( (size_t) iSync ) ;
		}
	}
	m_csDispStock.Unlock() ;
}

// 非同期実行
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AsyncDispatchTimer
	( S3DScene::Space * pSpace, uint32_t msecPast,
				S3DScene::SyncItemDispatcher * pSync )
{
	if ( m_nAsyncDispatching == 0 )
	{
		pSpace->OnTimer( *this, msecPast ) ;
		return ;
	}
	m_csDispStock.Lock() ;
	//
	RunAsyncDispatcherThread() ;
	//
	if ( m_aPendingDispatch.GetLength() >= m_nAsyncThreadLimit )
	{
		m_csDispStock.Unlock() ;
		//
		pSpace->OnTimer( *this, msecPast ) ;
		return ;
	}
	//
	AsyncItemDispatcher *	pDisp = GetAsyncItemDispatcher() ;
	AttachSyncItemDispatcher( pDisp, pSync ) ;
	pDisp->m_pSpace = pSpace ;
	pDisp->m_pItem = nullptr ;
	pDisp->m_msecTimer = msecPast ;
	pDisp->m_pfnDispatch = &AsyncItemDispatcher::DispatchOnTimer ;
	m_aPendingDispatch.Add( pDisp ) ;
	//
	m_syncItemDispatcher.AddDispatch() ;
	//
	m_csDispStock.Unlock() ;
}

void S3DScene::AsyncDispatchUpdateBehaviorFlags
	( Space * pSpace, bool flagSpaceVisible, SyncItemDispatcher * pSync )
{
	if ( m_nAsyncDispatching == 0 )
	{
		UpdateBehaviorFlags( pSpace, flagSpaceVisible ) ;
		return ;
	}
	m_csDispStock.Lock() ;
	//
	RunAsyncDispatcherThread() ;
	//
	if ( m_aPendingDispatch.GetLength() >= m_nAsyncThreadLimit )
	{
		m_csDispStock.Unlock() ;
		//
		UpdateBehaviorFlags( pSpace, flagSpaceVisible ) ;
		return ;
	}
	//
	AsyncItemDispatcher *	pDisp = GetAsyncItemDispatcher() ;
	AttachSyncItemDispatcher( pDisp, pSync ) ;
	pDisp->m_pSpace = pSpace ;
	pDisp->m_pItem = nullptr ;
	pDisp->m_flagSpaceVisisble = flagSpaceVisible ;
	pDisp->m_pfnDispatch = &AsyncItemDispatcher::DispatchUpdateBehaviorFlags ;
	m_aPendingDispatch.Add( pDisp ) ;
	//
	m_syncItemDispatcher.AddDispatch() ;
	//
	m_csDispStock.Unlock() ;
}

void S3DScene::AsyncDispatchRenderEvent
	( S3DScene::Space * pSpace, ItemClass clsItem,
				S3DScene::SyncItemDispatcher * pSync )
{
	if ( m_nAsyncDispatching == 0 )
	{
		pSpace->OnRenderEvent( *this, clsItem ) ;
		return ;
	}
	m_csDispStock.Lock() ;
	//
	RunAsyncDispatcherThread() ;
	//
	if ( m_aPendingDispatch.GetLength() >= m_nAsyncThreadLimit )
	{
		m_csDispStock.Unlock() ;
		//
		pSpace->OnRenderEvent( *this, clsItem ) ;
		return ;
	}
	//
	AsyncItemDispatcher *	pDisp = GetAsyncItemDispatcher() ;
	AttachSyncItemDispatcher( pDisp, pSync ) ;
	pDisp->m_pSpace = pSpace ;
	pDisp->m_pItem = nullptr ;
	pDisp->m_clsItem = clsItem ;
	pDisp->m_pfnDispatch = &AsyncItemDispatcher::DispatchOnRenderEvent ;
	m_aPendingDispatch.Add( pDisp ) ;
	//
	m_syncItemDispatcher.AddDispatch() ;
	//
	m_csDispStock.Unlock() ;
}

// AsyncItemDispatcher に同期オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AttachSyncItemDispatcher
	( S3DScene::AsyncItemDispatcher * pDisp,
			S3DScene::SyncItemDispatcher * pSync )
{
	if ( pSync != nullptr )
	{
		pDisp->m_aSyncDisp.Add( pSync ) ;
		pSync->AddDispatch() ;
	}
	ThreadSyncDispatcher *
		ptsd = m_soaAsyncThreads.GetAs( SThread::GetCurrentId() ) ;
	if ( ptsd != nullptr )
	{
		for ( size_t i = 0; i < ptsd->GetLength(); i ++ )
		{
			SyncItemDispatcher *	psid = ptsd->GetAt( i ) ;
			if ( psid != nullptr )
			{
				pDisp->m_aSyncDisp.Add( psid ) ;
				psid->AddDispatch() ;
			}
		}
	}
}

// スレッド起動
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RunAsyncDispatcherThread( void )
{
	if ( ((size_t) m_nAsyncThreadRunning < m_nAsyncThreadLimit)
		&& (m_aPendingDispatch.GetLength() >= (size_t) m_nAsyncThreadRunning) )
	{
		m_csDispStock.Lock() ;
		AtomicAdd( &m_nAsyncThreadRunning, 1 ) ;
		if ( SThread::BeginStockThread
			( &S3DScene::AsyncDispatcherThreadProc, this ) != nullptr )
		{
			m_signalNoAsyncThreads.ResetSignal() ;
		}
		else
		{
			AtomicSub( &m_nAsyncThreadRunning, 1 ) ;
		}
		m_csDispStock.Unlock() ;
	}
}

// 非同期ディスパッチャー取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::AsyncItemDispatcher * S3DScene::GetAsyncItemDispatcher( void )
{
	AsyncItemDispatcher *	pDisp = m_aAsyncDispStock.Pop() ;
	if ( pDisp == nullptr )
	{
		pDisp = new AsyncItemDispatcher ;
		pDisp->m_pScene = this ;
	}
	return	pDisp ;
}

// 非同期処理実行
//////////////////////////////////////////////////////////////////////////////
void S3DScene::DispatchAsyncProc( S3DScene::AsyncItemDispatcher * pDisp )
{
	ESLAssert( pDisp != nullptr ) ;
	(pDisp->*(pDisp->m_pfnDispatch))() ;
	//
	for ( size_t i = 0; i < pDisp->m_aSyncDisp.GetLength(); i ++ )
	{
		SyncItemDispatcher *	psid = pDisp->m_aSyncDisp.GetAt( i ) ;
		if ( psid != nullptr )
		{
			psid->ReleaseDispatch() ;
		}
	}
	pDisp->m_aSyncDisp.RemoveAll() ;
	//
	m_syncItemDispatcher.ReleaseDispatch() ;
	//
	m_csDispStock.Lock() ;
	m_aAsyncDispStock.Add( pDisp ) ;
	m_csDispStock.Unlock() ;
}

// 非同期処理スレッド関数
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AsyncDispatcherThreadProc( void * pInstance )
{
	S3DScene *	pScene = (S3DScene*) pInstance ;
	pScene->AsyncDispatcherProc() ;
}

void S3DScene::AsyncDispatcherProc( void )
{
	GetUIThreadMutex()->SharedLock() ;
	FlushDispatchedAsyncProc() ;
	GetUIThreadMutex()->SharedUnlock() ;
	//
	m_csDispStock.Lock() ;
	m_soaAsyncThreads.RemoveAs( SThread::GetCurrentId() ) ;
	//
	if ( AtomicSub( &m_nAsyncThreadRunning, 1 ) <= 0 )
	{
		m_signalNoAsyncThreads.SetSignal() ;
		ESLAssert( m_nAsyncThreadRunning >= 0 ) ;
	}
	m_csDispStock.Unlock() ;
}

// 処理ディスパッチ
//////////////////////////////////////////////////////////////////////////////
void S3DScene::FlushDispatchedAsyncProc( void )
{
	for ( ; ; )
	{
		AsyncItemDispatcher *	pDisp = nullptr ;
		m_csDispStock.Lock() ;
		pDisp = m_aPendingDispatch.Pop() ;
		m_csDispStock.Unlock() ;
		if ( pDisp == nullptr )
		{
			break ;
		}
		DispatchAsyncProc( pDisp ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 3D シーン
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DScene, S3DCollider )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::S3DScene( void )
{
	m_msecAsyncDispTime = 0.0 ;
	m_nAsyncDispatching = 0 ;
	m_flagEnteredTimer = false ;
	m_nAsyncThreadRunning = 0 ;
	m_signalNoAsyncThreads.Initialize( true ) ;
	m_nAsyncThreadLimit =
		(size_t) esl_max( (int) SSystem::g_cpuLogicalCount - 1, 0 ) ;
	//
	m_fUpdateFieldColl = true ;
	m_fUpdatedCollision = false ;
	//
	m_errGapPhys.fpHitGap = 0.0001f ;
	m_errGapPhys.fpGroundGap = 0.001f ;
	m_flagManualPhysTime = false ;
	m_secPhysAdvance = 0.0f ;
	m_msecMaxPhysProcess = 0.0 ;
	m_msecAccPhysProcess = 0.0 ;
	m_nPhysProcess = 0 ;
	//
	m_pRenderDevice = nullptr ;
	//
	m_pTargetColor = nullptr ;
	m_pTargetColorSampler = nullptr ;
	m_pTargetDepth = nullptr ;
	m_pTargetDepthSampler = nullptr ;
	m_pTargetLayered = nullptr ;
	m_pDrawTargetLayered = nullptr ;
	m_pTargetLayeredDepth = nullptr ;
	m_pDrawTargetLayeredDepth = nullptr ;
	m_maskRenderRequirement = 0 ;
	m_maskCurrentRenderRequirement = 0 ;
	m_pTargetLeft = nullptr ;
	m_pTargetLeftSampler = nullptr ;
	m_fParallaxParam = false ;
	m_xParallax = 0.0 ;
	m_zParallaxFocus = 1.0 ;
	m_xParallaxScreenDelta = 0.0 ;
	//
	m_typeShading = shadingMethodPhong ;
	//
	m_fMainRenderScene = false ;
	//
	m_fFillBack = false ;
	m_fpVisibleNearDistance = 300.0 ;
	m_fpVisibleFarDistance = 5000.0 ;
	m_fEnableFog = false ;
	//
	m_flagDynEnvMap = false ;
	m_flagDynEnvUpdate = false ;
	m_flagDynEnvManual = false ;
	m_pWorkDynMapCube = nullptr ;
	m_pWorkDynMapZBuf = nullptr ;
	//
	for ( int i = 0; i < classCount; i ++ )
	{
		m_emsReflectionSource[i] = envmapSourceDefault ;
		m_emsRefractionSource[i] = envmapSourceDefault ;
	}
	//
	m_spaceRoot.m_refParent.SetReference( this ) ;
	//
	m_pFirstCamera = nullptr ;
	//
	m_fpDazzlement = 0.0 ;
	m_fpLastDazzlement = 0.0 ;
	m_fpLightSensitivity = 1.0 ;
	m_fpAmbientLightSensitivity = 1.0 ;
	//
	m_pShadowmapFilterShader = nullptr ;
	m_fUnsupportedShadowmapFilter = false ;
	//
//	m_pMainCamera = nullptr ;
//	m_pSoundCamera = nullptr ;
	m_pCurrentCamera = nullptr ;
	//
	m_fRenderingOffset = false ;
	//
	m_rsCurrentRenderingStage = renderingMain ;
	m_sviStereoViewTarget = RenderContext::stereoViewAuto ;
	m_classCurrentRendering = classEndFrame ;
	m_maskCurrentCollision = 0 ;
	//
	m_nRenderingBufferSize = 0x10000 ;
	//
	m_enabledAsyncProc = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DScene::~S3DScene( void )
{
	if ( m_enabledAsyncProc )
	{
		EndAsyncThread() ;
	}
}

// ルート空間へ子空間追加（静的なコライダ追加も）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddSceneSpace( S3DScene::Space * pSpace, uint32_t nOptFlags )
{
	m_spaceRoot.AddChild( pSpace ) ;
	//
	CollectBehaviorFlags( pSpace ) ;
	//
	m_spaceRoot.m_flagsBehavior |=
				pSpace->m_flagsBehavior & ~itemSpaceLocalFlags ;
	m_spaceRoot.m_maskItemClasses |= pSpace->m_maskItemClasses ;
	//
	m_colField.BeginBatchBuild() ;
	RenderSceneCollision
		( m_colField, pSpace,
				classBitField | classBitsAllStaticItem ) ;
	m_colField.EndBatchBuild() ;
	//
	RenderSceneCollision
		( m_colItems, pSpace,
			classBitsAllDynamicItem
				| classBitEffectItem | classBitLayeredItems ) ;
}

// ルート空間から空間削除（分離）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::DetachSceneSpace( S3DScene::Space * pSpace )
{
	m_spaceRoot.RemoveChild( pSpace ) ;
}

// レンダリングデバイスの設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::SetRenderDevice( S3DRenderDevice * pDevice )
{
	SGLError	err1, err2, err3 ;
	if ( m_pRenderDevice != pDevice )
	{
		m_pRenderDevice = pDevice ;
		m_pShadowmapFilterShader = nullptr ;
		m_fUnsupportedShadowmapFilter = false ;
	}
	err1 = m_render.SetRenderDeviceObject( pDevice ) ;
	err2 = m_renderEffect.SetRenderDeviceObject( pDevice ) ;
	err3 = m_renderSampler.SetRenderDeviceObject( pDevice ) ;
	m_spaceRoot.SetRenderDevice( pDevice ) ;
	return	err1 ? err1 : (err2 ? err2 : err3) ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AttachRenderTarget
	( SGLImageObject * pImage,
		SGLImageObject * pDepth,
		const SGLImageRect * pRect,
		SGLImageObject * pImageForSampler,
		SGLImageObject * pDepthForSampler )
{
	m_pTargetColor = pImage ;
	m_pTargetDepth = pDepth ;
	m_pTargetColorSampler =
		(pImageForSampler != nullptr)  ? pImageForSampler : pImage ;
	m_pTargetDepthSampler =
		(pDepthForSampler != nullptr)  ? pDepthForSampler : pDepth ;

	if ( pRect != nullptr )
	{
		m_rectTargetClip = *pRect ;
	}
	else if ( pImage != nullptr )
	{
		SGLSize	sizeImage = pImage->GetImageSize() ;
		m_rectTargetClip.x = 0 ;
		m_rectTargetClip.y = 0 ;
		m_rectTargetClip.SetSize( sizeImage ) ;
	}
}

void S3DScene::AttachStereoTargetLeft
	( SGLImageObject * pImageLeft,
		double xParallax, double zFocusRatio,
		SGLImageObject * pImageLeftForSampler )
{
	m_pTargetLeft = pImageLeft ;
	m_pTargetLeftSampler =
		(pImageLeftForSampler != nullptr) ? pImageLeftForSampler : pImageLeft ;
//	m_fParallaxParam = false ;
	m_xParallax = xParallax ;
	m_zParallaxFocus = zFocusRatio ;
	m_xParallaxScreenDelta = 0.0 ;
}

void S3DScene::AttachMultiRenderTarget
	( SGLImageObject*const* ppImages, size_t nImages,
		SGLImageObject*const* ppForSampler )
{
	m_aMultiTargets.SetLength( nImages ) ;
	m_aMultiDrawTargets.SetLength( nImages ) ;
	//
	for ( size_t i = 0; i < nImages; i ++ )
	{
		m_aMultiTargets.SetAt
			( i, (ppForSampler != nullptr) ? ppForSampler[i] : ppImages[i] ) ;
		m_aMultiDrawTargets.SetAt( i, ppImages[i] ) ;
	}
}

void S3DScene::AttachLayeredRenderTarget
	( SGLImageObject * pImageLayered,
		SGLImageObject * pDepthLayered,
		SGLImageObject * pImageLayeredForSampler,
		SGLImageObject * pDepthLayeredForSampler )
{
	m_pTargetLayered = (pImageLayeredForSampler != nullptr)
							? pImageLayeredForSampler : pImageLayered ;
	m_pDrawTargetLayered = pImageLayered ;
	m_pTargetLayeredDepth = (pDepthLayeredForSampler != nullptr)
							? pDepthLayeredForSampler : pDepthLayered ;
	m_pDrawTargetLayeredDepth = pDepthLayered ;
}

void S3DScene::AttachTemporaryRenderBuffers
	( SGLImageObject*const* ppImages, size_t nImages )
{
	m_aTempRenderBufs.SetLength( nImages ) ;
	//
	for ( size_t i = 0; i < nImages; i ++ )
	{
		m_aTempRenderBufs.SetAt( i, ppImages[i] ) ;
	}
}

SGLImageObject * S3DScene::GetTemporaryRenderBuffer( size_t i ) const
{
	return	m_aTempRenderBufs.GetAt( i ) ;
}

void S3DScene::RequireRenderTargetMask( uint32_t maskRT )
{
	m_maskRenderRequirement = maskRT ;
}

uint32_t S3DScene::GetRenderTargetRequirementMask( void ) const
{
	return	m_maskRenderRequirement ;
}

// 透視変換
//////////////////////////////////////////////////////////////////////////////
void S3DScene::GetProjection( S3DScene::ProjectionParam& projParam ) const
{
	projParam = m_projParam ;
}

void S3DScene::GetCurrentProjection( ProjectionParam& projParam ) const
{
	projParam = m_projCurrent ;
}

SGLError S3DScene::GetProjectionOfView
	( ProjectionParam& projParam, SGLSecondaryViewProducer * psvp ) const
{
	projParam = m_projParam ;
	return	(psvp == nullptr) ? sglErrSuccess : sglErrInvalidParam ;
}

void S3DScene::SetProjection( const S3DScene::ProjectionParam& projParam )
{
	m_projParam = projParam ;
	m_projCurrent = projParam ;
}

// 視差設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetParallax
	( double xParallax, double zFocusRate, double xScreenDelta )
{
	m_fParallaxParam = false ;
	m_xParallax = xParallax ;
	m_zParallaxFocus = zFocusRate ;
	m_xParallaxScreenDelta = xScreenDelta ;
}

bool S3DScene::GetParallax
	( double& xParallax, double& zFocusRate, double& xScreenDelta ) const
{
	xParallax = m_xParallax ;
	zFocusRate = m_zParallaxFocus ;
	xScreenDelta = m_xParallaxScreenDelta ;
	return	!m_fParallaxParam ;
}

void S3DScene::SetParallaxParam
	( const S3DScene::ParallaxParam& ppRight,
		const S3DScene::ParallaxParam& ppLeft )
{
	m_fParallaxParam = true ;
	m_parallaxParam[RenderContext::stereoViewRight] = ppRight ;
	m_parallaxParam[RenderContext::stereoViewLeft] = ppLeft ;
}

bool S3DScene::GetParallaxParam
	( S3DScene::ParallaxParam& ppRight, S3DScene::ParallaxParam& ppLeft ) const
{
	ppRight = m_parallaxParam[RenderContext::stereoViewRight] ;
	ppLeft = m_parallaxParam[RenderContext::stereoViewLeft] ;
	return	m_fParallaxParam ;
}

// シェーディング方法
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::GetShadingMethod( void ) const
{
	return	m_typeShading ;
}

void S3DScene::SetShadingMethod( uint32_t typeShading )
{
	m_typeShading = typeShading ;
}

// メイン画面設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetMainRenderSceneFlag( bool fMainScene )
{
	m_fMainRenderScene = fMainScene ;
}

bool S3DScene::IsMainRenderScene( void ) const
{
	return	m_fMainRenderScene ;
}

// 背景色
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::GetBackColor( SGLPalette& rgbaBack ) const
{
	rgbaBack = m_rgbaFillBack ;
	return	m_fFillBack ;
}

void S3DScene::SetBackColor
	( const SGLPalette& rgbaBack, bool fFillBack )
{
	m_fFillBack = fFillBack ;
	m_rgbaFillBack = rgbaBack ;
}

// レンダリングオフセット（精度向上）を有効／禁止にする
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EnableRenderingOffset( bool fDynamicOffset )
{
	m_fRenderingOffset = fDynamicOffset ;
}

// 有効表示距離
//////////////////////////////////////////////////////////////////////////////
double S3DScene::GetVisibleNearDistance( void ) const
{
	return	m_fpVisibleNearDistance ;
}

double S3DScene::GetVisibleFarDistance( void ) const
{
	return	m_fpVisibleFarDistance ;
}

void S3DScene::SetVisibleDistance( double fpNear, double fpFar )
{
	m_fpVisibleNearDistance = fpNear ;
	m_fpVisibleFarDistance = fpFar ;
}

// 大域疑似フォッグ
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::GetGlobalFog( S3DScene::FogParam& fogParam ) const
{
	fogParam = m_fogParam ;
	return	m_fEnableFog ;
}

void S3DScene::SetGlobalFog( const S3DScene::FogParam& fogParam, bool fFog )
{
	m_fogParam = fogParam ;
	m_fEnableFog = fFog ;
}

void S3DScene::EnableGlobalFog( bool fFog )
{
	m_fEnableFog = fFog ;
}

// 大域環境マッピング
//////////////////////////////////////////////////////////////////////////////
void S3DScene::GetEnvironmentMapping( S3DScene::EnvMappingParam& envMapParam ) const
{
	envMapParam = m_envMapParam ;
}

void S3DScene::SetEnvironmentMapping( const S3DScene::EnvMappingParam& envMapParam )
{
	m_envMapParam = envMapParam ;
}

// 大域環境マッピングソース
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetEnvironmentMappingSource
	( S3DScene::ItemClass clsItem,
		S3DScene::EnvironmentMappingSource emsReflection,
		S3DScene::EnvironmentMappingSource emsRefraction )
{
	m_emsReflectionSource[clsItem] = emsReflection ;
	m_emsRefractionSource[clsItem] = emsRefraction ;
}

void S3DScene::GetEnvironmentMappingSource
	( S3DScene::ItemClass clsItem,
		S3DScene::EnvironmentMappingSource& emsReflection,
		S3DScene::EnvironmentMappingSource& emsRefraction ) const
{
	emsReflection = m_emsReflectionSource[clsItem] ;
	emsRefraction = m_emsRefractionSource[clsItem] ;
}

// 動的環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::EnableDynamicEnvironment( bool flafDynEnv )
{
	m_flagDynEnvMap = flafDynEnv ;
}

bool S3DScene::IsEnabledDynamicEnvironment( void ) const
{
	return	m_flagDynEnvMap ;
}

void S3DScene::AttachDynamicEnvironmentTarget
	( SGLImageObject * pCubeTarget, SGLImageObject * pDepth )
{
	if ( m_pWorkDynMapCube != pCubeTarget )
	{
		m_flagDynEnvUpdate = true ;
	}
	m_pWorkDynMapCube = pCubeTarget ;
	m_pWorkDynMapZBuf = pDepth ;
}

bool S3DScene::GetDynamicEnvironment
	( S3DScene::DynamicEnvironment& dynEnvMap ) const
{
	dynEnvMap = m_envDynParam ;
	return	m_flagDynEnvManual ;
}

void S3DScene::SetDynamicEnvironment
	( const S3DScene::DynamicEnvironment& dynEnvMap, bool flagManualUpdate )
{
	m_envDynParam = dynEnvMap ;
	m_flagDynEnvManual = flagManualUpdate ;
}

// 動的環境マッピング更新設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetUpdateDynamicEnvironment( void )
{
	m_flagDynEnvUpdate = true ;
}

// 輪郭線 (shaderForceBorder) 設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::GetOffsetBorderParameter( S3DScene::OffsetBorderParam& borderParam ) const
{
	borderParam = m_borderParam ;
}

void S3DScene::SetOffsetBorderParameter( const S3DScene::OffsetBorderParam& borderParam )
{
	m_borderParam = borderParam ;
}

// レンダリングバッファサイズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetRenderingBufferSize( uint32_t countVertex )
{
	m_nRenderingBufferSize = countVertex ;
}

// カメラ位置に対する光源の輝度取得
//////////////////////////////////////////////////////////////////////////////
double S3DScene::GetLastDazzlement( void ) const
{
	return	m_fpLastDazzlement ;
}

// カメラ位置に対する光源の輝度を加算（フレーム内積算／OnUpdateBehavior, or classPreRender 内から呼び出し）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddLightDazzlement
	( const S3DDMatrix& matSpace,
		const S3DDVector& vSpace, const S3DLightEntry& light )
{
	S3DDVector	vCameraPos( 0, 0, 0 ) ;
	if ( m_pCurrentCamera != nullptr )
	{
		S3DDMatrix	matCamera ;
		m_pCurrentCamera->CalcGlobalTransformation( matCamera, vCameraPos ) ;
	}
	S3DDVector	vLight ;
	S3DDVector	vLightDir ;
	S3DDVector	vDelta ;
	double		fpDistance ;
	switch ( light.typeLight & lightTypeMask )
	{
	case	lightTypeVector:
		m_fpDazzlement += light.fpBrightness ;
		break ;
	case	lightTypePoint:
		if ( light.fpAttenuationPower > 0.0 )
		{
			vLight = light.vecPosition ;
			vLight = matSpace * vLight + vSpace ;
			vDelta = vCameraPos - vLight ;
			fpDistance = vDelta.Absolute() ;
			m_fpDazzlement +=
				light.fpBrightness
					* pow( esl_fmax( fpDistance, 1.0e-8 ),
							(double) - light.fpAttenuationPower ) ;
		}
		else
		{
			m_fpDazzlement += light.fpBrightness ;
		}
		break ;
	case	lightTypeSpot:
		vLight = light.vecPosition ;
		vLightDir = light.vecDirection ;
		vLight = matSpace * vLight + vSpace ;
		vLightDir = matSpace * vLightDir ;
		vLightDir.Normalize() ;
		vDelta = vCameraPos - vLight ;
		fpDistance = vDelta.Absolute() ;
		if ( fpDistance > 1.0e-8 )
		{
			double	cosAngle =
						vDelta.InnerProduct( vLightDir ) / fpDistance ;
			if ( cosAngle > light.fpAngle + light.fpGradation )
			{
				double	g = 1.0 ;
				if ( cosAngle < light.fpAngle )
				{
					g = (light.fpAngle - cosAngle) / light.fpGradation + 1.0 ;
				}
				m_fpDazzlement +=
					light.fpBrightness * g
						* pow( esl_fmax( fpDistance, 1.0e-8 ),
								(double) - light.fpAttenuationPower ) ;
			}
		}
		break ;
	}
}

// 光源輝度感度を設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetLightSensitivity( double fpLight, double fpAmbient )
{
	m_fpLightSensitivity = fpLight ;
	m_fpAmbientLightSensitivity = fpAmbient ;
}

// 光源輝度感度を取得
//////////////////////////////////////////////////////////////////////////////
double S3DScene::GetLightSensitivity( void ) const
{
	return	m_fpLightSensitivity ;
}

double S3DScene::GetAmbientLightSensitivity( void ) const
{
	return	m_fpAmbientLightSensitivity ;
}

// 画面効果設定（classLayeredSpace 後に処理）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddEffect( S3DScene::Effector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	m_raEffector.Add( pEffector ) ;
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

void S3DScene::AddSmartEffect( S3DScene::Effector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	m_raEffector.SmartAdd( pEffector ) ;
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

// 画面効果設定（全ての描画の後に処理）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddPostEffect( S3DScene::Effector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	m_raPostEffector.Add( pEffector ) ;
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

void S3DScene::AddSmartPostEffect( S3DScene::Effector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	m_raPostEffector.SmartAdd( pEffector ) ;
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

// 画面効果削除
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RemoveEffect( S3DScene::Effector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	ssize_t	i = m_raEffector.FindPtr( pEffector ) ;
	if ( i >= 0 )
	{
		m_raEffector.RemoveAt( (size_t) i ) ;
	}
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

void S3DScene::RemoveAllEffects( void )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	m_raEffector.RemoveAll() ;
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

void S3DScene::RemovePostEffect( S3DScene::Effector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	ssize_t	i = m_raPostEffector.FindPtr( pEffector ) ;
	if ( i >= 0 )
	{
		m_raPostEffector.RemoveAt( (size_t) i ) ;
	}
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

void S3DScene::RemoveAllPostEffects( void )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	m_raPostEffector.RemoveAll() ;
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

// レイヤード効果設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AttachLayeredEffect
	( S3DScene::ItemClass classItem, S3DScene::LayeredEffector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	switch ( classItem )
	{
	case	classLayeredItem1:
		m_refLayerEffector1.SetReference( pEffector ) ;
		break ;
	case	classLayeredItem2:
		m_refLayerEffector2.SetReference( pEffector ) ;
		break ;
	default:
		break ;
	}
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

void S3DScene::SetSmartLayeredEffect
	( S3DScene::ItemClass classItem, S3DScene::LayeredEffector * pEffector )
{
	Lock() ;
	m_csEffectorSync.Lock() ;
	switch ( classItem )
	{
	case	classLayeredItem1:
		m_refLayerEffector1.SetSmartReference( pEffector ) ;
		break ;
	case	classLayeredItem2:
		m_refLayerEffector2.SetSmartReference( pEffector ) ;
		break ;
	default:
		break ;
	}
	m_csEffectorSync.Unlock() ;
	Unlock() ;
}

// 一時画面効果追加（classLayeredSpace 後に処理）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddTemporaryEffect( S3DScene::DrawLayerEffector * pEffector )
{
	m_csEffectorSync.Lock() ;
	ESLAssert( m_aEffector.FindPtr( pEffector ) < 0 ) ;
	m_aEffector.InsertAt
		( OrderToAddEffect
			( m_aEffector, pEffector->GetDrawingPriority() ), pEffector ) ;
	m_csEffectorSync.Unlock() ;
}

// 一時画面効果追加（全ての描画の後に処理）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddTemporaryPostEffect( S3DScene::DrawLayerEffector * pEffector )
{
	m_csEffectorSync.Lock() ;
	ESLAssert( m_aPostEffector.FindPtr( pEffector ) < 0 ) ;
	m_aPostEffector.InsertAt
		( OrderToAddEffect
			( m_aPostEffector, pEffector->GetDrawingPriority() ), pEffector ) ;
	m_csEffectorSync.Unlock() ;
}

// 常設エフェクタを一時エフェクタ配列に追加
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddAllEffectorToTemporary
	( SSystem::SPointerArray<DrawLayerEffector>& aTempEffect,
		const SSystem::SReferenceArray<Effector>& raEffect )
{
	m_csEffectorSync.Lock() ;
	for ( size_t i = 0; i < raEffect.GetLength(); i ++ )
	{
		Effector *	pEffector = raEffect.GetAt( i ) ;
		if ( pEffector == nullptr )
		{
			continue ;
		}
		aTempEffect.InsertAt
			( OrderToAddEffect
				( aTempEffect, pEffector->GetDrawingPriority() ), pEffector ) ;
	}
	m_csEffectorSync.Unlock() ;
}

// 効果描画に必要なレンダーターゲットを収集
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DScene::CollectRequiredColorBuffers
	( SSystem::SPointerArray<DrawLayerEffector>& aTempEffect )
{
	uint32_t	maskBuffers = 0 ;
	for ( size_t i = 0; i < aTempEffect.GetLength(); i ++ )
	{
		DrawLayerEffector *	pEffector = aTempEffect.GetAt( i ) ;
		if ( pEffector != nullptr )
		{
			maskBuffers |= pEffector->GetRequiredColorBufferMask() ;
		}
	}
	return	maskBuffers ;
}

// 画面効果挿入位置検索
//////////////////////////////////////////////////////////////////////////////
size_t S3DScene::OrderToAddEffect
	( const SSystem::SPointerArray<DrawLayerEffector>& aEffect, int nPriority )
{
	size_t	nCount = aEffect.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DrawLayerEffector *	pEffect = aEffect.GetAt( i ) ;
		if ( pEffect != nullptr )
		{
			if ( nPriority < pEffect->GetDrawingPriority() )
			{
				return	i ;
			}
		}
	}
	return	nCount ;
}

// 非同期処理の開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::BeginAsyncThread( void )
{
	if ( m_enabledAsyncProc )
	{
		return	sglErrContinue ;
	}
	m_queAsyncProc.Reset() ;
	m_queAsyncProc.AsyncRun() ;
	m_enabledAsyncProc = true ;
	return	sglErrSuccess ;
}

// 非同期処理の終了
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::EndAsyncThread( void )
{
	if ( !m_enabledAsyncProc )
	{
		return	sglErrSuccess ;
	}
	m_queAsyncProc.RequestQuit( SProcedure::quitAbort ) ;
	m_queAsyncProc.WaitAllRunLoops() ;
	m_queAsyncProc.Reset() ;
	m_enabledAsyncProc = false ;
	return	sglErrSuccess ;
}

// 全ての非同期処理が完了するのを待つ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::WaitForAllAsyncProc( int64_t msecTimeout )
{
	if ( m_enabledAsyncProc )
	{
		return	(SGLError) m_queAsyncProc.WaitUntilEmpty( msecTimeout ) ;
	}
	return	sglErrSuccess;
}

// 非同期処理にフェンスを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetFenceAsyncProcedure( void )
{
	m_queAsyncProc.SetFence() ;
}

// 非同期処理を追加する
//////////////////////////////////////////////////////////////////////////////
void S3DScene::PostAsyncProcedure
	( SSystem::SProcedure * pProc,
		SSystem::SSignalEvent * pDoneSignal,
		bool flagAutoDelete, bool flagFence )
{
	m_queAsyncProc.AddProcedure( pProc, pDoneSignal, flagAutoDelete, flagFence ) ;
	//
	if ( !m_enabledAsyncProc )
	{
		m_queAsyncProc.Flush() ;
	}
}

void S3DScene::PostAsyncFuncProcedure
	( SSystem::SProcedureCaller::PFUNC_PTR pfnProc,
		void * pProcInstance,
		SSystem::SSignalEvent * pDoneSignal, bool flagFence )
{
	PostAsyncProcedure
		( new SProcedureCaller( pfnProc, pProcInstance ), pDoneSignal, true, flagFence ) ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::OnTimer( uint32_t msecPast )
{
	if ( m_flagEnteredTimer )
	{
		ESLTrace( "\nWARNING: Can not Re-enter S3DScene::OnTimer\n\n" ) ;
		return ;
	}
	m_flagEnteredTimer = true ;

	if ( !IsCollisionUpdated() )
	{
		UpdateSceneCollision() ;
	}
	ResetCollisionUpdatedFlag() ;
	//
	if ( !m_flagManualPhysTime )
	{
		AddPhysicsTime( (float32_t) msecPast * 0.001f ) ;
	}
	//
	BeginAsyncDispatcher() ;
	m_spaceRoot.OnTimer( *this, msecPast ) ;
	//
	for ( size_t i = 0; i < m_raEffector.GetLength(); i ++ )
	{
		Effector *	pEffector = m_raEffector.GetAt( i ) ;
		if ( pEffector  != nullptr )
		{
			pEffector->OnTimer( *this, msecPast ) ;
		}
	}
	for ( size_t i = 0; i < m_raPostEffector.GetLength(); i ++ )
	{
		Effector *	pEffector = m_raPostEffector.GetAt( i ) ;
		if ( pEffector  != nullptr )
		{
			pEffector->OnTimer( *this, msecPast ) ;
		}
	}
	LayeredEffector *	pEffector1 = m_refLayerEffector1 ;
	if ( pEffector1 != nullptr )
	{
		pEffector1->OnTimer( *this, msecPast ) ;
	}
	LayeredEffector *	pEffector2 = m_refLayerEffector2 ;
	if ( pEffector2 != nullptr )
	{
		pEffector2->OnTimer( *this, msecPast ) ;
	}
	//
	EndAsyncDispatcher() ;
	m_flagEnteredTimer = false ;
}

// レンダリング実行
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::RenderScene
	( S3DScene::Camera* pCamera, S3DRenderContextInterface * pRender )
{
	SGLError	err = PrepareRenderScene( pCamera ) ;
	if ( !err )
	{
		DoRenderScene( pRender ) ;
		FinishRenderScene() ;
	}
	return	err ;
}

// レンダリング前処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::PrepareRenderScene
	( S3DScene::Camera * pCamera, uint64_t nOptFlags )
{
	//
	// 全アイテム作用フラグ更新／カメラ・光源情報収集
	//
	m_pFirstCamera = nullptr ;
	m_ptaLights.RemoveAll() ;
	m_fpLastDazzlement = m_fpDazzlement ;
	m_fpDazzlement = 0.0 ;
	//
	m_aLayeredSpaces.RemoveAll() ;
	m_aEffector.RemoveAll() ;
	m_aPostEffector.RemoveAll() ;
	//
	SetCurrentCamera( pCamera ) ;
	AddAllEffectorToTemporary( m_aEffector, m_raEffector ) ;
	AddAllEffectorToTemporary( m_aPostEffector, m_raPostEffector ) ;
	//
	BeginAsyncDispatcher() ;
	//
	UpdateBehaviorFlags
		( &m_spaceRoot,
			!(m_spaceRoot.m_flagsSpaceBehavior & itemSpaceHidden) ) ;
	//
	EndAsyncDispatcher() ;
	//
	m_fpLightSensitivity = 1.0 / sqrt( esl_fmax( m_fpDazzlement, 1.0 ) ) ;
	m_fpAmbientLightSensitivity = m_fpLightSensitivity ;
	//
	// カメラ情報
	//
	if ( pCamera == nullptr )
	{
		pCamera = m_pFirstCamera ;
		if ( pCamera == nullptr )
		{
			ESLTrace( "not found scene camera.\n" ) ;
			return	sglErrFailed ;
		}
	}
	SetCurrentCamera( pCamera ) ;
	//
	// 前処理
	//
	m_rsCurrentRenderingStage = renderingMain ;
	RenderSceneEventItems( &m_spaceRoot, classPreRender ) ;
	RenderSceneEventItems( &m_spaceRoot, classPreRender2 ) ;
	//
	// 光源情報準備
	//
	S3DRenderingCapacity	caps ;
	m_render.GetRenderingCapacity( caps ) ;
	//
	size_t			nLights = m_ptaLights.GetLength() ;
	Light*const*	ppLights = m_ptaLights.GetConstArray() ;
	//
	S3DLightEntry *	pLights ;
	m_bufLights.SetLength( nLights ) ;
	pLights = m_bufLights.GetArray() ;
	//
	size_t	nShadowMap = 0 ;
	m_aShadhowMapLIds.RemoveAll();
	//
	for ( size_t iLight = 0; iLight < nLights; iLight ++ )
	{
		Light *	pLight = ppLights[iLight] ;
		ESLAssert( pLight != nullptr ) ;
		S3DLightEntry&	light = pLights[iLight] ;
		light = pLight->m_light ;
		if ( (light.typeLight & lightTypeMask) != lightTypeFog )
		{
			light.fpBrightness *= (float32_t) m_fpLightSensitivity ;
		}
		//
		S3DDMatrix	matLightSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		S3DDVector	posLightSpace( 0, 0, 0 ) ;
		pLight->CalcGlobalTransformation( matLightSpace, posLightSpace ) ;
		posLightSpace += m_vRenderingOffset ;
		//
		switch ( light.typeLight & lightTypeMask )
		{
		case	lightTypeVector:
			{
				light.vecDirection =
					matLightSpace * S3DDVector(light.vecDirection) ;
			}
			break ;
		case	lightTypePoint:
			{
				posLightSpace += matLightSpace * S3DDVector(light.vecPosition) ;
				light.vecPosition = posLightSpace ;
			}
			break ;
		case	lightTypeAmbient:
			if ( fabs(m_fpAmbientLightSensitivity - 1.0) > 0.001 )
			{
				float32_t	r =
					(float32_t) (light.rgbColor.argb.Red
									* m_fpAmbientLightSensitivity) ;
				float32_t	g =
					(float32_t) (light.rgbColor.argb.Green
									* m_fpAmbientLightSensitivity) ;
				float32_t	b =
					(float32_t) (light.rgbColor.argb.Blue
									* m_fpAmbientLightSensitivity) ;
				light.rgbColor.argb.Red =
					(uint8_t) esl_clampi( eslRoundR32ToInt( r ), 0, 0xFF ) ;
				light.rgbColor.argb.Green =
					(uint8_t) esl_clampi( eslRoundR32ToInt( g ), 0, 0xFF ) ;
				light.rgbColor.argb.Blue =
					(uint8_t) esl_clampi( eslRoundR32ToInt( b ), 0, 0xFF ) ;
			}
			break ;
		case	lightTypeSpot:
		case	lightTypeFog:
			{
				posLightSpace += matLightSpace * S3DDVector(light.vecPosition) ;
				light.vecPosition = posLightSpace ;
				light.vecDirection =
					matLightSpace * S3DDVector(light.vecDirection) ;
			}
			break ;
		}
		if ( (pLight->m_light.typeLight & lightShadowMapping)
						&& (nShadowMap < caps.maxShadowmapCount)
						&& !(nOptFlags & prepareSceneWithoutRender) )
		{
			//
			// シャドウマッピング
			//
			size_t	nRendered =
				RenderSceneToShadowmap
					( *pLight, light, caps.maxShadowmapCount - nShadowMap ) ;
			nShadowMap += nRendered ;
			if ( nRendered > 0 )
			{
				m_aShadhowMapLIds.Add( (uint32_t) iLight ) ;
			}
		}
	}
	m_bufLights.FinishArray() ;
	//
	// 動的大域環境マッピング
	//
	if ( m_flagDynEnvMap && (m_pWorkDynMapCube != nullptr) )
	{
		if ( m_flagDynEnvUpdate || !m_flagDynEnvManual )
		{
			RenderSceneToCube
				( m_pWorkDynMapCube, m_pWorkDynMapZBuf,
					m_envDynParam.vCenterPos,
					m_envDynParam.flagsExclusion,
					m_envDynParam.maskTargetClasses ) ;
			m_flagDynEnvUpdate = false ;
		}
	}
	//
	// コリジョン・物理演算
	//
	UpdateSceneCollision( nOptFlags ) ;
	//
	// レンダリング開始前イベント通知
	//
	m_rsCurrentRenderingStage = renderingMain ;
	RenderSceneEventItems( &m_spaceRoot, classPreRenderFrame ) ;
	//
	return	sglErrSuccess ;
}

// レンダリングは実行せず、コリジョンの更新のみ実行
//////////////////////////////////////////////////////////////////////////////
void S3DScene::UpdateSceneCollision( uint64_t nOptFlags )
{
	//
	// コリジョン
	//
	m_physScene.DetachAllActor() ;
	m_physScene.Collision().ClearBuffer() ;
	m_physScene.Collision().SetUserClassesMask( S3DCollision::colliderPhysItem ) ;
	//
	CollectBehaviorFlags( &m_spaceRoot ) ;
	//
	if ( m_fUpdateFieldColl )
	{
		m_colField.ClearBuffer() ;
		m_colField.ResetTransformation() ;
		m_colField.BeginBatchBuild() ;
		//
		RenderSceneCollision
			( m_colField, &m_spaceRoot,
				classBitField | classBitsAllStaticItem ) ;
		//
		m_colField.EndBatchBuild() ;
		m_fUpdateFieldColl = false ;
	}
	//
	m_colItems.ClearBuffer() ;
	m_colItems.ResetTransformation() ;
	//
	RenderSceneCollision
		( m_colItems, &m_spaceRoot,
			classBitsAllDynamicItem | classBitEffectItem
				| classBitLayeredItems | classBitsEffects ) ;
	//
	#if	defined(__DEBUG__)
//	ESLAssert( m_colItems.VerifyMeshTree() ) ;
	#endif
	//
	// 物理演算
	//
	if ( m_secPhysAdvance > 0.001f )
	{
		m_timerPhys.Reset() ;
		//
		m_physScene.AdvanceTime( *this, m_errGapPhys, m_secPhysAdvance ) ;
		m_secPhysAdvance = 0.0f ;
		//
		double	msecPhys = m_timerPhys.GetRealTime() ;
		m_msecMaxPhysProcess = esl_fmax( m_msecMaxPhysProcess, msecPhys ) ;
		m_msecAccPhysProcess += msecPhys ;
		m_nPhysProcess ++ ;
	}
	//
	// コリジョン更新済みフラグ
	//
	m_fUpdatedCollision = true ;
}

// レンダリング後処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::FinishRenderScene( void )
{
	m_pCurrentCamera = nullptr ;
	return	sglErrSuccess ;
}

// レンダリング実行処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::DoRenderScene
	( S3DRenderContextInterface * pRender,
		RenderContext::StereoViewIndex sviView )
{
	SGLSecondaryViewProducer *	psvp = SGLSecondaryViewProducer::GetCurrent() ;
	//
	// レンダリング実行前処理
	//
	m_sviStereoViewTarget = sviView ;
	//
	RenderSceneEventItems( &m_spaceRoot, classBeginFrame ) ;
	//
	// 描画先情報
	//
	EffectInternalInfo				eii ;
	SGLImageRect					rectView ;
	RenderContext::StereoViewIndex	sviViewParallax = sviView ;
	//
	eii.pTempRenderer = pRender ;
	//
	if ( pRender == nullptr )
	{
		m_csRender.Lock() ;
		pRender = (S3DRenderContextInterface*) &m_render ;
		if ( (m_pTargetLeft != nullptr)
			&& (sviView == RenderContext::stereoViewLeft) )
		{
			pRender->AttachTargetImage
				( m_pTargetLeft, m_pTargetDepth, &m_rectTargetClip ) ;
		}
		else
		{
			pRender->AttachTargetImage
				( m_pTargetColor, m_pTargetDepth, &m_rectTargetClip ) ;
			if ( sviView == RenderContext::stereoViewLeft )
			{
				sviViewParallax = RenderContext::stereoViewRight ;
			}
		}
		eii.fLocalRenderer = true ;
		eii.pTempRenderer = pRender ;
		rectView = m_rectTargetClip ;
	}
	else
	{
		pRender->GetViewPort( rectView ) ;
	}
	m_rectCurrentTargetClip = rectView ;
	//
	// 視差情報
	//
	ParallaxParam	pParallax ;
	if ( (sviViewParallax == RenderContext::stereoViewLeft)
		|| (sviViewParallax == RenderContext::stereoViewRight) )
	{
		const StereoParallaxParam *	pspp = nullptr ;
		if ( (m_pCurrentCamera != nullptr)
			&& ((pspp = m_pCurrentCamera->GetParallaxOfView(psvp)) != nullptr) )
		{
			pParallax = pspp->ppView[sviViewParallax] ;
		}
		else if ( m_fParallaxParam )
		{
			pParallax = m_parallaxParam[sviViewParallax] ;
		}
		else if ( sviViewParallax == RenderContext::stereoViewLeft )
		{
			pParallax.SetParallax
				( - m_xParallax,
					m_projParam.vScreen.z * m_zParallaxFocus,
					- m_xParallaxScreenDelta ) ;
		}
		else
		{
			pParallax.SetParallax
				( m_xParallax,
					m_projParam.vScreen.z * m_zParallaxFocus,
					m_xParallaxScreenDelta ) ;
		}
	}
	//
	// 透視変換設定
	//
	S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posZ( 0, 0, 0 ) ;
	pRender->SetMatrixTransformation( matI, posZ ) ;
	//
	ProjectionParam	paramProj = m_projParam ;
	if ( m_pCurrentCamera
		&& (m_pCurrentCamera->m_nCameraFlags
							& Camera::cameraForceHFOV) )
	{
		if ( m_pCurrentCamera->CalcProjectionZbyFOV
				( paramProj.vScreen.z, rectView.GetSize() ) )
		{
			paramProj.fpZoom = 1.0f ;
		}
	}
	paramProj.vScreen += pParallax.vScreenDelta ;
	paramProj.fpPixelAspect += pParallax.fpAspectDelta ;
	m_projCurrent = paramProj ;
	//
	pRender->SetProjectionScreen
		( paramProj.vScreen,
			paramProj.fpZoom, paramProj.fpPixelAspect ) ;
	pRender->SetZClipRange( paramProj.zNear, paramProj.zFar ) ;
	pRender->SetRenderingBufferSize( m_nRenderingBufferSize ) ;
	//
	if ( pRender->GetPerspectiveMatrix( m_mat4CurrentPerspective ) )
	{
		if ( m_fMainRenderScene )
		{
			SGLSecondaryViewProducer *
					psvp = SGLSecondaryViewProducer::GetCurrent() ;
			if ( psvp != nullptr )
			{
				psvp->SetMainPerspectiveMatrix
							( sviViewParallax, m_mat4CurrentPerspective ) ;
			}
		}
	}
	//
	// カメラ設定
	//
	S3DDMatrix	matCamera ;
	S3DDVector	posCamera ;
	if ( sviViewParallax != RenderContext::stereoViewAuto )
	{
		posCamera =
			matCamera.CameraAngleOf
					( m_vCameraTarget + m_vRenderingOffset,
						m_vCameraPos + m_vRenderingOffset, m_vCameraTop ) ;
		//
		S3DDMatrix	matIPosture ;
		matIPosture.InverseOf( pParallax.matPosture ) ;
		matCamera = matIPosture * matCamera ;
		posCamera = matIPosture * posCamera + pParallax.vParallax ;
		//
		pRender->SetCamera( matCamera, posCamera ) ;
	}
	else
	{
		pRender->SetCameraAngleVector
				( m_vCameraTarget + m_vRenderingOffset,
					m_vCameraPos + m_vRenderingOffset, m_vCameraTop ) ;
		pRender->GetCamera( matCamera, posCamera ) ;
	}
	m_vCameraTrans = posCamera ;
	m_matCamera = matCamera ;
	m_matICamera.InverseOf( matCamera ) ;
	//
	// 光源設定
	//
	SetPreparedLight( *pRender ) ;
	//
	// 効果描画に必要なレンダーターゲットを収集
	//
	uint32_t	maskEffectSrcBuffers = m_maskRenderRequirement ;
	bool		flagEffect = (m_aEffector.GetLength() != 0) ;
	bool		flagLayeredSpace = (m_aLayeredSpaces.GetLength() != 0) ;
	bool		flagLayeredEffect =
						((m_spaceRoot.m_maskItemClasses
									& classBitLayeredItems) != 0) ;
	//
	uint32_t	maskEffectorReqBuffers =
						CollectRequiredColorBuffers( m_aEffector ) ;
	uint32_t	maskPostEffectorReqBuffers =
						CollectRequiredColorBuffers( m_aPostEffector ) ;
	maskEffectSrcBuffers |=
				maskEffectorReqBuffers | maskPostEffectorReqBuffers ;
	//
	for ( size_t i = 0; i < m_aLayeredSpaces.GetLength(); i ++ )
	{
		Space *	pSpace = m_aLayeredSpaces.GetAt( i ) ;
		ESLAssert( pSpace != nullptr ) ;
		maskEffectSrcBuffers |= pSpace->GetRequiredColorBufferMask() ;
	}
	maskEffectSrcBuffers |= maskEffectSrcBuffers >> 1 ;
	maskEffectSrcBuffers |= maskEffectSrcBuffers >> 2 ;
	maskEffectSrcBuffers |= maskEffectSrcBuffers >> 4 ;
	maskEffectSrcBuffers |= maskEffectSrcBuffers >> 8 ;
	maskEffectSrcBuffers |= maskEffectSrcBuffers >> 16 ;
	//
	SSystem::SPointerArray<SGLImageObject>	aMultiTargets ;
	for ( size_t i = 0; i < m_aMultiDrawTargets.GetLength(); i ++ )
	{
		SGLImageObject *	pImage = nullptr ;
		if ( maskEffectSrcBuffers & (2 << i) )
		{
			pImage = m_aMultiDrawTargets.GetAt( i ) ;
		}
		aMultiTargets.Add( pImage ) ;
	}
	m_maskCurrentRenderRequirement = maskEffectSrcBuffers ;
	//
	// マルチターゲット
	//
	SGLImageObject*const*	pLastMultiTarget ;
	size_t					nLastMultiTargets ;
	pLastMultiTarget = pRender->GetMultiTargetImages( nLastMultiTargets ) ;
	//
	SSystem::SPointerArray<SGLImageObject>	aSaveMultiTargets ;
	aSaveMultiTargets.AddArray( pLastMultiTarget, nLastMultiTargets ) ;
	//
	pRender->AttachMultiTargetImages
		( aMultiTargets.GetConstArray(), aMultiTargets.GetLength() ) ;
	//
	// 描画開始
	//
	uint64_t			nSaveShading = pRender->GetShadingFlag() ;
	S3DCustomShader *	pSaveShader = pRender->GetCustomShader() ;
	SetShadingConfig( *pRender ) ;
	pRender->AttachCustomShader( nullptr ) ;
	//
	pRender->Begin3DRenderer() ;
	//
	if ( m_fFillBack )
	{
		pRender->FillClearTarget( m_rgbaFillBack.ui32 ) ;
	}
	else
	{
		pRender->FillClearTarget( 0, RenderContext::clearTargetZBuffer ) ;
	}
	//
	// 背景球
	//
	RenderSceneItems( *pRender, &m_spaceRoot, classBackscape ) ;
	//
	// 背景
	//
	RenderSceneItems( *pRender, &m_spaceRoot, classScape ) ;
	//
	// フィールド
	//
	RenderSceneItems( *pRender, &m_spaceRoot, classField ) ;
	//
	// アイテム
	//
	RenderSceneItems( *pRender, &m_spaceRoot, classStaticItem1 ) ;
	RenderSceneItems( *pRender, &m_spaceRoot, classStaticItem2 ) ;
	RenderSceneItems( *pRender, &m_spaceRoot, classDynamicItem1 ) ;
	RenderSceneItems( *pRender, &m_spaceRoot, classDynamicItem2 ) ;
	RenderSceneItems( *pRender, &m_spaceRoot, classDynamicItem3 ) ;
	RenderSceneItems( *pRender, &m_spaceRoot, classEffectItem ) ;
	//
	// 画面効果／レイヤード効果アイテム
	//
	if ( (m_pTargetLayered != nullptr)
		&& (flagEffect | flagLayeredSpace | flagLayeredEffect) )
	{
		//
		// レンダリング出力先情報取得
		//
		S3DRenderContextInterface *
			prcEffect = GetRenderTargetForEffect( eii, *pRender ) ;
		//
		if ( (eii.pEffectDepth != m_pTargetLayeredDepth)
			&& (m_pTargetLayeredDepth != nullptr) )
		{
			prcEffect->AttachTargetImage
				( m_pTargetLayered, m_pTargetLayeredDepth, &rectView ) ;
			prcEffect->CopyBufferFrom
				( *pRender, S3DRenderContext::copyBufferDepth ) ;
		}
		//
		m_csRender.Lock() ;
		//
		// これまでのレンダリングを確定する
		//
		pRender->AttachMultiTargetImages( nullptr, 0 ) ;
		//
		// 中間バッファにレンダリング出力設定する
		// ※ｚバッファはｚテストのために使いまわす
		//
		prcEffect->SetRenderingBufferSize( m_nRenderingBufferSize ) ;
		prcEffect->AttachTargetImage
			( m_pDrawTargetLayered, eii.pTargetDepth, &rectView ) ;
		prcEffect->AttachMultiTargetImages
			( aMultiTargets.GetConstArray(), aMultiTargets.GetLength() ) ;
		//
		prcEffect->ResetTransformation() ;
		//
		prcEffect->SetProjectionScreen
			( paramProj.vScreen,
				paramProj.fpZoom, paramProj.fpPixelAspect ) ;
		prcEffect->SetZClipRange( paramProj.zNear, paramProj.zFar ) ;
		//
		prcEffect->SetCamera( matCamera, posCamera ) ;
		//
		SetPreparedLight( *prcEffect ) ;
		//
		SetShadingConfig( *prcEffect ) ;
		//
		prcEffect->Begin3DRenderer() ;
		//
		if ( flagLayeredSpace )
		{
			//
			// レイヤー描画空間
			//
			EnvironmentMappingSource
					emsReflection = m_emsReflectionSource[classLayeredSpace] ;
			EnvironmentMappingSource
					emsRefraction = m_emsRefractionSource[classLayeredSpace] ;
			SetGlobalEnvironmentMapping
				( *prcEffect, eii.pTargetColor[0],
					m_pTargetLayeredDepth, emsReflection, emsRefraction ) ;
			//
			for ( size_t i = 0; i < m_aLayeredSpaces.GetLength(); i ++ )
			{
				Space *	pSpace = m_aLayeredSpaces.GetAt( i ) ;
				ESLAssert( pSpace != nullptr ) ;
				if ( pSpace->m_flagsSpaceBehavior & itemSpaceHidden )
				{
					continue ;
				}
				prcEffect->AttachMultiTargetImages( nullptr, 0 ) ;
				prcEffect->FillClearTarget( 0, PaintContext::clearTargetColor ) ;
				prcEffect->Finish() ;
				//
				EnvironmentMappingSource	emsRefl, emsRefr ;
				pSpace->GetEnvironmentMappingSource( emsRefl, emsRefr ) ;
				if ( emsRefl == envmapSourceDefault )
				{
					emsRefl = m_emsReflectionSource[classLayeredSpace] ;
				}
				if ( emsRefr == envmapSourceDefault )
				{
					emsRefr = m_emsRefractionSource[classLayeredSpace] ;
				}
				if ( (emsReflection != emsRefl) || (emsRefraction != emsRefr) )
				{
					emsReflection = emsRefl ;
					emsRefraction = emsRefr ;
					//
					SetGlobalEnvironmentMapping
						( *prcEffect, eii.pTargetColor[0],
							m_pTargetLayeredDepth, emsReflection, emsRefraction ) ;
				}
				prcEffect->AttachMultiTargetImages
					( aMultiTargets.GetConstArray(), aMultiTargets.GetLength() ) ;
				RenderLayeredSpace( *prcEffect, pSpace ) ;
				prcEffect->Finish() ;
				//
				SGLImageObject *	pSaveTarget0 = eii.pEffectColor[0] ;
				if ( pSpace->m_aTempEffector.GetLength() > 0 )
				{
					prcEffect->AttachMultiTargetImages( nullptr, 0 ) ;
					RenderEffectors
						( *pRender, *prcEffect,
							pSpace->m_aTempEffector,
							eii, maskEffectorReqBuffers ) ;
				}
				else
				{
					SampleLayeredTargetForEffect( eii, *prcEffect ) ;
				}
				eii.pEffectColor[0] = m_pTargetLayered ;
				pSpace->DrawEffect
					( *this, *pRender,
						eii.pEffectColor,
						eii.nTargetColors, eii.pEffectDepth ) ;
				eii.pEffectColor[0] = pSaveTarget0 ;
			}
		}
		if ( flagEffect )
		{
			//
			// 画面効果
			//
			prcEffect->AttachMultiTargetImages( nullptr, 0 ) ;
			RenderEffectors
				( *pRender, *prcEffect,
					m_aEffector, eii, maskEffectorReqBuffers ) ;
		}
		bool	flagLayeredEffect1 =
					((m_spaceRoot.m_maskItemClasses
							& (1 << classLayeredItem1)) != 0) ;
		bool	flagLayeredEffect2 =
					((m_spaceRoot.m_maskItemClasses
							& (1 << classLayeredItem2)) != 0) ;
		if ( flagLayeredEffect1 )
		{
			//
			// レイヤード効果アイテム描画
			//
			prcEffect->AttachMultiTargetImages( nullptr, 0 ) ;
			prcEffect->FillClearTarget( 0, PaintContext::clearTargetColor ) ;
			prcEffect->Finish() ;
			//
			prcEffect->AttachMultiTargetImages
				( aMultiTargets.GetConstArray(), aMultiTargets.GetLength() ) ;
			SetGlobalEnvironmentMapping
				( *prcEffect, eii.pTargetColor[0], m_pTargetLayeredDepth,
					m_emsReflectionSource[classLayeredItem1],
					m_emsRefractionSource[classLayeredItem1] ) ;
			RenderSceneItems( *prcEffect, &m_spaceRoot, classLayeredItem1 ) ;
			prcEffect->Finish() ;
			//
			// レイヤード効果描画
			//
			SGLImageObject *	pSaveTarget0 = eii.pEffectColor[0] ;
			SampleLayeredTargetForEffect( eii, *prcEffect ) ;
			//
			LayeredEffector *	pEffector = m_refLayerEffector1 ;
			if ( pEffector != nullptr )
			{
				eii.pEffectColor[0] = m_pTargetLayered ;
				pEffector->DrawLayer
					( *this, *pRender,
						eii.pEffectColor,
						eii.nTargetColors, eii.pEffectDepth ) ;
			}
			else
			{
				SGLPaintParam	pp ;
				pRender->DrawImage( pp, m_pTargetLayered ) ;
			}
			pRender->Finish() ;
			eii.pEffectColor[0] = pSaveTarget0 ;
		}
		if ( flagLayeredEffect2 )
		{
			//
			// レイヤード効果アイテム描画
			//
			prcEffect->AttachMultiTargetImages( nullptr, 0 ) ;
			prcEffect->FillClearTarget( 0, PaintContext::clearTargetColor ) ;
			prcEffect->Finish() ;
			//
			prcEffect->AttachMultiTargetImages
				( aMultiTargets.GetConstArray(), aMultiTargets.GetLength() ) ;
			SetGlobalEnvironmentMapping
				( *prcEffect, eii.pTargetColor[0], m_pTargetLayeredDepth,
					m_emsReflectionSource[classLayeredItem2],
					m_emsRefractionSource[classLayeredItem2] ) ;
			RenderSceneItems( *prcEffect, &m_spaceRoot, classLayeredItem2 ) ;
			prcEffect->Finish() ;
			//
			// レイヤード効果描画
			//
			SGLImageObject *	pSaveTarget0 = eii.pEffectColor[0] ;
			SampleLayeredTargetForEffect( eii, *prcEffect ) ;
			//
			LayeredEffector *	pEffector = m_refLayerEffector2 ;
			if ( pEffector != nullptr )
			{
				eii.pEffectColor[0] = m_pTargetLayered ;
				pEffector->DrawLayer
					( *this, *pRender,
						eii.pEffectColor,
						eii.nTargetColors, eii.pEffectDepth ) ;
			}
			else
			{
				SGLPaintParam	pp ;
				pRender->DrawImage( pp, m_pTargetLayered ) ;
			}
			pRender->Finish() ;
			eii.pEffectColor[0] = pSaveTarget0 ;
		}
		prcEffect->End3DRenderer() ;
		prcEffect->SetEnvironmentMappingImage( nullptr, 0 ) ;
		prcEffect->SetEnvironmentMappingImage
			( nullptr, RenderContext::envMappingViewportDepth ) ;
		prcEffect->SetEnvironmentMappingImage
			( nullptr, (RenderContext::envMappingViewport
						| RenderContext::envMappingRefraction) ) ;
		prcEffect->DetachTargetImage() ;
		//
		prcEffect->AttachMultiTargetImages( nullptr, 0 ) ;
		prcEffect->SetLightEntries( nullptr, 0 ) ;
		//
		m_csRender.Unlock() ;
		//
		pRender->AttachMultiTargetImages
			( aMultiTargets.GetConstArray(), aMultiTargets.GetLength() ) ;
	}
	//
	// 効果
	//
	RenderSceneItems( *pRender, &m_spaceRoot, classEffect1 ) ;
	RenderSceneItems( *pRender, &m_spaceRoot, classEffect2 ) ;
	//
	// 後効果
	//
	if ( m_aPostEffector.GetLength() > 0 )
	{
		S3DRenderContextInterface *
			prcEffect = GetRenderTargetForEffect( eii, *pRender ) ;
		//
		prcEffect->AttachTargetImage( m_pTargetLayered, nullptr, &rectView ) ;
		prcEffect->AttachMultiTargetImages( nullptr, 0 ) ;
		pRender->AttachMultiTargetImages( nullptr, 0 ) ;
		//
		RenderEffectors
			( *pRender, *prcEffect,
				m_aPostEffector, eii, maskPostEffectorReqBuffers ) ;
		//
		prcEffect->DetachTargetImage() ;
		pRender->AttachMultiTargetImages
			( aMultiTargets.GetConstArray(), aMultiTargets.GetLength() ) ;
	}
	//
	// 最終効果
	//
	RenderSceneItems( *pRender, &m_spaceRoot, classEffect3 ) ;
	//
	// 描画完了
	//
	pRender->End3DRenderer() ;
	pRender->Finish() ;
	pRender->SetShadingFlag( nSaveShading ) ;
	pRender->AttachCustomShader( pSaveShader ) ;
	//
	pRender->AttachMultiTargetImages
		( aSaveMultiTargets.GetConstArray(), aSaveMultiTargets.GetLength() ) ;
	//
	for ( size_t i = 0; i < m_aShadhowMapLIds.GetLength(); i ++ )
	{
		uint32_t	idLight = m_aShadhowMapLIds.At(i) ;
		Light *		pLight = m_ptaLights.GetAt( (size_t) idLight ) ;
		if ( pLight != nullptr )
		{
			pRender->SetShadowMap
				( idLight, nullptr, pLight->m_smiShadowMapInfo[0] ) ;
		}
	}
	pRender->SetEnvironmentMappingImage
		( nullptr, RenderContext::envMappingHemisphere ) ;
	pRender->SetEnvironmentMappingImage
		( nullptr, (RenderContext::envMappingHemisphere
					| RenderContext::envMappingRefraction) ) ;
	pRender->SetLightEntries( nullptr, 0 ) ;
	//
	if ( eii.fLocalRenderer )
	{
		pRender->DetachTargetImage() ;
		m_csRender.Unlock() ;
	}
	//
	// レンダリング実行後処理
	//
	RenderSceneEventItems( &m_spaceRoot, classEndFrame ) ;
	//
	m_sviStereoViewTarget = RenderContext::stereoViewAuto ;
	m_projCurrent = m_projParam ;
}

// classPreRenderFrame 時に任意カメラレンダリングする
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderSceneTemporary
	( S3DRenderContextInterface& render,
		S3DScene::Camera * pCamera, bool flagFillBack,
		uint64_t flagsExclusion, uint64_t maskTargetClasses )
{
	//
	// カメラ
	//
	Camera *	pSaveCamera = GetCurrentCamera() ;
	SetCurrentCamera( pCamera ) ;
	//
	render.SetCameraAngleVector
			( m_vCameraTarget + m_vRenderingOffset,
				m_vCameraPos + m_vRenderingOffset, m_vCameraTop ) ;
	//
	// 光源
	//
	SetPreparedLight( render ) ;
	//
	// 描画
	//
	render.Begin3DRenderer() ;
	if ( flagFillBack )
	{
		render.FillClearTarget( m_rgbaFillBack.ui32 ) ;
	}
	//
	uint64_t	maskTarget = maskTargetClasses
							| ((uint64_t)1 << classBeginFrame)
							| ((uint64_t)1 << classEndFrame) ;
	for ( size_t iClass = 0;
			maskTarget && (iClass < classCount);
			iClass ++, maskTarget >>= 1 )
	{
		if ( maskTarget & 1 )
		{
			if ( iClass == classLayeredSpace )
			{
				RenderAllLayeredSpaces( render, flagsExclusion ) ;
			}
			else if ( (iClass >= classFirstRenderClass)
					& (iClass <= classLastRenderClass) )
			{
				RenderSceneItems
					( render, &m_spaceRoot,
						(ItemClass) iClass, flagsExclusion ) ;
			}
			else
			{
				RenderSceneEventItems( &m_spaceRoot, (ItemClass) iClass ) ;
			}
		}
	}
	//
	render.End3DRenderer() ;
	render.Finish() ;
	render.DetachTargetImage() ;
	//
	SetCurrentCamera( pSaveCamera ) ;
}

// Cube マッピング用レンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderSceneToCube
	( SGLImageObject * pCubeImage, SGLImageObject * pDepthBuffer,
		const S3DDVector& vCenterPos,
		uint64_t flagsExclusion, uint64_t maskTargetClasses )
{
	//
	// 描画準備
	//
	S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posZ( 0, 0, 0 ) ;
	m_render.ResetTransformation() ;
	m_render.SetMatrixTransformation( matI, posZ ) ;
	//
	SGLSize		sizeImage = pCubeImage->GetImageSize() ;
	S3DVector	vScreen( sizeImage.w * 0.5, sizeImage.h * 0.5, sizeImage.w * 0.5 ) ;
	m_render.SetProjectionScreen( vScreen, 1.0 ) ;
	m_render.SetZClipRange( m_projParam.zNear, m_projParam.zFar ) ;
	//
	m_render.SetRenderingBufferSize( m_nRenderingBufferSize ) ;
	//
	SetShadingConfig( m_render, true ) ;
	//
	m_rsCurrentRenderingStage = renderingEnvironment ;
	//
	// カメラ準備
	//
	S3DDVector	vCameraDir[6] =
	{
		S3DDVector( -1, 0, 0 ),
		S3DDVector( 0, 1, 0 ),
		S3DDVector( 0, 0, -1 ),
		S3DDVector( 1, 0, 0 ),
		S3DDVector( 0, -1, 0 ),
		S3DDVector( 0, 0, 1 ),
	} ;
	S3DDVector	vCameraTop[6] =
	{
		S3DDVector( 0, -1, 0 ),
		S3DDVector( 0, 0, 1 ),
		S3DDVector( 0, -1, 0 ),
		S3DDVector( 0, -1, 0 ),
		S3DDVector( 0, 0, -1 ),
		S3DDVector( 0, -1, 0 ),
	} ;
	Camera	camera ;
	m_spaceRoot.AddItem( &camera ) ;
	//
	size_t	iCurFrame = pCubeImage->GetSelectedFrame() ;
	for ( size_t iFace = 0; iFace < 6; iFace ++ )
	{
		//
		// ターゲット
		//
		pCubeImage->SelectFrame( iFace ) ;
		m_render.AttachTargetImage( pCubeImage, pDepthBuffer, nullptr ) ;
		m_render.AttachMultiTargetImages( nullptr, 0 ) ;
		//
		// カメラ
		//
		camera.m_space.m_vCenter = vCenterPos ;
		camera.m_vTarget = vCenterPos + vCameraDir[iFace] * vScreen.z ;
		camera.m_vTop = vCameraTop[iFace] ;
		//
		// 描画
		//
		RenderSceneTemporary
			( m_render, &camera,
				m_fFillBack, flagsExclusion, maskTargetClasses ) ;
	}
	//
	m_spaceRoot.RemoveItem( &camera ) ;
	//
	pCubeImage->SelectFrame( iCurFrame ) ;
}

// Shadowmap 用レンダリング
//////////////////////////////////////////////////////////////////////////////
size_t S3DScene::RenderSceneToShadowmap
	( S3DScene::ShadowMapBuffer& bufShadowmap,
		const S3DLightEntry& light, size_t nCascadeMaxCount )
{
	const ShadowMapParam &	smp = bufShadowmap.m_smpShadowMap ;
	if ( nCascadeMaxCount > smp.nCascadeMaxCount + 1 )
	{
		nCascadeMaxCount = smp.nCascadeMaxCount + 1 ;
	}
	if ( nCascadeMaxCount > ShadowMapBuffer::maxCascadeShadowmap )
	{
		nCascadeMaxCount = ShadowMapBuffer::maxCascadeShadowmap ;
	}
	S3DRenderDevice *	pDevice = m_render.GetRenderDeviceObject() ;
	if ( (pDevice != nullptr)
		&& (m_pShadowmapFilterShader == nullptr)
		&& !m_fUnsupportedShadowmapFilter )
	{
		S3DRenderDevice::Features	features ;
		pDevice->GetDeviceFeatures( features ) ;
		//
		if ( (features.flagsFeatures[0] & S3DRenderDevice::feature0_ComputeShader)
			&& (features.flagsFeatures[0] & S3DRenderDevice::feature0_TextureFloat) )
		{
			m_pShadowmapFilterShader =
				pDevice->GetDefaultShaderProgramAs
					( S3DRenderDevice::DefaultShaderId::ShadowmapFilter ) ;
		}
		m_fUnsupportedShadowmapFilter = (m_pShadowmapFilterShader == nullptr) ;
	}
	SGLSize	sizeImage = bufShadowmap.m_imgShadowColor.GetImageSize() ;
	if ( (sizeImage != smp.sizeMapping)
		|| (bufShadowmap.m_imgShadowColor.GetFrameCount() != nCascadeMaxCount) )
	{
		bufShadowmap.m_imgShadowColor.CreateImage
			( smp.sizeMapping.w, smp.sizeMapping.h,
				formatImageDefaultRGBA, 32,
				SGLImageObject::bufferForRenderTarget
					| SGLImageObject::bufferTextureArray
					| SGLImageObject::bufferOnDeviceOnly,
				nCascadeMaxCount ) ;
		bufShadowmap.m_imgShadowDepth.CreateImage
			( smp.sizeMapping.w, smp.sizeMapping.h,
				formatImageDepth, 32,
				SGLImageObject::bufferForRenderTarget
					| SGLImageObject::bufferTextureArray
					| SGLImageObject::bufferOnDeviceOnly,
				nCascadeMaxCount ) ;
		bufShadowmap.m_imgShadowColor.SetImageIdentity( L"shadow map color" ) ;
		bufShadowmap.m_imgShadowDepth.SetImageIdentity( L"shadow map depth" ) ;
		sizeImage = smp.sizeMapping ;
	}
	if ( !m_fUnsupportedShadowmapFilter
		&& ((bufShadowmap.m_imgShadowDepthBuf.GetImageSize() != smp.sizeMapping)
			|| (bufShadowmap.m_imgShadowDepthBuf.GetFrameCount() != nCascadeMaxCount)) )
	{
		bufShadowmap.m_imgShadowDepthBuf.CreateImage
			( smp.sizeMapping.w, smp.sizeMapping.h,
				formatImageABGR
					| formatImageFlagFloat, 32*4,
				/*formatImageGray
					| formatImageFlagFloat
					| formatImageFlagAlpha, 32*2,*/
				SGLImageObject::bufferOnDeviceOnly
					| SGLImageObject::bufferTextureArray
					| SGLImageObject::bufferTextureStorage,
				nCascadeMaxCount ) ;
		bufShadowmap.m_imgShadowDepthBuf.SetImageIdentity( L"shadow map filtering buffer" ) ;
	}
	//
	// シャドウマッピング
	//
	size_t	nRenderedShadowmap = 0 ;
	bufShadowmap.m_nRenderedShadowmap = 0 ;
	//
	for ( size_t iShadow = 0; iShadow < nCascadeMaxCount; iShadow ++ )
	{
		bufShadowmap.m_imgShadowColor.SelectFrame( iShadow ) ;
		bufShadowmap.m_imgShadowDepth.SelectFrame( iShadow ) ;
		//
		m_render.SetRenderingBufferSize( m_nRenderingBufferSize ) ;
		m_render.AttachTargetImage
			( &(bufShadowmap.m_imgShadowColor),
				&(bufShadowmap.m_imgShadowDepth) ) ;
		m_render.AttachMultiTargetImages( nullptr, 0 ) ;
		m_render.SetShadingFlag
			( shadingMethodNothing | shadingTextureSmoothing ) ;
		//
		S3DDVector	vCameraTarget = m_vCameraTarget + m_vRenderingOffset ;
		S3DDVector	vCameraPos = m_vCameraPos + m_vRenderingOffset ;
		if ( !bufShadowmap.SetShadowMappingInfo
			( m_render, bufShadowmap.m_smiShadowMapInfo[iShadow], smp,
				light, iShadow, sizeImage, vCameraTarget, vCameraPos ) )
		{
			m_rsCurrentRenderingStage = renderingShadow ;
			m_render.SetLightEntries( nullptr, 0 ) ;
			m_render.Begin3DRenderer() ;
			m_render.FillClearTarget( 0, PaintContext::clearTargetZBuffer ) ;
			//
			uint64_t	flagsExclusion = shadingNoZBuffer
							| shadingZBufferNoWrite | shadingNoShadowObject ;
			uint32_t	optShader = shaderNoDrawBorder ;
			RenderSceneEventItems( &m_spaceRoot, classBeginFrame ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classField, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classStaticItem1, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classStaticItem2, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classDynamicItem1, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classDynamicItem2, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classDynamicItem3, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classEffectItem, flagsExclusion, optShader ) ;
			RenderAllLayeredSpaces
				( m_render, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classLayeredItem1, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classLayeredItem2, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classEffect1, flagsExclusion, optShader ) ;
			RenderSceneItems
				( m_render, &m_spaceRoot,
					classEffect2, flagsExclusion, optShader ) ;
			RenderSceneEventItems( &m_spaceRoot, classEndFrame ) ;
			//
			m_render.End3DRenderer() ;
			m_render.Finish() ;
			//
			bufShadowmap.m_nRenderedShadowmap = iShadow + 1 ;
			nRenderedShadowmap ++ ;
		}
		m_render.DetachTargetImage() ;
	}
	if ( (pDevice != nullptr)
		&& (m_pShadowmapFilterShader != nullptr)
		&& (bufShadowmap.m_nRenderedShadowmap > 0) )
	{
		ESLAssert( !m_fUnsupportedShadowmapFilter ) ;
		S3DShadowmapDepthFilter5x5Interface *	pLoopFilter =
			ESLTypeCast<S3DShadowmapDepthFilter5x5Interface>( m_pShadowmapFilterShader ) ;
		if ( pLoopFilter != nullptr )
		{
			pLoopFilter->SetInputDepth( &(bufShadowmap.m_imgShadowDepth) ) ;
			pLoopFilter->SetOutputDepth( &(bufShadowmap.m_imgShadowDepthBuf) ) ;
			pLoopFilter->SetGaussianFilter( smp.fpFilterGauss ) ;
			pLoopFilter->SyncExecute( pDevice ) ;
		}
	}
	return	nRenderedShadowmap ;
}

// 準備済みの光源を設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetPreparedLight( S3DRenderContextInterface & render ) const
{
	render.SetLightEntries
		( m_bufLights.GetConstArray(), m_bufLights.GetLength() ) ;
	//
	for ( size_t i = 0; i < m_aShadhowMapLIds.GetLength(); i ++ )
	{
		uint32_t	idLight = m_aShadhowMapLIds.At(i) ;
		Light *		pLight = m_ptaLights.GetAt( (size_t) idLight ) ;
		if ( pLight != nullptr )
		{
			for ( size_t iShadow = 0;
					iShadow < pLight->m_nRenderedShadowmap; iShadow ++ )
			{
				SGLImageObject *	pDepth = &(pLight->m_imgShadowDepth) ;
				if ( !m_fUnsupportedShadowmapFilter )
				{
					pDepth = &(pLight->m_imgShadowDepthBuf) ;
				}
				render.SetShadowMap
					( idLight, pDepth, pLight->m_smiShadowMapInfo[iShadow] ) ;
			}
		}
	}
}

// 効果用レンダリング出力先情報取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface *
	S3DScene::GetRenderTargetForEffect
		( S3DScene::EffectInternalInfo& eii, S3DRenderContextInterface& renderSrc )
{
/*
	// CopyBufferFrom を使うので関係なくなった
	if ( !eii.fLocalRenderer )
	{
		SGLImageObject *	pDepth = eii.pTempRenderer->GetTargetZBuffer() ;
		if ( (pDepth != nullptr)
			&& (m_pTargetLayered->GetImageSize() == pDepth->GetImageSize()) )
		{
			// ※標準的な描画条件
			eii.nTargetColors = 1 ;
			eii.pTargetColor[0] = eii.pTempRenderer->GetTargetImage() ;
			eii.pEffectDepth = pDepth ;
			eii.fCommonDepth = true ;
		}
		else
		{
			// ※通常はこの条件では描画しない
			eii.nTargetColors = 1 ;
			eii.pTargetColor[0] = eii.pTempRenderer->GetTargetImage() ;
			eii.pEffectDepth = m_pTargetDepthSampler ;
			eii.fCommonDepth = false ;
		}
	}
	else
*/	{
		eii.nTargetColors = 1 ;
		if ( m_sviStereoViewTarget == RenderContext::stereoViewLeft )
		{
			eii.pTargetColor[0] =
				(m_pTargetLeftSampler != nullptr)
					? m_pTargetLeftSampler : m_pTargetColorSampler ;
		}
		else
		{
			eii.pTargetColor[0] = m_pTargetColorSampler ;
		}
		eii.pEffectDepth = m_pTargetDepthSampler ;
		eii.fCommonDepth = true ;
	}
	if ( eii.pTargetColor[0] == nullptr )
	{
		eii.pTargetColor[0] = renderSrc.GetTargetImage() ;
	}
	if ( eii.pEffectDepth == nullptr )
	{
		eii.pEffectDepth = renderSrc.GetTargetZBuffer() ;
	}
	eii.pTargetDepth = renderSrc.GetTargetZBuffer() ;	// ※ｚバッファはｚテストのために使いまわす
	eii.pEffectColor[0] = eii.pTargetColor[0] ;
	//
	for ( size_t i = 0; (i + 1 < 8) && (i < m_aMultiTargets.GetLength()); i ++ )
	{
		eii.pTargetColor[i + 1] = m_aMultiTargets.GetAt( i ) ;
		eii.pEffectColor[i + 1] = eii.pTargetColor[i + 1] ;
		eii.nTargetColors = i + 2 ;
	}
	//
	renderSrc.Finish() ;
	//
	uint32_t	nCopyFlags = 0 ;
	for ( size_t i = 0; (i + 1 < 8) && (i < m_aMultiDrawTargets.GetLength()); i ++ )
	{
		SGLImageObject *	pRendered = m_aMultiDrawTargets.GetAt(i) ;
		if ( (pRendered != nullptr)
			&& (eii.pEffectColor[i + 1] != nullptr)
			&& (pRendered != eii.pEffectColor[i + 1])
			&& (m_maskCurrentRenderRequirement & (2 << i)) )
		{
			nCopyFlags |= S3DRenderContext::copyBufferColor ;
		}
	}
	//
	if ( renderSrc.GetTargetImage() != eii.pTargetColor[0] )
	{
		nCopyFlags |= S3DRenderContext::copyBufferColor ;
	}
	if ( renderSrc.GetTargetZBuffer() != eii.pEffectDepth )
	{
		nCopyFlags |= S3DRenderContext::copyBufferDepth ;
	}
	if ( nCopyFlags != 0 )
	{
		m_renderEffect.AttachTargetImage
			( eii.pTargetColor[0], eii.pEffectDepth, &m_rectCurrentTargetClip ) ;
		//
		if ( nCopyFlags & S3DRenderContext::copyBufferColor )
		{
			m_renderEffect.AttachMultiTargetImages
				( m_aMultiTargets.GetConstArray(), m_aMultiTargets.GetLength() ) ;
		}
		m_renderEffect.CopyBufferFrom( renderSrc, nCopyFlags ) ;
	}
	//
	return	(S3DRenderContextInterface*) &m_renderEffect ;
}

// 効果用描画バッファをサンプラー用バッファにコピーする
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SampleLayeredTargetForEffect
	( S3DScene::EffectInternalInfo& eii, S3DRenderContextInterface& render )
{
	if ( (eii.pEffectColor[0] != m_pTargetLayered)
		&& (m_pTargetLayered != m_pDrawTargetLayered) )
	{
		m_renderSampler.AttachTargetImage( m_pTargetLayered, nullptr ) ;
		m_renderSampler.AttachMultiTargetImages( nullptr, 0 ) ;
		m_renderSampler.CopyBufferFrom( render, S3DRenderContext::copyBufferColor ) ;
		eii.pEffectColor[0] = m_pTargetLayered ;
	}
}

// シェーディング・描画設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetShadingConfig( S3DRenderContextInterface& render, bool fDisableEnv ) const
{
	render.EnableFog( m_fEnableFog ) ;
	if ( m_fEnableFog )
	{
		render.SetFog
			( m_fogParam.rgbFog.ui32, m_fogParam.zNear, m_fogParam.zFar ) ;
	}
	if ( !fDisableEnv )
	{
		if ( m_flagDynEnvMap && (m_pWorkDynMapCube != nullptr) )
		{
			S3DMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			render.SetEnvironmentMappingMatrix( matI ) ;
			render.SetEnvironmentMappingImage
				( m_pWorkDynMapCube,
					S3DRenderContextInterface::envMappingCube ) ;
			render.SetEnvironmentMappingImage
				( m_pWorkDynMapCube,
					S3DRenderContextInterface::envMappingCube
					| S3DRenderContextInterface::envMappingRefraction ) ;
		}
		else if ( m_envMapParam.pImage != nullptr )
		{
			render.SetEnvironmentMappingMatrix( m_envMapParam.matMap ) ;
			render.SetEnvironmentMappingImage
						( m_envMapParam.pImage, m_envMapParam.typeMap ) ;
			render.SetEnvironmentMappingImage
				( m_envMapParam.pImage,
					m_envMapParam.typeMap
					| S3DRenderContextInterface::envMappingRefraction ) ;
		}
		else
		{
			render.SetEnvironmentMappingImage( nullptr, 0 ) ;
			render.SetEnvironmentMappingImage
				( nullptr, S3DRenderContextInterface::envMappingRefraction ) ;
		}
	}
	else
	{
		S3DMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		render.SetEnvironmentMappingMatrix( matI ) ;
		render.SetEnvironmentMappingImage
			( nullptr, RenderContext::envMappingHemisphere ) ;
		render.SetEnvironmentMappingImage
			( nullptr, RenderContext::envMappingHemisphere
					| S3DRenderContextInterface::envMappingRefraction ) ;
	}
	render.SetOffsetBorderColor( m_borderParam.rgbBorder.ui32 ) ;
	render.SetOffsetBorderCoefficient
			( m_borderParam.aThickness, m_borderParam.bThickness ) ;
	//
	render.SetShadingFlag( m_typeShading | shadingTextureSmoothing ) ;
}

// 大域環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetGlobalEnvironmentMapping
	( S3DRenderContextInterface& render,
		SGLImageObject * pViewport,
		SGLImageObject * pDepth,
		S3DScene::EnvironmentMappingSource emsReflection,
		S3DScene::EnvironmentMappingSource emsRefraction ) const
{
	if ( emsReflection == envmapSourceViewport )
	{
		render.SetEnvironmentMappingImage
			( pViewport,
				S3DRenderContextInterface::envMappingViewport ) ;
	}
	if ( emsRefraction == envmapSourceViewport )
	{
		render.SetEnvironmentMappingImage
			( pViewport,
				S3DRenderContextInterface::envMappingViewport
				| S3DRenderContextInterface::envMappingRefraction ) ;
	}
	render.SetEnvironmentMappingImage
		( pDepth, S3DRenderContextInterface::envMappingViewportDepth ) ;
	//
	if ( m_flagDynEnvMap && (m_pWorkDynMapCube != nullptr) )
	{
		S3DMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		render.SetEnvironmentMappingMatrix( matI ) ;
		if ( emsReflection != envmapSourceViewport )
		{
			render.SetEnvironmentMappingImage
				( m_pWorkDynMapCube,
					S3DRenderContextInterface::envMappingCube ) ;
		}
		if ( emsRefraction != envmapSourceViewport )
		{
			render.SetEnvironmentMappingImage
				( m_pWorkDynMapCube,
					S3DRenderContextInterface::envMappingCube
					| S3DRenderContextInterface::envMappingRefraction ) ;
		}
	}
	else if ( m_envMapParam.pImage != nullptr )
	{
		render.SetEnvironmentMappingMatrix( m_envMapParam.matMap ) ;
		if ( emsReflection != envmapSourceViewport )
		{
			render.SetEnvironmentMappingImage
				( m_envMapParam.pImage, m_envMapParam.typeMap ) ;
		}
		if ( emsRefraction != envmapSourceViewport )
		{
			render.SetEnvironmentMappingImage
				( m_envMapParam.pImage,
					m_envMapParam.typeMap
					| S3DRenderContextInterface::envMappingRefraction ) ;
		}
	}
	else
	{
		S3DMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		render.SetEnvironmentMappingMatrix( matI ) ;
		if ( emsReflection != envmapSourceViewport )
		{
			render.SetEnvironmentMappingImage
				( nullptr, RenderContext::envMappingHemisphere ) ;
		}
		if ( emsRefraction != envmapSourceViewport )
		{
			render.SetEnvironmentMappingImage
				( nullptr, RenderContext::envMappingHemisphere
						| S3DRenderContextInterface::envMappingRefraction ) ;
		}
	}
}

// エフェクタ描画処理
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderEffectors
	( S3DRenderContextInterface& renderDst,
		S3DRenderContextInterface& renderLayered,
		const SSystem::SPointerArray<S3DScene::DrawLayerEffector>& aEffect,
		S3DScene::EffectInternalInfo& eii, uint32_t maskReqBuffers )
{
	SGLImageObject *	pSaveColor0 = eii.pEffectColor[0] ;
	if ( (maskReqBuffers & (1 << renderTargetComposed))
		&& (eii.pEffectColor[0] != m_pTargetLayered) )
	{
		if ( m_pTargetLayered != m_pDrawTargetLayered )
		{
			m_renderSampler.AttachTargetImage( m_pTargetLayered, nullptr ) ;
			m_renderSampler.AttachMultiTargetImages( nullptr, 0 ) ;
			m_renderSampler.CopyBufferFrom( renderDst, S3DRenderContext::copyBufferColor ) ;
		}
		else
		{
			SGLPaintParam	pp ;
			renderLayered.FillClearTarget( 0, PaintContext::clearTargetColor ) ;
			renderLayered.DrawImage( pp, eii.pTargetColor[0], nullptr ) ;
			renderLayered.Finish() ;
		}
		eii.pEffectColor[0] = m_pTargetLayered ;
	}
	for ( size_t i = 0; i < aEffect.GetLength(); i ++ )
	{
		DrawLayerEffector *	pEffector = aEffect.GetAt( i ) ;
		if ( pEffector != nullptr )
		{
			pEffector->DrawEffect
				( *this, renderDst,
					eii.pEffectColor,
					eii.nTargetColors, eii.pEffectDepth ) ;
		}
	}
	eii.pEffectColor[0] = pSaveColor0 ;
}

// S3DPhysicsScene 取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::PhysicsSceneItem& S3DScene::PhysicsScene( void ) const
{
	return	((S3DScene*)this)->m_physScene ;
}

// 物理演算当たり判定誤差設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetPhysicsErrorGap
	( const S3DPhysicsScene::ErrorGap& errGap )
{
	m_errGapPhys = errGap ;
}

// 物理演算時間を手動実行するか？
// ※デフォルト（false）では OnTimer から AddPhysicsTime が呼び出される
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetManualPhysTimeFlag( bool flagManualTime )
{
	m_flagManualPhysTime = flagManualTime ;
}

// 物理演算時間進行
//////////////////////////////////////////////////////////////////////////////
void S3DScene::AddPhysicsTime( float32_t secPhysTime )
{
	m_secPhysAdvance += secPhysTime ;
}

// 物理演算処理パフォーマンス取得
//////////////////////////////////////////////////////////////////////////////
void S3DScene::GetPhysicsPerformanceLog( PhysicsPerformanceLog& ppl ) const
{
	ppl.countProcess = m_nPhysProcess ;
	ppl.msecAccTime = m_msecAccPhysProcess ;
	ppl.msecMaxTime = m_msecMaxPhysProcess ;
}

// パフォーマンスログ・リセット
//////////////////////////////////////////////////////////////////////////////
void S3DScene::ResetPhysicsPerformanceLog( void )
{
	m_nPhysProcess = 0 ;
	m_msecAccPhysProcess = 0.0 ;
	m_msecMaxPhysProcess = 0.0 ;
}

// フィールドコリジョンの更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetUpdateFieldCollisionFlag( void )
{
	m_fUpdateFieldColl = true ;
}

// コリジョン更新済みフラグ
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::IsCollisionUpdated( void ) const
{
	return	m_fUpdatedCollision ;
}

void S3DScene::ResetCollisionUpdatedFlag( void )
{
	m_fUpdatedCollision = false ;
}

// アイテムと球との交差判定
//////////////////////////////////////////////////////////////////////////////
S3DScene::Item * S3DScene::IsItemHitAgainstSphere
	( const S3DDVector& vPos,
		float fpRadius, S3DCollision::Result& rsHit ) const
{
	rsHit.matLocal.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
	rsHit.vLocalBase = S3DVector( 0, 0, 0 ) ;
	rsHit.fpDistance = 1.0e+16f ;
	rsHit.pMesh = nullptr ;
	rsHit.pqpcHit = nullptr ;
	//
	if ( IsHitAgainstSphere( vPos, fpRadius, rsHit ) )
	{
		if ( rsHit.pMesh && rsHit.pMesh->pUserData )
		{
			S3DScene::Item *
				pItem = ESLTypeCast<Item>( rsHit.pMesh->pUserData ) ;
			if ( pItem == nullptr )
			{
				return	(S3DScene::Item*) &m_physScene ;
			}
			return	pItem ;
		}
	}
	return	nullptr ;
}

S3DScene::Item * S3DScene::IsItemHitAgainstSphereInMeshNextPoly
	( const S3DDVector& vPos,
		float fpRadius, S3DCollision::Result& rsHit ) const
{
	const S3DCollision::MeshCollision *	pmcMesh = rsHit.pMesh ;
	if ( pmcMesh == nullptr )
	{
		return	nullptr ;
	}
	rsHit.matLocal.InitializeMatrix( S3DVector( 1, 1, 1 ) ) ;
	rsHit.vLocalBase = S3DVector( 0, 0, 0 ) ;
	rsHit.fpDistance = 1.0e+16f ;
	rsHit.pMesh = nullptr ;
	rsHit.pqpcHit = nullptr ;
	//
	if ( S3DCollision::DoesMeshHitAgainstSphere
		( pmcMesh, vPos, fpRadius, rsHit, rsHit.iPolygon + 1 ) )
	{
		S3DCollision::ComputeGlobalOfHitAgainstSphere( rsHit, *pmcMesh ) ;
		//
		if ( rsHit.pMesh && rsHit.pMesh->pUserData )
		{
			return	ESLTypeCast<Item>( rsHit.pMesh->pUserData ) ;
		}
	}
	return	nullptr ;
}

// アイテムと線分との交差判定
//////////////////////////////////////////////////////////////////////////////
S3DScene::Item * S3DScene::IsItemSegmentCrossing
	( const S3DDVector& vPos0, const S3DDVector& vPos1,
			float fpErrorGap, S3DCollision::Result& rsCross ) const
{
	rsCross.fpDistance = (float32_t) (vPos1 - vPos0).Absolute() ;
	rsCross.pMesh = nullptr ;
	rsCross.pqpcHit = nullptr ;
	//
	if ( IsSegmentCrossing( vPos0, vPos1, fpErrorGap, rsCross ) )
	{
		if ( rsCross.pMesh && rsCross.pMesh->pUserData )
		{
			S3DScene::Item *
				pItem = ESLTypeCast<Item>( rsCross.pMesh->pUserData ) ;
			if ( pItem == nullptr )
			{
				return	(S3DScene::Item*) &m_physScene ;
			}
			return	pItem ;
		}
	}
	return	nullptr ;
}

// 範囲取得
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::GetCollisionRange( S3DDVector& vCenter, double& fpRadius ) const
{
	const S3DCollision *	pCollisions[3] =
	{
		&m_colField, &m_colItems, &(m_physScene.GetCollision())
	} ;
	bool	flagRange = false ;
	for ( int i = 0; i < 3; i ++ )
	{
		S3DDVector	vItemsCenter ;
		double		fpItemRadius ;
		if ( pCollisions[i]->GetCollisionRange( vItemsCenter, fpItemRadius ) )
		{
			if ( flagRange )
			{
				S3DDVector	vDelta = vItemsCenter - vCenter ;
				double	fpDelta = vDelta.Absolute() ;
				double	fpMin = esl_fmin( - fpRadius, fpDelta - fpItemRadius ) ;
				double	fpMax = esl_fmax( fpRadius, fpDelta + fpItemRadius ) ;
				vDelta.Normalize() ;
				//
				vCenter += vDelta * ((fpMin + fpMax) * 0.5) ;
				fpRadius = (fpMax - fpMin) * 0.5 ;
			}
			else
			{
				vCenter = vItemsCenter ;
				fpRadius = fpItemRadius ;
				flagRange = true ;
			}
		}
	}
	return	flagRange ;
}

// 凸形状の内側判定
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::IsSphereInclusive
	( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsIncluded ) const
{
	if ( m_physScene.GetCollision().IsSphereInclusive( vPos, fpRadius, rsIncluded ) )
	{
		return	true ;
	}
	if ( m_colField.IsSphereInclusive( vPos, fpRadius, rsIncluded ) )
	{
		return	true ;
	}
	return	m_colItems.IsSphereInclusive( vPos, fpRadius, rsIncluded ) ;
}

// 球との交差判定
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::IsHitAgainstSphere
	( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsHit ) const
{
	if ( m_colField.IsHitAgainstSphere( vPos, fpRadius, rsHit ) )
	{
		return	true ;
	}
	if ( m_colItems.IsHitAgainstSphere( vPos, fpRadius, rsHit ) )
	{
		return	true ;
	}
	return	m_physScene.GetCollision().IsHitAgainstSphere( vPos, fpRadius, rsHit ) ;
}

// 線分との交差判定
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::IsSegmentCrossing
	( const S3DDVector& vPos0, const S3DDVector& vPos1,
					float fpErrorGap, S3DCollisionResult& rsCross ) const
{
	bool	flagCross =
		m_physScene.GetCollision().
			IsSegmentCrossing( vPos0, vPos1, fpErrorGap, rsCross ) ;
	flagCross |=
		m_colField.IsSegmentCrossing( vPos0, vPos1, fpErrorGap, rsCross ) ;
	return	m_colItems.IsSegmentCrossing
					( vPos0, vPos1, fpErrorGap, rsCross ) | flagCross ;
}

// 視線ベクトル計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RayProjectionFor
	( S3DDVector& vRay, S3DDVector& vRayOrigin,
		const S2DDVector& vPosOnScreen, Camera * pCamera ) const
{
	S3DDMatrix	matCameraSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posCameraSpace( 0, 0, 0 ) ;
	if ( pCamera != nullptr )
	{
		pCamera->CalcItemLinkTransformation( matCameraSpace, posCameraSpace ) ;
		//
		S3DDVector	vCameraTarget =
			matCameraSpace * pCamera->m_vTarget + posCameraSpace ;
		S3DDVector	vCameraPos =
			matCameraSpace * pCamera->m_space.m_vCenter + posCameraSpace ;
		S3DDVector	vCameraTop = matCameraSpace * pCamera->m_vTop ;
		//
		posCameraSpace =
			matCameraSpace.CameraAngleOf
				( vCameraTarget, vCameraPos, vCameraTop ) ;
		//
		vRayOrigin = vCameraPos ;
	}
	else
	{
		vRayOrigin = S3DDVector( 0, 0, 0 ) ;
	}
	vRay = S3DDVector
		( (vPosOnScreen.x
				- m_projCurrent.vScreen.x) * m_projCurrent.fpPixelAspect,
				vPosOnScreen.y - m_projCurrent.vScreen.y,
				m_projCurrent.vScreen.z * m_projCurrent.fpZoom ) ;
	vRay.Normalize() ;
	//
	S3DDMatrix	matICamera ;
	matICamera.InverseOf( matCameraSpace ) ;
	matICamera.RevolveVector( vRay ) ;
}

// 平面投影座標計算
//////////////////////////////////////////////////////////////////////////////
const S2DDVector& S3DScene::PlaneProjectionOf
	( S2DDVector& vScreen,
		const S3DScene::Camera * pCamera,
		const S3DScene::Space * pTargetSpace,
		const S3DDVector& vBaseO,
		const S3DDVector& vBaseX, const S3DDVector& vBaseY ) const
{
	//
	// カメラ変換行列
	//
	S3DDMatrix	matCameraSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posCameraSpace( 0, 0, 0 ) ;
	if ( pCamera != nullptr )
	{
		CalcCameraTransformation
			( matCameraSpace, posCameraSpace, pCamera ) ;
	}
	//
	// 空間変換
	//
	S3DDMatrix	matTargetSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posTargetSpace( 0, 0, 0 ) ;
	if ( pTargetSpace != nullptr )
	{
		pTargetSpace->CalcGlobalTransformation
						( matTargetSpace, posTargetSpace ) ;
	}
	//
	// 基底ベクトル
	//
	S3DDVector	vO = matTargetSpace * vBaseO + posTargetSpace ;
	S3DDVector	vX = matTargetSpace * vBaseX ;
	S3DDVector	vY = matTargetSpace * vBaseY ;
	vO = matCameraSpace * vO - posCameraSpace ;
	vX = matCameraSpace * vX ;
	vY = matCameraSpace * vY ;
	//
	// 平面との交点
	//
	S3DDVector	vRay
		( (vScreen.x - m_projCurrent.vScreen.x) * m_projCurrent.fpPixelAspect,
			vScreen.y - m_projCurrent.vScreen.y,
			m_projCurrent.vScreen.z * m_projCurrent.fpZoom ) ;
	S3DDVector	vXY = vX * vY ;
	vRay.Normalize() ;
	vXY.Normalize() ;
	//
	double	r = (vRay | vXY) ;
	if ( fabs( r ) < 1.0e-8 )
	{
		vScreen.x = 0 ;
		vScreen.y = 0 ;
		return	vScreen ;
	}
	r = (vO | vXY) / r ;
	//
	S3DDVector	vHit = vRay * r ;
	//
	// 投影座標系へ変換
	//
	S3DDVector	vLocal = vHit - vO ;
	vScreen.x = vXY.InnerProduct( vLocal * vY ) ;
	vScreen.y = vXY.InnerProduct( vX * vLocal ) ;
	return	vScreen ;
}

// 座標投影
//////////////////////////////////////////////////////////////////////////////
const S2DDVector& S3DScene::ViewProjectionOf
	( S2DDVector& vScreen, const S3DDVector& vPos ) const
{
	return	m_projCurrent.ViewProjectionOf( vScreen, vPos ) ;
/*
	if ( m_projCurrent.vScreen.z != 0.0f )
	{
		double	dy = m_projCurrent.vScreen.z * m_projCurrent.fpZoom / vPos.z ;
		double	dx = dy / m_projCurrent.fpPixelAspect ;
		vScreen.x = vPos.x * dx + m_projCurrent.vScreen.x ;
		vScreen.y = vPos.y * dy + m_projCurrent.vScreen.y ;
	}
	else
	{
		double	dy = m_projCurrent.fpZoom ;
		double	dx = dy / m_projCurrent.fpPixelAspect ;
		vScreen.x = vPos.x * dx + m_projCurrent.vScreen.x ;
		vScreen.y = vPos.y * dy + m_projCurrent.vScreen.y ;
	}
	return	vScreen ;
*/
}

const S2DDVector& S3DScene::ViewProjectionAndScaleOf
	( S2DDVector& vScreen, S2DDVector& vScale, const S3DDVector& vPos ) const
{
	return	m_projCurrent.ViewProjectionAndScaleOf( vScreen, vScale, vPos ) ;
/*
	if ( m_projCurrent.vScreen.z != 0.0f )
	{
		double	dy = m_projCurrent.vScreen.z * m_projCurrent.fpZoom / vPos.z ;
		double	dx = dy / m_projCurrent.fpPixelAspect ;
		vScale.x = dx ;
		vScale.y = dy ;
		vScreen.x = vPos.x * dx + m_projCurrent.vScreen.x ;
		vScreen.y = vPos.y * dy + m_projCurrent.vScreen.y ;
	}
	else
	{
		double	dy = m_projCurrent.fpZoom ;
		double	dx = dy / m_projCurrent.fpPixelAspect ;
		vScale.x = dx ;
		vScale.y = dy ;
		vScreen.x = vPos.x * dx + m_projCurrent.vScreen.x ;
		vScreen.y = vPos.y * dy + m_projCurrent.vScreen.y ;
	}
	return	vScreen ;
*/
}

// 現在のカメラで球（大域座標）が視界に入るか判定する
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::IsSphereIntoView
	( const S3DDMatrix& matSpace,
		const S3DDVector& vGlobalPos,
		double fpLocalRadius, const SGLImageRect& rectView ) const
{
	//
	// 座標変換
	//
	S3DDVector	vProjPos = vGlobalPos ;
	TransformByCurrentCamera( vProjPos ) ;
	//
	// 視界境界面法線
	//
	S3DDVector	vFrame[5] ;		// 背上下左右枠
	float32_t	zScreen = m_projCurrent.vScreen.z * m_projCurrent.fpZoom ;
	vFrame[0] = S3DDVector( 0, 0, -1 ) ;
	vFrame[1] =
		S3DDVector( 0, - zScreen,
					(rectView.y - m_projCurrent.vScreen.y) ) ;
	vFrame[2] =
		S3DDVector( 0, zScreen,
					- (rectView.x + rectView.h - m_projCurrent.vScreen.y) ) ;
	vFrame[3] =
		S3DDVector( - zScreen, 0,
					(rectView.x - m_projCurrent.vScreen.x)
											* m_projCurrent.fpPixelAspect ) ;
	vFrame[4] =
		S3DDVector( zScreen, 0,
					- (rectView.x + rectView.w - m_projCurrent.vScreen.x)
											* m_projCurrent.fpPixelAspect ) ;
	//
	// 視界からのはみ出しを判定する
	//
	S3DDMatrix	matISpace ;
	bool		flagISpace = false ;
	double		r ;
	for ( int i = 0; i < 5; i ++ )
	{
		vFrame[i].Normalize() ;
		r = vFrame[i].InnerProduct( vProjPos ) ;
		if ( r > 0 )
		{
			//
			// はみ出し距離に相当するベクトルをローカル空間に変換し
			// ローカル空間でのはみ出し距離を算出し比較する
			//
			S3DDVector	vLocal = vFrame[i] * r ;
			if ( !flagISpace )
			{
				matISpace.InverseOf( m_matCamera * matSpace ) ;
				flagISpace = true ;
			}
			matISpace.RevolveVector( vLocal ) ;
			r = vLocal.Absolute() ;
			if ( r > fpLocalRadius )
			{
				// 枠外
				return	false ;
			}
		}
	}
	return	true ;
}

// カメラからの距離が非表示距離を越えているか判定
//////////////////////////////////////////////////////////////////////////////
bool S3DScene::IsBehindFarHiddenDistance( S3DRenderContextInterface& render ) const
{
	S3DDMatrix	matSpace ;
	S3DDVector	posSpace ;
	if ( render.GetMatrixTransformation( matSpace, posSpace ) )
	{
		return	false ;
	}
	return	(TransformByCurrentCamera( posSpace ).Absolute()
										>= m_fpVisibleFarDistance) ;
}

bool S3DScene::IsAheadNearHiddenDistance( S3DRenderContextInterface& render ) const
{
	S3DDMatrix	matSpace ;
	S3DDVector	posSpace ;
	if ( render.GetMatrixTransformation( matSpace, posSpace ) )
	{
		return	false ;
	}
	return	(TransformByCurrentCamera( posSpace ).Absolute()
										<= m_fpVisibleNearDistance) ;
}

// カメラオブジェクト削除通知（参照している場合に安全に参照解除）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::NotifyDeleteCamera( Camera * pCamera )
{
	Lock() ;
	if ( m_refMainCamera.GetReference() == pCamera )
	{
		m_refMainCamera = nullptr ;
	}
	if ( m_refSoundCamera.GetReference() == pCamera )
	{
		m_refSoundCamera = nullptr ;
	}
	ESLAssert( m_pCurrentCamera != pCamera ) ;
	Unlock() ;
}

// メインカメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetMainCamera( S3DScene::Camera * pCamera )
{
	Lock() ;
	m_refMainCamera = pCamera ;
	Unlock() ;
}

// 現在のカメラに設定し各種パラメータを計算
//////////////////////////////////////////////////////////////////////////////
void S3DScene::SetCurrentCamera( S3DScene::Camera * pCamera )
{
	if ( pCamera != nullptr )
	{
		S3DDVector	posCameraSpace( 0, 0, 0 ) ;
		pCamera->CalcItemLinkTransformation( m_matCameraSpace, posCameraSpace ) ;
		//
		m_pCurrentCamera = pCamera ;
		m_vCameraTarget = m_matCameraSpace
							* pCamera->m_vTarget + posCameraSpace ;
		m_vCameraPos = m_matCameraSpace
							* pCamera->m_space.m_vCenter + posCameraSpace ;
		m_vCameraTop = m_matCameraSpace * pCamera->m_vTop ;
		//
		m_vCameraTrans =
			m_matCamera.CameraAngleOf
				( m_vCameraTarget, m_vCameraPos, m_vCameraTop ) ;
		m_matICamera.InverseOf( m_matCamera ) ;
		//
		if ( m_fRenderingOffset )
		{
			m_vRenderingOffset = - m_vCameraPos ;
		}
		else
		{
			m_vRenderingOffset.x = 0 ;
			m_vRenderingOffset.y = 0 ;
			m_vRenderingOffset.z = 0 ;
		}
	}
	else
	{
		m_pCurrentCamera = nullptr ;
		m_vRenderingOffset.x = 0 ;
		m_vRenderingOffset.y = 0 ;
		m_vRenderingOffset.z = 0 ;
	}
}

// カメラによる変換行列計算（修正後座標）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::CalcCameraTransformation
	( S3DDMatrix& matCamera, S3DDVector& vCamera,
							const S3DScene::Camera * pCamera )
{
	S3DDMatrix	matCameraSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posCameraSpace( 0, 0, 0 ) ;
	pCamera->CalcItemLinkTransformation( matCameraSpace, posCameraSpace ) ;
	//
	S3DDVector	vCameraTarget =
					matCameraSpace
						* pCamera->m_vTarget + posCameraSpace ;
	S3DDVector	vCameraPos =
					matCameraSpace
						* pCamera->m_space.m_vCenter + posCameraSpace ;
	S3DDVector	vCameraTop = matCameraSpace * pCamera->m_vTop ;
	//
	vCamera = matCamera.CameraAngleOf
					( vCameraTarget, vCameraPos, vCameraTop ) ;
}

// カメラによる変換行列計算（修正前座標）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::CalcUnmodifiedCameraTransformation
	( S3DDMatrix& matCamera,
		S3DDVector& vCamera, const Camera * pCamera )
{
	S3DDMatrix	matCameraSpace( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posCameraSpace( 0, 0, 0 ) ;
	pCamera->CalcItemLinkTransformation( matCameraSpace, posCameraSpace ) ;
	//
	S3DDVector	vCameraTarget =
					matCameraSpace
						* pCamera->GetCameraTarget() + posCameraSpace ;
	S3DDVector	vCameraPos =
					matCameraSpace
						* pCamera->GetCameraPosition() + posCameraSpace ;
	S3DDVector	vCameraTop = matCameraSpace * pCamera->GetCameraTop() ;
	//
	vCamera = matCamera.CameraAngleOf
					( vCameraTarget, vCameraPos, vCameraTop ) ;
}

// ローカル空間から現在のカメラ視点の座標へ変換
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DScene::TransformByCurrentCamera
		( S3DScene::Item * pItem, S3DDVector& vPos ) const
{
	ESLAssert( pItem != nullptr ) ;
	if ( pItem == nullptr )
	{
		return	TransformByCurrentCamera( vPos ) ;
	}
	pItem->m_space.m_matTransformation.RevolveVector( vPos ) ;
	vPos += pItem->m_space.m_vCenter ;
	//
	pItem->m_matLink.RevolveVector( vPos ) ;
	vPos += pItem->m_vLink ;
	//
	return	TransformByCurrentCamera( pItem->GetReferenceSpace(), vPos ) ;
}

const S3DDVector& S3DScene::TransformByCurrentCamera
		( S3DScene::Space * pSpace, S3DDVector& vPos ) const
{
	while ( pSpace != nullptr )
	{
		pSpace->m_matTransformation.RevolveVector( vPos ) ;
		vPos += pSpace->m_vCenter ;
		pSpace = pSpace->GetParentSpace() ;
	}
	return	TransformByCurrentCamera( vPos ) ;
}

// サウンド再生開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DScene::PlaySound( S3DScene::SoundItem * pSound, uint64_t nFlags )
{
	SGLAudioPlayerInterface *	pPlayer = pSound->GetAudioPlayer() ;
	if ( pPlayer != nullptr )
	{
		Camera *	pCamera = GetSoundCamera() ;
		if ( pCamera != nullptr )
		{
			pSound->ApplySoundVolume( pCamera ) ;
		}
		return	pPlayer->Play( nFlags ) ;
	}
	else
	{
		return	sglErrFailed ;
	}
}

// アイテム作用フラグ更新／カメラ・光源情報収集
//////////////////////////////////////////////////////////////////////////////
void S3DScene::UpdateBehaviorFlags
		( S3DScene::Space * pSpace, bool flagSpaceVisible )
{
	SyncItemDispatcher *	pSync = GetSyncDispatcher() ;
	//
	pSpace->m_aTempEffector.RemoveAll() ;
	if ( m_enabledAsyncProc )
	{
		SetFenceAsyncProcedure() ;
		PostAsyncProcedure
			( new SpaceDelayRemoveProc( pSpace->m_aDelayRemove ) ) ;
	}
	else
	{
		pSpace->m_aDelayRemove.RemoveAll() ;
	}
	//
	size_t			i ;
	size_t			countChildren = pSpace->m_children.GetLength() ;
	Space*const*	ppChildren = pSpace->m_children.GetConstArray() ;
	uint32_t		flagsBehavior = 0 ;
	uint32_t		maskItemClasses = 0 ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		if ( pChild != nullptr )
		{
			ESLAssert( pChild->m_refParent.GetReference() == pSpace ) ;
			if ( !(pChild->m_flagsSpaceBehavior & itemIgnore) )
			{
				AsyncDispatchUpdateBehaviorFlags
					( pChild, flagSpaceVisible
								& !(pChild->m_flagsSpaceBehavior
												& itemSpaceHidden), pSync ) ;
				/*
				UpdateBehaviorFlags
					( pChild, flagSpaceVisible
								& !(pChild->m_flagsSpaceBehavior
												& itemSpaceHidden) ) ;
				flagsBehavior |=
						pChild->m_flagsBehavior & ~itemSpaceLocalFlags ;
				maskItemClasses |= pChild->m_maskItemClasses ;
				*/
			}
		}
	}
	size_t		countItems = pSpace->m_items.GetLength() ;
	Item*const*	ppItems = pSpace->m_items.GetConstArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		if ( pItem != nullptr )
		{
			if ( flagSpaceVisible
				&& (pItem->m_flagsBehavior & (itemVisible | itemCollision))
				&& !(pItem->m_flagsBehavior & itemIgnore) )
			{
				flagsBehavior |= pItem->m_flagsBehavior & itemInheritFlags ;
				maskItemClasses |= (1 << pItem->m_classItem) ;
				maskItemClasses |= pItem->m_maskClasses ;
				//
				if ( pItem->m_classItem == classLight )
				{
					m_csEffectorSync.Lock() ;
					Light *	pLight = ESLTypeCast<Light>( pItem ) ;
					if ( pLight != nullptr )
					{
						m_ptaLights.Add( pLight ) ;
						//
						S3DDMatrix	matLight ;
						S3DDVector	vLight ;
						pItem->CalcGlobalTransformation( matLight, vLight ) ;
						//
						AddLightDazzlement
							( matLight, vLight, pLight->m_light ) ;
					}
					m_csEffectorSync.Unlock() ;
					//
					pItem->OnRenderEvent( *this, classLight ) ;
				}
				else if ( pItem->m_classItem == classCamera )
				{
					pItem->OnRenderEvent( *this, classCamera ) ;
				}
			}
			if ( pItem->m_classItem == classCamera )
			{
				m_csEffectorSync.Lock() ;
				if ( m_pFirstCamera == nullptr )
				{
					m_pFirstCamera = ESLTypeCast<Camera>( pItem ) ;
				}
				m_csEffectorSync.Unlock() ;
			}
			if ( pItem->m_flagsBehavior & itemOwnerBehavior )
			{
				pItem->OnUpdateBehavior( *this ) ;
				//
				if ( flagSpaceVisible
					&& (pItem->m_flagsBehavior & (itemVisible | itemCollision))
					&& !(pItem->m_flagsBehavior & itemIgnore) )
				{
					flagsBehavior |= pItem->m_flagsBehavior & itemInheritFlags ;
					maskItemClasses |= (1 << pItem->m_classItem) ;
					maskItemClasses |= pItem->m_maskClasses ;
				}
			}
		}
	}
	//
	ReleaseSyncDispatcher( pSync ) ;
	//
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		if ( pChild != nullptr )
		{
			ESLAssert( pChild->m_refParent.GetReference() == pSpace ) ;
			if ( !(pChild->m_flagsSpaceBehavior & itemIgnore) )
			{
				flagsBehavior |=
						pChild->m_flagsBehavior & ~itemSpaceLocalFlags ;
				maskItemClasses |= pChild->m_maskItemClasses ;
			}
		}
	}
	//
	pSpace->m_flagsBehavior =
				flagsBehavior | pSpace->m_flagsSpaceBehavior ;
	pSpace->m_maskItemClasses =
				(maskItemClasses | pSpace->m_maskSpaceClasses)
									& ~pSpace->m_maskLayeredClasses ;
	//
	if ( pSpace->m_flagsSpaceBehavior & itemOwnerBehavior )
	{
		pSpace->OnUpdateBehavior( *this ) ;
	}
	if ( pSpace->m_maskSpaceClasses & (1 << classLayeredSpace) )
	{
		m_csEffectorSync.Lock() ;
		//
		Space *const*	ppSpaces = m_aLayeredSpaces.GetConstArray() ;
		size_t			nLayeredCount = m_aLayeredSpaces.GetLength() ;
		int32_t			zPriority = pSpace->m_zLayeredPriority ;
		for ( i = 0; i < nLayeredCount; i ++ )
		{
			ESLAssert( ppSpaces[i] != nullptr ) ;
			if ( zPriority > ppSpaces[i]->m_zLayeredPriority )
			{
				break ;
			}
		}
		m_aLayeredSpaces.InsertAt( i, pSpace ) ;
		//
		m_csEffectorSync.Unlock() ;
	}
}

void S3DScene::CollectBehaviorFlags
	( S3DScene::Space * pSpace, bool flagSpaceVisible )
{
	size_t			i ;
	size_t			countChildren = pSpace->m_children.GetLength() ;
	Space*const*	ppChildren = pSpace->m_children.GetConstArray() ;
	uint32_t		flagsBehavior = 0 ;
	uint32_t		maskItemClasses = 0 ;
	for ( i = 0; i < countChildren; i ++ )
	{
		Space *	pChild = ppChildren[i] ;
		if ( pChild != nullptr )
		{
			ESLAssert( pChild->m_refParent.GetReference() == pSpace ) ;
			if ( !(pChild->m_flagsSpaceBehavior & itemIgnore) )
			{
				CollectBehaviorFlags
					( pChild, flagSpaceVisible
								& !(pChild->m_flagsSpaceBehavior
												& itemSpaceHidden) ) ;
				flagsBehavior |=
						pChild->m_flagsBehavior & ~itemSpaceLocalFlags ;
				maskItemClasses |= pChild->m_maskItemClasses ;
			}
		}
	}
	size_t		countItems = pSpace->m_items.GetLength() ;
	Item*const*	ppItems = pSpace->m_items.GetConstArray() ;
	for ( i = 0; i < countItems; i ++ )
	{
		Item*	pItem = ppItems[i] ;
		if ( pItem != nullptr )
		{
			if ( flagSpaceVisible
				&& (pItem->m_flagsBehavior & (itemVisible | itemCollision))
				&& !(pItem->m_flagsBehavior & itemIgnore) )
			{
				flagsBehavior |= pItem->m_flagsBehavior & itemInheritFlags ;
				maskItemClasses |= (1 << pItem->m_classItem) ;
				maskItemClasses |= pItem->m_maskClasses ;
			}
		}
	}
	pSpace->m_flagsBehavior =
				flagsBehavior | pSpace->m_flagsSpaceBehavior ;
	pSpace->m_maskItemClasses =
				(maskItemClasses | pSpace->m_maskSpaceClasses)
									& ~pSpace->m_maskLayeredClasses ;
}

// レンダリングイベント
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderSceneEventItems
	( S3DScene::Space* pSpace, S3DScene::ItemClass clsItem )
{
	if ( pSpace->m_maskItemClasses & (1 << clsItem) )
	{
		BeginAsyncDispatcher() ;
		//
		m_classCurrentRendering = clsItem ;
		pSpace->OnRenderEvent( *this, clsItem ) ;
		//
		EndAsyncDispatcher() ;
	}
}

// コリジョン
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderSceneCollision
	( S3DCollision& collision,
		const S3DScene::Space* pSpace, uint32_t maskClasses )
{
	if ( pSpace->m_maskItemClasses & maskClasses )
	{
		m_maskCurrentCollision = maskClasses ;
		collision.SetSceneClassesMask( maskClasses ) ;
		collision.SetUserClassesMask( 0 ) ;
		pSpace->RenderCollision( *this, collision, maskClasses ) ;
	}
}

// レンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderSceneItems
	( S3DRenderContextInterface& render,
		const S3DScene::Space* pSpace,
		S3DScene::ItemClass clsItem,
		uint64_t flagsExclusion, uint32_t optShader )
{
	if ( pSpace->m_maskItemClasses & (1 << clsItem) )
	{
		m_classCurrentRendering = clsItem ;
		//
		S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		render.PushTransformation() ;
		render.AppendMatrixTransformation( matI, m_vRenderingOffset ) ;
		pSpace->RenderModel
			( *this, render, optShader, clsItem, flagsExclusion ) ;
		render.PopTransformation() ;
		render.Flush() ;
	}
}

// レイヤー空間レンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderLayeredSpace
	( S3DRenderContextInterface& render,
		const S3DScene::Space* pSpace,
		uint64_t flagsExclusion, uint32_t optShader )
{
	ESLAssert( pSpace != nullptr ) ;
	uint32_t	maskClasses = pSpace->m_maskLayeredClasses ;
	for ( int i = classFirstRenderClass;
				(maskClasses != 0) && (i <= classLastRenderClass); i ++ )
	{
		S3DDMatrix	matParent( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
		S3DDVector	vParentPos = m_vRenderingOffset ;
		Space *		pParent = pSpace->GetParentSpace() ;
		if ( pParent != nullptr )
		{
			pParent->CalcGlobalTransformation( matParent, vParentPos ) ;
			vParentPos += m_vRenderingOffset ;
		}
		uint32_t	mask = (1 << i) ;
		if ( maskClasses & mask )
		{
			m_classCurrentRendering = (S3DScene::ItemClass) i ;
			//
			S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			render.PushTransformation() ;
			render.SetMatrixTransformation( matParent, vParentPos ) ;
			pSpace->RenderModel
				( *this, render, optShader,
					(S3DScene::ItemClass) i, flagsExclusion ) ;
			render.PopTransformation() ;
			render.Flush() ;
			//
			maskClasses &= ~mask ;
		}
	}
}

// 全レイヤード空間をレンダリング
//////////////////////////////////////////////////////////////////////////////
void S3DScene::RenderAllLayeredSpaces
	( S3DRenderContextInterface& render,
		uint64_t flagsExclusion, uint32_t optShader )
{
	for ( size_t i = 0; i < m_aLayeredSpaces.GetLength(); i ++ )
	{
		Space *	pSpace = m_aLayeredSpaces.GetAt( i ) ;
		ESLAssert( pSpace != nullptr ) ;
		RenderLayeredSpace( render, pSpace, flagsExclusion, optShader ) ;
	}
}

// 画面更新通知（S3DScene::PostSceneUpdate はプレースホルダ）
//////////////////////////////////////////////////////////////////////////////
void S3DScene::PostSceneUpdate( void )
{
}

// レンダリングスレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SSystem::SError S3DScene::Lock( int64_t msecTimeout ) const
{
	return	SSystem::Lock( msecTimeout ) ;
}

SSystem::SError S3DScene::Unlock( void ) const
{
	return	SSystem::Unlock() ;
}

atomic_int_t S3DScene::TestLocked( void ) const
{
	return	SSystem::TestLocked() ;
}

SSystem::SSharableMutex * S3DScene::GetUIThreadMutex( void ) const
{
	return	SSystem::g_mutexGlobal ;
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DScene::GetLQClassName( void ) const
{
	return	L"EntisGLS4.Scene" ;
}

