
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_gdi_window_producer.h>
#include <d3d9.h>
#include <dxerr.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// GDI ウィンドウ・表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLWin32GDIWindowProducer, SGLSoftwareRenderWindowProducer )

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLWin32GDIWindowProducer::FlipView
	( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread )
{
	SGLImageWin32DIBitmap *	pDIB =
		SGLImageWin32DIBitmap::CommitDIB( &m_imgPhysical ) ;
	if ( pDIB != NULL )
	{
		HWND	hWnd = pWnd->GetWindowHandle() ;
		HDC		hDC = ::GetDC( hWnd ) ;
		::BitBlt
			( hDC, 0, 0,
				m_sizePhysical.w, m_sizePhysical.h,
				pDIB->m_hDC, 0, 0, SRCCOPY ) ;
		::ReleaseDC( hWnd, hDC ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// Direct3D9 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLWin32D3D9WindowProducer, SGLSoftwareRenderWindowProducer )

HMODULE	SGLWin32D3D9WindowProducer::s_hModuleD3D9 = nullptr ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWin32D3D9WindowProducer::SGLWin32D3D9WindowProducer( void )
	: m_hWnd( NULL ),
		m_id3d9( nullptr ), m_id3d9Dev( nullptr ),
		m_pd3dpp( nullptr ), m_idds9DispBuf( nullptr ),
		m_sizeD3D9DispBuf( 0, 0 )
{
	if ( s_hModuleD3D9 == nullptr )
	{
		s_hModuleD3D9 = ::LoadLibrary( "d3d9.dll" ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWin32D3D9WindowProducer::~SGLWin32D3D9WindowProducer( void )
{
	ReleaseD3D9Device() ;
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLWin32D3D9WindowProducer::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	ResetD3D9Device( (int) nWidth, (int) nHeight ) ;
	SGLSoftwareRenderWindowProducer::OnChangePhysicalViewSize( pWnd, nWidth, nHeight ) ;
}

// ウィンドウに関連付けられた（作成された）
//////////////////////////////////////////////////////////////////////////////
void SGLWin32D3D9WindowProducer::OnAttachedWindow( SGLAbstractWindow * pWnd )
{
	HWND	hWnd = pWnd->GetWindowHandle() ;
	//
	RECT	rect = { 0, 0, 1, 1 } ;
	::GetClientRect( hWnd, &rect ) ;
	//
	CreateD3D9Device
		( hWnd, rect.right - rect.left, rect.bottom - rect.top, 0, TRUE ) ;
	//
	SGLSoftwareRenderWindowProducer::OnAttachedWindow( pWnd ) ;
}

// ウィンドウから分離された（ウィンドウが破棄される）
//////////////////////////////////////////////////////////////////////////////
void SGLWin32D3D9WindowProducer::OnDetachedWindow( SGLAbstractWindow * pWnd )
{
	ReleaseD3D9Device() ;
	//
	SGLSoftwareRenderWindowProducer::OnDetachedWindow( pWnd ) ;
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLWin32D3D9WindowProducer::FlipView
	( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread )
{
	DrawImageToD3D9( &m_imgPhysical, 0, 0 ) ;
}

// Direct3D9 初期化
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32D3D9WindowProducer::CreateD3D9Device
	( HWND hWnd, int nWidth, int nHeight, int nAdapter, BOOL fWindowed )
{
	if ( s_hModuleD3D9 == nullptr )
	{
		return	sglErrFailed ;
	}
	if ( m_id3d9 != nullptr )
	{
		ReleaseD3D9Device() ;
	}
	//
	// Dreict3D9 生成
	//
	typedef	IDirect3D9 * (WINAPI *API_Direct3DCreate9)( UINT SDKVersion ) ;
	API_Direct3DCreate9	apiDirect3DCreate9 =
		(API_Direct3DCreate9)
			::GetProcAddress( s_hModuleD3D9, "Direct3DCreate9" ) ;
	if ( apiDirect3DCreate9 == nullptr )
	{
		ESLTrace( "Not found Direct3DCreate.\n" ) ;
		return	sglErrFailed ;
	}
	m_id3d9 = apiDirect3DCreate9( D3D_SDK_VERSION ) ;
	if ( m_id3d9 == nullptr )
	{
		ESLTrace( "Failed to create Direct3D9.\n" ) ;
		return	sglErrFailed ;
	}
	//
	// Direct3DDevice9 生成
	//
	if ( m_pd3dpp == nullptr )
	{
		m_pd3dpp = new D3DPRESENT_PARAMETERS ;
	}
	memset( m_pd3dpp, 0, sizeof(D3DPRESENT_PARAMETERS) ) ;
	m_hWnd = hWnd ;
	m_pd3dpp->Windowed = fWindowed ;
	m_pd3dpp->EnableAutoDepthStencil = TRUE ;
	m_pd3dpp->AutoDepthStencilFormat = D3DFMT_D16 ;
	m_pd3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD ;
	m_pd3dpp->BackBufferWidth = nWidth ;
	m_pd3dpp->BackBufferHeight = nHeight ;
	m_pd3dpp->BackBufferFormat = D3DFMT_A8R8G8B8 ;
	m_pd3dpp->PresentationInterval = D3DPRESENT_INTERVAL_ONE ;
	m_pd3dpp->BackBufferCount = 1 ;
	//
	if ( m_id3d9->CreateDevice
			( nAdapter, D3DDEVTYPE_HAL, hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
								m_pd3dpp, &m_id3d9Dev ) != D3D_OK )
	{
		ESLTrace( "Failed to create Direct3DDevice9.\n" ) ;
		//
		ReleaseD3D9Device() ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// Direct3D9 開放
//////////////////////////////////////////////////////////////////////////////
void SGLWin32D3D9WindowProducer::ReleaseD3D9Device( void )
{
	ReleaseD3D9Surface() ;
	//
	if ( m_idds9DispBuf != nullptr )
	{
		m_idds9DispBuf->Release() ;
		m_idds9DispBuf = nullptr ;
	}
	if ( m_id3d9Dev != nullptr )
	{
		m_id3d9Dev->Release() ;
		m_id3d9Dev = nullptr ;
	}
	if ( m_id3d9 != nullptr )
	{
		m_id3d9->Release() ;
		m_id3d9 = nullptr ;
	}
	if ( m_pd3dpp != nullptr )
	{
		delete	m_pd3dpp ;
		m_pd3dpp = nullptr ;
	}
}

// Direct3D9 リセット
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32D3D9WindowProducer::ResetD3D9Device( int nWidth, int nHeight )
{
	if ( m_id3d9 && m_id3d9Dev )
	{
		ReleaseD3D9Surface() ;
		//
		m_pd3dpp->BackBufferWidth = nWidth ;
		m_pd3dpp->BackBufferHeight = nHeight ;
		//
		HRESULT	hr = m_id3d9Dev->Reset( m_pd3dpp ) ;
		if ( hr != D3D_OK )
		{
			ESLTrace( "failed ot IDirect3DDevice9::Reset %d x %d (%08X).",
					m_pd3dpp->BackBufferWidth,
					m_pd3dpp->BackBufferHeight, hr ) ;
			return	sglErrFailed ;
		}
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// サーフェースを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32D3D9WindowProducer::ReleaseD3D9Surface( void )
{
	if ( m_idds9DispBuf != NULL )
	{
		m_idds9DispBuf->Release() ;
		m_idds9DispBuf = nullptr ;
	}
	return	sglErrSuccess ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32D3D9WindowProducer::DrawImageToD3D9
			( SGLImageObject * pImage, int xDst, int yDst )
{
	if ( (pImage == nullptr)
		|| (m_id3d9Dev == nullptr) )
	{
		return	sglErrInvalidParam ;
	}
	//
	// 描画用サーフェス準備
	//
	SGLSize	sizeImage = pImage->GetImageSize() ;
	if ( (m_idds9DispBuf == nullptr)
		|| (sizeImage != m_sizeD3D9DispBuf) )
	{
		if ( m_idds9DispBuf != nullptr )
		{
			m_idds9DispBuf->Release() ;
			m_idds9DispBuf = nullptr ;
		}
		m_id3d9Dev->CreateOffscreenPlainSurface
			( sizeImage.w, sizeImage.h, D3DFMT_A8R8G8B8,
						D3DPOOL_DEFAULT, &m_idds9DispBuf, nullptr ) ;
		if ( m_idds9DispBuf == nullptr )
		{
			ESLTrace( "Failed to CreateOffscreenPlainSurface.\n" ) ;
			return	sglErrFailed ;
		}
		m_sizeD3D9DispBuf = sizeImage ;
	}
	//
	// 描画用サーフェスに画像データ転送
	//
	D3DSURFACE_DESC	d3dsd ;
	if ( m_idds9DispBuf->GetDesc( &d3dsd ) != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DSurface9::GetDesc.\n" ) ;
		return	sglErrFailed ;
	}
	D3DLOCKED_RECT	lr ;
	HRESULT	hr = m_idds9DispBuf->LockRect( &lr, nullptr, 0 ) ;
	if ( hr != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DSurface9::LockRect.(%08X)\n", hr ) ;
		return	sglErrFailed ;
	}
	SGLImageInfo	infSurface ;
	infSurface.format = formatImageARGB ;
	infSurface.width = (uint32_t) d3dsd.Width ;
	infSurface.height = (uint32_t) d3dsd.Height ;
	infSurface.depth = 32 ;
	infSurface.pitchPixel = 4 ;
	infSurface.pitchLine = lr.Pitch ;
	//
	pImage->ReadFrameBuffer( infSurface, (uint8_t*) lr.pBits ) ;
	//
	m_idds9DispBuf->UnlockRect() ;
	//
	// 画面に描画
	//
	if ( m_id3d9Dev->BeginScene() != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::BeginScene.\n" ) ;
	}
	IDirect3DSurface9 *	id3dsBack = nullptr ;
	if ( m_id3d9Dev->GetBackBuffer
		( 0, 0, D3DBACKBUFFER_TYPE_MONO, &id3dsBack ) != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::GetBackBuffer.\n" ) ;
		return	sglErrFailed ;
	}
	if ( m_id3d9Dev->Clear
		( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				D3DCOLOR_ARGB(0xFF,0,0,0), 1.0, 0 ) != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::Clear.\n" ) ;
	}
	RECT	rcSrc ;
	rcSrc.left = 0 ;
	rcSrc.top = 0 ;
	rcSrc.right = sizeImage.w ;
	rcSrc.bottom = sizeImage.h ;
	//
	RECT	rcDst ;
	rcDst.left = xDst ;
	rcDst.top = yDst ;
	rcDst.right = xDst + sizeImage.w ;
	rcDst.bottom = yDst + sizeImage.h ;
	//
	SGLError	err = sglErrSuccess ;
	hr = m_id3d9Dev->StretchRect
		( m_idds9DispBuf, &rcSrc,
			id3dsBack, &rcDst, D3DTEXF_LINEAR ) ;
	if ( hr != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::StretchRect.(%08X)\n", hr ) ;
		ESLTrace( "(%08X:%s)\n", hr, DXGetErrorString(hr) ) ;
		err = sglErrFailed ;
	}
	if ( m_id3d9Dev->EndScene() != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::EndScene.\n" ) ;
	}
	hr = m_id3d9Dev->Present( NULL, NULL, NULL, NULL ) ;
	//
	id3dsBack->Release() ;
	//
	if ( hr != D3D_OK )
	{
		ESLTrace( "Failed to IDirect3DDevice9::Present.(%08X)\n", hr ) ;
		ESLTrace( "(%08X:%s)\n", hr, DXGetErrorString(hr) ) ;
		err = sglErrFailed ;
		//
		if ( (hr == D3DERR_DEVICEREMOVED) || (hr == D3DERR_DEVICELOST)
			&& (m_id3d9Dev->TestCooperativeLevel() != D3DERR_DEVICELOST) )
		{
			// ロストデバイスの復帰
			RECT	rect = { 0, 0, 1, 1 } ;
			::GetClientRect( m_hWnd, &rect ) ;
			//
			ResetD3D9Device( rect.right - rect.left, rect.bottom - rect.top ) ;
		}
	}
	return	err ;
}


