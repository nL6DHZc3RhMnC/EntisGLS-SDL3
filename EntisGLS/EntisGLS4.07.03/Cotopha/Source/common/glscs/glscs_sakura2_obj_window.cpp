
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/window/sgl_window_menu.h>
#include <glscs/glscs_sakura2_obj_render.h>
#include <glscs/glscs_sakura2_obj_window.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// 描画関数呼び出し
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( ECSSakura2::WindowObject::SGLPaintCaller, SGLPaintInterface )

// 描画
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2::WindowObject::SGLPaintCaller::OnPaint
	( SakuraGL::Window * pWnd, SakuraGL::RenderContext * context )
{
	if ( (m_pWnd != NULL) && (m_addrPaint != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			SSystem::LockTrace( __FILE__, __LINE__ ) ;
			//
			RenderContextObject *	pRender =
				ESLTypeCast<RenderContextObject>
					( pVM->ObjectFromAddress( (DWORD) (m_addrRender >> 32) ) ) ;
			if ( pRender != NULL )
			{
				pRender->AttachRenderInterface( context ) ;
			}
			//
			Register	regArg[3] ;
			regArg[0].i = m_addrPaint ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			regArg[2].i = m_addrRender ;
			//
			pVM->CallVirtualOnSysThread
				( m_addrPaint, WindowObject::vectorOnPaint, regArg, 3 ) ;
			//
			SSystem::Unlock() ;
		}
	}
}

// 描画前フレーム準備処理（全視点共通処理）
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2::WindowObject::SGLPaintCaller::OnPrepareFrame( SakuraGL::Window * pWnd )
{
	if ( (m_pWnd != NULL) && (m_addrPaint != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			SSystem::LockTrace( __FILE__, __LINE__ ) ;
			//
			Register	regArg[2] ;
			regArg[0].i = m_addrPaint ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			//
			pVM->CallVirtualOnSysThread
				( m_addrPaint, WindowObject::vectorOnPrepareFrame, regArg, 2 ) ;
			//
			SSystem::Unlock() ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// マウス関数呼び出し
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( ECSSakura2::WindowObject::SGLMouseCaller, SGLMouseInterface )

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::SGLMouseCaller::OnMouseMove
	( SakuraGL::Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	bool	fProcessed = false ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrMouse != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[5] ;
			regArg[0].i = m_addrMouse ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			regArg[2].i = xPos ;
			regArg[3].i = yPos ;
			regArg[4].i = nFlags ;
			//
			fProcessed = (pVM->CallVirtualOnSysThread
				( m_addrMouse, WindowObject::vectorOnMouseMove, regArg, 5 ) != 0) ;
		}
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

void WindowObject::SGLMouseCaller::OnMouseLeave
	( SakuraGL::Window * pWnd, int64_t nFlags )
{
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrMouse != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[3] ;
			regArg[0].i = m_addrMouse ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			regArg[2].i = nFlags ;
			//
			pVM->CallVirtualOnSysThread
				( m_addrMouse, WindowObject::vectorOnMouseLeave, regArg, 3 ) ;
		}
	}
	SSystem::Unlock() ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::SGLMouseCaller::OnMouseWheel
	( SakuraGL::Window * pWnd, int32_t zDelta,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	bool	fProcessed = false ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrMouse != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[6] ;
			regArg[0].i = m_addrMouse ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			regArg[2].i = zDelta ;
			regArg[3].i = xPos ;
			regArg[4].i = yPos ;
			regArg[5].i = nFlags ;
			//
			fProcessed = (pVM->CallVirtualOnSysThread
				( m_addrMouse, WindowObject::vectorOnMouseWheel, regArg, 6 ) != 0) ;
		}
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

// マウスボタン
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::SGLMouseCaller::OnButtonDown
	( SakuraGL::Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	bool	fProcessed = false ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrMouse != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[5] ;
			regArg[0].i = m_addrMouse ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			regArg[2].i = xPos ;
			regArg[3].i = yPos ;
			regArg[4].i = nFlags ;
			//
			fProcessed = (pVM->CallVirtualOnSysThread
				( m_addrMouse, WindowObject::vectorOnButtonDown, regArg, 5 ) != 0) ;
		}
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

bool WindowObject::SGLMouseCaller::OnButtonUp
	( SakuraGL::Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	bool	fProcessed = false ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrMouse != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[5] ;
			regArg[0].i = m_addrMouse ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			regArg[2].i = xPos ;
			regArg[3].i = yPos ;
			regArg[4].i = nFlags ;
			//
			fProcessed = (pVM->CallVirtualOnSysThread
				( m_addrMouse, WindowObject::vectorOnButtonUp, regArg, 5 ) != 0) ;
		}
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

bool WindowObject::SGLMouseCaller::OnButtonDblClk
	( SakuraGL::Window * pWnd,
		int32_t xPos, int32_t yPos, int64_t nFlags )
{
	bool	fProcessed = false ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrMouse != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[5] ;
			regArg[0].i = m_addrMouse ;
			regArg[1].i = (INT64) m_pWnd->m_dwHighAddr << 32 ;
			regArg[2].i = xPos ;
			regArg[3].i = yPos ;
			regArg[4].i = nFlags ;
			//
			fProcessed = (pVM->CallVirtualOnSysThread
				( m_addrMouse, WindowObject::vectorOnButtonDblClk, regArg, 5 ) != 0) ;
		}
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}


//////////////////////////////////////////////////////////////////////////////
// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::WindowObject::SGLProcedureCaller, SProcedure )

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void WindowObject::SGLProcedureCaller::Run( void )
{
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrProc != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[1] ;
			regArg[0].i = m_addrProc ;
			//
			pVM->CallVirtualOnSysThread
				( m_addrProc, ThreadObject::procVectorRun, regArg, 1 ) ;
		}
	}
	SSystem::Unlock() ;
}

// 開始前の処理
//////////////////////////////////////////////////////////////////////////////
void WindowObject::SGLProcedureCaller::Prepare( void )
{
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrProc != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[1] ;
			regArg[0].i = m_addrProc ;
			//
			pVM->CallVirtualOnSysThread
				( m_addrProc, ThreadObject::procVectorPrepare, regArg, 1 ) ;
		}
	}
	SSystem::Unlock() ;
}

