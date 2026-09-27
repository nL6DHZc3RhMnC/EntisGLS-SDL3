
/*****************************************************************************
               Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2003-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// スクリプトインターフェース＋スプライトインターフェース
//////////////////////////////////////////////////////////////////////////////

ECSWindow *	ECSSprite::m_pMainWnd = NULL ;

ECSStrTagArray *	ECSSprite::m_staFuncName = NULL ;
const wchar_t *		ECSSprite::m_pwszFuncName[92] =
{
	// ImageSprite 操作
	L"GetInfo", L"GetImageInfo",
	L"Release", L"AttachImage", L"CreateSprite",
	L"SetBackColor", L"EnableDynamicMode",
	L"CreateZBuffer", L"DeleteZBuffer",
	L"CreateStereoBuffer", L"SetStereoViewInfo", L"DeleteStereoBuffer",
	L"Set3DViewCamera", L"Enable3DViewCamera", L"Get3DViewCamera",
	L"GetScreenPosition", L"SetScreenPosition",
	L"GetDrawFunctionFlags", L"SetDrawFunctionFlags",
	L"GetRenderFunctionFlags", L"SetRenderFunctionFlags",
	L"GetParent", L"IsVisible", L"SetVisible",
	L"GetRectangle", L"MovePosition", L"GetPosition",
	L"GetTransparency", L"SetTransparency",
	L"SetZPosition", L"GetZPosition",
	L"GetParameter", L"SetParameter", L"CopyParameters",
	L"UpdateRect", L"Refresh", L"GetPriority", L"ChangePriority",
	L"AddSprite", L"DetachSprite", L"DetachAllSprite",
	L"DrawImage", L"FillRect", L"DrawText",
	// SpriteInterface 操作
	L"GetSpriteID", L"SetSpriteID",
	L"Enable", L"IsEnabled", L"GetSpriteText",
	L"SetSpriteText", L"SetSpriteFontFace", L"SetSpriteImage",
	L"IsHitSprite", L"GetSpriteAtPoint",
	L"GetFocus", L"SetFocus", L"KillFocus", L"MoveFocus",
	L"SetCapture", L"ReleaseCapture",
	L"GetVertScrollPos", L"SetVertScrollPos", L"GetVertScrollRange",
	L"SetVertScrollRange", L"GetHorzScrollPos", L"SetHorzScrollPos",
	L"GetHorzScrollRange", L"SetHorzScrollRange", L"IsButtonChecked",
	L"CheckButton", L"GetButtonViewStyle", L"SendCommand",
	L"SetHitTestProcedure", L"SetTimerProcedure",
	L"SetMouseInterface", L"SetKeyInterface",
	// ScriptSpriteInterface 操作
	L"ModifyAnimationFlags",
	L"BeginAnimation", L"EndAnimation", L"IsDuringAnimation",
	L"SetAlphaImage", L"GetBlendDegree", L"SetBlendDegree",
	L"SetBlendingEnvelope", L"SetBezierCurve",
	L"SetCameraCurve", L"BeginActivation",
	L"FlushActivation", L"CancelActivation", L"IsActivation",
	L"AttachToneFilter",
	NULL
} ;

const ECSSprite::PFUNC_CALL
	ECSSprite::m_pfnCallFunc[91] =
{
	// ImageSprite 系
	&ECSSprite::Call_GetInfo,
	&ECSSprite::Call_GetImageInfo,
	&ECSSprite::Call_Release,
	&ECSSprite::Call_AttachImage,
	&ECSSprite::Call_CreateSprite,
	&ECSSprite::Call_SetBackColor,
	&ECSSprite::Call_EnableDynamicMode,
	&ECSSprite::Call_CreateZBuffer,
	&ECSSprite::Call_DeleteZBuffer,
	&ECSSprite::Call_CreateStereoBuffer,
	&ECSSprite::Call_SetStereoViewInfo,
	&ECSSprite::Call_DeleteStereoBuffer,
	&ECSSprite::Call_Set3DViewCamera,
	&ECSSprite::Call_Enable3DViewCamera,
	&ECSSprite::Call_Get3DViewCamera,
	&ECSSprite::Call_GetScreenPosition,
	&ECSSprite::Call_SetScreenPosition,
	&ECSSprite::Call_GetDrawFunctionFlags,
	&ECSSprite::Call_SetDrawFunctionFlags,
	&ECSSprite::Call_GetRenderFunctionFlags,
	&ECSSprite::Call_SetRenderFunctionFlags,
	&ECSSprite::Call_GetParent,
	&ECSSprite::Call_IsVisible,
	&ECSSprite::Call_SetVisible,
	&ECSSprite::Call_GetRectangle,
	&ECSSprite::Call_MovePosition,
	&ECSSprite::Call_GetPosition,
	&ECSSprite::Call_GetTransparency,
	&ECSSprite::Call_SetTransparency,
	&ECSSprite::Call_SetZPosition,
	&ECSSprite::Call_GetZPosition,
	&ECSSprite::Call_GetParameter,
	&ECSSprite::Call_SetParameter,
	&ECSSprite::Call_CopyParameters,
	&ECSSprite::Call_UpdateRect,
	&ECSSprite::Call_Refresh,
	&ECSSprite::Call_GetPriority,
	&ECSSprite::Call_ChangePriority,
	&ECSSprite::Call_AddSprite,
	&ECSSprite::Call_DetachSprite,
	&ECSSprite::Call_DetachAllSprite,
	&ECSSprite::Call_DrawImage,
	&ECSSprite::Call_FillRect,
	&ECSSprite::Call_DrawText,
	// SpriteInterface 系
	&ECSSprite::Call_GetSpriteID,
	&ECSSprite::Call_SetSpriteID,
	&ECSSprite::Call_Enable,
	&ECSSprite::Call_IsEnabled,
	&ECSSprite::Call_GetSpriteText,
	&ECSSprite::Call_SetSpriteText,
	&ECSSprite::Call_SetSpriteFontFace,
	&ECSSprite::Call_SetSpriteImage,
	&ECSSprite::Call_IsHitSprite,
	&ECSSprite::Call_GetSpriteAtPoint,
	&ECSSprite::Call_GetFocus,
	&ECSSprite::Call_SetFocus,
	&ECSSprite::Call_KillFocus,
	&ECSSprite::Call_MoveFocus,
	&ECSSprite::Call_SetCapture,
	&ECSSprite::Call_ReleaseCapture,
	&ECSSprite::Call_GetVertScrollPos,
	&ECSSprite::Call_SetVertScrollPos,
	&ECSSprite::Call_GetVertScrollRange,
	&ECSSprite::Call_SetVertScrollRange,
	&ECSSprite::Call_GetHorzScrollPos,
	&ECSSprite::Call_SetHorzScrollPos,
	&ECSSprite::Call_GetHorzScrollRange,
	&ECSSprite::Call_SetHorzScrollRange,
	&ECSSprite::Call_IsButtonChecked,
	&ECSSprite::Call_CheckButton,
	&ECSSprite::Call_GetButtonViewStyle,
	&ECSSprite::Call_SendCommand,
	&ECSSprite::Call_SetHitTestProcedure,
	&ECSSprite::Call_SetTimerProcedure,
	&ECSSprite::Call_SetMouseInterface,
	&ECSSprite::Call_SetKeyInterface,
	// ScriptSpriteInterface 系
	&ECSSprite::Call_ModifyAnimationFlags,
	&ECSSprite::Call_BeginAnimation,
	&ECSSprite::Call_EndAnimation,
	&ECSSprite::Call_IsDuringAnimation,
	&ECSSprite::Call_SetAlphaImage,
	&ECSSprite::Call_GetBlendDegree,
	&ECSSprite::Call_SetBlendDegree,
	&ECSSprite::Call_SetBlendingEnvelope,
	&ECSSprite::Call_SetBezierCurve,
	&ECSSprite::Call_SetCameraCurve,
	&ECSSprite::Call_BeginActivation,
	&ECSSprite::Call_FlushActivation,
	&ECSSprite::Call_CancelActivation,
	&ECSSprite::Call_IsActivation,
	&ECSSprite::Call_AttachToneFilter,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSSprite, ECSResource, EAnimationSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSSprite::ECSSprite( void )
{
	m_vtType = csvtObject ;
	m_dwFlags = ffGroup | ffTimer ;
	//
	m_nAlphaRange = 0x100 ;
	m_nBlendDegree = 0 ;
	//
	m_nActionType = actNormal ;
	m_fEnableFading = false ;
	m_fEnableMoving = false ;
	m_fCameraMoving = false ;
	m_nDurationTime = 0 ;
	m_hDoneEvent = NULL ;
	//
	m_pToneFilter = NULL ;
	//
	m_fdwFormatImage = (DWORD) -1 ;
	m_dwWidthImage = 0 ;
	m_dwHeightImage = 0 ;
	m_nFrameNum = -1 ;
	//
	m_ppis = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSSprite::~ECSSprite( void )
{
	ESpriteServer *	pParent
		= ESLTypeCast<ESpriteServer>( GetParent() ) ;
	if ( pParent != NULL )
	{
		Lock( ) ;
		SetVisible( false ) ;
		pParent->DetachSprite( this ) ;
		Unlock( ) ;
	}
	RemoveAllSkinItems( ) ;
//	if ( m_wstrPageID.IsEmpty() )
	{
		DetachAllSprite( ) ;
	}
	//
	if ( m_ppis != NULL )
	{
		::eslHeapFree( NULL, m_ppis, 0 ) ;
	}
	if ( m_pRsrc == (EGLImage*) this )
	{
		m_pRsrc = NULL ;
	}
}

// スプライトを追加
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::AddSprite( int nPriority, ESprite * pSprite )
{
	ESpriteInterface *	pItem = ESLTypeCast<ESpriteInterface>( pSprite ) ;
	if ( (pItem != NULL) && (pItem->GetFunctionFlags() & ffTabStop) )
	{
		SetFunctionFlags( GetFunctionFlags() | ffTabStop ) ;
	}
	EAnimationSprite::AddSprite( nPriority, pSprite ) ;
}

// パラメータ複製
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::CopyParameters( const EImageSprite * pSrc )
{
	EAnimationSprite::CopyParameters( pSrc ) ;
	//
	const ECSSprite *
		pSprSrc = ESLTypeCast<ECSSprite,EImageSprite>( pSrc ) ;
	if ( pSprSrc != NULL )
	{
		m_nAlphaRange = pSprSrc->m_nAlphaRange ;
		m_nBlendDegree = pSprSrc->m_nBlendDegree ;
		//
		m_fEnableFading = pSprSrc->m_fEnableFading ;
		if ( m_fEnableFading )
		{
			m_bzDegreeCurve = pSprSrc->m_bzDegreeCurve ;
			m_bzColorFade = pSprSrc->m_bzColorFade ;
		}
		m_fEnableMoving = pSprSrc->m_fEnableMoving ;
		if ( m_fEnableMoving )
		{
			m_bzPositionCurves = pSprSrc->m_bzPositionCurves ;
			m_bzRevolutionCurves = pSprSrc->m_bzRevolutionCurves ;
			m_bzMagnificationCurves = pSprSrc->m_bzMagnificationCurves ;
		}
		m_fCameraMoving = pSprSrc->m_fCameraMoving ;
		if ( m_fCameraMoving )
		{
			m_bzCameraPos = pSprSrc->m_bzCameraPos ;
			m_bzCameraTarget = pSprSrc->m_bzCameraTarget ;
			m_bzRevCameraZ = pSprSrc->m_bzRevCameraZ ;
		}
		m_nActionType = pSprSrc->m_nActionType ;
		m_nDurationTime = pSprSrc->m_nDurationTime ;
		m_dwCurrentMovingTime = pSprSrc->m_dwCurrentMovingTime ;
		m_lstDurationsTime = pSprSrc->m_lstDurationsTime ;
	}
}

// リソース取得
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO ECSSprite::GetImageInfo( void ) const
{
	if ( m_pImage != NULL )
	{
		return	m_pImage ;
	}
	return	ECSResource::GetImageInfo() ;
}

// 画像バッファ作成
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO ECSSprite::CreateImage
		( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
				DWORD dwBitsPerPixel, DWORD dwFlags )
{
	PEGL_IMAGE_INFO	pImage =
		EAnimationSprite::CreateImage
			( fdwFormat, dwWidth, dwHeight, dwBitsPerPixel, dwFlags ) ;
	if ( pImage != NULL )
	{
		m_wstrFileName.FreeString() ;
		m_fOwnRsrc = rofNothing ;
		m_pRsrc = (EGLImage*) this ;
	}
	return	pImage ;
}

// 合成マスク設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::SetAlphaImage
	( PEGL_IMAGE_INFO pBlendAlpha, unsigned int nAlphaRange )
{
	m_imgBlendAlpha.AttachImage( pBlendAlpha ) ;
	m_nAlphaRange = nAlphaRange ;
	//
	if ( (m_imgBlendAlpha.GetInfo() != NULL) && (m_nAlphaRange <= 0x100) )
	{
		PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
		PEGL_IMAGE_INFO	pAlpha = m_imgBlendAlpha.GetInfo( ) ;
		if ( (pImage == NULL)
			|| (pImage->dwImageWidth != pAlpha->dwImageWidth)
			|| (pImage->dwImageHeight != pAlpha->dwImageHeight)
			|| (pImage->dwBitsPerPixel != 32) )
		{
			CreateImage
				( EIF_RGBA_BITMAP,
					pAlpha->dwImageWidth, pAlpha->dwImageHeight, 32, 0 ) ;
		}
		else
		{
			pImage->fdwFormatType = EIF_RGBA_BITMAP ;
		}
	}
	UpdateRect( NULL ) ;
	return	eslErrSuccess ;
}

// 現在の合成マスクの度合いを取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSSprite::GetBlendDegree( void )
{
	if ( (m_imgBlendAlpha.GetInfo() == NULL) || (m_nAlphaRange >= 0x100) )
	{
		return	GetTransparency( ) ;
	}
	else
	{
		return	m_nBlendDegree ;
	}
}

// 合成マスクの度合いを設定
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::SetBlendDegree( unsigned int nDegree )
{
	if ( (m_imgBlendAlpha.GetInfo() == NULL) || (m_nAlphaRange >= 0x100) )
	{
		SetTransparency( nDegree ) ;
		m_nBlendDegree = nDegree ;
	}
	else
	{
		m_nBlendDegree = nDegree ;
		UpdateRect( NULL ) ;
	}
}

// スプライト表示アニメーションを指定の位置で切断し、後半だけ残す
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::DivideActivation( SDWORD dwOffsetTime )
{
	if ( (m_nDurationTime > 0) && (dwOffsetTime > 0) )
	{
		if ( (DWORD) dwOffsetTime > (DWORD) m_nDurationTime )
		{
			dwOffsetTime = m_nDurationTime ;
		}
		m_dwCurrentMovingTime = 0 ;
		//
		double	t = (double) dwOffsetTime / m_nDurationTime ;
		//
		if ( m_fEnableFading )
		{
			EBezierCurves<double>	bzTempFirst, bzTempLast ;
			EBezierCurves<EGL_PALETTE>	bzFadeFirst, bzFadeLast ;
			m_bzDegreeCurve.DivideBezier( t, bzTempFirst, bzTempLast ) ;
			m_bzColorFade.DivideBezier( t, bzFadeFirst, bzFadeLast ) ;
			m_bzDegreeCurve = bzTempLast ;
			m_bzColorFade = bzFadeLast ;
		}
		if ( m_fEnableMoving )
		{
			EBezierCurves<E3D_VECTOR>		bzPosFirst, bzPosLast ;
			EBezierCurves<double>			bzRevFirst, bzRevLast ;
			EBezierCurves<E3D_VECTOR_2D>	bzMagFirst, bzMagLast ;
			m_bzPositionCurves.DivideBezier( t, bzPosFirst, bzPosLast ) ;
			m_bzRevolutionCurves.DivideBezier( t, bzRevFirst, bzRevLast ) ;
			m_bzMagnificationCurves.DivideBezier( t, bzMagFirst, bzMagLast ) ;
			m_bzPositionCurves = bzPosLast ;
			m_bzRevolutionCurves = bzRevLast ;
			m_bzMagnificationCurves = bzMagLast ;
		}
		if ( m_fCameraMoving )
		{
			EBezierCurves<E3D_VECTOR>		bzPosFirst, bzPosLast ;
			EBezierCurves<E3D_VECTOR>		bzTargetFirst, bzTargetLast ;
			EBezierCurves<double>			bzRevFirst, bzRevLast ;
			m_bzCameraPos.DivideBezier( t, bzPosFirst, bzPosLast ) ;
			m_bzCameraTarget.DivideBezier( t, bzTargetFirst, bzTargetLast ) ;
			m_bzRevCameraZ.DivideBezier( t, bzRevFirst, bzRevLast ) ;
			m_bzCameraPos = bzPosLast ;
			m_bzCameraTarget = bzTargetLast ;
			m_bzRevCameraZ = bzRevLast ;
		}
	}
}

// フェード処理設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::SetBlendingEnvelope
	( unsigned int nTargetDegree )
{
	if ( (m_nDurationTime > 0) && (m_nActionType == actNormal) )
	{
		m_fEnableFading = false ;
		DivideActivation( m_dwCurrentMovingTime ) ;
	}
	double	rStartDegree = (double) GetBlendDegree() / 256.0 ;
	double	rTargetDegree = (double) nTargetDegree / 256.0 ;
	double	rDelta = (rTargetDegree - rStartDegree) / 3.0 ;
	m_bzDegreeCurve.SetCount( 4 ) ;
	m_bzDegreeCurve[0] = rStartDegree ;
	m_bzDegreeCurve[1] = rStartDegree + rDelta ;
	m_bzDegreeCurve[2] = rTargetDegree - rDelta ;
	m_bzDegreeCurve[3] = rTargetDegree ;
	m_fEnableFading = true ;
	return	eslErrSuccess ;
}

ESLError ECSSprite::SetBlendingEnvelope
	( const EBezierCurves<double> & bzFading )
{
	return	SetFadeEnvelope( &bzFading, NULL ) ;
}

ESLError ECSSprite::SetFadeEnvelope
	( const EBezierCurves<double> * pbzFade,
		const EBezierCurves<EGL_PALETTE> * pbzColor )
{
	if ( (m_nDurationTime > 0) && (m_nActionType == actNormal) )
	{
//		m_fEnableFading = false ;
		DivideActivation( m_dwCurrentMovingTime ) ;
	}
	if ( pbzFade != NULL )
	{
		m_bzDegreeCurve = *pbzFade ;
	}
	if ( pbzColor != NULL )
	{
		m_bzColorFade = *pbzColor ;
	}
	m_fEnableFading = true ;
	return	eslErrSuccess ;
}

// 移動パラメータ設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::SetBezierCurve
	( const EBezierCurves<E3D_VECTOR> * pbzCurve,
		const EBezierCurves<double> * pbzRev,
		const EBezierCurves<E3D_VECTOR_2D> * pbzMagnify )
{
	if ( (m_nDurationTime > 0) && (m_nActionType == actNormal) )
	{
		DivideActivation( m_dwCurrentMovingTime ) ;
	}
	//
	if ( pbzCurve != NULL )
	{
		m_bzPositionCurves = *pbzCurve ;
	}
	if ( pbzRev != NULL )
	{
		m_bzRevolutionCurves = *pbzRev ;
	}
	if ( pbzMagnify != NULL )
	{
		m_bzMagnificationCurves = *pbzMagnify ;
	}
	m_fEnableMoving = true ;
	return	eslErrSuccess ;
}

ESLError ECSSprite::SetBezierCurve
	( const EBezierCurves<E3D_VECTOR_2D> * pbzCurve,
		const EBezierCurves<double> * pbzRev,
		const EBezierCurves<E3D_VECTOR_2D> * pbzMagnify )
{
	EBezierCurves<E3D_VECTOR>	bzCurve3D ;
	EBezierCurves<E3D_VECTOR> *	pbzCurve3D = NULL ;
	if ( pbzCurve != NULL )
	{
		PARAMETER	param ;
		GetParameter( param ) ;
		//
		int	i, nCount ;
		nCount = pbzCurve->GetCount() ;
		//
		bzCurve3D.SetCount( nCount ) ;
		//
		for ( i = 0; i < nCount; i ++ )
		{
			bzCurve3D[i].x = (*pbzCurve)[i].x ;
			bzCurve3D[i].y = (*pbzCurve)[i].y ;
			bzCurve3D[i].z = param.rZOrder ;
		}
		pbzCurve3D = &bzCurve3D ;
	}
	return	SetBezierCurve( pbzCurve3D, pbzRev, pbzMagnify ) ;
}

// カメラアニメーション設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::SetCameraCurve
	( const EBezierCurves<E3D_VECTOR> & bzCamera,
		const EBezierCurves<E3D_VECTOR> & bzTarget,
		const EBezierCurves<double> & bzRevAngle )
{
	if ( (m_nDurationTime > 0) && (m_nActionType == actNormal) )
	{
		DivideActivation( m_dwCurrentMovingTime ) ;
	}
	m_bzCameraPos = bzCamera ;
	m_bzCameraTarget = bzTarget ;
	m_bzRevCameraZ = bzRevAngle ;
	m_fCameraMoving = true ;
	return	eslErrSuccess ;
}

// フェード処理・移動処理開始
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::BeginActivation
	( const unsigned int nDurationTime[],
		int nDurationCount, HANDLE hDoneEvent, int nActionType )
{
	m_nDurationTime = 0 ;
	m_lstDurationsTime.SetSize( nDurationCount ) ;
	for ( int i = 0; i < nDurationCount; i ++ )
	{
		m_nDurationTime += nDurationTime[i] ;
		m_lstDurationsTime.SetAt( i, nDurationTime[i] ) ;
	}
	//
	m_dwCurrentMovingTime = 0 ;
	m_nActionType = nActionType ;
	m_hDoneEvent = hDoneEvent ;
	//
	if ( m_nDurationTime == 0 )
	{
		m_nDurationTime = 1 ;
		return	FlushActivation( ) ;
	}
	else if ( m_fEnableFading || m_fEnableMoving || m_fCameraMoving )
	{
		PARAMETER	param ;
		GetParameter( param ) ;
		if ( m_fEnableFading )
		{
			if ( m_bzDegreeCurve.GetCount() )
			{
				SetBlendDegree( ::eriRoundR32ToInt
						( (REAL32) (m_bzDegreeCurve[0] * 0x100) ) ) ;
				GetParameter( param ) ;
			}
			if ( m_bzColorFade.GetCount() )
			{
				param.rgbColorParam1 = m_bzColorFade[0] ;
			}
		}
		if ( m_fEnableMoving )
		{
			if ( m_bzPositionCurves.GetCount() )
			{
				if ( param.dwFlags & EGL_FIXED_POSITION )
				{
					param.ptDstPos.x = ::eriRoundR32ToInt
						( (REAL32) (m_bzPositionCurves[0].x * 0x10000) ) ;
					param.ptDstPos.y = ::eriRoundR32ToInt
						( (REAL32) (m_bzPositionCurves[0].y * 0x10000) ) ;
				}
				else
				{
					param.ptDstPos.x =
						::eriRoundR32ToInt( m_bzPositionCurves[0].x ) ;
					param.ptDstPos.y =
						::eriRoundR32ToInt( m_bzPositionCurves[0].y ) ;
				}
				param.rZOrder = m_bzPositionCurves[0].z ;
			}
			if ( m_bzRevolutionCurves.GetCount() )
			{
				param.rRevAngle = (REAL32) m_bzRevolutionCurves[0] ;
			}
			if ( m_bzMagnificationCurves.GetCount() )
			{
				param.rHorzUnit = m_bzMagnificationCurves[0].x ;
				param.rVertUnit = m_bzMagnificationCurves[0].y ;
			}
		}
		SetParameter( param ) ;
		//
		if ( m_fCameraMoving )
		{
			if ( m_bzCameraPos.GetCount()
				&& m_bzCameraTarget.GetCount()
				&& m_bzRevCameraZ.GetCount() )
			{
				Set3DViewCamera
					( m_bzCameraPos[0],
						m_bzCameraTarget[0], m_bzRevCameraZ[0] ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// フェード処理を即時完了させる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::FlushActivation( void )
{
	if ( m_nDurationTime > 0 )
	{
		if ( m_fEnableFading )
		{
			int	nCount = m_bzDegreeCurve.GetCount( ) ;
			if ( nCount > 0 )
			{
				SetBlendDegree( ::eriRoundR32ToInt
					( (REAL32) (m_bzDegreeCurve[nCount - 1] * 0x100) ) ) ;
				m_bzDegreeCurve.SetCount( 0 ) ;
			}
			nCount = m_bzColorFade.GetCount( ) ;
			if ( nCount > 0 )
			{
				PARAMETER	param ;
				GetParameter( param ) ;
				param.rgbColorParam1 = m_bzColorFade[nCount - 1] ;
				SetParameter( param ) ;
				m_bzColorFade.SetCount( 0 ) ;
			}
			m_fEnableFading = false ;
		}
		if ( m_fEnableMoving )
		{
			PARAMETER	param ;
			GetParameter( param ) ;
			//
			int	nCount = m_bzPositionCurves.GetCount( ) ;
			if ( nCount > 0 )
			{
				if ( param.dwFlags & EGL_FIXED_POSITION )
				{
					param.ptDstPos.x = ::eriRoundR32ToInt
						( (REAL32) (m_bzPositionCurves[nCount - 1].x * 0x10000) ) ;
					param.ptDstPos.y = ::eriRoundR32ToInt
						( (REAL32) (m_bzPositionCurves[nCount - 1].y * 0x10000) ) ;
				}
				else
				{
					param.ptDstPos.x =
						::eriRoundR32ToInt( m_bzPositionCurves[nCount - 1].x ) ;
					param.ptDstPos.y =
						::eriRoundR32ToInt( m_bzPositionCurves[nCount - 1].y ) ;
				}
				param.rZOrder = m_bzPositionCurves[nCount - 1].z ;
				m_bzPositionCurves.SetCount( 0 ) ;
			}
			nCount = m_bzRevolutionCurves.GetCount( ) ;
			if ( nCount > 0 )
			{
				param.rRevAngle = (REAL32) m_bzRevolutionCurves[nCount - 1] ;
				m_bzRevolutionCurves.SetCount( 0 ) ;
			}
			nCount = m_bzMagnificationCurves.GetCount( ) ;
			if ( nCount > 0 )
			{
				param.rHorzUnit = m_bzMagnificationCurves[nCount - 1].x ;
				param.rVertUnit = m_bzMagnificationCurves[nCount - 1].y ;
				m_bzMagnificationCurves.SetCount( 0 ) ;
			}
			//
			SetParameter( param ) ;
			m_fEnableMoving = false ;
		}
		if ( m_fCameraMoving )
		{
			if ( m_bzCameraPos.GetCount()
				|| m_bzCameraTarget.GetCount()
				|| m_bzRevCameraZ.GetCount() )
			{
				int	i, j, k ;
				i = m_bzCameraPos.GetCount() - 1 ;
				j = m_bzCameraTarget.GetCount() - 1 ;
				k = m_bzRevCameraZ.GetCount() - 1 ;
				//
				Set3DViewCamera
					( m_bzCameraPos[i],
						m_bzCameraTarget[j], m_bzRevCameraZ[k] ) ;
				UpdateRect( NULL ) ;
			}
			m_fCameraMoving = false ;
		}
		if ( m_hDoneEvent != NULL )
		{
			::SetEvent( m_hDoneEvent ) ;
			m_hDoneEvent = NULL ;
		}
		m_nDurationTime = 0 ;
		m_nActionType = actNormal ;
	}
	return	eslErrSuccess ;
}

// フェード処理をキャンセルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::CancelActivation( void )
{
	if ( m_nDurationTime > 0 )
	{
		m_nDurationTime = 0 ;
		m_fEnableFading = false ;
		m_fEnableMoving = false ;
		m_fCameraMoving = false ;
		if ( m_hDoneEvent != NULL )
		{
			::SetEvent( m_hDoneEvent ) ;
			m_hDoneEvent = NULL ;
		}
	}
	return	eslErrSuccess ;
}

// フェード処理中か？
//////////////////////////////////////////////////////////////////////////////
bool ECSSprite::IsActivation( void ) const
{
	return	(m_nDurationTime > 0) ;
}

// トーンフィルタを関連付ける
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::AttachToneFilter( ECSToneFilter * pFilter )
{
	m_pToneFilter = pFilter ;
	m_refToneFilter.SetReference( pFilter ) ;
	UpdateRect( NULL ) ;
}

// 画像ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::LoadImageFile
	( const wchar_t * pwszFileName, ECSContext * pContext )
{
	Lock( ) ;
	UpdateRect( NULL ) ;
	EAnimationSprite::DeleteImage( ) ;
	Unlock( ) ;
	//
	ESLError	err = ECSResource::LoadImageFile( pwszFileName, pContext ) ;
	if ( !err )
	{
		EGLAnimation *	pImage = ESLTypeCast<EGLAnimation>( GetResource() ) ;
		if ( pImage != NULL )
		{
			Lock( ) ;
			m_nFrameNum = -1 ;
			CreateAnimation( pImage ) ;
			Unlock( ) ;
		}
	}
	return	err ;
}

// 画像バッファ消去
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Release( void )
{
	bool	fLock = (GetParent() != NULL) ;
	if ( fLock )
	{
		Lock( ) ;
	}
	RemoveAllSkinItems( ) ;
//	DetachAllSprite( ) ;
	EAnimationSprite::DeleteImage( ) ;
	ECSResource::Release( ) ;
	m_imgBlendAlpha.DeleteImage( ) ;
	m_wstrPageID.FreeString( ) ;
	//
	m_fdwFormatImage = (DWORD) -1 ;
	m_dwWidthImage = 0 ;
	m_dwHeightImage = 0 ;
	//
	m_refImage.SetReference( NULL ) ;
	m_refAlphaImage.SetReference( NULL ) ;
	m_refRsrcManager.SetReference( NULL ) ;
	m_hashItemStatus.m_varArray.RemoveAll( ) ;
	if ( fLock )
	{
		Unlock( ) ;
	}
	return	eslErrSuccess ;
}

// スレッド同期
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Lock( DWORD dwTimeout )
{
	if ( m_pMainWnd != NULL )
	{
		return	m_pMainWnd->Lock( dwTimeout ) ;
	}
	return	eslErrTimeout ;
}

ESLError ECSSprite::Unlock( void )
{
	if ( m_pMainWnd != NULL )
	{
		m_pMainWnd->Unlock( ) ;
		return	eslErrSuccess ;
	}
	return	eslErrSuccess ;
}

// スキンアイテムを削除する
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::RemoveAllSkinItems( void )
{
	for ( unsigned int i = 0; i < GetSpriteCount(); i ++ )
	{
		ESprite *	pChild = GetSpriteAt( i ) ;
		if ( !pChild->IsKindOf( ESL_RUNTIME_CLASS(ECSObject) ) )
		{
			RemoveSprite( pChild ) ;
			i -- ;
		}
	}
	SetFunctionFlags( GetFunctionFlags() & ~ffTabStop ) ;
}

// 指定された識別子のアイテムを取得
//////////////////////////////////////////////////////////////////////////////
ESpriteInterface *
	ECSSprite::GetSpriteItemAs
			( const wchar_t * pwszID, bool fChild )
{
	if ( (pwszID == NULL) || (pwszID[0] == L'\0') )
	{
		return	this ;
	}
	if ( !fChild )
	{
		return	ESpriteInterface::GetSpriteItemAs( pwszID, fChild ) ;
	}
	ESpriteInterface *	pParent = this ;
	EWideString			wstrID = pwszID ;
	int					iLast = 0 ;
	for ( ; ; )
	{
		int		i = wstrID.Find( '\\', iLast ) ;
		if ( i < 0 )
		{
			return	pParent->GetSpriteItemAs( wstrID.Middle(iLast), fChild ) ;
		}
		pParent = pParent->GetSpriteItemAs
			( wstrID.Middle( iLast, i - iLast ), fChild ) ;
		if ( pParent == NULL )
			break ;
		iLast = i + 1 ;
	}
	return	NULL ;
}

// 動的スプライトモード化取得
//////////////////////////////////////////////////////////////////////////////
bool ECSSprite::IsDynamicSpriteMode( void )
{
	return	EAnimationSprite::IsDynamicSpriteMode()
				&& ((m_pImage == NULL) | (m_nBlendDegree == 0)) ;
}

// 陰になる内接（最大）矩形取得
//////////////////////////////////////////////////////////////////////////////
bool ECSSprite::GetHiddenRectangle( EGL_RECT & rect )
{
	if ( m_nBlendDegree > 0 )
	{
		return	false ;
	}
	return	EAnimationSprite::GetHiddenRectangle( rect ) ;
}

// スプライト描画
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::MTDraw( HEGL_RENDER_POLYGON hRenderPoly )
{
#if	!defined(_DEBUG)
	try
#endif
	{
		EAnimationSprite::MTDraw( hRenderPoly ) ;
	}
#if	!defined(_DEBUG)
	catch ( ... )
	{
		EString	strErrMsg =
			"ECSSprite::Draw 関数で例外エラーが発生しました。(" ;
		strErrMsg += EString( (DWORD) this ) ;
		strErrMsg += ")\n" ;
		::OutputDebugString( strErrMsg ) ;
		//
		AttachImage( NULL ) ;
	}
#endif
}

// 更新領域を再描画
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::RefreshRect( const EGL_RECT & rectRefresh )
{
	EAnimationSprite::RefreshRect( rectRefresh ) ;
	RefreshRectPostFilter( rectRefresh ) ;
}

// 後処理フィルター
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::RefreshRectPostFilter( const EGL_RECT & rectRefresh )
{
	//
	// トーンフィルタ
	//
	PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
	if ( pImage == NULL )
	{
		return ;
	}
	EGL_IMAGE_INFO	eiiImage ;
	EGLImageRect	rectClip = rectRefresh ;
	if ( ::eglGetClippedImageInfo( &eiiImage, pImage, &rectClip ) )
	{
		return ;
	}
	if ( m_pToneFilter->IsValidObject() )
	{
		m_pToneFilter->ApplyToneFilter( &eiiImage ) ;
	}
	//
	// αチャネル合成
	//
	PEGL_IMAGE_INFO	pAlpha = m_imgBlendAlpha.GetInfo( ) ;
	if ( (pAlpha == NULL)
		|| (m_nAlphaRange >= 0x100) || (m_nAlphaRange <= 0) )
	{
		return ;
	}
	if ( m_nBlendDegree <= 0 )
	{
		return ;
	}
	EGL_IMAGE_INFO	eiiAlpha ;
	if ( ::eglGetClippedImageInfo( &eiiAlpha, pAlpha, &rectClip ) )
	{
		return ;
	}
	if ( (eiiImage.dwImageWidth != eiiAlpha.dwImageWidth)
		|| (eiiImage.dwImageHeight != eiiAlpha.dwImageHeight) )
	{
		return ;
	}
	const DWORD	dwFlags =
		EGL_BAC_MULTIPLY | EGL_BAC_ADD_ALPHA | EGL_BAC_MULTIPLY_ALPHA ;
	SDWORD	nAlphaRange = m_nAlphaRange * 0x10 ;
	SDWORD	nAlphaBase =
		0x100 - (nAlphaRange * 0x10 + 0x100) * m_nBlendDegree / 0x100 ;
	::eglBlendAlphaChannel
		( &eiiImage, &eiiImage, &eiiAlpha, dwFlags, nAlphaBase, nAlphaRange ) ;
}

// アニメーション進行
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::OnAdvanceAnimation( unsigned int nPastTime )
{
	if ( m_refTimerProcedure.m_pRef->IsValidObject() )
	{
		int	nArg[1] = { (int) nPastTime } ;
		CallVoidScriptCallback
			( m_refTimerProcedure, virtualOnTimer, nArg, 1 ) ;
	}
	if ( m_nDurationTime > 0 )
	{
		m_dwCurrentMovingTime += nPastTime ;
		//
		double	t = 0 ;
		SDWORD	nTime = m_dwCurrentMovingTime ;
		if ( m_nDurationTime != 0 )
		{
			if ( m_nActionType == actLoop )
			{
				nTime %= m_nDurationTime ;
				m_dwCurrentMovingTime = nTime ;
			}
			else if ( m_nActionType == actTurnLoop )
			{
				int	nTurn = nTime / m_nDurationTime ;
				nTime %= m_nDurationTime ;
				if ( !(nTurn & 0x01) )
				{
					m_dwCurrentMovingTime = nTime ;
				}
				else
				{
					m_dwCurrentMovingTime = m_nDurationTime + nTime ;
					nTime = m_nDurationTime - nTime ;
				}
			}
		}
		if ( !(m_dwAnimationFlags & animeAction)
					|| (nTime > m_nDurationTime) )
		{
			nTime = m_nDurationTime ;
		}
		int		i, nCount = m_lstDurationsTime.GetSize( ) ;
		if ( nTime < m_nDurationTime )
		{
			SDWORD	nDeltaTime = nTime ;
			for ( i = 0; i < nCount; i ++ )
			{
				SDWORD	dwDuration = m_lstDurationsTime[i] ;
				if ( nDeltaTime < dwDuration )
				{
					t = ((double) nDeltaTime / dwDuration + i) / nCount ;
					break ;
				}
				nDeltaTime -= dwDuration ;
			}
		}
		else
		{
			t = 1.0 ;
		}
		if ( m_fEnableFading )
		{
			if ( m_bzDegreeCurve.GetCount() )
			{
				double	p = 0 ;
				SDWORD	nDegree =
					::eriRoundR32ToInt
						( (REAL32) (m_bzDegreeCurve.pt( p, t ) * 0x100) ) ;
				if ( nDegree != (SDWORD) GetBlendDegree() )
				{
					SetBlendDegree( nDegree ) ;
				}
			}
			if ( m_bzColorFade.GetCount() )
			{
				PARAMETER	param ;
				EGL_PALETTE	rgbEffect ;
				GetParameter( param ) ;
				if ( param.rgbColorParam1 != m_bzColorFade.pt( rgbEffect, t ) )
				{
					param.rgbColorParam1 = rgbEffect ;
					SetParameter( param ) ;
				}
			}
		}
		if ( m_fEnableMoving )
		{
			PARAMETER	param ;
			EGL_POINT	ptDstPos ;
			REAL32		zPos ;
			E3DVector2D	vMagnify( 0, 0 ) ;
			double		rRevAngle = 0 ;
			GetParameter( param ) ;
			if ( m_bzPositionCurves.GetCount() )
			{
				E3DVector	vPos( 0, 0, 0 ) ;
				m_bzPositionCurves.pt( vPos, t ) ;
				//
				if ( param.dwFlags & EGL_FIXED_POSITION )
				{
					vPos.x *= 0x10000 ;
					vPos.y *= 0x10000 ;
				}
				ptDstPos.x = ::eriRoundR32ToInt( vPos.x ) ;
				ptDstPos.y = ::eriRoundR32ToInt( vPos.y ) ;
				zPos = vPos.z ;
			}
			else
			{
				ptDstPos = param.ptDstPos ;
				zPos = param.rZOrder ;
			}
			if ( m_bzRevolutionCurves.GetCount() )
			{
				m_bzRevolutionCurves.pt( rRevAngle, t ) ;
			}
			else
			{
				rRevAngle = param.rRevAngle ;
			}
			if ( m_bzMagnificationCurves.GetCount() )
			{
				m_bzMagnificationCurves.pt( vMagnify, t ) ;
			}
			else
			{
				vMagnify.x = param.rHorzUnit ;
				vMagnify.y = param.rVertUnit ;
			}
			if ( (ptDstPos.x != param.ptDstPos.x)
				| (ptDstPos.y != param.ptDstPos.y)
				| (zPos != param.rZOrder)
				| (rRevAngle != param.rRevAngle)
				| (vMagnify.x != param.rHorzUnit)
				| (vMagnify.y != param.rVertUnit) )
			{
				param.ptDstPos = ptDstPos ;
				param.rZOrder = zPos ;
				param.rRevAngle = (REAL32) rRevAngle ;
				param.rHorzUnit = vMagnify.x ;
				param.rVertUnit = vMagnify.y ;
				SetParameter( param ) ;
			}
		}
		if ( m_fCameraMoving )
		{
			E3D_VECTOR	vPos = m_bzCameraPos.pt( t ) ;
			E3D_VECTOR	vTarget = m_bzCameraTarget.pt( t ) ;
			double		rRevZ = m_bzRevCameraZ.pt( t ) ;
			//
			Set3DViewCamera( vPos, vTarget, rRevZ ) ;
		}
		if ( (m_nActionType == actNormal)
					&& (nTime >= m_nDurationTime) )
		{
			if ( m_hDoneEvent != NULL )
			{
				::SetEvent( m_hDoneEvent ) ;
			}
			m_fEnableFading = false ;
			m_fEnableMoving = false ;
			m_fCameraMoving = false ;
			m_nDurationTime = 0 ;
			m_hDoneEvent = NULL ;
			//
			m_bzDegreeCurve.SetCount( 0 ) ;
			m_bzColorFade.SetCount( 0 ) ;
			m_bzPositionCurves.SetCount( 0 ) ;
			m_bzRevolutionCurves.SetCount( 0 ) ;
			m_bzMagnificationCurves.SetCount( 0 ) ;
			m_bzCameraPos.SetCount( 0 ) ;
			m_bzCameraTarget.SetCount( 0 ) ;
			m_bzRevCameraZ.SetCount( 0 ) ;
			m_lstDurationsTime.SetSize( 0 ) ;
		}
	}
	return	EAnimationSprite::OnAdvanceAnimation( nPastTime ) ;
}

// 効果音を再生する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::PlaySoundEffect
	( const wchar_t * pwszID, bool fRepeat )
{
	ECSResourceManager *	pRsrcManager =
		ESLTypeCast<ECSResourceManager>( m_refRsrcManager.m_pRef ) ;
	if ( pRsrcManager != NULL )
	{
		EWaveSound *	pSound =
			ESLTypeCast<EWaveSound>( pRsrcManager->GetResourceAs( pwszID ) ) ;
		if ( pSound != NULL )
		{
			if ( GetWaveOutDevice() != NULL )
			{
				pSound->AttachWaveDevice( GetWaveOutDevice() ) ;
				pSound->SetVolume
					( m_rTotalVol[ptfSystem], m_rTotalVol[ptfSystem] ) ;
				pSound->PlayWave( fRepeat ) ;
				return	eslErrSuccess ;
			}
		}
	}
	return	EAnimationSprite::PlaySoundEffect( pwszID, fRepeat ) ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool ECSSprite::IsHitSprite( int xPos, int yPos )
{
	if ( m_refHitTestProcedure.m_pRef->IsValidObject() )
	{
		int	nArg[2] = { xPos, yPos } ;
		return	CallBooleanScriptCallback
			( m_refHitTestProcedure, virtualIsHitSprite, nArg, 2 ) ;
	}
	return	EAnimationSprite::IsHitSprite( xPos, yPos ) ;
}

// メッセージ処理（マウスが上に乗っているかキャプチャーしているもののみ）
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::OnMouseMove( UINT nFlags, int xPos, int yPos )
{
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[2] = { xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnMouseMove, nArg, 2 ) )
		{
			return ;
		}
	}
	EAnimationSprite::OnMouseMove( nFlags, xPos, yPos ) ;
}

void ECSSprite::OnMouseLeave( UINT nFlags, int xPos, int yPos )
{
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[2] = { xPos, yPos } ;
		CallVoidScriptCallback
			( m_refMouseInterface, virtualOnMouseLeave, nArg, 0 ) ;
	}
	EAnimationSprite::OnMouseLeave( nFlags, xPos, yPos ) ;
}

bool ECSSprite::OnMouseWheel
	( UINT nFlags, short int zDelta, int xPos, int yPos )
{
	if ( EAnimationSprite::OnMouseWheel( nFlags, zDelta, xPos, yPos ) )
	{
		return	true ;
	}
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[3] = { zDelta / WHEEL_DELTA, xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnMouseWheel, nArg, 3 ) )
		{
			return	true ;
		}
	}
	return	false ;
}

bool ECSSprite::OnLButtonDown( UINT nFlags, int xPos, int yPos )
{
	if ( EAnimationSprite::OnLButtonDown( nFlags, xPos, yPos ) )
	{
		return	true ;
	}
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[3] = { xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnLButtonDown, nArg, 2 ) )
		{
			return	true ;
		}
	}
	return	false ;
}

bool ECSSprite::OnLButtonUp( UINT nFlags, int xPos, int yPos )
{
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[3] = { xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnLButtonUp, nArg, 2 ) )
		{
			return	true ;
		}
	}
	if ( EAnimationSprite::OnLButtonUp( nFlags, xPos, yPos ) )
	{
		return	true ;
	}
	return	false ;
}

bool ECSSprite::OnLButtonDblClk( UINT nFlags, int xPos, int yPos )
{
	if ( EAnimationSprite::OnLButtonDblClk( nFlags, xPos, yPos ) )
	{
		return	true ;
	}
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[3] = { xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnLButtonDblClk, nArg, 2 ) )
		{
			return	true ;
		}
	}
	return	false ;
}

bool ECSSprite::OnRButtonDown( UINT nFlags, int xPos, int yPos )
{
	if ( EAnimationSprite::OnRButtonDown( nFlags, xPos, yPos ) )
	{
		return	true ;
	}
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[3] = { xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnRButtonDown, nArg, 2 ) )
		{
			return	true ;
		}
	}
	return	false ;
}

bool ECSSprite::OnRButtonUp( UINT nFlags, int xPos, int yPos )
{
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[3] = { xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnRButtonUp, nArg, 2 ) )
		{
			return	true ;
		}
	}
	if ( EAnimationSprite::OnRButtonUp( nFlags, xPos, yPos ) )
	{
		return	true ;
	}
	return	false ;
}

bool ECSSprite::OnRButtonDblClk( UINT nFlags, int xPos, int yPos )
{
	if ( EAnimationSprite::OnRButtonDblClk( nFlags, xPos, yPos ) )
	{
		return	true ;
	}
	if ( m_refMouseInterface.m_pRef->IsValidObject() )
	{
		int	nArg[3] = { xPos, yPos } ;
		if ( CallBooleanScriptCallback
			( m_refMouseInterface, virtualOnRButtonDblClk, nArg, 2 ) )
		{
			return	true ;
		}
	}
	return	false ;
}

// メッセージ処理（フォーカスを持っているアイテムのみ）
//////////////////////////////////////////////////////////////////////////////
bool ECSSprite::MessageProc
	( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( EAnimationSprite::MessageProc( hWnd, uMsg, wParam, lParam ) )
	{
		return	true ;
	}
	bool	fKeyInput = (uMsg >= WM_KEYFIRST) && (uMsg <= WM_KEYLAST) ;
	if ( m_refKeyInterface.m_pRef->IsValidObject() )
	{
		int	nArg[1] = { (int) wParam } ;
		if ( uMsg == WM_KEYDOWN )
		{
			if ( CallBooleanScriptCallback
				( m_refKeyInterface, virtualOnKeyDown, nArg, 1 ) )
			{
				return	true ;
			}
		}
		else if ( uMsg == WM_KEYUP )
		{
			if ( CallBooleanScriptCallback
				( m_refKeyInterface, virtualOnKeyUp, nArg, 1 ) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// コールバック関数実行用コンテキスト取得
//////////////////////////////////////////////////////////////////////////////
ECSContext * ECSSprite::GetCallbackContext( void )
{
	EWindowSpriteInterface *	pwsiInterface = GetWindowInterface() ;
	if ( pwsiInterface != NULL )
	{
		ECSWindow::EInterface *	pInterface =
				ESLTypeCast<ECSWindow::EInterface>( pwsiInterface ) ;
		if ( pInterface != NULL )
		{
			ECSWindow *	pWnd = pInterface->m_pWnd ;
			if ( pWnd != NULL )
			{
				return	pWnd->GetCallbackContext() ;
			}
		}
	}
	return	NULL ;
}

// スクリプト呼び出し
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::CallVoidScriptCallback
	( ECSReference& refInterface,
		int iVirtual, const int * pArg, int nArgCount )
{
	ECSContext *	pContext = GetCallbackContext() ;
	ESLAssert( pContext != NULL ) ;
	if ( pContext != NULL )
	{
		ESLError	err =
			CallScriptCallbackIntArgs
				( *pContext, refInterface, iVirtual, pArg, nArgCount ) ;
		if ( err )
		{
			ESLTrace( "詞葉実行時例外エラー：\n%s\n", GetESLErrorMsg(err) ) ;
			//
			EString	strErrMsg = "詞葉実行時例外エラー：\r\n" ;
			strErrMsg += GetESLErrorMsg(err) ;
			//
			DWORD	dwWrittenBytes ;
			::WriteFile
				( ::GetStdHandle( STD_OUTPUT_HANDLE ),
					strErrMsg.CharPtr(), strErrMsg.GetLength(),
					&dwWrittenBytes, NULL ) ;
		}
	}
}

bool ECSSprite::CallBooleanScriptCallback
	( ECSReference& refInterface,
		int iVirtual, const int * pArg, int nArgCount )
{
	ECSContext *	pContext = GetCallbackContext() ;
	ESLAssert( pContext != NULL ) ;
	bool	fResult = false ;
	if ( pContext != NULL )
	{
		ESLError	err =
			CallScriptCallbackIntArgs
				( *pContext, refInterface, iVirtual, pArg, nArgCount ) ;
		if ( err )
		{
			ESLTrace( "詞葉実行時例外エラー：\n%s\n", GetESLErrorMsg(err) ) ;
			//
			EString	strErrMsg = "詞葉実行時例外エラー：\r\n" ;
			strErrMsg += GetESLErrorMsg(err) ;
			//
			DWORD	dwWrittenBytes ;
			::WriteFile
				( ::GetStdHandle( STD_OUTPUT_HANDLE ),
					strErrMsg.CharPtr(), strErrMsg.GetLength(),
					&dwWrittenBytes, NULL ) ;
		}
		else
		{
			ECSObject *	pObjReturn = pContext->m_pRetObj ;
			if ( pObjReturn != NULL )
			{
				int	nBoolean ;
				err = pObjReturn->OperateBoolean( nBoolean ) ;
				if ( err )
				{
					ESLTrace( "Sprite コールバック関数の返り値を"
									" Boolean 判定出来ませんでした" ) ;
				}
				else
				{
					fResult = (nBoolean != 0) ;
				}
			}
		}
	}
	return	fResult ;
}

// スクリプト呼び出し低水準関数
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::CallScriptCallbackIntArgs
	( ECSContext & context,
		ECSReference& refInterface,
		int iVirtual, const int * pArg, int nArgCount )
{
	ECS_FUNCTION_POINTER	fptr ;
	ESLError	err ;
	err = refInterface.GetFunctionPointer( context, fptr, iVirtual ) ;
	if ( err )
	{
		return	err ;
	}
	if ( fptr.m_ftType != ECS_FUNCTION_POINTER::funcScriptCall )
	{
		return	ESLErrorMsg
			( "スクリプトコールバック関数のアドレスを取得出来ませんでした" ) ;
	}
	Lock() ;
	static int	countEntered = 0 ;
	if ( countEntered == 0 )
	{
		countEntered ++ ;
		//
		ECSReference *	pRefThis = context.new_CSReference() ;
		pRefThis->SetReferenceCastInterface
				( fptr.m_castThis.pCastObject, &context, fptr.m_castThis ) ;
		ECSReference *	pRefSprite = context.new_CSReference() ;
		pRefSprite->SetReference( this, &context ) ;
		//
		ECSArray	arg ;
		arg.m_varArray.Add( pRefThis ) ;
		arg.m_varArray.Add( pRefSprite ) ;
		//
		for ( int i = 0; i < nArgCount; i ++ )
		{
			arg.m_varArray.Add( context.new_CSInteger( pArg[i] ) ) ;
		}
		//
		DWORD			ipLast = context.m_ip ;
		unsigned int	nLastStack = context.m_stack.m_varArray.GetSize() ;
		context.PushObject( context.new_CSInteger( -1 ) ) ;
		//
		err = context.CallFunction( fptr.m_varFunc.addrScript, arg.m_varArray ) ;
		//
		context.delete_CSObject( context.m_pRetObj ) ;
		context.m_pRetObj = NULL ;
		if ( !err )
		{
			ECSObject *	pRetObj = context.PopObject( ) ;
			context.m_pRetObj = pRetObj ;
			if ( context.m_stack.m_varArray.GetSize() != nLastStack )
			{
				context.m_stack.RemoveBetween( context, nLastStack ) ;
			}
		}
		else
		{
			context.m_stack.RemoveBetween( context, nLastStack ) ;
		}
		arg.RemoveBetween( context ) ;
		//
		countEntered-- ;
	}
	else
	{
		ESLTrace( "error CallScriptCallbackIntArgs %d\n", countEntered ) ;
	}
	//
	Unlock() ;
	//
	return	err ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSprite::GetTypeName( void ) const
{
	return	L"Sprite" ;
}

ECSObject * ECSSprite::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( !EWideString::Compare( pwszTypeName, L"Sprite" )
		|| !EWideString::Compare( pwszTypeName, L"Resource" ) )
	{
		return	this ;
	}
	return	NULL ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSSprite::Duplicate( void )
{
	ECSSprite *	pSprite = new ECSSprite ;
	if ( GetInfo() != NULL )
	{
		pSprite->DuplicateImage( GetInfo(), 0 ) ;
	}
	PARAMETER	param ;
	GetParameter( param ) ;
	pSprite->SetParameter( param ) ;
	return	pSprite ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Move( ECSContext & context, ECSObject * obj )
{
	ECSSprite *	pSprite =
		ESLTypeCast<ECSSprite>( ECSObject::GetEntity( obj ) ) ;
	if ( pSprite == NULL )
	{
		return	ESLErrorMsg
			( "Sprite への代入元オブジェクトが Sprite ではありません。" ) ;
	}
	if ( pSprite->GetInfo() != NULL )
	{
		DuplicateImage( pSprite->GetInfo(), 0 ) ;
	}
	PARAMETER	param ;
	pSprite->GetParameter( param ) ;
	SetParameter( param ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "Sprite の定義されていない単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "Sprite の定義されていない二項演算子です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "Sprite の定義されていない比較演算子です。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSSprite::GetVariableAt( int nIndex )
{
	if ( nIndex < 0 )
	{
		nIndex = - nIndex - 1 ;
		if ( nIndex < ECSResource::MAX_INNER_MEMBER_COUNT )
		{
			return	ECSResource::GetVariableAt( -1 - nIndex ) ;
		}
		else
		{
			ECSReference*	pMemberRef[] =
			{
				&m_refImage, &m_refAlphaImage, &m_refParent,
				&m_refRsrcManager, &m_refToneFilter,
				&m_refHitTestProcedure, &m_refTimerProcedure,
				&m_refMouseInterface, &m_refKeyInterface,
			} ;
			nIndex -= ECSResource::MAX_INNER_MEMBER_COUNT ;
			if ( nIndex < sizeof(pMemberRef)/sizeof(pMemberRef[0]) )
			{
				return	pMemberRef[nIndex] ;
			}
		}
	}
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSSprite::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		if ( ECSResource::GetFunction( context, nIndex, pwszName ) )
		{
			return	ESLErrorMsg
				( "Sprite の定義されていないメンバ関数を呼び出そうとしています。" ) ;
		}
	}
	else
	{
		nIndex += ECSResource::m_staFuncName->GetSize( ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (nIndex >= 0)
		&& (nIndex < (int) ECSResource::m_staFuncName->GetSize()) )
	{
		return	ECSResource::CallFunction( context, nIndex, lstArg ) ;
	}
	nIndex -= ECSResource::m_staFuncName->GetSize() ;
	if ( (nIndex < 0) || (nIndex >= (int) m_staFuncName->GetSize()) )
	{
		return	ESLErrorMsg
			( "Sprite の定義されていないメンバ関数を呼び出そうとしています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::IndexAllMember( void )
{
	ECSResource::IndexAllMember( ) ;
	m_refImage.IndexAllMember( ) ;
	m_refAlphaImage.IndexAllMember( ) ;
	m_refParent.IndexAllMember( ) ;
	m_refRsrcManager.IndexAllMember( ) ;
	m_hashItemStatus.IndexAllMember( ) ;
	m_refToneFilter.IndexAllMember( ) ;
	//
	m_refHitTestProcedure.IndexAllMember() ;
	m_refTimerProcedure.IndexAllMember() ;
	m_refMouseInterface.IndexAllMember() ;
	m_refKeyInterface.IndexAllMember() ;
	//
	ECSReference*	pMemberRef[] =
	{
		&m_refImage, &m_refAlphaImage, &m_refParent,
		&m_refRsrcManager, &m_refToneFilter,
		&m_refHitTestProcedure, &m_refTimerProcedure,
		&m_refMouseInterface, &m_refKeyInterface,
		NULL,
	} ;
	for ( int i = 0; pMemberRef[i] != NULL; i ++ )
	{
		pMemberRef[i]->m_pParent = this ;
		pMemberRef[i]->m_nIndex = -1 - i - ECSResource::MAX_INNER_MEMBER_COUNT ;
	}
}

// 全てのメンバ変数の参照を解消する
//////////////////////////////////////////////////////////////////////////////
void ECSSprite::CleanupAllReference( ECSContext & context )
{
	ESpriteServer *	pParent
		= ESLTypeCast<ESpriteServer>( GetParent() ) ;
	if ( pParent != NULL )
	{
		Lock( ) ;
		SetVisible( false ) ;
		pParent->DetachSprite( this ) ;
		Unlock( ) ;
	}
	ECSResource::CleanupAllReference( context ) ;
	//
	m_refImage.CleanupAllReference( context ) ;
	m_refAlphaImage.CleanupAllReference( context ) ;
	m_refParent.CleanupAllReference( context ) ;
	m_refRsrcManager.CleanupAllReference( context ) ;
	m_hashItemStatus.CleanupAllReference( context ) ;
	m_refToneFilter.CleanupAllReference( context ) ;
	//
	m_refHitTestProcedure.CleanupAllReference( context ) ;
	m_refTimerProcedure.CleanupAllReference( context ) ;
	m_refMouseInterface.CleanupAllReference( context ) ;
	m_refKeyInterface.CleanupAllReference( context ) ;
	//
	RemoveAllSkinItems( ) ;
	DetachAllSprite( ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::CommitAllReference( ECSContext & context )
{
	ESLError	err ;
	err = ECSResource::CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refImage.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refAlphaImage.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refParent.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refRsrcManager.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_hashItemStatus.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refToneFilter.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	//
	err = m_refHitTestProcedure.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refTimerProcedure.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refMouseInterface.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	err = m_refKeyInterface.CommitAllReference( context ) ;
//	if ( err )
//		return	err ;
	//
	// 画像設定取得
	//
	EGLAnimation *	pImage ;
	ECSResource *	pRsrcImage =
		ESLTypeCast<ECSResource>( m_refImage.m_pRef ) ;
	if ( pRsrcImage != NULL )
	{
		pImage = pRsrcImage->GetImage( ) ;
		if ( pImage != NULL )
		{
			m_fOwnRsrc = rofNothing ;
//			m_pRsrc = pImage ;
		}
	}
	else
	{
		pImage = GetImage( ) ;
	}
	if ( pImage != NULL )
	{
		PARAMETER	param ;
		GetParameter( param ) ;
		if ( m_nFrameNum >= 0 )
		{
			SetImageView( pImage->GetFrameAt(m_nFrameNum), &m_rectView ) ;
		}
		else
		{
			unsigned long int	nLoopCount = m_nLoopCount ;
			unsigned long int	nBeginFrame = m_nCurrentSequence ;
			unsigned long int	nAnimationTime = m_nAnimationDuration ;
			unsigned long int	nRewindSequence = m_nRewindSequence ;
			unsigned long int	nTurnSequence = m_nTurnSequence ;
			CreateAnimation( pImage ) ;
			BeginAnimation
				( nLoopCount, nBeginFrame, nAnimationTime,
							nRewindSequence, nTurnSequence ) ;
		}
		SetParameter( param ) ;
	}
	else if ( pRsrcImage != NULL )
	{
		SetImageView( pRsrcImage->GetImageInfo(), &m_rectView ) ;
	}
	//
	// スキン復元
	//
	ECSResourceManager *	prmRsrc = NULL ;
	if ( !m_wstrPageID.IsEmpty() )
	{
		ECSHash	hashItemStatus ;
		PARAMETER	param ;
		EWideString	wstrID = ESpriteInterface::ID() ;
		GetParameter( param ) ;
		hashItemStatus.CopyFrom( m_hashItemStatus ) ;
		prmRsrc = ESLTypeCast<ECSResourceManager>( m_refRsrcManager.m_pRef ) ;
		prmRsrc->CreateFormPage( *this, ECSWideString(m_wstrPageID) ) ;
		SetParameter( param ) ;
		SetID( wstrID ) ;
		m_hashItemStatus.CopyFrom( hashItemStatus ) ;
	}
	unsigned int	i ;
	for ( i = 0; i < m_hashItemStatus.m_varArray.GetSize(); i ++ )
	{
		ETaggedElement<ECSWideString,ECSObject> *	pElement ;
		pElement = m_hashItemStatus.m_varArray.GetAt( i ) ;
		if ( pElement == NULL )
			continue ;
		ECSHash *	pStatus =
			ESLTypeCast<ECSHash>( pElement->GetObject() ) ;
		if ( pStatus == NULL )
			continue ;
		ESpriteInterface *	pItem = GetSpriteItemAs( pElement->Tag() ) ;
		if ( pItem == NULL )
			continue ;
		//
		ECSString *	pstrText =
			ESLTypeCast<ECSString>( pStatus->m_varArray.GetAs( L"text" ) ) ;
		if ( pstrText != NULL )
		{
			pItem->SetSpriteText( pstrText->m_varStr ) ;
		}
		//
		ECSString *	pstrFont =
			ESLTypeCast<ECSString>( pStatus->m_varArray.GetAs( L"font" ) ) ;
		if ( pstrFont != NULL )
		{
			pItem->SetSpriteFontFace( pstrFont->m_varStr ) ;
		}
		//
		ECSString *	pstrImage =
			ESLTypeCast<ECSString>( pStatus->m_varArray.GetAs( L"image" ) ) ;
		EAnimationSprite *	pImageItem =
					ESLTypeCast<EAnimationSprite>( pItem ) ;
		if ( (pstrImage != NULL) && (pImageItem != NULL) && (prmRsrc != NULL) )
		{
			EGL_IMAGE_INFO	eiiImage ;
			EGLAnimation *	pImage =
				ESLTypeCast<EGLAnimation>
					( prmRsrc->GetResourceAs( pstrImage->m_varStr ) ) ;
			if ( pImage != NULL )
			{
				pImageItem->CreateAnimation( pImage ) ;
			}
			else if ( prmRsrc->GetStillImageResource
						( pstrImage->m_varStr, &eiiImage ) != NULL )
			{
				pImageItem->SetImageView( &eiiImage ) ;
			}
			else
			{
				pImageItem->AttachImage( NULL ) ;
			}
		}
		//
		ECSInteger *	pintTrans =
			ESLTypeCast<ECSInteger>
				( pStatus->m_varArray.GetAs( L"transparency" ) ) ;
		if ( pintTrans != NULL )
		{
			pItem->SetTransparency( pintTrans->GetInt() ) ;
		}
		//
		ECSInteger *	pintEnabled =
			ESLTypeCast<ECSInteger>
				( pStatus->m_varArray.GetAs( L"enabled" ) ) ;
		if ( pintEnabled != NULL )
		{
			pItem->Enable( pintEnabled->GetValue() != 0 ) ;
		}
		//
		ECSString *	pstrCommand =
			ESLTypeCast<ECSString>
				( pStatus->m_varArray.GetAs( L"command" ) ) ;
		if ( pstrCommand != NULL )
		{
			EDescription	dscCmd ;
			dscCmd.ReadDescription
				( EStreamWideString
					( pstrCommand->m_varStr ), dscCmd.dftXML ) ;
			pItem->SendCommand( dscCmd ) ;
		}
	}
	//
	// トーンフィルタ復元
	//
	if ( m_refToneFilter.m_pRef != NULL )
	{
		ECSToneFilter *	pToneFilter =
			ESLTypeCast<ECSToneFilter>( m_refToneFilter.m_pRef ) ;
		if ( pToneFilter != NULL )
		{
			AttachToneFilter( pToneFilter ) ;
		}
	}
	//
	// アルファチャネル合成復元
	//
	if ( m_refAlphaImage.m_pRef != NULL )
	{
		ECSResource *	pRsrcAlpha =
			ESLTypeCast<ECSResource>( m_refAlphaImage.m_pRef ) ;
		if ( pRsrcAlpha != NULL )
		{
			EGLImage *	pAlpha =
				ESLTypeCast<EGLImage>( pRsrcAlpha->GetResource() ) ;
			if ( pAlpha != NULL )
			{
				SetAlphaImage( *pAlpha, m_nAlphaRange ) ;
			}
		}
	}
	//
	// 親スプライト復元
	//
//	if ( m_nParentFlag != 0 )
	{
		ECSSprite *	pParent
			= ESLTypeCast<ECSSprite>( m_refParent.m_pRef ) ;
		if ( pParent != NULL )
		{
			pParent->Lock( ) ;
			pParent->AddSprite( GetPriority(), this ) ;
			UpdateRect( NULL ) ;
			pParent->Unlock( ) ;
		}
		else
		{
			Refresh() ;
		}
	}
/*	else if ( m_pMainSprite != NULL )
	{
		m_pMainSprite->AddSprite( GetPriority(), this ) ;
		UpdateRect( NULL ) ;
	}*/
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Save
	( ESLFileObject & file, ECSContext & context )
{
	//
	// 活動キャンセル
	//
//	if ( m_nActionType == actNormal )
//	{
//		FlushActivation( ) ;
//	}
	//
	// 親スプライト保存
	//
/*	if ( GetParent() == m_pMainSprite )
	{
		m_refParent.SetReference( NULL ) ;
		m_nParentFlag = 0 ;
	}
	else
*/	{
		ECSSprite *	pParent
			= ESLTypeCast<ECSSprite>( GetParent() ) ;
		m_refParent.SetReference( pParent, &context ) ;
		m_nParentFlag = 1 ;
	}
	file.Write( &m_nParentFlag, sizeof(m_nParentFlag) ) ;
	//
	ESLError	err = m_refParent.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スプライトの生成パラメータ保存
	//
	file.Write( &m_fdwFormatImage, sizeof(DWORD) ) ;
	file.Write( &m_dwWidthImage, sizeof(DWORD) ) ;
	file.Write( &m_dwHeightImage, sizeof(DWORD) ) ;
	//
	EGLImage &	imgZBuf = GetZBuffer( ) ;
	DWORD	fdwZBuffer = 0 ;
	if ( imgZBuf.GetInfo() != NULL )
	{
		fdwZBuffer = imgZBuf.GetInfo()->fdwFormatType ;
	}
	file.Write( &fdwZBuffer, sizeof(DWORD) ) ;
	//
	// スキンページ参照保存
	//
	DWORD	dwLength ;
	dwLength = m_wstrPageID.GetLength( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	if ( dwLength > 0 )
	{
		file.Write( m_wstrPageID.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	}
	err = m_refRsrcManager.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// リソース保存
	//
	err = ECSResource::Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// アイテムの設定履歴保存
	//
	err = m_hashItemStatus.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// トーンフィルタ関連付け保存
	//
	err = m_refToneFilter.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スクリプトコールバック保存
	//
	DWORD	dwReserved = 0 ;
	file.Write( &dwReserved, sizeof(DWORD) ) ;
	err = m_refHitTestProcedure.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refTimerProcedure.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refMouseInterface.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refKeyInterface.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スプライト表示パラメータ保存
	//
	file.Write( &m_dwFlags, sizeof(m_dwFlags) ) ;
	//
	DWORD	dwEnableFlags = 0 ;
	if ( m_fEnabled )
		dwEnableFlags |= 0x0001 ;
	if ( m_fEnabledKeyInput )
		dwEnableFlags |= 0x0002 ;
	if ( m_fEnabledMouseWheel )
		dwEnableFlags |= 0x0004 ;
	//
	file.Write( &dwEnableFlags, sizeof(dwEnableFlags) ) ;
	//
	int			fEnableFillBack = IsEnabledFillBack( ) ;
	EGL_PALETTE	rgbBackColor = GetBackColor( ) ;
	file.Write( &fEnableFillBack, sizeof(int) ) ;
	file.Write( &rgbBackColor, sizeof(EGL_PALETTE) ) ;
	//
	int		fEnableDynamicMode = m_fEnableDynamicMode ;
	file.Write( &fEnableDynamicMode, sizeof(int) ) ;
	//
	int		nPriority = GetPriority( ) ;
	file.Write( &nPriority, sizeof(nPriority) ) ;
	int		nVisible = m_visible ;
	file.Write( &nVisible, sizeof(nVisible) ) ;
	//
	PARAMETER	param ;
	GetParameter( param ) ;
	file.Write( &param, sizeof(param) ) ;
	//
	file.Write( &m_nAlphaRange, sizeof(m_nAlphaRange) ) ;
	file.Write( &m_nBlendDegree, sizeof(m_nBlendDegree) ) ;
	//
	E3D_VECTOR	vScreen = GetScreenPosition() ;
	DWORD	dwDrawFlags = GetDrawFunctionFlags() ;
	DWORD	dwRenderFlags = GetRenderFunctionFlags() ;
	file.Write( &vScreen, sizeof(E3D_VECTOR) ) ;
	file.Write( &dwDrawFlags, sizeof(DWORD) ) ;
	file.Write( &dwRenderFlags, sizeof(DWORD) ) ;
	//
	dwLength = m_wstrID.GetLength( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	if ( dwLength > 0 )
	{
		file.Write( m_wstrID.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	}
	//
	DWORD	dwCameraFlags = m_fCamera ? 1 : 0 ;
	file.Write( &dwCameraFlags, sizeof(DWORD) ) ;
	file.Write( &m_vCameraPos, sizeof(m_vCameraPos) ) ;
	file.Write( &m_vTargetPos, sizeof(m_vTargetPos) ) ;
	file.Write( &m_zRevAngle, sizeof(m_zRevAngle) ) ;
	//
	file.Write( &dwReserved, sizeof(DWORD) ) ;
	//
	// 参照画像保存
	//
	file.Write( &m_nFrameNum, sizeof(m_nFrameNum) ) ;
	file.Write( &m_rectView, sizeof(m_rectView) ) ;
	err = m_refImage.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refAlphaImage.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スプライトアニメーション保存
	//
	file.Write( &m_nLoopCount, sizeof(m_nLoopCount) ) ;
	file.Write( &m_nCurrentSequence, sizeof(m_nCurrentSequence) ) ;
	file.Write( &m_nRewindSequence, sizeof(m_nRewindSequence) ) ;
	file.Write( &m_nTurnSequence, sizeof(m_nTurnSequence) ) ;
	file.Write( &m_nAnimationDuration, sizeof(m_nAnimationDuration) ) ;
	//
	file.Write( &m_dwCurrentMovingTime, sizeof(m_dwCurrentMovingTime) ) ;
	//
	int	fEnableFading = m_fEnableFading ;
	file.Write( &fEnableFading, sizeof(fEnableFading) ) ;
	int	nBezierCount = m_bzDegreeCurve.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzDegreeCurve.GetArrayPtr(), nBezierCount * sizeof(double) ) ;
	nBezierCount = m_bzColorFade.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzColorFade.GetArrayPtr(), nBezierCount * sizeof(EGL_PALETTE) ) ;
//	file.Write( &m_nStartDegree, sizeof(m_nStartDegree) ) ;
//	file.Write( &m_nTargetDegree, sizeof(m_nTargetDegree) ) ;
	//
//	E3D_VECTOR_2D	vPosition[4] ;
//	double			rRevolution[4] ;
//	E3D_VECTOR_2D	vMagnification[4] ;
	int				fEnableMoving = m_fEnableMoving ;
/*	for ( int i = 0; i < 4; i ++ )
	{
		vPosition[i] = m_bzPosition[i] ;
		rRevolution[i] = m_bzRevolution[i] ;
		vMagnification[i] = m_bzMagnification[i] ;
	}*/
	file.Write( &fEnableMoving, sizeof(fEnableMoving) ) ;
	nBezierCount = m_bzPositionCurves.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzPositionCurves.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	nBezierCount = m_bzRevolutionCurves.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzRevolutionCurves.GetArrayPtr(),
				nBezierCount * sizeof(double) ) ;
	nBezierCount = m_bzMagnificationCurves.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzMagnificationCurves.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR_2D) ) ;
