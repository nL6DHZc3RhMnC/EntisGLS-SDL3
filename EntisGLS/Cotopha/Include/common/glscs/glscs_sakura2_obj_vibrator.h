
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_VIBRATOR_H__)
#define	__GLSCS_SAKURA2_OBJECT_VIBRATOR_H__

#include <sakuraglx/sglx_platform_ui.h>

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// UI::Vibrator オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	VibratorObject	: public ECSSakura2::Object, public SakuraGL::UI::SGLVibrator
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( VibratorObject, ECSSakura2::Object, SGLVibrator )
		// 構築関数
		VibratorObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;

}


//////////////////////////////////////////////////////////////////////////////
// クリップボードスタブ
//////////////////////////////////////////////////////////////////////////////

// bool UI::Clipboard::HasPlaneText( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_Clipboard_HasPlaneText) ;

// bool UI::Clipboard::GetPlaneText( SString& strText ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_Clipboard_GetPlaneText) ;

// bool UI::Clipboard::SetPlaneText( const char * pszText ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_Clipboard_SetPlaneText) ;


//////////////////////////////////////////////////////////////////////////////
// Vibrator スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::UI::Vibrator
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_UI_Vibrator) ;

// void SetPattern
//	( const uint32_t* pPattern, size_t nCount,
//			bool fLoop = true, size_t iLoopStart = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_SetPattern) ;

// SGLError Start( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_Start) ;

// void Stop( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_Stop) ;

// bool IsInstalled( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_IsInstalled) ;


#endif
