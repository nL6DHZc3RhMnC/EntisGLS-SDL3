
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_window.h>
#include <glscs/glscs_sakura2_obj_window_menu.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// ウィンドウメニューオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::WindowMenuObject, Object, SGLWindowMenu )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
WindowMenuObject::WindowMenuObject( void )
{
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * WindowMenuObject::GetTypeName( void ) const
{
	return	L"SakuraGL::WindowMenu" ;
}

// メニュー項目変換
//////////////////////////////////////////////////////////////////////////////
SGLWindowMenu::Entry * WindowMenuObject::TranslateMenuEntries
	( Context * context,
		const WindowMenuObject::VMEntry * pEntries, ssize_t& nCount )
{
	m_bufMenuEntries.RemoveAll() ;
	m_bufStrings.RemoveAll() ;
	//
	SSystem::SArray<SGLWindowMenu::Entry> *
		pBuf = new SSystem::SArray<SGLWindowMenu::Entry> ;
	m_bufMenuEntries.Add( pBuf ) ;
	//
	return	TranslateSubMenuEntries( context, *pBuf, pEntries, nCount ) ;
}

SGLWindowMenu::Entry * WindowMenuObject::TranslateSubMenuEntries
	( Context * context,
		SSystem::SArray<SGLWindowMenu::Entry> & arrayBuf,
			const WindowMenuObject::VMEntry * pEntries, ssize_t& nCount )
{
	if ( pEntries == NULL )
	{
		nCount = 0 ;
		return	NULL ;
	}
	if ( nCount < 0 )
	{
		for ( nCount = 0; (pEntries[nCount].nFlags != 0)
						|| (pEntries[nCount].pwszText != 0)
						|| (pEntries[nCount].pwszID != 0)
						|| (pEntries[nCount].pSubMenu != 0); nCount ++ )
		{
		}
	}
	for ( size_t i = 0; i < (size_t) nCount; i ++ )
	{
		const VMEntry *	pEntry = pEntries + i ;
		arrayBuf.SetLength( i + 1 ) ;
		//
		Entry *		pItem = arrayBuf.GetAt( i ) ;
		SString *	pstrID = new SString ;
		SString *	pstrText = new SString ;
		m_bufStrings.Add( pstrID ) ;
		m_bufStrings.Add( pstrText ) ;
		//
		*pstrText =
			(const uint16_t*)
				context->AtomicTranslateAddress
					( pEntry->pwszText, sizeof(uint16_t) ) ;
		*pstrID =
			(const uint16_t*)
				context->AtomicTranslateAddress
					( pEntry->pwszID, sizeof(uint16_t) ) ;
		//
		pItem->nFlags = pEntry->nFlags ;
		pItem->pwszText = *pstrText ;
		pItem->pwszID = *pstrID ;
		//
		if ( pItem->nFlags & flagSubMenu )
		{
			SSystem::SArray<SGLWindowMenu::Entry> *
				pBuf = new SSystem::SArray<SGLWindowMenu::Entry> ;
			m_bufMenuEntries.Add( pBuf ) ;
			//
			const VMEntry *	pSubEntries =
				(const VMEntry*) 
					context->AtomicTranslateAddress
						( pEntry->pSubMenu, sizeof(VMEntry) ) ;
			pItem->nSubCount = (ssize_t) pEntry->nSubCount ;
			pItem->pSubMenu =
				TranslateSubMenuEntries
					( context, *pBuf, pSubEntries, pItem->nSubCount ) ;
		}
	}
	return	arrayBuf.GetArray() ;
}


#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::WindowMenu
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_WindowMenu, context, cls_id)
{
	return	new WindowMenuObject ;
}

