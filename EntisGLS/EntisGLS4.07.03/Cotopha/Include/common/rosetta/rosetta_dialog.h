
#if	!defined(__ROSETTA_DIALOG_H__)
#define	__ROSETTA_DIALOG_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// Window 型クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSWindowClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSWindowClass, RSClass )
		// 構築関数
		RSWindowClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Window" ) ;
		// 消滅関数
		virtual ~RSWindowClass( void ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// オブジェクトを取得
		static SakuraGL::SGLAbstractWindow *
				GetWindow( RSContext& context, RSObject* pObj ) ;

	protected:	// Window method
		// RenderDevice getRenderDevice()
		static RSObject * method_getRenderDevice
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Dialog オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSDialogWindow	: public SakuraGL::SGLWindow,
								public SSystem::SCustomDialog::Listener
	{
	public:
		SSystem::SCustomDialog	m_dialog ;
		RSContext				m_context ;
		RSObject *				m_pObject ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( RSDialogWindow, SGLWindow, Listener )
		// 構築関数
		RSDialogWindow( RSVirtualMachine * pVM, RSObject * pThread ) ;
		// 消滅関数
		virtual ~RSDialogWindow( void ) ;

	public:
		#if	defined(__PLATFORM_WINDOWS__)
		// プラットフォーム固有オブジェクト
		virtual HWND GetWindowHandle( void ) const ;
		#endif

	public:
		// 初期化処理
		virtual void OnInitDialog( SSystem::SCustomDialog& dlg ) ;
		// キャンセル処理
		virtual bool OnCancel( SSystem::SCustomDialog& dlg ) ;
		// コールバック関数
		static bool ItemCallback
			( SSystem::SCustomDialog& dlg,
				const SSystem::SCustomDialog::ElementInfo& item,
				int code, void * instance ) ;
		virtual bool OnItemCallback
			( const SSystem::SCustomDialog::ElementInfo& item, int code ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Dialog 型クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSDialogClass	: public RSClass
	{
	public:
		// Dialog.ElementInfo クラス
		class	ElementInfoClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ElementInfoClass, RSClass )
			// 構築関数
			ElementInfoClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"ElementInfo" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		public:
			// SCustomDialog::ElementInfo -> Object 変換
			static void ToObject
				( RSContext& context, RSObject * pObj,
					const SSystem::SCustomDialog::ElementInfo& elInfo ) ;
			// SCustomDialog::ElementData <- Object 変換
			static void FromObject
				( RSContext& context,
					SSystem::SCustomDialog::ElementData& elData, RSObject * pObj ) ;
		public:
			// void <init>()
			static RSObject * method_init1
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// void <init>
			//	( String id, int type, int nFlags = 0,
			//		int optFlags = 0, int nMinWidth = 0,
			//		String strText = null, int nValue = 0,
			//		int minRange = 0, int maxRange = 0 )
			static RSObject * method_init2
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSDialogClass, RSClass )
		// 構築関数
		RSDialogClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Dialog" ) ;
		// 消滅関数
		virtual ~RSDialogClass( void ) ;
		// メンバ初期設定
		void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// オブジェクトを取得
		static RSDialogWindow *
				GetThisDialog( RSContext& context, RSObject* pObj ) ;

	protected:	// Dialog method
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setCustomItems( Dialog.ElementInfo[] elements )
		static RSObject * method_setCustomItems
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setCaption( String strCaption )
		static RSObject * method_setCaption
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int doModal( long nFlags = 0, Window wndParent = null )
		static RSObject * method_doModal
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getInputIntegerAs( String id )
		static RSObject * method_getInputIntegerAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const String getInputStringAs( String id )
		static RSObject * method_getInputStringAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setItemIntegerAs( String id, int nValue )
		static RSObject * method_setItemIntegerAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setItemStringAs( String id, String strText )
		static RSObject * method_setItemStringAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void enableItemAs( String id, boolean fEnable )
		static RSObject * method_enableItemAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void endDialog( int nResultCode )
		static RSObject * method_endDialog
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void onInitDialog()
		static RSObject * method_onInitDialog
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean onCancel()
		static RSObject * method_onCancel
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean onItemCallback( Dialog.ElementInfo item, int code )
		static RSObject * method_onItemCallback
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int messageBox
		//	( String msg, String caption = null,
		//		int styles = msgboxStyleOk, Window wndParent = null )
		static RSObject * method_messageBox
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int browseDirectory
		//	( String strDirPath, String strCaption = null,
		//		String strInitDir = null,
		//		int nFlags = 0, Window wndParent = null ) ;
		static RSObject * method_browseDirectory
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int browseOpenFile
		//	( String strFilePath, String strCaption = null,
		//		String strInitDir = null,
		//		String[] strFileFilters = null,
		//		int nFlags = 0, Window wndParent = null ) ;
		static RSObject * method_browseOpenFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int browseSaveFile
		//	( String strFilePath, String strCaption = null,
		//		String strInitDir = null,
		//		String[] strFileFilters = null,
		//		int nFlags = 0, Window wndParent = null ) ;
		static RSObject * method_browseSaveFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;

}

#endif

