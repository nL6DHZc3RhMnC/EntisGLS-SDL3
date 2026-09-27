
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_win_display_mode.h>
#include <ddraw.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ディスプレイモード
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLDisplayMode, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDisplayMode::SGLDisplayMode( void )
{
	m_flagChanged = false ;
	//
	m_hUser32 = NULL ;
	m_apiEnumDisplayMonitors = NULL ;
	m_apiMonitorFromRect = NULL ;
	m_apiGetMonitorInfo = NULL ;
	m_apiChangeDisplaySettingsEx = NULL ;
	//
	m_hDDraw = NULL ;
	m_apiDirectDrawEnumerateEx = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDisplayMode::~SGLDisplayMode( void )
{
	if ( m_flagChanged )
	{
		RestoreDisplayMode() ;
	}
	if ( m_hUser32 != NULL )
	{
		::FreeLibrary( m_hUser32 ) ;
		m_hUser32 = NULL ;
	}
	if ( m_hDDraw != NULL )
	{
		::FreeLibrary( m_hDDraw ) ;
		m_hDDraw = NULL ;
	}
}

// user32.dll をロードして各種 API を初期化する
//////////////////////////////////////////////////////////////////////////////
void SGLDisplayMode::PrepareMonitorAPIs( void )
{
	if ( m_hUser32 == NULL )
	{
		m_hUser32 = ::LoadLibrary( "user32.dll" ) ;
		m_apiEnumDisplayMonitors =
			(API_EnumDisplayMonitors)
				::GetProcAddress( m_hUser32, "EnumDisplayMonitors" ) ;
		m_apiMonitorFromRect =
			(API_MonitorFromRect)
				::GetProcAddress( m_hUser32, "MonitorFromRect" ) ;
		m_apiGetMonitorInfo =
			(API_GetMonitorInfo)
				::GetProcAddress( m_hUser32, "GetMonitorInfoA" ) ;
		m_apiChangeDisplaySettingsEx =
			(API_ChangeDisplaySettingsEx)
				::GetProcAddress( m_hUser32, "ChangeDisplaySettingsExA" ) ;
	}
}

// ディスプレイを列挙
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDisplayMode::EnumerateDisplay
	( SSystem::SArray<MonitorHandle> & lstMonitors )
{
	PrepareMonitorAPIs() ;
	if ( m_apiEnumDisplayMonitors == NULL )
	{
		return	sglErrFailed ;
	}
	m_apiEnumDisplayMonitors
		( NULL, NULL, &SGLDisplayMode::MyMonitorEnumProc, (LPARAM) &lstMonitors ) ;
	return	sglErrSuccess ;
}

BOOL CALLBACK SGLDisplayMode::MyMonitorEnumProc
	( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	SArray<MonitorHandle> *	plstMonitors = (SArray<MonitorHandle>*) dwData ;
	plstMonitors->Add( hMonitor ) ;
	return	TRUE ;
}

// ディスプレイを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLDisplayMode::GetDisplayNameFromRect
	( SSystem::SString & strDisplayName,
		const SGLImageRect * pRect, SGLDisplayMode::MonitorHandle * phMonitor )
{
	strDisplayName.FreeArray( ) ;
	//
	PrepareMonitorAPIs() ;
	if ( (m_apiMonitorFromRect == NULL) || (m_apiGetMonitorInfo == NULL) )
	{
		return	NULL ;
	}
	RECT		rectMonitor ;
	rectMonitor.left = pRect->x ;
	rectMonitor.top = pRect->y ;
	rectMonitor.right = pRect->x + pRect->w ;
	rectMonitor.bottom = pRect->y + pRect->h ;
	//
	HMONITOR	hMonitor =
		m_apiMonitorFromRect( &rectMonitor, MONITOR_DEFAULTTONEAREST ) ;
	if ( hMonitor == NULL )
	{
		return	NULL ;
	}
	MONITORINFOEX	mix ;
	mix.cbSize = sizeof(mix) ;
	if ( !m_apiGetMonitorInfo( hMonitor, &mix ) )
	{
		return	NULL ;
	}
	strDisplayName = mix.szDevice ;
	if ( phMonitor != NULL )
	{
		*phMonitor = hMonitor ;
	}
	return	strDisplayName ;
}

// ディスプレイの矩形を取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDisplayMode::GetMonitorRect
	( SGLDisplayMode::MonitorHandle hMonitor,
		SGLImageRect& rectMonitor, SGLImageRect& rectVirtualWork )
{
	PrepareMonitorAPIs() ;
	if ( m_apiGetMonitorInfo == NULL )
	{
		return	sglErrFailed ;
	}
	MONITORINFO	mi ;
	mi.cbSize = sizeof(MONITORINFO) ;
	if ( !m_apiGetMonitorInfo( hMonitor, &mi ) )
	{
		return	sglErrFailed ;
	}
	rectMonitor.x = mi.rcMonitor.left ;
	rectMonitor.y = mi.rcMonitor.top ;
	rectMonitor.w = mi.rcMonitor.right - mi.rcMonitor.left ;
	rectMonitor.h = mi.rcMonitor.bottom - mi.rcMonitor.top ;
	rectVirtualWork.x = mi.rcWork.left ;
	rectVirtualWork.y = mi.rcWork.top ;
	rectVirtualWork.w = mi.rcWork.right - mi.rcWork.left ;
	rectVirtualWork.h = mi.rcWork.bottom - mi.rcWork.top ;
	return	sglErrSuccess ;
}

SGLError SGLDisplayMode::GetMonitorRect
	( const wchar_t * pwszDisplayName,
		SGLImageRect& rectMonitor,
		SGLImageRect& rectVirtualWork,
		SGLDisplayMode::MonitorHandle * phMonitor )
{
	PrepareMonitorAPIs() ;
	if ( m_apiGetMonitorInfo == NULL )
	{
		return	sglErrFailed ;
	}
	SArray<MonitorHandle>	lstMonitors ;
	EnumerateDisplay( lstMonitors ) ;
	//
	MonitorHandle *	pHandles = lstMonitors.GetArray() ;
	size_t			nCount = lstMonitors.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MONITORINFOEX	mix ;
		mix.cbSize = sizeof(mix) ;
		if ( m_apiGetMonitorInfo( pHandles[i], &mix ) )
		{
			SString	strDisplayName = mix.szDevice ;
			if ( strDisplayName == pwszDisplayName )
			{
				rectMonitor.x = mix.rcMonitor.left ;
				rectMonitor.y = mix.rcMonitor.top ;
				rectMonitor.w = mix.rcMonitor.right - mix.rcMonitor.left ;
				rectMonitor.h = mix.rcMonitor.bottom - mix.rcMonitor.top ;
				rectVirtualWork.x = mix.rcWork.left ;
				rectVirtualWork.y = mix.rcWork.top ;
				rectVirtualWork.w = mix.rcWork.right - mix.rcWork.left ;
				rectVirtualWork.h = mix.rcWork.bottom - mix.rcWork.top ;
				//
				if ( phMonitor != NULL )
				{
					*phMonitor = pHandles[i] ;
				}
				lstMonitors.FinishArray() ;
				return	sglErrSuccess ;
			}
		}
	}
	lstMonitors.FinishArray() ;
	return	sglErrFailed ;
}

