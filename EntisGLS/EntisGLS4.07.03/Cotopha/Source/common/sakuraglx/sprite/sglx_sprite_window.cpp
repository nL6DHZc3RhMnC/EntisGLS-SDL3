
#include <sakuraglx/sakuraglx.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Window-Sprite インターフェース実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLSpriteWindowPaintInterface,
				SGLPaintInterface, SGLTimerInterface )
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteWindowMouseInterface, SGLMouseInterface )
SGL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::SGLSpriteWindowKeyInterface,
		SGLKeyInterface, SGLCharInputInterface, SGLCommandInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteWindowPaintInterface::SGLSpriteWindowPaintInterface( void )
	: m_pSprite( NULL )
{
}

SGLSpriteWindowPaintInterface::SGLSpriteWindowPaintInterface( SGLWindowSprite * pSprite )
	: m_pSprite( pSprite )
{
}

SGLSpriteWindowMouseInterface::SGLSpriteWindowMouseInterface( void )
	: m_pSprite( NULL )
{
}

SGLSpriteWindowMouseInterface::SGLSpriteWindowMouseInterface( SGLSprite * pSprite )
	: m_pSprite( pSprite )
{
}

SGLSpriteWindowKeyInterface::SGLSpriteWindowKeyInterface( void )
	: m_pSprite( NULL )
{
}

SGLSpriteWindowKeyInterface::SGLSpriteWindowKeyInterface( SGLSprite * pSprite )
	: m_pSprite( pSprite )
{
}

// Sprite 関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowPaintInterface::AttachSprite( SGLWindowSprite * pSprite )
{
	m_pSprite = pSprite ;
	m_timer.Reset() ;
}

void SGLSpriteWindowMouseInterface::AttachSprite( SGLSprite * pSprite )
{
	m_pSprite = pSprite ;
}

void SGLSpriteWindowKeyInterface::AttachSprite( SGLSprite * pSprite )
{
	m_pSprite = pSprite ;
}

// 後方リスナ関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowMouseInterface::AttachPostListener( SGLSpriteMouseListener * pListener )
{
	m_refPostListener = pListener ;
}

void SGLSpriteWindowKeyInterface::AttachPostListener( SGLSpriteKeyListener * pListener )
{
	m_refPostListener = pListener ;
}

