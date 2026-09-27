
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/window/sgl_window_menu.h>

#if	defined(__PLATFORM_WINDOWS__)
	#if	!defined(MIIM_STRING)
		#define	MIIM_STRING	0x00000040
	#endif
	#if	!defined(MIIM_FTYPE)
		#define	MIIM_FTYPE	0x00000100
	#endif
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// メニュー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_ANDROID__)
ESL_IMPLEMENT_CLASS_INFO2( SakuraGL::SGLWindowMenu, SObject, JavaObject )
#else
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowMenu, SObject )
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowMenu::SGLWindowMenu( void )
{
#if	defined(__COTOPHA__)
	m_pMenu = NULL ;
#elif	defined(__PLATFORM_WINDOWS__)
	m_hMenu = NULL ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowMenu::~SGLWindowMenu( void )
{
	DeleteMenu() ;
}

// メニュー生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::CreateMenu
	( const WindowMenu::Entry * pMenuEntries, ssize_t nCount )
{
#if	defined(__COTOPHA__)
	delete	m_pMenu ;
	m_pMenu = new WindowMenu ;
	return	m_pMenu->CreateMenu( pMenuEntries, nCount ) ;
#else
	DeleteMenu() ;
	return	BuildMenuObject( pMenuEntries, nCount, false ) ;
#endif
}

// ポップアップメニュー生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::CreatePopupMenu
	( const WindowMenu::Entry * pMenuEntries, ssize_t nCount )
{
#if	defined(__COTOPHA__)
	delete	m_pMenu ;
	m_pMenu = new WindowMenu ;
	return	m_pMenu->CreatePopupMenu( pMenuEntries, nCount ) ;
#else
	DeleteMenu() ;
	return	BuildMenuObject( pMenuEntries, nCount, true ) ;
#endif
}

// メニュー削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::DeleteMenu( void )
{
#if	defined(__COTOPHA__)
	delete	m_pMenu ;
	m_pMenu = NULL ;
#else
	DeleteMenuObject() ;
#endif
	return	sglErrSuccess ;
}

// ポップアップメニュー表示
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::ShowPopupMenu
	( SGLAbstractWindow * pWindow,
			double x, double y, uint32_t nFlags )
{
	if ( pWindow == NULL )
	{
		return	sglErrFailed ;
	}
#if	defined(__COTOPHA__)
	if ( m_pMenu != NULL )
	{
		SGLWindow *	pSGLWnd = ESLTypeCast<SGLWindow>( pWindow ) ;
		Window *	pWndObj = NULL ;
		if ( pSGLWnd != NULL )
		{
			pWndObj = pSGLWnd->GetWindowObject() ;
		}
		return	m_pMenu->ShowPopupMenu( pWndObj, x, y, nFlags ) ;
	}

#elif	defined(__PLATFORM_WINDOWS__)
	if ( m_hMenu != NULL )
	{
		S2DDVector	vPosition( x, y ) ;
		pWindow->ScreenPositionFromClient( vPosition ) ;
		//
		UpdateMenuBarProcedure *	pProc =
			new UpdateMenuBarProcedure
				( m_hMenu,  (int) vPosition.x,
					(int) vPosition.y, pWindow->GetWindowHandle() ) ;
		pWindow->PostUIThread( pProc ) ;
	}

#elif	defined(__PLATFORM_ANDROID__)
	if ( GetObject() != NULL )
	{
		jmethodID	jmidShowPopupMenu =
			JavaObject::GetMethodID( "showPopupMenu", "()V" ) ;
		JavaObject::CallVoidMethod( jmidShowPopupMenu ) ;
		return	sglErrSuccess ;
	}

#endif
	return	sglErrFailed ;
}

#if	defined(__PLATFORM_WINDOWS__)
void SGLWindowMenu::UpdateMenuBarProcedure::Run( void )
{
	::TrackPopupMenu
		( m_hMenu, TPM_LEFTALIGN | TPM_TOPALIGN,
				m_xPos, m_yPos, 0, m_hWnd, NULL ) ;
}
void SGLWindowMenu::UpdateMenuBarProcedure::Finalize( void )
{
	delete	this ;
}
#endif

// メニューアイテム有効状態変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::EnableMenuItem
	( const wchar_t * pwszID, bool fEnabled )
{
#if	defined(__COTOPHA__)
	if ( m_pMenu != NULL )
	{
		return	m_pMenu->EnableMenuItem( pwszID, fEnabled ) ;
	}
#else
	MenuItem *	pItem = m_ssoaMenuItems.GetAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->nFlags &= ~flagDisabled ;
		if ( !fEnabled )
		{
			pItem->nFlags |= flagDisabled ;
		}
		#if	defined(__PLATFORM_WINDOWS__)
		if ( pItem->hParentMenu && pItem->idMenuCommand )
		{
			MENUITEMINFO	mii ;
			SArray<char>	bufText ;
			pItem->ToMenuItemInfo( mii, bufText ) ;
			::SetMenuItemInfo
				( pItem->hParentMenu, pItem->idMenuCommand, FALSE, &mii ) ;
			UpdateMenuBar() ;
			return	sglErrSuccess ;
		}
		#elif	defined(__PLATFORM_ANDROID__)
		if ( pItem->idMenuCommand )
		{
			jmethodID	jmidEnableMenuItem =
				JavaObject::GetMethodID( "enableMenuItem", "(IZ)Z" ) ;
			JavaObject::CallBooleanMethod
				( jmidEnableMenuItem, pItem->idMenuCommand, fEnabled ) ;
			return	sglErrSuccess ;
		}
		#endif
	}
#endif
	return	sglErrFailed ;
}