// 完了後の処理
//////////////////////////////////////////////////////////////////////////////
void WindowObject::SGLProcedureCaller::Finalize( void )
{
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_pWnd != NULL) && (m_addrProc != 0) )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pWnd->m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[1] ;
			regArg[0].i = m_addrProc ;
			//
			pVM->CallVirtualOnSysThread
				( m_addrProc, ThreadObject::procVectorFinalize, regArg, 1 ) ;
		}
	}
	SSystem::Unlock() ;
	//
	delete	this ;
}


//////////////////////////////////////////////////////////////////////////////
// ウィンドウオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::WindowObject, Object, SGLWindow )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
WindowObject::WindowObject( VirtualMachine * vm )
{
	eslFillMemory( &m_paramWindow, 0, sizeof(CREATION_PARAM) ) ;
	m_paramWindow.statusCreation = createdNothing ;
	m_paramWindow.modeCopperation = modeWindow ;
	m_nStereoParam = 0 ;
	//
	eslFillMemory( &m_paramExFrame, 0, sizeof(EXTERIOR_FRAME_PARAM) ) ;
	//
	m_pVM = vm ;
	m_dwRenderObj = 0 ;
	//
	eslFillMemory( &m_handler, 0, sizeof(HANDLER_PARAM) ) ;
	//
	RegisterRenderObject( vm ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
WindowObject::~WindowObject( void )
{
	WindowObject::OnDestruction( NULL, NULL ) ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * WindowObject::GetTypeName( void ) const
{
	return	L"SakuraGL::Window" ;
}

// 破棄処理
//////////////////////////////////////////////////////////////////////////////
void WindowObject::OnDestruction
	( VirtualMachine * vm, Context * context )
{
	if ( m_paramWindow.statusCreation == createdDisplay )
	{
		CloseDisplay() ;
	}
	else if ( m_paramWindow.statusCreation == createdWindow )
	{
		CloseWindow() ;
	}
	if ( (m_dwRenderObj != 0) && (vm != NULL) )
	{
//		AssertLock() ;
//		vm->Lock() ;
		vm->FreeHeapObjectAddress
			( (((INT64) m_dwRenderObj) << 32), context ) ;
		m_dwRenderObj = 0 ;
//		vm->Unlock() ;
//		AssertUnlock() ;
	}
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError WindowObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	file->Write( &m_paramWindow, sizeof(CREATION_PARAM) ) ;
	file->WriteString( m_strWindowName ) ;
	file->WriteString( m_strStereoMethodID ) ;
	file->Write( &m_nStereoParam, sizeof(uint64_t) ) ;
	file->Write( &m_paramExFrame, sizeof(EXTERIOR_FRAME_PARAM) ) ;
	file->Write( &m_handler, sizeof(HANDLER_PARAM) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError WindowObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	file->Read( &m_paramWindow, sizeof(CREATION_PARAM) ) ;
	file->ReadString( m_strWindowName ) ;
	file->ReadString( m_strStereoMethodID ) ;
	file->Read( &m_nStereoParam, sizeof(uint64_t) ) ;
	file->Read( &m_paramExFrame, sizeof(EXTERIOR_FRAME_PARAM) ) ;
	file->Read( &m_handler, sizeof(HANDLER_PARAM) ) ;
	return	errSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError WindowObject::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	CREATION_PARAM			paramCreation = m_paramWindow ;
	SString					strWindowName = m_strWindowName ;
	SString					strStereoMethodID = m_strStereoMethodID ;
	uint64_t				nStereoParam = m_nStereoParam ;
	EXTERIOR_FRAME_PARAM	paramExFrame = m_paramExFrame ;
	//
	if ( paramCreation.statusCreation == createdDisplay )
	{
		EnableChangePhysicalMode( paramCreation.flagChangePhysMode ) ;
		SetOptionalFlags( paramCreation.nOptinalFlags ) ;
		CreateDisplay
			( strWindowName, paramCreation.modeCopperation,
				paramCreation.widthWindow, paramCreation.heightWindow,
				paramCreation.nBitsPerPixel, paramCreation.nFrequency ) ;
		SetOptionalFlags( paramCreation.nOptinalFlags ) ;
	}
	else if ( paramCreation.statusCreation == createdWindow )
	{
		SetOptionalFlags( paramCreation.nOptinalFlags ) ;
		CreateWindow
			( strWindowName,
				paramCreation.widthWindow, paramCreation.heightWindow,
				paramCreation.nCreationFlags,
				ESLTypeCast<SGLAbstractWindow>
					( vm->ObjectFromAddress( paramCreation.addrParentWnd ) ) ) ;
		SetWindowLayout
			( paramCreation.nLayoutFlags,
				paramCreation.ptLayout.x, paramCreation.ptLayout.y ) ;
	}
	if ( paramCreation.statusCreation != createdNothing )
	{
		EnableZBuffer( paramCreation.flagZBuffer ) ;
		//
		if ( !strStereoMethodID.IsEmpty() )
		{
			SetStereoDisplayMode( strStereoMethodID, nStereoParam ) ;
		}
		SetExteriorBackgroundFrame
			( paramExFrame.nFlags, paramExFrame.rgbColor,
				ESLTypeCast<SGLImageObject>
					( vm->ObjectFromAddress( paramExFrame.addrTile ) ),
				ESLTypeCast<SGLImageObject>
					( vm->ObjectFromAddress( paramExFrame.addrLeft ) ),
				ESLTypeCast<SGLImageObject>
					( vm->ObjectFromAddress( paramExFrame.addrRight ) ),
				ESLTypeCast<SGLImageObject>
					( vm->ObjectFromAddress( paramExFrame.addrUpper ) ),
				ESLTypeCast<SGLImageObject>
					( vm->ObjectFromAddress( paramExFrame.addrUnder ) ) ) ;
	}
	return	errSuccess ;
}

// レンダリングオブジェクトを仮想マシンに登録する
//////////////////////////////////////////////////////////////////////////////
void WindowObject::RegisterRenderObject( VirtualMachine * vm )
{
	RenderContextObject *	pRender =
		new RenderContextObject( L"SakuraGL::Window::RenderContext", NULL ) ;
	//
	AssertLock() ;
	vm->Lock() ;
	//
	m_dwRenderObj =
		(DWORD) (vm->AllocateHeapObjectAddress
						( pRender, mallocModeShared ) >> 32) ;
	//
	vm->Unlock() ;
	AssertUnlock() ;
}

// ウィンドウハンドラ適用
//////////////////////////////////////////////////////////////////////////////
void WindowObject::EnableWindowHandler( void )
{
	SetPaintInterface( &m_callPaint ) ;
	SetDirectPaintInterface( &m_callDirectPaint ) ;
	SetMouseInterface( &m_callMouse ) ;
	SetDirectMouseInterface( &m_callDirectMouse ) ;
	SetTimerInterface( this ) ;
	SetKeyInterface( this ) ;
	SetCharInputInterface( this ) ;
	SetCommandInterface( this ) ;
}

// 仮想ディスプレイ開始
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::CreateDisplay
	( const wchar_t * pszWindowName,
		CooperationMode mode,
		uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	SGLError	err = SGLWindow::CreateDisplay
		( pszWindowName, mode, nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
	if ( err )
	{
		return	err ;
	}
	m_paramWindow.statusCreation = createdDisplay ;
	m_paramWindow.modeCopperation = mode ;
	m_paramWindow.widthWindow = nWidth ;
	m_paramWindow.heightWindow = nHeight ;
	m_paramWindow.nBitsPerPixel = nBitsPerPixel ;
	m_paramWindow.nFrequency = nFrequency ;
	m_paramWindow.nOptinalFlags = SGLWindow::GetOptionalFlags() ;
	//
	EnableChangePhysicalMode( m_paramWindow.flagChangePhysMode ) ;
	EnableZBuffer( m_paramWindow.flagZBuffer ) ;
	//
	if ( !m_strStereoMethodID.IsEmpty() )
	{
		SetStereoDisplayMode( m_strStereoMethodID, m_nStereoParam ) ;
	}
	//
	EnableWindowHandler() ;
	//
	return	err ;
}

// 仮想ディスプレイ終了
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::CloseDisplay( void )
{
	SGLError	err = SGLWindow::CloseDisplay() ;
	//
	m_paramWindow.statusCreation = createdNothing ;
	eslFillMemory( &m_paramExFrame, 0, sizeof(EXTERIOR_FRAME_PARAM) ) ;
	//
	return	err ;
}

// オプション機能フラグ設定
//////////////////////////////////////////////////////////////////////////////
void WindowObject::SetOptionalFlags( uint64_t nFlags )
{
	m_paramWindow.nOptinalFlags = nFlags ;
	SGLWindow::SetOptionalFlags( nFlags ) ;
}

// ウィンドウモード変更
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::ChangeCooperationLevel( CooperationMode mode )
{
	SGLError	err = SGLWindow::ChangeCooperationLevel( mode ) ;
	if ( !err )
	{
		m_paramWindow.modeCopperation = mode ;
	}
	return	err ;
}

// 仮想ディスプレイサイズ変更
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::ChangeDisplaySize
	( uint32_t nWidth, uint32_t nHeight,
		uint32_t nBitsPerPixel, uint32_t nFrequency )
{
	SGLError	err =
		SGLWindow::ChangeDisplaySize
				( nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
	if ( !err )
	{
		m_paramWindow.widthWindow = nWidth ;
		m_paramWindow.heightWindow = nHeight ;
		m_paramWindow.nBitsPerPixel = nBitsPerPixel ;
		m_paramWindow.nFrequency = nFrequency ;
	}
	return	err ;
}

// 物理モニタの解像度を変更するか？
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::EnableChangePhysicalMode( bool fEnable )
{
	m_paramWindow.flagChangePhysMode = fEnable ;
	return	SGLWindow::EnableChangePhysicalMode( fEnable ) ;
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::EnableZBuffer( bool flagZBuffer )
{
	m_paramWindow.flagZBuffer = flagZBuffer ;
	return	SGLWindow::EnableZBuffer( flagZBuffer ) ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::SetStereoDisplayMode
	( const wchar_t * pszMethodID, uint64_t nParam )
{
	m_strStereoMethodID = pszMethodID ;
	m_nStereoParam = nParam ;
	return	SGLWindow::SetStereoDisplayMode( pszMethodID, nParam ) ;
}

// 仮想ディスプレイ・有効画面外枠表示設定
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::SetExteriorBackgroundFrame
	( uint32_t nFlags, uint32_t rgbColor,
		SakuraGL::SGLImageObject* pTile,
		SakuraGL::SGLImageObject* pLeft,
		SakuraGL::SGLImageObject* pRight,
		SakuraGL::SGLImageObject* pUpper,
		SakuraGL::SGLImageObject* pUnder )
{
	m_paramExFrame.nFlags = nFlags ;
	m_paramExFrame.rgbColor = rgbColor ;
	m_paramExFrame.addrTile = Object::GetHighAddressOf( pTile ) ;
	m_paramExFrame.addrLeft = Object::GetHighAddressOf( pLeft ) ;
	m_paramExFrame.addrRight = Object::GetHighAddressOf( pRight ) ;
	m_paramExFrame.addrUpper = Object::GetHighAddressOf( pUpper ) ;
	m_paramExFrame.addrUnder = Object::GetHighAddressOf( pUnder ) ;
	//
	return	SGLWindow::SetExteriorBackgroundFrame
				( nFlags, rgbColor, pTile, pLeft, pRight, pUpper, pUnder ) ;
}

// ウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::CreateWindow
	( const wchar_t * pszWindowName,
		uint32_t nWidth, uint32_t nHeight, uint32_t nFlags,
		SakuraGL::SGLAbstractWindow * pParentWnd )
{
	SGLError	err = SGLWindow::CreateWindow
		( pszWindowName, nWidth, nHeight, nFlags, pParentWnd ) ;
	if ( err )
	{
		return	err ;
	}
	m_paramWindow.statusCreation = createdWindow ;
	m_paramWindow.widthWindow = nWidth ;
	m_paramWindow.heightWindow = nHeight ;
	m_paramWindow.nCreationFlags = nFlags ;
	m_paramWindow.nOptinalFlags = SGLWindow::GetOptionalFlags() ;
	m_paramWindow.addrParentWnd = Object::GetHighAddressOf( pParentWnd ) ;
	m_paramWindow.nLayoutFlags = 0 ;
	//
	EnableZBuffer( m_paramWindow.flagZBuffer ) ;
	//
	EnableWindowHandler() ;
	//
	return	err ;
}

// ウィンドウを閉じる
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::CloseWindow( void )
{
	SGLError	err = SGLWindow::CloseWindow() ;
	//
	m_paramWindow.statusCreation = createdNothing ;
	eslFillMemory( &m_paramExFrame, 0, sizeof(EXTERIOR_FRAME_PARAM) ) ;
	//
	return	err ;
}

// ウィンドウ位置を設定する
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::SetWindowLayout
	( uint32_t nFlags, int xPos, int yPos )
{
	m_paramWindow.nLayoutFlags = nFlags ;
	m_paramWindow.ptLayout.x = xPos ;
	m_paramWindow.ptLayout.y = yPos ;
	//
	return	SGLWindow::SetWindowLayout( nFlags, xPos, yPos ) ;
}

// レンダリングスレッドで実行
//////////////////////////////////////////////////////////////////////////////
SGLError WindowObject::PostRenderingThread
	( uint64_t addrHandler, PostThreadType postType )
{
	SGLProcedureCaller *	pProc = new SGLProcedureCaller( this, addrHandler ) ;
	SGLError	err = SGLWindow::PostRenderingThread( pProc, postType ) ;
	if ( err )
	{
		delete	pProc ;
	}
	return	err ;
}

// UI スレッドで実行
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError WindowObject::PostUIThread( uint64_t addrHandler )
{
	SGLProcedureCaller *	pProc = new SGLProcedureCaller( this, addrHandler ) ;
	SGLError	err = SGLWindow::PostUIThread( pProc ) ;
	if ( err )
	{
		delete	pProc ;
	}
	return	err ;
}

// 描画ハンドラ
//////////////////////////////////////////////////////////////////////////////
uint64_t WindowObject::SetScriptPaintHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	//
	addrLast = m_handler.addrPaint;
	m_handler.addrPaint = addrHandler ;
	//
	m_callPaint.m_pWnd = this ;
	m_callPaint.m_addrPaint = addrHandler ;
	m_callPaint.m_addrRender = ((INT64) m_dwRenderObj) << 32 ;
	//
	SSystem::Unlock() ;
	return	addrLast ;
}

uint64_t WindowObject::SetScriptDirectPaintHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	//
	addrLast = m_handler.addrDirectPaint;
	m_handler.addrDirectPaint = addrHandler ;
	//
	m_callDirectPaint.m_pWnd = this ;
	m_callDirectPaint.m_addrPaint = addrHandler ;
	m_callDirectPaint.m_addrRender = ((INT64) m_dwRenderObj) << 32 ;
	//
	SSystem::Unlock() ;
	return	addrLast ;
}

// タイマーハンドラ
//////////////////////////////////////////////////////////////////////////////
uint64_t WindowObject::SetScriptTimerHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	addrLast = m_handler.addrTimer;
	m_handler.addrTimer = addrHandler ;
	SSystem::Unlock() ;
	return	addrLast ;
}

// マウス入力インターフェース
//////////////////////////////////////////////////////////////////////////////
uint64_t WindowObject::SetScriptMouseHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	//
	addrLast = m_handler.addrMouse;
	m_handler.addrMouse = addrHandler ;
	//
	m_callMouse.m_pWnd = this ;
	m_callMouse.m_addrMouse = addrHandler ;
	//
	SSystem::Unlock() ;
	return	addrLast ;
}

uint64_t WindowObject::SetScriptDirectMouseHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	//
	addrLast = m_handler.addrDirectMouse;
	m_handler.addrDirectMouse = addrHandler ;
	//
	m_callDirectMouse.m_pWnd = this ;
	m_callDirectMouse.m_addrMouse = addrHandler ;
	//
	SSystem::Unlock() ;
	return	addrLast ;
}

// キー入力インターフェース
//////////////////////////////////////////////////////////////////////////////
uint64_t WindowObject::SetScriptKeyHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::Lock() ;
	addrLast = m_handler.addrKey;
	m_handler.addrKey = addrHandler ;
	SSystem::Unlock() ;
	return	addrLast ;
}

// 文字入力インターフェース
//////////////////////////////////////////////////////////////////////////////
uint64_t WindowObject::SetScriptCharInputHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::Lock() ;
	addrLast = m_handler.addrCharInput;
	m_handler.addrCharInput = addrHandler ;
	SSystem::Unlock() ;
	return	addrLast ;
}

// コマンド・インターフェース
//////////////////////////////////////////////////////////////////////////////
uint64_t WindowObject::SetScriptCommandHandler( uint64_t addrHandler )
{
	uint64_t	addrLast ;
	SSystem::Lock() ;
	addrLast = m_handler.addrCommand;
	m_handler.addrCommand = addrHandler ;
	SSystem::Unlock() ;
	return	addrLast ;
}

// タイマーハンドラ
//////////////////////////////////////////////////////////////////////////////
void WindowObject::OnTimer( SakuraGL::Window * pWnd, uint64_t idTimer )
{
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrTimer != 0) && (pVM != NULL) )
	{
		Register	regArg[3] ;
		regArg[0].i = m_handler.addrTimer ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		regArg[2].i = idTimer ;
		//
		pVM->CallVirtualOnSysThread
			( m_handler.addrTimer, vectorOnTimer, regArg, 3 ) ;
	}
	SSystem::Unlock() ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::OnKeyDown
	( SakuraGL::Window * pWnd, int64_t nVirtKey, int64_t nFlags )
{
	bool			fProcessed = false ;
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrKey != 0) && (m_pVM != NULL) )
	{
		Register	regArg[4] ;
		regArg[0].i = m_handler.addrKey ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		regArg[2].i = nVirtKey ;
		regArg[3].i = nFlags ;
		//
		fProcessed = (pVM->CallVirtualOnSysThread
			( m_handler.addrKey, vectorOnKeyDown, regArg, 4 ) != 0) ;
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

bool WindowObject::OnKeyUp
	( SakuraGL::Window * pWnd, int64_t nVirtKey, int64_t nFlags )
{
	bool			fProcessed = false ;
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrKey != 0) && (pVM != NULL) )
	{
		Register	regArg[4] ;
		regArg[0].i = m_handler.addrKey ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		regArg[2].i = nVirtKey ;
		regArg[3].i = nFlags ;
		//
		fProcessed = (pVM->CallVirtualOnSysThread
			( m_handler.addrKey, vectorOnKeyUp, regArg, 4 ) != 0) ;
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

// フォーカス
//////////////////////////////////////////////////////////////////////////////
void WindowObject::OnSetFocus( SakuraGL::Window * pWnd )
{
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrKey != 0) && (pVM != NULL) )
	{
		Register	regArg[2] ;
		regArg[0].i = m_handler.addrKey ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		//
		pVM->CallVirtualOnSysThread
			( m_handler.addrKey, vectorOnSetFocus, regArg, 2 ) ;
	}
	SSystem::Unlock() ;
}

