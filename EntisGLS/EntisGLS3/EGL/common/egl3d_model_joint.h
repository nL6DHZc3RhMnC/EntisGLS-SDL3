
//////////////////////////////////////////////////////////////////////////////
// モデルジョイント
//////////////////////////////////////////////////////////////////////////////

class	E3DModelJoint	: public	ESLObject
{
public:
	E3DModelJoint *	m_parent ;		// 親ジョイント

	E3DDFRevMatrix	m_dfrvmat ;		// 回転行列（階層化されたパラメータ）
	E3DRevMatrix	m_rvmat ;		// 回転行列（階層化されたパラメータ）
	E3DDFVector		m_dfvmove ;		// 平行移動
	E3DVector		m_vmove ;		// 平行移動

	EPtrObjArray<E3DPolygonModel>
					m_models ;		// モデルデータへの参照リスト
	EObjArray<E3DModelJoint>
					m_joints ;		// サブジョイント

protected:
	E3DDFRevMatrix	m_rvmatParam ;	// 回転行列（階層化されていないパラメータ）
	E3DDFVector		m_vmoveParam ;	// 平行移動

public:
	// 構築関数
	E3DModelJoint( void ) ;
	E3DModelJoint( E3DPolygonModel * pmodel ) ;
	// 消滅関数
	virtual ~E3DModelJoint( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DModelJoint, ESLObject )

public:
	// ジョイント生成
	virtual E3DModelJoint * CreateJoint( void ) const ;
	// パラメータ反映
	virtual void RefreshJoint( void ) ;
	// ジョイント回転処理
	virtual void TransformJoint( const E3DModelJoint & mjParent ) ;
	// モデル回転処理
	virtual void TransformModel( E3DPolygonModel & model ) const ;
	// 座標回転処理
	void RevolveVector( E3DDF_VECTOR & vPos ) const
		{
			m_dfrvmat.RevolveVector( vPos ) ;
		}
	void RevolveVector( E3D_VECTOR & vPos ) const
		{
			m_rvmat.RevolveVector( vPos ) ;
		}
	void TransformPosition( E3DDF_VECTOR & vPos ) const
		{
			m_dfrvmat.RevolveVector( vPos ) ;
			vPos += m_dfvmove ;
		}
	void TransformPosition( E3D_VECTOR & vPos ) const
		{
			m_rvmat.RevolveVector( vPos ) ;
			vPos += m_vmove ;
		}
	// 全てのジョイントを削除
	void DeleteContents( void ) ;
	// ジョイント複製
	void CopyModelJoint( const E3DModelJoint & model ) ;

public:
	// 基準座標
	E3DDFVector & Position( void )
		{
			return	m_vmoveParam ;
		}
	// 変換行列
	E3DDFRevMatrix & Matrix( void )
		{
			return	m_rvmatParam ;
		}
	// 変換行列を初期化
	void InitializeMatrix( void )
		{
			m_rvmatParam.InitializeMatrix( E3DDFVector( 1, 1, 1 ) ) ;
		}
	// ｘ軸回転
	void RevolveOnX( double rDeg ) ;
	// ｙ軸回転
	void RevolveOnY( double rDeg ) ;
	// ｚ軸回転
	void RevolveOnZ( double rDeg ) ;
	// ベクトルの方向に向く
	void RevolveByAngleOn( const E3DDF_VECTOR & angle )
		{
			m_rvmatParam.RevolveByAngleOn( angle ) ;
		}
	void RevolveByAngleOn( const E3D_VECTOR & angle )
		{
			m_rvmatParam.RevolveByAngleOn( E3DDFVector( angle ) ) ;
		}
	void RevolveForAngle( const E3DDF_VECTOR & angle )
		{
			m_rvmatParam.RevolveForAngle( angle ) ;
		}
	void RevolveForAngle( const E3D_VECTOR & angle )
		{
			m_rvmatParam.RevolveForAngle( E3DDFVector( angle ) ) ;
		}
	// 拡大
	void MagnifyByVector( const E3DDF_VECTOR & vector )
		{
			m_rvmatParam.MagnifyByVector( vector ) ;
		}
	void MagnifyByVector( const E3D_VECTOR & vector )
		{
			m_rvmatParam.MagnifyByVector( E3DDFVector( vector ) ) ;
		}

public:
	// モデルデータ追加
	void AddModelRef( E3DPolygonModel * pmodel )
		{
			m_models.Add( pmodel ) ;
		}
	// モデルデータ削除
	void RemoveModelRef( E3DPolygonModel * pmodel )
		{
			int	nIndex = m_models.FindPtr( pmodel ) ;
			if ( nIndex >= 0 )
			{
				m_models.RemoveAt( nIndex ) ;
			}
		}
	// モデルデータ配列を参照
	EPtrObjArray<E3DPolygonModel> & ModelList( void )
		{
			return	m_models ;
		}
	// ジョイント追加
	void AddSubJoint( E3DModelJoint * pJoint )
		{
			pJoint->m_parent = this ;
			m_joints.Add( pJoint ) ;
		}
	// ジョイント削除
	void RemoveSubJoint( E3DModelJoint * pJoint )
		{
			int	nIndex = m_joints.FindPtr( pJoint ) ;
			if ( nIndex >= 0 )
			{
				m_joints.RemoveAt( nIndex ) ;
			}
		}
	// ジョイント配列を参照
	const EObjArray<E3DModelJoint> & JointList( void ) const
		{
			return	m_joints ;
		}
	EObjArray<E3DModelJoint> & JointList( void )
		{
			return	m_joints ;
		}

} ;