// メニューアイテムチェック状態変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::CheckMenuItem
	( const wchar_t * pwszID, bool fChecked )
{
#if	defined(__COTOPHA__)
	if ( m_pMenu != NULL )
	{
		return	m_pMenu->CheckMenuItem( pwszID, fChecked ) ;
	}
#else
	MenuItem *	pItem = m_ssoaMenuItems.GetAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->nFlags &= ~flagChecked ;
		if ( fChecked )
		{
			pItem->nFlags |= flagChecked ;
		}
		#if	defined(__PLATFORM_WINDOWS__)
		if ( pItem->hParentMenu && pItem->idMenuCommand )
		{
			MENUITEMINFO	mii ;
			SArray<char>	bufText ;
			pItem->ToMenuItemInfo( mii, bufText ) ;
			::SetMenuItemInfo
				( pItem->hParentMenu, pItem->idMenuCommand, FALSE, &mii ) ;
			UpdateMenuBar() ;
			return	sglErrSuccess ;
		}
		#elif	defined(__PLATFORM_ANDROID__)
		if ( pItem->idMenuCommand )
		{
			jmethodID	jmidCheckMenuItem =
				JavaObject::GetMethodID( "checkMenuItem", "(IZ)Z" ) ;
			JavaObject::CallBooleanMethod
				( jmidCheckMenuItem, pItem->idMenuCommand, fChecked ) ;
			return	sglErrSuccess ;
		}
		#endif
	}
#endif
	return	sglErrFailed ;
}

// メニューアイテム状態変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::SetMenuItemText
	( const wchar_t * pwszID, const wchar_t * pwszText )
{
#if	defined(__COTOPHA__)
	if ( m_pMenu != NULL )
	{
		return	m_pMenu->SetMenuItemText( pwszID, pwszText ) ;
	}
#else
	MenuItem *	pItem = m_ssoaMenuItems.GetAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->nFlags &= ~flagChecked ;
		if ( flagChecked )
		{
			pItem->nFlags |= flagChecked ;
		}
		#if	defined(__PLATFORM_WINDOWS__)
		if ( pItem->hParentMenu && pItem->idMenuCommand )
		{
			MENUITEMINFO	mii ;
			SArray<char>	bufText ;
			pItem->ToMenuItemInfo( mii, bufText ) ;
			::SetMenuItemInfo
				( pItem->hParentMenu, pItem->idMenuCommand, FALSE, &mii ) ;
			UpdateMenuBar() ;
			return	sglErrSuccess ;
		}
		#elif	defined(__PLATFORM_ANDROID__)
		if ( pItem->idMenuCommand )
		{
			jmethodID	jmidSetMenuItemText =
				JavaObject::GetMethodID
					( "setMenuItemText", "(IL" JAVA_LANG_STRING ";)Z" ) ;
			JNI::JavaObject	jobjText ;
			JavaObject::CallBooleanMethod
				( jmidSetMenuItemText,
					pItem->idMenuCommand,
					jobjText.CreateWideString( pwszText ) ) ;
			return	sglErrSuccess ;
		}
		#endif
	}
