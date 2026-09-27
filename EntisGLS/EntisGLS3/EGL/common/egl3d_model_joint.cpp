
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
       Copyright (c) 2003-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <egl.h>
#include <math.h>

static const double	pi_rad = 3.141592653589 / 180.0 ;


//////////////////////////////////////////////////////////////////////////////
// モデルジョイント
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DModelJoint, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DModelJoint::E3DModelJoint( void )
	: m_parent( NULL ), m_vmoveParam( 0, 0, 0 )
{
	InitializeMatrix( ) ;
}

E3DModelJoint::E3DModelJoint( E3DPolygonModel * pmodel )
	: m_parent( NULL ), m_vmoveParam( 0, 0, 0 )
{
	InitializeMatrix( ) ;
	//
	m_models.Add( pmodel ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DModelJoint::~E3DModelJoint( void )
{
}

// ジョイント生成
//////////////////////////////////////////////////////////////////////////////
E3DModelJoint * E3DModelJoint::CreateJoint( void ) const
{
	return	new E3DModelJoint ;
}

// パラメータ反映
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::RefreshJoint( void )
{
	m_dfrvmat = m_rvmatParam ;
	m_dfvmove = m_vmoveParam ;
	m_rvmat = m_dfrvmat ;
	m_vmove = m_dfvmove ;
}

// ジョイント回転処理
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::TransformJoint( const E3DModelJoint & mjParent )
{
	mjParent.m_dfrvmat.RevolveMatrix( m_dfrvmat ) ;
	//
	mjParent.m_dfrvmat.RevolveVector( m_dfvmove ) ;
	m_dfvmove += mjParent.m_dfvmove ;
	//
	m_rvmat = m_dfrvmat ;
	m_vmove = m_dfvmove ;
}

// モデル回転処理
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::TransformModel( E3DPolygonModel & model ) const
{
	if ( model.GetVertexCount() > 0 )
	{
		m_rvmat.RevolveVectors
			( model.GetVertexBuffer(),
				model.GetVertexList(),
				&m_vmove, model.GetVertexCount() ) ;
	}
	if ( model.GetNormalCount() > 0 )
	{
		m_rvmat.RevolveVectors
			( model.GetNormalBuffer(),
				model.GetNormalList(),
				NULL, model.GetNormalCount() ) ;
	}
}

// 全てのジョイントを削除
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::DeleteContents( void )
{
	m_models.RemoveAll( ) ;
	m_joints.RemoveAll( ) ;
}

// ジョイント複製
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::CopyModelJoint( const E3DModelJoint & model )
{
	DeleteContents( ) ;
	//
	m_models.Merge( 0, model.m_models ) ;
	//
	for ( int i = 0; i < (int) model.m_joints.GetSize(); i ++ )
	{
		E3DModelJoint *	pJoint = model.m_joints.GetAt( i ) ;
		if ( pJoint == NULL )
			continue ;
		//
		E3DModelJoint *	pDupJoint = pJoint->CreateJoint( ) ;
		pDupJoint->CopyModelJoint( *pJoint ) ;
		m_joints.Add( pDupJoint ) ;
	}
}

// ｘ軸回転
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::RevolveOnX( double rDeg )
{
	m_rvmatParam.RevolveOnX
		( sin( rDeg * pi_rad ), cos( rDeg * pi_rad ) ) ;
}

// ｙ軸回転
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::RevolveOnY( double rDeg )
{
	m_rvmatParam.RevolveOnY
		( sin( rDeg * pi_rad ), cos( rDeg * pi_rad ) ) ;
}

// ｚ軸回転
//////////////////////////////////////////////////////////////////////////////
void E3DModelJoint::RevolveOnZ( double rDeg )
{
	m_rvmatParam.RevolveOnZ
		( sin( rDeg * pi_rad ), cos( rDeg * pi_rad ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// カメラオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DViewPointJoint, E3DModelJoint )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DViewPointJoint::E3DViewPointJoint( void )
	: m_vViewAngle( 0, 0, 1 ), m_vViewPoint( 0, 0, 0 ), m_rRevAngleZ( 0 )
{
	RefreshJoint() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DViewPointJoint::~E3DViewPointJoint( void )
{
}

// パラメータ反映
//////////////////////////////////////////////////////////////////////////////
void E3DViewPointJoint::RefreshJoint( void )
{
	InitializeMatrix( ) ;
	RevolveOnZ( m_rRevAngleZ ) ;
	m_rvmatParam.RevolveByAngleOn( m_vViewAngle ) ;
	//
	m_vmoveParam = - m_vViewPoint ;
	m_rvmatParam.RevolveVector( m_vmoveParam ) ;
	//
	E3DModelJoint::RefreshJoint( ) ;
}

// 注視点設定
//////////////////////////////////////////////////////////////////////////////
void E3DViewPointJoint::SetTarget( const E3DDF_VECTOR & vTarget )
{
	m_vViewTarget = vTarget ;
	m_vViewAngle = vTarget ;
	m_vViewAngle -= m_vViewPoint ;
}

// 視線ベクトルを設定
//////////////////////////////////////////////////////////////////////////////
void E3DViewPointJoint::SetViewAngle( const E3DDF_VECTOR & vViewAngle )
{
	m_vViewAngle = vViewAngle ;
}

// 視点を設定
//////////////////////////////////////////////////////////////////////////////
void E3DViewPointJoint::SetViewPoint( const E3DDF_VECTOR & vViewPoint )
{
	m_vViewPoint = vViewPoint ;
}

// ｚ軸回転角度を設定 [deg]
//////////////////////////////////////////////////////////////////////////////
void E3DViewPointJoint::SetRevolveZ( double rDegAngle )
{
	m_rRevAngleZ = rDegAngle ;
}



//////////////////////////////////////////////////////////////////////////////
// カメラ補完曲線
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DViewAngleCurve, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DViewAngleCurve::E3DViewAngleCurve( void )
{
	m_vTarget[0].x = 0 ;
	m_vTarget[0].y = 0 ;
	m_vTarget[0].z = 1 ;
	m_vViewPoint[0].x = 0 ;
	m_vViewPoint[0].y = 0 ;
	m_vViewPoint[0].z = 0 ;
	m_vTarget[1] = m_vTarget[0] ;
	m_vViewPoint[1] = m_vViewPoint[0] ;
	m_rRevAngleZ[0] = m_rRevAngleZ[1] = 0.0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DViewAngleCurve::~E3DViewAngleCurve( void )
{
}

// カメラアングル設定
//////////////////////////////////////////////////////////////////////////////
void E3DViewAngleCurve::SetViewPoint
	( int nIndex, const E3D_VECTOR & vViewPoint,
		const E3D_VECTOR & vTarget, double rRevAngleZ )
{
	ESLAssert( (unsigned int) nIndex < 2 ) ;
	m_vViewPoint[nIndex] = vViewPoint ;
	m_vTarget[nIndex] = vTarget ;
	m_rRevAngleZ[nIndex] = rRevAngleZ ;
}

// カメラアングル取得
//////////////////////////////////////////////////////////////////////////////
void E3DViewAngleCurve::GetViewPoint
	( double t, E3D_VECTOR & vViewPoint,
		E3D_VECTOR & vViewAngle, double & rRevAngleZ, DWORD dwFlags )
{
	//
	// 始点・終点の x-z 平面上の視線ベクトルの角度を計算する
	//
	const double	rPI = 3.1415926535898 ;
	double			rRevY[2] ;
	double			rXZ[2] ;
	E3D_VECTOR		vAngle[2] ;
	for ( int i = 0; i < 2; i ++ )
	{
		vAngle[i] = m_vTarget[i] - m_vViewPoint[i] ;
		rRevY[i] = atan2( vAngle[i].x, vAngle[i].z ) ;
		rXZ[i] = sqrt( vAngle[i].x * vAngle[i].x + vAngle[i].z * vAngle[i].z ) ;
	}
	//
	// 回転方向を決定する
	//
	double	rRevDelta = rRevY[1] - rRevY[0] ;
	if ( rRevDelta < - rPI )
	{
		rRevDelta += rPI * 2 ;
	}
	else if ( rRevDelta > rPI )
	{
		rRevDelta -= rPI * 2 ;
	}
	//
	// 補完処理
	//
	E3DVector	vTarget =
		m_vTarget[0] + (m_vTarget[1] - m_vTarget[0]) * (REAL32) t ;
	if ( (rXZ[0] + rXZ[1] > 10e-5) || (vAngle[0].y * vAngle[1].y >= 0) )
	{
		// x-z 面回転補完
		double	rXZt = rXZ[0] + (rXZ[1] - rXZ[0]) * t ;
		double	rRevYt = rRevY[0] + rRevDelta * t ;
		vViewPoint.x = (REAL32) (vTarget.x - rXZt * sin( rRevYt )) ;
		vViewPoint.y = (REAL32)
			(m_vViewPoint[0].y + (m_vViewPoint[1].y - m_vViewPoint[0].y) * t) ;
		vViewPoint.z = (REAL32) (vTarget.z - rXZt * cos( rRevYt )) ;
		vViewAngle = vTarget - vViewPoint ;
	}
	else
	{
		// 特殊処理 : y-z 面回転補完
		double	rRevXt = rPI * t ;
		vViewAngle.x = 0 ;
		vViewAngle.y = (REAL32) (vAngle[0].y * cos( rRevXt )) ;
		vViewAngle.z = (REAL32) (- vAngle[0].y * sin( rRevXt )) ;
		//
		if ( !(dwFlags & ctOnlyAngle) )
		{
			double	rYZt =
				fabs( vAngle[0].y ) * (1.0 - t) + fabs( vAngle[1].y ) * t ;
			vViewAngle.Normalize( ) ;
			vViewPoint = vTarget - vViewAngle * (REAL32) rYZt ;
		}
	}
	if ( dwFlags & ctOnlyAngle )
	{
		vViewPoint =
			m_vViewPoint[0]
				+ (m_vViewPoint[1] - m_vViewPoint[0]) * (REAL32) t ;
	}
	rRevAngleZ = m_rRevAngleZ[0] + (m_rRevAngleZ[1] - m_rRevAngleZ[0]) * t ;
}

// カメラアングル設定
//////////////////////////////////////////////////////////////////////////////
void E3DViewAngleCurve::SetViewCurveFor
	( E3DViewPointJoint & vpjoint, double t, DWORD dwFlags )
{
	E3D_VECTOR	vViewPoint, vViewAngle ;
	double		rRevAngleZ ;
	GetViewPoint( t, vViewPoint, vViewAngle, rRevAngleZ, dwFlags ) ;
	vpjoint.SetViewPoint( vViewPoint ) ;
	vpjoint.SetViewAngle( vViewAngle ) ;
	vpjoint.SetRevolveZ( rRevAngleZ ) ;
	vpjoint.RefreshJoint( ) ;
}