void WindowObject::OnKillFocus( SakuraGL::Window * pWnd )
{
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrKey != 0) && (pVM != NULL) )
	{
		Register	regArg[2] ;
		regArg[0].i = m_handler.addrKey ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		//
		pVM->CallVirtualOnSysThread
			( m_handler.addrKey, vectorOnKillFocus, regArg, 2 ) ;
	}
	SSystem::Unlock() ;
}

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::OnChar( SakuraGL::Window * pWnd, uint16_t codeChar )
{
	bool			fProcessed = false ;
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrCharInput != 0) && (pVM != NULL) )
	{
		Register	regArg[3] ;
		regArg[0].i = m_handler.addrCharInput ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		regArg[2].i = codeChar ;
		//
		fProcessed = (pVM->CallVirtualOnSysThread
			( m_handler.addrCharInput, vectorOnChar, regArg, 3 ) != 0) ;
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::OnStartComposition
	( SakuraGL::Window * pWnd,
			SakuraGL::SGLInputStartComposition& iscForm )
{
	bool			fProcessed = false ;
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrCharInput != 0) && (pVM != NULL) )
	{
		ThreadObject *	pSysThread = pVM->LockSystemThread() ;
		//
		INPUT_START_COMPOSITION	iscFormSrc ;
		iscFormSrc.nFlags = iscForm.nFlags ;
		iscFormSrc.ptStart = iscForm.ptStart ;
		iscFormSrc.rctArea = iscForm.rctArea ;
		iscFormSrc.fsFontStyle.nStyles = iscForm.fsFontStyle.nStyles ;
		iscFormSrc.fsFontStyle.nSize = iscForm.fsFontStyle.nSize ;
		iscFormSrc.fsFontStyle.pszFace = 0 ;
		//
		int	nPushedCount = 0 ;
		pSysThread->PushBinaryOnStack
			( nPushedCount, &iscFormSrc, sizeof(INPUT_START_COMPOSITION) ) ;
		//
		Register	regArg[3] ;
		regArg[0].i = m_handler.addrCharInput ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		regArg[2].i = pSysThread->m_regset[regSP].i ;
		//
		fProcessed = (pVM->CallVirtualOnSysThread
			( m_handler.addrCharInput, vectorOnStartComposition, regArg, 3 ) != 0) ;
		//
		INPUT_START_COMPOSITION *	pStartComp =
			(INPUT_START_COMPOSITION*)
					pSysThread->AtomicTranslateAddress
						( regArg[2].i, sizeof(INPUT_START_COMPOSITION) ) ;
		iscForm.nFlags = pStartComp->nFlags ;
		iscForm.ptStart = pStartComp->ptStart ;
		iscForm.rctArea = pStartComp->rctArea ;
		iscForm.fsFontStyle.nStyles = pStartComp->fsFontStyle.nStyles ;
		iscForm.fsFontStyle.nSize = pStartComp->fsFontStyle.nSize ;
		iscForm.fsFontStyle.pszFace =
			(const wchar_t *)
					pSysThread->AtomicTranslateAddress
						( pStartComp->fsFontStyle.pszFace ) ;
		//
		pSysThread->FreeStack( nPushedCount ) ;
		pVM->UnlockSystemThread( pSysThread ) ;
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::OnEndComposition( SakuraGL::Window * pWnd )
{
	bool			fProcessed = false ;
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrCharInput != 0) && (pVM != NULL) )
	{
		Register	regArg[2] ;
		regArg[0].i = m_handler.addrCharInput ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		//
		fProcessed = (pVM->CallVirtualOnSysThread
			( m_handler.addrCharInput, vectorOnEndComposition, regArg, 2 ) != 0) ;
	}
	SSystem::Unlock() ;
	return	fProcessed ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::OnCompositionString
	( SakuraGL::Window * pWnd,
			const SakuraGL::SGLInputCompositionString& icsComp )
{
	bool			fProcessed = false ;
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrCharInput != 0) && (pVM != NULL) )
	do
	{
		ThreadObject *	pSysThread = pVM->LockSystemThread() ;
		int				nStrPushedCount = 0 ;
		const wchar_t *	pwszErr ;
		pwszErr = pSysThread->PushStringOnStack
			( nStrPushedCount, icsComp.pszComposition ) ;
		if ( pwszErr != NULL )
		{
			pVM->HandleExceptionError( pSysThread, pwszErr ) ;
			pVM->UnlockSystemThread( pSysThread ) ;
			break ;
		}
		INT64	ptrComposition = pSysThread->m_regset[regSP].i ;
		//
		INPUT_COMPOSITION_STRING	icsCompStr ;
		icsCompStr.nFlags = icsComp.nFlags ;
		icsCompStr.pszComposition = ptrComposition ;
		icsCompStr.nStart = icsComp.nStart ;
		icsCompStr.nCount = icsComp.nCount ;
		//
		int	nCompPushedCount = 0 ;
		pSysThread->PushBinaryOnStack
			( nCompPushedCount, &icsCompStr, sizeof(INPUT_COMPOSITION_STRING) ) ;
		//
		Register	regArg[3] ;
		regArg[0].i = m_handler.addrCharInput ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		regArg[2].i = pSysThread->m_regset[regSP].i ;
		//
		fProcessed = (pVM->CallVirtualOnSysThread
			( m_handler.addrCharInput, vectorOnCompositionString, regArg, 3 ) != 0) ;
		//
		pSysThread->FreeStack( nCompPushedCount + nStrPushedCount ) ;
		pVM->UnlockSystemThread( pSysThread ) ;
	}
	while ( false ) ;
	SSystem::Unlock() ;
	return	fProcessed ;
}