#endif
	return	sglErrFailed ;
}


#if	defined(__COTOPHA__)

// メニューオブジェクト取得
//////////////////////////////////////////////////////////////////////////////
WindowMenu * SGLWindowMenu::GetMenuObject( void ) const
{
	return	m_pMenu ;
}

#else

// メニュー情報構築
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowMenu::BuildMenuObject
	( const WindowMenu::Entry * pMenuEntries, ssize_t nCount, bool flagPopup )
{
	BuildMenuInfo( m_listRootMenu, pMenuEntries, nCount ) ;

#if	defined(__PLATFORM_WINDOWS__)
	m_hMenu = CreateMenuObject( m_listRootMenu, flagPopup ) ;

#elif	defined(__PLATFORM_ANDROID__)
	CreateMenuObject( m_listRootMenu ) ;

#endif

	return	sglErrSuccess ;
}

// メニュー情報構築
//////////////////////////////////////////////////////////////////////////////
void SGLWindowMenu::BuildMenuInfo
	( SSystem::SPointerArray<MenuItem>& listMenu,
			const Entry * pMenuEntries, ssize_t nCount )
{
	size_t	i ;
	if ( pMenuEntries == NULL )
	{
		return ;
	}
	if ( nCount < 0 )
	{
		for ( nCount = 0; (pMenuEntries[nCount].nFlags != 0)
						|| (pMenuEntries[nCount].pwszText != NULL)
						|| (pMenuEntries[nCount].pwszID != NULL)
						|| (pMenuEntries[nCount].pSubMenu != NULL); nCount ++ )
		{
		}
	}
	for ( i = 0; i < (size_t) nCount; i ++ )
	{
		const Entry *	pEntry = pMenuEntries + i ;
		MenuItem *		pItem = new MenuItem ;
		const wchar_t *	pwszID = pEntry->pwszID ;
		if ( pwszID == NULL )
		{
			pwszID = L"" ;
		}
		pItem->nFlags = pEntry->nFlags ;
		pItem->strID = pEntry->pwszID ;
		pItem->strText = pEntry->pwszText ;
		pItem->hSubMenu = NULL ;
		pItem->hParentMenu = NULL ;
		pItem->idMenuCommand = 0 ;
		//
		m_ssoaMenuItems.Add( pwszID, pItem ) ;
		listMenu.Add( pItem ) ;
		//
		if ( pItem->nFlags & flagSubMenu )
		{
			BuildMenuInfo
				( pItem->listSubMenu,
					pEntry->pSubMenu, pEntry->nSubCount ) ;
		}
	}
}

// メニューオブジェクト生成
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_WINDOWS__)

HMENU SGLWindowMenu::CreateMenuObject
	( SSystem::SPointerArray<MenuItem>& listMenu, bool flagPopup )
{
	HMENU	hMenu ;
	if ( flagPopup )
	{
		hMenu = ::CreatePopupMenu() ;
	}
	else
	{
		hMenu = ::CreateMenu() ;
	}
	const size_t	nCount = listMenu.GetLength() ;
	size_t	nNextPos = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MenuItem *	pItem = listMenu.GetAt( i ) ;
		ESLAssert( pItem != NULL ) ;
		if ( pItem != NULL )
		{
			ESLAssert( pItem->hParentMenu == NULL ) ;
			pItem->hParentMenu = hMenu ;
			if ( !(pItem->nFlags & flagSeparator) )
			{
				if ( pItem->nFlags & flagSubMenu )
				{
					pItem->hSubMenu =
						CreateMenuObject( pItem->listSubMenu, true ) ;
				}
				pItem->idMenuCommand = RegisterCommandID( pItem->strID ) ;
			}
			MENUITEMINFO	mii ;
			SArray<char>	bufText ;
			pItem->ToMenuItemInfo( mii, bufText ) ;
			::InsertMenuItem( hMenu, (UINT) (nNextPos ++), TRUE, &mii ) ;
		}
	}
	return	hMenu ;
}

