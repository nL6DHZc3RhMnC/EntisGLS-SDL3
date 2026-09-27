
/*****************************************************************************
				詞葉環境・Sakura2 仮想マシン
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_ENVIRONMENT_VM_H__)
#define	__GLSCS_SAKURA2_ENVIRONMENT_VM_H__

#if	defined(__PLATFORM_ANDROID__)
#include <esl/esl_java_object.h>
#endif

namespace	SakuraGL
{
	class	S3DShaderBinaryLibrary ;
} ;

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// Sakura2 環境＆仮想マシン
	//////////////////////////////////////////////////////////////////////////

	class	EnvironmentVM	: public StandardVM,
								public SSystem::SEnvironment,
								public SSystem::SParserErrorInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( EnvironmentVM, StandardVM, SEnvironment, SParserErrorInterface )
		// 構築関数
		EnvironmentVM( void ) ;
		// 消滅関数
		virtual ~EnvironmentVM( void ) ;

	public:
		// プラグイン・インターフェース
		struct	PluginEntry
		{
			void (__stdcall *pfnStartup)
				( EnvironmentVM * vm, const wchar_t * pwszArg ) ;
			void (__stdcall *pfnShutdown)( void ) ;
			void (__stdcall *pfnOnModuleAttached)
				( EnvironmentVM * vm, int iModule, ExecutableModule * module ) ;
			void (__stdcall *pfnOnModuleDetached)
				( EnvironmentVM * vm, int iModule, ExecutableModule * module ) ;
			void (__stdcall *pfnOnThreadAttached)
				( EnvironmentVM * vm, ThreadObject * thread ) ;
			void (__stdcall *pfnOnThreadDetached)
				( EnvironmentVM * vm, ThreadObject * thread ) ;
			DWORD (__stdcall *pfnOnExceptionEscape)
				( EnvironmentVM * vm, Context * context, DWORD maskException ) ;
			bool (__stdcall *pfnOnHandleExceptionError)
				( EnvironmentVM * vm, Context * context, const wchar_t * pwszErr ) ;
		} ;
		class	PluginInterface	: public ESLObject
		{
		public:
			PluginEntry *	m_entry ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( PluginInterface, ESLObject )
			// 構築関数
			PluginInterface
				( EnvironmentVM * vm,
					PluginEntry * entry, const wchar_t * pwszArg ) ;
			// 消滅関数
			virtual ~PluginInterface( void ) ;
			// エクスポート関数取得
			virtual void * GetModuleExportFunction( const char * pszFuncName ) ;
			// モジュール通知
			void OnModuleAttached
				( EnvironmentVM * vm, int iModule, ExecutableModule * module ) ;
			void OnModuleDetached
				( EnvironmentVM * vm, int iModule, ExecutableModule * module ) ;
			// スレッド通知
			void OnThreadAttached
				( EnvironmentVM * vm, ThreadObject * thread ) ;
			void OnThreadDetached
				( EnvironmentVM * vm, ThreadObject * thread ) ;
			// デバッグ用例外エラー処理
			DWORD OnExceptionEscape
				( EnvironmentVM * vm, Context * context, DWORD maskException ) ;
			// 処理されない例外エラー処理
			bool OnHandleExceptionError
				( EnvironmentVM * vm, Context * context, const wchar_t * pwszErr ) ;
		} ;

	protected:
		enum	LoadedStatus
		{
			loadedNothing,
			loadedEnvironment,
			loadedModule,
			initializingModule,
			initializedModule,
			runningModule,
			finishedModule,
		} ;
		LoadedStatus		m_statusLoaded ;
		SSystem::SString	m_strErrMsg ;
		ExecutableModule	m_module ;

		SSystem::SObjectArray<PluginInterface>	m_plugins ;

		SakuraGL::S3DShaderBinaryLibrary *	m_plibShaderBinary ;

	public:
		// 環境初期設定
		virtual SError InitEnvironment( void ) ;
		// 環境読み込み
		virtual SError LoadEnvironment
			( SFileInterface& file, SProgressiveUserInterface * pUI = NULL ) ;
		// プライマリ詞葉モジュール読み込み
		virtual SError LoadPrimaryModule( SFileInterface* file = NULL ) ;
		// プライマリ詞葉モジュールをアンロード
		virtual SError UnloadPrimaryModule( void ) ;
		// プライマリスレッド実行
		virtual SError Run( const wchar_t * pwszArg ) ;
		// StaticInitialize 関数実行
		virtual SError RunStaticInitialize( void ) ;
		// main 関数実行
		virtual SError RunMain( const wchar_t * pwszArg ) ;

	public:
		// コンテキスト保存処理
		virtual SError SaveDynamicContext( SFileInterface& file ) ;
		// コンテキスト復元処理
		virtual SError LoadDynamicContext( SFileInterface& file ) ;

	public:
		// プライマリ詞葉モジュール取得
		const ExecutableModule& GetPrimaryModule( void ) const
		{
			return	m_module ;
		}
		// エラーメッセージを取得
		const SSystem::SString & GetErrorMessage( void ) const
		{
			return	m_strErrMsg ;
		}

	public:	// StandardVM オーバーライド
		// 仮想マシンの解放
		virtual void ReleaseVM( void ) ;
		// 処理されない例外エラー処理
		virtual void HandleExceptionError
			( Context * context, const wchar_t * pwszErr ) ;
		// デバッグ用例外エラー処理
		virtual DWORD HandleExceptionEscape
				( Context * context, DWORD maskException ) ;
		// モジュールがアタッチされた（デバッグ・フック処理用）
		virtual void OnModuleAttached( int iModule, ExecutableModule * module ) ;
		// モジュールがデタッチされる（デバッグ・フック処理用）
		virtual void OnModuleDetached( int iModule, ExecutableModule * module ) ;
		// スレッドがアタッチされた（デバッグ・フック処理用）
		virtual void OnThreadAttached( ThreadObject * thread ) ;
		// スレッドがデタッチされた（デバッグ・フック処理用）
		virtual void OnThreadDetached( ThreadObject * thread ) ;
		// エクスポート関数取得
		virtual void * GetModuleExportFunction( const wchar_t * pszFuncName ) ;

	public:	// SEnvironment オーバーライド
		// 非標準タグ解釈
		virtual void ParseExtendedEnvironment
				( const SSystem::SXMLDocument& xmlTag ) ;
		// <module> タグ解釈
		virtual void ParseEnvironmentModuleTag
				( const SSystem::SXMLDocument& xmlTag ) ;
		// <fonts> タグ解釈
		virtual void ParseEnvironmentFontsTag
				( const SSystem::SXMLDocument& xmlTag ) ;
		// <fonts><filter> タグ解釈
		virtual void ParseEnvironmentFontsFilterTag
				( const SSystem::SXMLDocument& xmlTag ) ;
		// <fonts><file> タグ解釈
		virtual void ParseEnvironmentFontsFileTag
				( const SSystem::SXMLDocument& xmlTag ) ;
		// <sound> タグ解釈
		virtual void ParseEnvironmentSoundTag
				( const SSystem::SXMLDocument& xmlTag ) ;
		// <opengl> タグ解釈
		virtual void ParseEnvironmentOpenGLTag
				( const SSystem::SXMLDocument& xmlTag ) ;
		void ParseOpenGLDisableSwitch
			( bool& fSwitch,
				const SSystem::SXMLDocument& xmlTag,
				const wchar_t * pwszAttr ) ;

	public:
		// ファイルを開く
		virtual SSystem::SFileInterface * NewOpenFile
				( const wchar_t * pwszFilePath, long int nOpenFlags ) const ;
		// ファイルは存在するか？
		virtual bool IsExistingFile( const wchar_t * pwszFilePath ) const ;

	public:	// SParserErrorInterface 実装
		virtual void OutputError
			( const SSystem::SStringParser& ss, const wchar_t * pszError ) ;
		virtual void OutputWarning
			( const SSystem::SStringParser& ss, const wchar_t * pszWarning ) ;

	protected:
		SSystem::SSmartPointer<ESLObject>	m_pSilentSound ;

	public:
		// 無音音声をループ再生する
		void StartSilentSound
			( uint32_t msec = 500,
				uint32_t freq = 44100, uint32_t ch = 2, uint32_t bits = 16 ) ;
		// 無音音声のループ再生を停止する
		void EndSilentSound( void ) ;

	#if	defined(__PLATFORM_WINDOWS__)
		// プラグイン・オブジェクト (Windows)
		class	PluginObject : public PluginInterface
		{
		public:
			enum	VersionNumber
			{
				numVersion1	= 0x00010000,
			} ;
			typedef PluginEntry *
					(__stdcall *SVM_PLUGIN_ENTRYPOINT)( int nVersion ) ;
		public:
			HMODULE	m_hModule ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( PluginObject, PluginInterface )
			// 構築関数
			PluginObject
				( HMODULE hModule, EnvironmentVM * vm,
					PluginEntry * entry, const wchar_t * pwszArg ) ;
			// 消滅関数
			virtual ~PluginObject( void ) ;
			// エクスポート関数取得
			virtual void * GetModuleExportFunction( const char * pszFuncName ) ;
		public:
			// プラグイン・ロード
			static PluginObject * LoadPlugin
				( EnvironmentVM * vm,
					const char * pszDLL, const wchar_t * pwszArg ) ;
		} ;
	#endif

	} ;

}

#endif
