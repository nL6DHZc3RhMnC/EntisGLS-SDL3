
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_directx_object.h>
#include <sakuragl/sgl_win_display_mode.h>
#include <d3d9.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Direct3D9 オブジェクト・通知インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLDirect3D9Device::INotify, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D9Device::INotify::INotify( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D9Device::INotify::~INotify( void )
{
}

// 依存先オブジェクト生成／再生成後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::INotify::OnCreateObject
		( SDependentNotificationServer * pObject, bool fInitialize )
{
	SGLDirect3D9Device *	pDevice =
		ESLTypeCast<SGLDirect3D9Device>( pObject ) ;
	if ( pDevice != NULL )
	{
		OnCreateDevice( pDevice, fInitialize ) ;
	}
}

// 依存先のリソースが解放される／リセットされる前等に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::INotify::OnReleaseObject
				( SDependentNotificationServer * pObject )
{
	SGLDirect3D9Device *	pDevice =
		ESLTypeCast<SGLDirect3D9Device>( pObject ) ;
	if ( pDevice != NULL )
	{
		OnReleaseDevice( pDevice ) ;
	}
}

// 依存先オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::INotify::OnFinalizeObject
				( SDependentNotificationServer * pObject )
{
	SGLDirect3D9Device *	pDevice =
		ESLTypeCast<SGLDirect3D9Device>( pObject ) ;
	if ( pDevice != NULL )
	{
		OnFinalizeDevice( pDevice ) ;
	}
}

// Direct3D オブジェクトが作成された後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::INotify::OnCreateDevice
		( SGLDirect3D9Device * pd3dDev, bool fInitialize )
{
}

// Direct3D オブジェクトがリセットされる前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::INotify::OnReleaseDevice( SGLDirect3D9Device * pd3dDev )
{
}

// Direct3D オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::INotify::OnFinalizeDevice( SGLDirect3D9Device * pd3dDev )
{
}


//////////////////////////////////////////////////////////////////////////////
// Direct3D9 オブジェクト
//////////////////////////////////////////////////////////////////////////////