void SGLWindowMenu::MenuItem::ToMenuItemInfo
	( MENUITEMINFO& mii, SSystem::SArray<char>& bufText ) const
{
	memset( &mii, 0, sizeof(MENUITEMINFO) ) ;
	mii.cbSize = sizeof(MENUITEMINFO) ;
	if ( nFlags & flagSeparator )
	{
		mii.fMask = MIIM_FTYPE ;
		mii.fType = MFT_SEPARATOR ;
	}
	else
	{
		mii.fMask = MIIM_STRING | MIIM_STATE ;
		mii.fState = 0 ;
		mii.dwTypeData = (char*) strText.EncodeDefaultTo( bufText ) ;
		if ( nFlags & flagRadioItem )
		{
			mii.fMask |= MIIM_FTYPE ;
			mii.fType |= MFT_RADIOCHECK ;
			mii.fState = (nFlags & flagChecked)
								? MFS_CHECKED : MFS_UNCHECKED ;
		}
		else if ( nFlags & flagCheckItem )
		{
			mii.fState = (nFlags & flagChecked)
								? MFS_CHECKED : MFS_UNCHECKED ;
		}
		if ( nFlags & flagDisabled )
		{
			mii.fState |= (MFS_DISABLED | MFS_GRAYED) ;
		}
		else
		{
			mii.fState |= MFS_ENABLED ;
		}
		if ( nFlags & flagSubMenu )
		{
			mii.fMask |= MIIM_SUBMENU ;
			mii.hSubMenu = hSubMenu ;
		}
		if ( idMenuCommand != 0 )
		{
			mii.fMask |= MIIM_ID ;
			mii.wID = (UINT) idMenuCommand ;
		}
	}
}

#elif	defined(__PLATFORM_ANDROID__)

void SGLWindowMenu::CreateMenuObject
	( SSystem::SPointerArray<MenuItem>& listMenu )
{
	JNI::JavaObject	jobjMenuList ;
	CreateJavaMenuInfo( jobjMenuList, listMenu ) ;
	//
	if ( JavaObject::GetObject() == NULL )
	{
		JavaObject::CreateJavaObject
			( ENTIS_GLS4_JAVA_PACKAGE "/MenuData" ) ;
	}
	jmethodID	jmidSetMenuInfo =
		JavaObject::GetMethodID
			( "setMenuInfo",
				"([L" ENTIS_GLS4_JAVA_PACKAGE "/MenuData$ItemInfo;)V" ) ;
	JavaObject::CallVoidMethod
		( jmidSetMenuInfo, jobjMenuList.GetObject() ) ;
}

void SGLWindowMenu::CreateJavaMenuInfo
	( JNI::JavaObject& jobjMenuList,
			SSystem::SPointerArray<MenuItem>& listMenu )
{
	JNI::JSmartClass	jclsMenuItemInfo
		( JNI::FindJavaClass
			( ENTIS_GLS4_JAVA_PACKAGE "/MenuData$ItemInfo" ) ) ;
	const size_t	nCount = listMenu.GetLength() ;
	jobjectArray	jobjArray =
		jobjMenuList.CreateObjectArray
				( nCount, jclsMenuItemInfo.GetObject() ) ;
	JNI::JObjectArray	jobjMenuArray( jobjArray ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MenuItem *	pItem = listMenu.GetAt( i ) ;
		ESLAssert( pItem != NULL ) ;
		if ( pItem != NULL )
		{
			JNI::JavaObject	jobjItemInfo ;
			jobjItemInfo.CreateJavaObject
				( ENTIS_GLS4_JAVA_PACKAGE "/MenuData$ItemInfo" ) ;
			jobjItemInfo.SetLongField
				( jobjItemInfo.GetLongFieldID("nFlags"),
									(jlong) pItem->nFlags ) ;
			//
			if ( !(pItem->nFlags & flagSeparator) )
			{
				JNI::JavaObject	jstrItemText ;
				jobjItemInfo.SetObjectField
					( jobjItemInfo.GetFieldID
						( "strText", "L" JAVA_LANG_STRING ";" ),
							jstrItemText.CreateWideString( pItem->strText ) ) ;
				//
				if ( pItem->nFlags & flagSubMenu )
				{
					JNI::JavaObject	jobjSubList ;
					CreateJavaMenuInfo( jobjSubList, pItem->listSubMenu ) ;
					//
					jobjItemInfo.SetObjectField
						( jobjItemInfo.GetFieldID
							( "listSubMenu",
									"[L" ENTIS_GLS4_JAVA_PACKAGE
											"/MenuData$ItemInfo;" ),
								jobjSubList.GetObject() ) ;
				}
				//
				pItem->idMenuCommand = RegisterCommandID( pItem->strID ) ;
				jobjItemInfo.SetIntField
					( jobjItemInfo.GetIntFieldID("nID"),
									(jint) pItem->idMenuCommand ) ;
			}
			//
			jobjMenuArray.SetAt( i, jobjItemInfo.GetObject() ) ;
		}
	}
}

