
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (C) 2004-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <gls.h>
#include <math.h>


//////////////////////////////////////////////////////////////////////////////
// モデルジョイント・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSModelJoint::m_staFuncName = NULL ;
const wchar_t *		ECSModelJoint::m_pwszFuncName[31] =
{
	L"Position", L"InitializeMatrix",
	L"RevolveOnX", L"RevolveOnY", L"RevolveOnZ", L"RevolveByAngleOn",
	L"RotateOnX", L"RotateOnY", L"RotateOnZ", L"RotateByAngleOn",
	L"MagnifyByVector", L"GetQuaternion", L"SetQuaternion",
	L"AddModelRef", L"ClearModelRef",
	L"AddSubJoint", L"CreateSubJoint", L"GetLength",
	L"RemoveSubJoint", L"RemoveAllSubJoint",
	L"GetColorAttribute", L"GetTransparency",
	L"SetColorAttribute", L"SetTransparency",
	L"SetColorMorphing", L"SetBezierCurve", L"SetRotationBezier",
	L"BeginActivation", L"FlushActivation",
	L"IsActivation", NULL,
} ;
const ECSModelJoint::PFUNC_CALL	ECSModelJoint::m_pfnCallFunc[30] =
{
	&ECSModelJoint::Call_Position,
	&ECSModelJoint::Call_InitializeMatrix,
	&ECSModelJoint::Call_RevolveOnX,
	&ECSModelJoint::Call_RevolveOnY,
	&ECSModelJoint::Call_RevolveOnZ,
	&ECSModelJoint::Call_RevolveByAngleOn,
	&ECSModelJoint::Call_RevolveOnX,
	&ECSModelJoint::Call_RevolveOnY,
	&ECSModelJoint::Call_RevolveOnZ,
	&ECSModelJoint::Call_RevolveByAngleOn,
	&ECSModelJoint::Call_MagnifyByVector,
	&ECSModelJoint::Call_GetQuaternion,
	&ECSModelJoint::Call_SetQuaternion,
	&ECSModelJoint::Call_AddModelRef,
	&ECSModelJoint::Call_ClearModelRef,
	&ECSModelJoint::Call_AddSubJoint,
	&ECSModelJoint::Call_CreateSubJoint,
	&ECSModelJoint::Call_GetLength,
	&ECSModelJoint::Call_RemoveSubJoint,
	&ECSModelJoint::Call_RemoveAllSubJoint,
	&ECSModelJoint::Call_GetColorAttribute,
	&ECSModelJoint::Call_GetTransparency,
	&ECSModelJoint::Call_SetColorAttribute,
	&ECSModelJoint::Call_SetTransparency,
	&ECSModelJoint::Call_SetColorMorphing,
	&ECSModelJoint::Call_SetBezierCurve,
	&ECSModelJoint::Call_SetRotationBezier,
	&ECSModelJoint::Call_BeginActivation,
	&ECSModelJoint::Call_FlushActivation,
	&ECSModelJoint::Call_IsActivation,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSModelJoint, ECSObject, E3DModelJoint )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSModelJoint::ECSModelJoint( void )
{
	m_pRefJoint = NULL ;
	m_strcPosition = NULL ;
	//
	ECSContext *	pContext = ECotophaScript::GetPrimaryContext() ;
	if ( pContext != NULL )
	{
		m_strcPosition =
			pContext->CreateUserStructureObject( L"Vector" ) ;
		if ( m_strcPosition != NULL )
		{
			m_pcsrPosition[0] = ESLTypeCast<ECSReal>( m_strcPosition->GetMemberAs( L"x" ) ) ;
			m_pcsrPosition[1] = ESLTypeCast<ECSReal>( m_strcPosition->GetMemberAs( L"y" ) ) ;
			m_pcsrPosition[2] = ESLTypeCast<ECSReal>( m_strcPosition->GetMemberAs( L"z" ) ) ;
		}
	}
	if ( m_strcPosition == NULL )
	{
		static const wchar_t *	pwszVector[3] = { L"x", L"y", L"z" } ;
		int		i ;
		m_strcPosition = new ECSStructure ;
		m_strcPosition->m_pwszTag = L"Vector" ;
		for ( i = 0; i < 3; i ++ )
		{
			m_pcsrPosition[i] = new ECSReal ;
			m_strcPosition->AddNewVariable( pwszVector[i], m_pcsrPosition[i] ) ;
		}
	}
	//
	m_clrModelColor.rgbMul.dwPixelCode = 0xFFFFFF ;
	m_clrModelColor.rgbAdd.dwPixelCode = 0 ;
	m_nTransparency = 0 ;
	m_dwDurationTime = 0 ;
	m_dwAnimationTime = 0 ;
	m_fEnableFading = false ;
	m_fEnableMoving = false ;
	m_fEnableRotation = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSModelJoint::~ECSModelJoint( void )
{
	delete	m_strcPosition ;
}

// ジョイント生成
//////////////////////////////////////////////////////////////////////////////
E3DModelJoint * ECSModelJoint::CreateJoint( void ) const
{
	return	new ECSModelJoint ;
}

// パラメータ反映
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::RefreshJoint( void )
{
	m_vmoveParam.x = (REAL32) m_pcsrPosition[0]->m_varReal ;
	m_vmoveParam.y = (REAL32) m_pcsrPosition[1]->m_varReal ;
	m_vmoveParam.z = (REAL32) m_pcsrPosition[2]->m_varReal ;
	E3DModelJoint::RefreshJoint( ) ;
}

void ECSModelJoint::RefreshJointPosition( void )
{
	m_pcsrPosition[0]->m_varReal = m_vmoveParam.x ;
	m_pcsrPosition[1]->m_varReal = m_vmoveParam.y ;
	m_pcsrPosition[2]->m_varReal = m_vmoveParam.z ;
}

// ジョイント複製
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::CopyModelJoint( const E3DModelJoint & model )
{
	if ( model.IsKindOf( ESL_RUNTIME_CLASS(ECSModelJoint) ) )
	{
		ECSModelJoint *	pJoint = (ECSModelJoint*) &model ;
		for ( int i = 0; i < 3; i ++ )
		{
			m_pcsrPosition[i]->m_varReal =
				pJoint->m_pcsrPosition[i]->m_varReal ;
		}
	}
	E3DModelJoint::CopyModelJoint( model ) ;
}

// ジョイントアニメーション
//////////////////////////////////////////////////////////////////////////////
bool ECSModelJoint::OnAdvanceAnimation( unsigned int nTime )
{
	//
	// このジョイントのアニメーション
	//
	bool	fAnimate = false ;
	if ( (m_dwDurationTime != 0) && (m_fEnableFading || m_fEnableMoving) )
	{
		m_dwAnimationTime += nTime ;
		//
		bool	fFinishAnimation = false ;
		SDWORD	dwOffsetTime = m_dwAnimationTime ;
		if ( dwOffsetTime > m_dwDurationTime )
		{
			if ( m_nActionType == actLoop )
			{
				dwOffsetTime %= m_dwDurationTime ;
				m_dwAnimationTime = dwOffsetTime ;
			}
			else if ( m_nActionType == actTurnLoop )
			{
				dwOffsetTime %= (m_dwDurationTime * 2) ;
				m_dwAnimationTime = dwOffsetTime ;
				//
				if ( dwOffsetTime > m_dwDurationTime )
				{
					dwOffsetTime = m_dwDurationTime * 2 - dwOffsetTime ;
				}
			}
			else
			{
				dwOffsetTime = m_dwDurationTime ;
				fFinishAnimation = true ;
			}
		}
		if ( m_fEnableFading )
		{
			//
			// 色フェード
			//
			E3D_COLOR		clrCurrent ;
			unsigned int	nTransparency ;
			unsigned int	nDegree = 0x100 * dwOffsetTime / m_dwDurationTime ;
			//
			clrCurrent.rgbMul =
				EGL_PALETTE( EGLPalette( m_clrEndColor.rgbMul ) * nDegree
					+ EGLPalette( m_clrStartColor.rgbMul ) * (0x100 - nDegree ) ) ;
			clrCurrent.rgbAdd =
				EGL_PALETTE( EGLPalette( m_clrEndColor.rgbAdd ) * nDegree
					+ EGLPalette( m_clrStartColor.rgbAdd ) * (0x100 - nDegree) ) ;
			nTransparency =
				(m_nEndTransparency * nDegree
					+ m_nStartTransparency * (0x100 - nDegree)) / 0x100 ;
			//
			if ( (nTransparency != m_nTransparency)
				|| (m_clrModelColor.rgbMul.dwPixelCode
								!= clrCurrent.rgbMul.dwPixelCode)
				|| (m_clrModelColor.rgbAdd.dwPixelCode
								!= clrCurrent.rgbAdd.dwPixelCode) )
			{
				m_clrModelColor = clrCurrent ;
				m_nTransparency = nTransparency ;
				fAnimate = true ;
			}
		}
		bool	fInitMat = false ;
		if ( m_fEnableRotation )
		{
			double	t = (double) dwOffsetTime / m_dwDurationTime ;
			//
			E3DDFQuaternion	q = m_bzRotation.pt( t ) ;
			q.Normalize() ;
			q.ToMatrix( Matrix() ) ;
			//
			fInitMat = true ;
		}
		if ( m_fEnableMoving )
		{
			//
			// 移動・拡大・回転
			//
			double	t = (double) dwOffsetTime / m_dwDurationTime ;
			//
			E3DVector	vPos( 0, 0, 0 ) ;
			E3DVector	vRev( 0, 0, 0 ) ;
			E3DVector	vMag( 0, 0, 0 ) ;
			if ( m_bzPosition.GetCount() > 0 )
			{
				m_bzPosition.pt( vPos, t ) ;
				//
				m_pcsrPosition[0]->m_varReal = vPos.x ;
				m_pcsrPosition[1]->m_varReal = vPos.y ;
				m_pcsrPosition[2]->m_varReal = vPos.z ;
				m_vmoveParam = vPos ;
			}
			if ( m_bzRevolution.GetCount() > 0 )
			{
				if ( !fInitMat )
				{
					InitializeMatrix( ) ;
					fInitMat = true ;
				}
				m_bzRevolution.pt( vRev, t ) ;
				RevolveOnZ( vRev.z ) ;
				RevolveOnY( vRev.y ) ;
				RevolveOnX( vRev.x ) ;
			}
			if ( m_bzMagnification.GetCount() > 0 )
			{
				if ( !fInitMat )
				{
					InitializeMatrix( ) ;
					fInitMat = true ;
				}
				m_bzMagnification.pt( vMag, t ) ;
				MagnifyByVector( vMag ) ;
			}
			fAnimate = true ;
		}
		if ( fFinishAnimation )
		{
			m_fEnableFading = false ;
			m_fEnableMoving = false ;
			m_fEnableRotation = false ;
			m_dwDurationTime = 0 ;
		}
	}
	//
	// サブジョイントのアニメーション
	//
	for ( unsigned int i = 0; i < m_joints.GetSize(); i ++ )
	{
		ECSModelJoint *	pJoint =
			ESLTypeCast<ECSModelJoint>( m_joints.GetAt( i ) ) ;
		if ( pJoint != NULL )
		{
			if ( pJoint->OnAdvanceAnimation( nTime ) )
			{
				fAnimate = true ;
			}
		}
	}
	return	fAnimate ;
}

// モデルをレンダリングバッファに追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::AddModelToRender( E3DRenderPolygon & render )
{
	ESLError	err = eslErrSuccess ;
	E3D_COLOR *	pColor = NULL ;
	if ( ((m_clrModelColor.rgbMul.dwPixelCode & 0xFFFFFF) != 0xFFFFFF)
		|| ((m_clrModelColor.rgbAdd.dwPixelCode & 0xFFFFFF) != 0) )
	{
		pColor = &m_clrModelColor ;
	}
	if ( render.AddModel( *this, pColor, m_nTransparency, false ) )
	{
		err = eslErrGeneral ;
	}
	for ( unsigned int i = 0; i < m_joints.GetSize(); i ++ )
	{
		E3DModelJoint *	pSubJoint = m_joints.GetAt( i ) ;
		ECSModelJoint *	pJoint = ESLTypeCast<ECSModelJoint>( pSubJoint ) ;
		if ( pJoint != NULL )
		{
			if ( pJoint->AddModelToRender( render ) )
			{
				err = eslErrGeneral ;
			}
		}
		else if ( pSubJoint != NULL )
		{
			if ( render.AddModel( *pSubJoint, pColor, m_nTransparency, false ) )
			{
				err = eslErrGeneral ;
			}
		}
	}
	return	err ;
}

// 適用色設定
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::SetColorAttribute( const E3D_COLOR * pColor )
{
	if ( pColor != NULL )
	{
		m_clrModelColor = *pColor ;
	}
	else
	{
		m_clrModelColor.rgbMul.dwPixelCode = 0x00FFFFFF ;
		m_clrModelColor.rgbAdd.dwPixelCode = 0 ;
	}
}

// 透明度設定
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::SetTransparency( unsigned int nTransparency )
{
	m_nTransparency = nTransparency ;
}

// 適用色フェード設定
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::SetColorMorphing
	( const E3D_COLOR * pColor, unsigned int nTransparency )
{
	m_clrStartColor = m_clrModelColor ;
	if ( pColor != NULL )
	{
		m_clrEndColor = *pColor ;
	}
	else
	{
		m_clrEndColor = m_clrModelColor ;
	}
	//
	m_nStartTransparency = m_nTransparency ;
	m_nEndTransparency = nTransparency ;
	//
	m_fEnableFading = true ;
}

// 移動アニメーション設定
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::SetBezierCurve
	( const EBezierCurves<E3D_VECTOR> * pbzCurve,
		const EBezierCurves<E3D_VECTOR> * pbzRevolution,
		const EBezierCurves<E3D_VECTOR> * pbzMagnification )
{
	if ( pbzCurve != NULL )
	{
		m_bzPosition = *pbzCurve ;
	}
	else
	{
		m_bzPosition.SetCount( 0 ) ;
		/*
		m_bzPosition.SetCount( 4 ) ;
		m_bzPosition[0].x = (REAL32) m_pcsrPosition[0]->m_varReal ;
		m_bzPosition[0].y = (REAL32) m_pcsrPosition[1]->m_varReal ;
		m_bzPosition[0].z = (REAL32) m_pcsrPosition[2]->m_varReal ;
		m_bzPosition[3] = m_bzPosition[2]
			= m_bzPosition[1] = m_bzPosition[0] ;
		*/
	}
	if ( pbzRevolution != NULL )
	{
		m_bzRevolution = *pbzRevolution ;
	}
	else
	{
		m_bzRevolution.SetCount( 0 ) ;
		/*
		m_bzRevolution.SetCount( 4 ) ;
		m_bzRevolution[0].x = 0 ;
		m_bzRevolution[0].y = 0 ;
		m_bzRevolution[0].z = 0 ;
		m_bzRevolution[3] = m_bzRevolution[2]
			= m_bzRevolution[1] = m_bzRevolution[0] ;
		*/
	}
	if ( pbzMagnification != NULL )
	{
		m_bzMagnification = *pbzMagnification ;
	}
	else
	{
		m_bzMagnification.SetCount( 0 ) ;
		/*
		m_bzMagnification.SetCount( 4 ) ;
		m_bzMagnification[0].x = 1 ;
		m_bzMagnification[0].y = 1 ;
		m_bzMagnification[0].z = 1 ;
		m_bzMagnification[3] = m_bzMagnification[2]
			= m_bzMagnification[1] = m_bzMagnification[0] ;
		*/
	}
	m_fEnableMoving = true ;
}

// 回転アニメーション設定
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::SetRotationBezier
	( const EBezierCurves<E3D_QUATERNION> & bzRotaion )
{
	m_bzRotation = bzRotaion ;
	m_fEnableRotation = true ;
}

// アニメーション開始
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::BeginActivation( unsigned int nDuration, int nActionType )
{
	m_nActionType = nActionType ;
	m_dwDurationTime = nDuration ;
	m_dwAnimationTime = 0 ;
	//
	if ( m_dwDurationTime == 0 )
	{
		FlushActivation( ) ;
		return ;
	}
	bool	fInitMat = false ;
	if ( m_fEnableRotation )
	{
		E3DDFQuaternion(m_bzRotation[0]).ToMatrix( Matrix() ) ;
		fInitMat = true ;
	}
	if ( m_fEnableMoving )
	{
		m_pcsrPosition[0]->m_varReal = m_bzPosition[0].x ;
		m_pcsrPosition[1]->m_varReal = m_bzPosition[0].y ;
		m_pcsrPosition[2]->m_varReal = m_bzPosition[0].z ;
		m_vmoveParam = m_bzPosition[0] ;
		//
		if ( (m_bzRevolution.GetCount() > 0)
			|| (m_bzMagnification.GetCount() > 0) )
		{
			if ( !fInitMat )
			{
				InitializeMatrix( ) ;
			}
			if ( m_bzRevolution.GetCount() > 0 )
			{
				RevolveOnZ( m_bzRevolution[0].z ) ;
				RevolveOnY( m_bzRevolution[0].y ) ;
				RevolveOnX( m_bzRevolution[0].x ) ;
			}
			if ( m_bzMagnification.GetCount() > 0 )
			{
				MagnifyByVector( m_bzMagnification[0] ) ;
			}
		}
	}
}

// アニメーション終了
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::FlushActivation( void )
{
	if ( m_dwDurationTime > 0 )
	{
		if ( m_fEnableFading )
		{
			SetColorAttribute( &m_clrEndColor ) ;
			SetTransparency( m_nEndTransparency ) ;
			m_fEnableFading = false ;
		}
		bool	fInitMat = false ;
		if ( m_fEnableRotation )
		{
			int	nCount = m_bzRotation.GetCount() - 1 ;
			if ( nCount >= 0 )
			{
				E3DDFQuaternion(m_bzRotation[nCount]).ToMatrix( Matrix() ) ;
				fInitMat = true ;
			}
			m_fEnableRotation = false ;
		}
		if ( m_fEnableMoving )
		{
			int	nCount = m_bzPosition.GetCount() - 1 ;
			if ( nCount >= 0 )
			{
				m_pcsrPosition[0]->m_varReal = m_bzPosition[nCount].x ;
				m_pcsrPosition[1]->m_varReal = m_bzPosition[nCount].y ;
				m_pcsrPosition[2]->m_varReal = m_bzPosition[nCount].z ;
				m_vmoveParam = m_bzPosition[nCount] ;
			}
			nCount = m_bzRevolution.GetCount() - 1 ;
			if ( nCount >= 0 )
			{
				if ( !fInitMat )
				{
					InitializeMatrix( ) ;
					fInitMat = true ;
				}
				RevolveOnZ( m_bzRevolution[nCount].z ) ;
				RevolveOnY( m_bzRevolution[nCount].y ) ;
				RevolveOnX( m_bzRevolution[nCount].x ) ;
			}
			nCount = m_bzMagnification.GetCount() - 1 ;
			if ( nCount >= 0 )
			{
				if ( !fInitMat )
				{
					InitializeMatrix( ) ;
					fInitMat = true ;
				}
				MagnifyByVector( m_bzMagnification[nCount] ) ;
			}
			m_fEnableMoving = false ;
		}
		m_dwDurationTime = 0 ;
		m_nActionType = actNormal ;
	}
	for ( unsigned int i = 0; i < m_joints.GetSize(); i ++ )
	{
		ECSModelJoint *	pJoint =
			ESLTypeCast<ECSModelJoint>( m_joints.GetAt( i ) ) ;
		if ( pJoint != NULL )
		{
			pJoint->FlushActivation( ) ;
		}
	}
}

// アニメーション中か？
//////////////////////////////////////////////////////////////////////////////
bool ECSModelJoint::IsActivation( void ) const
{
	if ( (m_dwDurationTime != 0)
		&& (m_fEnableFading || m_fEnableMoving || m_fEnableRotation) )
	{
		return	true ;
	}
	for ( unsigned int i = 0; i < m_joints.GetSize(); i ++ )
	{
		ECSModelJoint *	pJoint =
			ESLTypeCast<ECSModelJoint>( m_joints.GetAt( i ) ) ;
		if ( pJoint != NULL )
		{
			if ( pJoint->IsActivation() )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSModelJoint::GetTypeName( void ) const
{
	return	L"ModelJoint" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSModelJoint::Duplicate( void )
{
	ECSModelJoint *	pJoint = new ECSModelJoint ;
	pJoint->CopyModelJoint( *this ) ;
	return	pJoint ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Move( ECSContext & context, ECSObject * obj )
{
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>( ECSObject::GetEntity( obj ) ) ;
	if ( pJoint == NULL )
	{
		return	ESLErrorMsg( "ModelJoint への不正な代入操作です。" ) ;
	}
	if ( m_pRefJoint != NULL )
	{
		m_pRefJoint->Position() = pJoint->Position() ;
		m_pRefJoint->Matrix() = pJoint->Matrix() ;
	}
	else
	{
		CopyModelJoint( *pJoint ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "ModelJoint の定義されていない演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "ModelJoint の定義されていない演算子です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "ModelJoint の定義されていない比較です。" ) ;
}

// メンバ変数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::GetVariableIndex( int & nIndex, int iElement )
{
	nIndex = iElement ;
	if ( JointList().GetAt( nIndex ) == NULL )
	{
		return	ESLErrorMsg
			( "ModelJoint のサブジョイントの指標が範囲外です。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSModelJoint::GetVariableAt( int nIndex )
{
	if ( nIndex >= 0 )
	{
		return	ESLTypeCast<ECSModelJoint>( JointList().GetAt( nIndex ) ) ;
	}
	else
	{
		switch ( nIndex )
		{
		case	-1:
			return	m_strcPosition ;
		}
	}
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSModelJoint::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "ModelJoint の定義されていない"
				"メンバ関数を呼び出そうとしました。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::CallFunction
	( ECSContext & context, int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg
			( "ModelJoint の定義されていない"
				"メンバ関数を呼び出しています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSModelJoint::IndexAllMember( void )
{
	m_strcPosition->m_pParent = this ;
	m_strcPosition->m_nIndex = -1 ;
	m_strcPosition->IndexAllMember( ) ;
	//
	unsigned int	i ;
	m_lstRefModel.RemoveAll( ) ;
	for ( i = 0; i < ModelList().GetSize(); i ++ )
	{
		ECSObject *	pModel =
			ESLTypeCast<ECSObject>( ModelList().GetAt( i ) ) ;
		if ( pModel != NULL )
		{
			m_lstRefModel.Add( new ECSReference( pModel ) ) ;
		}
	}
	for ( i = 0; i < JointList().GetSize(); i ++ )
	{
		ECSObject *	pJoint =
			ESLTypeCast<ECSObject>( JointList().GetAt( i ) ) ;
		if ( pJoint != NULL )
		{
			pJoint->m_pParent = this ;
			pJoint->m_nIndex = i ;
			pJoint->IndexAllMember( ) ;
		}
	}
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::CommitAllReference( ECSContext & context )
{
	m_strcPosition->CommitAllReference( context ) ;
	//
	unsigned int	i, nCount ;
	ModelList().RemoveAll( ) ;
	for ( i = 0; i < m_lstRefModel.GetSize(); i ++ )
	{
		ESLAssert( m_lstRefModel.GetAt(i) != NULL ) ;
		ECSReference *	pRef = m_lstRefModel.GetAt( i ) ;
		if ( pRef == NULL )
		{
			continue ;
		}
		pRef->CommitAllReference( context ) ;
		//
		E3DPolygonModel *	pModel =
			ESLTypeCast<E3DPolygonModel>( pRef->m_pRef ) ;
		if ( pModel != NULL )
		{
			AddModelRef( pModel ) ;
		}
	}
	m_lstRefModel.RemoveAll( ) ;
	//
	nCount = JointList().GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSModelJoint *	pJoint =
			ESLTypeCast<ECSModelJoint>( JointList().GetAt(i) ) ;
		if ( pJoint != NULL )
		{
			pJoint->CommitAllReference( context ) ;
		}
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError				err ;
	MODEL_JOINT_SAVE_DATA	mjsd ;
	DWORD					dwVersion = 1 ;
	DWORD					dwBytes, i, dwCount ;
	//
	file.Write( &dwVersion, sizeof(DWORD) ) ;
	//
	dwBytes = sizeof(MODEL_JOINT_SAVE_DATA) ;
	mjsd.vPos.x = (REAL32) m_pcsrPosition[0]->m_varReal ;
	mjsd.vPos.y = (REAL32) m_pcsrPosition[1]->m_varReal ;
	mjsd.vPos.z = (REAL32) m_pcsrPosition[2]->m_varReal ;
	mjsd.matrix = E3DRevMatrix( Matrix() ) ;
	//
	mjsd.clrModelColor = m_clrModelColor ;
	mjsd.nTransparency = m_nTransparency ;
	mjsd.nActionType = m_nActionType ;
	mjsd.dwOffsetTime = m_dwAnimationTime ;
	mjsd.dwDurationTime = m_dwDurationTime ;
	mjsd.fEnableFading = m_fEnableFading ;
	mjsd.fEnableMoving = m_fEnableMoving ;
	mjsd.clrStartColor = m_clrStartColor ;
	mjsd.clrEndColor = m_clrEndColor ;
	mjsd.nStartTransparency = m_nStartTransparency ;
	mjsd.nEndTransparency = m_nEndTransparency ;
	//
/*	for ( i = 0; i < 4; i ++ )
	{
		mjsd.bzPosition[i] = m_bzPosition[i] ;
		mjsd.bzRevolution[i] = m_bzRevolution[i] ;
		mjsd.bzMagnification[i] = m_bzMagnification[i] ;
	}
*/	//
	dwBytes = sizeof(mjsd) ;
	if ( file.Write( &dwBytes, sizeof(dwBytes) ) < sizeof(dwBytes) )
	{
		return	eslErrGeneral ;
	}
	if ( file.Write( &mjsd, dwBytes ) < dwBytes )
	{
		return	eslErrGeneral ;
	}
	//
	int	nBezierCount ;
	nBezierCount = m_bzPosition.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzPosition.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	nBezierCount = m_bzRevolution.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzRevolution.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	nBezierCount = m_bzMagnification.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzMagnification.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	nBezierCount = m_bzRotation.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzRotation.GetArrayPtr(),
				nBezierCount * sizeof(E3D_QUATERNION) ) ;
	//
	dwCount = m_lstRefModel.GetSize( ) ;
	file.Write( &dwCount, sizeof(DWORD) ) ;
	for ( i = 0; i < dwCount; i ++ )
	{
		err = m_lstRefModel.GetAt(i)->Save( file, context ) ;
		if ( err )
		{
			return	err ;
		}
	}
	m_lstRefModel.RemoveAll( ) ;
	//
	dwCount = JointList().GetSize( ) ;
	file.Write( &dwCount, sizeof(DWORD) ) ;
	for ( i = 0; i < dwCount; i ++ )
	{
		err = context.SaveObject
			( file, ESLTypeCast<ECSObject>( JointList().GetAt(i) ) ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Load( ESLFileObject & file, ECSContext & context )
{
	ESLError	err ;
	DWORD		dwVersion ;
	if ( file.Read( &dwVersion, sizeof(dwVersion) ) < sizeof(dwVersion) )
	{
		return	eslErrGeneral ;
	}
	if ( dwVersion < 1 )
	{
		return	eslErrGeneral ;
	}
	//
	MODEL_JOINT_SAVE_DATA	mjsd ;
	DWORD					dwBytes, i, dwCount ;
	::eslFillMemory( &mjsd, 0, sizeof(mjsd) ) ;
	//
	if ( file.Read( &dwBytes, sizeof(dwBytes) ) < sizeof(dwBytes) )
	{
		return	eslErrGeneral ;
	}
	if ( dwBytes > sizeof(mjsd) )
	{
		return	eslErrGeneral ;
	}
	if ( file.Read( &mjsd, dwBytes ) < dwBytes )
	{
		return	eslErrGeneral ;
	}
	//
	m_pcsrPosition[0]->m_varReal = mjsd.vPos.x ;
	m_pcsrPosition[1]->m_varReal = mjsd.vPos.y ;
	m_pcsrPosition[2]->m_varReal = mjsd.vPos.z ;
	Matrix() = mjsd.matrix ;
	//
	m_clrModelColor = mjsd.clrModelColor ;
	m_nTransparency = mjsd.nTransparency ;
	m_nActionType = mjsd.nActionType ;
	m_dwAnimationTime = mjsd.dwOffsetTime ;
	m_dwDurationTime = mjsd.dwDurationTime ;
	m_fEnableFading = (mjsd.fEnableFading != 0) ;
	m_fEnableMoving = (mjsd.fEnableMoving != 0) ;
	m_clrStartColor = mjsd.clrStartColor ;
	m_clrEndColor = mjsd.clrEndColor ;
	m_nStartTransparency = mjsd.nStartTransparency ;
	m_nEndTransparency = mjsd.nEndTransparency ;
	//
	int	nBezierCount ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzPosition.SetCount( nBezierCount ) ;
	file.Read( m_bzPosition.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzRevolution.SetCount( nBezierCount ) ;
	file.Read( m_bzRevolution.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzMagnification.SetCount( nBezierCount ) ;
	file.Read( m_bzMagnification.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzRotation.SetCount( nBezierCount ) ;
	file.Read( m_bzRotation.GetArrayPtr(),
				nBezierCount * sizeof(E3D_QUATERNION) ) ;
	//
/*	for ( i = 0; i < 4; i ++ )
	{
		m_bzPosition[i] = mjsd.bzPosition[i] ;
		m_bzRevolution[i] = mjsd.bzRevolution[i] ;
		m_bzMagnification[i] = mjsd.bzMagnification[i] ;
	}
*/	//
	if ( file.Read( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	m_lstRefModel.RemoveAll( ) ;
	for ( i = 0; i < dwCount; i ++ )
	{
		ECSReference *	pRef = new ECSReference ;
		err = pRef->Load( file, context ) ;
		if ( err )
		{
			delete	pRef ;
			return	err ;
		}
		m_lstRefModel.Add( pRef ) ;
	}
	//
	if ( file.Read( &dwCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	JointList().RemoveAll( ) ;
	for ( i = 0; i < dwCount; i ++ )
	{
		ECSObject *	pObj = NULL ;
		err = context.LoadObject( file, pObj ) ;
		if ( err )
		{
			return	err ;
		}
		E3DModelJoint *	pJoint = ESLTypeCast<E3DModelJoint>( pObj ) ;
		if ( pJoint == NULL )
		{
			context.delete_CSObject( pObj ) ;
			continue ;
		}
		AddSubJoint( pJoint ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	buf.Write( "\r\n", 2 ) ;
	//
	int	i, j ;
	EString	strIndent = EString('\t') * (nIndent + 1) ;
	EString	strDump = strIndent + "pos ( " ;
	for ( i = 0; i < 3; i ++ )
	{
		if ( i != 0 )
		{
			strDump += ", " ;
		}
		strDump += EString( m_pcsrPosition[i]->m_varReal ) ;
	}
	strDump += " )\r\n" ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	for ( i = 0; i < 3; i ++ )
	{
		strDump = strIndent + "m" + EString(i + 1) + " ( " ;
		for ( j = 0; j < 3; j ++ )
		{
			if ( j != 0 )
			{
				strDump += ", " ;
			}
			strDump += EString( Matrix().matrix[i][j] ) ;
		}
		strDump += " )\r\n" ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	}
	//
	for ( i = 0; i < (int) JointList().GetSize(); i ++ )
	{
		ECSObject *	pObj = ESLTypeCast<ECSObject>( JointList().GetAt( i ) ) ;
		if ( pObj == NULL )
		{
			continue ;
		}
		strDump = strIndent + "サブジョイント [" + EString(i) + "] " ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
		pObj->DumpObject( buf, nIndent + 1, context ) ;
	}
	//
	return	eslErrSuccess ;
}

// メンバ関数 : Reference Position()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_Position
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSReference( m_strcPosition ) ) ;
}

// メンバ関数 : InitializeMatrix()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_InitializeMatrix
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pRefJoint == NULL )
	{
		InitializeMatrix() ;
	}
	else
	{
		m_pRefJoint->InitializeMatrix( ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : RevolveOnX( Real rDeg )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_RevolveOnX
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	double	rDeg ;
	err = context.GetArgumentAsReal( rDeg, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pRefJoint == NULL )
	{
		RevolveOnX( rDeg ) ;
	}
	else
	{
		m_pRefJoint->RevolveOnX( rDeg ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : RevolveOnY( Real rDeg )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_RevolveOnY
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	double	rDeg ;
	err = context.GetArgumentAsReal( rDeg, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pRefJoint == NULL )
	{
		RevolveOnY( rDeg ) ;
	}
	else
	{
		m_pRefJoint->RevolveOnY( rDeg ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : RevolveOnZ( Real rDeg )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_RevolveOnZ
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	double	rDeg ;
	err = context.GetArgumentAsReal( rDeg, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pRefJoint == NULL )
	{
		RevolveOnZ( rDeg ) ;
	}
	else
	{
		m_pRefJoint->RevolveOnZ( rDeg ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : RevolveByAngleOn( Real x, Real y, Real z )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_RevolveByAngleOn
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	double	x, y, z ;
	err = context.GetArgumentAsReal( x, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( y, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( z, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pRefJoint == NULL )
	{
		RevolveByAngleOn( E3DVector( (REAL32) x, (REAL32) y, (REAL32) z ) ) ;
	}
	else
	{
		m_pRefJoint->RevolveByAngleOn
			( E3DVector( (REAL32) x, (REAL32) y, (REAL32) z ) ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : MagnifyByVector( Real x, Real y, Real z )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_MagnifyByVector
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	double	x, y, z ;
	err = context.GetArgumentAsReal( x, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( y, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( z, lstArg, 1, 0.0 ) ;
	if ( err )
		return	err ;
	//
	if ( m_pRefJoint == NULL )
	{
		MagnifyByVector( E3DVector( (REAL32) x, (REAL32) y, (REAL32) z ) ) ;
	}
	else
	{
		m_pRefJoint->MagnifyByVector
			( E3DVector( (REAL32) x, (REAL32) y, (REAL32) z ) ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Quaternion GetQuaternion()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_GetQuaternion
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	E3DDFQuaternion	q = Matrix() ;
	//
	ECSStructureInterface *	pq = context.CreateUserStructure( L"Quaternion" ) ;
	pq->SetMemberAsReal( L"q0", q.q[0] ) ;
	pq->SetMemberAsReal( L"q1", q.q[1] ) ;
	pq->SetMemberAsReal( L"q2", q.q[2] ) ;
	pq->SetMemberAsReal( L"q3", q.q[3] ) ;
	//
	return	context.PushObject( *pq ) ;
}

// メンバ関数 : Error SetQuaternion( const Quaternion& q )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_SetQuaternion
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pq =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Quaternion" ) ) ;
	if ( pq == NULL )
	{
		return	ESLErrorMsg
			( "SetQuaternion の引数に Quaternion が指定されていません" ) ;
	}
	E3DDFQuaternion	q ;
	q.q[0] = pq->GetMemberAsReal( L"q0", 1 ) ;
	q.q[1] = pq->GetMemberAsReal( L"q1", 0 ) ;
	q.q[2] = pq->GetMemberAsReal( L"q2", 0 ) ;
	q.q[3] = pq->GetMemberAsReal( L"q3", 0 ) ;
	//
	q.ToMatrix( Matrix() ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : AddModelRef( Reference rModel )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_AddModelRef
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	E3DPolygonModel *	pModel =
		ESLTypeCast<E3DPolygonModel>
			( context.GetArgumentObjectAs( lstArg, 1, L"PolygonModel" ) ) ;
	if ( pModel != NULL )
	{
		AddModelRef( pModel ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : ClearModelRef()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_ClearModelRef
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ModelList().RemoveAll( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer AddSubJoint( [joint] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_AddSubJoint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ;
	if ( pJoint != NULL )
	{
		ECSObject *	pDupJoint = pJoint->Duplicate( ) ;
		pJoint = ESLTypeCast<ECSModelJoint>( pDupJoint ) ;
		if ( pJoint == NULL )
		{
			context.delete_CSObject( pDupJoint ) ;
		}
	}
	if ( pJoint == NULL )
	{
		pJoint = new ECSModelJoint ;
	}
	//
	int	nIndex = JointList().GetSize() ;
	JointList().SetAt( nIndex, pJoint ) ;
	pJoint->m_parent = this ;
	//
	return	context.PushObject( context.new_CSInteger( nIndex ) ) ;
}

// メンバ関数 : ModelJoint& CreateSubJoint()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_CreateSubJoint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSModelJoint *	pJoint = new ECSModelJoint ;
	//
	int	nIndex = JointList().GetSize() ;
	JointList().SetAt( nIndex, pJoint ) ;
	pJoint->m_parent = this ;
	//
	return	context.PushObject( context.new_CSReference( pJoint ) ) ;
}

// メンバ関数 : Integer GetLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_GetLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( JointList().GetSize() ) ) ;
}

// メンバ関数 : RemoveSubJoint( Integer nIndex )
// メンバ関数 : RemoveSubJoint( ModelJoint& joint )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_RemoveSubJoint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nIndex ;
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>
			( context.GetArgumentObjectAs( lstArg, 1, L"ModelJoint" ) ) ;
	if ( pJoint != NULL )
	{
		nIndex = JointList().FindPtr( (E3DModelJoint*) pJoint ) ;
		if ( nIndex < 0 )
		{
			return	context.PushObject
				( context.new_CSInteger( eslErrFailed ) ) ;
		}
	}
	else
	{
		err = context.GetArgumentAsInt( nIndex, lstArg, 1, 0 ) ;
		if ( err )
			return	err ;
	}
	JointList().RemoveAt( nIndex ) ;
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : RemoveAllSubJoint()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_RemoveAllSubJoint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	JointList().RemoveAll( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : E3DColor GetColorAttribute()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_GetColorAttribute
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pColor = context.CreateUserStructure( L"E3DColor" ) ;
	pColor->SetMemberAsInt( L"rgbMul", m_clrModelColor.rgbMul.dwPixelCode ) ;
	pColor->SetMemberAsInt( L"rgbAdd", m_clrModelColor.rgbAdd.dwPixelCode ) ;
	//
	return	context.PushObject( *pColor ) ;
}

// メンバ関数 : Integer GetTransparency()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_GetTransparency
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( context.new_CSInteger( m_nTransparency ) ) ;
}

// メンバ関数 : SetColorAttribute( [E3DColor color] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_SetColorAttribute
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pColor =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"E3DColor" ) ) ;
	if ( pColor != NULL )
	{
		E3D_COLOR	clrModelColor ;
		clrModelColor.rgbMul.dwPixelCode =
					pColor->GetMemberAsInt( L"rgbMul", 0xFFFFFF ) ;
		clrModelColor.rgbAdd.dwPixelCode =
					pColor->GetMemberAsInt( L"rgbAdd", 0 ) ;
		SetColorAttribute( &clrModelColor ) ;
	}
	else
	{
		SetColorAttribute( NULL ) ;
	}
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : SetTransparency( Integer nTransparency )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_SetTransparency
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nTransparency ;
	err = context.GetArgumentAsInt( nTransparency, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetTransparency( nTransparency ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : SetColorMorphing( [E3DColor color], [Integer nTransparency] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_SetColorMorphing
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	E3D_COLOR *	pColor = NULL ;
	E3D_COLOR	clrModelColor ;
	int			nTransparency ;
	//
	ECSStructureInterface *	pstrcColor =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"E3DColor" ) ) ;
	if ( pstrcColor != NULL )
	{
		clrModelColor.rgbMul.dwPixelCode =
					pstrcColor->GetMemberAsInt( L"rgbMul", 0xFFFFFF ) ;
		clrModelColor.rgbAdd.dwPixelCode =
					pstrcColor->GetMemberAsInt( L"rgbAdd", 0 ) ;
		pColor = &clrModelColor ;
	}
	//
	err = context.GetArgumentAsInt
			( nTransparency, lstArg, 2, m_nTransparency ) ;
	if ( err )
		return	err ;
	//
	SetColorMorphing( pColor, nTransparency ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : SetBezierCurve
//		( [Array aCurve], [Array aRevolution], [Array aMagnification] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_SetBezierCurve
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 4 ) ;
	if ( err )
		return	err ;
	//
	EBezierCurves<E3D_VECTOR> *	pbzParam[3] = { NULL, NULL, NULL } ;
	EBezierCurves<E3D_VECTOR>	bzParam[3] ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		ECSArray *	pArray =
			ESLTypeCast<ECSArray>
				( context.GetArgumentObjectAs( lstArg, i + 1, L"Array" ) ) ;
		if ( pArray != NULL )
		{
			pbzParam[i] = &(bzParam[i]) ;
			for ( int j = 0; j < (int) pArray->m_varArray.GetSize(); j ++ )
			{
				ECSStructureInterface *	pVector =
					ESLTypeCast<ECSStructureInterface>
						( context.GetArgumentObjectAs
								( pArray->m_varArray, j, L"Vector" ) ) ;
				if ( pVector != NULL )
				{
					bzParam[i][j].x =
						(REAL32) pVector->GetMemberAsReal( L"x", 0 ) ;
					bzParam[i][j].y =
						(REAL32) pVector->GetMemberAsReal( L"y", 0 ) ;
					bzParam[i][j].z =
						(REAL32) pVector->GetMemberAsReal( L"z", 0 ) ;
				}
				else
				{
					pbzParam[i] = NULL ;
					break ;
				}
			}
		}
	}
	//
	SetBezierCurve( pbzParam[0], pbzParam[1], pbzParam[2] ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : Error SetRotationBezier( const Quaternion[]& aRotation )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_SetRotationBezier
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	EBezierCurves<E3D_QUATERNION>	bzRotation ;
	ECSArray *	pArray =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	if ( pArray != NULL )
	{
		for ( int i = 0; i < (int) pArray->m_varArray.GetSize(); i ++ )
		{
			ECSStructureInterface *	pq =
				ESLTypeCast<ECSStructureInterface>
					( context.GetArgumentObjectAs
							( pArray->m_varArray, i, L"Quaternion" ) ) ;
			if ( pq != NULL )
			{
				bzRotation[i].q[0] =
					(REAL32) pq->GetMemberAsReal( L"q0", 1 ) ;
				bzRotation[i].q[1] =
					(REAL32) pq->GetMemberAsReal( L"q1", 0 ) ;
				bzRotation[i].q[2] =
					(REAL32) pq->GetMemberAsReal( L"q2", 0 ) ;
				bzRotation[i].q[3] =
					(REAL32) pq->GetMemberAsReal( L"q3", 0 ) ;
			}
			else
			{
				return	context.PushObject
							( context.new_CSInteger( eslErrFailed ) ) ;
			}
		}
		SetRotationBezier( bzRotation ) ;
	}
	else
	{
		return	context.PushObject
					( context.new_CSInteger( eslErrFailed ) ) ;
	}
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : BeginActivation( Integer nDuration, Integer nActionType := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_BeginActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int	nDuration, nActionType ;
	err = context.GetArgumentAsInt( nDuration, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nActionType, lstArg, 2, actNormal ) ;
	if ( err )
		return	err ;
	//
	BeginActivation( nDuration, nActionType ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : FlushActivation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_FlushActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	FlushActivation( ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : Integer IsActivation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSModelJoint::Call_IsActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( context.new_CSInteger( - (long int) IsActivation() ) ) ;
}
