
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_window.h>
#include <glscs/glscs_sakura2_obj_std_ui.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// OS/シェルでファイルを開く
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// SError SSystem::OpenShellFile
//	( const wchar_t * pwszURI,
//		ShellAction actShell = shellOpenURI,
//		const wchar_t * pwszAppPath = NULL,
//		const wchar_t * pwszAppPlacement = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_OpenShellFile,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	uint16_t *	pszURI =
		(uint16_t*) context->AtomicTranslateAddress
								( arg[0].i, sizeof(uint16_t) ) ;
	uint16_t *	pszAppPath =
		(uint16_t*) context->AtomicTranslateAddress
								( arg[2].i, sizeof(uint16_t) ) ;
	uint16_t *	pszAppPlacement =
		(uint16_t*) context->AtomicTranslateAddress
								( arg[3].i, sizeof(uint16_t) ) ;
	//
	SSystem::SString	strURI = pszURI ;
	SSystem::SString	strAppPath = pszAppPath ;
	SSystem::SString	strAppPlacement = pszAppPlacement ;
	//
	context->m_regset[regAcc].i =
		OpenShellFile
			( strURI, (SSystem::ShellAction) arg[1].i,
								strAppPath, strAppPlacement ) ;
	//
	return	NULL ;
}

// SError SSystem::ActivateWindow
//	( const wchar_t * pwszName = NULL,
//			const wchar_t * pwszClass = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_ActivateWindow,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	//
	uint16_t *	pszName =
		(uint16_t*) context->AtomicTranslateAddress
								( arg[0].i, sizeof(uint16_t) ) ;
	uint16_t *	pszClass =
		(uint16_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(uint16_t) ) ;
	//
	SSystem::SString	strName = pszName ;
	SSystem::SString	strClass = pszClass ;
	//
	context->m_regset[regAcc].i = ActivateWindow( strName, strClass ) ;
	//
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// ファイルブラウザダイアログ
//////////////////////////////////////////////////////////////////////////////

// int SSystem::BrowseDirectoryDialog
//	( SString& strDirPath,
//		const wchar_t * pwszCaption = NULL,
//		const wchar_t * pwszInitDir = NULL,
//		uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_BrowseDirectoryDialog,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pstrDirPath,
				arg[0].i, strInitEdit at SSystem::BrowseDirectoryDialog ) ;
	//
	uint16_t *	pszCaption =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(uint16_t) ) ;
	uint16_t *	pszInitDir =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[2].i, sizeof(uint16_t) ) ;
	SGLAbstractWindow *
		pParentWnd = ESLTypeCast<SGLAbstractWindow>
						( vm->AtomicObjectFromAddress( arg[4].h32 ) ) ;
	//
	SString	strDirPath ;
	SString	strCaption = pszCaption ;
	SString	strInitDir = pszInitDir ;
	//
	int	nResult =
		SSystem::BrowseDirectoryDialog
			( strDirPath, strCaption, strInitDir,
						(uint32_t) arg[3].i, pParentWnd ) ;
	//
	context->m_regset[regAcc].i = nResult ;
	if ( nResult == msgboxResultOk )
	{
		size_t		lenResult = strDirPath.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pstrDirPath->AllocateArray
						( lenResult + 1, sizeof(uint16_t), vm ) ;
		//
		const uint16_t *	pszResult = strDirPath ;
		for ( size_t i = 0; i < lenResult; i ++ )
		{
			pStrArray[i] = pszResult[i] ;
		}
		pStrArray[lenResult] = 0 ;
		//
		pstrDirPath->m_nLength = (DWORD) lenResult ;
	}
	//
	return	NULL ;
}

