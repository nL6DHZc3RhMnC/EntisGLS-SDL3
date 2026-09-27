
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_WINDOW_MENU_H__)
#define	__GLSCS_SAKURA2_OBJECT_WINDOW_MENU_H__

#include <sakuragl/window/sgl_window_menu.h>

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// ウィンドウメニューオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	WindowMenuObject	: public ECSSakura2::Object,
									public SakuraGL::SGLWindowMenu
	{
	public:
		struct	VMEntry
		{
			uint64_t	nFlags ;
			uint64_t	pwszText ;
			uint64_t	pwszID ;
			uint64_t	pSubMenu ;
			int32_t		nSubCount ;
			uint32_t	nReserved ;
		} ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( WindowMenuObject, ECSSakura2::Object, SGLWindowMenu )
		// 構築関数
		WindowMenuObject( void ) ;
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;

	protected:
		SSystem::SObjectArray
			< SSystem::SArray<SGLWindowMenu::Entry> >	m_bufMenuEntries ;
		SSystem::SObjectArray<SSystem::SString>			m_bufStrings ;

	public:
		// メニュー項目変換
		SGLWindowMenu::Entry * TranslateMenuEntries
			( Context * context,
				const VMEntry * pEntries, ssize_t& nCount ) ;
	protected:
		SGLWindowMenu::Entry * TranslateSubMenuEntries
			( Context * context,
				SSystem::SArray<SGLWindowMenu::Entry> & arrayBuf,
					const VMEntry * pEntries, ssize_t& nCount ) ;

	} ;

}

// new SakuraGL::WindowMenu
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_WindowMenu) ;

// SGLError WindowMenu::CreateMenu
//	( const Entry * pMenuEntries, ssize_t nCount = -1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_WindowMenu_CreateMenu) ;

// SGLError WindowMenu::CreatePopupMenu
//	( const Entry * pMenuEntries, ssize_t nCount = -1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_WindowMenu_CreatePopupMenu) ;

// SGLError WindowMenu::DeleteMenu( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_WindowMenu_DeleteMenu) ;

// SGLError WindowMenu::ShowPopupMenu
//	( Window * pWindow, double x, double y, uint32_t nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_WindowMenu_ShowPopupMenu) ;

// SGLError WindowMenu::EnableMenuItem
//	( const wchar_t * pwszID, bool fEnable ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_WindowMenu_EnableMenuItem) ;

// SGLError WindowMenu::CheckMenuItem
//	( const wchar_t * pwszID, bool fChecked ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_WindowMenu_CheckMenuItem) ;

// SGLError WindowMenu::SetMenuItemText
//	( const wchar_t * pwszID, const wchar_t * pwszText ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_WindowMenu_SetMenuItemText) ;


#endif