// コマンド
//////////////////////////////////////////////////////////////////////////////
bool WindowObject::OnCommand
	( SakuraGL::Window * pWnd, const uint16_t * pszCmd,
						int64_t nParam, int64_t nCode )
{
	bool			fProcessed = false ;
	StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
	SSystem::LockTrace( __FILE__, __LINE__ ) ;
	if ( (m_handler.addrCommand != 0) && (pVM != NULL) )
	do
	{
		ThreadObject *	pSysThread = pVM->LockSystemThread() ;
		int				nStrPushedCount = 0 ;
		const wchar_t *	pwszErr ;
		pwszErr = pSysThread->PushStringOnStack
						( nStrPushedCount, SString( pszCmd ) ) ;
		if ( pwszErr != NULL )
		{
			pVM->HandleExceptionError( pSysThread, pwszErr ) ;
			pVM->UnlockSystemThread( pSysThread ) ;
			break ;
		}
		//
		Register	regArg[5] ;
		regArg[0].i = m_handler.addrCommand ;
		regArg[1].i = (INT64) m_dwHighAddr << 32 ;
		regArg[2].i = pSysThread->m_regset[regSP].i ;
		regArg[3].i = nParam ;
		regArg[4].i = nCode ;
		//
		fProcessed = (pVM->CallVirtualOnSysThread
			( m_handler.addrCommand, vectorOnCommand, regArg, 5 ) != 0) ;
		//
		pSysThread->FreeStack( nStrPushedCount ) ;
		pVM->UnlockSystemThread( pSysThread ) ;
	}
	while ( false ) ;
	SSystem::Unlock() ;
	return	fProcessed ;
}


