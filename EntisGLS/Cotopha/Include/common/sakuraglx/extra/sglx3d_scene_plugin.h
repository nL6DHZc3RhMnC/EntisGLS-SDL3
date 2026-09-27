
#include <sakuraglx/render/sglx3d_scene_composer.h>

#if	!defined(__SAKURAGLX_SCENE_PLUGIN_H__)
#define	__SAKURAGLX_SCENE_PLUGIN_H__	1

#if	!defined(S3D_PLUGIN_STDCALL)
	#if	defined(__PLATFORM_WINDOWS__)
		#define	S3D_PLUGIN_STDCALL	__stdcall
	#else
		#define	S3D_PLUGIN_STDCALL
	#endif
#endif

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// プラグイン・エントリ
	//////////////////////////////////////////////////////////////////////////

	enum	S3DSceneComposerPluginDescriptorType
	{
		s3dscenePluginTypeNull			= 0,
		s3dscenePluginTypeItem			= 1,
		s3dscenePluginTypeController	= 2,
		s3dscenePluginTypeProcRsrc		= 3,
	} ;
	enum	S3DSceneComposerPluginMenuState
	{
		s3dscenePluginMenuDisabled	= 0x0001,
		s3dscenePluginMenuChecked	= 0x0002,
	} ;

	struct	S3DSceneComposerEditorEnvironment
	{
		double	fpSceneScale ;				// {1.0 | 100.0}
	} ;

	struct	S3DSceneComposerPluginDescriptor
	{
		S3DSceneComposerPluginDescriptorType	type ;
		const wchar_t *							pwszMenuPath ;
		const wchar_t *							pwszID ;
		ESLObject * (S3D_PLUGIN_STDCALL *pfnCreateItem)
				( void * pPluginInstance,
					const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposerEditorEnvironment& env ) ;
		uint32_t (S3D_PLUGIN_STDCALL *pfnOnUpdateMenu)
				( void * pPluginInstance,
					const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposer::ItemSerializer* pParentItem ) ;
	} ;

	struct	S3DSceneComposerPluginSceneInfo
	{
		class S3DSceneComposer *	pComposer ;
		class SGLWindowSprite *		pWindow ;
		class S3DSceneSprite *		pScene ;
		class SGLVirtualInput *		pInput ;
		SGLVRViewProducer *			pVR ;
		void *						pReserved[27] ;
	} ;

	struct	S3DSceneComposerPluginEntry
	{
		void * (S3D_PLUGIN_STDCALL *pfnStartup)
			( S3DCompositionManager * pManager, const S3DSceneComposerPluginEntry * pEntry ) ;
		void (S3D_PLUGIN_STDCALL *pfnShoutdown)
				( S3DCompositionManager * pManager, void * pPluginInstance ) ;
		const S3DSceneComposerPluginDescriptor *
			(S3D_PLUGIN_STDCALL *pfnGetDescriptorTable)
				( void * pPluginInstance, size_t& nTableSize ) ;
		void * (S3D_PLUGIN_STDCALL *pfnOnStartScene)
			( void * pPluginInstance,
				const S3DSceneComposerPluginSceneInfo& scpsi ) ;
		void (S3D_PLUGIN_STDCALL *pfnOnEndScene)
			( void * pPluginInstance, void * pSceneInstance ) ;
		void (S3D_PLUGIN_STDCALL *pfnOnStartComposition)
			( void * pPluginInstance, void * pSceneInstance,
				S3DSceneComposer::Composition * pComposition ) ;
	} ;

	typedef	const S3DSceneComposerPluginEntry *
				(S3D_PLUGIN_STDCALL *API_S3D_GetPluginEntry)( void ) ;


	//////////////////////////////////////////////////////////////////////////
	// プラグイン・実装基底
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneComposerPluginBase
	{
	protected:
		SSystem::SArray<S3DSceneComposerPluginDescriptor>
									m_descriptor ;
		S3DCompositionManager *		m_pManager ;

	public:
		// 構築関数
		S3DSceneComposerPluginBase
			( const S3DSceneComposerPluginDescriptor * pDesc, size_t nCount ) ;
		// 消滅関数
		virtual ~S3DSceneComposerPluginBase( void ) ;

	public:	// エントリ関数
		static void * S3D_PLUGIN_STDCALL entry_Startup
			( S3DCompositionManager * pManager, const S3DSceneComposerPluginEntry * pEntry ) ;
		static void S3D_PLUGIN_STDCALL entry_Shoutdown
				( S3DCompositionManager * pManager, void * pPluginInstance ) ;
		static const S3DSceneComposerPluginDescriptor *
			S3D_PLUGIN_STDCALL entry_GetDescriptorTable
				( void * pPluginInstance, size_t& nTableSize ) ;
		static void * S3D_PLUGIN_STDCALL entry_OnStartScene
			( void * pPluginInstance,
				const S3DSceneComposerPluginSceneInfo& scpsi ) ;
		static void S3D_PLUGIN_STDCALL entry_OnEndScene
			( void * pPluginInstance, void * pSceneInstance ) ;
		static void S3D_PLUGIN_STDCALL entry_OnStartComposition
			( void * pPluginInstance, void * pSceneInstance,
				S3DSceneComposer::Composition * pComposition ) ;

	public:
		// プラグインエントリ取得
		virtual const S3DSceneComposerPluginEntry * GetPluginEntry( void ) const = 0 ;
		// インスタンス作成時
		virtual void OnStartup( S3DCompositionManager * pManager ) ;
		// インスタンス破棄時
		virtual void OnShoutdown( S3DCompositionManager * pManager ) ;
		// プラグイン・ディスクリプタ取得
		virtual const S3DSceneComposerPluginDescriptor *
						GetDescriptorTable( size_t& nTableSize ) ;
		// シーン作成時
		virtual void * OnStartScene
			( const S3DSceneComposerPluginSceneInfo& scpsi ) ;
		// シーン破棄前
		virtual void OnEndScene( void * pSceneInstance ) ;
		// コンポジション開始前
		virtual void OnStartComposition
			( void * pSceneInstance,
				S3DSceneComposer::Composition * pComposition ) ;

	public:
		// S3DCompositionManager 取得
		S3DCompositionManager * GetCompositionManager( void ) const ;

	private:	// ディスクリプタ関数
		static ESLObject * S3D_PLUGIN_STDCALL desc_CreateItem
				( void * pPluginInstance,
					const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposerEditorEnvironment& env ) ;
		static uint32_t S3D_PLUGIN_STDCALL desc_OnUpdateMenu
				( void * pPluginInstance,
					const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposer::ItemSerializer* pParentItem ) ;

	protected:
		// アイテム作成
		virtual ESLObject * CreateItem
				( const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposerEditorEnvironment& env ) ;
		// メニュー状態
		virtual uint32_t OnUpdateMenu
				( const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposer::ItemSerializer* pParentItem ) ;

	} ;

	#define	S3D_SCENE_DECLARE_PLUGIN_CLASS(class_name)	\
		public:	\
		static void * S3D_PLUGIN_STDCALL entry_Startup( S3DCompositionManager * pManager, const S3DSceneComposerPluginEntry * pEntry ) ;	\
		static void S3D_PLUGIN_STDCALL entry_Shoutdown( S3DCompositionManager * pManager, void * pPluginInstance ) ; \
		static const S3DSceneComposerPluginEntry	s_scpePluginEntry ;	\
		virtual const S3DSceneComposerPluginEntry * GetPluginEntry( void ) const ;

	#define	S3D_SCENE_IMPLEMENT_PLUGIN_CLASS(class_name)	\
		void * class_name::entry_Startup( S3DCompositionManager * pManager, const S3DSceneComposerPluginEntry * pEntry )	\
		{	\
			class_name *	p = new class_name ;	\
			p->OnStartup( pManager ) ;	\
			return	p ;	\
		}	\
		void class_name::entry_Shoutdown( S3DCompositionManager * pManager, void * pPluginInstance )	\
		{	\
			class_name *	p = (class_name*) pPluginInstance ;	\
			p->OnShoutdown( pManager ) ;	\
			delete	p ;	\
		}	\
		const S3DSceneComposerPluginEntry * class_name::GetPluginEntry( void ) const	\
		{	\
			return	&class_name::s_scpePluginEntry ;	\
		}

	#define	S3D_SCENE_IMPLEMENT_PLUGIN_ENTRY(class_name)	\
		const SakuraGL::S3DSceneComposerPluginEntry	class_name::s_scpePluginEntry =	\
		{	\
			&class_name::entry_Startup,		\
			&class_name::entry_Shoutdown,	\
			&S3DSceneComposerPluginBase::entry_GetDescriptorTable,	\
			&S3DSceneComposerPluginBase::entry_OnStartScene,	\
			&S3DSceneComposerPluginBase::entry_OnEndScene,	\
			&S3DSceneComposerPluginBase::entry_OnStartComposition,	\
		} ;	\
		ECS_EXPORT const SakuraGL::S3DSceneComposerPluginEntry *	\
					S3D_PLUGIN_STDCALL s3dscene_GetPluginEntry( void )	\
		{	\
			return	&class_name::s_scpePluginEntry ;	\
		}


	//////////////////////////////////////////////////////////////////////////
	// プラグイン・インスタンス（ローダー）
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneComposerPluginInstance	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSceneComposerPluginInstance, ESLObject )
		// 構築関数
		S3DSceneComposerPluginInstance( void ) ;
		// 消滅関数
		virtual ~S3DSceneComposerPluginInstance( void ) ;

	protected:
		#if	defined(__PLATFORM_WINDOWS__)
		HMODULE										m_hModule ;
		#endif
		S3DCompositionManager *						m_pManager ;
		const S3DSceneComposerPluginEntry *			m_pEntry ;
		void *										m_pInstance ;
		void *										m_pSceneInstance ;
		const S3DSceneComposerPluginDescriptor *	m_pDescriptor ;
		size_t										m_nDescLength ;

	public:
		// プラグイン読み込む
		#if	defined(__PLATFORM_WINDOWS__)
		SGLError LoadPlugin
			( const wchar_t * pwszFileName,
						S3DCompositionManager * pManager ) ;
		#endif
		SGLError LoadPlugin
			( S3DSceneComposerPluginBase * pPlugin,
						S3DCompositionManager * pManager ) ;
		// プラグインを開放する
		SGLError Release( void ) ;
		// ディスクリプタテーブル取得
		const S3DSceneComposerPluginDescriptor *
			GetDescriptorTable( size_t& nTableSize ) const ;
		// シーン開始時準備処理
		void OnStartScene
			( const S3DSceneComposerPluginSceneInfo& scpsi ) ;
		// シーン終了時処理
		void OnEndScene( void ) ;
		// コンポジション開始前処理
		void OnStartComposition
			( S3DSceneComposer::Composition * pComposition ) ;
		// プラグイン・インスタンス取得
		void * GetInstance( void ) const ;
		// シーン・インスタンス取得
		void * GetSceneInstance( void ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 動的プラグイン
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneComposerDynamicPlugin : public S3DSceneComposerPluginInstance
	{
	public:
		class	Descriptor
		{
		public:
			S3DSceneComposerPluginDescriptorType	m_type ;
			SSystem::SString						m_strMenuPath ;
			SSystem::SString						m_strID ;
		public:
			Descriptor( S3DSceneComposerPluginDescriptorType type,
						const wchar_t * pwszMenuPath, const wchar_t * pwszID )
				: m_type( type ), m_strMenuPath( pwszMenuPath ), m_strID( pwszID ) {}
			virtual ~Descriptor( void ) {}
			virtual ESLObject * CreateItem
					( const S3DSceneComposerPluginDescriptor& desc,
						const S3DSceneComposerEditorEnvironment& env ) = 0 ;
			virtual uint32_t OnUpdateMenu
					( const S3DSceneComposerPluginDescriptor& desc,
						const S3DSceneComposer::ItemSerializer* pParentItem )
			{
				return	0 ;	// S3DSceneComposerPluginMenuState
			}
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSceneComposerDynamicPlugin, S3DSceneComposerPluginInstance )
		// 構築関数
		S3DSceneComposerDynamicPlugin( void ) ;
		// 消滅関数
		virtual ~S3DSceneComposerDynamicPlugin( void ) ;

	protected:
		SSystem::SArray<S3DSceneComposerPluginDescriptor>	m_aDescTable ;
		SSystem::SObjectArray<Descriptor>					m_aDescriptors ;

		struct	Instance
		{
			S3DSceneComposerDynamicPlugin *	pPlugin ;
			void *							pInstance ;
		} ;
		struct	PluginEntry	: public S3DSceneComposerPluginEntry
		{
			S3DSceneComposerDynamicPlugin *	pPlugin ;
		} ;
		PluginEntry	m_entry ;

	public:
		// プラグイン・インスタンス生成
		void InitPlugin( S3DCompositionManager * pManager ) ;
		// Descriptor 追加
		void AddDescriptor( Descriptor * pDesc ) ;

	protected:
		static ESLObject * S3D_PLUGIN_STDCALL CreateItemProc
				( void * pPluginInstance,
					const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposerEditorEnvironment& env ) ;
		static uint32_t S3D_PLUGIN_STDCALL OnUpdateMenuProc
				( void * pPluginInstance,
					const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposer::ItemSerializer* pParentItem ) ;
		static void * S3D_PLUGIN_STDCALL StartupProc
			( S3DCompositionManager * pManager, const S3DSceneComposerPluginEntry * pEntry ) ;
		static void S3D_PLUGIN_STDCALL ShoutdownProc
				( S3DCompositionManager * pManager, void * pPluginInstance ) ;
		static const S3DSceneComposerPluginDescriptor *
			S3D_PLUGIN_STDCALL GetDescriptorTableProc
				( void * pPluginInstance, size_t& nTableSize ) ;
		static void * S3D_PLUGIN_STDCALL OnStartSceneProc
			( void * pPluginInstance,
				const S3DSceneComposerPluginSceneInfo& scpsi ) ;
		static void S3D_PLUGIN_STDCALL OnEndSceneProc
			( void * pPluginInstance, void * pSceneInstance ) ;
		static void S3D_PLUGIN_STDCALL OnStartCompositionProc
			( void * pPluginInstance, void * pSceneInstance,
				S3DSceneComposer::Composition * pComposition ) ;

	public:
		// プラグイン開始処理実装
		virtual void * Startup
			( S3DCompositionManager * pManager ) ;
		// プラグイン後始末処理実装
		virtual void Shoutdown
				( S3DCompositionManager * pManager, void * pPluginInstance ) ;
		// プラグイン・シーン開始処理実装
		virtual void * StartScene
			( void * pPluginInstance,
				const S3DSceneComposerPluginSceneInfo& scpsi ) ;
		// プラグイン・シーン終了処理実装
		virtual void EndScene( void * pPluginInstance, void * pSceneInstance ) ;
		// プラグイン・コンポジション開始処理実装
		virtual void StartComposition
			( void * pPluginInstance, void * pSceneInstance,
				S3DSceneComposer::Composition * pComposition ) ;

	} ;


}

#endif