//	file.Write( &vPosition, sizeof(vPosition) ) ;
//	file.Write( &rRevolution, sizeof(rRevolution) ) ;
//	file.Write( &vMagnification, sizeof(vMagnification) ) ;
	//
	int	fCameraMoving = m_fCameraMoving ;
	file.Write( &fCameraMoving, sizeof(fCameraMoving) ) ;
	nBezierCount = m_bzCameraPos.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzCameraPos.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	nBezierCount = m_bzCameraTarget.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzCameraTarget.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	nBezierCount = m_bzRevCameraZ.GetCount( ) ;
	file.Write( &nBezierCount, sizeof(nBezierCount) ) ;
	file.Write( m_bzRevCameraZ.GetArrayPtr(),
				nBezierCount * sizeof(double) ) ;
	//
	file.Write( &m_nActionType, sizeof(m_nActionType) ) ;
	file.Write( &m_nDurationTime, sizeof(m_nDurationTime) ) ;
	//
	int	nDurationCount = m_lstDurationsTime.GetSize( ) ;
	file.Write( &nDurationCount, sizeof(nDurationCount) ) ;
	file.Write( m_lstDurationsTime.GetData(),
					nDurationCount * sizeof(ULONG_PTR) ) ;
	//
	file.Write( &dwReserved, sizeof(DWORD) ) ;
	//
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Load
	( ESLFileObject & file, ECSContext & context )
{
	//
	// 親スプライト読み込み
	//
	file.Read( &m_nParentFlag, sizeof(m_nParentFlag) ) ;
	//
	ESLError	err = m_refParent.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スプライトの生成パラメータ読み込み
	//
	file.Read( &m_fdwFormatImage, sizeof(DWORD) ) ;
	file.Read( &m_dwWidthImage, sizeof(DWORD) ) ;
	file.Read( &m_dwHeightImage, sizeof(DWORD) ) ;
	if ( (m_fdwFormatImage != (DWORD) -1)
			&& m_dwWidthImage && m_dwHeightImage )
	{
		CreateImage
			( m_fdwFormatImage, m_dwWidthImage, m_dwHeightImage, 32 ) ;
		UpdateRect( NULL ) ;
	}
	//
	DWORD	fdwZBuffer = 0 ;
	file.Read( &fdwZBuffer, sizeof(DWORD) ) ;
	if ( fdwZBuffer == EIF_Z_BUFFER_R4 )
	{
		CreateZBuffer( ) ;
	}
	//
	// スキンページ参照読み込み
	//
	DWORD	dwLength = 0 ;
	file.Read( &dwLength, sizeof(dwLength) ) ;
	if ( dwLength != 0 )
	{
		file.Read( m_wstrPageID.GetBuffer(dwLength), dwLength * sizeof(wchar_t) ) ;
		m_wstrPageID.ReleaseBuffer( dwLength ) ;
	}
	else
	{
		m_wstrPageID.FreeString() ;
	}
	err = m_refRsrcManager.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// リソース復元
	//
	err = ECSResource::Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// アイテムの設定履歴読み込み
	//
	err = m_hashItemStatus.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// トーンフィルタ関連付け読み込み
	//
	err = m_refToneFilter.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スクリプトコールバック読み込み
	//
	DWORD	dwReserved ;
	file.Read( &dwReserved, sizeof(DWORD) ) ;
	if ( dwReserved != 0 )
	{
		return	eslErrGeneral ;
	}
	err = m_refHitTestProcedure.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refTimerProcedure.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refMouseInterface.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refKeyInterface.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スプライト表示パラメータ読み込み
	//
	file.Read( &m_dwFlags, sizeof(m_dwFlags) ) ;
	//
	DWORD	dwEnableFlags ;
	file.Read( &dwEnableFlags, sizeof(dwEnableFlags) ) ;
	//
	m_fEnabled = ((dwEnableFlags & 0x0001) != 0) ;
	dwEnableFlags = ((dwEnableFlags & 0x0002) != 0) ;
	m_fEnabledMouseWheel = ((dwEnableFlags & 0x0004) != 0) ;
	//
	int			fEnableFillBack ;
	EGL_PALETTE	rgbBackColor ;
	file.Read( &fEnableFillBack, sizeof(int) ) ;
	file.Read( &rgbBackColor, sizeof(EGL_PALETTE) ) ;
	SetBackColor( rgbBackColor, (fEnableFillBack != 0) ) ;
	//
	int		fEnableDynamicMode ;
	file.Read( &fEnableDynamicMode, sizeof(int) ) ;
	m_fEnableDynamicMode = (fEnableDynamicMode != 0) ;
	//
	int		nPriority, nVisible ;
	file.Read( &nPriority, sizeof(nPriority) ) ;
	SetSpritePriority( nPriority ) ;
	file.Read( &nVisible, sizeof(nVisible) ) ;
	SetVisible( (nVisible != 0) ) ;
	//
	PARAMETER	param ;
	file.Read( &param, sizeof(param) ) ;
	SetParameter( param ) ;
	//
	file.Read( &m_nAlphaRange, sizeof(m_nAlphaRange) ) ;
	file.Read( &m_nBlendDegree, sizeof(m_nBlendDegree) ) ;
	//
	E3D_VECTOR	vScreen ;
	DWORD	dwDrawFlags, dwRenderFlags ;
	file.Read( &vScreen, sizeof(E3D_VECTOR) ) ;
	file.Read( &dwDrawFlags, sizeof(DWORD) ) ;
	file.Read( &dwRenderFlags, sizeof(DWORD) ) ;
	SetScreenPosition( vScreen ) ;
	SetDrawFunctionFlags( dwDrawFlags ) ;
	SetRenderFunctionFlags( dwRenderFlags ) ;
	//
	file.Read( &dwLength, sizeof(DWORD) ) ;
	if ( dwLength != 0 )
	{
		file.Read( m_wstrID.GetBuffer(dwLength), dwLength * sizeof(wchar_t) ) ;
		m_wstrID.ReleaseBuffer( dwLength ) ;
	}
	else
	{
		m_wstrID.FreeString() ;
	}
	//
	DWORD	dwCameraFlags ;
	file.Read( &dwCameraFlags, sizeof(DWORD) ) ;
	file.Read( &m_vCameraPos, sizeof(m_vCameraPos) ) ;
	file.Read( &m_vTargetPos, sizeof(m_vTargetPos) ) ;
	file.Read( &m_zRevAngle, sizeof(m_zRevAngle) ) ;
	m_fCamera = ((dwCameraFlags & 0x01) != 0) ;
	//
	file.Read( &dwReserved, sizeof(DWORD) ) ;
	if ( dwReserved != 0 )
	{
		return	eslErrGeneral ;
	}
	//
	// 参照画像読み込み
	//
	file.Read( &m_nFrameNum, sizeof(m_nFrameNum) ) ;
	file.Read( &m_rectView, sizeof(m_rectView) ) ;
	err = m_refImage.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	err = m_refAlphaImage.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// スプライトアニメーション読み込み
	//
	file.Read( &m_nLoopCount, sizeof(m_nLoopCount) ) ;
	file.Read( &m_nCurrentSequence, sizeof(m_nCurrentSequence) ) ;
	file.Read( &m_nRewindSequence, sizeof(m_nRewindSequence) ) ;
	file.Read( &m_nTurnSequence, sizeof(m_nTurnSequence) ) ;
	file.Read( &m_nAnimationDuration, sizeof(m_nAnimationDuration) ) ;
	//
	file.Read( &m_dwCurrentMovingTime, sizeof(m_dwCurrentMovingTime) ) ;
	//
	int	fEnableFading ;
	file.Read( &fEnableFading, sizeof(fEnableFading) ) ;
	m_fEnableFading = (fEnableFading != 0) ;
//	file.Read( &m_nStartDegree, sizeof(m_nStartDegree) ) ;
//	file.Read( &m_nTargetDegree, sizeof(m_nTargetDegree) ) ;
	int	nBezierCount = 0 ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzDegreeCurve.SetCount( nBezierCount ) ;
	file.Read( m_bzDegreeCurve.GetArrayPtr(),
				nBezierCount * sizeof(double) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzColorFade.SetCount( nBezierCount ) ;
	file.Read( m_bzColorFade.GetArrayPtr(),
				nBezierCount * sizeof(EGL_PALETTE) ) ;
	//
//	E3D_VECTOR_2D	vPosition[4] ;
//	double			rRevolution[4] ;
//	E3D_VECTOR_2D	vMagnification[4] ;
	int				fEnableMoving ;
	file.Read( &fEnableMoving, sizeof(fEnableMoving) ) ;
//	file.Read( &vPosition, sizeof(vPosition) ) ;
//	file.Read( &rRevolution, sizeof(rRevolution) ) ;
//	file.Read( &vMagnification, sizeof(vMagnification) ) ;
	m_fEnableMoving = (fEnableMoving != 0) ;
/*	for ( int i = 0; i < 4; i ++ )
	{
		m_bzPosition[i] = vPosition[i] ;
		m_bzRevolution[i] = rRevolution[i] ;
		m_bzMagnification[i] = vMagnification[i] ;
	}*/
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzPositionCurves.SetCount( nBezierCount ) ;
	file.Read( m_bzPositionCurves.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzRevolutionCurves.SetCount( nBezierCount ) ;
	file.Read( m_bzRevolutionCurves.GetArrayPtr(),
				nBezierCount * sizeof(double) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzMagnificationCurves.SetCount( nBezierCount ) ;
	file.Read( m_bzMagnificationCurves.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR_2D) ) ;
	//
	int	fCameraMoving ;
	file.Read( &fCameraMoving, sizeof(int) ) ;
	m_fCameraMoving = (fCameraMoving != 0) ;
	//
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzCameraPos.SetCount( nBezierCount ) ;
	file.Read( m_bzCameraPos.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzCameraTarget.SetCount( nBezierCount ) ;
	file.Read( m_bzCameraTarget.GetArrayPtr(),
				nBezierCount * sizeof(E3D_VECTOR) ) ;
	file.Read( &nBezierCount, sizeof(nBezierCount) ) ;
	m_bzRevCameraZ.SetCount( nBezierCount ) ;
	file.Read( m_bzRevCameraZ.GetArrayPtr(),
				nBezierCount * sizeof(double) ) ;
	//
	file.Read( &m_nActionType, sizeof(m_nActionType) ) ;
	file.Read( &m_nDurationTime, sizeof(m_nDurationTime) ) ;
	//
	int	nDurationCount ;
	file.Read( &nDurationCount, sizeof(nDurationCount) ) ;
	m_lstDurationsTime.SetSize( nDurationCount ) ;
	file.Read( m_lstDurationsTime.GetData(),
					nDurationCount * sizeof(ULONG_PTR) ) ;
	//
//	if ( (m_nDurationTime != 0) && (m_fEnableFading || m_fEnableMoving) )
//	{
//		m_dwStartTime = ::timeGetTime( ) ;
//	}
	//
	file.Read( &dwReserved, sizeof(DWORD) ) ;
	if ( dwReserved != 0 )
	{
		return	eslErrGeneral ;
	}
	//
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	PEGL_IMAGE_INFO	pInfo = GetInfo( ) ;
	EString		strDump ;
	EString		strIndent = '\t' ;
	strIndent = strIndent * (nIndent + 1) ;
	strDump = "<画像未設定>\r\n" ;
	try
	{
		if ( pInfo != NULL )
		{
			strDump = "<画像サイズ = "
				+ EString((int)pInfo->dwImageWidth)
				+ "x" + EString((int)pInfo->dwImageHeight)
				+ " " + EString((int)pInfo->dwBitsPerPixel) + "bpp>\r\n" ;
		}
	}
	catch ( ... )
	{
		strDump = "<画像未設定>\r\n" ;
	}
	if ( !m_wstrFileName.IsEmpty() )
	{
		strDump += strIndent
			+ "ファイル = \"" + EString(m_wstrFileName) + "\"\r\n" ;
	}
	EGL_POINT	ptDstPos = GetPosition( ) ;
	int			nTransparency = GetTransparency( ) ;
	strDump +=
		strIndent
			+ "表示座標 = (" + EString(ptDstPos.x)
			+ "," + EString(ptDstPos.y) + ")\r\n" ;
	strDump += strIndent + "透明度 = " + EString(nTransparency) + "\r\n" ;
	strDump += strIndent + "表示優先度 = " + EString(GetPriority()) ;
	//
	buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	//
	return	eslErrSuccess ;
}

// メンバ変数 : ImageInfo GetInfo()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetInfo
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pHash = context.CreateUserStructure( L"ImageInfo" ) ;
	EGLAnimation *	pImage = ESLTypeCast<EGLAnimation>( m_pRsrc ) ;
	PEGL_IMAGE_INFO	pInfo = NULL ;
	if ( pImage != NULL )
	{
		pInfo = *pImage ;
	}
	if ( pInfo == NULL )
	{
		pInfo = GetInfo() ;
	}
	if ( pInfo != NULL )
	{
		pHash->SetMemberAsInt( L"nFormatType", pInfo->fdwFormatType ) ;
		pHash->SetMemberAsInt( L"nImageWidth", pInfo->dwImageWidth ) ;
		pHash->SetMemberAsInt( L"nImageHeight", pInfo->dwImageHeight ) ;
		pHash->SetMemberAsInt( L"nBitsPerPixel", pInfo->dwBitsPerPixel ) ;
		if ( pImage != NULL )
		{
			pHash->SetMemberAsInt( L"nFrameCount", pImage->GetTotalFrameCount() ) ;
			pHash->SetMemberAsInt( L"xHotSpot", pImage->GetHotSpot().x ) ;
			pHash->SetMemberAsInt( L"yHotSpot", pImage->GetHotSpot().y ) ;
			pHash->SetMemberAsInt
				( L"nResourceBytes", abs( pInfo->dwSizeOfImage )
										* pImage->GetTotalFrameCount() ) ;
		}
		else
		{
			pHash->SetMemberAsInt
				( L"nResourceBytes", abs( pInfo->dwSizeOfImage ) ) ;
		}
	}
	return	context.PushObject( *pHash ) ;
}

// メンバ関数 : ImageInfo& GetImageInfo()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetImageInfo
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	Call_GetInfo( context, lstArg ) ;
}

// メンバ関数 : Release()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_Release
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	UpdateRect( NULL ) ;
	Release( ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 :
//	Integer AttachImage
//		( Reference refImage、Integer nFrameNum := -1[, Rect rcClip] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_AttachImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( pRsrc == NULL )
	{
		return	ESLErrorMsg
			( "引数に Resource オブジェクトが指定されていません。" ) ;
	}
	int		nFrameNum ;
	err = context.GetArgumentAsInt( nFrameNum, lstArg, 2, -1 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pRect =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 3, L"Rect" ) ) ;
	//
	EGLAnimation *	pImage = pRsrc->GetImage( ) ;
	QuickLock( ) ;
	UpdateRect( NULL ) ;
	Release( ) ;
	if ( pImage != NULL )
	{
		m_refImage.SetReference( pRsrc, &context ) ;
		m_nFrameNum = nFrameNum ;
		//
		if ( nFrameNum >= 0 )
		{
			PEGL_IMAGE_INFO	pInfo = pImage->GetFrameAt(nFrameNum) ;
			if ( pRect != NULL )
			{
				m_rectView.left = pRect->GetMemberAsInt( L"left", 0 ) ;
				m_rectView.top = pRect->GetMemberAsInt( L"top", 0 ) ;
				m_rectView.right = pRect->GetMemberAsInt( L"right", 0 ) ;
				m_rectView.bottom = pRect->GetMemberAsInt( L"bottom", 0 ) ;
			}
			else if ( pInfo != NULL )
			{
				m_rectView.left = 0 ;
				m_rectView.top = 0 ;
				m_rectView.right = pInfo->dwImageWidth - 1 ;
				m_rectView.bottom = pInfo->dwImageHeight - 1 ;
			}
			SetImageView( pInfo, &m_rectView ) ;
			//
			if ( pRect == NULL )
			{
				PARAMETER	param ;
				GetParameter( param ) ;
				param.ptRevCenter = pImage->GetHotSpot( ) ;
				SetParameter( param ) ;
			}
		}
		else
		{
			nFrameNum = -1 ;
			CreateAnimation( pImage ) ;
		}
	}
	else
	{
		m_refImage.SetReference( pRsrc, &context ) ;
		m_nFrameNum = nFrameNum ;
		//
		PEGL_IMAGE_INFO	pInfo = pRsrc->GetImageInfo() ;
		if ( pRect != NULL )
		{
			m_rectView.left = pRect->GetMemberAsInt( L"left", 0 ) ;
			m_rectView.top = pRect->GetMemberAsInt( L"top", 0 ) ;
			m_rectView.right = pRect->GetMemberAsInt( L"right", 0 ) ;
			m_rectView.bottom = pRect->GetMemberAsInt( L"bottom", 0 ) ;
		}
		else if ( pInfo != NULL )
		{
			m_rectView.left = 0 ;
			m_rectView.top = 0 ;
			m_rectView.right = pInfo->dwImageWidth - 1 ;
			m_rectView.bottom = pInfo->dwImageHeight - 1 ;
		}
		SetImageView( pRsrc->GetImageInfo(), &m_rectView ) ;
	}
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数
//	Integer CreateSprite( Integer format, Integer width, Integer height )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_CreateSprite
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	int		nFormat, nWidth, nHeight ;
	err = context.GetArgumentAsInt( nFormat, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nWidth, lstArg, 2, 16 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 3, 16 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	Release( ) ;
	int	nBitsPerPixel = 32 ;
	if ( nFormat == EIF_GRAY_BITMAP )
	{
		nBitsPerPixel = 8 ;
	}
	CreateImage( nFormat, nWidth, nHeight, nBitsPerPixel, 0 ) ;
	m_fdwFormatImage = nFormat ;
	m_dwWidthImage = nWidth ;
	m_dwHeightImage = nHeight ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : SetBackColor( Integer rgbBack, Integer fEnableBack )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetBackColor
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int	rgbBack, fFillBack ;
	err = context.GetArgumentAsInt( rgbBack, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fFillBack, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	SetBackColor( EGLPalette( (DWORD) rgbBack ), (fFillBack != 0) ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ変数 : EnableDynamicMode( Integer fDynamicMode )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_EnableDynamicMode
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fDynamicMode ;
	err = context.GetArgumentAsInt( fDynamicMode, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	bool	fOldDynamic ;
	QuickLock( ) ;
	fOldDynamic = EnableDynamicMode( (fDynamicMode != 0) ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( - (int) fOldDynamic ) ) ;
}

// メンバ関数 : CreateZBuffer()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_CreateZBuffer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	if ( GetInfo() != NULL )
	{
		CreateZBuffer( ) ;
	}
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : DeleteZBuffer()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_DeleteZBuffer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	DeleteZBuffer( ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : Integer CreateStereoBuffer( View3DInfo v3dInfo )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_CreateStereoBuffer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pStruct =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"View3DInfo" ) ) ;
	if ( pStruct == NULL )
	{
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	}
	//
	VIEW3D_INFO	v3dInfo ;
	memset( &v3dInfo, 0, sizeof(v3dInfo) ) ;
	v3dInfo.zFocus = pStruct->GetMemberAsReal( L"zFocus", 512 ) ;
	v3dInfo.xParallax = pStruct->GetMemberAsReal( L"xParallax", 5 ) ;
	v3dInfo.zOffset = pStruct->GetMemberAsReal( L"zOffset", 0 ) ;
	//
	QuickLock() ;
	err = CreateStereoBuffer( v3dInfo ) ;
	QuickUnlock() ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : SetStereoViewInfo( View3DInfo v3dInfo )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetStereoViewInfo
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pStruct =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"View3DInfo" ) ) ;
	if ( pStruct == NULL )
	{
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	}
	//
	VIEW3D_INFO	v3dInfo ;
	memset( &v3dInfo, 0, sizeof(v3dInfo) ) ;
	v3dInfo.zFocus = pStruct->GetMemberAsReal( L"zFocus", 512 ) ;
	v3dInfo.xParallax = pStruct->GetMemberAsReal( L"xParallax", 5 ) ;
	v3dInfo.zOffset = pStruct->GetMemberAsReal( L"zOffset", 0 ) ;
	//
	QuickLock() ;
	SetStereoViewInfo( v3dInfo ) ;
	QuickUnlock() ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : DeleteStereoBuffer()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_DeleteStereoBuffer
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock() ;
	DeleteStereoBuffer() ;
	QuickUnlock() ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : Set3DViewCamera
//		( Vector vCamera, Vector vTarget, Real rRevCameraZ := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_Set3DViewCamera
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	E3DVector	vCamera( 0, 0, 0 ), vTarget( 0, 0, 1 ) ;
	double	rRevCameraZ  = 0 ;
	Get3DViewCamera( vCamera, vTarget, rRevCameraZ ) ;
	//
	ECSStructureInterface *	pCamera =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Vector" ) ) ;
	if ( pCamera == NULL )
	{
		return	ESLErrorMsg( "カメラ座標が指定されていません。" ) ;
	}
	ECSStructureInterface *	pTarget =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Vector" ) ) ;
	if ( pTarget == NULL )
	{
		return	ESLErrorMsg( "注視点座標が指定されていません。" ) ;
	}
	err = context.GetArgumentAsReal( rRevCameraZ, lstArg, 3, rRevCameraZ ) ;
	if ( err )
		return	err ;
	//
	vCamera.x = (REAL32) pCamera->GetMemberAsReal( L"x", vCamera.x ) ;
	vCamera.y = (REAL32) pCamera->GetMemberAsReal( L"y", vCamera.x ) ;
	vCamera.z = (REAL32) pCamera->GetMemberAsReal( L"z", vCamera.x ) ;
	//
	vTarget.x = (REAL32) pTarget->GetMemberAsReal( L"x", vTarget.x ) ;
	vTarget.y = (REAL32) pTarget->GetMemberAsReal( L"y", vTarget.x ) ;
	vTarget.z = (REAL32) pTarget->GetMemberAsReal( L"z", vTarget.x ) ;
	//
	Set3DViewCamera( vCamera, vTarget, rRevCameraZ ) ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : Enable3DViewCamera( Integer fEnableCamera )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_Enable3DViewCamera
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fEnableCamera ;
	err = context.GetArgumentAsInt( fEnableCamera, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	Enable3DViewCamera( fEnableCamera != 0 ) ;
	//
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : Integer Get3DViewCamera
//		( Vector vCamera, Vector vTarget, Real rRevCameraZ )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_Get3DViewCamera
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	E3DVector	vCamera( 0, 0, 0 ), vTarget( 0, 0, 1 ) ;
	double	rRevCameraZ ;
	bool	fCamera = Get3DViewCamera( vCamera, vTarget, rRevCameraZ ) ;
	//
	if ( fCamera )
	{
		ECSStructureInterface *	pCamera =
			ESLTypeCast<ECSStructureInterface>
				( context.GetArgumentObjectAs( lstArg, 1, L"Vector" ) ) ;
		if ( pCamera == NULL )
		{
			return	ESLErrorMsg( "カメラ座標が指定されていません。" ) ;
		}
		ECSStructureInterface *	pTarget =
			ESLTypeCast<ECSStructureInterface>
				( context.GetArgumentObjectAs( lstArg, 2, L"Vector" ) ) ;
		if ( pTarget == NULL )
		{
			return	ESLErrorMsg( "注視点座標が指定されていません。" ) ;
		}
		ECSReal *	pRealRevZ =
			ESLTypeCast<ECSReal>( ECSObject::GetEntity( lstArg.GetAt(3) ) ) ;
		if ( pRealRevZ == NULL )
		{
			return	ESLErrorMsg( "引数に Real オブジェクトが指定されていません。" ) ;
		}
		//
		pCamera->SetMemberAsReal( L"x", vCamera.x ) ;
		pCamera->SetMemberAsReal( L"y", vCamera.x ) ;
		pCamera->SetMemberAsReal( L"z", vCamera.x ) ;
		//
		pTarget->SetMemberAsReal( L"x", vTarget.x ) ;
		pTarget->SetMemberAsReal( L"y", vTarget.x ) ;
		pTarget->SetMemberAsReal( L"z", vTarget.x ) ;
		//
		pRealRevZ->m_varReal = rRevCameraZ ;
	}
	//
	return	context.PushObject( new ECSInteger( fCamera ? -1 : 0 ) ) ;
}

// メンバ関数 : Vector GetScreenPosition()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetScreenPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	E3D_VECTOR	vScreenPos ;
	vScreenPos = GetScreenPosition() ;
	//
	ECSStructureInterface *	pVector = context.CreateUserStructure( L"Vector" ) ;
	pVector->SetMemberAsReal( L"x", vScreenPos.x ) ;
	pVector->SetMemberAsReal( L"y", vScreenPos.y ) ;
	pVector->SetMemberAsReal( L"z", vScreenPos.z ) ;
	return	context.PushObject( *pVector ) ;
}

