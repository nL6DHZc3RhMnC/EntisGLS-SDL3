
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_STD_UI_H__)
#define	__GLSCS_SAKURA2_OBJECT_STD_UI_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// 進行状況ダイアログ
	//////////////////////////////////////////////////////////////////////////

	class	ProgressiveDialogObject
				: public ECSSakura2::Object, public SSystem::SProgressiveDialog
	{
	protected:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
				( ProgressiveDialogObject, ECSSakura2::Object, SProgressiveDialog )
		// 構築関数
		ProgressiveDialogObject( void ) ;
		// 消滅関数
		virtual ~ProgressiveDialogObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;

	} ;

}

//////////////////////////////////////////////////////////////////////////////
// OS/シェル UI 操作
//////////////////////////////////////////////////////////////////////////////

// SError SSystem::OpenShellFile
//	( const wchar_t * pwszURI,
//		ShellAction actShell = shellOpenURI,
//		const wchar_t * pwszAppPath = NULL,
//		const wchar_t * pwszAppPlacement = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_OpenShellFile) ;

// SError SSystem::ActivateWindow
//	( const wchar_t * pwszName = NULL,
//			const wchar_t * pwszClass = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_ActivateWindow) ;


//////////////////////////////////////////////////////////////////////////////
// ファイルブラウザダイアログ
//////////////////////////////////////////////////////////////////////////////

// int SSystem::BrowseDirectoryDialog
//	( SString& strDirPath,
//		const wchar_t * pwszCaption = NULL,
//		const wchar_t * pwszInitDir = NULL,
//		uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_BrowseDirectoryDialog) ;

// int SSystem::BrowseOpenFileDialog
//	( SString& strFilePath,
//		const wchar_t * pwszCaption = NULL,
//		const wchar_t * pwszInitDir = NULL,
//		const wchar_t ** ppwszFileFilters = NULL,
//		uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_BrowseOpenFileDialog) ;

// int SSystem::BrowseSaveFileDialog
//	( SString& strDirPath,
//		const wchar_t * pwszCaption = NULL,
//		const wchar_t * pwszInitDir = NULL,
//		uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_BrowseSaveFileDialog) ;


//////////////////////////////////////////////////////////////////////////////
// 入力テキストボックススタブ
//////////////////////////////////////////////////////////////////////////////

// const wchar_t * SSystem::MessageEditBox
//		( SArray<uint16_t>& strEditText,
//			const wchar_t * pszMsg = NULL,
//			const wchar_t * pszCaption = NULL,
//			int nStyles = 0, SakuraGL::Window * pParentWnd = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_MessageEditBox) ;


//////////////////////////////////////////////////////////////////////////////
// 進行状況ダイアログスタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::ProgressiveDialog
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_ProgressiveDialog) ;

// SError ProgressiveDialog::Create
//	( uint64_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_ProgressiveDialog_Create) ;

// SError ProgressiveDialog::Close( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_ProgressiveDialog_Close) ;

// SError ProgressiveDialog::SetCaption( const wchar_t * pwszCaption ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_ProgressiveDialog_SetCaption) ;

// SError ProgressiveDialog::SetMessage( const wchar_t * pwszMessage ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_ProgressiveDialog_SetMessage) ;

// SError ProgressiveDialog::SetStatus( uint32_t nCurrent, uint32_t nTotal ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_ProgressiveDialog_SetStatus) ;

// bool ProgressiveDialog::IsCanceled( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_ProgressiveDialog_IsCanceled) ;


#endif
