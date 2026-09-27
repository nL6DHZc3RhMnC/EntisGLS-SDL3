
#include <gls.h>
#include <sakuragl/sgl_window.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 標準レンダリング実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLStandardRenderContext, SGLRenderPolygonInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLStandardRenderContext::SGLStandardRenderContext( void )
	: SGLRenderPolygonInterface( ::eglCreateRenderPolygon() )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLStandardRenderContext::~SGLStandardRenderContext( void )
{
	if ( m_hRender != NULL )
	{
		m_hRender->Release() ;
		m_hRender = NULL ;
	}
}


#if	defined(USE_ENTIS_GLS3_WINDOW)

//////////////////////////////////////////////////////////////////////////////
// SGLCSWindow
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindow::SGLCSWindow, ECSWindow )

// ウィンドウインターフェースオブジェクトを作成する
//////////////////////////////////////////////////////////////////////////////
ECSWindow::EInterface * SGLWindow::SGLCSWindow::OnCreateInterface( void )
{
	return	new SGLWindow::SGLInterface ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SGLCSWindow::AfterRefreshRectWith3DView
	( HEGL_RENDER_POLYGON hRender, const ESprite::VIEW3D_INFO * pv3dInfo )
{
	SGLWindow *	pSGLWnd = m_pWnd ;
	if ( pSGLWnd == NULL )
	{
		return ;
	}
	EWindowSpriteInterface *	pInterface = GetInterface() ;
	if ( pInterface )
	{
		pInterface->UnlockOnMsgHandler() ;
	}
	SSystem::Lock() ;
	pSGLWnd->m_render.m_pSprite = this ;
	pSGLWnd->m_render.AttachRenderPolygon( hRender ) ;
	//
	S3DRenderContextInterface::StereoViewIndex
			sviView = S3DRenderContextInterface::stereoViewAuto ;
	if ( pv3dInfo != NULL )
	{
		if ( pv3dInfo->nViewIndex == ESprite::sviStereoViewRight )
		{
			sviView = S3DRenderContextInterface::stereoViewRight ;
		}
		else if ( pv3dInfo->nViewIndex == ESprite::sviStereoViewRight )
		{
			sviView = S3DRenderContextInterface::stereoViewLeft ;
		}
	}
	//
	SGLPaintInterface *	pPaint = pSGLWnd->GetPaintInterface() ;
	if ( pPaint != NULL )
	{
		pSGLWnd->m_render.ResetTransformation() ;
		pSGLWnd->m_render.SelectParallaxView( sviView ) ;
		pPaint->OnPaint( pSGLWnd, &(pSGLWnd->m_render) ) ;
		pSGLWnd->m_render.Flush() ;
	}
	pPaint = pSGLWnd->GetDirectPaintInterface() ;
	if ( pPaint != NULL )
	{
		pSGLWnd->m_render.ResetTransformation() ;
		pSGLWnd->m_render.SelectParallaxView( sviView ) ;
		pPaint->OnPaint( pSGLWnd, &(pSGLWnd->m_render) ) ;
		pSGLWnd->m_render.Flush() ;
	}
	SSystem::Unlock() ;
	if ( pInterface )
	{
		pInterface->RelockOnMsgHandler() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウインターフェース SGLWindow::SGLInterface
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindow::SGLInterface, EInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindow::SGLInterface::SGLInterface( void )
{
	m_strCursorID = L"IDC_ARROW" ;
	m_bytLeadChar = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindow::SGLInterface::~SGLInterface( void )
{
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
LRESULT SGLWindow::SGLInterface::WindowProc
	( EWindow * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	SGLCSWindow *	pSGLCSWnd = ESLTypeCast<SGLCSWindow>( m_pWnd ) ;
	if ( pSGLCSWnd != NULL )
	{
		SGLWindow *	pSGLWnd = pSGLCSWnd->m_pWnd ;
		if ( pSGLWnd != NULL )
		{
			if ( (uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) )
			{
				SSystem::Lock() ;
				SGLMouseInterface *
					pMouse = pSGLWnd->GetDirectMouseInterface() ;
				bool	fProcessed = false ;
				if ( pMouse != NULL )
				{
					int64_t	nFlags = 0 ;
					int		xPos, yPos, zDelta ;
					POINT	ptCursor ;
					xPos = (SWORD) lParam ;
					yPos = (SWORD) (lParam >> 16) ;
					switch ( uMsg )
					{
					case	WM_MOUSEMOVE:
						fProcessed =
							pMouse->OnMouseMove( pSGLWnd, xPos, yPos, 0 ) ;
						break ;
					case	WM_MOUSEWHEEL:
						ptCursor.x = xPos ;
						ptCursor.y = yPos ;
						pWnd->ScreenToClient( &ptCursor ) ;
						//
						zDelta = (SWORD) (wParam >> 16) ;
						zDelta = zDelta * SGLMouseInterface::WheelDeltaUnit / WHEEL_DELTA ;
						fProcessed =
							pMouse->OnMouseWheel
								( pSGLWnd, zDelta, ptCursor.x, ptCursor.y, 0 ) ;
						break ;
					case	WM_LBUTTONDOWN:
						nFlags = (SGLMouseInterface::LeftButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDown
								( pSGLWnd, xPos, yPos, nFlags ) ;
						break ;
					case	WM_LBUTTONUP:
						nFlags = (SGLMouseInterface::LeftButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonUp
								( pSGLWnd, xPos, yPos, nFlags ) ;
						break ;
					case	WM_LBUTTONDBLCLK:
						nFlags = (SGLMouseInterface::LeftButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDblClk
								( pSGLWnd, xPos, yPos, nFlags ) ;
						break ;
					case	WM_RBUTTONDOWN:
						nFlags = (SGLMouseInterface::RightButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDown
								( pSGLWnd, xPos, yPos, nFlags ) ;
						break ;
					case	WM_RBUTTONUP:
						nFlags = (SGLMouseInterface::RightButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonUp
								( pSGLWnd, xPos, yPos, nFlags ) ;
						break ;
					case	WM_RBUTTONDBLCLK:
						nFlags = (SGLMouseInterface::RightButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDblClk
								( pSGLWnd, xPos, yPos, nFlags ) ;
						break ;
					}
				}
				pMouse = pSGLWnd->GetMouseInterface() ;
				if ( !fProcessed && (pMouse != NULL) )
				{
					int64_t		nFlags = 0 ;
					EGL_POINT	ptCursor ;
					int			zDelta ;
					ptCursor.x = (SWORD) lParam ;
					ptCursor.y = (SWORD) (lParam >> 16) ;
					switch ( uMsg )
					{
					case	WM_MOUSEMOVE:
						WindowToClient( ptCursor ) ;
						fProcessed =
							pMouse->OnMouseMove
								( pSGLWnd, ptCursor.x, ptCursor.y, 0 ) ;
						break ;
					case	WM_MOUSEWHEEL:
						ScreenToClient( ptCursor ) ;
						zDelta = (SWORD) (wParam >> 16) ;
						zDelta = zDelta * SGLMouseInterface::WheelDeltaUnit / WHEEL_DELTA ;
						fProcessed =
							pMouse->OnMouseWheel
								( pSGLWnd, zDelta, ptCursor.x, ptCursor.y, 0 ) ;
						break ;
					case	WM_LBUTTONDOWN:
						WindowToClient( ptCursor ) ;
						nFlags = (SGLMouseInterface::LeftButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDown
								( pSGLWnd, ptCursor.x, ptCursor.y, nFlags ) ;
						break ;
					case	WM_LBUTTONUP:
						WindowToClient( ptCursor ) ;
						nFlags = (SGLMouseInterface::LeftButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonUp
								( pSGLWnd, ptCursor.x, ptCursor.y, nFlags ) ;
						break ;
					case	WM_LBUTTONDBLCLK:
						WindowToClient( ptCursor ) ;
						nFlags = (SGLMouseInterface::LeftButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDblClk
								( pSGLWnd, ptCursor.x, ptCursor.y, nFlags ) ;
						break ;
					case	WM_RBUTTONDOWN:
						WindowToClient( ptCursor ) ;
						nFlags = (SGLMouseInterface::RightButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDown
								( pSGLWnd, ptCursor.x, ptCursor.y, nFlags ) ;
						break ;
					case	WM_RBUTTONUP:
						WindowToClient( ptCursor ) ;
						nFlags = (SGLMouseInterface::RightButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonUp
								( pSGLWnd, ptCursor.x, ptCursor.y, nFlags ) ;
						break ;
					case	WM_RBUTTONDBLCLK:
						WindowToClient( ptCursor ) ;
						nFlags = (SGLMouseInterface::RightButtonID
										<< SGLMouseInterface::ButtonIDShifter) ;
						fProcessed =
							pMouse->OnButtonDblClk
								( pSGLWnd, ptCursor.x, ptCursor.y, nFlags ) ;
						break ;
					}
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_MOUSELEAVE )
			{
				SSystem::Lock() ;
				SGLMouseInterface *
					pMouse = pSGLWnd->GetDirectMouseInterface() ;
				if ( pMouse != NULL )
				{
					pMouse->OnMouseLeave( pSGLWnd, 0 ) ;
				}
				pMouse = pSGLWnd->GetMouseInterface() ;
				if ( pMouse != NULL )
				{
					pMouse->OnMouseLeave( pSGLWnd, 0 ) ;
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_PAINT )
			{
			}
			else if ( uMsg == WM_TIMER )
			{
				SSystem::Lock() ;
				SGLTimerInterface *	pTimer = pSGLWnd->GetTimerInterface() ;
				if ( pTimer != NULL )
				{
					if ( wParam == 1 )
					{
						pTimer->OnTimer( pSGLWnd, 1 ) ;
					}
				}
				SSystem::Unlock() ;
			}
			else if ( (uMsg >= WM_KEYFIRST) && (uMsg <= WM_KEYLAST) )
			{
				SSystem::Lock() ;
				SGLKeyInterface *
						pKey = pSGLWnd->GetKeyInterface() ;
				SGLCharInputInterface *
						pChar = pSGLWnd->GetCharInputInterface() ;
				bool	fProcessed = false ;
				switch ( uMsg )
				{
				case	WM_KEYDOWN:
					if ( pKey != NULL )
					{
						fProcessed =
							pKey->OnKeyDown
								( pSGLWnd, wParam, GetKeyContextFlag() ) ;
					}
					break ;
				case	WM_KEYUP:
					if ( pKey != NULL )
					{
						fProcessed =
							pKey->OnKeyUp
								( pSGLWnd, wParam, GetKeyContextFlag() ) ;
					}
					break ;
				case	WM_CHAR:
					if ( pChar != NULL )
					{
						if ( m_bytLeadChar != 0 )
						{
							char	ch[3] ;
							ch[0] = m_bytLeadChar ;
							ch[1] = (char) wParam ;
							ch[2] = 0 ;
							m_bytLeadChar = 0 ;
							//
							SString	strChar = ch ;
							fProcessed = pChar->OnChar
									( pSGLWnd, (uint16_t) strChar.GetAt(0) ) ;
						}
						else if ( IsDBCSLeadByte( (BYTE) wParam ) )
						{
							m_bytLeadChar = (BYTE) wParam ;
						}
						else if ( wParam & 0x80 )
						{
							char	ch[2] ;
							ch[0] = (char) wParam ;
							ch[1] = 0 ;
							//
							SString	strChar = ch ;
							fProcessed = pChar->OnChar
									( pSGLWnd, (uint16_t) strChar.GetAt(0) ) ;
						}
						else
						{
							fProcessed =
								pChar->OnChar( pSGLWnd, (uint16_t) wParam ) ;
						}
					}
					break ;
				}
				SSystem::Unlock() ;
				if ( fProcessed )
				{
					return	0 ;
				}
			}
			else if ( uMsg == WM_CLOSE )
			{
				SSystem::Lock() ;
				SGLCommandInterface *	pCmd = pSGLWnd->GetCommandInterface() ;
				if ( pCmd != NULL )
				{
					SString	strCmd = L"ID_APP_EXIT" ;
					if ( pCmd->OnCommand( pSGLWnd, strCmd, 0, 0 ) )
					{
						SSystem::Unlock() ;
						return	0 ;
					}
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_IME_CHAR )
			{
				SSystem::Lock() ;
				SGLCharInputInterface *
						pChar = pSGLWnd->GetCharInputInterface() ;
				if ( pChar != NULL )
				{
					if ( wParam & 0xFF80 )
					{
						char	ch[3] ;
						if ( wParam & 0xFF00 )
						{
							ch[0] = (char) ((wParam >> 8) & 0xFF) ;
							ch[1] = (char) (wParam & 0xFF) ;
							ch[2] = 0 ;
						}
						else
						{
							ch[0] = (char) (wParam & 0xFF) ;
							ch[1] = 0 ;
						}
						SString	strChar = ch ;
						if ( pChar->OnChar
								( pSGLWnd, (uint16_t) strChar.GetAt(0) ) )
						{
							SSystem::Unlock() ;
							return	0 ;
						}
					}
					else
					{
						if ( pChar->OnChar( pSGLWnd, (uint16_t) wParam ) )
						{
							SSystem::Unlock() ;
							return	0 ;
						}
					}
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_IME_STARTCOMPOSITION )
			{
				SSystem::Lock() ;
				SGLInputStartComposition	iscForm ;
				::eslFillMemory
					( &iscForm, 0, sizeof(SGLInputStartComposition) ) ;
				SGLCharInputInterface *
					pChar = pSGLWnd->GetCharInputInterface() ;
				if ( (pChar != NULL)
					&& pChar->OnStartComposition( pSGLWnd, iscForm ) )
				{
					HIMC	hIMC = ::ImmGetContext( *pWnd ) ;
					if ( iscForm.nFlags
						& (SGLInputStartComposition::flagPosition
								| SGLInputStartComposition::flagRectangle) )
					{
						COMPOSITIONFORM	cf ;
						::eslFillMemory( &cf, 0, sizeof(cf) ) ;
						if ( iscForm.nFlags
							& SGLInputStartComposition::flagPosition )
						{
							EGLPoint	ptPos
								( iscForm.ptStart.x, iscForm.ptStart.y ) ;
							ClientToWindow( ptPos ) ;
							//
							cf.dwStyle |= CFS_POINT ;
							cf.ptCurrentPos.x = ptPos.x ;
							cf.ptCurrentPos.y = ptPos.y ;
						}
						if ( iscForm.nFlags
							& SGLInputStartComposition::flagRectangle )
						{
							EGLPoint	ptPos0
								( iscForm.rctArea.x, iscForm.rctArea.y ) ;
							EGLPoint	ptPos1
								( iscForm.rctArea.x + iscForm.rctArea.w,
									iscForm.rctArea.y + iscForm.rctArea.h ) ;
							ClientToWindow( ptPos0 ) ;
							ClientToWindow( ptPos1 ) ;
							//
							cf.dwStyle |= CFS_RECT ;
							cf.rcArea.left = ptPos0.x ;
							cf.rcArea.top = ptPos0.y ;
							cf.rcArea.right = ptPos1.x ;
							cf.rcArea.bottom = ptPos1.y ;
						}
						::ImmSetCompositionWindow( hIMC, &cf ) ;
					}
					if ( iscForm.nFlags
							& SGLInputStartComposition::flagFont )
					{
						LOGFONT			lf ;
						SGLFontStyle	fs = iscForm.fsFontStyle ;
						EGLSize	szFont( fs.nSize, fs.nSize ) ;
						ClientToWindowSize( szFont ) ;
						fs.nSize = szFont.h ;
						fs.ToLogFont( lf ) ;
						::ImmSetCompositionFont( hIMC, &lf ) ;
					}
					::ImmReleaseContext( *pWnd, hIMC ) ;
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_IME_ENDCOMPOSITION )
			{
				SSystem::Lock() ;
				SGLCharInputInterface *
					pChar = pSGLWnd->GetCharInputInterface() ;
				if ( pChar != NULL )
				{
					pChar->OnEndComposition( pSGLWnd ) ;
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_IME_COMPOSITION )
			{
				SSystem::Lock() ;
				SGLCharInputInterface *
					pChar = pSGLWnd->GetCharInputInterface() ;
				if ( pChar != NULL )
				{
					DWORD	dwIndex = GCS_COMPSTR ;
					if ( lParam & GCS_RESULTSTR )
					{
						dwIndex = GCS_RESULTSTR ;
					}
					HIMC	hIMC = ::ImmGetContext( *pWnd ) ;
					LONG	nLen =
						::ImmGetCompositionStringW
							( hIMC, dwIndex, NULL, 0 ) ;
					nLen /= sizeof(wchar_t) ;
					//
					SString	strIME ;
					::ImmGetCompositionStringW
						( hIMC, dwIndex,
							strIME.LockBuffer( nLen ),
							nLen * sizeof(wchar_t) + 1 ) ;
					strIME.UnlockBuffer( nLen ) ;
					::ImmReleaseContext( *pWnd, hIMC ) ;
					//
					SGLInputCompositionString	icsComp ;
					::eslFillMemory
						( &icsComp, 0, sizeof(SGLInputCompositionString) ) ;
					if ( lParam & GCS_RESULTSTR )
					{
						icsComp.nFlags =
							SGLInputCompositionString::flagResult ;
					}
					icsComp.pszComposition = strIME ;
					icsComp.nStart = 0 ;
					icsComp.nCount = strIME.GetLength() ;
					//
					if ( pChar->OnCompositionString( pSGLWnd, icsComp ) )
					{
						if ( lParam & GCS_RESULTSTR )
						{
							SSystem::Unlock() ;
							return	0 ;
						}
					}
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_KILLFOCUS )
			{
				SSystem::Lock() ;
				SGLKeyInterface *	pKey = pSGLWnd->GetKeyInterface() ;
				if ( pKey != NULL )
				{
					pKey->OnKillFocus( pSGLWnd ) ;
				}
				SSystem::Unlock() ;
			}
			else if ( uMsg == WM_SETFOCUS )
			{
				SSystem::Lock() ;
				SGLKeyInterface *	pKey = pSGLWnd->GetKeyInterface() ;
				if ( pKey != NULL )
				{
					pKey->OnSetFocus( pSGLWnd ) ;
				}
				SSystem::Unlock() ;
			}
		}
	}
	return	EInterface::WindowProc( pWnd, uMsg, wParam, lParam ) ;
}

// マウスカーソルを設定する
//////////////////////////////////////////////////////////////////////////////
bool SGLWindow::SGLInterface::OnSetCursor( int xPos, int yPos )
{
	if ( m_pWnd && m_pWnd->IsShowCursor() )
	{
		SetMouseCursor( m_strCursorID ) ;
		return	true ;
	}
	else
	{
		::SetCursor( NULL ) ;
	}
	return	true ;
}

// マウスカーソルを変更
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SGLInterface::ChangeMouseCursor( const wchar_t * pwszCursorID )
{
	Lock() ;
	m_strCursorID = pwszCursorID ;
	Unlock() ;
}

// キーコンテキストフラグを取得する
//////////////////////////////////////////////////////////////////////////////
int64_t SGLWindow::SGLInterface::GetKeyContextFlag( void )
{
	int64_t	nFlags = 0 ;
	if ( ::GetKeyState( VK_CAPITAL ) & 0x01 )
	{
		nFlags |= vkeyContextCapital ;
	}
	if ( ::GetKeyState( VK_SHIFT ) & 0x80 )
	{
		nFlags |= vkeyContextShift ;
	}
	if ( ::GetKeyState( VK_CONTROL ) & 0x80 )
	{
		nFlags |= vkeyContextControl ;
	}
	if ( ::GetKeyState( VK_MENU ) & 0x80 )
	{
		nFlags |= vkeyContextMenu ;
	}
	return	nFlags ;
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ描画インターフェース SGLWindow::SGLRenderPolygon
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLWindow::SGLRenderPolygon, SGLRenderPolygonInterface )

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SGLRenderPolygon::SetPaintFlags( int64_t nFlags )
{
	SGLRenderPolygonInterface::SetPaintFlags( nFlags ) ;
	//
	if ( (m_pSprite != NULL) & (m_hRender != NULL) & (m_hDraw != NULL) )
	{
		m_pSprite->SetDrawFunctionFlags( m_hDraw->GetFunctionFlags() ) ;
		m_pSprite->SetRenderFunctionFlags( m_hRender->GetFunctionFlags() ) ;
	}
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SGLRenderPolygon::SetShadingFlag( uint64_t nShadingMethod )
{
	SGLRenderPolygonInterface::SetShadingFlag( nShadingMethod ) ;
	//
	if ( (m_pSprite != NULL) & (m_hRender != NULL) )
	{
		m_pSprite->SetRenderFunctionFlags( m_hRender->GetFunctionFlags() ) ;
	}
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SGLRenderPolygon::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	SGLError	err =
		SGLRenderPolygonInterface::SetProjectionScreen
						( vScreen, zScale, fpPixelAspect ) ;
	//
	if ( m_pSprite != NULL )
	{
		ESprite::VIEW3D_INFO	v3dInfo ;
		GetView3DInfo( v3dInfo ) ;
		m_pSprite->SetStereoViewInfo( v3dInfo ) ;
	}
	return	err ;
}

// 立体視視差設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SGLRenderPolygon::SetParallax( double xParallax, double zFocusRate )
{
	SGLRenderPolygonInterface::SetParallax( xParallax, zFocusRate ) ;
	//
	if ( m_pSprite != NULL )
	{
		ESprite::VIEW3D_INFO	v3dInfo ;
		GetView3DInfo( v3dInfo ) ;
		m_pSprite->SetStereoViewInfo( v3dInfo ) ;
	}
}

// ステレオ立体視パラメータ取得
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SGLRenderPolygon::GetView3DInfo( ESprite::VIEW3D_INFO& v3dInfo )
{
	S3DVector	vScreen ;
	double		zScale, fpPixelAspect ;
	GetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
	//
	v3dInfo.xParallax = GetParallax() ;
	v3dInfo.zFocus = vScreen.z ;
	v3dInfo.zOffset = 0 ;
	v3dInfo.nViewIndex = 0 ;
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindow, SGLAbstractWindow )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindow::SGLWindow( void )
{
	m_flagOptions = 0 ; // flagBlackBack ;
	m_cswnd.SetChangeDisplayModeFlag( false ) ;
	m_hRender = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindow::~SGLWindow( void )
{
	if ( m_hRender != NULL )
	{
		m_hRender->Release() ;
		m_hRender = NULL ;
	}
}

// アイコン取得
//////////////////////////////////////////////////////////////////////////////
HICON SGLWindow::LoadMainIcon( void )
{
	HICON			hIcon = NULL ;
	SEnvironmentInterface *	pEnv = SEnvironmentInterface::GetInstance() ;
	if ( pEnv != NULL )
	{
		SString	strIconID ;
		if ( pEnv->GetEnvironmentString( strIconID, L"script\\icon\\id" )
			|| pEnv->GetEnvironmentString( strIconID, L"script\\icon\\id" ) )
		{
			SArray<char>	bufIconID ;
			hIcon = ::LoadIcon
				( ::GetModuleHandle(NULL),
					strIconID.EncodeDefaultTo(bufIconID) ) ;
		}
		if ( (hIcon == NULL)
			&& (pEnv->GetEnvironmentString( strIconID, L"script\\icon\\src" )
				|| pEnv->GetEnvironmentString( strIconID, L"script\\icon\\src" )) )
		{
			SArray<char>	bufIconSrc ;
			hIcon = (HICON) ::LoadImage
				( NULL, strIconID.EncodeDefaultTo(bufIconSrc),
					IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE ) ;
		}
	}
	return	hIcon ;
}

// 仮想ディスプレイ開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CreateDisplay
	( const wchar_t * pszWindowName,
		CooperationMode mode,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	SString			strWindowName = pszWindowName ;
	SArray<char>	bufWindowName ;
	HICON			hIcon = LoadMainIcon() ;
	m_cswnd.AttachContext( &m_contextDummy ) ;
	m_cswnd.m_pWnd = this ;
	//
	ESLError	err =
		m_cswnd.CreateDisplay
			( strWindowName.EncodeDefaultTo(bufWindowName),
				(EGameWindow::CooperationLevel) mode,
				nWidth, nHeight, nBitsPerPixel, nFrequency, hIcon ) ;
	if ( err )
	{
		return	sglErrFailed ;
	}
	AddWindowToChain() ;
	m_cswnd.AttachContext( &m_contextDummy ) ;
	m_cswnd.EnableFillBack( false ) ;
	m_cswnd.SetExteriorBackgroundFrame
		( EWindowSpriteInterface::bgfFillColor, 0xFF000000, NULL ) ;
	m_cswnd.SetOptionalFuncFlag( (unsigned int) m_flagOptions ) ;
	//
	m_cswnd.Lock() ;
	m_render.m_pSprite = ESLTypeCast<SGLInterface>( m_cswnd.GetInterface() ) ;
	m_cswnd.Unlock() ;
	//
	return	sglErrSuccess ;
}

// 仮想ディスプレイ終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CloseDisplay( void )
{
	DetachWindowFromChain() ;
	m_cswnd.CloseDisplay() ;
	m_cswnd.AttachContext( NULL ) ;
	return	sglErrSuccess ;
}

// オプション機能フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLWindow::GetOptionalFlags( void )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		return	pWnd->GetOptionalFuncFlag() ;
	}
	return	m_flagOptions ;
}

// オプション機能フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindow::SetOptionalFlags( uint64_t nFlags )
{
	m_flagOptions = nFlags ;
	m_cswnd.SetOptionalFuncFlag( (unsigned int) nFlags ) ;
}

// ウィンドウモード変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ChangeCooperationLevel( SGLWindow::CooperationMode mode )
{
	if ( m_cswnd.ChangeCooperationLevel
			( (EGameWindow::CooperationLevel) mode ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// ウィンドウモード取得
//////////////////////////////////////////////////////////////////////////////
Window::CooperationMode SGLWindow::GetCooperationLevel( void )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		return	(EGameWindow::CooperationLevel) pWnd->GetCooperationLevel() ;
	}
	return	Window::modeWindow ;
}

// 仮想ディスプレイサイズ変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ChangeDisplaySize
	( uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	if ( m_cswnd.ChangeDisplaySize
			( nWidth, nHeight, nBitsPerPixel, nFrequency ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 仮想ディスプレイサイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetDisplaySize( SGLSize& sizeDisplay )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd == NULL )
	{
		return	sglErrFailed ;
	}
	const SIZE &	sizeDisp = pWnd->GetDisplaySize() ;
	sizeDisplay.w = (int32_t) sizeDisp.cx ;
	sizeDisplay.h = (int32_t) sizeDisp.cy ;
	return	sglErrSuccess ;
}

// 物理モニタの解像度を変更するか？
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::EnableChangePhysicalMode( bool fEnable )
{
	if ( m_cswnd.SetChangeDisplayModeFlag( fEnable ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::EnableZBuffer( bool flagZBuffer )
{
	if ( flagZBuffer )
	{
		return	CreateZBuffer() ;
	}
	else
	{
		return	DeleteZBuffer() ;
	}
}

SGLError SGLWindow::CreateZBuffer( void )
{
	m_cswnd.Lock() ;
	if ( m_cswnd.GetZBuffer().GetInfo() == NULL )
	{
		m_cswnd.CreateZBuffer() ;
	}
	m_cswnd.Unlock() ;
	return	sglErrSuccess ;
}

SGLError SGLWindow::DeleteZBuffer( void )
{
	m_cswnd.Lock() ;
	if ( m_cswnd.GetZBuffer().GetInfo() != NULL )
	{
		m_cswnd.DeleteZBuffer() ;
	}
	m_cswnd.Unlock() ;
	return	sglErrSuccess ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetStereoDisplayMode
	( const wchar_t * pszMethodID, uint64_t nParam )
{
	SString	strMethodID = pszMethodID ;
	if ( strMethodID == Stereo3D::AnaglyphView )
	{
		CreateStereoBuffer() ;
		//
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		m_cswnd.Set3DViewDisplay
			( new E3DStereoDisplayAnaglyphView
				( pDrawImage,
					(E3DStereoDisplayAnaglyphView::Mode) nParam ) ) ;
	}
	else if ( strMethodID == Stereo3D::OpenGLQuadBuffer )
	{
		CreateStereoBuffer() ;
		//
		m_cswnd.Set3DViewDisplay( new E3DStereoDisplayOpenGL ) ;
	}
	else if ( strMethodID == Stereo3D::NVStereoBLT )
	{
		CreateStereoBuffer() ;
		//
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		m_cswnd.Set3DViewDisplay( new E3DStereoDisplayNVStereoBLT( pDrawImage ) ) ;
	}
	else if ( strMethodID == Stereo3D::InterleavedView )
	{
		CreateStereoBuffer() ;
		//
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		m_cswnd.Set3DViewDisplay
			( new E3DStereoDisplayInterleaved
				( pDrawImage, ((nParam & 0x01) != 0),
									((nParam & 0x02) != 0) ) ) ;
	}
	else if ( strMethodID == Stereo3D::MonoView )
	{
		m_cswnd.Set3DViewDisplay( NULL ) ;
		//
		DeleteStereoBuffer() ;
	}
	else
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

SGLError SGLWindow::CreateStereoBuffer( void )
{
	m_cswnd.Lock() ;
	ECSWindow::EInterface *	pInterface = m_cswnd.GetInterface() ;
	if ( pInterface != NULL )
	{
		if ( pInterface->GetStereoLeftBuffer().GetInfo() == NULL )
		{
			ESprite::VIEW3D_INFO	v3dInfo ;
			m_render.GetView3DInfo( v3dInfo ) ;
			pInterface->CreateStereoBuffer( v3dInfo ) ;
		}
	}
	m_cswnd.Unlock() ;
	return	sglErrSuccess ;
}

SGLError SGLWindow::DeleteStereoBuffer( void )
{
	m_cswnd.Lock() ;
	ECSWindow::EInterface *	pInterface = m_cswnd.GetInterface() ;
	if ( pInterface != NULL )
	{
		pInterface->DeleteStereoBuffer() ;
	}
	m_cswnd.Unlock() ;
	return	sglErrSuccess ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLWindow::IsSupportedStereoDisplayMode( const wchar_t * pszMethodID )
{
	SString	strMethodID = pszMethodID ;
	if ( strMethodID == Stereo3D::AnaglyphView )
	{
		return	true ;
	}
	else if ( strMethodID == Stereo3D::OpenGLQuadBuffer )
	{
		return	E3DStereoDisplayOpenGL::IsSupportedStereo() ;
	}
	else if ( strMethodID == Stereo3D::NVStereoBLT )
	{
		EGLDrawImage *	pDrawImage = ECotophaScript::GetDrawImage() ;
		if ( pDrawImage != NULL )
		{
			if ( pDrawImage->IsInstalledDirectX9() )
			{
				return	true ;
			}
		}
	}
	else if ( strMethodID == Stereo3D::InterleavedView )
	{
		return	true ;
	}
	else if ( strMethodID == Stereo3D::MonoView )
	{
		return	true ;
	}
	return	false ;
}

// 仮想ディスプレイ・ウィンドウ初期座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::InitWindowPosition
	( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize )
{
	int	nWidth = 0x80000000 ;
	int	nHeight = 0x80000000 ;
	if ( pInitExSize != NULL )
	{
		nWidth = pInitExSize->w ;
		nHeight = pInitExSize->h ;
	}
	m_cswnd.InitWindowPosition( xPos, yPos, nWidth, nHeight ) ;
	return	sglErrSuccess ;
}

// 仮想ディスプレイ・ウィンドウの通常座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetNormalWindowPosition
	( SGLPoint& ptWindow, SGLSize * pExSize )
{
	EGL_POINT	posWnd ;
	EGL_SIZE	sizeWnd ;
	if ( !m_cswnd.GetNormalWindowPosition( posWnd, sizeWnd ) )
	{
		return	sglErrFailed ;
	}
	ptWindow.x = posWnd.x ;
	ptWindow.y = posWnd.y ;
	if ( pExSize != NULL )
	{
		pExSize->w = sizeWnd.w ;
		pExSize->h = sizeWnd.h ;
	}
	return	sglErrSuccess ;
}

// 仮想ディスプレイ・ウィンドウ内表示座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetInternalDisplayPosition
	( SGLImageRect& rctRender, SGLImageRect& rctDisplay )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd == NULL )
	{
		return	sglErrFailed ;
	}
	rctRender.x = 0 ;
	rctRender.y = 0 ;
	rctRender.w = m_cswnd.GetWidth() ;
	rctRender.h = m_cswnd.GetHeight() ;
	//
	const POINT &	ptOffset = pWnd->GetOffsetPos() ;
	const SIZE &	szScreen = pWnd->GetScreenSize() ;
	rctDisplay.x = (int32_t) ptOffset.x ;
	rctDisplay.y = (int32_t) ptOffset.y ;
	rctDisplay.w = (int32_t) szScreen.cx ;
	rctDisplay.h = (int32_t) szScreen.cy ;
	//
	return	sglErrSuccess ;
}

// 仮想ディスプレイ・有効画面外枠表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetExteriorBackgroundFrame
	( uint32_t nFlags, uint32_t rgbColor, SGLImageObject* pTile,
		SGLImageObject* pLeft, SGLImageObject* pRight,
		SGLImageObject* pUpper, SGLImageObject* pUnder )
{
	return	sglErrFailed ;
}

// ウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CreateWindow
	( const wchar_t * pszWindowName,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nFlags, SGLAbstractWindow * pParentWnd )
{
	SString			strWindowName = pszWindowName ;
	SArray<char>	bufWindowName ;
	HICON			hIcon = LoadMainIcon() ;
	m_cswnd.AttachContext( &m_contextDummy ) ;
	m_cswnd.m_pWnd = this ;
	//
	ESLError	err =
		m_cswnd.CreateDisplayWindow
			( strWindowName.EncodeDefaultTo(bufWindowName),
				nWidth, nHeight,
				ESLTypeCast<ECSWindow>( pParentWnd ), hIcon ) ;
	if ( err )
	{
		return	sglErrFailed ;
	}
	AddWindowToChain() ;
	m_cswnd.AttachContext( &m_contextDummy ) ;
	m_cswnd.EnableFillBack( false ) ;
	m_cswnd.SetExteriorBackgroundFrame
		( EWindowSpriteInterface::bgfFillColor, 0xFF000000, NULL ) ;
	m_cswnd.SetOptionalFuncFlag( (unsigned int) m_flagOptions ) ;
	//
	m_cswnd.Lock() ;
	m_render.m_pSprite = ESLTypeCast<SGLInterface>( m_cswnd.GetInterface() ) ;
	m_cswnd.Unlock() ;
	//
	if ( nFlags & flagLayeredWindow )
	{
		m_cswnd.SetLayeredWindow( true ) ;
	}
	return	sglErrSuccess ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CloseWindow( void )
{
	DetachWindowFromChain() ;
	m_cswnd.CloseDisplay() ;
	return	sglErrSuccess ;
}

// ウィンドウ位置を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetWindowLayout( uint32_t nFlags, int xPos, int yPos )
{
	if ( m_cswnd.SetWindowLayout( nFlags, xPos, yPos ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// クライアント座標→スクリーン座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLWindow::ScreenPositionFromClient( S2DDVector& vClient )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		POINT	pos =
			{ eslRoundR32ToInt( (float32_t) vClient.x ),
				eslRoundR32ToInt( (float32_t) vClient.y ) } ;
		if ( pWnd->ClientToScreen( &pos ) )
		{
			vClient.x = pos.x ;
			vClient.y = pos.y ;
		}
	}
	return	vClient ;
}

// スクリーン座標→クライアント座標変換
//////////////////////////////////////////////////////////////////////////////
S2DDVector& SGLWindow::ClientPositionFromScreen( S2DDVector& vScreen )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		POINT	pos =
			{ eslRoundR32ToInt( (float32_t) vScreen.x ),
				eslRoundR32ToInt( (float32_t) vScreen.y ) } ;
		if ( pWnd->ScreenToClient( &pos ) )
		{
			vScreen.x = pos.x ;
			vScreen.y = pos.y ;
		}
	}
	return	vScreen ;
}

// 画面の更新通知
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::PostUpdate( const SGLImageRect* pUpdate )
{
//	ECSWindow::EInterface *	pInterface = m_cswnd.GetInterface() ;
//	if ( pInterface != NULL )
	{
		if ( pUpdate != NULL )
		{
			EGLRect	rctUpdate
				( pUpdate->x, pUpdate->y,
					pUpdate->x + pUpdate->w - 1,
					pUpdate->y + pUpdate->h - 1 ) ;
			
			m_cswnd.Lock() ;
			m_cswnd.UpdateRect( &rctUpdate ) ;
			m_cswnd.Unlock() ;
		}
		else
		{
			m_cswnd.Lock() ;
			m_cswnd.UpdateRect( NULL ) ;
			m_cswnd.Unlock() ;
		}
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 更新領域が存在する場合、即座に描画ハンドラ呼び出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::UpdateWindow( Window::UpdateParameter * pUpdate )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( (pWnd != NULL) && !m_cswnd.IsQuickLockedOnScript() )
	{
		if ( pUpdate == NULL )
		{
			pWnd->UpdateWindow() ;
			return	sglErrSuccess ;
		}
		else
		{
			ECSWindow::EInterface *	pInterface = m_cswnd.GetInterface() ;
			ESLAssert( pInterface != NULL ) ;
			//
			// バックバッファへ描画
			//
			LARGE_INTEGER	liStart, liEnd, liFreq ;
			BOOL	bPerfCounter = ::QueryPerformanceFrequency( &liFreq ) ;
			DWORD	dwStartTime, dwEndTime ;
			bool	fControlNoUpdate ;
			::QueryPerformanceCounter( &liStart ) ;
			dwStartTime = ::timeGetTime() ;
			Lock() ;
			m_cswnd.Lock() ;
			//
			fControlNoUpdate = pInterface->GetConntrolAutoUpdate() ;
			pInterface->ConntrolAutoUpdate( true ) ;
			m_cswnd.UpdateRect( NULL ) ;
			//
			m_cswnd.Refresh() ;
			//
			pInterface->ConntrolAutoUpdate( fControlNoUpdate ) ;
			//
			m_cswnd.Unlock() ;
			Unlock() ;
			::QueryPerformanceCounter( &liEnd ) ;
			dwEndTime = ::timeGetTime() ;
			//
			// 時間更新
			//
			uint32_t	msecRendering = dwEndTime - dwStartTime ;
			uint32_t	freqMonitor = pWnd->GetDisplayFrequency() ;
			if ( bPerfCounter )
			{
				msecRendering =
					(uint32_t) ((liEnd.QuadPart
								- liStart.QuadPart) * 1000
												/ liFreq.QuadPart) ;
			}
			pUpdate->WaitFrame( msecRendering, freqMonitor ) ;
			//
			// 画面へ描画
			//
			const bool	fVSync =
				((pUpdate->flagsUpdate & Window::updateVSync) != 0) ;
			if ( pInterface != NULL )
			{
				Lock() ;
				m_cswnd.Lock() ;
				HDC	hdc = pWnd->GetDC() ;
				pInterface->OnPaint( pWnd, hdc, fVSync ) ;
				pWnd->ReleaseDC( hdc ) ;
				m_cswnd.Unlock() ;
				Unlock() ;
			}
			return	sglErrSuccess ;
		}
	}
	return	sglErrFailed ;
}

// ユーザー入力処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ProcessUserInput( int64_t msecTimeout )
{
	m_cswnd.HandleWindowMessage( 0x20, (DWORD) msecTimeout ) ;
	return	sglErrSuccess ;
}

// 描画スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::PostRenderingThread
	( SSystem::SProcedure * pProc, PostThreadType postType )
{
	LRESULT		lResult ;
	ESLError	err =
		m_cswnd.ProcedureOnWindowThread
			( &SGLWindow::WindowThreadCallerProc, pProc, &lResult, true ) ;
	if ( err )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// UI スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::PostUIThread( SSystem::SProcedure * pProc )
{
	LRESULT		lResult ;
	ESLError	err =
		m_cswnd.ProcedureOnWindowThread
			( &SGLWindow::WindowThreadCallerProc, pProc, &lResult, true ) ;
	if ( err )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// ウィンドウスレッド実行関数
//////////////////////////////////////////////////////////////////////////////
LRESULT __stdcall SGLWindow::WindowThreadCallerProc( void * pInstance )
{
	SProcedure *	pProc = (SProcedure*) pInstance ;
	SSystem::Lock() ;
	pProc->Prepare() ;
	pProc->Run() ;
	pProc->Finalize() ;
	SSystem::Unlock() ;
	return	0 ;
}

// ウィンドウがアクティブ（最前面）か？
//////////////////////////////////////////////////////////////////////////////
bool SGLWindow::IsWindowActive( void )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		return	pWnd->IsWindowActive() ;
	}
	return	false ;
}

// ウィンドウキャプション設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetWindowCaption( const wchar_t * pszWindowName )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		SString			strName = pszWindowName ;
		SArray<char>	bufName ;
		::SetWindowText( *pWnd, strName.EncodeDefaultTo(bufName) ) ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// マウスカーソル表示
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::ShowCursor( bool fShow )
{
	m_cswnd.ShowCursor( fShow ) ;
	return	sglErrSuccess ;
}

// マウスカーソル変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::SetCursor( const wchar_t * pszCursorID )
{
	SGLInterface *	pInter =
		ESLTypeCast<SGLInterface>( m_cswnd.GetInterface() ) ;
	if ( pInter != NULL )
	{
		pInter->ChangeMouseCursor( pszCursorID ) ;
		//
		EGameWindow *	pWnd = m_cswnd.GetWindow() ;
		if ( (pWnd != NULL)
			&& (::GetWindowThreadProcessId( *pWnd, NULL )
										== ::GetCurrentThreadId()) )
		{
			pInter->SetMouseCursor( pszCursorID ) ;
		}
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// マウスカーソル表示状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLWindow::IsShowCursor( void )
{
	return	m_cswnd.IsShowCursor() ;
}

// マウスカーソル座標移動
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::MoveCursorPosition
	( int32_t xPos, int32_t yPos, int idMouse )
{
	ECSWindow::EInterface *	pInterface = m_cswnd.GetInterface() ;
	if ( pInterface != NULL )
	{
		EGLPoint	ptCursorPos( xPos, yPos ) ;
		pInterface->ClientToScreen( ptCursorPos ) ;
		::SetCursorPos( ptCursorPos.x, ptCursorPos.y ) ;
		return	sglErrSuccess ;
	}
	else
	{
		::SetCursorPos( xPos, yPos ) ;
		return	sglErrSuccess ;
	}
}

// マウスカーソル座標取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::GetCursorPosition
	( SGLPoint& ptCursor, int idMouse )
{
	POINT	posCursor ;
	::GetCursorPos( &posCursor ) ;
	//
	ECSWindow::EInterface *	pInterface = m_cswnd.GetInterface() ;
	if ( pInterface != NULL )
	{
		EGLPoint	ptCursorPos( posCursor.x, posCursor.y ) ;
		pInterface->ScreenToClient( ptCursorPos ) ;
		ptCursor.x = ptCursorPos.x ;
		ptCursor.y = ptCursorPos.y ;
		return	sglErrSuccess ;
	}
	else
	{
		ptCursor.x = posCursor.x ;
		ptCursor.y = posCursor.y ;
		return	sglErrSuccess ;
	}
}

// （ウィンドウが表示されている）物理モニタの垂直同期周波数取得
//////////////////////////////////////////////////////////////////////////////
int SGLWindow::GetMonitorFrequency( void )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		RECT	rectWindow ;
		if ( pWnd->GetWindowRect( &rectWindow ) )
		{
			EDisplayMode&	dmMode = pWnd->GetDisplayMode() ;
			EString			strDisplay ;
			const char *	pszDisplay =
				dmMode.GetDisplayNameFromRect( strDisplay, &rectWindow ) ;
			if ( pszDisplay != NULL )
			{
				return	dmMode.GetDisplayFrequency( pszDisplay ) ;
			}
		}
	}
	return	0 ;
}

// マウスイベントキャプチャー
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindow::CaptureMouse( int idMouse )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		pWnd->SetCapture() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

class	SGLWindow_ReleaseMouseProc	: public SProcedure
{
public:
	// スレッド関数
	virtual void Run( void )
	{
		::ReleaseCapture() ;
	}
	// 完了後の処理
	virtual void Finalize( void )
	{
		delete	this ;
	}
} ;

SGLError SGLWindow::ReleaseMouse( int idMouse )
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	DWORD	dwProcessId ;
	if ( (pWnd != NULL)
		&& (::GetCurrentThreadId()
				== ::GetWindowThreadProcessId( *pWnd, &dwProcessId )) )
	{
		::ReleaseCapture() ;
		return	sglErrSuccess ;
	}
	else
	{
		return	PostUIThread( new SGLWindow_ReleaseMouseProc ) ;
	}
}

// 描画インターフェース取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface * SGLWindow::GetRenderContext
		( S3DRenderContextInterface::StereoViewIndex sviView )
{
	S3DRenderContextInterface *	pContext = NULL ;
	SSystem::Lock() ;
	if ( m_hRender == NULL )
	{
		m_hRender = ::eglCreateRenderPolygon() ;
	}
	m_render.m_pSprite = &m_cswnd ;
	if ( !m_cswnd.IsEnabledStereoView()
		|| (sviView == S3DRenderContextInterface::stereoViewRight) )
	{
		m_hRender->Initialize
			( m_cswnd.GetInfo(), NULL,
				m_cswnd.GetZBuffer(), m_cswnd.GetScreen3DPosition() ) ;
		pContext = &m_render ;
	}
	else if ( m_cswnd.IsEnabledStereoView()
		&& (sviView == S3DRenderContextInterface::stereoViewLeft) )
	{
		m_hRender->Initialize
			( m_cswnd.GetStereoLeftBuffer(), NULL,
				m_cswnd.GetZBuffer(), m_cswnd.GetScreen3DPosition() ) ;
		pContext = &m_render ;
	}
	if ( pContext != NULL )
	{
		m_render.ResetTransformation() ;
		m_render.AttachRenderPolygon( m_hRender ) ;
		m_render.SelectParallaxView( sviView ) ;
	}
	SSystem::Unlock() ;
	return	pContext ;
}

void SGLWindow::ReleaseRenderContext( S3DRenderContextInterface * context )
{
	SSystem::Lock() ;
	if ( &m_render == context )
	{
		m_render.Flush() ;
		if ( !m_cswnd.IsEnabledStereoView()
			|| (m_render.CurrentParallaxView()
					== S3DRenderContextInterface::stereoViewLeft) )
		{
			EWindowSpriteInterface *	pInterface = m_cswnd.GetInterface() ;
			EGameWindow *	pWnd = m_cswnd.GetWindow() ;
			if ( pInterface && pWnd )
			{
				bool	fStretchingByCPU =
							pInterface->IsImageStretchingByCPU() ;
				HDC		hdc = pWnd->GetDC() ;
				pInterface->SetImageStretchingByCPU( false ) ;
				pInterface->OnPaint( pWnd, hdc ) ;
				pWnd->ReleaseDC( hdc ) ;
				pInterface->SetImageStretchingByCPU( fStretchingByCPU ) ;
			}
		}
	}
	SSystem::Unlock() ;
}

// プラットフォーム固有オブジェクト
//////////////////////////////////////////////////////////////////////////////
HWND SGLWindow::GetWindowHandle( void ) const
{
	EGameWindow *	pWnd = m_cswnd.GetWindow() ;
	if ( pWnd != NULL )
	{
		return	*pWnd ;
	}
	return	NULL ;
}

#else

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindow, SGLGenericWindowGLS3View )

#endif
