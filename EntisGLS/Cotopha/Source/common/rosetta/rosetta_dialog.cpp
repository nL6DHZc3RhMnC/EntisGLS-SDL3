
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl_window.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_string.h>
#include <rosetta/rosetta_dialog.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// Window 型クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSWindowClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSWindowClass::RSWindowClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSWindowClass::~RSWindowClass( void )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSWindowClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRenderDevice", L"RenderDevice", L"",
			NULL, &RSWindowClass::method_getRenderDevice,
			NULL, RSFunctionPrototype::flagConstant ) ;
}

// オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SGLAbstractWindow *
	RSWindowClass::GetWindow( RSContext& context, RSObject* pObj )
{
	SGLAbstractWindow *	pWindow = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pWindow = ESLTypeCast<SGLAbstractWindow>( pNativeObj->GetObject() ) ;
	}
	return	pWindow ;
}

// RenderDevice getRenderDevice()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowClass::method_getRenderDevice
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAbstractWindow *	pWindow = GetWindow( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pWindow, context.GetClassAs( L"RenderDevice" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Dialog オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( Rosetta::RSDialogWindow, SGLWindow, Listener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDialogWindow::RSDialogWindow( RSVirtualMachine * pVM, RSObject * pThread )
	: m_context( pVM, pThread )
{
	m_pObject = NULL ;
	//
	m_dialog.AttachListener( this ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSDialogWindow::~RSDialogWindow( void )
{
}

// プラットフォーム固有オブジェクト
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_WINDOWS__)
HWND RSDialogWindow::GetWindowHandle( void ) const
{
	return	m_dialog.GetWindowHandle() ;
}
#endif

// 初期化処理
//////////////////////////////////////////////////////////////////////////////
void RSDialogWindow::OnInitDialog( SSystem::SCustomDialog& dlg )
{
	if ( m_pObject && m_pObject->GetRSClass() )
	{
		RSObject *	pResult =
			m_context.CallMethod
				( m_pObject, L"onInitDialog", NULL, 0 ) ;
		m_context.ReleaseObjectRef( pResult ) ;
	}
}

// キャンセル処理
//////////////////////////////////////////////////////////////////////////////
bool RSDialogWindow::OnCancel( SCustomDialog& dlg )
{
	bool	fResult = false ;
	if ( m_pObject && m_pObject->GetRSClass() )
	{
		RSObject *	pResult =
			m_context.CallMethod
				( m_pObject, L"onCancel", NULL, 0 ) ;
		if ( pResult && pResult->AsBoolean() )
		{
			fResult = true ;
		}
		m_context.ReleaseObjectRef( pResult ) ;
	}
	return	false ;
}

// コールバック関数
//////////////////////////////////////////////////////////////////////////////
bool RSDialogWindow::ItemCallback
	( SCustomDialog& dlg,
		const SCustomDialog::ElementInfo& item, int code, void * instance )
{
	RSDialogWindow *	pDialog = (RSDialogWindow*) instance ;
	return	pDialog->OnItemCallback( item, code ) ;
}

bool RSDialogWindow::OnItemCallback
	( const SCustomDialog::ElementInfo& item, int code )
{
	bool	fResult = false ;
	if ( m_pObject && m_pObject->GetRSClass() )
	{
		RSObject *	pObjElInfo = m_context.new_Object( L"Dialog.ElementInfo" ) ;
		if ( pObjElInfo != NULL )
		{
			RSDialogClass::ElementInfoClass::ToObject
							( m_context, pObjElInfo, item ) ;
			//
			RSObject *	pArgs[2] =
			{
				pObjElInfo, m_context.new_Integer( code )
			} ;
			RSObject *	pResult =
				m_context.CallMethod
					( m_pObject, L"onItemCallback", pArgs, 2 ) ;
			if ( pResult && pResult->AsBoolean() )
			{
				fResult = true ;
			}
			m_context.ReleaseObjectRef( pResult ) ;
			m_context.ReleaseObjectRef( pObjElInfo ) ;
			m_context.ReleaseObjectRef( pArgs[1] ) ;
		}
	}
	return	fResult ;
}


//////////////////////////////////////////////////////////////////////////////
// Dialog.ElementInfo クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDialogClass::ElementInfoClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDialogClass::ElementInfoClass::ElementInfoClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSDialogClass::ElementInfoClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberStringAs( context, L"strID", L"" ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"type", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nFlags", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"optFlags", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nMinWidth", 0 ) ;
	m_pPrototype->CreateMemberStringAs( context, L"strText", L"" ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nValue", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"minRange", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"maxRange", 0 ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSDialogClass::ElementInfoClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL,
			L"String id, int type, int nFlags = 0, "
			L"int optFlags = 0, int nMinWidth = 0, "
			L"String strText = null, int nValue = 0, "
			L"int minRange = 0, int maxRange = 0",
			NULL, &RSDialogClass::ElementInfoClass::method_init2, NULL ) ;
}

// SCustomDialog::ElementInfo -> Object 変換
//////////////////////////////////////////////////////////////////////////////
void RSDialogClass::ElementInfoClass::ToObject
	( RSContext& context, RSObject * pObj,
		const SCustomDialog::ElementInfo& elInfo )
{
	context.SetObjMemberStringAs( pObj, L"strID", elInfo.pwszID ) ;
	context.SetObjMemberIntegerAs( pObj, L"type", elInfo.type ) ;
	context.SetObjMemberIntegerAs( pObj, L"nFlags", elInfo.nFlags ) ;
	context.SetObjMemberIntegerAs( pObj, L"optFlags", elInfo.optFlags ) ;
	context.SetObjMemberIntegerAs( pObj, L"nMinWidth", elInfo.nMinWidth ) ;
	context.SetObjMemberStringAs( pObj, L"strText", elInfo.pwszText ) ;
	context.SetObjMemberIntegerAs( pObj, L"nValue", elInfo.nValue ) ;
	context.SetObjMemberIntegerAs( pObj, L"minRange", elInfo.minRange ) ;
	context.SetObjMemberIntegerAs( pObj, L"maxRange", elInfo.maxRange ) ;
}

// SCustomDialog::ElementInfo <- Object 変換
//////////////////////////////////////////////////////////////////////////////
void RSDialogClass::ElementInfoClass::FromObject
	( RSContext& context,
		SCustomDialog::ElementData& elData, RSObject * pObj )
{
	elData.m_strID = context.GetObjMemberStringAs( pObj, L"strID" ) ;
	elData.m_info.pwszID = elData.m_strID ;
	elData.m_info.type =
		(SCustomDialog::ElementType)
			context.GetObjMemberIntegerAs( pObj, L"type" ) ;
	elData.m_info.nFlags =
		(uint32_t) context.GetObjMemberIntegerAs( pObj, L"nFlags" ) ;
	elData.m_info.optFlags =
		(uint32_t) context.GetObjMemberIntegerAs( pObj, L"optFlags" ) ;
	elData.m_info.nMinWidth =
		(uint32_t) context.GetObjMemberIntegerAs( pObj, L"nMinWidth" ) ;
	elData.m_strText = context.GetObjMemberStringAs( pObj, L"strText" ) ;
	elData.m_info.pwszText = elData.m_strText ;
	elData.m_info.nValue =
		(int) context.GetObjMemberIntegerAs( pObj, L"nValue" ) ;
	elData.m_info.minRange =
		(int) context.GetObjMemberIntegerAs( pObj, L"minRange" ) ;
	elData.m_info.maxRange =
		(int) context.GetObjMemberIntegerAs( pObj, L"maxRange" ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::ElementInfoClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	NULL ;
}

// void <init>
//	( String id, int type, int nFlags = 0,
//		int optFlags = 0, int nMinWidth = 0,
//		String strText = null, int nValue = 0,
//		int minRange = 0, int maxRange = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::ElementInfoClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	context.SetObjMemberStringAs( pThis, L"strID", arg.StringAt(0) ) ;
	context.SetObjMemberIntegerAs( pThis, L"type", arg.IntAt(1) ) ;
	context.SetObjMemberIntegerAs( pThis, L"nFlags", arg.IntAt(2) ) ;
	context.SetObjMemberIntegerAs( pThis, L"optFlags", arg.IntAt(3) ) ;
	context.SetObjMemberIntegerAs( pThis, L"nMinWidth", arg.IntAt(4) ) ;
	context.SetObjMemberStringAs( pThis, L"strText", arg.StringAt(5) ) ;
	context.SetObjMemberIntegerAs( pThis, L"nValue", arg.IntAt(6) ) ;
	context.SetObjMemberIntegerAs( pThis, L"minRange", arg.IntAt(7) ) ;
	context.SetObjMemberIntegerAs( pThis, L"maxRange", arg.IntAt(8) ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// Dialog 型クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDialogClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDialogClass::RSDialogClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSDialogClass::~RSDialogClass( void )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSDialogClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Window" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSDialogClass::OverrideVirtuals( RSContext& context )
{
	ElementInfoClass *
		pElementInfoClass =
			new ElementInfoClass( context.GetVM()->GetClassClass() ) ;
	pElementInfoClass->Initialize( context ) ;
	pElementInfoClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"ElementInfo", pElementInfoClass ) ) ;
	//
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"msgboxStyleOk", msgboxStyleOk, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxStyleOkCancel", msgboxStyleOkCancel, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxStyleYesNo", msgboxStyleYesNo, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxStyleYesNoCancel", msgboxStyleYesNoCancel, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxStyleRetryCancel", msgboxStyleRetryCancel, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxStyleAbortRetryIgnore", msgboxStyleAbortRetryIgnore, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"msgboxResultOk", msgboxResultOk, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxResultCancel", msgboxResultCancel, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxResultYes", msgboxResultYes, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxResultNo", msgboxResultNo, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxResultRetry", msgboxResultRetry, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxResultAbort", msgboxResultAbort, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxResultIgnore", msgboxResultIgnore, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"msgboxResultUser", msgboxResultUser, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"editboxStyleMultiLine", editboxStyleMultiLine, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"editboxStyleNumber", editboxStyleNumber, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"editboxStylePassword", editboxStylePassword, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"itemNull", SCustomDialog::itemNull, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemText", SCustomDialog::itemText, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemEdit", SCustomDialog::itemEdit, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemButton", SCustomDialog::itemButton, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemCheck", SCustomDialog::itemCheck, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemRadio", SCustomDialog::itemRadio, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemGroupBox", SCustomDialog::itemGroupBox, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemProgress", SCustomDialog::itemProgress, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemScroll", SCustomDialog::itemScroll, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"itemDropDownList", SCustomDialog::itemDropDownList, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"flagEndOfLine", SCustomDialog::flagEndOfLine, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagEndOfRadio", SCustomDialog::flagEndOfRadio, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagEndOfGroupBox", SCustomDialog::flagEndOfGroupBox, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagLineCenter", SCustomDialog::flagLineCenter, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagLineRight", SCustomDialog::flagLineRight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagFullWidth", SCustomDialog::flagFullWidth, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagMaskTypeButton", SCustomDialog::flagMaskTypeButton, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagPositiveButton", SCustomDialog::flagPositiveButton, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagNegativeButton", SCustomDialog::flagNegativeButton, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagNeutralButton", SCustomDialog::flagNeutralButton, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagMinWidth", SCustomDialog::flagMinWidth, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagArrangeCol1", SCustomDialog::flagArrangeCol1, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagArrangeCol2", SCustomDialog::flagArrangeCol2, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagArrangeCol3", SCustomDialog::flagArrangeCol3, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagArrangeCol4", SCustomDialog::flagArrangeCol4, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSDialogClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCustomItems", NULL,
			L"Dialog.ElementInfo[] elements",
			NULL, &RSDialogClass::method_setCustomItems, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCaption", L"boolean", L"String strCaption",
			NULL, &RSDialogClass::method_setCaption, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"doModal", L"int",
			L"long nFlags = 0, Window wndParent = null",
			NULL, &RSDialogClass::method_doModal, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInputIntegerAs",
			L"int", L"String id",
			NULL, &RSDialogClass::method_getInputIntegerAs,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInputStringAs",
			L"String", L"String id",
			NULL, &RSDialogClass::method_getInputStringAs,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setItemIntegerAs", NULL,
			L"String id, int nValue",
			NULL, &RSDialogClass::method_setItemIntegerAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setItemStringAs", NULL,
			L"String id, String strText",
			NULL, &RSDialogClass::method_setItemStringAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableItemAs", NULL,
			L"String id, boolean fEnable",
			NULL, &RSDialogClass::method_enableItemAs, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"endDialog", NULL, L"int nResultCode",
			NULL, &RSDialogClass::method_endDialog, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onInitDialog", NULL, L"",
			NULL, &RSDialogClass::method_onInitDialog, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onCancel", L"boolean", L"",
			NULL, &RSDialogClass::method_onCancel, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onItemCallback", L"boolean",
			L"Dialog.ElementInfo item, int code",
			NULL, &RSDialogClass::method_onItemCallback, NULL ) ;
	//
	AddFunctionDescriptiveAs
		( context, perr, L"messageBox", L"int",
			L"String msg, String caption = null, "
			L"int styles = Dialog.msgboxStyleOk, Window wndParent = null",
			NULL, &RSDialogClass::method_messageBox, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"browseDirectory", L"int",
			L"String strDirPath, String strCaption = null, "
			L"String strInitDir = null, "
			L"int nFlags = 0, Window wndParent = null",
			NULL, &RSDialogClass::method_browseDirectory, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"browseOpenFile", L"int",
			L"String strFilePath, String strCaption = null, "
			L"String strInitDir = null, "
			L"String[] strFileFilters = null, "
			L"int nFlags = 0, Window wndParent = null",
			NULL, &RSDialogClass::method_browseOpenFile, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"browseSaveFile", L"int",
			L"String strFilePath, String strCaption = null, "
			L"String strInitDir = null, "
			L"String[] strFileFilters = null, "
			L"int nFlags = 0, Window wndParent = null",
			NULL, &RSDialogClass::method_browseSaveFile, NULL ) ;
}

// オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
RSDialogWindow *
	RSDialogClass::GetThisDialog( RSContext& context, RSObject* pObj )
{
	RSDialogWindow *	pDialog = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		pDialog = ESLTypeCast<RSDialogWindow>( pNativeObj->GetObject() ) ;
	}
	if ( pDialog == NULL )
	{
		context.ThrowExceptionError( L"this が Dialog ではありません" ) ;
	}
	return	pDialog ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Dialog.<init> の this が Dialog ではありません" ) ;
		return	NULL ;
	}
	RSDialogWindow *	pDlg = new RSDialogWindow( context.GetVM(), NULL ) ;
	pDlg->m_pObject = pNativeObj ;
	pNativeObj->SetObject( (SGLAbstractWindow*) pDlg ) ;
	return	NULL ;
}