// int SSystem::BrowseOpenFileDialog
//	( SString& strFilePath,
//		const wchar_t * pwszCaption = NULL,
//		const wchar_t * pwszInitDir = NULL,
//		const wchar_t ** ppwszFileFilters = NULL,
//		uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_BrowseOpenFileDialog,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pstrFilePath,
				arg[0].i, strInitEdit at SSystem::BrowseOpenFileDialog ) ;
	//
	uint16_t *	pszCaption =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(uint16_t) ) ;
	uint16_t *	pszInitDir =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[2].i, sizeof(uint16_t) ) ;
	uint64_t *	ppszFileFilters =
			(uint64_t*) context->AtomicTranslateAddress
								( arg[3].i, sizeof(uint64_t) ) ;
	SGLAbstractWindow *
		pParentWnd = ESLTypeCast<SGLAbstractWindow>
						( vm->AtomicObjectFromAddress( arg[5].h32 ) ) ;
	//
	SString					strFilePath ;
	SString					strCaption = pszCaption ;
	SString					strInitDir = pszInitDir ;
	SObjectArray<SString>	aFileFilters ;
	SArray<const wchar_t*>	aPtrFilters ;
	//
	if ( ppszFileFilters != NULL )
	{
		for ( int i = 0; ppszFileFilters[i] != 0; i ++ )
		{
			SString *	pstrFilter =
				new SString
					( (uint16_t*) context->AtomicTranslateAddress
							( ppszFileFilters[i + 1], sizeof(uint16_t) ) ) ;
			aFileFilters.Add( pstrFilter ) ;
			aPtrFilters.Add( (const wchar_t*) *pstrFilter ) ;
		}
		aPtrFilters.Add( NULL ) ;
		aPtrFilters.Add( NULL ) ;
	}
	//
	int	nResult =
		SSystem::BrowseOpenFileDialog
			( strFilePath, strCaption, strInitDir,
				aPtrFilters.GetArray(),
				(uint32_t) arg[4].i, pParentWnd ) ;
	aPtrFilters.FinishArray() ;
	//
	context->m_regset[regAcc].i = nResult ;
	if ( nResult == msgboxResultOk )
	{
		size_t		lenResult = strFilePath.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pstrFilePath->AllocateArray
						( lenResult + 1, sizeof(uint16_t), vm ) ;
		//
		const uint16_t *	pszResult = strFilePath ;
		for ( size_t i = 0; i < lenResult; i ++ )
		{
			pStrArray[i] = pszResult[i] ;
		}
		pStrArray[lenResult] = 0 ;
		//
		pstrFilePath->m_nLength = (DWORD) lenResult ;
	}
	//
	return	NULL ;
}

// int SSystem::BrowseSaveFileDialog
//	( SString& strDirPath,
//		const wchar_t * pwszCaption = NULL,
//		const wchar_t * pwszInitDir = NULL,
//		uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_BrowseSaveFileDialog,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pstrFilePath,
				arg[0].i, strInitEdit at SSystem::BrowseSaveFileDialog ) ;
	//
	uint16_t *	pszCaption =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(uint16_t) ) ;
	uint16_t *	pszInitDir =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[2].i, sizeof(uint16_t) ) ;
	uint64_t *	ppszFileFilters =
			(uint64_t*) context->AtomicTranslateAddress
								( arg[3].i, sizeof(uint64_t) ) ;
	SGLAbstractWindow *
		pParentWnd = ESLTypeCast<SGLAbstractWindow>
						( vm->AtomicObjectFromAddress( arg[5].h32 ) ) ;
	//
	SString					strFilePath ;
	SString					strCaption = pszCaption ;
	SString					strInitDir = pszInitDir ;
	SObjectArray<SString>	aFileFilters ;
	SArray<const wchar_t*>	aPtrFilters ;
	//
	if ( ppszFileFilters != NULL )
	{
		for ( int i = 0; ppszFileFilters[i] != 0; i ++ )
		{
			SString *	pstrFilter =
				new SString
					( (uint16_t*) context->AtomicTranslateAddress
							( ppszFileFilters[i + 1], sizeof(uint16_t) ) ) ;
			aFileFilters.Add( pstrFilter ) ;
			aPtrFilters.Add( (const wchar_t*) *pstrFilter ) ;
		}
		aPtrFilters.Add( NULL ) ;
		aPtrFilters.Add( NULL ) ;
	}
	//
	int	nResult =
		SSystem::BrowseSaveFileDialog
			( strFilePath, strCaption, strInitDir,
				aPtrFilters.GetArray(),
				(uint32_t) arg[4].i, pParentWnd ) ;
	aPtrFilters.FinishArray() ;
	//
	context->m_regset[regAcc].i = nResult ;
	if ( nResult == msgboxResultOk )
	{
		size_t		lenResult = strFilePath.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pstrFilePath->AllocateArray
						( lenResult + 1, sizeof(uint16_t), vm ) ;
		//
		const uint16_t *	pszResult = strFilePath ;
		for ( size_t i = 0; i < lenResult; i ++ )
		{
			pStrArray[i] = pszResult[i] ;
		}
		pStrArray[lenResult] = 0 ;
		//
		pstrFilePath->m_nLength = (DWORD) lenResult ;
	}
	//
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// 入力テキストボックススタブ
//////////////////////////////////////////////////////////////////////////////