// ウィンドウ位置を正規化（画面内へ補正）
//////////////////////////////////////////////////////////////////////////////
void SGLDisplayMode::NormalizeWindowPos
	( SGLPoint & ptWindow, SGLSize & sizeWindow, bool fNoSize )
{
	//
	// モニタ範囲取得
	//
	SGLImageRect	rctMonitor ;
	SString			strDisplayName ;
	MonitorHandle	hMonitor ;
	SGLImageRect	rctWindow ;
	//
	rctMonitor.x = 0 ;
	rctMonitor.y = 0 ;
	rctMonitor.w = ::GetSystemMetrics( SM_CXSCREEN ) ;
	rctMonitor.h = ::GetSystemMetrics( SM_CYSCREEN ) ;
	//
	rctWindow.x = ptWindow.x ;
	rctWindow.y = ptWindow.y ;
	rctWindow.w = sizeWindow.w ;
	rctWindow.h = sizeWindow.h ;
	//
	if ( GetDisplayNameFromRect
		( strDisplayName, &rctWindow, &hMonitor ) != NULL )
	{
		SGLImageRect	rctGetMonitor, rctVirtualWork ;
		if ( !GetMonitorRect( hMonitor, rctGetMonitor, rctVirtualWork ) )
		{
			rctMonitor = rctGetMonitor ;
		}
	}
	//
	// 座標補正
	//
	if ( ptWindow.x + sizeWindow.w > rctMonitor.x + rctMonitor.w )
	{
		ptWindow.x = rctMonitor.x + rctMonitor.w - sizeWindow.w ;
	}
	if ( ptWindow.x < rctMonitor.x )
	{
		ptWindow.x = rctMonitor.x ;
	}
	if ( ptWindow.y + sizeWindow.h > rctMonitor.y + rctMonitor.h )
	{
		ptWindow.y = rctMonitor.y + rctMonitor.h - sizeWindow.h ;
	}
	if ( ptWindow.y < rctMonitor.y )
	{
		ptWindow.y = rctMonitor.y ;
	}
	//
	// サイズ補正
	//
	if ( !fNoSize )
	{
		if ( ptWindow.x + sizeWindow.w > rctMonitor.x + rctMonitor.w )
		{
			sizeWindow.w = rctMonitor.x + rctMonitor.w - ptWindow.x ;
		}
		if ( ptWindow.y + sizeWindow.h > rctMonitor.y + rctMonitor.h )
		{
			sizeWindow.h = rctMonitor.y + rctMonitor.h - ptWindow.y ;
		}
	}
}