#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::Window
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SakuraGL_Window, context, cls_id )
{
	return	new WindowObject( context->m_pSakura2VM ) ;
}

// SGLError SakuraGL::Window::CreateDisplay
//	( const char * pszWindowName,
//		CopperationMode mode, uint32_t nWidth, uint32_t nHeight,
//		uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_CreateDisplay, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::CreateDisplay ) ;
	const uint16_t *	pszWindowName =
		(const uint16_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->CreateDisplay
			( SString(pszWindowName),
				(SGLAbstractWindow::CooperationMode) arg[2].i,
				arg[3].l32, arg[4].l32, arg[5].l32, arg[6].l32 ) ;
	//
	return	NULL ;
}

// SGLError CloseDisplay( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_CloseDisplay, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::CloseDisplay ) ;
	//
	context->m_regset[regAcc].i = pWnd->CloseDisplay() ;
	//
	return	NULL ;
}

// uint64_t GetOptionalFlags( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_GetOptionalFlags, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::GetOptionalFlags ) ;
	//
	context->m_regset[regAcc].i = pWnd->GetOptionalFlags() ;
	//
	return	NULL ;
}

// void SetOptionalFlags( uint64_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_SetOptionalFlags, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::SetOptionalFlags ) ;
	//
	pWnd->SetOptionalFlags( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError ChangeCooperationLevel( CopperationMode mode ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_ChangeCooperationLevel, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::ChangeCooperationLevel ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->ChangeCooperationLevel
			( (SGLAbstractWindow::CooperationMode) arg[1].i ) ;
	//
	return	NULL ;
}

// CopperationMode GetCooperationLevel( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_GetCooperationLevel, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::GetCooperationLevel ) ;
	//
	context->m_regset[regAcc].i = pWnd->GetCooperationLevel() ;
	//
	return	NULL ;
}

