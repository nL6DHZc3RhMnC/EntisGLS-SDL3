
/*****************************************************************************
           ステレオ 3D ディスプレイ用プラグインインターフェース
 *****************************************************************************/

#if	!defined(__EGL3D_STEREO_DISPLAY__)
#define	__EGL3D_STEREO_DISPLAY__	1

#if	!defined(_WINDEF_)
#include <windef.h>
#endif

#include <multimon.h>


namespace	E3DSDisplayPlugin
{

// フラグ
//////////////////////////////////////////////////////////////////////////////

// エラーコード
enum	Error
{
	errSuccess	= 0,			// 成功
	errFailed,					// 失敗
	errInvalidParameter,
	errInsufficientResource,
	errNotSupported,
	errDeviceNotReady,
} ;

// 動作フラグ
enum	BehaviorFlag
{
	flagNeedToChangeDisplayMode	= 0x0001,	// 画面モードの変更が必要
	flagLostImageAfterView		= 0x0002,	// 画像バッファはテンポラリ
} ;

// 画像バッファフラグ
enum	ImageBufferFlag
{
	flagMemoryImage			= 0x0001,		// メモリ配列
	flagSourceRectangle		= 0x0002,		// ソース矩形
	flagDeviceContext		= 0x0004,		// デバイスコンテキスト
	flagBitmapHandle		= 0x0008,		// ビットマップハンドル
	flagDirectDrawSurface7	= 0x0020,		// DirectDraw サーフェス
} ;

// 画像フォーマット
enum	ImageFormat
{
	formatRGB	= 0,			// RGB
} ;

// 画像バッファ指標
enum	StereoBufferIndex
{
	stereoRightBuffer	= 0,
	stereoLeftBuffer,
	stereoBufferCount,
} ;

// ウィンドウプロシージャフック動作
enum	WindowProcMethod
{
	methodContinue	= 0,		// ウィンドウプロシージャ継続
	methodReturnProc,			// ウィンドウプロシージャ終了
} ;

// ディスプレイモード変更動作
enum	DisplayModeMethod
{
	methodNoModeChanged	= 0,	// 画面モードは変更されなかった
	methodModeChanged,			// 画面モードは変更された
	methodModeChangedAndWindow,	// 画面モードは変更され、
								// ウィンドウ制御もビューアが行う
} ;

// 画像描画動作フラグ
enum	DrawBufferFlag
{
	drawDynamic		= 0x0001,	// 中間バッファに書き込まない
	drawTemporary	= 0x0002,	// 出力用バッファの内容は ViewImage 呼出し後
								// 失われても良い
} ;

// 画面表示動作フラグ
enum	ViewImageFlag
{
	viewNoWait		= 0,
	viewWaitVSync	= 0x0001,	// VSYNC を待つ
} ;


// 画像バッファ
//////////////////////////////////////////////////////////////////////////////
struct	ImageRect
{
	long int	x ;
	long int	y ;
	long int	w ;
	long int	h ;
} ;

struct	ImageBuffer
{
	long int	nFlags ;		// complex of ImageBufferFlags
	long int	nFormat ;		// enum ImageFormat
	long int	nWidth ;
	long int	nHeight ;
	long int	nBitsPerPixel ;
	long int	nBytesPerLine ;
	void *		pBuffer ;
	ImageRect	rctSource ;
	HDC			hDC ;
	HBITMAP		hBitmap ;
	void *		iddsSurface ;	// IDirectDrawSurface7 *

	// EGL_IMAGE_INFO 構造体から変換
	void ConvertFrom( const EGL_IMAGE_INFO & imginf ) ;
} ;


// ステレオ画像バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
struct	I3DImageView
{
	void (*pfnRelease)( I3DImageView * piv ) ;
	long int (*pfnBehavior)( I3DImageView * piv ) ;
	long int (*pfnImageBufferCapacity)( I3DImageView * piv ) ;
	long int (*pfnBestImageBufferCapacity)( I3DImageView * piv ) ;
	Error (*pfnAttachWindow)( I3DImageView * piv, HWND hWnd ) ;
	Error (*pfnDetachWindow)( I3DImageView * piv ) ;
	Error (*pfnAttachThread)( I3DImageView * piv ) ;
	Error (*pfnDetachThread)( I3DImageView * piv ) ;
	WindowProcMethod (*pfnWindowProc)
		( I3DImageView * piv, HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam, LRESULT * pResult ) ;
	Error (*pfnSetBufferSize)
		( I3DImageView * piv,
				long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) ;
	Error (*pfnSetViewPosition)
		( I3DImageView * piv, int xDst, int yDst, int nWidth, int nHeight ) ;
	DisplayModeMethod (*pfnOnChangeDisplayMode)
		( I3DImageView * piv, HMONITOR hMonitor,
			int nWidth, int nHeight, int nBitsPerPixel, int nFrequency ) ;
	DisplayModeMethod (*pfnOnRestoreDisplayMode)( I3DImageView * piv ) ;
	Error (*pfnDrawBuffer)
		( I3DImageView * piv,
			long int nFlags, int x, int y,
				const ImageBuffer * bufImage[], int nViewCount ) ;
	Error (*pfnPrepareView)( I3DImageView * piv ) ;
	Error (*pfnViewImage)( I3DImageView * piv, long int nFlags ) ;

