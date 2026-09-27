
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl2d_image.h>
#include <glscs/glscs_sakura2_obj_joy_stick.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;



//////////////////////////////////////////////////////////////////////////////
// JoyStick オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::JoyStickObject, ECSSakura2::Object, SGLJoyStick )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
JoyStickObject::JoyStickObject( void )
{
	m_dwWndCaptured = 0 ;
	m_maskCaptureDev = 0 ;
}

// キャプチャー開始
//////////////////////////////////////////////////////////////////////////////
SGLError JoyStickObject::BeginCapture( SGLWindow* pWnd, uint32_t maskCaptureDev )
{
	ECSSakura2::Object *	pObj = ESLTypeCast<ECSSakura2::Object>( pWnd ) ;
	if ( pObj != NULL )
	{
		m_dwWndCaptured = pObj->m_dwHighAddr ;
		m_maskCaptureDev = maskCaptureDev ;
	}
	else
	{
		m_dwWndCaptured = 0 ;
	}
	return	SGLJoyStick::BeginCapture( pWnd, maskCaptureDev ) ;
}

// キャプチャー終了
//////////////////////////////////////////////////////////////////////////////
SGLError JoyStickObject::ReleaseCapture( SGLWindow* pWnd )
{
	m_dwWndCaptured = 0 ;
	return	SGLJoyStick::ReleaseCapture( pWnd ) ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * JoyStickObject::GetTypeName( void ) const
{
	return	L"SakuraGL::UI::JoyStick" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError JoyStickObject::SaveStatic
	( SSystem::SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	file->Write( &m_dwWndCaptured, sizeof(DWORD) ) ;
	file->Write( &m_maskCaptureDev, sizeof(uint32_t) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError JoyStickObject::LoadStatic
	( SSystem::SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	if ( file->Read( &m_dwWndCaptured, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	errFailed ;
	}
	if ( file->Read( &m_maskCaptureDev, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SError JoyStickObject::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	if ( m_dwWndCaptured != 0 )
	{
		SGLWindow *	pWnd =
			ESLTypeCast<SGLWindow>
				( vm->AtomicObjectFromAddress( m_dwWndCaptured ) ) ;
		if ( pWnd != NULL )
		{
			return	(SError) BeginCapture( pWnd, m_maskCaptureDev ) ;
		}
	}
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// JoyStick スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::UI::JoyStick
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_UI_JoyStick,context,cls_id)
{
	return	new JoyStickObject ;
}

// SGLError BeginCapture( Window* pWnd, uint32_t maskDev ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_BeginCapture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, JoyStickObject, pJoy, arg, JoyStick::BeginCapture ) ;
	//
	context->m_regset[regAcc].i =
		pJoy->BeginCapture
			( ESLTypeCast<SGLWindow>
				( vm->AtomicObjectFromAddress( arg[1].h32 ) ), arg[2].l32 ) ;
	//
	return	NULL ;
}

// SGLError WaitReadyCapture( int64_t msecTimeout ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_WaitReadyCapture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, JoyStickObject, pJoy, arg, JoyStick::WaitReadyCapture ) ;
	//
	context->m_regset[regAcc].i = pJoy->WaitReadyCapture( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError ReleaseCapture( Window* pWnd ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_ReleaseCapture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, JoyStickObject, pJoy, arg, JoyStick::ReleaseCapture ) ;
	//
	context->m_regset[regAcc].i =
		pJoy->ReleaseCapture
			( ESLTypeCast<SGLWindow>
				( vm->AtomicObjectFromAddress( arg[1].h32 ) ) ) ;
	//
	return	NULL ;
}

// SGLError PollJoyStick
//	( SGLJoyStickState& joyState, size_t idJoyStick = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_PollJoyStick,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, JoyStickObject, pJoy, arg, JoyStick::PollJoyStick ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, UI::SGLJoyStickState, pJoyState,
				arg[1].i, joyState at JoyStick::PollJoyStick ) ;
	//
	context->m_regset[regAcc].i =
		pJoy->PollJoyStick( *pJoyState, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

#endif