// SGLError WindowMenu::CreateMenu
//	( const Entry * pMenuEntries, ssize_t nCount = -1 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_WindowMenu_CreateMenu, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowMenuObject, pMenu, arg, WindowMenu::CreateMenu ) ;
	const WindowMenuObject::VMEntry *	pVMEntries = 
		(const WindowMenuObject::VMEntry*)
			context->AtomicTranslateAddress
				( arg[1].i, sizeof(WindowMenuObject::VMEntry) ) ;
	//
	ssize_t	nCount = (ssize_t) arg[2].i ;
	WindowMenu::Entry *
		pEntries = pMenu->TranslateMenuEntries
						( context, pVMEntries, nCount ) ;
	context->m_regset[regAcc].i =
		pMenu->CreateMenu( pEntries, nCount ) ;
	//
	return	NULL ;
}

// SGLError WindowMenu::CreatePopupMenu
//	( const Entry * pMenuEntries, ssize_t nCount = -1 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_WindowMenu_CreatePopupMenu, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowMenuObject, pMenu, arg, WindowMenu::CreatePopupMenu ) ;
	const WindowMenuObject::VMEntry *	pVMEntries = 
		(const WindowMenuObject::VMEntry*)
			context->AtomicTranslateAddress
				( arg[1].i, sizeof(WindowMenuObject::VMEntry) ) ;
	//
	ssize_t	nCount = (ssize_t) arg[2].i ;
	WindowMenu::Entry *
		pEntries = pMenu->TranslateMenuEntries
						( context, pVMEntries, nCount ) ;
	context->m_regset[regAcc].i =
		pMenu->CreatePopupMenu( pEntries, nCount ) ;
	//
	return	NULL ;
}

// SGLError WindowMenu::DeleteMenu( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_WindowMenu_DeleteMenu, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowMenu, pMenu, arg, WindowMenu::DeleteMenu ) ;
	//
	context->m_regset[regAcc].i = pMenu->DeleteMenu() ;
	//
	return	NULL ;
}

// SGLError WindowMenu::ShowPopupMenu
//	( Window * pWindow, double x, double y, uint32_t nFlags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_WindowMenu_ShowPopupMenu, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowMenu, pMenu, arg, WindowMenu::ShowPopupMenu ) ;
	//
	context->m_regset[regAcc].i =
		pMenu->ShowPopupMenu
			( ESLTypeCast<SGLAbstractWindow>
					( vm->AtomicObjectFromAddress( arg[1].h32 ) ),
				arg[2].f, arg[3].f, (uint32_t) arg[4].i ) ;
	//
	return	NULL ;
}

// SGLError WindowMenu::EnableMenuItem
//	( const wchar_t * pwszID, bool fEnabled ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_WindowMenu_EnableMenuItem, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowMenu, pMenu, arg, WindowMenu::EnableMenuItem ) ;
	SString	strID
		( (const uint16_t*)
			context->AtomicTranslateAddress
					( arg[1].i, sizeof(uint16_t) ) ) ;
	//
	context->m_regset[regAcc].i =
		pMenu->EnableMenuItem( strID, (arg[2].i != 0) ) ;
	//
	return	NULL ;
}

// SGLError WindowMenu::CheckMenuItem
//	( const wchar_t * pwszID, bool fChecked ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_WindowMenu_CheckMenuItem, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowMenu, pMenu, arg, WindowMenu::CheckMenuItem ) ;
	SString	strID
		( (const uint16_t*)
			context->AtomicTranslateAddress
					( arg[1].i, sizeof(uint16_t) ) ) ;
	//
	context->m_regset[regAcc].i =
		pMenu->CheckMenuItem( strID, (arg[2].i != 0) ) ;
	//
	return	NULL ;
}

// SGLError WindowMenu::SetMenuItemText
//	( const wchar_t * pwszID, const wchar_t * pwszText ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_WindowMenu_SetMenuItemText, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, WindowMenu, pMenu, arg, WindowMenu::SetMenuItemText ) ;
	SString	strID
		( (const uint16_t*)
			context->AtomicTranslateAddress
					( arg[1].i, sizeof(uint16_t) ) ) ;
	SString	strText
		( (const uint16_t*)
			context->AtomicTranslateAddress
					( arg[2].i, sizeof(uint16_t) ) ) ;
	//
	context->m_regset[regAcc].i =
		pMenu->SetMenuItemText( strID, strText ) ;
	//
	return	NULL ;
}

#endif