	void Release( void )
		{	pfnRelease( this ) ;	}
	long int Behavior( void )
		{	return	pfnBehavior( this ) ;	}
	long int ImageBufferCapacity( void )
		{	return	pfnImageBufferCapacity( this ) ;	}
	long int BestImageBufferCapacity( void )
		{	return	pfnBestImageBufferCapacity( this ) ;	}
	Error AttachWindow( HWND hWnd )
		{	return	pfnAttachWindow( this, hWnd ) ;	}
	Error DetachWindow( void )
		{	return	pfnDetachWindow( this ) ;	}
	Error AttachThread( void )
		{	return	pfnAttachThread( this ) ;	}
	Error DetachThread( void )
		{	return	pfnDetachThread( this ) ;	}
	WindowProcMethod WindowProc
		( HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam, LRESULT * pResult )
		{	return	pfnWindowProc
				( this, hWnd, uMsg, wParam, lParam, pResult ) ;	}
	Error SetBufferSize
			( long int nFlags, long int nFormat,
				int nWidth, int nHeight, int nViewCount )
		{	return	pfnSetBufferSize
				( this, nFlags, nFormat, nWidth, nHeight, nViewCount ) ;	}
	Error SetViewPosition( int xDst, int yDst, int nWidth, int nHeight )
		{	return	pfnSetViewPosition( this, xDst, yDst, nWidth, nHeight ) ;	}
	DisplayModeMethod OnChangeDisplayMode
			( HMONITOR hMonitor,
				int nWidth, int nHeight, int nBitsPerPixel, int nFrequency )
		{	return	pfnOnChangeDisplayMode
				( this, hMonitor, nWidth, nHeight, nBitsPerPixel, nFrequency ) ;	}
	DisplayModeMethod OnRestoreDisplayMode( void )
		{	return	pfnOnRestoreDisplayMode( this ) ;	}
	Error DrawBuffer
		( long int nFlags, int x, int y,
			const ImageBuffer * bufImage[], int nViewCount )
		{	return	pfnDrawBuffer( this, nFlags, x, y, bufImage, nViewCount ) ;	}
	Error PrepareView( void )
		{	return	pfnPrepareView( this ) ;	}
	Error ViewImage( long int nFlags )
		{	return	pfnViewImage( this, nFlags ) ;	}
} ;

// プラグインインターフェース
//////////////////////////////////////////////////////////////////////////////
enum	PluginTypes
{
	plugin3DStereoDisplay	= 0x00020000
} ;

struct	INTERFACE_ENTRY
{
	DWORD	dwPluginType ;				// = plugin3DStereoDisplay
	Error (*pfnStartup)( void * pReserved ) ;
	Error (*pfnShutdown)( void ) ;
	const char * (*pfnGetPluginID)( void ) ;
	Error (*pfnGetPluginName)( char * pszBuf, int * pBufLen ) ;
	int (*pfnIsDeviceSupported)( void ) ;
	void (*pfnPropertyUI)( HWND hwndParent ) ;
	I3DImageView * (*pfnCreateStereoImageView)( void ) ;
} ;

typedef	INTERFACE_ENTRY * PINTERFACE_ENTRY ;
typedef	const INTERFACE_ENTRY * PCINTERFACE_ENTRY ;


// プラグインエントリポイント
//////////////////////////////////////////////////////////////////////////////
typedef	PCINTERFACE_ENTRY
		(*PluginEntryPointProc)( DWORD_PTR dwType, int iEntry ) ;

// モジュールローダ
//////////////////////////////////////////////////////////////////////////////
class	Module
{
protected:
	HMODULE				m_hModule ;
	PCINTERFACE_ENTRY	m_ientry ;

public:
	Module( void ) : m_hModule(NULL), m_ientry(NULL) {}
	~Module( void )
		{	Release() ;	}
	Error LoadModule( const char * pszFileName )
		{
			Release( ) ;
			m_hModule = ::LoadLibrary( pszFileName ) ;
			if ( m_hModule == NULL )
			{
				return	errFailed ;
			}
			PluginEntryPointProc	pfnEntryPoint =
				(PluginEntryPointProc) ::GetProcAddress
						( m_hModule, "EntryPointProc" ) ;
			if ( pfnEntryPoint == NULL )
			{
				return	errFailed ;
			}
			m_ientry = pfnEntryPoint( plugin3DStereoDisplay, 0 ) ;
			if ( m_ientry == NULL )
			{
				return	errFailed ;
			}
			if ( m_ientry->dwPluginType != plugin3DStereoDisplay )
			{
				m_ientry = NULL ;
				return	errFailed ;
			}
			return	(m_ientry->pfnStartup)( NULL ) ;
		}
	void Release( void )
		{
			if ( m_ientry != NULL )
			{
				(m_ientry->pfnShutdown)( ) ;
				m_ientry = NULL ;
			}
			if ( m_hModule != NULL )
			{
				::FreeLibrary( m_hModule ) ;
				m_hModule = NULL ;
			}
		}

public:
	const INTERFACE_ENTRY * Interface( void ) const
		{
			return	m_ientry ;
		}
	const char * GetPluginID( void ) const
		{
			if ( m_ientry == NULL )
			{
				return	NULL ;
			}
			return	(m_ientry->pfnGetPluginID)( ) ;
		}
	Error GetPluginName( char * pszBuf, int * pBufLen )
		{
			if ( m_ientry == NULL )
			{
				return	errFailed ;
			}
			return	(m_ientry->pfnGetPluginName)( pszBuf, pBufLen ) ;
		}
	int IsDeviceSupported( void )
		{
			if ( m_ientry == NULL )
			{
				return	0 ;
			}
			return	(m_ientry->pfnIsDeviceSupported)( ) ;
		}
	void PropertyUI( HWND hwndParent )
		{
			if ( m_ientry != NULL )
			{
				(m_ientry->pfnPropertyUI)( hwndParent ) ;
			}
		}
	I3DImageView * CreateStereoImageView( void )
		{
			if ( m_ientry == NULL )
			{
				return	NULL ;
			}
			return	(m_ientry->pfnCreateStereoImageView)( ) ;
		}

} ;


} ;

#endif