HMODULE	SGLDirect3D9Device::m_hModuleD3D9 = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLDirect3D9Device, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D9Device::SGLDirect3D9Device( void )
{
	m_id3d9 = NULL ;
	m_id3d9Dev = NULL ;
	m_pd3dpp = NULL ;
	m_fFullscreen = false ;
	m_pFirstNotify = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDirect3D9Device::~SGLDirect3D9Device( void )
{
}

// Direct3D9 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D9Device::CreateDevice
	( HWND hWnd,
		unsigned int nWidth,
		unsigned int nHeight, bool fFullscreen )
{
	D3DPRESENT_PARAMETERS	d3dpp ;
	memset( &d3dpp, 0, sizeof(D3DPRESENT_PARAMETERS) ) ;
	d3dpp.Windowed = !fFullscreen ;
	d3dpp.EnableAutoDepthStencil = TRUE ;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16 ;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD ;
	d3dpp.BackBufferWidth = nWidth ;
	d3dpp.BackBufferHeight = nHeight ;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8 ;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE ;
	d3dpp.BackBufferCount = 1 ;
	//
	RECT	rctWindow ;
	if ( !::GetWindowRect( hWnd, &rctWindow ) )
	{
		return	sglErrFailed ;
	}
	SGLDisplayMode::MonitorHandle	hMonitor ;
	SGLDisplayMode	dm ;
	SString			strDispName ;
	SGLImageRect	irWindow
		( rctWindow.left, rctWindow.top,
			rctWindow.right - rctWindow.left,
				rctWindow.bottom - rctWindow.top ) ;
	if ( dm.GetDisplayNameFromRect
		( strDispName, &irWindow, &hMonitor ) == NULL )
	{
		return	sglErrFailed ;
	}
	GUID	guidMonitor ;
	size_t	iMonitor ;
	if ( dm.GetDDMonitorGUID
		( &guidMonitor, hMonitor, &iMonitor ) == NULL )
	{
		return	sglErrFailed ;
	}
	return	CreateDevice( hWnd, (UINT) iMonitor, &d3dpp ) ;
}

SGLError SGLDirect3D9Device::CreateDevice
	( HWND hWnd, UINT nAdapter,
		const _D3DPRESENT_PARAMETERS_ * pd3dpp )
{
	if ( m_id3d9Dev != NULL )
	{
		return	sglErrFailed ;
	}
	//
	// Direct3D9 生成
	//
	if ( m_hModuleD3D9 == NULL )
	{
		m_hModuleD3D9 = ::LoadLibrary( "d3d9.dll" ) ;
		if ( m_hModuleD3D9 == NULL )
		{
			ESLTrace( "d3d9.dll をロードできませんでした。\n" ) ;
			return	sglErrFailed ;
		}
	}
	typedef	IDirect3D9 * (WINAPI *API_Direct3DCreate9)( UINT SDKVersion ) ;
	API_Direct3DCreate9	apiDirect3DCreate9 =
		(API_Direct3DCreate9)
			::GetProcAddress( m_hModuleD3D9, "Direct3DCreate9" ) ;
	if ( apiDirect3DCreate9 == NULL )
	{
		ESLTrace( "Direct3DCreate が見つかりませんでした。\n" ) ;
		return	sglErrFailed ;
	}
	if ( m_id3d9 == NULL )
	{
		m_id3d9 = apiDirect3DCreate9( D3D_SDK_VERSION ) ;
		if ( m_id3d9 == NULL )
		{
			ESLTrace( "Direct3D の生成に失敗しました。" ) ;
			return	sglErrFailed ;
		}
	}
	//
	// Direct3DDevice9 生成
	//
	if ( m_pd3dpp == NULL )
	{
		m_pd3dpp = new D3DPRESENT_PARAMETERS ;
	}
	*m_pd3dpp = *pd3dpp ;
	//
	if ( m_id3d9->CreateDevice
			( nAdapter /*D3DADAPTER_DEFAULT*/,
				D3DDEVTYPE_HAL, hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING
							/*| D3DCREATE_MULTITHREADED*/,
								m_pd3dpp, &m_id3d9Dev ) != D3D_OK )
	{
		ESLTrace( "Direct3DDevice9 の生成に失敗しました。\n" ) ;
		//
		Release() ;
		return	sglErrFailed ;
	}
	m_fFullscreen = !m_pd3dpp->Windowed ;
	//
	NotifyAllOnCreateDevice( true ) ;
	//
	return	sglErrSuccess ;
}

// Direct3D9 リセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirect3D9Device::ResetDevice
	( const _D3DPRESENT_PARAMETERS_ * pd3dpp )
{
	if ( (m_id3d9Dev == NULL) || (m_pd3dpp == NULL) )
	{
		return	sglErrFailed ;
	}
	if ( pd3dpp != NULL )
	{
		*m_pd3dpp = *pd3dpp ;
	}
	NotifyAllOnReleaseDevice() ;
	//
	HRESULT	hr = m_id3d9Dev->Reset( m_pd3dpp ) ;
	if ( hr != D3D_OK )
	{
		ESLTrace( "IDirect3DDevice9::Reset %d x %d は"
								"失敗しました。(%08X)\n",
				m_pd3dpp->BackBufferWidth,
				m_pd3dpp->BackBufferHeight, hr ) ;
		return	sglErrFailed ;
	}
	//
	NotifyAllOnCreateDevice( false ) ;
	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::Release( void )
{
	NotifyAllOnFinalizeDevice() ;
	//
	if ( m_id3d9Dev != NULL )
	{
		m_id3d9Dev->Release() ;
		m_id3d9Dev = NULL ;
	}
	if ( m_id3d9 != NULL )
	{
		m_id3d9->Release() ;
		m_id3d9 = NULL ;
	}
	if ( m_pd3dpp != NULL )
	{
		delete	m_pd3dpp ;
		m_pd3dpp = NULL ;
	}
}

// 通知オブジェクトを追加する
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::AddNotify( SGLDirect3D9Device::INotify * pNotify )
{
	AddNotification( pNotify ) ;
}

// 通知を解除する
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::DetachNotify( SGLDirect3D9Device::INotify * pNotify )
{
	DetachNotification( pNotify ) ;
}

// OnCreateDevice を通知する
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::NotifyAllOnCreateDevice( bool fInitialize )
{
	NotifyAllOnCreate( fInitialize ) ;
}

// OnReleaseDevice を通知する
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::NotifyAllOnReleaseDevice( void )
{
	NotifyAllOnRelease() ;
}

// OnFinalizeDevice を通知する
//////////////////////////////////////////////////////////////////////////////
void SGLDirect3D9Device::NotifyAllOnFinalizeDevice( void )
{
	NotifyAllOnFinalize() ;
}

// DirectX 9 がインストールされているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLDirect3D9Device::IsInstalledDirectX9( void )
{
	if ( m_hModuleD3D9 == NULL )
	{
		m_hModuleD3D9 = ::LoadLibrary( "d3d9.dll" ) ;
		if ( m_hModuleD3D9 == NULL )
		{
			return	false ;
		}
	}
	return	true ;
}

