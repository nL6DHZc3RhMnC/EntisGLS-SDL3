
#if	!defined(__SAKURA2_STD_UI_H__)
#define	__SAKURA2_STD_UI_H__

#if	defined(__PLATFORM_ANDROID__)
#include <esl/esl_java_object.h>
#endif

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// 日本語以外への対応用
	//////////////////////////////////////////////////////////////////////////

	enum	LanguageType
	{
		languageInvalid	= -1,
		languageJapanese,		// デフォルト
		languageEnglish,		// 英語
		languageTypeCount,		// 対応言語数
	} ;

	extern ESL_DLL_EXPORT LanguageType	g_languageTarget ;
	extern const wchar_t *		g_pwszLanguageSignatures[languageTypeCount] ;

	// シグネチャから言語タイプ取得
	LanguageType GetLanguageTypeBySignature( const wchar_t * pwszLangSig ) ;

	// 複合テキストからターゲット言語テキストを選択
	// format: '\x1b' '[' <lang-sig> ']' <text> '\0' ... '\0' '\0'
	// '\x1b' から始まる 0 終端テキストは言語指定ありとして判別
	// '\x1b' から始まらない文字列はそのまま返す
	const wchar_t * MultiLanguageComplexText( const wchar_t * pwszMultiLangText ) ;

	inline const wchar_t * _TX( const wchar_t * pwszComplexText )
	{
		return	MultiLanguageComplexText( pwszComplexText ) ;
	}


	//////////////////////////////////////////////////////////////////////////
	// OS/シェル UI 操作
	//////////////////////////////////////////////////////////////////////////

	// OS/シェルでファイルを開く
	enum	ShellAction
	{
		shellOpenURI,
	} ;
	enum	ShellActionFlag
	{
		shellOpenSync	= 0x0001,
		shellExeSync	= 0x0002,
		shellNoConsole	= 0x0010,
	} ;
	enum	ShellResultFlag
	{
		shellResultSuccess	= 0x0001,		// 起動は成功した
		shellResultExitCode	= 0x0002,		// nExitCode 取得成功
	} ;
	struct	ShellOpenResult
	{
		uint32_t	nFlags ;
		uint32_t	nExitCode ;
	} ;
	__native SError OpenShellFile
		( const wchar_t * pwszURI,
			ShellAction actShell = shellOpenURI,
			const wchar_t * pwszAppPath = NULL,
			const wchar_t * pwszAppPlacement = NULL,
			uint32_t nFlags = 0, ShellOpenResult * pResult = NULL ) ;

	// 特定のウィンドウをフォアグラウンドにする
	__native SError ActivateWindow
		( const wchar_t * pwszName = NULL,
			const wchar_t * pwszClass = NULL ) ;


	//////////////////////////////////////////////////////////////////////////
	// ファイルブラウザダイアログ
	//////////////////////////////////////////////////////////////////////////

	// ディレクトリを選択
	__native int BrowseDirectoryDialog
		( SString& strDirPath,
			const wchar_t * pwszCaption = NULL,
			const wchar_t * pwszInitDir = NULL,
			uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;

	// ファイルを選択（既存ファイル）
	__native int BrowseOpenFileDialog
		( SString& strFilePath,
			const wchar_t * pwszCaption = NULL,
			const wchar_t * pwszInitDir = NULL,
			const wchar_t ** ppwszFileFilters = NULL,
			uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;

	// ファイルを選択（既存／新規ファイル）
	__native int BrowseSaveFileDialog
		( SString& strFilePath,
			const wchar_t * pwszCaption = NULL,
			const wchar_t * pwszInitDir = NULL,
			const wchar_t ** ppwszFileFilters = NULL,
			uint32_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;

	#if	defined(__PLATFORM_WINDOWS__)
	// ディレクトリを選択
	int DoBrowseDirectoryDialog
		( SString& strDirPath,
			const wchar_t * pwszCaption = NULL,
			const wchar_t * pwszInitDir = NULL,
			uint32_t nFlags = 0, HWND hParentWnd = NULL ) ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 入力テキストボックス
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__PLATFORM_WINDOWS__)
	class	SMessageEditBoxDialog : public ESLObject, public SProcedure
	{
	protected:
		HWND			m_hDialog ;
		HWND			m_hParentWnd ;
		SThread			m_threadUI ;
		SSignalEvent	m_signalCreated ;
		SString			m_strCaption ;
		SString			m_strMessage ;
		SString			m_strEdit ;
		int				m_nStyles ;
		int				m_nResult ;

		enum	ControlIDs
		{
			IDD_EDITBOX_DIALOG		= 130,
			IDC_STATIC_MESSAGE		= 1004,
			IDC_EDIT_TEXT			= 1005,
		} ;
		static const BYTE	m_bytEditBoxDlgData[232] ;

		// ダイアログ関数
		static INT_PTR CALLBACK EditBoxDialogProc
			( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
		// ダイアログアイテムを移動する
		static void OffsetDialogItems
			( HWND hwndDlg, const UINT * pItemIDs,
					size_t nCount, int xOffset, int yOffset ) ;
		// ダイアログアイテム座標補正値を取得
		static void OffsetDialogClientPos( HWND hwndDlg, POINT& ptOffset ) ;

		// スレッド関数
		virtual void Run( void ) ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SMessageEditBoxDialog, ESLObject )
		// 構築関数
		SMessageEditBoxDialog( void ) ;
		// 消滅関数
		virtual ~SMessageEditBoxDialog( void ) ;
		// 実行
		SError DoModal
			( const wchar_t * pwszInitEdit,
				const wchar_t * pwszMsg = NULL,
				const wchar_t * pwszCaption = NULL,
				int nStyles = 0, SakuraGL::Window * pParentWnd = NULL ) ;
		// 編集文字列取得
		const SString& GetEditString( void ) const ;
		// 結果取得
		int GetResult( void ) const ;
	} ;
	#endif

	enum	MessageEditBoxStyle
	{
		editboxStyleMultiLine	= 0x00000001,
		editboxStyleNumber		= 0x00000002,
		editboxStylePassword	= 0x00000004,
	} ;
	__native int MessageEditBox
		( SString& strEditText,
			const wchar_t * pwszMsg = NULL,
			const wchar_t * pwszCaption = NULL,
			int nStyles = 0, SakuraGL::Window * pParentWnd = NULL ) ;


	//////////////////////////////////////////////////////////////////////////
	// 進行状況ダイアログ
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	__native ProgressiveDialog
	{
	public:
		// フラグ
		enum	Flags
		{
			flagStyleSpinner	= 0x01,
		} ;
		// ダイアログ作成
		__native SError Create
			( uint64_t nFlags = 0, SakuraGL::Window * pParentWnd = NULL ) ;
		// ダイアログ消去
		__native SError Close( void ) ;
		// キャプション設定
		__native SError SetCaption( const wchar_t * pwszCaption ) ;
		// メッセージ設定
		__native SError SetMessage( const wchar_t * pwszMessage ) ;
		// 進捗状況設定
		__native SError SetStatus( uint32_t nCurrent, uint32_t nTotal ) ;
		// キャンセルが押されたか？
		__native bool IsCanceled( void ) ;
	} ;
	#endif

	class	SProgressiveUserInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SProgressiveUserInterface, ESLObject )

	public:
		// 進行状況ダイアログ表示
		virtual void CreateProgressiveDialog( void ) = 0 ;
		// 進行状況ダイアログ消去
		virtual void CloseProgressiveDialog( void ) = 0 ;
		// 進行状況ダイアログキャプション設定
		virtual void SetProgressiveCaption( const wchar_t * pwszCaption ) = 0 ;
		// 進行状況ダイアログメッセージ設定
		virtual void SetProgressiveMessage( const wchar_t * pwszMessage ) = 0 ;
		// 進行状況ダイアログメッセージ設定
		virtual void SetProgressiveStatus( int nCurrent, int nTotal ) = 0 ;
		// ユーザーがキャンセル操作したか？
		virtual bool IsProgressiveCanceled( void ) = 0 ;
		// メッセージボックス表示
		virtual int DoMessageBox
			( const wchar_t * pwszMsg,
				const wchar_t * pwszCaption = NULL,
						int nStyles = msgboxStyleOk ) = 0 ;
	} ;

	class	SProgressiveDialog : public SProgressiveUserInterface
	#if	defined(__PLATFORM_WINDOWS__)
								, public SProcedure
	#endif
	{
	protected:
	#if	defined(__COTOPHA__)
		ProgressiveDialog *	m_dlg ;

	#elif	defined(__PLATFORM_WINDOWS__)
		SThread			m_threadUI ;
		HWND			m_hDialog ;
		HWND			m_hParentWnd ;
		bool			m_flagCanceled ;
		SSignalEvent	m_signalCreated ;
		SString			m_strCaption ;
		SString			m_strMessage ;

		enum	ControlIDs
		{
			IDD_PROGRESSIVE_DIALOG	= 130,
			IDC_STATIC_MESSAGE		= 1000,
			IDC_PROGRESS			= 1001,
		} ;
		static const BYTE	m_bytProgressiveDlgData[184] ;

		// ダイアログ関数
		static INT_PTR CALLBACK ProgressiveDialogProc
			( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;

		// スレッド関数
		virtual void Run( void ) ;

	#elif	defined(__PLATFORM_ANDROID__)
		SString				m_strCaption ;
		SString				m_strMessage ;
		JNI::JavaObject		m_jobjDialog ;
		jmethodID			m_jmidSetTitle ;
		jmethodID			m_jmidSetMessage ;
		jmethodID			m_jmidCloseDialog ;
		jmethodID			m_jmidIsCanceled ;
		jmethodID			m_jmidSetProgress ;

	#endif

		uint64_t						m_nCreationFlags ;
		SakuraGL::SGLAbstractWindow *	m_pParentWnd ;
		

	public:
		// フラグ
		enum	Flags
		{
			flagStyleSpinner	= 0x01,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SProgressiveDialog, SProgressiveUserInterface )
		// 構築関数
		SProgressiveDialog( void ) ;
		// 消滅関数
		virtual ~SProgressiveDialog( void ) ;
		// ダイアログ作成
		SError Create
			( uint64_t nFlags = 0,
				SakuraGL::SGLAbstractWindow * pParentWnd = NULL ) ;
		// ダイアログ消去
		SError Close( void ) ;
		// キャプション設定
		SError SetCaption( const wchar_t * pwszCaption ) ;
		// メッセージ設定
		SError SetMessage( const wchar_t * pwszMessage ) ;
		// 進捗状況設定
		SError SetStatus( uint32_t nCurrent, uint32_t nTotal ) ;
		// キャンセルが押されたか？
		bool IsCanceled( void ) ;
		// CreateProgressiveDialog での生成パラメータ
		void SetCreationParam
			( uint64_t nFlags = 0,
				SakuraGL::SGLAbstractWindow * pParentWnd = NULL ) ;

	public:	// SProgressiveUserInterface 実装
		// 進行状況ダイアログ表示
		virtual void CreateProgressiveDialog( void ) ;
		// 進行状況ダイアログ消去
		virtual void CloseProgressiveDialog( void ) ;
		// 進行状況ダイアログキャプション設定
		virtual void SetProgressiveCaption( const wchar_t * pwszCaption ) ;
		// 進行状況ダイアログメッセージ設定
		virtual void SetProgressiveMessage( const wchar_t * pwszMessage ) ;
		// 進行状況ダイアログメッセージ設定
		virtual void SetProgressiveStatus( int nCurrent, int nTotal ) ;
		// ユーザーがキャンセル操作したか？
		virtual bool IsProgressiveCanceled( void ) ;
		// メッセージボックス表示
		virtual int DoMessageBox
			( const wchar_t * pwszMsg,
				const wchar_t * pwszCaption = NULL,
						int nStyles = msgboxStyleOk ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 任意ダイアログ
	//////////////////////////////////////////////////////////////////////////

	#if	!defined(__COTOPHA__)
	class	SCustomDialog	: public ESLObject
	#if	defined(__PLATFORM_WINDOWS__)
								, public SProcedure
	#endif
	{
	public:
		// 要素
		enum	ElementType
		{
			itemNull,
			itemText,
			itemEdit,
			itemButton,
			itemCheck,
			itemRadio,
			itemGroupBox,
			itemProgress,
			itemScroll,
			itemDropDownList,	// 複数文字列から選択 / pwszText に '\n' で連結された文字列
		} ;
		enum	ElementFlag
		{
			flagEndOfLine		= 0x0001,
			flagEndOfRadio		= 0x0002,
			flagEndOfGroupBox	= 0x0004,
			flagLineCenter		= 0x0008,		// 行の中で中央揃え
			flagLineRight		= 0x0010,		// 行の中で右揃え
			flagFullWidth		= 0x0020,		// 右端を全体幅まで伸ばす
			flagMaskTypeButton	= 0x0300,
			flagPositiveButton	= 0x0100,
			flagNegativeButton	= 0x0200,
			flagNeutralButton	= 0x0300,
			flagMinWidth		= 0x0400,
			flagArrangeCol1		= 0x1000,
			flagArrangeCol2		= 0x2000,
			flagArrangeCol3		= 0x4000,
			flagArrangeCol4		= 0x8000,
		} ;
		enum	CallbackCode
		{
			codeOnPushed	= 1,
			codeOnChanged,
			#if	defined(__PLATFORM_ANDROID__)
			codeOnCanceled,
			#endif
		} ;
		struct	ElementInfo ;
		typedef	bool (*ItemCallback)
			( SCustomDialog& dlg,
				const ElementInfo& item,
				int code, void * instance ) ;
		struct	ElementInfo
		{
			const wchar_t *	pwszID ;
			ElementType		type ;
			uint32_t		nFlags ;	// complex of enum ElementFlag
			uint32_t		optFlags ;	// complex of enum MessageEditBoxStyle
			uint32_t		nMinWidth ;
			const wchar_t *	pwszText ;
			int				nValue ;
			int				minRange ;
			int				maxRange ;
			ItemCallback	pfnCallback ;
			void *			pInstance ;
		} ;
		// リスナー
		class	Listener	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Listener, ESLObject )
			// 初期化処理
			virtual void OnInitDialog( SCustomDialog& dlg ) ;
			// キャンセル処理
			virtual bool OnCancel( SCustomDialog& dlg ) ;
		} ;

		#if	!defined(__COTOPHA__)
		class	ElementData
		{
		public:
			ElementInfo	m_info ;
			SString		m_strID ;
			SString		m_strText ;

			#if	defined(__PLATFORM_WINDOWS__)
			UINT		m_nCtrlID ;
			HWND		m_hWndCtrl ;

			#elif	defined(__PLATFORM_ANDROID__)
			JNI::JavaObject	m_jobjItem ;

			#endif

		public:
			// 構築関数
			ElementData( const ElementInfo& ei ) ;
			ElementData( const ElementData& ed ) ;
			// 設定
			void SetElementInfo( const ElementInfo& ei ) ;

			#if	defined(__PLATFORM_ANDROID__)
			// Java オブジェクトへ更新
			void ToJavaObject( void ) ;
			// Java オブジェクトから取得
			void FromJavaObject( void ) ;
			// 入力値を取得
			void GetInputValue( void ) ;
			// 数値設定
			void SetViewInteger( int nValue ) ;
			// 文字列設定
			void SetViewString( const wchar_t * pwszText ) ;
			// 有効状態設定
			void SetEnabled( bool flagEnable ) ;
			#endif
		} ;

	protected:
		SObjectArray<ElementData>	m_elements ;
		SString						m_strCaption ;
		Listener *					m_pListener ;
		bool						m_flagInModal ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SCustomDialog, ESLObject )
		// 構築関数
		SCustomDialog( void ) ;
		// 消滅関数
		virtual ~SCustomDialog( void ) ;

	public:
		// フォーム設定
		void SetCustomItems
			( const ElementInfo * pElements, size_t nCount ) ;
		// キャプション設定
		SError SetCaption( const wchar_t * pwszCaption ) ;
		// リスナ設定
		void AttachListener( Listener * pListener ) ;
		// 表示
		int DoModal
			( uint64_t nFlags = 0,
				SakuraGL::SGLAbstractWindow * pParentWnd = NULL ) ;
		// 入力結果取得
		int GetInputIntegerAs( const wchar_t * pwszID ) const ;
		const wchar_t * GetInputStringAs( const wchar_t * pwszID ) const ;

	protected:
		// アイテム情報取得
		ElementData * GetItemData( const wchar_t * pwszID ) const ;

	public:	// コールバック関数内から呼び出し可能
		// 数値設定
		void SetItemIntegerAs
			( const wchar_t * pwszID, int nValue ) ;
		// テキスト設定
		void SetItemStringAs
			( const wchar_t * pwszID, const wchar_t * pwszText ) ;
		// 有効／禁止状態設定
		void EnableItemAs( const wchar_t * pwszID, bool flagEnable ) ;
		// ダイアログ終了
		void EndDialog( int nResultCode ) ;
		// メッセージボックス表示
		int DoMessageBox
			( const wchar_t * pwszMsg,
				const wchar_t * pwszCaption, int nStyles ) ;
		// ウィンドウハンドル
		#if	defined(__PLATFORM_WINDOWS__)
		HWND GetWindowHandle( void ) const ;
		#endif

	protected:
	#if	defined(__PLATFORM_WINDOWS__)
		SThread			m_threadUI ;
		HWND			m_hDialog ;
		HWND			m_hParentWnd ;
		HFONT			m_hFont ;
		INT_PTR			m_nResultCode ;
		UINT			m_nNextCtrlID ;
		bool			m_flagGroupRadio ;

		class	StructItem
		{
		public:
			ElementData *				m_pElement ;
			SObjectArray<StructItem>	m_aElements ;
			POINT						m_ptElements ;
			SIZE						m_sizeElements ;
		public:
			StructItem( void )
				: m_pElement( NULL )
			{
				m_ptElements.x = 0 ;
				m_ptElements.y = 0 ;
				m_sizeElements.cx = 0 ;
				m_sizeElements.cy = 0 ;
			}
		} ;
		struct	LineItemsInfo
		{
			size_t		iFirst ;
			size_t		nCount ;
			int			nWidth ;
			uint32_t	nFlags ;
		} ;

		static const BYTE	m_bytCustomDlgData[64] ;

		// ダイアログ関数
		static INT_PTR CALLBACK CustomDialogProc
			( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
		// カスタムアイテムを生成
		void OnInitDialog( void ) ;
		void LayoutCustomItems
			( StructItem& si,
				size_t iFirst, size_t iEnd, int xPos, int yPos ) ;
		void CreateCustomItem( ElementData& ed ) ;
		// アイテムのサイズ取得
		SIZE SizeofStructItem( const StructItem& si ) const ;
		// アイテムの位置取得
		bool GetStructItemPosition
			( POINT& ptItem, const StructItem& si ) const ;
		// アイテム移動
		void OffsetMoveStructItem
			( StructItem& si, int xOffset, int yOffset ) const ;
		// アイテムサイズ変更
		void ResizeStructItem
			( StructItem& si, int nWidth, int nHeight ) const ;
		// コマンドメッセージ処理
		void OnCommand( WORD wID, WORD wNotifyCode ) ;
		// スクロールメッセージ処理
		void OnHScroll( WORD wSBCode, short int wPos, HWND hwndScroll ) ;
		// 入力結果を取得する
		void GetInputResult( void ) const ;
		void GetInputResultAs( ElementData& ed ) const ;

		// スレッド関数
		virtual void Run( void ) ;

	#elif	defined(__PLATFORM_ANDROID__)
		JNI::JavaObject		m_jobjDialog ;

		// 入力結果を取得する
		void GetInputResult( void ) const ;

	public:
		// コールバック関数
		bool OnCallbackItem( ElementData * ped, int code ) ;
	#endif

	} ;
	#endif

}

#endif