// void setCustomItems( Dialog.ElementInfo[] elements )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_setCustomItems
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjElements = arg.ObjectAt( 0 ) ;
	if ( pObjElements == NULL )
	{
		return	NULL ;
	}
	SArray<SCustomDialog::ElementInfo>			aElements ;
	SObjectArray<SCustomDialog::ElementData>	aElDatas ;
	SCustomDialog::ElementInfo					elInfoDummy ;
	eslFillMemory( &elInfoDummy, 0, sizeof(SCustomDialog::ElementInfo) ) ;
	//
	size_t	nCount = pObjElements->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSObject *	pObj = pObjElements->GetElementAt( context, (int) i ) ;
		SCustomDialog::ElementData *
				pElData = new SCustomDialog::ElementData( elInfoDummy ) ;
		ElementInfoClass::FromObject( context, *pElData, pObj ) ;
		context.ReleaseObjectRef( pObj ) ;
		//
		pElData->m_info.pfnCallback = &RSDialogWindow::ItemCallback ;
		pElData->m_info.pInstance = pDlg ;
		//
		aElDatas.Add( pElData ) ;
		aElements.Add( pElData->m_info ) ;
	}
	pDlg->m_dialog.SetCustomItems
			( aElements.GetConstArray(), aElements.GetLength() ) ;
	return	NULL ;
}

