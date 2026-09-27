
#if	!defined(__SAKURAGL_SGL_WINDOW_MENU_H__)
#define	__SAKURAGL_SGL_WINDOW_MENU_H__	1

#if	defined(__PLATFORM_ANDROID__)
#include <esl/esl_java_object.h>
#endif

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// メニュー
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native WindowMenu
	{
	public:
		struct	Entry
		{
			uint64_t		nFlags ;
			const wchar_t *	pwszText ;
			const wchar_t *	pwszID ;
			const Entry *	pSubMenu ;
			ssize_t			nSubCount ;
			size_t			nReserved ;
		} ;
		enum	Flags
		{
			flagChecked			= 0x0001,
			flagDisabled		= 0x0002,
			flagCheckItem		= 0x0010,
			flagRadioItem		= 0x0020,
			flagSeparator		= 0x1000,
			flagSubMenu			= 0x2000,
		} ;

	public:
		// メニュー生成
		native SGLError CreateMenu
			( const Entry * pMenuEntries, ssize_t nCount = -1 ) ;
		// ポップアップメニュー生成
		native SGLError CreatePopupMenu
			( const Entry * pMenuEntries, ssize_t nCount = -1 ) ;
		// メニュー削除
		native SGLError DeleteMenu( void ) ;
		// ポップアップメニュー表示
		native SGLError ShowPopupMenu
			( Window * pWindow,
					double x, double y, uint32_t nFlags = 0 ) ;
		// メニューアイテム有効状態変更
		native SGLError EnableMenuItem
			( const wchar_t * pwszID, bool fEnabled ) ;
		// メニューアイテムチェック状態変更
		native SGLError CheckMenuItem
			( const wchar_t * pwszID, bool fChecked ) ;
		// メニューアイテムテキスト変更
		native SGLError SetMenuItemText
			( const wchar_t * pwszID, const wchar_t * pwszText ) ;

	} ;
	#endif

	class	SGLWindowMenu : public SSystem::SObject
						#if	defined(__PLATFORM_ANDROID__)
								, public JNI::JavaObject
						#endif
	{
	public:
		#if	!defined(__COTOPHA__)
		struct	Entry
		{
			uint64_t		nFlags ;
			const wchar_t *	pwszText ;
			const wchar_t *	pwszID ;
			const Entry *	pSubMenu ;
			ssize_t			nSubCount ;
			size_t			nReserved ;
		} ;
		enum	Flags
		{
			flagChecked			= 0x0001,
			flagDisabled		= 0x0002,
			flagCheckItem		= 0x0010,
			flagRadioItem		= 0x0020,
			flagSeparator		= 0x1000,
			flagSubMenu			= 0x2000,
		} ;
		#endif

	protected:
		#if	defined(__COTOPHA__)
			WindowMenu *	m_pMenu ;
		public:
			// メニューオブジェクト取得
			WindowMenu * GetMenuObject( void ) const ;

		#else
			// メニューハンドル
			#if	defined(__PLATFORM_WINDOWS__)
				typedef	HMENU	MenuItemHandle ;
				HMENU			m_hMenu ;
			#else
				typedef	void *	MenuItemHandle ;
			#endif

			// 設定先ウィンドウ
			SSystem::SSmartReference<SGLAbstractWindow>	m_refWindow ;

			// メニューアイテム
			struct	MenuItem
			{
				uint64_t							nFlags ;
				SSystem::SString					strID ;
				SSystem::SString					strText ;
				SSystem::SPointerArray<MenuItem>	listSubMenu ;
				MenuItemHandle						hSubMenu ;
				MenuItemHandle						hParentMenu ;
				uint32_t							idMenuCommand ;

				#if	defined(__PLATFORM_WINDOWS__)
				void ToMenuItemInfo
					( MENUITEMINFO& mii, SSystem::SArray<char>& bufText ) const ;
				#endif
			} ;
			SSystem::SStrSortObjectArray<MenuItem>	m_ssoaMenuItems ;
			SSystem::SPointerArray<MenuItem>		m_listRootMenu ;

			// メニュー構築
			SGLError BuildMenuObject
				( const WindowMenu::Entry * pMenuEntries, ssize_t nCount, bool flagPopup ) ;
			// メニュー情報構築
			void BuildMenuInfo
				( SSystem::SPointerArray<MenuItem>& listMenu,
					const WindowMenu::Entry * pMenuEntries, ssize_t nCount ) ;
			// メニューオブジェクト生成
			#if	defined(__PLATFORM_WINDOWS__)
			HMENU CreateMenuObject
				( SSystem::SPointerArray<MenuItem>& listMenu, bool flagPopup ) ;
			#elif	defined(__PLATFORM_ANDROID__)
			void CreateMenuObject
				( SSystem::SPointerArray<MenuItem>& listMenu ) ;
			void CreateJavaMenuInfo
				( JNI::JavaObject& jobjMenuList, SSystem::SPointerArray<MenuItem>& listMenu ) ;
			#endif
			// メニューオブジェクト削除
			void DeleteMenuObject( void ) ;
			// メニュー表示更新
			#if	defined(__PLATFORM_WINDOWS__)
			void UpdateMenuBar( void ) ;
			class	UpdateMenuBarProcedure	: public SSystem::SProcedure
			{
			protected:
				HMENU	m_hMenu ;
				int		m_xPos, m_yPos ;
				HWND	m_hWnd ;
			public:
				// 構築関数
				UpdateMenuBarProcedure
					( HMENU hMenu, int xPos, int yPos, HWND hWnd )
					: m_hMenu(hMenu), m_xPos(xPos), m_yPos(yPos), m_hWnd(hWnd) { }
				// スレッド関数
				virtual void Run( void ) ;
				// 完了後の処理
				virtual void Finalize( void ) ;
			} ;
			#endif

		public:
			// ウィンドウ関連付け
			void OnAttachReferenceWindow( SGLAbstractWindow * pWindow ) ;
			// メニューハンドル取得
			#if	defined(__PLATFORM_WINDOWS__)
			HMENU GetMenuHandle( void ) const ;
			#endif

		protected:
			// コマンドID (0x1000～0x2FFF)
			static ESL_DLL_EXPORT SSystem::SObjectArray<SSystem::SString>	m_mapCommandIDs ;

		public:
			// コマンドID登録
			static uint32_t RegisterCommandID( const wchar_t * pwszID ) ;
			// コマンドID名逆引き
			static const wchar_t * GetCommandIDOf( UINT nID ) ;
			// 終了時解放処理
			static void FinalizeWindowMenu( void ) ;
		#endif

	public:
		// クラス情報
		#if	defined(__PLATFORM_ANDROID__)
		ESL_DECLARE_CLASS_INFO2( SGLWindowMenu, SObject, JavaObject )
		#else
		ESL_DECLARE_CLASS_INFO( SGLWindowMenu, SObject )
		#endif
		// 構築関数
		SGLWindowMenu( void ) ;
		// 消滅関数
		virtual ~SGLWindowMenu( void ) ;
		// メニュー生成
		SGLError CreateMenu
			( const WindowMenu::Entry * pMenuEntries, ssize_t nCount = -1 ) ;
		// ポップアップメニュー生成
		SGLError CreatePopupMenu
			( const WindowMenu::Entry * pMenuEntries, ssize_t nCount = -1 ) ;
		// メニュー削除
		SGLError DeleteMenu( void ) ;
		// ポップアップメニュー表示
		SGLError ShowPopupMenu
			( SGLAbstractWindow * pWindow,
					double x, double y, uint32_t nFlags = 0 ) ;
		// メニューアイテム有効状態変更
		SGLError EnableMenuItem
			( const wchar_t * pwszID, bool fEnabled ) ;
		// メニューアイテムチェック状態変更
		SGLError CheckMenuItem
			( const wchar_t * pwszID, bool fChecked ) ;
		// メニューアイテムテキスト変更
		SGLError SetMenuItemText
			( const wchar_t * pwszID, const wchar_t * pwszText ) ;

	} ;

}

#endif

