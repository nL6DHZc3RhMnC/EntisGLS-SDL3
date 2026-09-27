
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_JOY_STICK_H__)
#define	__GLSCS_SAKURA2_OBJECT_JOY_STICK_H__

#include <sakuraglx/ui/sglx_joy_stick.h>

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// JoyStick オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	JoyStickObject	: public ECSSakura2::Object, public SakuraGL::UI::SGLJoyStick
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( JoyStickObject, ECSSakura2::Object, SGLJoyStick )
		// 構築関数
		JoyStickObject( void ) ;

	protected:
		DWORD		m_dwWndCaptured ;
		uint32_t	m_maskCaptureDev ;

	public:
		// キャプチャー開始
		virtual SakuraGL::SGLError BeginCapture
			( SakuraGL::SGLWindow* pWnd, uint32_t maskCaptureDev ) ;
		// キャプチャー終了
		virtual SakuraGL::SGLError ReleaseCapture( SakuraGL::SGLWindow* pWnd ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SSystem::SError SaveStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SSystem::SError LoadStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後の後のスクリプト処理
		virtual SError OnLoadedDynamic
			( VirtualMachine * vm, Context * context ) ;
	} ;

}


//////////////////////////////////////////////////////////////////////////////
// JoyStick スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::UI::JoyStick
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_UI_JoyStick) ;

// SGLError BeginCapture( Window* pWnd ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_BeginCapture) ;

// SGLError WaitReadyCapture( int64_t msecTimeout ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_WaitReadyCapture) ;

// SGLError ReleaseCapture( Window* pWnd ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_ReleaseCapture) ;

// SGLError PollJoyStick
//	( SGLJoyStickState& joyState, size_t idJoyStick = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_JoyStick_PollJoyStick) ;


#endif