// boolean setCaption( String strCaption )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_setCaption
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SError	err = pDlg->m_dialog.SetCaption( arg.StringAt( 0 ) ) ;
	return	context.new_Boolean( err == errSuccess ) ;
}

// int doModal( long nFlags = 0, Window wndParent = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_doModal
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAbstractWindow *	pWindow =
		RSWindowClass::GetWindow( context, arg.ObjectAt( 1 ) ) ;
	//
	int	nResult = pDlg->m_dialog.DoModal( arg.LongAt(0), pWindow ) ;
	//
	return	context.new_Integer( nResult ) ;
}

// const int getInputIntegerAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_getInputIntegerAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	int	nValue = pDlg->m_dialog.GetInputIntegerAs( arg.StringAt(0) ) ;
	return	context.new_Integer( nValue ) ;
}

// const String getInputStringAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_getInputStringAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
				( pDlg->m_dialog.GetInputStringAs( arg.StringAt(0) ) ) ;
}

// void setItemIntegerAs( String id, int nValue )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_setItemIntegerAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDlg->m_dialog.SetItemIntegerAs( arg.StringAt(0), arg.IntAt(1) ) ;
	return	NULL ;
}

// void setItemStringAs( String id, String strText )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_setItemStringAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDlg->m_dialog.SetItemStringAs( arg.StringAt(0), arg.StringAt(1) ) ;
	return	NULL ;
}