// SGLError ChangeDisplaySize
//		( uint32_t nWidth, uint32_t nHeight,
//			uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_ChangeDisplaySize, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::ChangeDisplaySize ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->ChangeDisplaySize
			( arg[1].l32, arg[2].l32, arg[3].l32, arg[4].l32 ) ;
	//
	return	NULL ;
}

// SGLError GetDisplaySize( SGLSize& sizeDisplay ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_GetDisplaySize, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::GetDisplaySize ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLSize, pSizeDisplay,
				arg[1].i, sizeDisplay at Window::GetDisplaySize ) ;
	//
	context->m_regset[regAcc].i = pWnd->GetDisplaySize( *pSizeDisplay ) ;
	//
	return	NULL ;
}

// SGLError EnableChangePhysicalMode( bool fEnable ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_EnableChangePhysicalMode, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::EnableChangePhysicalMode ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->EnableChangePhysicalMode( arg[1].i != 0 ) ;
	//
	return	NULL ;
}

// SGLError EnableZBuffer( bool flagZBuffer ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_EnableZBuffer, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::EnableZBuffer ) ;
	//
	context->m_regset[regAcc].i = pWnd->EnableZBuffer( arg[1].i != 0 ) ;
	//
	return	NULL ;
}

// SGLError SetStereoDisplayMode
//		( const char * pszMethodID, uint64_t nParam = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetStereoDisplayMode, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::SetStereoDisplayMode ) ;
	const uint16_t *	pszMethodID =
		(const uint16_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->SetStereoDisplayMode( SString(pszMethodID), arg[2].i ) ;
	//
	return	NULL ;
}

// bool IsSupportedStereoDisplayMode( const char * pszMethodID ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_IsSupportedStereoDisplayMode, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::IsSupportedStereoDisplayMode ) ;
	const uint16_t *	pszMethodID =
		(const uint16_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->IsSupportedStereoDisplayMode( SString(pszMethodID) ) ? -1 : 0 ;
	//
	return	NULL ;
}

// SGLError InitWindowPosition
//	( int32_t xPos, int32_t yPos, const SGLSize * pInitExSize = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_InitWindowPosition, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::InitWindowPosition ) ;
	SGLSize *	pInitExSize =
		(SGLSize*) context->AtomicTranslateAddress( arg[3].i, sizeof(SGLSize) ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->InitWindowPosition
			( (int32_t) arg[1].i, (int32_t) arg[2].i, pInitExSize ) ;
	//
	return	NULL ;
}

