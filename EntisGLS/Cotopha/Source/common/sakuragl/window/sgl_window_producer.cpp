
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/window/sgl_window_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 汎用ウィンドウ・表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowViewProducer, ESLObject )

// 論理座標→物理ビュー座標変換
//////////////////////////////////////////////////////////////////////////////
void SGLWindowViewProducer::VirtualToPhysicalPosition( S2DDVector& vPos )
{
	SGLAffine	affine ;
	GetAffineVirtualToPhysical( affine ) ;
	//
	affine.TransformVectors( &vPos, &vPos, 1 ) ;
}

// 物理ビュー座標→論理座標変換
//////////////////////////////////////////////////////////////////////////////
void SGLWindowViewProducer::PhysicalToVirtualPosition( S2DDVector& vPos )
{
	SGLAffine	affine, iaffine ;
	GetAffineVirtualToPhysical( affine ) ;
	//
	iaffine.InverseOf( affine ) ;
	iaffine.TransformVectors( &vPos, &vPos, 1 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ウィンドウ更新タイミング同期用インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SGLWindowViewSynchronizer )



//////////////////////////////////////////////////////////////////////////////
// 非ウィンドウ・表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSecondaryViewProducer, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSecondaryViewProducer::SGLSecondaryViewProducer( void )
{
	m_matPerspective[0] = S4DMatrix( 1, 1, 1, 1 ) ;
	m_matPerspective[1] = S4DMatrix( 1, 1, 1, 1 ) ;
}

// レンダリング透視変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLSecondaryViewProducer::SetMainPerspectiveMatrix
	( RenderContext::StereoViewIndex sviIndex, const S4DMatrix& matPers )
{
	if ( (sviIndex == RenderContext::stereoViewRight)
		|| (sviIndex == RenderContext::stereoViewLeft) )
	{
		m_matPerspective[sviIndex] = matPers ;
	}
	else
	{
		m_matPerspective[RenderContext::stereoViewRight] = matPers ;
		m_matPerspective[RenderContext::stereoViewLeft] = matPers ;
	}
}

// 現在の描画対象取得
//////////////////////////////////////////////////////////////////////////////
SGLSecondaryViewProducer * SGLSecondaryViewProducer::GetCurrent( void )
{
	return	ESLTypeCast<SGLSecondaryViewProducer>
				( SThread::GetLocalStorageAs
					( L"SGLSecondaryViewProducer::Current" ) ) ;
}

// 現在の描画対象設定
//////////////////////////////////////////////////////////////////////////////
void SGLSecondaryViewProducer::SetCurrent( SGLSecondaryViewProducer * psvp )
{
	SThread::SetLocalStorageAs
		( L"SGLSecondaryViewProducer::Current", psvp ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ウィンドウ描画フレームワーク
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowViewFramework, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowViewFramework::SGLWindowViewFramework
		( SGLAbstractWindow * pWnd, SGLWindowViewProducer * pwvp )
	: m_pWnd( pWnd ), m_pViewProducer( pwvp )
{
	m_pVSyncSecondaryView = NULL ;
	m_pCurrentView = NULL ;
	m_flagsExFrame = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowViewFramework::~SGLWindowViewFramework( void )
{
}

// 表示インターフェース変更
//////////////////////////////////////////////////////////////////////////////
SGLWindowViewProducer *
	SGLWindowViewFramework::ChangeWindowViewProducer( SGLWindowViewProducer * pwvp )
{
	SGLWindowViewProducer *	pwvpOld = m_pViewProducer.Detach() ;
	m_pViewProducer = pwvp ;
	return	pwvpOld ;
}

// セカンダリビュー追加
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowViewFramework::AttachSecondaryView
		( SGLSecondaryViewProducer * psvp, bool fVSync )
{
	if ( psvp == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = sglErrFailed ;
	m_pWnd->Lock() ;
	if ( m_aSecondaryViews.FindPtr( psvp ) < 0 )
	{
		m_aSecondaryViews.Add( psvp ) ;
		if ( fVSync )
		{
			m_pVSyncSecondaryView = psvp ;
		}
		err = sglErrSuccess ;
	}
	m_pWnd->Unlock() ;
	return	err ;
}

// セカンダリビュー削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowViewFramework::DetachSecondaryView( SGLSecondaryViewProducer * psvp )
{
	SGLError	err = sglErrFailed ;
	m_pWnd->Lock() ;
	ssize_t	i = m_aSecondaryViews.FindPtr( psvp ) ;
	if ( i >= 0 )
	{
		m_aSecondaryViews.RemoveAt( (size_t) i ) ;
		//
		if ( m_pVSyncSecondaryView == psvp )
		{
			m_pVSyncSecondaryView = NULL ;
		}
		if ( m_pCurrentView == psvp )
		{
			m_pCurrentView = NULL ;
		}
		err = sglErrSuccess ;
	}
	m_pWnd->Unlock() ;
	return	err ;
}

// VSync ビュー設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowViewFramework::SetVSyncSecondaryView
		( SGLSecondaryViewProducer * psvp, bool fVSync )
{
	SGLError	err = sglErrFailed ;
	m_pWnd->Lock() ;
	if ( (psvp == NULL) && fVSync )
	{
		m_pVSyncSecondaryView = NULL ;
		err = sglErrSuccess ;
	}
	else
	{
		ssize_t	i = m_aSecondaryViews.FindPtr( psvp ) ;
		if ( i >= 0 )
		{
			if ( fVSync )
			{
				m_pVSyncSecondaryView = psvp ;
			}
			else if ( m_pVSyncSecondaryView == psvp )
			{
				m_pVSyncSecondaryView = NULL ;
			}
			err = sglErrSuccess ;
		}
	}
	m_pWnd->Unlock() ;
	return	err ;
}

// 仮想ディスプレイ・有効画面外枠表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowViewFramework::SetExteriorBackgroundFrame
	( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
		SGLImageObject* pLeft, SGLImageObject* pRight,
		SGLImageObject* pUpper, SGLImageObject* pUnder )
{
	m_flagsExFrame = nFlags ;
	m_rgbExColor.ui32 = rgbColor ;
	m_refExFrameTile = pTile ;
	m_refExFrameLeft = pLeft ;
	m_refExFrameRight = pRight ;
	m_refExFrameUpper = pUpper ;
	m_refExFrameUnder = pUnder ;
	return	sglErrSuccess ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowViewFramework::DrawWindow
	( SGLAbstractWindow * pWnd,
		bool fOnWinThread, const SGLImageRect * pWindow,
		SGLImageObject * pImage,
		SGLImageObject * pZBuffer, SGLImageObject * pStereoLeft )
{
	pWnd->LockTrace( __FILE__, __LINE__ ) ;
	SGLPaintInterface *	pPaint = pWnd->GetPaintInterface() ;
	if ( pPaint != NULL )
	{
		pPaint->OnPrepareFrame( pWnd ) ;
	}
	SGLPaintInterface *	pDirectPaint = pWnd->GetDirectPaintInterface() ;
	if ( pDirectPaint != NULL )
	{
		pDirectPaint->OnPrepareFrame( pWnd ) ;
	}
	bool	fWithoutPrimary = false ;
	if ( m_pVSyncSecondaryView && m_pVSyncSecondaryView->IsVisibleView() )
	{
		fWithoutPrimary = m_pVSyncSecondaryView->DoesDrawToPrimaryWindow() ;
	}
	for ( size_t i = 0; i < m_aSecondaryViews.GetLength(); i ++ )
	{
		SGLSecondaryViewProducer *	psvp = m_aSecondaryViews.GetAt( i ) ;
		if ( (psvp != NULL) && psvp->IsVisibleView() )
		{
			SGLSecondaryViewProducer::SetCurrent( psvp ) ;
			//
			RenderContext *	render = psvp->BeginDrawView( pWnd ) ;
			if ( render != NULL )
			{
				if ( pPaint != NULL )
				{
					if ( psvp->IsStereoDisplayMode() )
					{
						render->SelectParallaxView
									( RenderContext::stereoViewRight ) ;
						render->ResetTransformation() ;
						pPaint->OnPaint( pWnd, render ) ;
						//
						render->SelectParallaxView
									( RenderContext::stereoViewLeft ) ;
						render->ResetTransformation() ;
						pPaint->OnPaint( pWnd, render ) ;
					}
					else
					{
						render->SelectParallaxView
									( RenderContext::stereoViewAuto ) ;
						render->ResetTransformation() ;
						pPaint->OnPaint( pWnd, render ) ;
					}
				}
				psvp->EndDrawView( pWnd, render ) ;
			}
			//
			SGLSecondaryViewProducer::SetCurrent( NULL ) ;
		}
	}
	if ( !fWithoutPrimary )
	{
		SGLWindowViewProducer *	pwvp = m_pViewProducer ;
		if ( pwvp != NULL )
		{
			SGLSecondaryViewProducer::SetCurrent( NULL ) ;
			//
			// 通常描画ハンドラ
			//
			SGLImageObject * pZBufferLeft = NULL ;
			if ( pStereoLeft != NULL )
			{
				pZBufferLeft = pZBuffer ;
			}
			RenderContext *	render =
					pwvp->BeginDrawView
						( pWnd, fOnWinThread, pWindow,
							pImage, pZBuffer, pStereoLeft, pZBufferLeft ) ;
			if ( render != NULL )
			{
				if ( pPaint != NULL )
				{
					if ( pwvp->IsStereoDisplayMode() )
					{
						render->SelectParallaxView
								( RenderContext::stereoViewRight ) ;
						render->ResetTransformation() ;
						pPaint->OnPaint( pWnd, render ) ;
						//
						render->SelectParallaxView
								( RenderContext::stereoViewLeft ) ;
						render->ResetTransformation() ;
						pPaint->OnPaint( pWnd, render ) ;
					}
					else
					{
						render->SelectParallaxView
								( RenderContext::stereoViewAuto ) ;
						render->ResetTransformation() ;
						pPaint->OnPaint( pWnd, render ) ;
					}
				}
				pwvp->EndDrawView( pWnd, render, fOnWinThread ) ;
			}
			//
			// 直接描画ハンドラ
			//
			render = pwvp->BeginDirectView
						( pWnd, fOnWinThread, pWindow,
							pImage, pZBuffer, pStereoLeft, pZBufferLeft ) ;
			if ( render != NULL )
			{
				if ( pwvp->IsStereoDisplayMode() )
				{
					render->SelectParallaxView
								( RenderContext::stereoViewRight ) ;
					render->ResetTransformation() ;
					DrawExteriorFrame( pWnd, render ) ;
					if ( pDirectPaint != NULL )
					{
						pDirectPaint->OnPaint( pWnd, render ) ;
					}
					render->SelectParallaxView
								( RenderContext::stereoViewLeft ) ;
					render->ResetTransformation() ;
					DrawExteriorFrame( pWnd, render ) ;
					if ( pDirectPaint != NULL )
					{
						pDirectPaint->OnPaint( pWnd, render ) ;
					}
				}
				else
				{
					render->SelectParallaxView
								( RenderContext::stereoViewAuto ) ;
					render->ResetTransformation() ;
					//
					DrawExteriorFrame( pWnd, render ) ;
					//
					if ( pDirectPaint != NULL )
					{
						pDirectPaint->OnPaint( pWnd, render ) ;
					}
				}
				pwvp->EndDirectView( pWnd, render, fOnWinThread ) ;
			}
		}
	}
	if ( pPaint != NULL )
	{
		pPaint->OnFinishedFrame( pWnd ) ;
	}
	if ( pDirectPaint != NULL )
	{
		pDirectPaint->OnFinishedFrame( pWnd ) ;
	}
	pWnd->Unlock() ;
}

// フレーム描画処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowViewFramework::DrawExteriorFrame
		( SGLAbstractWindow * pWnd, RenderContext * render )
{
	SGLImageRect	rctRender, rctDisplay ;
	if ( pWnd->GetInternalDisplayPosition( rctRender, rctDisplay ) )
	{
		return ;
	}
	if ( rctDisplay.x > 0 )
	{
		//
		// フレーム左側描画
		//
		if ( m_flagsExFrame & SGLAbstractWindow::exteriorFillColor )
		{
			render->FillRectangle
				( 0, 0, rctDisplay.x, rctRender.h, m_rgbExColor.ui32 ) ;
		}
		SGLImageObject *	pLeft = m_refExFrameLeft ;
		if ( pLeft == NULL )
		{
			pLeft = m_refExFrameTile ;
		}
		if ( pLeft != NULL )
		{
			double	scale = 1.0 ;
			if ( m_flagsExFrame & SGLAbstractWindow::exteriorStretch )
			{
				scale = (double) rctRender.h / pLeft->GetImageHeight() ;
			}
			DrawExteriorFrameImage
				( render, pLeft, 0, 0,
					rctDisplay.x, rctRender.h, scale, true, false ) ;
		}
	}
	if ( rctDisplay.x + rctDisplay.w < rctRender.w )
	{
		//
		// フレーム右側描画
		//
		int	left = rctDisplay.x + rctDisplay.w ;
		if ( m_flagsExFrame & SGLAbstractWindow::exteriorFillColor )
		{
			render->FillRectangle
				( left, 0, rctRender.w - left, rctRender.h, m_rgbExColor.ui32 ) ;
		}
		SGLImageObject *	pRight = m_refExFrameRight ;
		if ( pRight == NULL )
		{
			pRight = m_refExFrameTile ;
		}
		if ( pRight != NULL )
		{
			double	scale = 1.0 ;
			if ( m_flagsExFrame & SGLAbstractWindow::exteriorStretch )
			{
				scale = (double) rctRender.h / pRight->GetImageHeight() ;
			}
			DrawExteriorFrameImage
				( render, pRight, left, 0,
					rctRender.w, rctRender.h, scale, false, false ) ;
		}
	}
	if ( rctDisplay.y > 0 )
	{
		//
		// フレーム上側描画
		//
		if ( m_flagsExFrame & SGLAbstractWindow::exteriorFillColor )
		{
			render->FillRectangle
				( rctDisplay.x, 0,
					rctDisplay.w, rctDisplay.y, m_rgbExColor.ui32 ) ;
		}
		SGLImageObject *	pUpper = m_refExFrameUpper ;
		if ( pUpper == NULL )
		{
			pUpper = m_refExFrameTile ;
		}
		if ( pUpper != NULL )
		{
			double	scale = 1.0 ;
			if ( m_flagsExFrame & SGLAbstractWindow::exteriorStretch )
			{
				scale = (double) rctDisplay.w / pUpper->GetImageWidth() ;
			}
			DrawExteriorFrameImage
				( render, pUpper, rctDisplay.x, 0,
					rctDisplay.x + rctDisplay.w, rctDisplay.y, scale, false, true ) ;
		}
	}
	if ( rctDisplay.y + rctDisplay.h < rctRender.h )
	{
		//
		// フレーム下側描画
		//
		int	top = rctDisplay.y + rctDisplay.h ;
		if ( m_flagsExFrame & SGLAbstractWindow::exteriorFillColor )
		{
			render->FillRectangle
				( rctDisplay.x, top,
					rctDisplay.w, rctRender.h - top, m_rgbExColor.ui32 ) ;
		}
		SGLImageObject *	pUnder = m_refExFrameUnder ;
		if ( pUnder == NULL )
		{
			pUnder = m_refExFrameTile ;
		}
		if ( pUnder != NULL )
		{
			double	scale = 1.0 ;
			if ( m_flagsExFrame & SGLAbstractWindow::exteriorStretch )
			{
				scale = (double) rctDisplay.w / pUnder->GetImageWidth() ;
			}
			DrawExteriorFrameImage
				( render, pUnder, rctDisplay.x, top,
					rctDisplay.x + rctDisplay.w, rctRender.h, scale, false, false ) ;
		}
	}
}

void SGLWindowViewFramework::DrawExteriorFrameImage
	( RenderContext * render, SGLImageObject * pImage,
		int left, int top, int right, int bottom,
		double scale, bool flagLeft, bool flagUpper )
{
	double	xLeft = left ;
	double	yTop = top ;
	SGLSize	sizeImage = pImage->GetImageSize() ;
	double	xStep = scale * sizeImage.w ;
	double	yStep = scale * sizeImage.h ;
	if ( flagLeft )
	{
		xLeft = right - xStep ;
		while ( xLeft > left )
		{
			xLeft -= xStep ;
		}
	}
	if ( flagUpper )
	{
		yTop = bottom - yStep ;
		while ( yTop > top )
		{
			yTop -= yStep ;
		}
	}
	for ( double y = yTop; y + 0.1 < bottom; y += yStep )
	{
		for ( double x = xLeft; x + 0.1 < right; x += xStep )
		{
			SGLPaintParam	ppPaint ;
			SGLAffine		affine ;
			ppPaint.SetAffine( affine, x, y, 0, 0, scale, scale ) ;
			render->DrawImage( ppPaint, pImage ) ;
		}
	}
}

// 描画反映処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowViewFramework::FlipView
	( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread )
{
	bool	fWithoutPrimary = false ;
	if ( m_pVSyncSecondaryView )
	{
		if ( m_pVSyncSecondaryView->IsVisibleView() )
		{
			fWithoutPrimary = m_pVSyncSecondaryView->DoesDrawToPrimaryWindow() ;
			m_pVSyncSecondaryView->FlipView( pWnd, true ) ;
		}
		fVSync = false ;
	}
	if ( !fWithoutPrimary )
	{
		SGLWindowViewProducer *	pwvp = m_pViewProducer ;
		if ( pwvp != NULL )
		{
			pwvp->FlipView( pWnd, fVSync, fOnWinThread ) ;
		}
	}
	for ( size_t i = 0; i < m_aSecondaryViews.GetLength(); i ++ )
	{
		SGLSecondaryViewProducer *	psvp = m_aSecondaryViews.GetAt( i ) ;
		if ( (psvp != NULL)
			&& (m_pVSyncSecondaryView != psvp) )
		{
			psvp->FlipView( pWnd, false ) ;
		}
	}
	m_pCurrentView = NULL ;
	if ( fWithoutPrimary )
	{
//		SGLSecondaryViewProducer::SetCurrent( m_pVSyncSecondaryView ) ;
//		m_pCurrentView = m_pVSyncSecondaryView ;
	}
}

// 現在のビュー取得（セカンダリビューがメインウィンドウに描画している場合）
//////////////////////////////////////////////////////////////////////////////
SGLSecondaryViewProducer * SGLWindowViewFramework::GetCurrentView( void ) const
{
	return	m_pCurrentView ;
}



//////////////////////////////////////////////////////////////////////////////
// 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowDisplayMethod, ESLObject )
ESL_IMPLEMENT_CLASS_INFO
		( SakuraGL::SGLWindowDisplayMethodProducer, SGLWindowViewProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowDisplayMethodProducer::SGLWindowDisplayMethodProducer
		( SGLAbstractWindow * pWnd, SGLWindowViewProducer * pwvpView )
	: m_pWnd( pWnd ), m_pwdmDisplay( NULL ), m_pwvpView( pwvpView )
{
	if ( pwvpView != NULL )
	{
//		pwvpView->EnableLayeredWindow( pWnd, true ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowDisplayMethodProducer::~SGLWindowDisplayMethodProducer( void )
{
	delete	m_pwdmDisplay ;
	m_pwdmDisplay = NULL ;
	delete	m_pwvpView ;
	m_pwvpView = NULL ;
}

// SGLWindowViewProducer 分離
//////////////////////////////////////////////////////////////////////////////
SGLWindowViewProducer * SGLWindowDisplayMethodProducer::DetachWindowViewProducer( void )
{
	SGLWindowViewProducer *	pwvpView = m_pwvpView ;
	m_pwvpView = NULL ;
	return	pwvpView ;
}

// SGLWindowDisplayMethod 設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::SetDisplayMethod( SGLWindowDisplayMethod * pMethod )
{
	if ( m_pwdmDisplay != NULL )
	{
		m_pwdmDisplay->OnDetachedWindow( m_pWnd ) ;
	}
	m_pwdmDisplay = pMethod ;
	if ( m_pwdmDisplay != NULL )
	{
		m_pwdmDisplay->OnAttachedWindow( m_pWnd ) ;
		m_pwdmDisplay->OnChangePhysicalViewSize
			( m_pWnd, (uint32_t) m_sizePhysical.w, (uint32_t) m_sizePhysical.h ) ;
		m_pwdmDisplay->OnMovedWindow( m_pWnd ) ;
	}
}

// 対応 SGLWindowDisplayMethod の生成
//////////////////////////////////////////////////////////////////////////////
SGLWindowDisplayMethod *
	SGLWindowDisplayMethodProducer::NewDisplayMethod( const wchar_t * pszMethodID )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( SString::Compare( pszMethodID, Window::Stereo3D::AnaglyphView ) == 0 )
	{
		return	new SGLAnaglyphGDIDisplayMethod ;
	}
	if ( SString::Compare( pszMethodID, Window::Stereo3D::InterleavedView ) == 0 )
	{
		return	new SGLInterleavedGDIDisplayMethod ;
	}
	if ( SString::Compare( pszMethodID, Window::Stereo3D::NVStereoBLT ) == 0 )
	{
		return	new SGLNvidia3DVisionDisplayMethod ;
	}
#endif
	return	NULL ;
}

// 対応機能フラグ
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLWindowDisplayMethodProducer::GetCapacityFlags( void ) const
{
	ESLAssert( m_pwvpView != NULL ) ;
	uint64_t	nCapsFlags = m_pwvpView->GetCapacityFlags() ;
	return	nCapsFlags ;
}

// 論理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::OnChangeVirtualViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	ESLAssert( m_pwvpView != NULL ) ;
	m_pwvpView->OnChangeVirtualViewSize( pWnd, nWidth, nHeight ) ;
	//
	m_sizeVirtual.w = (int32_t) nWidth ;
	m_sizeVirtual.h = (int32_t) nHeight ;
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	ESLAssert( m_pwvpView != NULL ) ;
	m_pwvpView->OnChangePhysicalViewSize( pWnd, nWidth, nHeight ) ;
	//
	if ( m_pwdmDisplay != NULL )
	{
		m_pwdmDisplay->OnChangePhysicalViewSize( pWnd, nWidth, nHeight ) ;
	}
	//
	m_sizePhysical.w = (int32_t) nWidth ;
	m_sizePhysical.h = (int32_t) nHeight ;
	//
	m_imgViewRight.CreateImage( nWidth, nHeight, formatImageABGR, 32 ) ;
	m_imgViewLeft.CreateImage( nWidth, nHeight, formatImageABGR, 32 ) ;
	m_imgZBuffer.CreateImage
		( nWidth, nHeight,
			formatImageDepth, 32, SGLImageObject::bufferOnDeviceOnly ) ;
}

// ウィンドウの位置が変化した
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::OnMovedWindow( SGLAbstractWindow * pWnd )
{
	ESLAssert( m_pwvpView != NULL ) ;
	m_pwvpView->OnMovedWindow( pWnd ) ;
	//
	if ( m_pwdmDisplay != NULL )
	{
		m_pwdmDisplay->OnMovedWindow( pWnd ) ;
	}
}

// ウィンドウに関連付けられた（作成された）
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::OnAttachedWindow( SGLAbstractWindow * pWnd )
{
	ESLAssert( m_pwvpView != NULL ) ;
	m_pwvpView->OnAttachedWindow( pWnd ) ;
	//
	if ( m_pwdmDisplay != NULL )
	{
		m_pwdmDisplay->OnAttachedWindow( pWnd ) ;
	}
}

// ウィンドウから分離された（ウィンドウが破棄される）
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::OnDetachedWindow( SGLAbstractWindow * pWnd )
{
	ESLAssert( m_pwvpView != NULL ) ;
	m_pwvpView->OnDetachedWindow( pWnd ) ;
	//
	if ( m_pwdmDisplay != NULL )
	{
		m_pwdmDisplay->OnDetachedWindow( pWnd ) ;
	}
}

// フルスクリーンモードへ変更する
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowDisplayMethodProducer::OnChangeFullscreen
	( SGLAbstractWindow * pWnd,
		uint32_t nBitsPerPixel, uint32_t nFrequency,
		bool flagChangePhysicalMode, const wchar_t * pszDisplayName )
{
	if ( m_pwdmDisplay != NULL )
	{
		return	m_pwdmDisplay->OnChangeFullscreen
			( pWnd, nBitsPerPixel, nFrequency,
				flagChangePhysicalMode, pszDisplayName ) ;
	}
	return	false ;
}

// フルスクリーンモードから復帰する
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::OnRestoreFullscreen( SGLAbstractWindow * pWnd )
{
	if ( m_pwdmDisplay != NULL )
	{
		m_pwdmDisplay->OnRestoreFullscreen( pWnd ) ;
	}
}

// 論理ビュー表示座標取得
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::GetInternalViewPosition( SGLImageRect& rctVirtualView )
{
	ESLAssert( m_pwvpView != NULL ) ;
	m_pwvpView->GetInternalViewPosition( rctVirtualView ) ;
}

// 物理ビュー表示領域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowDisplayMethodProducer::GetExternalViewPosition( SGLImageRect& rctPhysicalView )
{
	ESLAssert( m_pwvpView != NULL ) ;
	return	m_pwvpView->GetExternalViewPosition( rctPhysicalView ) ;
}

// 論理座標→物理ビュー座標変換行列取得
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::GetAffineVirtualToPhysical( SGLAffine& affine )
{
	ESLAssert( m_pwvpView != NULL ) ;
	m_pwvpView->GetAffineVirtualToPhysical( affine ) ;
}

// 描画スレッドの関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowDisplayMethodProducer::AttachViewThread( SGLAbstractWindow * pWnd )
{
	ESLAssert( m_pwvpView != NULL ) ;
	return	m_pwvpView->AttachViewThread( pWnd ) ;
}

// 描画スレッドの関連付け解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowDisplayMethodProducer::DetachViewThread( SGLAbstractWindow * pWnd )
{
	ESLAssert( m_pwvpView != NULL ) ;
	return	m_pwvpView->DetachViewThread( pWnd ) ;
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLWindowDisplayMethodProducer::BeginDrawView
		( SGLAbstractWindow * pWnd,
			bool fOnWinThread,
			const SGLImageRect * pWindow,
			SGLImageObject * pImage, SGLImageObject * pZBuffer,
			SGLImageObject * pImageLeft, SGLImageObject * pZBufferLeft )
{
	ESLAssert( m_pwvpView != NULL ) ;
	if ( m_pwdmDisplay == NULL )
	{
		return	m_pwvpView->BeginDrawView
			( pWnd, fOnWinThread, pWindow,
				pImage, pZBuffer, pImageLeft, pZBufferLeft ) ;
	}
	pImageLeft = NULL ;
	pZBufferLeft = NULL ;
	if ( m_pwdmDisplay && m_pwdmDisplay->IsStereoDisplayMode() )
	{
		pImageLeft = &m_imgViewLeft ;
		pZBufferLeft = &m_imgZBuffer ;
	}
	return	m_pwvpView->BeginDrawView
		( pWnd, fOnWinThread, pWindow,
			&m_imgViewRight, &m_imgZBuffer, pImageLeft, pZBufferLeft ) ;
}

// 描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::EndDrawView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	ESLAssert( m_pwvpView != NULL ) ;
	return	m_pwvpView->EndDrawView( pWnd, render, fOnWinThread ) ;
}

// 直接描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLWindowDisplayMethodProducer::BeginDirectView
		( SGLAbstractWindow * pWnd,
			bool fOnWinThread,
			const SGLImageRect * pWindow,
			SGLImageObject * pImage, SGLImageObject * pZBuffer,
			SGLImageObject * pImageLeft, SGLImageObject * pZBufferLeft )
{
	ESLAssert( m_pwvpView != NULL ) ;
	if ( m_pwdmDisplay == NULL )
	{
		return	m_pwvpView->BeginDirectView
			( pWnd, fOnWinThread, pWindow,
				pImage, pZBuffer, pImageLeft, pZBufferLeft ) ;
	}
	pImageLeft = NULL ;
	pZBufferLeft = NULL ;
	if ( m_pwdmDisplay && m_pwdmDisplay->IsStereoDisplayMode() )
	{
		pImageLeft = &m_imgViewLeft ;
		pZBufferLeft = &m_imgZBuffer ;
	}
	return	m_pwvpView->BeginDirectView
		( pWnd, fOnWinThread, pWindow,
			&m_imgViewRight, &m_imgZBuffer, pImageLeft, pZBufferLeft ) ;
}

// 直接描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::EndDirectView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	ESLAssert( m_pwvpView != NULL ) ;
	return	m_pwvpView->EndDirectView( pWnd, render, fOnWinThread ) ;
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowDisplayMethodProducer::FlipView
	( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread )
{
	if ( m_pwdmDisplay != NULL )
	{
		SGLImage *	pImageLeft = NULL ;
		if ( m_pwdmDisplay && m_pwdmDisplay->IsStereoDisplayMode() )
		{
			pImageLeft = &m_imgViewLeft ;
		}
		m_pwdmDisplay->FlipView( pWnd, &m_imgViewRight, pImageLeft ) ;
	}
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowDisplayMethodProducer::EnableZBuffer
	( SGLAbstractWindow * pWnd, bool flagZBuffer )
{
	return	sglErrSuccess ;
}

// レイヤードウィンドウ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowDisplayMethodProducer::EnableLayeredWindow
	( SGLAbstractWindow * pWnd, bool flagLayeredWindow )
{
	return	sglErrSuccess ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowDisplayMethodProducer::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	if ( m_pwdmDisplay != NULL )
	{
		return	m_pwdmDisplay->SetStereoDisplayMode
							( pWnd, pszMethodID, nParam ) ;
	}
	return	sglErrFailed ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowDisplayMethodProducer::IsStereoDisplayMode( void )
{
	if ( m_pwdmDisplay != NULL )
	{
		return	m_pwdmDisplay->IsStereoDisplayMode() ;
	}
	return	false ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowDisplayMethodProducer::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
	if ( m_pwdmDisplay != NULL )
	{
		return	m_pwdmDisplay->IsSupportedStereoDisplayMode( pWnd, pszMethodID ) ;
	}
	return	false ;
}

// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLWindowDisplayMethodProducer::GetStandardDisplaySize( void ) const
{
	ESLAssert( m_pwvpView != NULL ) ;
	return	m_pwvpView->GetStandardDisplaySize() ;
}

// レンダリングデバイス取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * SGLWindowDisplayMethodProducer::GetRenderDevice( void )
{
	ESLAssert( m_pwvpView != NULL ) ;
	return	m_pwvpView->GetRenderDevice() ;
}