// メンバ関数 : SetScreenPosition( Real x, Real y, Real z )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetScreenPosition
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
	err = context.GetArgumentAsReal( y, lstArg, 2, 0.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsReal( z, lstArg, 3, 0.0 ) ;
	if ( err )
		return	err ;
	//
	SetScreenPosition( E3DVector( (REAL32) x, (REAL32) y, (REAL32) z ) ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetDrawFunctionFlags()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetDrawFunctionFlags
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	long int	nFlags = GetDrawFunctionFlags( ) ;
	return	context.PushObject( new ECSInteger( nFlags ) ) ;
}

// メンバ関数 : SetDrawFunctionFlags( Integer nFlags )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetDrawFunctionFlags
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nFlags ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetDrawFunctionFlags( nFlags ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetRenderFunctionFlags()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetRenderFunctionFlags
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	long int	nFlags = 0 ;
	nFlags = GetRenderFunctionFlags( ) ;
	//
	return	context.PushObject( new ECSInteger( nFlags ) ) ;
}

// メンバ関数 : SetRenderFunctionFlags( Integer nFlags )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetRenderFunctionFlags
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int	nFlags ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	SetRenderFunctionFlags( nFlags ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Reference GetParent()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetParent
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSReference
				( ESLTypeCast<ECSSprite>( GetParent() ) ) ) ;
}