// SGLError GetNormalWindowPosition( SGLPoint& ptWindow, SGLSize * pWindowSize ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_GetNormalWindowPosition, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::GetNormalWindowPosition ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLPoint, pptWindow,
				arg[1].i, ptWindow at Window::GetNormalWindowPosition ) ;
	SGLSize *	pWindowSize =
		(SGLSize*) context->AtomicTranslateAddress( arg[2].i, sizeof(SGLSize) ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->GetNormalWindowPosition( *pptWindow, pWindowSize ) ;
	//
	return	NULL ;
}

// SGLError GetInternalDisplayPosition
//				( SGLImageRect& rctRender, SGLImageRect& rctDisplay ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_GetInternalDisplayPosition, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::GetInternalDisplayPosition ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLImageRect, prctRender,
				arg[1].i, rctRender at Window::GetInternalDisplayPosition ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLImageRect, prctDisplay,
				arg[2].i, rctDisplay at Window::GetInternalDisplayPosition ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->GetInternalDisplayPosition( *prctRender, *prctDisplay ) ;
	//
	return	NULL ;
}

// SGLError SetExteriorBackgroundFrame
//		( uint32_t nFlags, uint32_t rgbColor, Image* pTile,
//			Image* pLeft = NULL, Image* pRight = NULL,
//			Image* pUpper = NULL, Image* pUnder = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetExteriorBackgroundFrame, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::SetExteriorBackgroundFrame ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->SetExteriorBackgroundFrame
			( arg[1].l32, arg[2].l32,
				ESLTypeCast<SGLImageObject>
					( vm->AtomicObjectFromAddress( arg[3].h32 ) ),
				ESLTypeCast<SGLImageObject>
					( vm->AtomicObjectFromAddress( arg[4].h32 ) ),
				ESLTypeCast<SGLImageObject>
					( vm->AtomicObjectFromAddress( arg[5].h32 ) ),
				ESLTypeCast<SGLImageObject>
					( vm->AtomicObjectFromAddress( arg[6].h32 ) ),
				ESLTypeCast<SGLImageObject>
					( vm->AtomicObjectFromAddress( arg[7].h32 ) ) ) ;
	//
	return	NULL ;
}

// SGLError CreateWindow
//	( const char * pszWindowName,
//		uint32_t nWidth, uint32_t nHeight,
//		uint32_t nFlags = 0, Window * pParentWnd = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_CreateWindow, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::CreateWindow ) ;
	const uint16_t *	pszWindowName =
		(const uint16_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->CreateWindow
			( SString(pszWindowName),
				arg[2].l32, arg[3].l32, arg[4].l32,
				ESLTypeCast<SGLAbstractWindow>
					( vm->AtomicObjectFromAddress( arg[5].h32 ) ) ) ;
	//
	return	NULL ;
}

// SGLError CloseWindow( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_CloseWindow, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::CloseWindow ) ;
	//
	context->m_regset[regAcc].i = pWnd->CloseWindow() ;
	//
	return	NULL ;
}

// SGLError SetWindowLayout
//		( uint32_t nFlags, int xPos = 0, int yPos = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_SetWindowLayout, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::SetWindowLayout ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->SetWindowLayout
			( arg[1].l32, (int) arg[2].i, (int) arg[3].i ) ;
	//
	return	NULL ;
}

// S2DDVector& ScreenPositionFromClient( S2DDVector& vClient ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_ScreenPositionFromClient, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::ScreenPositionFromClient ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S2DDVector, vClient,
				arg[1].i, vClient at Window::ScreenPositionFromClient ) ;
	//
	context->m_regset[regAcc] = arg[1] ;
	pWnd->ScreenPositionFromClient( *vClient ) ;
	//
	return	NULL ;
}

// S2DDVector& ClientPositionFromScreen( S2DDVector& vScreen ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_ClientPositionFromScreen, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::ClientPositionFromScreen ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S2DDVector, vScreen,
				arg[1].i, vScreen at Window::ClientPositionFromScreen ) ;
	//
	context->m_regset[regAcc] = arg[1] ;
	pWnd->ClientPositionFromScreen( *vScreen ) ;
	//
	return	NULL ;
}

// SGLError PostUpdate( const SGLImageRect* pUpdate = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_PostUpdate, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::PostUpdate ) ;
	SGLImageRect *	pUpdate = NULL ;
	if ( arg[1].i != 0 )
	{
		pUpdate = (SGLImageRect*)
					context->AtomicTranslateAddress
							( arg[1].i, sizeof(SGLImageRect) ) ;
	}
	//
	context->m_regset[regAcc].i = pWnd->PostUpdate( pUpdate ) ;
	//
	return	NULL ;
}

// SGLError UpdateWindow( Window::UpdateParameter * pUpdate ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_UpdateWindow, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::UpdateWindow ) ;
	//
	Window::UpdateParameter * pUpdate = NULL ;
	if ( arg[1].i != 0 )
	{
		pUpdate = (Window::UpdateParameter*)
					context->AtomicTranslateAddress
							( arg[1].i, sizeof(Window::UpdateParameter) ) ;
	}
	context->m_regset[regAcc].i = pWnd->UpdateWindow( pUpdate ) ;
	//
	return	NULL ;
}

// SGLError ProcessUserInput( int64_t msecTimeout = 1 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_ProcessUserInput, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::ProcessUserInput ) ;
	//
	context->m_regset[regAcc].i = pWnd->ProcessUserInput( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError PostRenderingThread( SSystem::SProcedure * pProc, PostThreadType postType ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_PostRenderingThread, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd, arg, Window::PostRenderingThread ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->PostRenderingThread
			( arg[1].i, (SGLAbstractWindow::PostThreadType) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError PostUIThread( SSystem::SProcedure * pProc ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_PostUIThread, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd, arg, Window::PostUIThread ) ;
	//
	context->m_regset[regAcc].i = pWnd->PostUIThread( arg[1].i ) ;
	//
	return	NULL ;
}

// bool IsWindowActive( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_IsWindowActive, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::IsWindowActive ) ;
	//
	context->m_regset[regAcc].i = pWnd->IsWindowActive() ? -1 : 0 ;
	//
	return	NULL ;
}

// void SetWindowCaption( const wchar_t * pszWindowName ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_SetWindowCaption, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::SetWindowCaption ) ;
	const uint16_t *	pszWindowName =
		(const uint16_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	//
	context->m_regset[regAcc].i =
				pWnd->SetWindowCaption( SString(pszWindowName) ) ;
	//
	return	NULL ;
}

// SGLError ShowCursor( bool fShow ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_ShowCursor, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::ShowCursor ) ;
	//
	context->m_regset[regAcc].i = pWnd->ShowCursor( arg[1].i != 0 ) ;
	//
	return	NULL ;
}

