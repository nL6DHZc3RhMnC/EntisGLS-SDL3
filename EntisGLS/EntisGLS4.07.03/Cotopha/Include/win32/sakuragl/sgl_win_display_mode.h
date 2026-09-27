
#if	!defined(__SAKURAGL_WIN_DISPLAY_MODE_H__)
#define	__SAKURAGL_WIN_DISPLAY_MODE_H__	1

#include <multimon.h>

#if	defined(GetSystemMetrics)
#undef	GetSystemMetrics
#endif


namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ディスプレイモード
	//////////////////////////////////////////////////////////////////////////

	class	SGLDisplayMode	: public ESLObject
	{
	protected:
		DEVMODE		m_dmMonitor ;
		bool		m_flagChanged ;

		typedef BOOL (WINAPI *API_EnumDisplayMonitors)
			( HDC hdc, LPCRECT lprcClip,
				MONITORENUMPROC lpfnEnum, LPARAM dwData ) ;
		typedef HMONITOR
			(WINAPI *API_MonitorFromRect)
				( IN LPCRECT lprc, IN DWORD dwFlags ) ;
		typedef BOOL
			(WINAPI *API_GetMonitorInfo)
				( IN HMONITOR hMonitor, OUT LPMONITORINFO lpmi ) ;
		typedef LONG (WINAPI *API_ChangeDisplaySettingsEx)
				( IN LPCSTR lpszDeviceName,
					IN LPDEVMODEA lpDevMode,
					IN HWND hwnd, IN DWORD dwflags, IN LPVOID lParam ) ;
	    typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXA)
					( GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR ) ;
		typedef	HRESULT (WINAPI *API_DirectDrawEnumerateEx)
			( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags ) ;

		HMODULE						m_hUser32 ;
		API_EnumDisplayMonitors		m_apiEnumDisplayMonitors ;
		API_MonitorFromRect			m_apiMonitorFromRect ;
		API_GetMonitorInfo			m_apiGetMonitorInfo ;
		API_ChangeDisplaySettingsEx	m_apiChangeDisplaySettingsEx ;

		HMODULE						m_hDDraw ;
		API_DirectDrawEnumerateEx	m_apiDirectDrawEnumerateEx ;

	public:
		typedef	HMONITOR	MonitorHandle ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLDisplayMode, ESLObject )
		// 構築関数
		SGLDisplayMode( void ) ;
		// 消滅関数
		virtual ~SGLDisplayMode( void ) ;

	public:
		// user32.dll をロードして各種 API を初期化する
		void PrepareMonitorAPIs( void ) ;
		// ディスプレイを列挙
		SGLError EnumerateDisplay
			( SSystem::SArray<MonitorHandle> & lstMonitors ) ;
		// ディスプレイを取得
		const wchar_t * GetDisplayNameFromRect
			( SSystem::SString & strDisplayName,
				const SGLImageRect * pRect, MonitorHandle * phMonitor = NULL ) ;
		// ディスプレイの矩形を取得
		SGLError GetMonitorRect
			( MonitorHandle hMonitor,
				SGLImageRect& rectMonitor, SGLImageRect& rectVirtualWork ) ;
		SGLError GetMonitorRect
			( const wchar_t * pwszDisplayName,
				SGLImageRect& rectMonitor,
				SGLImageRect& rectVirtualWork,
				MonitorHandle * phMonitor = NULL ) ;
		// ウィンドウ位置を正規化（画面内へ補正）
		void NormalizeWindowPos
			( SGLPoint & ptWindow, SGLSize & sizeWindow, bool fNoSize ) ;
		// DirectDraw 用ディスプレイの GUID を取得する
		GUID * GetDDMonitorGUID
			( GUID * pGUID, MonitorHandle hMonitor,
							size_t * pMonitorIndex = NULL ) ;

	protected:
		static BOOL CALLBACK MyMonitorEnumProc
			( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData ) ;

	public:
		// ディスプレイのビット深度を取得
		static uint32_t GetDisplayColorBitCount
					( const wchar_t * pszDisplayName = NULL ) ;
		// ディスプレイの周波数を取得
		static uint32_t GetDisplayFrequency
					( const wchar_t * pszDisplayName = NULL ) ;

	public:
		// 一致レベル
		enum	MatchingLevel
		{
			matchNo	= 0,
			matchAboveSize,		// 少なくとも要求サイズ以上
			matchEqualSize,		// 少なくともサイズは一致
			matchSizeAndColor,	// サイズと色ビット数は一致
			matchBest,			// 全てが一致
		} ;
		// 指定モードに切り替え可能かテストする
		MatchingLevel TestDisplayMode
			( uint32_t nWidth, uint32_t nHeight,
				uint32_t nBitsPerPixel = 0, uint32_t nFrequency = 0,
				const wchar_t * pszDisplayName = NULL ) ;
		// ディスプレイモードを切り替える
		SGLError ChangeDisplayMode( const wchar_t * pszDisplayName = NULL ) ;
		// ディスプレイモードを元に戻す
		SGLError RestoreDisplayMode( void ) ;
		// ディスプレイモードを変更しているか？
		bool IsChangedDisplayMode( void ) const
		{
			return	m_flagChanged ;
		}
	} ;
}

#endif

