
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (C) 2004-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <gls.h>
#include <math.h>


//////////////////////////////////////////////////////////////////////////////
// レンダリングスプライト・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSRenderSprite::m_staFuncName = NULL ;
const wchar_t *	ECSRenderSprite::m_pwszFuncName[20] =
{
	L"Initialize", L"Release", L"SetViewPoint",
	L"SetEndOfViewCurve", L"SetViewOnCurve",
	L"BeginViewAnimation", L"FlushViewAnimation", L"IsViewAnimation",
	L"FlushActivation", L"IsActivation",
	L"SetZClipRange", L"SetLightEntries", L"GetSortingFlags",
	L"SetSortingFlags", L"AddModel", L"PrepareRendering",
	L"FlushAllPolygon", L"AttachRootJoint", L"UpdateRendering", NULL
} ;
const ECSRenderSprite::PFUNC_CALL	ECSRenderSprite::m_pfnCallFunc[19] =
{
	&ECSRenderSprite::Call_Initialize,
	&ECSRenderSprite::Call_Release,
	&ECSRenderSprite::Call_SetViewPoint,
	&ECSRenderSprite::Call_SetEndOfViewCurve,
	&ECSRenderSprite::Call_SetViewOnCurve,
	&ECSRenderSprite::Call_BeginViewAnimation,
	&ECSRenderSprite::Call_FlushViewAnimation,
	&ECSRenderSprite::Call_IsViewAnimation,
	&ECSRenderSprite::Call_FlushActivation,
	&ECSRenderSprite::Call_IsActivation,
	&ECSRenderSprite::Call_SetZClipRange,
	&ECSRenderSprite::Call_SetLightEntries,
	&ECSRenderSprite::Call_GetSortingFlags,
	&ECSRenderSprite::Call_SetSortingFlags,
	&ECSRenderSprite::Call_AddModel,
	&ECSRenderSprite::Call_PrepareRendering,
	&ECSRenderSprite::Call_FlushAllPolygon,
	&ECSRenderSprite::Call_AttachRootJoint,
	&ECSRenderSprite::Call_UpdateRendering,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSRenderSprite, ECSObject, E3DRenderSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSRenderSprite::ECSRenderSprite( void )
{
	m_pleLightEntries = NULL ;
	m_nLightEntryCount = 0 ;
	::eslFillMemory( &m_spParam, 0, sizeof(m_spParam) ) ;
	//
	m_dwCameraFlags = 0 ;
	m_dwDurationTime = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSRenderSprite::~ECSRenderSprite( void )
{
	Release( ) ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::SetViewPoint
	( const E3D_VECTOR & vViewPoint,
		const E3D_VECTOR & vTarget, double rDegAngle )
{
	m_vacCameraCurve.SetViewPoint( 0, vViewPoint, vTarget, rDegAngle ) ;
	m_vacCameraCurve.SetViewPoint( 1, vViewPoint, vTarget, rDegAngle ) ;
	//
	m_spParam.vViewPoint[0] = m_spParam.vViewPoint[1] = vViewPoint ;
	m_spParam.vViewTarget[0] = m_spParam.vViewTarget[1] = vTarget ;
	m_spParam.rViewAngleZ[0] = m_spParam.rViewAngleZ[1] = rDegAngle ;
	//
	E3DRenderSprite::SetViewPoint( vViewPoint, vTarget, rDegAngle ) ;
}

// カメラ曲線設定
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::SetEndOfViewCurve
	( const E3D_VECTOR & vViewPoint,
		const E3D_VECTOR & vTarget, double rDegAngle, DWORD dwFlags )
{
	m_vacCameraCurve.SetViewPoint( 1, vViewPoint, vTarget, rDegAngle ) ;
	m_spParam.vViewPoint[1] = vViewPoint ;
	m_spParam.vViewTarget[1] = vTarget ;
	m_spParam.rViewAngleZ[1] = rDegAngle ;
	m_dwCameraFlags = dwFlags ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::SetViewOnCurve( double t )
{
	m_vacCameraCurve.SetViewCurveFor( m_vpjView, t, m_dwCameraFlags ) ;
}

// カメラアニメーション設定
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::BeginViewAnimation( DWORD dwDuration )
{
	m_dwDurationTime = dwDuration ;
	m_dwAnimationTime = 0 ;
	//
	if ( dwDuration == 0 )
	{
		m_dwDurationTime = 1 ;
		FlushViewAnimation( ) ;
	}
}

// カメラアニメーション終了
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::FlushViewAnimation( void )
{
	if ( m_dwDurationTime != 0 )
	{
		SetViewOnCurve( 1.0 ) ;
		//
		E3DVector	vViewAngle = m_vpjView.GetViewAngle( ) ;
		E3DVector	vViewPoint = m_vpjView.GetViewPoint( ) ;
		E3DVector	vTarget = vViewPoint + vViewAngle ;
		double		rRevZ = m_vpjView.GetRevolveZ( ) ;
		SetViewPoint( vViewPoint, vTarget, rRevZ ) ;
		//
		UpdateRendering( ) ;
		m_dwDurationTime = 0 ;
	}
}

// アニメーション終了
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::FlushActivation( void )
{
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>( m_refRootJoint.m_pRef ) ;
	if ( pJoint != NULL )
	{
		pJoint->FlushActivation() ;
	}
	if ( m_dwDurationTime != 0 )
	{
		FlushViewAnimation( ) ;
	}
	else
	{
		if ( pJoint != NULL )
		{
			UpdateRendering( ) ;
		}
	}
}

// アニメーション中か？
//////////////////////////////////////////////////////////////////////////////
bool ECSRenderSprite::IsActivation( void ) const
{
	if ( IsViewAnimation() )
	{
		return	true ;
	}
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>( m_refRootJoint.m_pRef ) ;
	if ( pJoint != NULL )
	{
		return	pJoint->IsActivation( ) ;
	}
	return	false ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
#if	!defined(_DEBUG)
	try
	{
#endif
		//
		// レンダリング実行
		//
		E3DRenderSprite::MTDraw( hRenderPoly ) ;
		//
#if	!defined(_DEBUG)
	}
	catch ( ... )
	{
		EString	strErrMsg =
			"ECSRenderSprite::Draw 関数で例外エラーが発生しました。(" ;
		strErrMsg += EString( (DWORD) this ) ;
		strErrMsg += ")\n" ;
		::OutputDebugString( strErrMsg ) ;
		//
		FlushAllPolygon( ) ;
	}
#endif
}

// リソース開放
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Release( void )
{
	ESpriteServer *	pParent =
		ESLTypeCast<ESpriteServer>( GetParent() ) ;
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>( GetParent() ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	if ( pParent != NULL )
	{
		SetVisible( false ) ;
		pParent->DetachSprite( this ) ;
	}
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	//
	m_refInitParent.SetReference( NULL ) ;
	//
	if ( m_pleLightEntries != NULL )
	{
		::eslHeapFree( NULL, m_pleLightEntries, 0 ) ;
		m_pleLightEntries = NULL ;
		m_nLightEntryCount = 0 ;
	}
	//
	m_refRootJoint.SetReference( NULL ) ;
	//
	return	E3DRenderSprite::Release( ) ;
}

// ルートジョイント関連付け
//////////////////////////////////////////////////////////////////////////////
ECSModelJoint * ECSRenderSprite::AttachRootJoint( ECSModelJoint * pJoint )
{
	ECSModelJoint *	pOldJoint =
		ESLTypeCast<ECSModelJoint>( m_refRootJoint.m_pRef ) ;
	m_refRootJoint.SetReference( pJoint ) ;
	return	pOldJoint ;
}

// 表示更新
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::UpdateRendering( void )
{
	FlushAllPolygon( ) ;
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>( m_refRootJoint.m_pRef ) ;
	if ( pJoint != NULL )
	{
		pJoint->AddModelToRender( *this ) ;
	}
	return	PrepareRendering( ) ;
}

// アニメーション処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::OnAdvanceAnimation( unsigned int nTime )
{
	bool	fUpdateRendering = false ;
	ECSModelJoint *	pJoint =
		ESLTypeCast<ECSModelJoint>( m_refRootJoint.m_pRef ) ;
	if ( pJoint != NULL )
	{
		if ( pJoint->OnAdvanceAnimation( nTime ) )
		{
			fUpdateRendering = true ;
		}
	}
	if ( m_dwDurationTime != 0 )
	{
		m_dwAnimationTime += nTime ;
		//
		SDWORD	dwOffsetTime = m_dwAnimationTime ;
		if ( dwOffsetTime > m_dwDurationTime )
		{
			dwOffsetTime = m_dwDurationTime ;
		}
		SetViewOnCurve( (double) dwOffsetTime / m_dwDurationTime ) ;
		//
		if ( dwOffsetTime >= m_dwDurationTime )
		{
			m_dwDurationTime = 0 ;
			m_dwAnimationTime = 0 ;
			//
			E3DVector	vViewAngle = m_vpjView.GetViewAngle( ) ;
			E3DVector	vViewPoint = m_vpjView.GetViewPoint( ) ;
			E3DVector	vTarget = vViewPoint + vViewAngle ;
			double		rRevZ = m_vpjView.GetRevolveZ( ) ;
			SetViewPoint( vViewPoint, vTarget, rRevZ ) ;
		}
		fUpdateRendering = true ;
	}
	if ( fUpdateRendering )
	{
		UpdateRendering( ) ;
	}
	return	eslErrSuccess ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSRenderSprite::GetTypeName( void ) const
{
	return	L"RenderSprite" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSRenderSprite::Duplicate( void )
{
	return	new ECSRenderSprite ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Move( ECSContext & context, ECSObject * obj )
{
	return	ESLErrorMsg( "RenderSprite への定義されていない代入操作です。" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "RenderSprite への定義されていない演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "RenderSprite への定義されていない演算子です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "RenderSprite の定義されていない比較です。" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "RenderSprite の定義されていない"
				"メンバ関数を呼び出そうとしています。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg
			( "RenderSprite の定義されていない"
				"メンバ関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::IndexAllMember( void )
{
	m_refInitParent.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSRenderSprite::CleanupAllReference( ECSContext & context )
{
	ESpriteServer *	pParent =
		ESLTypeCast<ESpriteServer>( GetParent() ) ;
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>( GetParent() ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	if ( pParent != NULL )
	{
		SetVisible( false ) ;
		pParent->DetachSprite( this ) ;
	}
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	m_refInitParent.CleanupAllReference( context ) ;
	m_refRootJoint.CleanupAllReference( context ) ;
	//
	ECSObject::CleanupAllReference( context ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::CommitAllReference( ECSContext & context )
{
	ESLError	err = m_refInitParent.CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	err = m_refRootJoint.CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	ECSSprite *	pParent = ESLTypeCast<ECSSprite>( m_refInitParent.m_pRef ) ;
	if ( pParent != NULL )
	{
		EGL_RECT *	pClipRect = NULL ;
		if ( (m_spParam.rctInitClip.left <= m_spParam.rctInitClip.right)
			&& (m_spParam.rctInitClip.top <= m_spParam.rctInitClip.bottom) )
		{
			pClipRect = &(m_spParam.rctInitClip) ;
		}
		err = Initialize
			( pParent->GetInfo(), pClipRect, pParent->GetZBuffer(),
				&pParent->GetScreenPosition(),
				m_spParam.nInitHeapSize, m_spParam.nInitPolyLimit,
				(MultiRenderingFlag) m_spParam.nInitFlags ) ;
		if ( !err )
		{
			if ( GetParent() != pParent )
			{
				ESpriteServer *	pOldParent =
					ESLTypeCast<ESpriteServer>( GetParent() ) ;
				if ( pOldParent != NULL )
				{
					pOldParent->DetachSprite( this ) ;
				}
				pParent->AddSprite( m_spParam.nInitPriority, this ) ;
				SetVisible( true ) ;
			}
			else if ( GetPriority() != m_spParam.nInitPriority )
			{
				ChangePriority( m_spParam.nInitPriority ) ;
			}
			//
			SetSortingFlags( m_spParam.dwSortingFlags ) ;
			//
			E3DRenderSprite::SetViewPoint
				( m_spParam.vViewPoint[0],
					m_spParam.vViewTarget[0], m_spParam.rViewAngleZ[0] ) ;
			//
			m_vacCameraCurve.SetViewPoint
				( 0, m_spParam.vViewPoint[0],
					m_spParam.vViewTarget[0], m_spParam.rViewAngleZ[0] ) ;
			m_vacCameraCurve.SetViewPoint
				( 1, m_spParam.vViewPoint[1],
					m_spParam.vViewTarget[1], m_spParam.rViewAngleZ[1] ) ;
			m_dwCameraFlags = m_spParam.dwCameraFlags ;
			m_dwDurationTime = m_spParam.dwDurationTime ;
			m_dwAnimationTime = m_spParam.dwOffsetTime ;
			//
			SetZClipRange
				( m_spParam.rClipMinZ, m_spParam.rClipMaxZ ) ;
			//
			if ( m_pleLightEntries && m_nLightEntryCount )
			{
				SetLightEntries( m_nLightEntryCount, m_pleLightEntries ) ;
			}
		}
	}
	err = UpdateRendering( ) ;
	if ( err )
	{
		ESLTrace( "Failed to UpdateRendering at ECSRenderSprite::CommitAllReference\n" ) ;
//		return	err ;
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Save( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwVersionFlags = 2 ;
	file.Write( &dwVersionFlags, sizeof(DWORD) ) ;
	//
	ESLError	err ;
	err = m_refInitParent.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	DWORD	dwBytes = sizeof(m_spParam) ;
	m_spParam.dwSortingFlags = GetSortingFlags( ) ;
	m_spParam.dwDurationTime = m_dwDurationTime ;
	m_spParam.dwOffsetTime = m_dwAnimationTime ;
	file.Write( &dwBytes, sizeof(dwBytes) ) ;
	file.Write( &m_spParam, dwBytes ) ;
	//
	file.Write( &m_nLightEntryCount, sizeof(int) ) ;
	if ( m_nLightEntryCount > 0 )
	{
		file.Write
			( m_pleLightEntries,
				sizeof(E3D_LIGHT_ENTRY) * m_nLightEntryCount ) ;
	}
	//
	err = m_refRootJoint.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Load( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwVersionFlags ;
	file.Read( &dwVersionFlags, sizeof(DWORD) ) ;
	if ( dwVersionFlags > 2 )
	{
		return	eslErrGeneral ;
	}
	ESLError	err ;
	err = m_refInitParent.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwBytes = 0 ;
	::eslFillMemory( &m_spParam, 0, sizeof(m_spParam) ) ;
	if ( file.Read( &dwBytes, sizeof(dwBytes) ) < sizeof(dwBytes) )
	{
		return	eslErrGeneral ;
	}
	if ( dwBytes > sizeof(m_spParam) )
	{
		return	eslErrGeneral ;
	}
	file.Read( &m_spParam, dwBytes ) ;
	//
	if ( file.Read( &m_nLightEntryCount, sizeof(int) ) < sizeof(int) )
	{
		return	eslErrGeneral ;
	}
	if ( m_nLightEntryCount > 0 )
	{
		m_pleLightEntries =
			(E3D_LIGHT_ENTRY*) ::eslHeapReallocate
				( NULL, m_pleLightEntries,
					sizeof(E3D_LIGHT_ENTRY) * m_nLightEntryCount, 0 ) ;
		file.Read
			( m_pleLightEntries,
				sizeof(E3D_LIGHT_ENTRY) * m_nLightEntryCount ) ;
	}
	//
	m_refRootJoint.SetReference( NULL ) ;
	if ( dwVersionFlags >= 2 )
	{
		err = m_refRootJoint.Load( file, context ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump = "ポリゴン数 = "
				+ EString( (int) m_plObject[m_iCurrentView].dwCount ) ;
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	return	eslErrSuccess ;
}

// メンバ関数 : Initialize Initialize
//		( Reference rParent, Integer nPriority,
//			Integer nHeapSize := 100000H,
//			Integer nPolyLimit := 8000H,
//			Integer nFlag := mtfAuto[, Rect rectClip] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_Initialize
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 7 ) ;
	if ( err )
		return	err ;
	//
	// 親スプライト取得
	//
	ESpriteServer *	pParent =
		ESLTypeCast<ESpriteServer>
			( context.GetArgumentObjectAs( lstArg, 1, L"Sprite" ) ) ;
	if ( pParent == NULL )
	{
		return	context.PushObject( new ECSInteger( eslErrInvalidParam ) ) ;
	}
	//
	// パラメータ取得
	//
	int	nPriority, nHeapSize, nPolyLimit, nFlag ;
	EGL_RECT *	pClipRect = NULL ;
	EGL_RECT	rectClip ;
	err = context.GetArgumentAsInt( nPriority, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeapSize, lstArg, 3, 0x100000 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nPolyLimit, lstArg, 4, 0x8000 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFlag, lstArg, 5, mrfAuto ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pStructRect =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 6, L"Rect" ) ) ;
	if ( pStructRect != NULL )
	{
		rectClip.left = pStructRect->GetMemberAsInt( L"left", 0 ) ;
		rectClip.top = pStructRect->GetMemberAsInt( L"top", 0 ) ;
		rectClip.right = pStructRect->GetMemberAsInt( L"right", 0 ) ;
		rectClip.bottom = pStructRect->GetMemberAsInt( L"bottom", 0 ) ;
		pClipRect = &rectClip ;
	}
	//
	// 初期化実行
	//
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>
			( context.GetArgumentObjectAs( lstArg, 1, L"Sprite" ) ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	//
	err = Initialize
		( pParent->GetInfo(), pClipRect, pParent->GetZBuffer(),
			&pParent->GetScreenPosition(),
			nHeapSize, nPolyLimit, (MultiRenderingFlag) nFlag ) ;
	if ( !err )
	{
		//
		// 親スプライト設定
		//
		if ( GetParent() != pParent )
		{
			ESpriteServer *	pOldParent =
				ESLTypeCast<ESpriteServer>( GetParent() ) ;
			if ( pOldParent != NULL )
			{
				pOldParent->DetachSprite( this ) ;
			}
			pParent->AddSprite( nPriority, this ) ;
			SetVisible( true ) ;
		}
		else if ( GetPriority() != nPriority )
		{
			ChangePriority( nPriority ) ;
		}
		//
		m_refInitParent.SetReference( pParentSprite ) ;
		m_spParam.nInitPriority = nPriority ;
		m_spParam.nInitHeapSize = nHeapSize ;
		m_spParam.nInitPolyLimit = nPolyLimit ;
		m_spParam.nInitFlags = nFlag ;
		if ( pClipRect != NULL )
		{
			m_spParam.rctInitClip = *pClipRect ;
		}
		else
		{
			m_spParam.rctInitClip.left = 0 ;
			m_spParam.rctInitClip.top = 0 ;
			m_spParam.rctInitClip.right = -1 ;
			m_spParam.rctInitClip.bottom = -1 ;
		}
		m_spParam.rClipMinZ = 1.0F ;
		m_spParam.rClipMaxZ = 4294967296.0F ;
	}
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Release()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_Release
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ESpriteServer *	pParent =
		ESLTypeCast<ESpriteServer>( GetParent() ) ;
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>( GetParent() ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	if ( pParent != NULL )
	{
		SetVisible( false ) ;
		pParent->DetachSprite( this ) ;
	}
	err = Release( ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : SetViewPoint
//		( Vector vViewPoint, Vector vTarget, Real rDegAngleZ )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_SetViewPoint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pViewPoint =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Vector" ) ) ;
	if ( pViewPoint == NULL )
	{
		return	ESLErrorMsg
			( "1 番目の引数に Vector 構造体が指定されていません。" ) ;
	}
	ECSStructureInterface *	pTarget =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Vector" ) ) ;
	if ( pTarget == NULL )
	{
		return	ESLErrorMsg
			( "2 番目の引数に Vector 構造体が指定されていません。" ) ;
	}
	double	rDegAngleZ ;
	err = context.GetArgumentAsReal( rDegAngleZ, lstArg, 3, 0.0 ) ;
	if ( err )
		return	err ;
	//
	E3D_VECTOR	vViewPoint, vTarget ;
	vViewPoint.x = (REAL32) pViewPoint->GetMemberAsReal( L"x", 0.0 ) ;
	vViewPoint.y = (REAL32) pViewPoint->GetMemberAsReal( L"y", 0.0 ) ;
	vViewPoint.z = (REAL32) pViewPoint->GetMemberAsReal( L"z", 0.0 ) ;
	vTarget.x = (REAL32) pTarget->GetMemberAsReal( L"x", 0.0 ) ;
	vTarget.y = (REAL32) pTarget->GetMemberAsReal( L"y", 0.0 ) ;
	vTarget.z = (REAL32) pTarget->GetMemberAsReal( L"z", 0.0 ) ;
	//
	SetViewPoint( vViewPoint, vTarget, rDegAngleZ ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetEndOfViewCurve
//		( Vector vViewPoint, Vector vTarget,
//				Real rDegAngleZ, Integer nFlags := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_SetEndOfViewCurve
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4, 5 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pViewPoint =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Vector" ) ) ;
	if ( pViewPoint == NULL )
	{
		return	ESLErrorMsg
			( "1 番目の引数に Vector 構造体が指定されていません。" ) ;
	}
	ECSStructureInterface *	pTarget =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Vector" ) ) ;
	if ( pTarget == NULL )
	{
		return	ESLErrorMsg
			( "2 番目の引数に Vector 構造体が指定されていません。" ) ;
	}
	double	rDegAngleZ ;
	err = context.GetArgumentAsReal( rDegAngleZ, lstArg, 3, 0.0 ) ;
	if ( err )
		return	err ;
	//
	int	nFlags ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 4, 0 ) ;
	if ( err )
		return	err ;
	//
	E3D_VECTOR	vViewPoint, vTarget ;
	vViewPoint.x = (REAL32) pViewPoint->GetMemberAsReal( L"x", 0.0 ) ;
	vViewPoint.y = (REAL32) pViewPoint->GetMemberAsReal( L"y", 0.0 ) ;
	vViewPoint.z = (REAL32) pViewPoint->GetMemberAsReal( L"z", 0.0 ) ;
	vTarget.x = (REAL32) pTarget->GetMemberAsReal( L"x", 0.0 ) ;
	vTarget.y = (REAL32) pTarget->GetMemberAsReal( L"y", 0.0 ) ;
	vTarget.z = (REAL32) pTarget->GetMemberAsReal( L"z", 0.0 ) ;
	//
	SetEndOfViewCurve( vViewPoint, vTarget, rDegAngleZ, nFlags ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetViewOnCurve( Real t )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_SetViewOnCurve
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	double	t ;
	err = context.GetArgumentAsReal( t, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetViewOnCurve( t ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : BeginViewAnimation( Integer nDuration )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_BeginViewAnimation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nDuration ;
	err = context.GetArgumentAsInt( nDuration, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>( GetParent() ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	//
	BeginViewAnimation( nDuration ) ;
	//
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : FlushViewAnimation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_FlushViewAnimation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>( GetParent() ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	//
	FlushViewAnimation( ) ;
	//
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsViewAnimation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_IsViewAnimation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( context.new_CSInteger( IsViewAnimation() ? -1 : 0 ) ) ;
}

// メンバ関数 : FlushActivation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_FlushActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>( GetParent() ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	FlushActivation( ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 : Integer IsActivation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_IsActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( context.new_CSInteger( IsActivation() ? -1 : 0 ) ) ;
}

// メンバ関数 : SetZClipRange( Real rMinZ, Real rMaxZ )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_SetZClipRange
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	double	rMin, rMax ;
	err = context.GetArgumentAsReal( rMin, lstArg, 1, 1.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( rMax, lstArg, 2, 1.0e9 ) ;
	if ( err )
		return	err ;
	//
	m_spParam.rClipMinZ = (REAL32) rMin ;
	m_spParam.rClipMaxZ = (REAL32) rMax ;
	err = SetZClipRange( (REAL32) rMin, (REAL32) rMax ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : SetLightEntries( Array aLights )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_SetLightEntries
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pLights =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	if ( pLights == NULL )
	{
		return	ESLErrorMsg
			( "1 番目の引数に Array 型が指定されていません。" ) ;
	}
	//
	int		i, nCount = 0 ;
	m_nLightEntryCount = pLights->m_varArray.GetSize( ) ;
	m_pleLightEntries =
		(E3D_LIGHT_ENTRY*) ::eslHeapReallocate
			( NULL, m_pleLightEntries,
				sizeof(E3D_LIGHT_ENTRY) * m_nLightEntryCount, 0 ) ;
	for ( i = 0; i < (int) m_nLightEntryCount; i ++ )
	{
		ECSStructure *	pLight =
			ESLTypeCast<ECSStructure>( pLights->m_varArray.GetAt( i ) ) ;
		if ( (pLight == NULL)
			|| EWideString::Compare( pLight->m_pwszTag, L"LightEntry") )
		{
			continue ;
		}
		m_pleLightEntries[nCount].dwLightType =
			pLight->GetMemberAsInt( L"nLightType", 0 ) ;
		m_pleLightEntries[nCount].rgbColor.dwPixelCode =
			pLight->GetMemberAsInt( L"rgbColor", 0x00FFFFFF ) ;
		m_pleLightEntries[nCount].rBrightness =
			(REAL32) pLight->GetMemberAsReal( L"rBrightness", 1.0 ) ;
		//
		if ( m_pleLightEntries[nCount].dwLightType == E3D_FOG_LIGHT )
		{
			m_pleLightEntries[nCount].rFogDeepness =
				(REAL32) pLight->GetMemberAsReal( L"rFogDeepness", 1000.0 ) ;
		}
		m_pleLightEntries[nCount].rFogDistance =
			(REAL32) pLight->GetMemberAsReal( L"rFogDistance", 1000.0 ) ;
		//
		ECSStructureInterface *	pVecLight =
			ESLTypeCast<ECSStructureInterface>
				( pLight->GetMemberAs( L"vecLight" ) ) ;
		if ( pVecLight != NULL )
		{
			m_pleLightEntries[nCount].vecLight.x =
				(REAL32) pVecLight->GetMemberAsReal( L"x", 1.0 ) ;
			m_pleLightEntries[nCount].vecLight.y =
				(REAL32) pVecLight->GetMemberAsReal( L"y", 1.0 ) ;
			m_pleLightEntries[nCount].vecLight.z =
				(REAL32) pVecLight->GetMemberAsReal( L"z", 1.0 ) ;
			nCount ++ ;
		}
	}
	m_nLightEntryCount = nCount ;
	err = SetLightEntries( nCount, m_pleLightEntries ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer GetSortingFlags()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_GetSortingFlags
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetSortingFlags() ) ) ;
}

// メンバ関数 : SetSortingFlags( Integer nFlags )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_SetSortingFlags
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nFlags = 0 ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetSortingFlags( nFlags ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : AddModel
//		( Reference rJoint[, E3DColor color[, Integer nTransparency]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_AddModel
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	E3DModelJoint *	pJoint =
		ESLTypeCast<E3DModelJoint>
			( context.GetArgumentObjectAs( lstArg, 1, L"ModelJoint" ) ) ;
	if ( pJoint == NULL )
	{
		return	ESLErrorMsg
			( "1 番目の引数にジョイントオブジェクトが指定されていません。" ) ;
	}
	E3D_COLOR *	pColor = NULL ;
	E3D_COLOR	clrApply ;
	int			nTransparency = 0 ;
	ECSStructureInterface *	p3DColor =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"E3DColor" ) ) ;
	if ( p3DColor != NULL )
	{
		clrApply.rgbMul.dwPixelCode =
			p3DColor->GetMemberAsInt( L"rgbMul", 0x00FFFFFF ) ;
		clrApply.rgbAdd.dwPixelCode =
			p3DColor->GetMemberAsInt( L"rgbAdd", 0x00FFFFFF ) ;
		pColor = &clrApply ;
	}
	err = context.GetArgumentAsInt( nTransparency, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
#if	!defined(_DEBUG)
	try
	{
#endif
		//
		err = AddModel( *pJoint, pColor, nTransparency ) ;
		//
#if	!defined(_DEBUG)
	}
	catch ( ... )
	{
		EString	strErrMsg =
			"Occured exception in ECSRenderSprite::AddModel (";
		strErrMsg += EString( (DWORD) this ) ;
		strErrMsg += ")\n" ;
		::OutputDebugString( strErrMsg ) ;
	}
#endif
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : PrepareRendering()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_PrepareRendering
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
#if	!defined(_DEBUG)
	try
	{
#endif
		//
		err = PrepareRendering( ) ;
		//
#if	!defined(_DEBUG)
	}
	catch ( ... )
	{
		EString	strErrMsg =
			"Occured exception in ECSRenderSprite::PrepareRendering (";
		strErrMsg += EString( (DWORD) this ) ;
		strErrMsg += ")\n" ;
		::OutputDebugString( strErrMsg ) ;
	}
#endif
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : FlushAllPolygon()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_FlushAllPolygon
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
#if	!defined(_DEBUG)
	try
	{
#endif
		//
		err = FlushAllPolygon( ) ;
		//
#if	!defined(_DEBUG)
	}
	catch ( ... )
	{
		EString	strErrMsg =
			"Occured exception in ECSRenderSprite::FlushAllPolygon (";
		strErrMsg += EString( (DWORD) this ) ;
		strErrMsg += ")\n" ;
		::OutputDebugString( strErrMsg ) ;
	}
#endif
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : AttachRootJoint( Reference rRootJoint )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_AttachRootJoint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	AttachRootJoint
		( ESLTypeCast<ECSModelJoint>( ECSObject::GetEntity( lstArg.GetAt(1) ) ) ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : UpdateRendering()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSRenderSprite::Call_UpdateRendering
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSSprite *	pParentSprite =
		ESLTypeCast<ECSSprite>( GetParent() ) ;
	if ( pParentSprite != NULL )
	{
		pParentSprite->Lock( ) ;
	}
	//
	err = UpdateRendering( ) ;
	//
	if ( pParentSprite != NULL )
	{
		pParentSprite->Unlock( ) ;
	}
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