// DirectDraw 用ディスプレイの GUID を取得する
//////////////////////////////////////////////////////////////////////////////
static BOOL WINAPI DDEnumCallback_GetDDMonitorGUID
	( GUID FAR *lpGUID, LPSTR lpDriverDescription,
		LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm ) ;
struct	DDEnumCallback_GetDDMonitorGUID_Param
{
	bool		flagFound ;
	size_t		nIndex ;
	HMONITOR	hMonitor ;
	GUID		guidFound ;
} ;

GUID * SGLDisplayMode::GetDDMonitorGUID
	( GUID * pGUID, SGLDisplayMode::MonitorHandle hMonitor, size_t * pMonitorIndex )
{
	if ( m_hDDraw == NULL )
	{
		m_hDDraw = ::LoadLibrary( "ddraw.dll" ) ;
		if ( m_hDDraw == NULL )
		{
			return	NULL ;
		}
		m_apiDirectDrawEnumerateEx =
			(API_DirectDrawEnumerateEx)
				::GetProcAddress( m_hDDraw, "DirectDrawEnumerateExA" ) ;
	}
	if ( m_apiDirectDrawEnumerateEx == NULL )
	{
		return	NULL ;
	}
	if ( pMonitorIndex != NULL )
	{
		*pMonitorIndex = 0 ;
	}
	DDEnumCallback_GetDDMonitorGUID_Param	param ;
	param.flagFound = false ;
	param.nIndex = 0 ;
	param.hMonitor = hMonitor ;
	//
	m_apiDirectDrawEnumerateEx
		( DDEnumCallback_GetDDMonitorGUID,
			&param, DDENUM_ATTACHEDSECONDARYDEVICES ) ;
	if ( !param.flagFound )
	{
		return	NULL ;
	}
	*pGUID = param.guidFound ;
	if ( (pMonitorIndex != NULL) && (param.nIndex > 0) )
	{
		*pMonitorIndex = param.nIndex - 1 ;
	}
	return	pGUID ;
}

static BOOL WINAPI DDEnumCallback_GetDDMonitorGUID
	( GUID FAR *lpGUID, LPSTR lpDriverDescription,
		LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm )
{
	DDEnumCallback_GetDDMonitorGUID_Param *	pParam =
		(DDEnumCallback_GetDDMonitorGUID_Param*) lpContext ;
	if ( (hm == pParam->hMonitor) && (lpGUID != NULL) )
	{
		pParam->guidFound = *lpGUID ;
		pParam->flagFound = true ;
		return	false ;
	}
	pParam->nIndex ++ ;
	return	true ;
}

// ディスプレイのビット深度を取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLDisplayMode::GetDisplayColorBitCount
							( const wchar_t * pszDisplayName )
{
	HDC	hDC = ::CreateCompatibleDC( NULL ) ;
	if ( hDC == NULL )
	{
		return	0 ;
	}
	uint32_t	nBitsPerPixel = (uint32_t) ::GetDeviceCaps( hDC, BITSPIXEL ) ;
	::DeleteDC( hDC ) ;
	return	nBitsPerPixel ;
}

// ディスプレイの周波数を取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLDisplayMode::GetDisplayFrequency
						( const wchar_t * pszDisplayName )
{
	DEVMODE			dmCurrent ;
	SString			strDisplayName = pszDisplayName ;
	SArray<char>	bufTemp ;
	if ( !::EnumDisplaySettings
		( strDisplayName.EncodeDefaultTo(bufTemp),
					ENUM_CURRENT_SETTINGS, &dmCurrent ) )
	{
		return	0 ;
	}
	return	(uint32_t) dmCurrent.dmDisplayFrequency ;
}