// 後方リスナ取得
//////////////////////////////////////////////////////////////////////////////
SGLSpriteKeyListener * SGLSpriteWindowKeyInterface::GetPostListener( void ) const
{
	return	m_refPostListener.GetReference() ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowPaintInterface::OnPaint
		( Window * pWnd, RenderContext * context )
{
	if ( m_pSprite != NULL )
	{
		S3DRenderContext	render( context, false ) ;
		uint32_t	rgbaFillBack ;
		if ( m_pSprite->GetFillBackColor( rgbaFillBack ) )
		{
			render.FillClearTarget( rgbaFillBack ) ;
		}
		//
		S3DRenderContextInterface::StereoViewIndex
					sviView = render.CurrentParallaxView() ;
		SGLSprite::Stereo3DView s3dView = SGLSprite::s3dMonoview ;
		if ( sviView == S3DRenderContextInterface::stereoViewRight )
		{
			s3dView = SGLSprite::s3dRightView ;
		}
		else if ( sviView == S3DRenderContextInterface::stereoViewLeft )
		{
			s3dView = SGLSprite::s3dLeftView ;
		}
		render.SetParallax
			( m_pSprite->GetParallax(),
				m_pSprite->GetParallaxFocusRatio(),
				m_pSprite->GetParallaxScreenX() ) ;
		//
		m_pSprite->BeforeDraw( s3dView ) ;
		m_pSprite->Draw( render, NULL, s3dView ) ;
		m_pSprite->AfterDraw( s3dView ) ;
	}
}

// 描画前フレーム準備処理（全視点共通処理）
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowPaintInterface::OnPrepareFrame( Window * pWnd )
{
	if ( m_pSprite != NULL )
	{
		m_pSprite->PrepareDrawFrame() ;
	}
}

// 全描画完了
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowPaintInterface::OnFinishedFrame( Window * pWnd )
{
	if ( m_pSprite != NULL )
	{
		m_pSprite->FinishDrawFrame() ;
	}
}

// タイマー
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowPaintInterface::OnTimer( Window * pWnd, uint64_t idTimer )
{
	if ( m_pSprite != NULL )
	{
		int64_t	msecPast = m_timer.GetTime() ;
		m_timer.Reset() ;
		if ( msecPast >= 1 )
		{
			if ( msecPast > 1000 )
			{
				msecPast = 1000 ;
			}
			m_pSprite->AdvanceTime( (uint32_t) msecPast ) ;
		}
	}
}

// フラグ正規化
//////////////////////////////////////////////////////////////////////////////
int64_t SGLSpriteWindowMouseInterface::NormalizeMouseFlags( int64_t nFlags )
{
	return	(nFlags & ~MouseIDMask) | AllocateMouseID( GetMouseID( nFlags ) ) ;
}

// MouseID 割り当て
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLSpriteWindowMouseInterface::AllocateMouseID( uint32_t idMouse )
{
	const uint32_t *	pMouseIDs = m_mapMouseID.GetConstArray() ;
	const size_t		nCount = m_mapMouseID.GetLength() ;
	uint32_t			i ;
	for ( i = 0; i < nCount; i ++ )
	{
		if ( pMouseIDs[i] == idMouse )
		{
			return	i ;
		}
	}
	if ( m_mapMouseID.GetLength() >= 0x100 )
	{
		Trace( "too much mouse IDs.\n" ) ;
		m_mapMouseID.RemoveAll() ;
	}
	if ( idMouse == 0 )
	{
		m_mapMouseID.SetAt( 0, 0 ) ;
		return	0 ;
	}
	for ( i = 1; i < nCount; i ++ )
	{
		if ( pMouseIDs[i] == 0 )
		{
			break ;
		}
	}
	m_mapMouseID.SetAt( i, idMouse ) ;
	return	i ;
}

// MouseID 削除
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowMouseInterface::FreeMouseID( uint32_t idMouse )
{
	const uint32_t *	pMouseIDs = m_mapMouseID.GetConstArray() ;
	const size_t		nCount = m_mapMouseID.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pMouseIDs[i] == idMouse )
		{
			bool	flagLastID = true ;
			for ( size_t j = i + 1; j < nCount; j ++ )
			{
				if ( pMouseIDs[j] != 0 )
				{
					flagLastID = false ;
					break ;
				}
			}
			if ( flagLastID )
			{
				m_mapMouseID.SetLength( i ) ;
			}
			else
			{
				m_mapMouseID.SetAt( i, 0 ) ;
			}
			break ;
		}
	}
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowMouseInterface::OnMouseMove
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		S2DDVector	vPos( xPos, yPos ) ;
		m_pSprite->GlobalToLocal( vPos ) ;
		//
		nFlags = NormalizeMouseFlags(nFlags) ;
		//
		#if	defined(__PLATFORM_WINDOWS__)
		pWnd->SetCursor
			( m_pSprite->HitTestMouseCursor( vPos.x, vPos.y, nFlags ) ) ;
		#endif
		if ( m_pSprite->OnMouseMove( vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
		SGLSpriteMouseListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnMouseMove( *m_pSprite, xPos, yPos, nFlags ) ;
		}
	}
	return	false ;
}