#endif

// メニューオブジェクト削除
//////////////////////////////////////////////////////////////////////////////
void SGLWindowMenu::DeleteMenuObject( void )
{
	SGLAbstractWindow *	pWindow = m_refWindow ;
	if ( pWindow != NULL )
	{
		pWindow->AttachMenu( NULL ) ;
	}

	#if	defined(__PLATFORM_WINDOWS__)
	if ( m_hMenu != NULL )
	{
		::DestroyMenu( m_hMenu ) ;
		m_hMenu = NULL ;
	}
	#elif	defined(__PLATFORM_ANDROID__)
	if ( JavaObject::GetObject() != NULL )
	{
		JavaObject::DetachJavaObject() ;
	}
	#endif

	m_listRootMenu.RemoveAll() ;
	m_ssoaMenuItems.RemoveAll() ;
	m_refWindow = NULL ;
}

// メニュー表示更新
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_WINDOWS__)
void SGLWindowMenu::UpdateMenuBar( void )
{
	SGLAbstractWindow *	pWindow = m_refWindow ;
	if ( pWindow != NULL )
	{
		::DrawMenuBar( pWindow->GetWindowHandle() ) ;
	}
}
#endif

// ウィンドウ関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLWindowMenu::OnAttachReferenceWindow( SGLAbstractWindow * pWindow )
{
	m_refWindow = pWindow ;
}

// メニューハンドル取得
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_WINDOWS__)
HMENU SGLWindowMenu::GetMenuHandle( void ) const
{
	return	m_hMenu ;
}
#endif

// コマンドID (0x1000～0x2FFF)
//////////////////////////////////////////////////////////////////////////////
ESL_DLL_DECL( SSystem::SObjectArray<SSystem::SString>	SGLWindowMenu::m_mapCommandIDs ) ;

// コマンドID登録
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLWindowMenu::RegisterCommandID( const wchar_t * pwszID )
{
	uint32_t	nID = 0 ;
	if ( (pwszID != NULL) && (pwszID[0] != 0) )
	{
		QuickLock() ;
		const size_t	nCount = m_mapCommandIDs.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SString *	pstrID = m_mapCommandIDs.GetAt( i ) ;
			if ( pstrID != NULL )
			{
				if ( *pstrID == pwszID )
				{
					nID = (uint32_t) (0x1000 + i) ;
					break ;
				}
			}
		}
		if ( nID == 0 )
		{
			m_mapCommandIDs.Add( new SString(pwszID) ) ;
			nID = (uint32_t) (0x1000 + nCount) ;
		}
		QuickUnlock() ;
	}
	return	nID ;
}

// コマンドID名逆引き
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLWindowMenu::GetCommandIDOf( UINT nID )
{
	const wchar_t *	pwszID = NULL ;
	QuickLock() ;
	SString *	pstrID = m_mapCommandIDs.GetAt( nID - 0x1000 ) ;
	if ( pstrID != NULL )
	{
		pwszID = *pstrID ;
	}
	QuickUnlock() ;
	return	pwszID ;
}

// 終了時解放処理
//////////////////////////////////////////////////////////////////////////////
void SGLWindowMenu::FinalizeWindowMenu( void )
{
	m_mapCommandIDs.FreeArray() ;
}

#endif