// bool IsShowCursor( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_IsShowCursor, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::IsShowCursor ) ;
	//
	context->m_regset[regAcc].i = pWnd->IsShowCursor() ? -1 : 0 ;
	//
	return	NULL ;
}

// void SetCursor( const wchar_t * pszCursorID ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_SetCursor, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::SetCursor ) ;
	const uint16_t *	pszCursorID =
		(const uint16_t*) context->AtomicTranslateAddress( arg[1].i ) ;
	//
	context->m_regset[regAcc].i = pWnd->SetCursor( SString(pszCursorID) ) ;
	//
	return	NULL ;
}

// SGLError MoveCursorPosition
//		( int32_t xPos, int32_t yPos, int idMouse = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_MoveCursorPosition, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::MoveCursorPosition ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->MoveCursorPosition
			( (int32_t) arg[1].i, (int32_t) arg[2].i, (int) arg[3].i ) ;
	//
	return	NULL ;
}

// SGLError GetCursorPosition
//		( SGLPoint& ptCursor, int idMouse = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_GetCursorPosition, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::GetCursorPosition ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLPoint, pptCursor,
				arg[1].i, ptCursor at Window::GetCursorPosition ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->GetCursorPosition( *pptCursor, (int) arg[2].i ) ;
	//
	return	NULL ;
}

// int GetMonitorFrequency( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_GetMonitorFrequency, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::GetMonitorFrequency ) ;
	//
	context->m_regset[regAcc].i = pWnd->GetMonitorFrequency() ;
	//
	return	NULL ;
}

// SGLError AttachMenu( WindowMenu * pMenu ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_AttachMenu, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::AttachMenu ) ;
	//
	context->m_regset[regAcc].i =
		pWnd->AttachMenu
			( ESLTypeCast<SGLWindowMenu>
					( vm->AtomicObjectFromAddress( arg[1].h32 ) ) ) ;
	//
	return	NULL ;
}

// SGLPaintInterface * SetPaintInterface( SGLPaintInterface * pPaint ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetPaintInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd, arg, Window::SetPaintInterface ) ;
	//
	context->m_regset[regAcc].i = pWnd->SetScriptPaintHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLPaintInterface * SetDirectPaintInterface( SGLPaintInterface * pPaint ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetDirectPaintInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd, arg, Window::SetDirectPaintInterface ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->SetScriptDirectPaintHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLTimerInterface * SetTimerInterface( SGLTimerInterface * pTimer ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetTimerInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd, arg, Window::SetTimerInterface ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->SetScriptTimerHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLMouseInterface * SetMouseInterface( SGLMouseInterface * pMouse ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetMouseInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd, arg, Window::SetMouseInterface ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->SetScriptMouseHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLMouseInterface * SetDirectMouseInterface( SGLMouseInterface * pMouse ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetDirectMouseInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd,
				arg, Window::SetDirectMouseInterface ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->SetScriptDirectMouseHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError CaptureMouse( int idMouse = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Window_CaptureMouse, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::CaptureMouse ) ;
	//
	context->m_regset[regAcc].i = pWnd->CaptureMouse( (int) arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError ReleaseMouse( int idMouse = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Window_ReleaseMouse, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd, arg, Window::ReleaseMouse ) ;
	//
	context->m_regset[regAcc].i = pWnd->ReleaseMouse( (int) arg[1].i ) ;
	//
	return	NULL ;
}

// SGLKeyInterface * SetKeyInterface( SGLKeyInterface * pKey ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetKeyInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd,
				arg, Window::SetKeyInterface ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->SetScriptKeyHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLCharInputInterface * SetCharInputInterface( SGLCharInputInterface * pChar ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetCharInputInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd,
				arg, Window::SetCharInputInterface ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->SetScriptCharInputHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLCommandInterface * SetCommandInterface( SGLCommandInterface * pCmd ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SakuraGL_Window_SetCommandInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowObject, pWnd,
				arg, Window::SetCommandInterface ) ;
	//
	context->m_regset[regAcc].i =
			pWnd->SetScriptCommandHandler( arg[1].i ) ;
	//
	return	NULL ;
}

// RenderContext* GetRenderContext( S3DRenderContextInterface::StereoViewIndex sviView ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_GetRenderContext, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::GetRenderContext ) ;
	//
	S3DRenderContextInterface *	pRender =
		pWnd->GetRenderContext
			( (S3DRenderContextInterface::StereoViewIndex) arg[1].i ) ;
	context->m_regset[regAcc].i = 0 ;
	if ( pRender != NULL )
	{
		AssertLock() ;
		vm->Lock() ;
		context->m_regset[regAcc].i =
			vm->AllocateHeapObjectAddress
				( new RenderContextObject
					( L"SakuraGL::Window::RenderContext", pRender ) ) ;
		vm->Unlock() ;
		AssertUnlock() ;
	}
	//
	return	NULL ;
}

// void ReleaseRenderContext( RenderContext* context ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_Window_ReleaseRenderContext, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLAbstractWindow, pWnd,
				arg, Window::ReleaseRenderContext ) ;
	//
	S3DRenderContextInterface *	pRender =
		ESLTypeCast<S3DRenderContextInterface>
				( vm->ObjectFromAddress( arg[1].h32 ) ) ;
	pWnd->ReleaseRenderContext( pRender ) ;
	//
//	AssertLock() ;
//	vm->Lock() ;
	vm->FreeHeapObjectAddress( arg[1].i, context ) ;
//	vm->Unlock() ;
//	AssertUnlock() ;
	//
	return	NULL ;
}

#endif