void SGLSpriteWindowMouseInterface::OnMouseLeave( Window * pWnd, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		nFlags = NormalizeMouseFlags(nFlags) ;
		m_pSprite->OnMouseLeave( nFlags ) ;
		//
		SGLSpriteMouseListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			pListener->OnMouseLeave( *m_pSprite, nFlags ) ;
		}
	}
	FreeMouseID( GetMouseID( nFlags ) ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowMouseInterface::OnMouseWheel
	( Window * pWnd, int32_t zDelta,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		S2DDVector	vPos( xPos, yPos ) ;
		m_pSprite->GlobalToLocal( vPos ) ;
		//
		nFlags = NormalizeMouseFlags(nFlags) ;
		if ( m_pSprite->OnMouseWheel( zDelta, vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
		SGLSpriteMouseListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnMouseWheel
						( *m_pSprite, zDelta, xPos, yPos, nFlags ) ;
		}
	}
	return	false ;
}

// マウスボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowMouseInterface::OnButtonDown
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		S2DDVector	vPos( xPos, yPos ) ;
		m_pSprite->GlobalToLocal( vPos ) ;
		//
		nFlags = NormalizeMouseFlags(nFlags) ;
		if ( m_pSprite->OnButtonDown( vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
		SGLSpriteMouseListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnButtonDown
						( *m_pSprite, xPos, yPos, nFlags ) ;
		}
	}
	return	false ;
}

bool SGLSpriteWindowMouseInterface::OnButtonUp
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		S2DDVector	vPos( xPos, yPos ) ;
		m_pSprite->GlobalToLocal( vPos ) ;
		//
		nFlags = NormalizeMouseFlags(nFlags) ;
		if ( m_pSprite->OnButtonUp( vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
		SGLSpriteMouseListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnButtonUp
						( *m_pSprite, xPos, yPos, nFlags ) ;
		}
	}
	return	false ;
}

bool SGLSpriteWindowMouseInterface::OnButtonDblClk
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		S2DDVector	vPos( xPos, yPos ) ;
		m_pSprite->GlobalToLocal( vPos ) ;
		//
		nFlags = NormalizeMouseFlags(nFlags) ;
		if ( m_pSprite->OnButtonDblClk( vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
		SGLSpriteMouseListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnButtonDblClk
						( *m_pSprite, xPos, yPos, nFlags ) ;
		}
	}
	return	false ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowKeyInterface::OnKeyDown
	( Window * pWnd, int64_t nVirtKey, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		if ( m_pSprite->OnKeyDown( nVirtKey, nFlags ) )
		{
			return	true ;
		}
		SGLSpriteKeyListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnKeyDown
						( *m_pSprite, nVirtKey, nFlags ) ;
		}
	}
	return	false ;
}

bool SGLSpriteWindowKeyInterface::OnKeyUp
	( Window * pWnd, int64_t nVirtKey, int64_t nFlags )
{
	if ( m_pSprite != NULL )
	{
		if ( m_pSprite->OnKeyUp( nVirtKey, nFlags ) )
		{
			return	true ;
		}
		SGLSpriteKeyListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnKeyUp
						( *m_pSprite, nVirtKey, nFlags ) ;
		}
	}
	return	false ;
}

// フォーカス
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWindowKeyInterface::OnSetFocus( Window * pWnd )
{
	if ( m_pSprite != NULL )
	{
		m_pSprite->OnSetKeyFocus() ;
	}
}

void SGLSpriteWindowKeyInterface::OnKillFocus( Window * pWnd )
{
	if ( m_pSprite != NULL )
	{
		m_pSprite->OnKillKeyFocus() ;
	}
}

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowKeyInterface::OnChar( Window * pWnd, uint16_t codeChar )
{
	if ( m_pSprite != NULL )
	{
		if ( m_pSprite->OnChar( codeChar ) )
		{
			return	true ;
		}
		SGLSpriteKeyListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnChar( *m_pSprite, codeChar ) ;
		}
	}
	return	false ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowKeyInterface::OnStartComposition
	( Window * pWnd, SGLInputStartComposition& iscForm )
{
	if ( m_pSprite != NULL )
	{
		if ( m_pSprite->OnStartComposition( iscForm ) )
		{
			return	true ;
		}
		SGLSpriteKeyListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnStartComposition( *m_pSprite, iscForm ) ;
		}
	}
	return	false ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowKeyInterface::OnEndComposition( Window * pWnd )
{
	if ( m_pSprite != NULL )
	{
		if ( m_pSprite->OnEndComposition() )
		{
			return	true ;
		}
		SGLSpriteKeyListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnEndComposition( *m_pSprite ) ;
		}
	}
	return	false ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowKeyInterface::OnCompositionString
	( Window * pWnd, const SGLInputCompositionString& icsComp )
{
	if ( m_pSprite != NULL )
	{
		if ( m_pSprite->OnCompositionString( icsComp ) )
		{
			return	true ;
		}
		SGLSpriteKeyListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnCompositionString( *m_pSprite, icsComp ) ;
		}
	}
	return	false ;
}

