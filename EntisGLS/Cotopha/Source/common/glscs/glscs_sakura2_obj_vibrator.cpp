
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_obj_vibrator.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// UI::Vibrator オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::VibratorObject, Object, SGLVibrator )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
VibratorObject::VibratorObject( void )
{
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * VibratorObject::GetTypeName( void ) const
{
	return	L"SakuraGL::UI::Vibrator" ;
}


//////////////////////////////////////////////////////////////////////////////
// クリップボードスタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// bool UI::Clipboard::HasPlaneText( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_Clipboard_HasPlaneText, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	context->m_regset[regAcc].i = UI::Clipboard::HasPlaneText() ? -1 : 0 ;
	return	NULL ;
}

// bool UI::Clipboard::GetPlaneText( SString& strText ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_Clipboard_GetPlaneText, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pText,
				arg[0].i, strText at UI::Clipboard::GetPlaneText ) ;
	//
	SString	strText ;
	if ( UI::Clipboard::GetPlaneText( strText ) )
	{
		size_t	nCount = strText.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pText->AllocateArray
					( nCount + 1, sizeof(uint16_t), vm ) ;
		if ( pStrArray != NULL )
		{
			const uint16_t *	pszValue = strText.GetConstArray() ;
			pText->m_nLength = (DWORD) nCount ;
			for ( size_t i = 0; i <= nCount; i ++ )
			{
				pStrArray[i] = pszValue[i] ;
			}
		}
		context->m_regset[regAcc].i = -1 ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// bool UI::Clipboard::SetPlaneText( const char * pszText ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_Clipboard_SetPlaneText, context, arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszText,
				arg[1].i, pszText at UI::Clipboard::SetPlaneText ) ;
	//
	SString	strText = pszText ;
	if ( UI::Clipboard::SetPlaneText( strText ) )
	{
		context->m_regset[regAcc].i = -1 ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// Vibrator スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::UI::Vibrator
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_UI_Vibrator,context,cls_id)
{
	return	new VibratorObject ;
}

// void SetPattern
//	( const uint32_t* pPattern, size_t nCount,
//			bool fLoop = true, size_t iLoopStart = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_SetPattern,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, UI::SGLVibrator, pVib, arg, Vibrator::SetPattern ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, const uint32_t, pPattern,
			arg[1].i, arg[2].i, Vibrator::SetPattern ) ;
	//
	pVib->SetPattern
		( pPattern, (size_t) arg[2].i,
			(arg[3].i != 0), (size_t) arg[4].i ) ;
	//
	return	NULL ;
}

// SGLError Start( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_Start,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, UI::SGLVibrator, pVib, arg, Vibrator::SetPattern ) ;
	//
	context->m_regset[regAcc].i = pVib->Start() ;
	//
	return	NULL ;
}

// void Stop( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_Stop,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, UI::SGLVibrator, pVib, arg, Vibrator::SetPattern ) ;
	//
	pVib->Stop() ;
	//
	return	NULL ;
}

// bool IsInstalled( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_UI_Vibrator_IsInstalled,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, UI::SGLVibrator, pVib, arg, Vibrator::SetPattern ) ;
	//
	context->m_regset[regAcc].i = pVib->IsInstalled() ? -1 : 0 ;
	//
	return	NULL ;
}

#endif