// const wchar_t * SSystem::MessageEditBox
//		( SArray<uint16_t>& strEditText,
//			const wchar_t * pszMsg = NULL,
//			const wchar_t * pszCaption = NULL,
//			int nStyles = 0, SakuraGL::Window * pParentWnd = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_MessageEditBox,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pstrEditText,
				arg[0].i, strInitEdit at SSystem::MessageEditBox ) ;
	//
	uint16_t *	pszInitEdit =
			(uint16_t*) context->AtomicTranslateAddress
					( pstrEditText->m_ptrArray, sizeof(uint16_t) ) ;
	uint16_t *	pszMsg =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(uint16_t) ) ;
	uint16_t *	pszCaption =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[2].i, sizeof(uint16_t) ) ;
	SakuraGL::SGLAbstractWindow *
		pParentWnd = ESLTypeCast<SakuraGL::SGLAbstractWindow>
						( vm->AtomicObjectFromAddress( arg[4].h32 ) ) ;
	//
	SSystem::SString	strEditText = pszInitEdit ;
	SSystem::SString	strMsg = pszMsg ;
	SSystem::SString	strCaption = pszCaption ;
	//
	int	nResult =
		SSystem::MessageEditBox
			( strEditText, strMsg, strCaption, (int) arg[3].i, pParentWnd ) ;
	//
	context->m_regset[regAcc].i = nResult ;
	if ( nResult == msgboxResultOk )
	{
		size_t		lenResult = strEditText.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pstrEditText->AllocateArray
						( lenResult + 1, sizeof(uint16_t), vm ) ;
		//
		const uint16_t *	pszResult = strEditText ;
		for ( size_t i = 0; i < lenResult; i ++ )
		{
			pStrArray[i] = pszResult[i] ;
		}
		pStrArray[lenResult] = 0 ;
		//
		pstrEditText->m_nLength = (DWORD) lenResult ;
	}
	//
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// 進行状況ダイアログ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( ECSSakura2::ProgressiveDialogObject, Object, SProgressiveDialog )

	// 構築関数
//////////////////////////////////////////////////////////////////////////////
ProgressiveDialogObject::ProgressiveDialogObject( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ProgressiveDialogObject::~ProgressiveDialogObject( void )
{
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ProgressiveDialogObject::GetTypeName( void ) const
{
	return	L"SSystem::ProgressiveDialog" ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::ProgressiveDialog
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SSystem_ProgressiveDialog,context,cls_id)
{
	return	new ProgressiveDialogObject ;
}

// SError ProgressiveDialog::Create
//	( uint64_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_ProgressiveDialog_Create,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SProgressiveDialog, pdlg,
					arg, ProgressiveDialog::Create ) ;
	SakuraGL::SGLAbstractWindow *
		pParentWnd = ESLTypeCast<SakuraGL::SGLAbstractWindow>
						( vm->AtomicObjectFromAddress( arg[2].h32 ) ) ;
	context->m_regset[regAcc].i = pdlg->Create( arg[1].i, pParentWnd ) ;
	return	NULL ;
}

// SError ProgressiveDialog::Close( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_ProgressiveDialog_Close,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SProgressiveDialog, pdlg,
					arg, ProgressiveDialog::Close ) ;
	context->m_regset[regAcc].i = pdlg->Close() ;
	return	NULL ;
}

// SError ProgressiveDialog::SetCaption( const wchar_t * pwszCaption ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_ProgressiveDialog_SetCaption,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SProgressiveDialog, pdlg,
					arg, ProgressiveDialog::SetCaption ) ;
	uint16_t *	pszCaption =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(uint16_t) ) ;
	SString	strCaption = pszCaption ;
	context->m_regset[regAcc].i = pdlg->SetCaption( strCaption ) ;
	return	NULL ;
}

// SError ProgressiveDialog::SetMessage( const wchar_t * pwszMessage ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_ProgressiveDialog_SetMessage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SProgressiveDialog, pdlg,
					arg, ProgressiveDialog::SetMessage ) ;
	uint16_t *	pszMessage =
			(uint16_t*) context->AtomicTranslateAddress
								( arg[1].i, sizeof(uint16_t) ) ;
	SString	strMessage = pszMessage ;
	context->m_regset[regAcc].i = pdlg->SetMessage( strMessage ) ;
	return	NULL ;
}

// SError ProgressiveDialog::SetStatus( uint32_t nCurrent, uint32_t nTotal ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_ProgressiveDialog_SetStatus,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SProgressiveDialog, pdlg,
					arg, ProgressiveDialog::SetStatus ) ;
	context->m_regset[regAcc].i =
			pdlg->SetStatus( arg[1].l32, arg[2].l32 ) ;
	return	NULL ;
}

// bool ProgressiveDialog::IsCanceled( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_ProgressiveDialog_IsCanceled,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SProgressiveDialog, pdlg,
					arg, ProgressiveDialog::IsCanceled ) ;
	context->m_regset[regAcc].i = pdlg->IsCanceled() ? -1 : 0 ;
	return	NULL ;
}

#endif