// void enableItemAs( String id, boolean fEnable )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_enableItemAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDlg->m_dialog.EnableItemAs( arg.StringAt(0), arg.BooleanAt(1) ) ;
	return	NULL ;
}

// void endDialog( int nResultCode )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_endDialog
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDialogWindow *	pDlg = GetThisDialog( context, pThis ) ;
	if ( pDlg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDlg->m_dialog.EndDialog( arg.IntAt(0) ) ;
	return	NULL ;
}

// void onInitDialog()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_onInitDialog
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	NULL ;
}

// boolean onCancel()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_onCancel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// boolean onItemCallback( Dialog.ElementInfo item, int code )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_onItemCallback
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// static int messageBox
//	( String msg, String caption = null,
//		int styles = msgboxStyleOk, Window wndParent = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_messageBox
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAbstractWindow *	pWindow =
		RSWindowClass::GetWindow( context, arg.ObjectAt( 3 ) ) ;
	//
	int	nResult =
		SSystem::MessageBox
			( arg.StringAt(0), arg.StringAt(1), arg.IntAt(2), pWindow ) ;
	//
	return	context.new_Integer( nResult ) ;
}

// static int browseDirectory
//	( String strDirPath, String strCaption = null,
//		String strInitDir = null,
//		int nFlags = 0, Window wndParent = null ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_browseDirectory
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAbstractWindow *	pWindow =
		RSWindowClass::GetWindow( context, arg.ObjectAt( 4 ) ) ;
	//
	SString	strDirPath ;
	int	nResult =
		BrowseDirectoryDialog
			( strDirPath, arg.StringAt(1),
				arg.StringAt(2), arg.IntAt(3), pWindow ) ;
	//
	RSString *	pstrResult = ESLTypeCast<RSString>( arg.ObjectAt( 0 ) ) ;
	if ( pstrResult != NULL )
	{
		pstrResult->m_strValue = strDirPath ;
	}
	return	context.new_Integer( nResult ) ;
}