// メンバ関数 : Integer IsVisible()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_IsVisible
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( IsVisible() ? -1 : 0 ) ) ;
}

// メンバ関数 : SetVisible( Integer visible )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetVisible
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nVisible ;
	err = context.GetArgumentAsInt( nVisible, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	SetVisible( nVisible != 0 ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Rect GetRectangle( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetRectangle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	EGLRect	rect( 0, 0, -1, -1 ) ;
	Lock( ) ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		rect = pItem->GetRectangle( ) ;
		while ( pItem != this )
		{
			ESpriteInterface *	pParent =
				ESLTypeCast<ESpriteInterface>( pItem->GetParent() ) ;
			if ( pParent == NULL )
			{
				break ;
			}
			rect = pParent->LocalToGlobal( rect ) ;
			pItem = pParent ;
		}
	}
	Unlock( ) ;
	//
	ECSStructureInterface *	pRectObj = context.CreateUserStructure( L"Rect" ) ;
	pRectObj->SetMemberAsInt( L"left", rect.left ) ;
	pRectObj->SetMemberAsInt( L"top", rect.top ) ;
	pRectObj->SetMemberAsInt( L"right", rect.right ) ;
	pRectObj->SetMemberAsInt( L"bottom", rect.bottom ) ;
	//
	return	context.PushObject( *pRectObj ) ;
}

// メンバ関数 : MovePosition( Integer x, Integer y )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_MovePosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		x, y ;
	err = context.GetArgumentAsInt( x, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( y, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	MovePosition( EGLPoint( x, y ) ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Hash GetPosition()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	EGL_POINT	ptDst = GetPosition( ) ;
	ECSStructureInterface *	pPos = context.CreateUserStructure( L"Point" ) ;
	pPos->SetMemberAsInt( L"x", ptDst.x ) ;
	pPos->SetMemberAsInt( L"y", ptDst.y ) ;
	//
	return	context.PushObject( *pPos ) ;
}

// メンバ関数 : Integer GetTransparency( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetTransparency
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	Lock( ) ;
	unsigned int	nTrans = 0 ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		nTrans = pItem->GetTransparency( ) ;
	}
	Unlock( ) ;
	//
	return	context.PushObject( new ECSInteger( nTrans ) ) ;
}

// メンバ関数 : SetTransparency( Integer trans, String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetTransparency
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nTrans ;
	err = context.GetArgumentAsInt( nTrans, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		pItem->SetTransparency( nTrans ) ;
		//
		if ( !wstrID.IsEmpty() )
		{
			ECSHash *	pHash =
				ESLTypeCast<ECSHash>
					( m_hashItemStatus.m_varArray.GetAs( wstrID ) ) ;
			if ( pHash == NULL )
			{
				pHash = new ECSHash ;
				m_hashItemStatus.m_varArray.SetAs( wstrID, pHash ) ;
			}
			pHash->SetMemberAsInt( L"transparency", nTrans ) ;
		}
	}
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetZPosition( Real zPos [, Real zScale] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetZPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	PARAMETER	param ;
	GetParameter( param ) ;
	//
	double	zPos, zScale ;
	err = context.GetArgumentAsReal( zPos, lstArg, 1, param.rZOrder ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsReal( zScale, lstArg, 1, param.rZScale ) ;
	if ( err )
	{
		return	err ;
	}
	param.rZOrder = (REAL32) zPos ;
	param.rZScale = (REAL32) zScale ;
	//
	QuickLock( ) ;
	SetParameter( param ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Real GetZPosition()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetZPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSReal( GetZPosition() ) ) ;
}

// メンバ関数 : GetParameter( Reference param )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetParameter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pParam =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 1, L"SpriteParam" ) ) ;
	if ( pParam == NULL )
	{
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	}
	//
	PARAMETER	param ;
	GetParameter( param ) ;
	//
	ECSStructureInterface *	pPoint ;
	pParam->SetMemberAsInt( L"nFlags", param.dwFlags ) ;
	pPoint = ESLTypeCast<ECSStructureInterface>
				( pParam->GetMemberAs( L"ptDstPos" ) ) ;
	if ( pPoint != NULL )
	{
		pPoint->SetMemberAsInt( L"x", param.ptDstPos.x ) ;
		pPoint->SetMemberAsInt( L"y", param.ptDstPos.y ) ;
	}
	pPoint = ESLTypeCast<ECSStructureInterface>
				( pParam->GetMemberAs( L"ptRevCenter" ) ) ;
	if ( pPoint != NULL )
	{
		pPoint->SetMemberAsInt( L"x", param.ptRevCenter.x ) ;
		pPoint->SetMemberAsInt( L"y", param.ptRevCenter.y ) ;
	}
	pParam->SetMemberAsReal( L"rHorzUnit", param.rHorzUnit ) ;
	pParam->SetMemberAsReal( L"rVertUnit", param.rVertUnit ) ;
	pParam->SetMemberAsReal( L"rRevAngle", param.rRevAngle ) ;
	pParam->SetMemberAsReal( L"rCrossingAngle", param.rCrossingAngle ) ;
	pParam->SetMemberAsInt( L"rgbDimColor", param.rgbDimColor.dwPixelCode ) ;
	pParam->SetMemberAsInt( L"rgbLightColor", param.rgbLightColor.dwPixelCode ) ;
	pParam->SetMemberAsInt( L"nTransparency", param.nTransparency ) ;
	pParam->SetMemberAsReal( L"rZOrder", param.rZOrder ) ;
	pParam->SetMemberAsInt( L"rgbColorParam1", param.rgbColorParam1.dwPixelCode ) ;
	pParam->SetMemberAsReal( L"rZScale", param.rZScale ) ;
	//
	return	context.PushObject( new ECSReference( pParam ) ) ;
}

// メンバ関数 : SetParameter( Hash param )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetParameter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructure *	pParam =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 1, L"SpriteParam" ) ) ;
	if ( pParam == NULL )
	{
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	}
	//
	PARAMETER	param ;
	GetParameter( param ) ;
	ECSStructureInterface *	pPoint ;
	//
	param.dwFlags = pParam->GetMemberAsInt( L"nFlags", param.dwFlags ) ;
	pPoint = ESLTypeCast<ECSStructureInterface>( pParam->GetMemberAs( L"ptDstPos" ) ) ;
	if ( pPoint != NULL )
	{
		param.ptDstPos.x = pPoint->GetMemberAsInt( L"x", param.ptDstPos.x ) ;
		param.ptDstPos.y = pPoint->GetMemberAsInt( L"y", param.ptDstPos.y ) ;
	}
	pPoint = ESLTypeCast<ECSStructureInterface>( pParam->GetMemberAs( L"ptRevCenter" ) ) ;
	if ( pPoint != NULL )
	{
		param.ptRevCenter.x = pPoint->GetMemberAsInt( L"x", param.ptRevCenter.x ) ;
		param.ptRevCenter.y = pPoint->GetMemberAsInt( L"y", param.ptRevCenter.y ) ;
	}
	param.rHorzUnit = (REAL32)
		pParam->GetMemberAsReal( L"rHorzUnit", param.rHorzUnit ) ;
	param.rVertUnit = (REAL32)
		pParam->GetMemberAsReal( L"rVertUnit", param.rVertUnit ) ;
	param.rRevAngle = (REAL32)
		pParam->GetMemberAsReal( L"rRevAngle", param.rRevAngle ) ;
	param.rCrossingAngle = (REAL32)
		pParam->GetMemberAsReal( L"rCrossingAngle", param.rCrossingAngle ) ;
	param.rgbDimColor.dwPixelCode =
		pParam->GetMemberAsInt( L"rgbDimColor", param.rgbDimColor.dwPixelCode ) ;
	param.rgbLightColor.dwPixelCode =
		pParam->GetMemberAsInt( L"rgbLightColor", param.rgbLightColor.dwPixelCode ) ;
	param.nTransparency =
		pParam->GetMemberAsInt( L"nTransparency", param.nTransparency ) ;
	param.rZOrder = (REAL32)
		pParam->GetMemberAsReal( L"rZOrder", param.rZOrder ) ;
	param.rgbColorParam1.dwPixelCode =
		pParam->GetMemberAsInt( L"rgbColorParam1", param.rgbColorParam1.dwPixelCode ) ;
	param.rZScale = (REAL32)
		pParam->GetMemberAsReal( L"rZScale", param.rZScale ) ;
	//
	QuickLock( ) ;
	SetParameter( param ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 ; Error CopyParameters( const Sprite& src )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_CopyParameters
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSSprite *	pSrc =
		ESLTypeCast<ECSSprite>
			( context.GetArgumentObjectAs( lstArg, 1, L"Sprite" ) ) ;
	if ( pSrc == NULL )
	{
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	}
	//
	QuickLock( ) ;
	CopyParameters( pSrc ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( context.new_CSInteger() ) ;
}

// メンバ関数 ; Integer UpdateRect( [Rect rect] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_UpdateRect
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	EGL_RECT *	pRectUpdate = NULL ;
	EGL_RECT	rectUpdate ;
	ECSStructureInterface *	pObj =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Rect" ) ) ;
	if ( pObj != NULL )
	{
		rectUpdate.left = pObj->GetMemberAsInt( L"left", 0 ) ;
		rectUpdate.top = pObj->GetMemberAsInt( L"top", 0 ) ;
		rectUpdate.right = pObj->GetMemberAsInt( L"right", 0 ) ;
		rectUpdate.bottom = pObj->GetMemberAsInt( L"bottom", 0 ) ;
		pRectUpdate = &rectUpdate ;
	}
	//
	ECSInteger *	pIntResult ;
	QuickLock( ) ;
//	if ( GetInfo() != NULL )
	{
		pIntResult =
			new ECSInteger( UpdateRect( pRectUpdate ) ) ;
	}
/*	else if ( m_pMainSprite != NULL )
	{
		pIntResult =
			new ECSInteger( m_pMainSprite->UpdateRect( pRectUpdate ) ) ;
	}
	else
	{
		pIntResult =
			new ECSInteger( UpdateRect( pRectUpdate ) ) ;
	}*/
	QuickUnlock( ) ;
	//
	return	context.PushObject( pIntResult ) ;
}

// メンバ関数 : Refresh()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_Refresh
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	bool	fLock = (GetParent() != NULL) ;
	if ( fLock )
	{
		QuickLock( ) ;
	}
	Refresh( ) ;
	if ( fLock )
	{
		QuickUnlock( ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetPriority( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetPriority
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	int	nPriority = 0 ;
	QuickLock( ) ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		nPriority = pItem->GetPriority( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( nPriority ) ) ;
}

// メンバ関数 : ChangePriority( Integer priority, String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_ChangePriority
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nPriority ;
	err = context.GetArgumentAsInt( nPriority, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		pItem->ChangePriority( nPriority ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : AddSprite( Integer priority, Reference sprite )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_AddSprite
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nPriority ;
	err = context.GetArgumentAsInt( nPriority, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	ESprite *	pSprite
		= ESLTypeCast<ECSSprite>
			( context.GetArgumentObjectAs( lstArg, 2, L"Sprite" ) ) ;
	if ( pSprite == NULL )
	{
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	}
	//
	QuickLock( ) ;
	ESpriteServer *	pParent =
		ESLTypeCast<ESpriteServer>( pSprite->GetParent() ) ;
	if ( pParent != NULL )
	{
		ECSSprite *	pParentSprite = ESLTypeCast<ECSSprite>( pParent ) ;
		if ( pParentSprite != NULL )
		{
			pParentSprite->Lock( ) ;
		}
		pSprite->UpdateRect( NULL ) ;
		pParent->DetachSprite( pSprite ) ;
		if ( pParentSprite != NULL )
		{
			pParentSprite->Unlock( ) ;
		}
	}
//	if ( GetInfo() != NULL )
	{
		AddSprite( nPriority, pSprite ) ;
	}
/*	else if ( m_pMainSprite != NULL )
	{
		m_pMainSprite->AddSprite( nPriority, pSprite ) ;
	}*/
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : DetachSprite( Refenrece sprite )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_DetachSprite
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ESprite *	pSprite
		= ESLTypeCast<ECSSprite>
			( context.GetArgumentObjectAs( lstArg, 1, L"Sprite" ) ) ;
	if ( pSprite == NULL )
	{
		return	ESLErrorMsg( "引数の型が不正です。" ) ;
	}
	QuickLock( ) ;
//	if ( GetInfo() != NULL )
	{
		DetachSprite( pSprite ) ;
	}
/*	else if ( m_pMainSprite != NULL )
	{
		m_pMainSprite->DetachSprite( pSprite ) ;
	}*/
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : DetachAllSprite()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_DetachAllSprite
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	RemoveAllSkinItems( ) ;
	m_wstrPageID.FreeString( ) ;
	DetachAllSprite( ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer DrawImage
//		( Reference rImage, SpriteParam param,
//				Integer iFrame := 0 [, Rect rcClip] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_DrawImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 5 ) ;
	if ( err )
		return	err ;
	//
	// 引数取得
	//
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( pRsrc == NULL )
	{
		return	ESLErrorMsg( "引数に画像リソースが指定されていません" ) ;
	}
	ECSStructure *	pParam =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 2, L"SpriteParam" ) ) ;
	if ( pParam == NULL )
	{
		return	ESLErrorMsg
					( "引数に SpriteParam 構造体が指定されていません" ) ;
	}
	int		iFrame ;
	err = context.GetArgumentAsInt( iFrame, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	EGL_RECT *	pViewRect = NULL ;
	EGL_RECT	rectClip ;
	ECSStructureInterface *	pClip =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 4, L"Rect" ) ) ;
	if ( pClip != NULL )
	{
		rectClip.left = pClip->GetMemberAsInt( L"left", 0 ) ;
		rectClip.top = pClip->GetMemberAsInt( L"top", 0 ) ;
		rectClip.right = pClip->GetMemberAsInt( L"right", -1 ) ;
		rectClip.bottom = pClip->GetMemberAsInt( L"bottom", -1 ) ;
		pViewRect = &rectClip ;
	}
	//
	// パラメータ取得
	//
	PARAMETER		param ;
	ECSStructureInterface *	pPoint ;
	::eslFillMemory( &param, 0, sizeof(param) ) ;
	//
	param.dwFlags = pParam->GetMemberAsInt( L"nFlags", param.dwFlags ) ;
	pPoint = ESLTypeCast<ECSStructureInterface>( pParam->GetMemberAs( L"ptDstPos" ) ) ;
	if ( pPoint != NULL )
	{
		param.ptDstPos.x = pPoint->GetMemberAsInt( L"x", param.ptDstPos.x ) ;
		param.ptDstPos.y = pPoint->GetMemberAsInt( L"y", param.ptDstPos.y ) ;
	}
	pPoint = ESLTypeCast<ECSStructureInterface>( pParam->GetMemberAs( L"ptRevCenter" ) ) ;
	if ( pPoint != NULL )
	{
		param.ptRevCenter.x = pPoint->GetMemberAsInt( L"x", param.ptRevCenter.x ) ;
		param.ptRevCenter.y = pPoint->GetMemberAsInt( L"y", param.ptRevCenter.y ) ;
	}
	param.rHorzUnit = (REAL32)
		pParam->GetMemberAsReal( L"rHorzUnit", param.rHorzUnit ) ;
	param.rVertUnit = (REAL32)
		pParam->GetMemberAsReal( L"rVertUnit", param.rVertUnit ) ;
	param.rRevAngle = (REAL32)
		pParam->GetMemberAsReal( L"rRevAngle", param.rRevAngle ) ;
	param.rCrossingAngle = (REAL32)
		pParam->GetMemberAsReal( L"rCrossingAngle", param.rCrossingAngle ) ;
	param.rgbDimColor.dwPixelCode =
		pParam->GetMemberAsInt( L"rgbDimColor", param.rgbDimColor.dwPixelCode ) ;
	param.rgbLightColor.dwPixelCode =
		pParam->GetMemberAsInt( L"rgbLightColor", param.rgbLightColor.dwPixelCode ) ;
	param.nTransparency =
		pParam->GetMemberAsInt( L"nTransparency", param.nTransparency ) ;
	param.rZOrder = (REAL32)
		pParam->GetMemberAsReal( L"rZOrder", param.rZOrder ) ;
	//
	// 描画元画像取得
	//
	PEGL_IMAGE_INFO	pSrcImage = NULL ;
	ECSSprite *	pSprite =
		ESLTypeCast<ECSSprite>( pRsrc ) ;
	if ( (pSprite == NULL) || (iFrame != 0) )
	{
		EGLAnimation *	pImage = pRsrc->GetImage( ) ;
		if ( pImage != NULL )
		{
			pSrcImage = pImage->GetFrameAt( iFrame ) ;
		}
	}
	else
	{
		pSrcImage = *pSprite ;
	}
	//
	// 描画先画像取得
	//
	PEGL_IMAGE_INFO	pDstImage = GetInfo( ) ;
	ESpriteServer *	pDstSprite = this ;
	if ( pDstImage == NULL )
	{
/*		if ( m_pMainSprite != NULL )
		{
			pDstImage = *m_pMainSprite ;
			pDstSprite = m_pMainSprite ;
		}*/
	}
	else if ( m_iofOwnerFlag != iofOwnBuffer )
	{
		pDstImage = NULL ;
	}
	if ( (pDstImage == NULL) || (pSrcImage == NULL) )
	{
		return	context.PushObject( new ECSInteger( eslErrGeneral ) ) ;
	}
	//
	// 描画実行
	//
	QuickLock( ) ;
	EImageSprite	isDrawSprite ;
	HEGL_RENDER_POLYGON	hRender = m_hRenderPoly ;
	if ( hRender == NULL )
	{
		hRender = ::eglCurrentRenderPolygon( ) ;
	}
	hRender->Initialize
		( pDstImage, NULL,
			pDstSprite->GetZBuffer(),
			&(pDstSprite->GetScreenPosition()) ) ;
	isDrawSprite.SetImageView( pSrcImage, pViewRect ) ;
	isDrawSprite.SetParameter( param ) ;
	isDrawSprite.Draw( hRender ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( eslErrSuccess ) ) ;
}

// メンバ関数 : Integer FillRect
//		( Rect rcFill, Integer rgbaFill,
//			Integer nTransparency := 0, Integer nFlags := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_FillRect
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 5 ) ;
	if ( err )
		return	err ;
	//
	// 引数取得
	//
	ECSStructureInterface *	pRect =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Rect" ) ) ;
	if ( pRect == NULL )
	{
		return	ESLErrorMsg( "引数に Rect 構造体が指定されていません。" ) ;
	}
	int	rgbFill, nTransparency, nFlags ;
	err = context.GetArgumentAsInt( rgbFill, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nTransparency, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 4, 0 ) ;
	if ( err )
		return	err ;
	//
	EGL_RECT	rectFill ;
	rectFill.left = pRect->GetMemberAsInt( L"left", -1 ) ;
	rectFill.top = pRect->GetMemberAsInt( L"top", -1 ) ;
	rectFill.right = pRect->GetMemberAsInt( L"right", -1 ) ;
	rectFill.bottom = pRect->GetMemberAsInt( L"bottom", -1 ) ;
	//
	// 描画先画像取得
	//
	PEGL_IMAGE_INFO	pDstImage = GetInfo( ) ;
	ESpriteServer *	pDstSprite = this ;
	if ( pDstImage == NULL )
	{
/*		if ( m_pMainSprite != NULL )
		{
			pDstImage = *m_pMainSprite ;
			pDstSprite = m_pMainSprite ;
		}*/
	}
	else if ( m_iofOwnerFlag != iofOwnBuffer )
	{
		pDstImage = NULL ;
	}
	if ( pDstImage == NULL )
	{
		return	context.PushObject( new ECSInteger( eslErrGeneral ) ) ;
	}
	//
	// 描画実行
	//
	QuickLock( ) ;
	HEGL_RENDER_POLYGON	hRender = m_hRenderPoly ;
	HEGL_DRAW_IMAGE	hDraw ;
	if ( hRender == NULL )
	{
		hRender = ::eglCurrentRenderPolygon( ) ;
	}
	hRender->Initialize
		( pDstImage, NULL,
			pDstSprite->GetZBuffer(),
			&(pDstSprite->GetScreenPosition()) ) ;
	hDraw = hRender->GetDrawImage( ) ;
	if ( !hDraw->PrepareFillRect
		( &rectFill, EGLPalette( (DWORD) rgbFill ), nTransparency, nFlags ) )
	{
		hDraw->FillRegion( ) ;
	}
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( eslErrSuccess ) ) ;
}