//////////////////////////////////////////////////////////////////////////////
// カメラオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	E3DViewPointJoint	: public	E3DModelJoint
{
public:
	// 構築関数
	E3DViewPointJoint( void ) ;
	// 消滅関数
	virtual ~E3DViewPointJoint( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DViewPointJoint, E3DModelJoint )

protected:
	E3DDFVector		m_vViewAngle ;		// 視線ベクトル
	E3DDFVector		m_vViewPoint ;		// 視点
	E3DDFVector		m_vViewTarget ;		// 注視点
	double			m_rRevAngleZ ;		// ｚ軸回転

public:
	// パラメータ反映
	virtual void RefreshJoint( void ) ;

public:
	// 視線ベクトルを取得
	const E3DDFVector & GetViewAngle( void ) const
		{
			return	m_vViewAngle ;
		}
	// 視点を取得
	const E3DDFVector & GetViewPoint( void ) const
		{
			return	m_vViewPoint ;
		}
	// 注視点を取得
	const E3DDFVector & GetViewTarget( void ) const
		{
			return	m_vViewTarget ;
		}
	// z 軸回転角度を取得
	double GetRevolveZ( void ) const
		{
			return	m_rRevAngleZ ;
		}
	// 注視点設定
	void SetTarget( const E3DDF_VECTOR & vTarget ) ;
	void SetTarget( const E3D_VECTOR & vTarget )
		{
			SetTarget( E3DDFVector( vTarget ) ) ;
		}
	// 視線ベクトルを設定
	void SetViewAngle( const E3DDF_VECTOR & vViewAngle ) ;
	void SetViewAngle( const E3D_VECTOR & vViewAngle )
		{
			SetViewAngle( E3DDFVector( vViewAngle ) ) ;
		}
	// 視点を設定
	void SetViewPoint( const E3DDF_VECTOR & vViewPoint ) ;
	void SetViewPoint( const E3D_VECTOR & vViewPoint )
		{
			SetViewPoint( E3DDFVector( vViewPoint ) ) ;
		}
	// ｚ軸回転角度を設定 [deg]
	void SetRevolveZ( double rDegAngle ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// カメラ補完曲線
//////////////////////////////////////////////////////////////////////////////

class	E3DViewAngleCurve	: public	ESLObject
{
public:
	// 構築関数
	E3DViewAngleCurve( void ) ;
	// 消滅関数
	virtual ~E3DViewAngleCurve( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DViewAngleCurve, ESLObject )

public:
	// 補完フラグ
	enum	CurveType
	{
		ctOnlyAngle		= 0x0001
	} ;

protected:
	E3DVector	m_vTarget[2] ;
	E3DVector	m_vViewPoint[2] ;
	double		m_rRevAngleZ[2] ;

public:
	// カメラアングル設定
	void SetViewPoint
		( int nIndex, const E3D_VECTOR & vViewPoint,
			const E3D_VECTOR & vTarget, double rRevAngleZ = 0.0 ) ;
	// カメラアングル取得
	void GetViewPoint
		( double t, E3D_VECTOR & vViewPoint,
			E3D_VECTOR & vViewAngle, double & rRevAngleZ, DWORD dwFlags = 0 ) ;
	// カメラアングル設定
	void SetViewCurveFor
		( E3DViewPointJoint & vpjoint, double t, DWORD dwFlags = 0 ) ;

} ;