// static int browseOpenFile
//	( String strFilePath, String strCaption = null,
//		String strInitDir = null,
//		String[] strFileFilters = null,
//		int nFlags = 0, Window wndParent = null ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_browseOpenFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAbstractWindow *	pWindow =
		RSWindowClass::GetWindow( context, arg.ObjectAt( 5 ) ) ;
	//
	SObjectArray<SString>			lstFilters ;
	SPointerArray<const wchar_t>	lstPtrFilters ;
	const wchar_t **				ppwszFileFilters = NULL ;
	RSObject *						pObjFilters = arg.ObjectAt( 3 ) ;
	if ( pObjFilters != NULL )
	{
		size_t	nCount = pObjFilters->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			RSObject *	pObj = pObjFilters->GetElementAt( context, (int) i ) ;
			SString *	pstrValue = new SString ;
			if ( pObj && pObj->AsString( *pstrValue ) )
			{
				lstPtrFilters.Add( (const wchar_t*) *pstrValue ) ;
			}
			context.ReleaseObjectRef( pObj ) ;
			lstFilters.Add( pstrValue ) ;
		}
		lstPtrFilters.Add( NULL ) ;
		lstPtrFilters.Add( NULL ) ;
		ppwszFileFilters = lstPtrFilters.GetArray() ;
	}
	//
	SString	strFilePath ;
	int	nResult =
		BrowseOpenFileDialog
			( strFilePath, arg.StringAt(1),
				arg.StringAt(2), ppwszFileFilters, arg.IntAt(4), pWindow ) ;
	//
	lstPtrFilters.FinishArray() ;
	//
	RSString *	pstrResult = ESLTypeCast<RSString>( arg.ObjectAt( 0 ) ) ;
	if ( pstrResult != NULL )
	{
		pstrResult->m_strValue = strFilePath ;
	}
	return	context.new_Integer( nResult ) ;
}