// メンバ関数 : Integer DrawText( DrawTextParam dtp, String strText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_DrawText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	// 引数取得
	//
	ECSStructure *	pParam =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 1, L"DrawTextParam" ) ) ;
	if ( pParam == NULL )
	{
		return	ESLErrorMsg
			( "引数に DrawTextParam 構造体が指定されていません。" ) ;
	}
	ECSWideString	wstrFaceName, wstrText ;
	err = context.GetArgumentAsStr( wstrText, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	// パラメータ取得
	//
	DRAW_TEXT_PARAM	dtp ;
	ECSStructureInterface *	pRect ;
	ECSStructureInterface *	pPoint ;
	::eslFillMemory( &dtp, 0, sizeof(dtp) ) ;
	//
	pRect = ESLTypeCast<ECSStructureInterface>( pParam->GetMemberAs( L"rcArea" ) ) ;
	if ( pRect != NULL )
	{
		dtp.rcArea.left = pRect->GetMemberAsInt( L"left", 0 ) ;
		dtp.rcArea.top = pRect->GetMemberAsInt( L"top", 0 ) ;
		dtp.rcArea.right = pRect->GetMemberAsInt( L"right", 0 ) ;
		dtp.rcArea.bottom = pRect->GetMemberAsInt( L"bottom", 0 ) ;
	}
	pPoint = ESLTypeCast<ECSStructureInterface>( pParam->GetMemberAs( L"ptCurPos" ) ) ;
	if ( pPoint != NULL )
	{
		dtp.ptCurPos.x = pPoint->GetMemberAsInt( L"x", 0 ) ;
		dtp.ptCurPos.y = pPoint->GetMemberAsInt( L"y", 0 ) ;
	}
	dtp.nFlags = pParam->GetMemberAsInt( L"nFlags", 0 ) ;
	dtp.rgbColor.dwPixelCode = pParam->GetMemberAsInt( L"rgbColor", 0 ) ;
	dtp.nTransparency = pParam->GetMemberAsInt( L"nTransparency", 0 ) ;
	dtp.nLineHeight = pParam->GetMemberAsInt( L"nLineHeight", 0 ) ;
	dtp.nIndentWidth = pParam->GetMemberAsInt( L"nIndentWidth", 0 ) ;
	dtp.nFontSize = pParam->GetMemberAsInt( L"nFontSize", 0 ) ;
	wstrFaceName = pParam->GetMemberAsStr( L"strFontFace", NULL ) ;
	dtp.pwszFontFace = wstrFaceName ;
	//
	// 描画先画像取得
	//
	PEGL_IMAGE_INFO	pDstImage = GetInfo( ) ;
	ESpriteServer *	pDstSprite = this ;
	if ( pDstImage == NULL )
	{
/*		if ( m_pMainSprite != NULL )
		{
			pDstImage = *m_pMainSprite ;
			pDstSprite = m_pMainSprite ;
		}*/
	}
	else if ( m_iofOwnerFlag != iofOwnBuffer )
	{
		pDstImage = NULL ;
	}
	if ( pDstImage == NULL )
	{
		return	context.PushObject( new ECSInteger( 0 ) ) ;
	}
	//
	// フォント＆描画オブジェクト生成
	//
	ERealFontImage	rfi ;
	EFontObject	font( dtp.nFontSize ) ;
	font.SetFaceName( EString( dtp.pwszFontFace ) ) ;
	rfi.SetFont( font.Create() ) ;
	rfi.SetViewRect( dtp.rcArea ) ;
	rfi.SetVerticalWriting( (dtp.nFlags & DTPF_VERTICAL) != 0 ) ;
	rfi.SetFontSmoothing( !(dtp.nFlags & DTPF_NOSMOOTHING) ) ;
	rfi.SetColor( dtp.rgbColor ) ;
	rfi.SetTransparency( dtp.nTransparency ) ;
	rfi.MoveCursorPos( dtp.ptCurPos ) ;
	rfi.SetLineHeight( dtp.nLineHeight ) ;
	rfi.SetIndentWidth( dtp.nIndentWidth ) ;
	//
	// 文字画像生成
	//
	int	nDrawnChars = 0 ;
	int	nDrawWidth ;
	int	nAlign = dtp.nFlags & DTPF_ALIGN_MASK ;
	int	nRightWidth = dtp.rcArea.right + 1 - dtp.ptCurPos.x ;
	if ( nAlign == DTPF_CENTER )
	{
		nDrawWidth = rfi.GetTextWidth( wstrText ) ;
		if ( nDrawWidth <= nRightWidth )
		{
			dtp.ptCurPos.x += (nRightWidth - nDrawWidth) / 2 ;
			rfi.MoveCursorPos( dtp.ptCurPos ) ;
		}
	}
	else if ( nAlign == DTPF_RIGHT )
	{
		nDrawWidth = rfi.GetTextWidth( wstrText ) ;
		if ( nDrawWidth <= nRightWidth )
		{
			dtp.ptCurPos.x += nRightWidth - nDrawWidth ;
			rfi.MoveCursorPos( dtp.ptCurPos ) ;
		}
	}
	if ( nAlign != DTPF_ACCORDING )
	{
		nDrawnChars = rfi.DrawText( wstrText ) ;
	}
	else
	{
		rfi.FitTextToWidth
			( wstrText, dtp.ptCurPos.x, dtp.ptCurPos.y, nRightWidth ) ;
		nDrawnChars = wstrText.GetLength( ) ;
	}
	if ( pPoint != NULL )
	{
		dtp.ptCurPos = rfi.GetCursorPos( ) ;
		pPoint->SetMemberAsInt( L"x", dtp.ptCurPos.x ) ;
		pPoint->SetMemberAsInt( L"y", dtp.ptCurPos.y ) ;
	}
	//
	// 描画実行
	//
	QuickLock( ) ;
	HEGL_RENDER_POLYGON	hRender = m_hRenderPoly ;
	if ( hRender == NULL )
	{
		hRender = ::eglCurrentRenderPolygon( ) ;
	}
	hRender->Initialize
		( pDstImage, NULL,
			pDstSprite->GetZBuffer(),
			&(pDstSprite->GetScreenPosition()) ) ;
	rfi.DrawCharacter( hRender ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( nDrawnChars ) ) ;
}