// コマンド
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWindowKeyInterface::OnCommand
	( Window * pWnd, const uint16_t * pszCmd,
					int64_t nParam, int64_t nCode )
{
	SString	strCmd = pszCmd ;
	int	nPriority = SGLSprite::commandNormal ;
	if ( (strCmd == SysCommandId::AppExit)
		|| (strCmd == SysCommandId::AppBack)
		|| (strCmd == SysCommandId::AppSuspend) )
	{
		nPriority = SGLSprite::commandHigh ;
	}
	if ( m_pSprite != NULL )
	{
		if ( m_pSprite->OnCommand( strCmd, nParam, nCode, nPriority ) )
		{
			return	true ;
		}
		SGLSpriteKeyListener *	pListener = m_refPostListener ;
		if ( pListener != NULL )
		{
			return	pListener->OnCommand
				( *m_pSprite, strCmd, nParam, nCode, nPriority, false ) ;
		}
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// Sprite 基底 Window 実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO3
	( SakuraGL::SGLWindowSprite, SGLSprite, SGLWindow, SGLWindowMonitorInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowSprite::SGLWindowSprite( void )
	: m_flagInPaint( false ), m_flagAsyncQueue( false )
{
	m_listenPaint.AttachSprite( this ) ;
	m_listenMouse.AttachSprite( this ) ;
	m_listenKey.AttachSprite( this ) ;
	m_spriteDirect.m_pWindow = this ;
	m_flagDisablePostUpdate = false ;
	m_flagFillBack = true ;
	m_rgbaFillBack = 0 ;
	m_ptLastMousePos.x = -1 ;
	m_ptLastMousePos.y = -1 ;
	m_msecAutoHideMouse = Synchronism::Infinite ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowSprite::~SGLWindowSprite( void )
{
	EndAsyncQueueThread() ;
}

// 直前のマウス座標を使って OnMouseMove を呼び出す
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::CallMouseMove( void )
{
	Lock() ;
	SGLPoint	ptCursor ;
	if ( GetCursorPosition( ptCursor, 0 ) == sglErrSuccess )
	{
		SGLSprite::OnMouseMove( ptCursor.x, ptCursor.y, 0 ) ;
	}
	Unlock() ;
}

// マウス入力後置リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::AttachMousePostListener( SGLSpriteMouseListener * pListener )
{
	m_listenMouse.AttachPostListener( pListener ) ;
}

// マウス入力後置リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::AttachKeyPostListener( SGLSpriteKeyListener * pListener )
{
	m_listenKey.AttachPostListener( pListener ) ;
}

// タイマ処理の有効／無効化
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::EnableSpriteTimer( bool flagTimer )
{
	if ( flagTimer )
	{
		m_listenPaint.AttachSprite( this ) ;
		SGLWindow::SetTimerInterface( &m_listenPaint ) ;
	}
	else
	{
		SGLWindow::SetTimerInterface( NULL ) ;
	}
}

// PostUpdate での遅延描画有効／無効化
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::EnablePostUpdate( bool flagUpdate )
{
	m_flagDisablePostUpdate = !flagUpdate ;
	if ( flagUpdate )
	{
		SGLWindow::PostUpdate( NULL ) ;
	}
}

// 自動カーソル消去設定
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::SetAutoHideCursor( int64_t msecTimeout )
{
	m_msecAutoHideMouse = msecTimeout ;
}

// 描画完了通知受け取り登録（シグナルを受け取ると自動的に登録解除）
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::AttachOnceSignalForFramePaint( SSystem::SSignalEvent * pSignal )
{
	m_notifierPaint.AttachSignalEvent( pSignal ) ;
}

// 描画処理中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::IsWindowPainting( double * pmsecPastPainting ) const
{
	if ( pmsecPastPainting != nullptr )
	{
		*pmsecPastPainting = m_timerPaint.GetRealTime() ;
	}
	return	m_flagInPaint ;
}

// 非同期処理追加
//////////////////////////////////////////////////////////////////////////////
SSystem::SProcedureQueue::ProcIdentity
	SGLWindowSprite::PostAsyncProcedure
		( SSystem::SProcedure * pProc,
			SSystem::SSignalEvent * pDoneSignal, bool flagAutoDelete, bool flagFence )
{
	if ( !m_flagAsyncQueue )
	{
		ESLVerify( BeginAsyncQueueThread() == sglErrSuccess ) ;
	}
	return	m_queAsyncProc.AddProcedure( pProc, pDoneSignal, flagAutoDelete, flagFence ) ;
}

// 非同期処理キャンセル
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::CancelAsyncProcedure
	( SSystem::SProcedureQueue::ProcIdentity procId )
{
	return	m_queAsyncProc.CancelProcedure( procId ) ;
}

// 全ての非同期処理が完了するまで待つ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::WaitForAllAsyncProcedure( int64_t msecTimeout )
{
	return	(SGLError) m_queAsyncProc.WaitUntilEmpty( msecTimeout ) ;
}

// 非同期処理スレッド開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::BeginAsyncQueueThread( void )
{
	m_queAsyncProc.Lock() ;
	if ( !m_flagAsyncQueue )
	{
		m_queAsyncProc.Unlock() ;
		return	sglErrSuccess ;
	}
	m_flagAsyncQueue = true ;
	m_queAsyncProc.Unlock() ;
	//
	m_queAsyncProc.AsyncRun() ;
	return	sglErrSuccess ;
}

// 非同期処理スレッド終了
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::EndAsyncQueueThread( void )
{
	m_queAsyncProc.Lock() ;
	if ( !m_flagAsyncQueue )
	{
		m_queAsyncProc.Unlock() ;
		return ;
	}
	m_flagAsyncQueue = false ;
	m_queAsyncProc.Unlock() ;
	//
	m_queAsyncProc.RequestQuit() ;
	m_queAsyncProc.WaitAllRunLoops() ;
}

// 背景色取得
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::GetFillBackColor( uint32_t& argbFill ) const
{
	argbFill = m_rgbaFillBack ;
	return	m_flagFillBack ;
}

// 背景色設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::SetFillBackColor
	( uint32_t argbFill, bool flagFillBack )
{
	m_rgbaFillBack = argbFill ;
	m_flagFillBack = flagFillBack ;
	//
	return	sglErrSuccess ;
}

// SGLSprite の属する SGLWindowSprite を取得
//////////////////////////////////////////////////////////////////////////////
SGLWindowSprite *
	SGLWindowSprite::WindowOf( SGLSprite * pSprite, S2DDVector * pvPos )
{
	while ( pSprite != NULL )
	{
		SGLWindowSprite *
			pWindow = ESLTypeCast<SGLWindowSprite>( pSprite ) ;
		if ( pWindow != NULL )
		{
			return	pWindow ;
		}
		if ( pvPos != NULL )
		{
			pSprite->LocalToGlobal( *pvPos ) ;
		}
		SGLSprite *	pParent = pSprite->GetParent() ;
		if ( pParent == nullptr )
		{
			SGLBasicForm::Sprite::StubSprite *	pStub =
				ESLTypeCast<SGLBasicForm::Sprite::StubSprite>( pSprite ) ;
			if ( pStub != nullptr )
			{
				SGLBasicForm::Sprite *	pOwnerItem = pStub->GetOwnerItem() ;
				if ( pOwnerItem != nullptr )
				{
					if ( pvPos != nullptr )
					{
						*pvPos = pOwnerItem->LocalToGlobal( *pvPos ) ;
					}
					SGLBasicForm *	pForm = pOwnerItem->GetParentForm() ;
					while ( pForm != nullptr )
					{
						pParent = pForm->GetOwnerSprite() ;
						if ( pParent != nullptr )
						{
							break ;
						}
						SGLBasicForm::SubForm *	pSubForm = pForm->GetOwnerItem() ;
						if ( pSubForm == nullptr )
						{
							break ;
						}
						if ( pvPos != nullptr )
						{
							*pvPos = pSubForm->LocalToGlobal( *pvPos ) ;
						}
						pForm = pSubForm->GetParentForm() ;
					}
				}
			}
		}
		pSprite = pParent ;
	}
	return	NULL ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::AdvanceTime( uint32_t msecPast )
{
	if ( (m_msecAutoHideMouse != Synchronism::Infinite)
		&& IsShowCursor()
		&& (m_timerStillMouse.GetTime() > m_msecAutoHideMouse) )
	{
		ShowCursor( false ) ;
	}
	m_spriteDirect.AdvanceTime( msecPast ) ;
	SGLSprite::AdvanceTime( msecPast ) ;
}

// フレーム描画（視点に関係しない）共通処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::PrepareDrawFrame( void )
{
	m_timerPaint.Reset() ;
	m_flagInPaint = true ;

	SGLSprite::PrepareDrawFrame() ;
}

// フレーム描画完了後処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::FinishDrawFrame( void )
{
	SGLSprite::FinishDrawFrame() ;

	m_flagInPaint = false ;
	m_notifierPaint.NotifyEventSignal() ;
}

// 更新領域通知
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::PostUpdate( SGLRect* pUpdate )
{
	SGLSprite::PostUpdate( pUpdate ) ;
	//
	if ( !m_flagDisablePostUpdate )
	{
		if ( pUpdate != NULL )
		{
			SGLImageRect	rect = *pUpdate ;
			SGLWindow::PostUpdate( &rect ) ;
		}
		else
		{
			SGLWindow::PostUpdate( NULL ) ;
		}
	}
}

void SGLWindowSprite::DirectSprite::PostUpdate( SGLRect* pUpdate )
{
	SGLSprite::PostUpdate( pUpdate ) ;
	m_pWindow->PostUpdate( NULL ) ;
}

// コマンド通知
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::DirectSprite::NotifyCommand
	( const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	SGLWindowSprite *	pSprite = ESLTypeCast<SGLWindowSprite>( m_pWindow ) ;
	if ( pSprite != NULL )
	{
		SGLSpriteKeyListener *
			pListener = pSprite->m_listenKey.GetPostListener() ;
		if ( pListener != NULL )
		{
			return	pListener->OnCommand
				( *this, pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
		}
	}
	return	SGLSprite::NotifyCommand
		( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
}

// マウス入力キャプチャー要求
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::SetMouseCapture( void )
{
	SGLSprite::SetMouseCapture() ;
	return	SGLWindow::CaptureMouse() ;
}

// マウス入力キャプチャー解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::ReleaseMouseCapture( void )
{
	SGLSprite::ReleaseMouseCapture() ;
	m_spriteDirect.ReleaseMouseCapture() ;
	return	SGLWindow::ReleaseMouse() ;
}

// キーフォーカスを要求
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::SetKeyFocus( void )
{
	SGLError	err = SGLSprite::SetKeyFocus() ;

#if	defined(__PLATFORM_WINDOWS__)
	HWND	hwnd = GetWindowHandle() ;
	if ( ::IsWindow( hwnd ) )
	{
		::SetFocus( hwnd ) ;
	}
#endif
	return	err ;
}

// コマンド通知
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::NotifyCommand
	( const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	SGLSpriteKeyListener *	pListener = m_listenKey.GetPostListener() ;
	if ( pListener != NULL )
	{
		return	pListener->OnCommand
			( *this, pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
	}
	return	SGLSprite::NotifyCommand
		( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
}

// 仮想ディスプレイ開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::CreateDisplay
	( const wchar_t * pszWindowName,
		Window::CooperationMode mode,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	SGLError	err =
		SGLWindow::CreateDisplay
			( pszWindowName, mode,
				nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
	if ( !err )
	{
		BindWindowToSprite() ;
	}
	return	err ;
}

// 仮想ディスプレイ終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::CloseDisplay( void )
{
	EndAsyncQueueThread() ;

	return	SGLWindow::CloseDisplay() ;
}

// ウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::CreateWindow
	( const wchar_t * pszWindowName,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nFlags, SGLAbstractWindow * pParentWnd )
{
	SGLError	err =
		SGLWindow::CreateWindow
			( pszWindowName,
				nWidth, nHeight, nFlags, pParentWnd ) ;
	if ( !err )
	{
		BindWindowToSprite() ;
	}
	return	err ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::CloseWindow( void )
{
	EndAsyncQueueThread() ;

	return	SGLWindow::CloseWindow() ;
}

// マウスカーソル表示
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowSprite::ShowCursor( bool fShow )
{
	if ( fShow )
	{
		m_timerStillMouse.Reset() ;
	}
	return	SGLWindow::ShowCursor( fShow ) ;
}

// Sprite 用インターフェースを Window に関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::BindWindowToSprite( void )
{
	SGLWindow::SetPaintInterface( &m_listenPaint ) ;
	SGLWindow::SetDirectPaintInterface( this ) ;
	SGLWindow::SetTimerInterface( &m_listenPaint ) ;
	SGLWindow::SetMouseInterface( &m_listenMouse ) ;
	SGLWindow::SetDirectMouseInterface( this ) ;
	SGLWindow::SetKeyInterface( &m_listenKey ) ;
	SGLWindow::SetCharInputInterface( &m_listenKey ) ;
	SGLWindow::SetCommandInterface( &m_listenKey ) ;
}

// 描画ハンドラ
//////////////////////////////////////////////////////////////////////////////
SGLPaintInterface *
	SGLWindowSprite::SetPaintInterface( SGLPaintInterface * pPaint )
{
	return	SGLWindow::SetPaintInterface( pPaint ) ;
}

SGLPaintInterface *
	SGLWindowSprite::SetDirectPaintInterface( SGLPaintInterface * pPaint )
{
	return	SGLWindow::SetDirectPaintInterface( pPaint ) ;
}

// タイマーハンドラ
//////////////////////////////////////////////////////////////////////////////
SGLTimerInterface *
	SGLWindowSprite::SetTimerInterface( SGLTimerInterface * pTimer )
{
	return	SGLWindow::SetTimerInterface( pTimer ) ;
}

// マウス入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLMouseInterface *
	SGLWindowSprite::SetMouseInterface( SGLMouseInterface * pMouse )
{
	return	SGLWindow::SetMouseInterface( pMouse ) ;
}

SGLMouseInterface *
	SGLWindowSprite::SetDirectMouseInterface( SGLMouseInterface * pMouse )
{
	return	SGLWindow::SetDirectMouseInterface( pMouse ) ;
}

// キー入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLKeyInterface * SGLWindowSprite::SetKeyInterface( SGLKeyInterface * pKey )
{
	return	SGLWindow::SetKeyInterface( pKey ) ;
}

// 文字入力インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLCharInputInterface *
	SGLWindowSprite::SetCharInputInterface( SGLCharInputInterface * pChar )
{
	return	SGLWindow::SetCharInputInterface( pChar ) ;
}

// コマンド・インターフェース
//////////////////////////////////////////////////////////////////////////////
SGLCommandInterface *
	SGLWindowSprite::SetCommandInterface( SGLCommandInterface * pCmd )
{
	return	SGLWindow::SetCommandInterface( pCmd ) ;
}

// スレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLWindowSprite::Lock( int64_t msecTimeout ) const
{
	return	SGLWindow::Lock( msecTimeout ) ;
}

SSystem::SError SGLWindowSprite::LockTrace
	( const char * pszSource, size_t nLineNum, int64_t msecTimeout ) const
{
	return	SGLWindow::LockTrace( pszSource, nLineNum, msecTimeout ) ;
}

SSystem::SError SGLWindowSprite::Unlock( void ) const
{
	return	SGLWindow::Unlock() ;
}

atomic_int_t SGLWindowSprite::UnlockAll( void ) const
{
	return	SGLWindow::UnlockAll() ;
}

SSystem::SError SGLWindowSprite::Relock( atomic_int_t nLock ) const
{
	return	SGLWindow::Relock( nLock ) ;
}

atomic_int_t SGLWindowSprite::TestLocked( void ) const
{
	return	SGLWindow::TestLocked() ;
}

// ウィンドウスレッド排他処理用ミューテックス変更
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::SetWindowUIThreadMutex( SSystem::SSharableMutex * pMutex )
{
	SGLSprite::SetUIThreadMutex( pMutex ) ;
	m_spriteDirect.SetUIThreadMutex( pMutex ) ;
	SGLWindow::SetWindowUIThreadMutex( pMutex ) ;
}

// Rosetta 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLWindowSprite::GetRSClassName( void ) const
{
	return	L"WindowSprite" ;
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLWindowSprite::GetLQClassName( void ) const
{
	return	L"EntisGLS4.WindowSprite" ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::OnPaint( Window * pWnd, RenderContext * context )
{
	S3DRenderContext	render( context, false ) ;
//	render.FillClearTarget( 0xff000000 ) ;
	//
	S3DRenderContextInterface::StereoViewIndex
				sviView = render.CurrentParallaxView() ;
	SGLSprite::Stereo3DView s3dView = SGLSprite::s3dMonoview ;
	if ( sviView == S3DRenderContextInterface::stereoViewRight )
	{
		s3dView = SGLSprite::s3dRightView ;
	}
	else if ( sviView == S3DRenderContextInterface::stereoViewLeft )
	{
		s3dView = SGLSprite::s3dLeftView ;
	}
	render.SetParallax
		( m_spriteDirect.GetParallax(),
			m_spriteDirect.GetParallaxFocusRatio(),
			m_spriteDirect.GetParallaxScreenX() ) ;
	//
	m_spriteDirect.BeforeDraw( s3dView ) ;
	m_spriteDirect.Draw( render, NULL, s3dView ) ;
	m_spriteDirect.AfterDraw( s3dView ) ;
}

// 描画前フレーム準備処理（全視点共通処理）
//////////////////////////////////////////////////////////////////////////////
void SGLWindowSprite::OnPrepareFrame( Window * pWnd )
{
	m_spriteDirect.PrepareDrawFrame() ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::OnMouseMove
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	if ( (m_ptLastMousePos.x != xPos)
		|| (m_ptLastMousePos.y != yPos) )
	{
		m_ptLastMousePos.x = xPos ;
		m_ptLastMousePos.y = yPos ;
		m_timerStillMouse.Reset() ;
		if ( !IsShowCursor() )
		{
			ShowCursor( true ) ;
		}
	}
	nFlags = m_listenMouse.NormalizeMouseFlags(nFlags) ;
	return	m_spriteDirect.OnMouseMove( xPos, yPos, nFlags ) ;
}

void SGLWindowSprite::OnMouseLeave( Window * pWnd, int64_t nFlags )
{
	nFlags = m_listenMouse.NormalizeMouseFlags(nFlags) ;
	m_spriteDirect.OnMouseLeave( nFlags ) ;
	m_listenMouse.FreeMouseID( SGLMouseInterface::GetMouseID( nFlags ) ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::OnMouseWheel
	( Window * pWnd, int32_t zDelta,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	nFlags = m_listenMouse.NormalizeMouseFlags(nFlags) ;
	return	m_spriteDirect.OnMouseWheel( zDelta, xPos, yPos, nFlags ) ;
}

// マウスボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowSprite::OnButtonDown
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	nFlags = m_listenMouse.NormalizeMouseFlags(nFlags) ;
	return	m_spriteDirect.OnButtonDown( xPos, yPos, nFlags ) ;
}

bool SGLWindowSprite::OnButtonUp
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	nFlags = m_listenMouse.NormalizeMouseFlags(nFlags) ;
	return	m_spriteDirect.OnButtonUp( xPos, yPos, nFlags ) ;
}

bool SGLWindowSprite::OnButtonDblClk
	( Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	nFlags = m_listenMouse.NormalizeMouseFlags(nFlags) ;
	return	m_spriteDirect.OnButtonDblClk( xPos, yPos, nFlags ) ;
}