// static int browseSaveFile
//	( String strFilePath, String strCaption = null,
//		String strInitDir = null,
//		String[] strFileFilters = null,
//		int nFlags = 0, Window wndParent = null ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDialogClass::method_browseSaveFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAbstractWindow *	pWindow =
		RSWindowClass::GetWindow( context, arg.ObjectAt( 5 ) ) ;
	//
	SObjectArray<SString>			lstFilters ;
	SPointerArray<const wchar_t>	lstPtrFilters ;
	const wchar_t **				ppwszFileFilters = NULL ;
	RSObject *						pObjFilters = arg.ObjectAt( 3 ) ;
	if ( pObjFilters != NULL )
	{
		size_t	nCount = pObjFilters->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			RSObject *	pObj = pObjFilters->GetElementAt( context, (int) i ) ;
			SString *	pstrValue = new SString ;
			if ( pObj && pObj->AsString( *pstrValue ) )
			{
				lstPtrFilters.Add( (const wchar_t*) *pstrValue ) ;
			}
			context.ReleaseObjectRef( pObj ) ;
			lstFilters.Add( pstrValue ) ;
		}
		lstPtrFilters.Add( NULL ) ;
		lstPtrFilters.Add( NULL ) ;
		ppwszFileFilters = lstPtrFilters.GetArray() ;
	}
	//
	SString	strFilePath ;
	int	nResult =
		BrowseSaveFileDialog
			( strFilePath, arg.StringAt(1),
				arg.StringAt(2), ppwszFileFilters, arg.IntAt(4), pWindow ) ;
	//
	lstPtrFilters.FinishArray() ;
	//
	RSString *	pstrResult = ESLTypeCast<RSString>( arg.ObjectAt( 0 ) ) ;
	if ( pstrResult != NULL )
	{
		pstrResult->m_strValue = strFilePath ;
	}
	return	context.new_Integer( nResult ) ;
}