// メンバ関数 : String GetSpriteID()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetSpriteID
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSString( m_wstrID ) ) ;
}

// メンバ関数 : SetSpriteID( String strID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetSpriteID
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock() ;
	SetID( wstrID ) ;
	QuickUnlock() ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Enable( Integer fEnable, String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_Enable
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		fEnable ;
	err = context.GetArgumentAsInt( fEnable, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		QuickLock( ) ;
		pItem->Enable( fEnable != 0 ) ;
		//
		if ( !wstrID.IsEmpty() )
		{
			ECSHash *	pHash =
				ESLTypeCast<ECSHash>
					( m_hashItemStatus.m_varArray.GetAs( wstrID ) ) ;
			if ( pHash == NULL )
			{
				pHash = new ECSHash ;
				m_hashItemStatus.m_varArray.SetAs( wstrID, pHash ) ;
			}
			pHash->SetMemberAsInt( L"enabled", fEnable != 0 ) ;
		}
		//
		QuickUnlock( ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsEnabled( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_IsEnabled
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	bool	fEnabled = false ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		fEnabled = pItem->IsEnabled( ) ;
	}
	return	context.PushObject( new ECSInteger( fEnabled ? -1 : 0 ) ) ;
}

// メンバ関数 : String GetSpriteText( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetSpriteText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pStrResult = new ECSString ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		QuickLock( ) ;
		pStrResult->m_varStr = pItem->GetSpriteText() ;
		QuickUnlock( ) ;
	}
	return	context.PushObject( pStrResult ) ;
}