// 指定モードに切り替え可能かテストする
//////////////////////////////////////////////////////////////////////////////
SGLDisplayMode::MatchingLevel
	SGLDisplayMode::TestDisplayMode
		( uint32_t nWidth, uint32_t nHeight,
			uint32_t nBitsPerPixel, uint32_t nFrequency,
			const wchar_t * pwszDisplayName )
{
	if ( nBitsPerPixel == 0 )
	{
		nBitsPerPixel = GetDisplayColorBitCount( pwszDisplayName ) ;
	}
	if ( nFrequency == 0 )
	{
		nFrequency = GetDisplayFrequency( pwszDisplayName ) ;
	}
	DWORD			iModeNum = 0 ;
	SString			strDisplayName = pwszDisplayName ;
	SArray<char>	bufTemp ;
	const char *	pszDisplayName = strDisplayName.EncodeDefaultTo(bufTemp) ;
	MatchingLevel	levelMostMatch = matchNo ;
	for ( ; ; )
	{
		DEVMODE	dmEnum ;
		if ( !::EnumDisplaySettings( pszDisplayName, iModeNum, &dmEnum ) )
		{
			break ;
		}
		iModeNum ++ ;
		//
		if ( (dmEnum.dmPelsWidth >= nWidth)
			&& (dmEnum.dmPelsHeight >= nHeight) )
		{
			if ( (levelMostMatch < matchAboveSize)
				|| ((levelMostMatch == matchAboveSize)
					&& (dmEnum.dmPelsWidth + dmEnum.dmPelsHeight
						< m_dmMonitor.dmPelsWidth + m_dmMonitor.dmPelsHeight))
				|| ((levelMostMatch == matchAboveSize)
					&& (dmEnum.dmPelsWidth == m_dmMonitor.dmPelsWidth)
					&& (dmEnum.dmPelsHeight == m_dmMonitor.dmPelsHeight)
					&& (dmEnum.dmBitsPerPel == nBitsPerPixel)) )
			{
				levelMostMatch = matchAboveSize ;
				m_dmMonitor = dmEnum ;
			}
			if ( (dmEnum.dmPelsWidth == nWidth)
				&& (dmEnum.dmPelsHeight == nHeight) )
			{
				if ( levelMostMatch < matchEqualSize )
				{
					levelMostMatch = matchEqualSize ;
					m_dmMonitor = dmEnum ;
				}
				if ( dmEnum.dmBitsPerPel >= nBitsPerPixel )
				{
					if ( levelMostMatch < matchSizeAndColor )
					{
						levelMostMatch = matchSizeAndColor ;
						m_dmMonitor = dmEnum ;
					}
					if ( dmEnum.dmDisplayFrequency == nFrequency )
					{
						levelMostMatch = matchBest ;
						m_dmMonitor = dmEnum ;
						break ;
					}
				}
			}
		}
	}
	m_dmMonitor.dmSize = sizeof(DEVMODE) ;
	m_dmMonitor.dmFields = DM_BITSPERPEL | DM_PELSWIDTH |
						DM_PELSHEIGHT | DM_DISPLAYFLAGS | DM_DISPLAYFREQUENCY ;
	return	levelMostMatch ;
}

// ディスプレイモードを切り替える
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDisplayMode::ChangeDisplayMode( const wchar_t * pwszDisplayName )
{
	SString			strDisplayName = pwszDisplayName ;
	SArray<char>	bufTemp ;
	const char *	pszDisplayName = strDisplayName.EncodeDefaultTo(bufTemp) ;
	LONG			nResult ;
	PrepareMonitorAPIs() ;
	if ( m_apiChangeDisplaySettingsEx && pszDisplayName )
	{
		nResult = m_apiChangeDisplaySettingsEx
			( pszDisplayName, &m_dmMonitor, NULL, CDS_FULLSCREEN, 0 ) ;
	}
	else
	{
		nResult = ::ChangeDisplaySettings( &m_dmMonitor, CDS_FULLSCREEN ) ;
	}
	if ( nResult != DISP_CHANGE_SUCCESSFUL )
	{
		return	sglErrFailed ;
	}
	m_flagChanged = true ;
	return	sglErrSuccess ;
}

// ディスプレイモードを元に戻す
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDisplayMode::RestoreDisplayMode( void )
{
	if ( !m_flagChanged )
	{
		return	sglErrSuccess ;
	}
	LONG	nResult = ::ChangeDisplaySettings( NULL, CDS_FULLSCREEN ) ;
	if ( nResult != DISP_CHANGE_SUCCESSFUL )
	{
		return	sglErrFailed ;
	}
	m_flagChanged = false ;
	return	sglErrSuccess ;
}