// メンバ関数 : SetSpriteText( String strID, String strText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetSpriteText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID, wstrText ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrText, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		QuickLock( ) ;
		pItem->SetSpriteText( wstrText ) ;
		//
		ECSHash *	pHash =
			ESLTypeCast<ECSHash>
				( m_hashItemStatus.m_varArray.GetAs( wstrID ) ) ;
		if ( pHash == NULL )
		{
			pHash = new ECSHash ;
			m_hashItemStatus.m_varArray.SetAs( wstrID, pHash ) ;
		}
		pHash->SetMemberAsStr( L"text", wstrText ) ;
		//
		QuickUnlock( ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetSpriteFontFace( String strID, String strFont )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetSpriteFontFace
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID, wstrFont ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrFont, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	if ( pItem != NULL )
	{
		QuickLock( ) ;
		pItem->SetSpriteFontFace( wstrFont ) ;
		//
		ECSHash *	pHash =
			ESLTypeCast<ECSHash>
				( m_hashItemStatus.m_varArray.GetAs( wstrID ) ) ;
		if ( pHash == NULL )
		{
			pHash = new ECSHash ;
			m_hashItemStatus.m_varArray.SetAs( wstrID, pHash ) ;
		}
		pHash->SetMemberAsStr( L"font", wstrFont ) ;
		//
		QuickUnlock( ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetSpriteImage( String strID, String strImageID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetSpriteImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID, wstrImage ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrImage, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSResourceManager *	pcsrm =
		ESLTypeCast<ECSResourceManager>( m_refRsrcManager.m_pRef ) ;
	EAnimationSprite *	pItem =
		ESLTypeCast<EAnimationSprite>( GetSpriteItemAs( wstrID ) ) ;
	if ( (pcsrm != NULL) && (pItem != NULL) )
	{
		EGL_IMAGE_INFO	eiiImage ;
		EGLAnimation *	pImage =
			ESLTypeCast<EGLAnimation>( pcsrm->GetResourceAs( wstrImage ) ) ;
		if ( pImage != NULL )
		{
			QuickLock( ) ;
			pItem->UpdateRect( NULL ) ;
			pItem->CreateAnimation( pImage ) ;
			QuickUnlock( ) ;
		}
		else if ( pcsrm->GetStillImageResource
							( wstrImage, &eiiImage ) != NULL )
		{
			QuickLock( ) ;
			pItem->SetImageView( &eiiImage ) ;
			QuickUnlock( ) ;
		}
		else
		{
			QuickLock( ) ;
			pItem->AttachImage( NULL ) ;
			QuickUnlock( ) ;
		}
		//
		ECSHash *	pHash =
			ESLTypeCast<ECSHash>
				( m_hashItemStatus.m_varArray.GetAs( wstrID ) ) ;
		if ( pHash == NULL )
		{
			pHash = new ECSHash ;
			m_hashItemStatus.m_varArray.SetAs( wstrID, pHash ) ;
		}
		pHash->SetMemberAsStr( L"image", wstrImage ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsHitSprite( Integer x, Integer y, String strID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_IsHitSprite
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	int		x, y ;
	ECSWideString	wstrID ;
	err = context.GetArgumentAsInt( x, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( y, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 3, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID, false ) ;
	if ( pSprite == NULL )
	{
		QuickUnlock( ) ;
		return	context.PushObject( new ECSInteger( 0 ) ) ;
	}
	if ( !wstrID.IsEmpty() )
	{
		EGL_POINT	ptGlobal, ptLocal ;
		ptGlobal.x = x ;
		ptGlobal.y = y ;
		ptLocal = GlobalToLocal( ptGlobal ) ;
		x = ptLocal.x ;
		y = ptLocal.y ;
	}
	long int	nResult ;
	nResult = pSprite->IsHitSprite( x, y ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( nResult ) ) ;
}

// メンバ関数 : String GetSpriteAtPoint( Integer x, Integer y )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetSpriteAtPoint
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	int		x, y ;
	err = context.GetArgumentAsInt( x, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( y, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	EGL_POINT	ptGlobal, ptLocal ;
	ptGlobal.x = x ;
	ptGlobal.y = y ;
	QuickLock( ) ;
	ptLocal = GlobalToLocal( ptGlobal ) ;
	ESpriteInterface *	pSprite =
		GetSpriteAtPoint( ptLocal.x, ptLocal.y ) ;
	ECSString *	pstrResult ;
	if ( pSprite != NULL )
	{
		pstrResult = new ECSString( pSprite->ID() ) ;
	}
	else
	{
		pstrResult = new ECSString( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( pstrResult ) ;
}

// メンバ関数 : String GetFocus()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetFocus
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ESpriteInterface *	pFocus ;
	ECSWideString	wstrFocus ;
	QuickLock( ) ;
	pFocus = GetFocus( ) ;
	while ( pFocus != NULL )
	{
		if ( !pFocus->ID().IsEmpty() )
		{
			if ( !wstrFocus.IsEmpty() )
				wstrFocus += L'\\' ;
			wstrFocus += pFocus->ID() ;
		}
		pFocus = pFocus->GetFocus( ) ;
	}
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSString( wstrFocus ) ) ;
}

// メンバ関数 : SetFocus( String strID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetFocus
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	SetFocus( GetSpriteItemAs( wstrID ) ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : KillFocus( String strID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_KillFocus
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	KillFocus( GetSpriteItemAs( wstrID ) ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : MoveFocus( Integer fNext := 1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_MoveFocus
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int		fNext ;
	err = context.GetArgumentAsInt( fNext, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	MoveFocus( fNext != 0 ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetCapture( [String strID] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetCapture
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	SetCapture( wstrID.IsEmpty() ? NULL : GetSpriteItemAs( wstrID ) ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : ReleaseCapture()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_ReleaseCapture
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ReleaseCapture( wstrID.IsEmpty() ? NULL : GetSpriteItemAs( wstrID ) ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetVertScrollPos( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetVertScrollPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	int		nPos = 0 ;
	if ( pSprite != NULL )
	{
		nPos = pSprite->GetVertScrollPos( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( nPos ) ) ;
}

// メンバ関数 : SetVertScrollPos( Integer nPos, String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetVertScrollPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nPos ;
	err = context.GetArgumentAsInt( nPos, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	if ( pSprite != NULL )
	{
		pSprite->SetVertScrollPos( nPos ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetVertScrollRange( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetVertScrollRange
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	int		nRange = 0 ;
	if ( pSprite != NULL )
	{
		nRange = pSprite->GetVertScrollRange( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( nRange ) ) ;
}

// メンバ関数 : SetVertScrollRange( Integer nRange, String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetVertScrollRange
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nRange ;
	err = context.GetArgumentAsInt( nRange, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	if ( pSprite != NULL )
	{
		pSprite->SetVertScrollRange( nRange ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetHorzScrollPos( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetHorzScrollPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	int		nPos = 0 ;
	if ( pSprite != NULL )
	{
		nPos = pSprite->GetHorzScrollPos( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( nPos ) ) ;
}

// メンバ関数 : SetHorzScrollPos( Integer nPos, String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetHorzScrollPos
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nPos ;
	err = context.GetArgumentAsInt( nPos, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	if ( pSprite != NULL )
	{
		pSprite->SetHorzScrollPos( nPos ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetHorzScrollRange( String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetHorzScrollRange
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	int		nRange = 0 ;
	if ( pSprite != NULL )
	{
		nRange = pSprite->GetHorzScrollRange( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( nRange ) ) ;
}

// メンバ関数 : SetHorzScrollRange( Integer nRange, String strID := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetHorzScrollRange
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nRange ;
	err = context.GetArgumentAsInt( nRange, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pSprite = GetSpriteItemAs( wstrID ) ;
	if ( pSprite != NULL )
	{
		pSprite->SetHorzScrollRange( nRange ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsButtonChecked( String strID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_IsButtonChecked
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	bool	fCheck = false ;
	QuickLock( ) ;
	EButtonSprite *	pButton =
		ESLTypeCast<EButtonSprite>( GetSpriteItemAs( wstrID ) ) ;
	if ( pButton != NULL )
	{
		fCheck = pButton->IsButtonChecked( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( fCheck ? -1 : 0 ) ) ;
}

// メンバ関数 : CheckButton( String strID, Integer fCheck )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_CheckButton
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	int		fCheck ;
	err = context.GetArgumentAsInt( fCheck, lstArg, 2, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	EButtonSprite *	pButton =
		ESLTypeCast<EButtonSprite>( GetSpriteItemAs( wstrID ) ) ;
	if ( pButton != NULL )
	{
		pButton->CheckButton( fCheck != 0 ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : GetButtonViewStyle( String strID )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetButtonViewStyle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	long int	nResult = -1 ;
	QuickLock( ) ;
	EButtonSprite *	pButton =
		ESLTypeCast<EButtonSprite>( GetSpriteItemAs( wstrID ) ) ;
	if ( pButton != NULL )
	{
		nResult = pButton->GetButtonViewStatus( ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger( nResult ) ) ;
}

// メンバ関数 : Integer SendCommand
//				( String strID, String strCmd[, String strResult] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SendCommand
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrID, wstrCmd ;
	err = context.GetArgumentAsStr( wstrID, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrCmd, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	ECSString *	pstrResult =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 3, L"String" ) ) ;
	//
	QuickLock( ) ;
	ESpriteInterface *	pItem = GetSpriteItemAs( wstrID ) ;
	long int	nResult = -1 ;
	if ( pItem != NULL )
	{
		EDescription		dscCmd ;
		EStreamWideString	swsCmd = wstrCmd ;
		EWideString			wstrResult ;
		dscCmd.ReadDescription( swsCmd, dscCmd.dftXML ) ;
		nResult = pItem->SendCommand( dscCmd, &wstrResult ) ;
		if ( pstrResult != NULL )
		{
			pstrResult->m_varStr = wstrResult ;
		}
		//
		ECSHash *	pHash =
			ESLTypeCast<ECSHash>
				( m_hashItemStatus.m_varArray.GetAs( wstrID ) ) ;
		if ( pHash == NULL )
		{
			pHash = new ECSHash ;
			m_hashItemStatus.m_varArray.SetAs( wstrID, pHash ) ;
		}
		pHash->SetMemberAsStr( L"command", wstrCmd ) ;
	}
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Error SetHitTestProcedure( SpriteHitTestProcedure& proc )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetHitTestProcedure
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pHook =
		context.GetArgumentObjectAs( lstArg, 1, L"SpriteHitTestProcedure" ) ;
	QuickLock() ;
	if ( pHook != NULL )
	{
		ECS_CAST_INTERFACE	ci ;
		err = pHook->OperateCastInterface( ci, L"SpriteHitTestProcedure" ) ;
		if ( !err )
		{
			m_refHitTestProcedure.SetReferenceCastInterface
								( ci.pCastObject, &context, ci ) ;
		}
	}
	else
	{
		m_refHitTestProcedure.SetReference( NULL, &context ) ;
	}
	QuickUnlock() ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Error SetTimerProcedure( SpriteTimerProcedure& proc )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetTimerProcedure
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pHook =
		context.GetArgumentObjectAs( lstArg, 1, L"SpriteTimerProcedure" ) ;
	QuickLock() ;
	if ( pHook != NULL )
	{
		ECS_CAST_INTERFACE	ci ;
		err = pHook->OperateCastInterface( ci, L"SpriteTimerProcedure" ) ;
		if ( !err )
		{
			m_refTimerProcedure.SetReferenceCastInterface
								( ci.pCastObject, &context, ci ) ;
		}
	}
	else
	{
		m_refTimerProcedure.SetReference( NULL, &context ) ;
	}
	QuickUnlock() ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Error SetMouseInterface( SpriteMouseInterface& hook )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetMouseInterface
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pHook =
		context.GetArgumentObjectAs( lstArg, 1, L"SpriteMouseInterface" ) ;
	QuickLock() ;
	if ( pHook != NULL )
	{
		ECS_CAST_INTERFACE	ci ;
		err = pHook->OperateCastInterface( ci, L"SpriteMouseInterface" ) ;
		if ( !err )
		{
			m_refMouseInterface.SetReferenceCastInterface
								( ci.pCastObject, &context, ci ) ;
		}
	}
	else
	{
		m_refMouseInterface.SetReference( NULL, &context ) ;
	}
	QuickUnlock() ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Error SetKeyInterface( SpriteKeyInterface& hook )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetKeyInterface
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pHook =
		context.GetArgumentObjectAs( lstArg, 1, L"SpriteKeyInterface" ) ;
	QuickLock() ;
	if ( pHook != NULL )
	{
		ECS_CAST_INTERFACE	ci ;
		err = pHook->OperateCastInterface( ci, L"SpriteKeyInterface" ) ;
		if ( !err )
		{
			m_refKeyInterface.SetReferenceCastInterface
								( ci.pCastObject, &context, ci ) ;
		}
	}
	else
	{
		m_refKeyInterface.SetReference( NULL, &context ) ;
	}
	QuickUnlock() ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 :
//	Integer ModifyAnimationFlags
//		( Integer nAddFlags := 0, Integer nRemoveFlags := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_ModifyAnimationFlags
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int	nAddFlags, nRemoveFlags ;
	err = context.GetArgumentAsInt( nAddFlags, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nRemoveFlags, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	DWORD	dwFlags =
		(EAnimationSprite::GetAnimationFlags() & ~nRemoveFlags) | nAddFlags ;
	EAnimationSprite::SetAnimationFlags( dwFlags ) ;
	//
	return	context.PushObject( new ECSInteger( dwFlags ) ) ;
}

// メンバ関数 :
//	Integer BeginAnimation
//		( Integer nLoopCount := 1, Integer nBeginFrame := 0,
//			Integer nAnimationTime := -1,
//			Integer nRewindSequence := 0, Integer nTurnSequence := -1 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_BeginAnimation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 6 ) ;
	if ( err )
		return	err ;
	//
	int	nLoopCount, nBeginFrame, nAnimationTime, nRewindSequence, nTurnSequence ;
	err = context.GetArgumentAsInt( nLoopCount, lstArg, 1, 1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nBeginFrame, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nAnimationTime, lstArg, 3, -1 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nRewindSequence, lstArg, 4, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nTurnSequence, lstArg, 5, -1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	BeginAnimation
		( nLoopCount, nBeginFrame,
			nAnimationTime, nRewindSequence, nTurnSequence ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger( eslErrSuccess ) ) ;
}

// メンバ関数 : Integer EndAnimation( )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_EndAnimation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	EndAnimation( ) ;
	return	context.PushObject( new ECSInteger( eslErrSuccess ) ) ;
}

// メンバ関数 : Integer IsDuringAnimation( )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_IsDuringAnimation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	int	fAnimation = IsDuringAnimation() ? -1 : 0 ;
	return	context.PushObject( new ECSInteger( fAnimation ) ) ;
}

// メンバ関数 :
//	Integer SetAlphaImage( Reference refAlpha, Integer nAlphaRange )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetAlphaImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( pRsrc == NULL )
	{
		return	ESLErrorMsg
			( "引数に Resource オブジェクトが指定されていません。" ) ;
	}
	int		nAlphaRange ;
	err = context.GetArgumentAsInt( nAlphaRange, lstArg, 2, 0x100 ) ;
	if ( err )
		return	err ;
	//
	err = eslErrGeneral ;
	EGLImage *	pImage = pRsrc->GetImage( ) ;
	if ( pImage == NULL )
	{
		pImage = ESLTypeCast<EGLImage>( pRsrc ) ;
	}
	if ( pImage != NULL )
	{
		QuickLock( ) ;
		err = SetAlphaImage( *pImage, nAlphaRange ) ;
		if ( !err )
			m_refAlphaImage.SetReference( pRsrc, &context ) ;
		else
			m_refAlphaImage.SetReference( NULL, &context ) ;
		QuickUnlock( ) ;
	}
	else
	{
		QuickLock( ) ;
		err = SetAlphaImage( NULL, nAlphaRange ) ;
		m_refAlphaImage.SetReference( NULL, &context ) ;
		QuickUnlock( ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer GetBlendDegree()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_GetBlendDegree
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetBlendDegree() ) ) ;
}

// メンバ関数 : SetBlendDegree( Integer nDegree )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetBlendDegree
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nDegree ;
	err = context.GetArgumentAsInt( nDegree, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	SetBlendDegree( nDegree ) ;
	QuickUnlock( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetBlendingEnlevope( { Integer nTargetDegree | Array bzFading } )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetBlendingEnvelope
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nTargetDegree ;
	ECSArray *	pCurve =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	if ( pCurve != NULL )
	{
		EBezierCurves<double>	bzFading ;
		int	i, nCount = pCurve->m_varArray.GetSize( ) ;
		bzFading.SetCount( nCount ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			ECSObject *	pObj = pCurve->m_varArray.GetAt( i ) ;
			if ( pObj != NULL )
			{
				if ( pObj->m_vtType == csvtReal )
				{
					ECSReal *	pReal = (ECSReal*) pObj ;
					bzFading[i] = pReal->m_varReal ;
				}
				else if ( pObj->m_vtType == csvtInteger )
				{
					ECSInteger *	pInteger = (ECSInteger*) pObj ;
					bzFading[i] = (double) pInteger->GetValue() ;
				}
				else
				{
					bzFading.SetCount( i ) ;
					break ;
				}
			}
			else
			{
				bzFading.SetCount( i ) ;
				break ;
			}
		}
		QuickLock( ) ;
		SetBlendingEnvelope( bzFading ) ;
		QuickUnlock( ) ;
	}
	else
	{
		err = context.GetArgumentAsInt( nTargetDegree, lstArg, 1, 0 ) ;
		if ( err )
			return	err ;
		//
		QuickLock( ) ;
		SetBlendingEnvelope( nTargetDegree ) ;
		QuickUnlock( ) ;
	}
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 :
//	SetBezierCurve( Array bzCurve[, Array bzRev[, Array bzMagnify]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetBezierCurve
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pCurve =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	ECSArray *	pRev =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 2, L"Array" ) ) ;
	ECSArray *	pMagnify =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 3, L"Array" ) ) ;
	//
	EBezierCurves<E3D_VECTOR>		bzCurve, * pbzCurve = NULL ;
	EBezierCurves<double>			bzRev, * pbzRev = NULL ;
	EBezierCurves<E3D_VECTOR_2D>	bzMagnify, * pbzMagnify = NULL ;
	int			i, nCount ;
	ECSStructureInterface *	pStruct ;
	ECSReal *				pReal ;
	if ( pCurve != NULL )
	{
		PARAMETER	param ;
		GetParameter( param ) ;
		//
		nCount = pCurve->m_varArray.GetSize( ) ;
		bzCurve.SetCount( nCount ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			pStruct = ESLTypeCast<ECSStructureInterface>
							( pCurve->m_varArray.GetAt( i ) ) ;
			if ( pStruct == NULL )
			{
				bzCurve.SetCount( i ) ;
				break ;
			}
			bzCurve[i].x = (REAL32) pStruct->GetMemberAsReal( L"x", 0 ) ;
			bzCurve[i].y = (REAL32) pStruct->GetMemberAsReal( L"y", 0 ) ;
			bzCurve[i].z = (REAL32) pStruct->GetMemberAsReal( L"z", param.rZOrder ) ;
		}
		pbzCurve = &bzCurve ;
	}
	if ( pRev != NULL )
	{
		nCount = pRev->m_varArray.GetSize( ) ;
		bzRev.SetCount( nCount ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			pReal = (ECSReal*) pRev->m_varArray.GetAt( i ) ;
			if ( (pReal == NULL) || (pReal->m_vtType != csvtReal) )
			{
				bzRev.SetCount( i ) ;
				break ;
			}
			bzRev[i] = pReal->m_varReal ;
		}
		pbzRev = &bzRev ;
	}
	if ( pMagnify != NULL )
	{
		nCount = pMagnify->m_varArray.GetSize( ) ;
		bzMagnify.SetCount( nCount ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			pStruct = ESLTypeCast<ECSStructureInterface>
							( pMagnify->m_varArray.GetAt( i ) ) ;
			if ( pStruct == NULL )
			{
				bzMagnify.SetCount( i ) ;
				break ;
			}
			bzMagnify[i].x = (REAL32) pStruct->GetMemberAsReal( L"x", 0 ) ;
			bzMagnify[i].y = (REAL32) pStruct->GetMemberAsReal( L"y", 0 ) ;
		}
		pbzMagnify = &bzMagnify ;
	}
	//
	QuickLock( ) ;
	SetBezierCurve( pbzCurve, pbzRev, pbzMagnify ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetCameraCurve
//	( Array bzCurve, Array bzTarget[, Array bzRevZ] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_SetCameraCurve
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	pCurve =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	ECSArray *	pTarget =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 2, L"Array" ) ) ;
	ECSArray *	pRevZ =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 3, L"Array" ) ) ;
	//
	E3DVector	vViewPos( 0, 0, 0 ), vTargetPos( 0, 0, 0 ) ;
	double		rRevAngleZ = 0 ;
	Get3DViewCamera( vViewPos, vTargetPos, rRevAngleZ ) ;
	//
	EBezierCurves<E3D_VECTOR>	bzCamera ;
	EBezierCurves<E3D_VECTOR>	bzTarget ;
	EBezierCurves<double>		bzRevAngle ;
	//
	int			i, nCount ;
	ECSStructureInterface *	pStruct ;
	ECSReal *				pReal ;
	if ( pCurve != NULL )
	{
		nCount = pCurve->m_varArray.GetSize( ) ;
		bzCamera.SetCount( nCount ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			pStruct = ESLTypeCast<ECSStructureInterface>
							( pCurve->m_varArray.GetAt( i ) ) ;
			if ( pStruct == NULL )
			{
				bzCamera.SetCount( i ) ;
				break ;
			}
			bzCamera[i].x =
				(REAL32) pStruct->GetMemberAsReal( L"x", vViewPos.x ) ;
			bzCamera[i].y =
				(REAL32) pStruct->GetMemberAsReal( L"y", vViewPos.y ) ;
			bzCamera[i].z =
				(REAL32) pStruct->GetMemberAsReal( L"z", vViewPos.z ) ;
		}
	}
	else
	{
		bzCamera.SetCount( 4 ) ;
		bzCamera.SetLine( vViewPos, vViewPos ) ;
	}
	if ( pTarget != NULL )
	{
		nCount = pTarget->m_varArray.GetSize( ) ;
		bzTarget.SetCount( nCount ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			pStruct = ESLTypeCast<ECSStructureInterface>
							( pCurve->m_varArray.GetAt( i ) ) ;
			if ( pStruct == NULL )
			{
				bzTarget.SetCount( i ) ;
				break ;
			}
			bzTarget[i].x =
				(REAL32) pStruct->GetMemberAsReal( L"x", vTargetPos.x ) ;
			bzTarget[i].y =
				(REAL32) pStruct->GetMemberAsReal( L"y", vTargetPos.y ) ;
			bzTarget[i].z =
				(REAL32) pStruct->GetMemberAsReal( L"z", vTargetPos.z ) ;
		}
	}
	else
	{
		bzTarget.SetCount( 4 ) ;
		bzTarget.SetLine( vTargetPos, vTargetPos ) ;
	}
	if ( pRevZ != NULL )
	{
		nCount = pRevZ->m_varArray.GetSize( ) ;
		bzRevAngle.SetCount( nCount ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			pReal = (ECSReal*) pRevZ->m_varArray.GetAt( i ) ;
			if ( (pReal == NULL) || (pReal->m_vtType != csvtReal) )
			{
				bzRevAngle.SetCount( i ) ;
				break ;
			}
			bzRevAngle[i] = pReal->m_varReal ;
		}
	}
	else
	{
		bzRevAngle.SetCount( 4 ) ;
		bzRevAngle.SetLine( rRevAngleZ, rRevAngleZ ) ;
	}
	//
	QuickLock( ) ;
	SetCameraCurve( bzCamera, bzTarget, bzRevAngle ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : BeginActivation
//	( { Integer nDurationTime | Array bzDurations }, Integer nActionType := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_BeginActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	int		nDurationTime, nActionType ;
	ECSArray *	pDurations =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	err = context.GetArgumentAsInt( nActionType, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	if ( pDurations != NULL )
	{
		int				i, nCount = pDurations->m_varArray.GetSize( ) ;
		unsigned int *	pDurationList = new unsigned int [nCount] ;
		for ( i = 0; i < nCount; i ++ )
		{
			ECSInteger *	pInt =
				(ECSInteger*) pDurations->m_varArray.GetAt( i ) ;
			if ( (pInt == NULL) || (pInt->m_vtType != csvtInteger) )
			{
				break ;
			}
			pDurationList[i] = pInt->GetInt() ;
		}
		//
		QuickLock( ) ;
		BeginActivation( pDurationList, nCount, NULL, nActionType ) ;
		QuickUnlock( ) ;
		//
		delete [] pDurationList ;
	}
	else
	{
		err = context.GetArgumentAsInt( nDurationTime, lstArg, 1, 0 ) ;
		if ( err )
			return	err ;
		//
		QuickLock( ) ;
		BeginActivation( nDurationTime, NULL, nActionType ) ;
		QuickUnlock( ) ;
	}
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : FlushActivation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_FlushActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	FlushActivation( ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : CancelActivation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_CancelActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	QuickLock( ) ;
	CancelActivation( ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsActivation()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_IsActivation
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( IsActivation() ? -1 : 0 ) ) ;
}

// メンバ関数 : AttachToneFilter( [Reference filter] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSprite::Call_AttachToneFilter
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSToneFilter *	pFilter =
		ESLTypeCast<ECSToneFilter>
			( context.GetArgumentObjectAs( lstArg, 1, L"ToneFilter" ) ) ;
	QuickLock( ) ;
	AttachToneFilter( pFilter ) ;
	QuickUnlock( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// スレッド同期（スクリプト用）
//		※この関数は（スクリプトを実行している）単一のスレッドからのみ
//		呼び出されると仮定して、不要な API の呼び出しを抑制します
//		※別のスレッドからは ECSSprite::m_nLockedCount を
//		読み取ることのみが許可されます
//////////////////////////////////////////////////////////////////////////////
/*
ESLError ECSSprite::QuickLock( DWORD dwTimeout )
{
	ESLError	err = eslErrSuccess ;
	if ( m_pMainWnd != NULL )
	{
#if	defined(ERI_INTEL_X86)
		// このアセンブリは少なくともシングルプロセッサ環境での
		// マルチスレッド呼び出しを保証する
		// ※ マルチプロセッサ環境でのマルチスレッド呼び出しでの
		// 正確な動作は保証されない
		long int	nLockedCount ;
		long int *	pLockedCount = &m_nLockedCount ;
		__asm
		{
			mov		edx, pLockedCount
			mov		eax, 1
			xadd	DWORD PTR [edx], eax
			mov		nLockedCount, eax
		}
		if ( nLockedCount == 0 )
#else
		if ( ++ m_nLockedCount == 1 )
#endif
		{
			err = Lock( dwTimeout ) ;
			if ( err )
			{
#if	defined(ERI_INTEL_X86)
				__asm
				{
					mov		edx, pLockedCount
					dec		DWORD PTR [edx]
				}
#else
				m_nLockedCount -- ;
#endif
				ESLTrace( "ECSSprite::QuickLock failed\n" ) ;
			}
		}
	}
	return	err ;
}

void ECSSprite::QuickUnlock( void )
{
	if ( m_pMainWnd != NULL )
	{
	#if	defined(ERI_INTEL_X86)
		long int	nLockedCount ;
		long int *	pLockedCount = &m_nLockedCount ;
		__asm
		{
			mov		edx, pLockedCount
			mov		eax, -1
			xadd	DWORD PTR [edx], eax
			mov		nLockedCount, eax
		}
		if ( nLockedCount == 1 )
	#else
		if ( -- m_nLockedCount == 0 )
	#endif
		{
			if ( Unlock( ) )
			{
	#if	defined(ERI_INTEL_X86)
				__asm
				{
					mov		edx, pLockedCount
					inc		DWORD PTR [edx]
				}
	#else
				m_nLockedCount ++ ;
	#endif
				ESLTrace( "ECSSprite::QuickUnlock failed\n" ) ;
			}
		}
		ESLAssert( nLockedCount >= 0 ) ;
	}
}
*/

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSSprite::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_SPRITE_INTERFACE" ) )
	{
		if ( m_ppis == NULL )
		{
			m_ppis = (PLUGIN_SPRITE*)
				::eslHeapAllocate( NULL, sizeof(PLUGIN_SPRITE), 0 ) ;
			m_ppis->pBackLink = this ;
			m_ppis->pfnGetImageBuffer = PIC_GetImageBuffer ;
			m_ppis->pfnGetWindow = PIC_GetWindow ;
			m_ppis->pfnAttachImage = PIC_AttachImage ;
			m_ppis->pfnSetImageView = PIC_SetImageView ;
			m_ppis->pfnCreateSprite = PIC_CreateSprite ;
			m_ppis->pfnRelease = PIC_Release ;
			m_ppis->pfnSetBackColor = PIC_SetBackColor ;
			m_ppis->pfnEnableDynamicMode = PIC_EnableDynamicMode ;
			m_ppis->pfnGetZBuffer = PIC_GetZBuffer ;
			m_ppis->pfnCreateZBuffer = PIC_CreateZBuffer ;
			m_ppis->pfnDeleteZBuffer = PIC_DeleteZBuffer ;
			m_ppis->pfnGetScreenPosition = PIC_GetScreenPosition ;
			m_ppis->pfnSetScreenPosition = PIC_SetScreenPosition ;
			m_ppis->pfnGetDrawFunctionFlags = PIC_GetDrawFunctionFlags ;
			m_ppis->pfnGetRenderFunctionFlags = PIC_GetRenderFunctionFlags ;
			m_ppis->pfnSetDrawFunctionFlags = PIC_SetDrawFunctionFlags ;
			m_ppis->pfnSetRenderFunctionFlags = PIC_SetRenderFunctionFlags ;
			m_ppis->pfnGetParent = PIC_GetParent ;
			m_ppis->pfnIsVisible = PIC_IsVisible ;
			m_ppis->pfnSetVisible = PIC_SetVisible ;
			m_ppis->pfnGetRectangle = PIC_GetRectangle ;
			m_ppis->pfnGetPosition = PIC_GetPosition ;
			m_ppis->pfnMovePosition = PIC_MovePosition ;
			m_ppis->pfnGetTransparency = PIC_GetTransparency ;
			m_ppis->pfnSetTransparency = PIC_SetTransparency ;
			m_ppis->pfnGetParameter = PIC_GetParameter ;
			m_ppis->pfnSetParameter = PIC_SetParameter ;
			m_ppis->pfnUpdateRect = PIC_UpdateRect ;
			m_ppis->pfnRefresh = PIC_Refresh ;
			m_ppis->pfnGetPriority = PIC_GetPriority ;
			m_ppis->pfnChangePriority = PIC_ChangePriority ;
			m_ppis->pfnAddSprite = PIC_AddSprite ;
			m_ppis->pfnDetachSprite = PIC_DetachSprite ;
			m_ppis->pfnDetachAllSprite = PIC_DetachAllSprite ;
			m_ppis->pfnModifyAnimationFlags = PIC_ModifyAnimationFlags ;
			m_ppis->pfnBeginAnimation = PIC_BeginAnimation ;
			m_ppis->pfnEndAnimation = PIC_EndAnimation ;
			m_ppis->pfnIsDuringAnimation = PIC_IsDuringAnimation ;
			m_ppis->pfnSetAlphaImage = PIC_SetAlphaImage ;
			m_ppis->pfnGetBlendDegree = PIC_GetBlendDegree ;
			m_ppis->pfnSetBlendDegree = PIC_SetBlendDegree ;
			m_ppis->pfnSetBlendingEnvelope = PIC_SetBlendingEnvelope ;
			m_ppis->pfnSetBezierCurve = PIC_SetBezierCurve ;
			m_ppis->pfnBeginActivation = PIC_BeginActivation ;
			m_ppis->pfnFlushActivation = PIC_FlushActivation ;
			m_ppis->pfnCancelActivation = PIC_CancelActivation ;
			m_ppis->pfnIsActivation = PIC_IsActivation ;
		}
		return	(ECS_SPRITE_INTERFACE*) m_ppis ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

PCEGL_IMAGE_INFO __stdcall ECSSprite::PIC_GetImageBuffer
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->GetInfo() ;
}

HWND __stdcall ECSSprite::PIC_GetWindow
	( ECS_SPRITE_INTERFACE * instance )
{
	EWindowSpriteInterface *
		pWndItf = SpriteFromPlugin(instance)->GetWindowInterface( ) ;
	if ( pWndItf != NULL )
	{
		EWindow *	pWnd = pWndItf->GetWindow( ) ;
		if ( pWnd != NULL )
		{
			return	*pWnd ;
		}
	}
	return	NULL ;
}

ESLError __stdcall ECSSprite::PIC_AttachImage
	( ECS_SPRITE_INTERFACE * instance, ECS_OBJECT * pImage )
{
	ECSSprite *		pSprite = SpriteFromPlugin( instance ) ;
	EGLAnimation *	pAnime = NULL ;
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>( ECSObject::ObjectFromPlugin( pImage ) ) ;
	if ( pRsrc != NULL )
	{
		pAnime = pRsrc->GetImage( ) ;
	}
	if ( pAnime != NULL )
	{
		return	pSprite->CreateAnimation( pAnime ) ;
	}
	ECSSprite *	pAttachImage =
		ESLTypeCast<ECSSprite>( ECSObject::ObjectFromPlugin( pImage ) ) ;
	if ( pAttachImage != NULL )
	{
		pSprite->AttachImage( pAttachImage->GetInfo() ) ;
	}
	else
	{
		pSprite->AttachImage( NULL ) ;
	}
	return	eslErrSuccess ;
}

ESLError __stdcall ECSSprite::PIC_SetImageView
	( ECS_SPRITE_INTERFACE * instance,
		PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect )
{
	SpriteFromPlugin(instance)->SetImageView( pImage, pViewRect ) ;
	return	eslErrSuccess ;
}

ESLError __stdcall ECSSprite::PIC_CreateSprite
	( ECS_SPRITE_INTERFACE * instance,
		DWORD fdwFormat, int nWidth, int nHeight )
{
	DWORD	dwBitsPerSample = 32 ;
	if ( fdwFormat == EIF_GRAY_BITMAP )
	{
		dwBitsPerSample = 8 ;
	}
	if ( SpriteFromPlugin(instance)->
			CreateImage( fdwFormat, nWidth, nHeight, dwBitsPerSample ) )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

void __stdcall ECSSprite::PIC_Release( ECS_SPRITE_INTERFACE * instance )
{
	SpriteFromPlugin(instance)->Release() ;
}

void __stdcall ECSSprite::PIC_SetBackColor
	( ECS_SPRITE_INTERFACE * instance,
		EGL_PALETTE rgbBack, int fEnableBack )
{
	SpriteFromPlugin(instance)->SetBackColor( rgbBack, (fEnableBack != 0) ) ;
}

int __stdcall ECSSprite::PIC_EnableDynamicMode
	( ECS_SPRITE_INTERFACE * instance, int fDynamicMode )
{
	return	SpriteFromPlugin(instance)->EnableDynamicMode( (fDynamicMode != 0) ) ;
}

PCEGL_IMAGE_INFO __stdcall ECSSprite::PIC_GetZBuffer
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->GetZBuffer() ;
}

void __stdcall ECSSprite::PIC_CreateZBuffer( ECS_SPRITE_INTERFACE * instance )
{
	SpriteFromPlugin(instance)->CreateZBuffer( ) ;
}

void __stdcall ECSSprite::PIC_DeleteZBuffer( ECS_SPRITE_INTERFACE * instance )
{
	SpriteFromPlugin(instance)->DeleteZBuffer( ) ;
}

const E3D_VECTOR * __stdcall ECSSprite::PIC_GetScreenPosition
	( ECS_SPRITE_INTERFACE * instance )
{
	return	&(SpriteFromPlugin(instance)->GetScreenPosition( )) ;
}

void __stdcall ECSSprite::PIC_SetScreenPosition
	( ECS_SPRITE_INTERFACE * instance, const E3D_VECTOR * vScreen )
{
	SpriteFromPlugin(instance)->SetScreenPosition( *vScreen ) ;
}

DWORD __stdcall ECSSprite::PIC_GetDrawFunctionFlags
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->GetDrawFunctionFlags( ) ;
}

DWORD __stdcall ECSSprite::PIC_GetRenderFunctionFlags
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->GetRenderFunctionFlags( ) ;
}

void __stdcall ECSSprite::PIC_SetDrawFunctionFlags
	( ECS_SPRITE_INTERFACE * instance, DWORD dwFlags )
{
	SpriteFromPlugin(instance)->SetDrawFunctionFlags( dwFlags ) ;
}

void __stdcall ECSSprite::PIC_SetRenderFunctionFlags
	( ECS_SPRITE_INTERFACE * instance, DWORD dwFlags )
{
	SpriteFromPlugin(instance)->SetRenderFunctionFlags( dwFlags ) ;
}

ECS_OBJECT * __stdcall ECSSprite::PIC_GetParent
	( ECS_SPRITE_INTERFACE * instance )
{
	ECSSprite *	pParent =
		ESLTypeCast<ECSSprite>( SpriteFromPlugin(instance)->GetParent( ) ) ;
	if ( pParent != NULL )
	{
		return	pParent->CreateInterface( ) ;
	}
	return	NULL ;
}

int __stdcall ECSSprite::PIC_IsVisible( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->IsVisible( ) ;
}

void __stdcall ECSSprite::PIC_SetVisible
	( ECS_SPRITE_INTERFACE * instance, int fVisible )
{
	SpriteFromPlugin(instance)->SetVisible( (fVisible != 0) ) ;
}

void __stdcall ECSSprite::PIC_GetRectangle
	( ECS_SPRITE_INTERFACE * instance, EGL_RECT * rect )
{
	*rect = SpriteFromPlugin(instance)->GetRectangle( ) ;
}

void __stdcall ECSSprite::PIC_GetPosition
	( ECS_SPRITE_INTERFACE * instance, EGL_POINT * pos )
{
	*pos = SpriteFromPlugin(instance)->GetPosition( ) ;
}

void __stdcall ECSSprite::PIC_MovePosition
	( ECS_SPRITE_INTERFACE * instance, long int xPos, long int yPos )
{
	EGL_POINT	pos = { xPos, yPos } ;
	SpriteFromPlugin(instance)->MovePosition( pos ) ;
}

unsigned int __stdcall ECSSprite::PIC_GetTransparency
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->GetTransparency( ) ;
}

void __stdcall ECSSprite::PIC_SetTransparency
	( ECS_SPRITE_INTERFACE * instance, unsigned int nTransparency )
{
	SpriteFromPlugin(instance)->SetTransparency( nTransparency ) ;
}

void __stdcall ECSSprite::PIC_GetParameter
	( ECS_SPRITE_INTERFACE * instance,
		EImageSprite::PARAMETER * param )
{
	SpriteFromPlugin(instance)->GetParameter( *param ) ;
}

void __stdcall ECSSprite::PIC_SetParameter
	( ECS_SPRITE_INTERFACE * instance,
		const EImageSprite::PARAMETER * param )
{
	SpriteFromPlugin(instance)->SetParameter( *param ) ;
}

void __stdcall ECSSprite::PIC_UpdateRect
	( ECS_SPRITE_INTERFACE * instance, const EGL_RECT * pUpdateRect )
{
	EGL_RECT	rctUpdate ;
	if ( pUpdateRect != NULL )
	{
		rctUpdate = *pUpdateRect ;
		SpriteFromPlugin(instance)->UpdateRect( &rctUpdate ) ;
	}
	else
	{
		SpriteFromPlugin(instance)->UpdateRect( NULL ) ;
	}
}

void __stdcall ECSSprite::PIC_Refresh( ECS_SPRITE_INTERFACE * instance )
{
	SpriteFromPlugin(instance)->Refresh( ) ;
}

int __stdcall ECSSprite::PIC_GetPriority( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->GetPriority( ) ;
}

void __stdcall ECSSprite::PIC_ChangePriority
	( ECS_SPRITE_INTERFACE * instance, int nPriority )
{
	SpriteFromPlugin(instance)->ChangePriority( nPriority ) ;
}

void __stdcall ECSSprite::PIC_AddSprite
	( ECS_SPRITE_INTERFACE * instance,
		int nPriority, ECS_SPRITE_INTERFACE * pChild )
{
	SpriteFromPlugin(instance)->
		AddSprite( nPriority, SpriteFromPlugin(pChild) ) ;
}

ESLError __stdcall ECSSprite::PIC_DetachSprite
	( ECS_SPRITE_INTERFACE * instance, ECS_SPRITE_INTERFACE * pChild )
{
	return	SpriteFromPlugin(instance)->
				DetachSprite( SpriteFromPlugin(pChild) ) ;
}

ESLError __stdcall ECSSprite::PIC_DetachAllSprite
	( ECS_SPRITE_INTERFACE * instance )
{
	SpriteFromPlugin(instance)->DetachAllSprite( ) ;
	return	eslErrSuccess ;
}

DWORD __stdcall ECSSprite::PIC_ModifyAnimationFlags
	( ECS_SPRITE_INTERFACE * instance,
		DWORD dwAddFlags, DWORD dwRemoveFlags )
{
	DWORD	dwFlags =
		(EAnimationSprite::GetAnimationFlags() & ~dwRemoveFlags) | dwAddFlags ;
	EAnimationSprite::SetAnimationFlags( dwFlags ) ;
	return	dwFlags ;
}

ESLError __stdcall ECSSprite::PIC_BeginAnimation
	( ECS_SPRITE_INTERFACE * instance,
		unsigned long int nLoopCount, unsigned long int nBeginFrame,
		unsigned long int nAnimationTime,
		unsigned long int nRewindSequence, unsigned long int nTurnSequence )
{
	return	SpriteFromPlugin(instance)->
				BeginAnimation( nLoopCount, nBeginFrame,
						nAnimationTime, nRewindSequence, nTurnSequence ) ;
}

ESLError __stdcall ECSSprite::PIC_EndAnimation( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->EndAnimation( ) ;
}

int __stdcall ECSSprite::PIC_IsDuringAnimation( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->IsDuringAnimation( ) ;
}

ESLError __stdcall ECSSprite::PIC_SetAlphaImage
	( ECS_SPRITE_INTERFACE * instance,
		PEGL_IMAGE_INFO pBlendAlpha, unsigned int nAlphaRange )
{
	return	SpriteFromPlugin(instance)->
				SetAlphaImage( pBlendAlpha, nAlphaRange ) ;
}

unsigned int __stdcall ECSSprite::PIC_GetBlendDegree
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->GetBlendDegree( ) ;
}

void __stdcall ECSSprite::PIC_SetBlendDegree
	( ECS_SPRITE_INTERFACE * instance, unsigned int nDegree )
{
	SpriteFromPlugin(instance)->SetBlendDegree( nDegree ) ;
}

ESLError __stdcall ECSSprite::PIC_SetBlendingEnvelope
	( ECS_SPRITE_INTERFACE * instance, const double * pbzEnvelope, int nCount )
{
	EBezierCurves<double>	bzCurve ;
	bzCurve.SetCount( nCount ) ;
	for ( int i = 0; i < nCount; i ++ )
	{
		bzCurve[i] = pbzEnvelope[i] ;
	}
	return	SpriteFromPlugin(instance)->SetBlendingEnvelope( bzCurve ) ;
}

ESLError __stdcall ECSSprite::PIC_SetBezierCurve
	( ECS_SPRITE_INTERFACE * instance,
		const E3D_VECTOR_2D * pCurve, int nCurveCount,
		const double * pRev, int nRevCount,
		const E3D_VECTOR_2D * pZoom, int nZoomCount )
{
	int			i ;
	EBezierCurves<E3D_VECTOR_2D>	bzCurve, *pbzCurve = NULL ;
	EBezierCurves<double>			bzRev, *pbzRev = NULL ;
	EBezierCurves<E3D_VECTOR_2D>	bzZoom, *pbzZoom = NULL ;
	if ( pCurve != NULL )
	{
		bzCurve.SetCount( nCurveCount ) ;
		for ( i = 0; i < nCurveCount; i ++ )
		{
			bzCurve[i] = pCurve[i] ;
		}
		pbzCurve = &bzCurve ;
	}
	if ( pZoom != NULL )
	{
		bzZoom.SetCount( nZoomCount ) ;
		for ( i = 0; i < nZoomCount; i ++ )
		{
			bzZoom[i] = pZoom[i] ;
		}
		pbzZoom = &bzZoom ;
	}
	if ( pRev != NULL )
	{
		bzRev.SetCount( nRevCount ) ;
		for ( i = 0; i < nRevCount; i ++ )
		{
			bzRev[i] = pRev[i] ;
		}
		pbzRev = &bzRev ;
	}
	return	SpriteFromPlugin(instance)->
				SetBezierCurve( pbzCurve, pbzRev, pbzZoom ) ;
}

ESLError __stdcall ECSSprite::PIC_BeginActivation
	( ECS_SPRITE_INTERFACE * instance,
		const unsigned int nDurationTime[],
			int nDurationCount, int nActionType  )
{
	return	SpriteFromPlugin(instance)->
				BeginActivation( nDurationTime, nDurationCount, NULL, nActionType ) ;
}

ESLError __stdcall ECSSprite::PIC_FlushActivation
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->FlushActivation( ) ;
}

ESLError __stdcall ECSSprite::PIC_CancelActivation
	( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->CancelActivation( ) ;
}

int __stdcall ECSSprite::PIC_IsActivation( ECS_SPRITE_INTERFACE * instance )
{
	return	SpriteFromPlugin(instance)->IsActivation( ) ;
}

