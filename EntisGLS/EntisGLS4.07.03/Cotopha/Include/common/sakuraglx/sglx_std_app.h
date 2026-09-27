
#if	!defined(__SAKURAGLX_STD_APP_H__)
#define	__SAKURAGLX_STD_APP_H__	1

#include <sakura/ssys_module.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// プロファイル
	//////////////////////////////////////////////////////////////////////////

	class	SGLAppProfile	: public SSystem::SXMLDocument
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAppProfile, SXMLDocument )
		// 構築関数
		SGLAppProfile( void ) ;
		// 消滅関数
		virtual ~SGLAppProfile( void ) ;

	protected:
		SSystem::SString	m_strFilePath ;
		SXMLDocument *		m_pxmlProfile ;

	public:
		// 圧縮・暗号化されたプロファイルを読み込む
		SGLError LoadProfile
			( const wchar_t * pwszFilePath, const wchar_t * pwszPassword = L"" ) ;
		// プロファイルを保存する
		SGLError SaveProfile( const wchar_t * pwszPassword = L"" ) ;
		SGLError SaveProfileAs
			( const wchar_t * pwszFilePath, const wchar_t * pwszPassword = L"" ) ;
		// 新規作成
		void CreateProfile( const wchar_t * pwszFilePath ) ;
		// ファイルパス取得
		const SSystem::SString& GetFilePath( void ) const ;
		// タグ取得／生成 (<profile> 以下のパス)
		SXMLDocument * GetProfileOf( const wchar_t * pwszPath = NULL ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 標準的なアプリケーション・テンプレート
	//////////////////////////////////////////////////////////////////////////

	class	SGLStdApplication	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLStdApplication, ESLObject )
		// 構築関数
		SGLStdApplication( void ) ;
		// 消滅関数
		virtual ~SGLStdApplication( void ) ;

	protected:
		#if	!defined(__COTOPHA__)
		ECSSakura2::EnvironmentVM	m_env ;			// Sakura2 仮想マシン環境

		#if	defined(__PLATFORM_WINDOWS__)
		HANDLE						m_hMutex ;		// 二重起動防止用

		#elif	defined(__PLATFORM_ANDROID__)
		SSystem::SProgressiveUserInterface *	m_pProgDialog ;

		#endif
		#endif

		SGLAppProfile				m_profile ;		// プロファイル

		static SGLStdApplication *	m_pApp ;
		static wchar_t	m_wszUserUniqueId[257] ;

	public:
		// 引数解釈
		virtual SGLError ParseCmdLine( const wchar_t * pwszArg ) ;
		// 初期化処理
		virtual SGLError Initialize( void ) ;
		// 環境設定ファイルパスを取得
		virtual const wchar_t *
			GetEnvironmentFilePath( SSystem::SString& strFilePath ) const ;
		// プロファイルパスを取得
		virtual const wchar_t *
			GetProfileFilePath( SSystem::SString& strFilePath ) const ;
		// プロファイルパスワードを取得
		virtual const wchar_t *
			GetProfilePassword( SSystem::SString& strPassword ) const ;
		// 初期準備
		virtual SGLError PrepareStart( void ) ;
		// アプリケーション準備
		virtual SGLError StartUpApp( void ) ;
		// 終了処理
		virtual void Release( int nExitCode ) ;
		// 実行
		virtual int Run( void ) ;

	public:
		#if	!defined(__COTOPHA__)
		// ECSSakura2::EnvironmentVM 取得
		ECSSakura2::EnvironmentVM & GetEnvironmentVM( void ) ;
		#endif

		#if	defined(__PLATFORM_WINDOWS__)
		// 二重起動チェック
		bool CheckMultiBoot( void ) ;
		// 二重起動排他オブジェクト解放
		void ReleaseExclusiveBoot( void ) ;
		#endif

	protected:
		// 固有IDの取得（準備）
		virtual void PrepareUserUniqueId( void ) ;
		static void PrepareUserUniqueId( const wchar_t * pwszUUID ) ;
		virtual SSystem::SString GetUserUniqueId( void ) ;
		static SSystem::SString GetBasicUserUniqueId( void ) ;

		#if	defined(__PLATFORM_WINDOWS__)
		// UUID のファイルからの読み込み／新規生成
		virtual SSystem::SString GetUserUniqueId( const wchar_t * pwszBaseDirName ) ;

		#elif	defined(__PLATFORM_ANDROID__)
		// UUID のファイルからの読み込み／新規生成
		virtual SSystem::SString GetUserUniqueId( const wchar_t * pwszBaseDirName ) ;
		#endif

		// UUID 読み込み
		virtual SSystem::SError LoadUserUniqueId
				( SSystem::SString& strUUID, const wchar_t * pwszBaseDirName ) ;
		static SSystem::SError LoadUserUniqueId
				( SSystem::SString& strUUID,
					const wchar_t * pwszFilePath, const wchar_t * pwszPassword ) ;
		// UUID 保存
		virtual SSystem::SError SaveUserUniqueId
				( const wchar_t * pwszUUID, const wchar_t * pwszBaseDirName ) ;
		static SSystem::SError SaveUserUniqueId
				( const wchar_t * pwszUUID,
					const wchar_t * pwszFilePath, const wchar_t * pwszPassword ) ;

		// UUID ストレージ保存パス取得（空文字列を返すと保存しない）
		virtual SSystem::SString GetUUIDStoragePath( const wchar_t * pwszBaseDirName ) ;
		// UUID ストレージ保存用ベースディレクトリ（オフセットパス）
		virtual SSystem::SString GetUUIDStorageBaseDirectory( void ) ;
		// UUID ストレージ保存用ファイル名
		virtual SSystem::SString GetUUIDStorageFileName( void ) ;
		// UUID ストレージ保存用パスワード
		virtual SSystem::SString GetUUIDCryptyPassword( void ) const ;
		// UUID をストレージに保存するか？（false では読み込みのみ）
		virtual bool IsSaveUUIDintoStorage( void ) const ;

	protected:
		// 進行状況表示インターフェース生成
		virtual SSystem::SProgressiveUserInterface *
							NewProgressiveUserInterface( void ) ;
	public:
		// StartUpApp 中の起動中待ちダイアログ表示メッセージ変更
		// ※2行目以降に追加。\r から始まる文字列の場合1行目は削除
		void ChangeStartUpSpinnerMessage( const wchar_t * pwszMsg ) ;
		// StartUpApp 中の起動中待ちダイアログを閉じる
		void CloseStartUpSpinner( void ) ;

	public:
		// プロファイル・タグ取得／生成 (<profile> 以下のパス)
		SSystem::SXMLDocument *
				GetProfileOf( const wchar_t * pwszPath = NULL ) ;
		// プロファイル値取得
		const wchar_t * GetProfileString
			( const wchar_t * pwszPath,
				const wchar_t * pwszName,
				const wchar_t * pwszDefValue = NULL ) ;
		int64_t GetProfileInteger
			( const wchar_t * pwszPath,
				const wchar_t * pwszName, int64_t nDefValue = 0 ) ;
		int64_t GetProfileHexInteger
			( const wchar_t * pwszPath,
				const wchar_t * pwszName, int64_t nDefValue = 0 ) ;
		double GetProfileNumber
			( const wchar_t * pwszPath,
				const wchar_t * pwszName, double nDefValue = 0.0 ) ;
		// プロファイル値設定
		void SetProfileString
			( const wchar_t * pwszPath,
				const wchar_t * pwszName, const wchar_t * pwszValue ) ;
		void SetProfileInteger
			( const wchar_t * pwszPath,
				const wchar_t * pwszName, int64_t nValue ) ;
		void SetProfileHexInteger
			( const wchar_t * pwszPath,
				const wchar_t * pwszName, int64_t nValue ) ;
		void SetProfileNumber
			( const wchar_t * pwszPath,
				const wchar_t * pwszName, double nValue ) ;
		// プロファイル保存
		void SaveProfile( void ) ;

	public:
		// 環境変数取得
		static const wchar_t * GetEnvironmentVariable
				( const wchar_t * pwszName, SSystem::SString& strVar ) ;
		// 環境変数名一覧
		static void EnumerateEnvironmentVariableNames
			( SSystem::SObjectArray<SSystem::SString>& lstVarNames ) ;
		// マシン固有値の取得
		static SSystem::SString GetMachineUniqueId( void ) ;
		// 表示用マシンの名前／OSバージョン
		static SSystem::SString GetMachineNameAndOSVersion( void ) ;
		// Windows コンピューター名取得
		static SSystem::SString GetComputerName( void ) ;
		// Windows ユーザー名取得
		static SSystem::SString GetUserName( void ) ;
		// 表示用OSバージョン
		static SSystem::SString GetOSVersionString( void ) ;
		// 実行中アプリケーション取得
		static SGLStdApplication * GetApp( void )
		{
			return	m_pApp ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// サービス
	//////////////////////////////////////////////////////////////////////////

	class	SGLService ;
	class	SGLServiceListener
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLServiceListener )
		// サービス初期化処理
		virtual void OnInitializeService( SGLService * service ) ;
		// サービス開始時処理
		virtual void OnStartService( SGLService * service ) ;
		// サービス終了時処理
		virtual void OnFinishService( SGLService * service ) ;
		// サービス実行
		virtual void OnServiceTask
			( SGLService * service, const wchar_t * pwszAction ) ;
	} ;

	class	SGLService	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLService, SObject )
		// 構築関数
		SGLService( void ) ;
		// 消滅関数
		virtual ~SGLService( void ) ;

	protected:
		#if	defined(__PLATFORM_ANDROID__)
			bool	m_flagStartup ;

		#else
			class	ServiceProc	: public	SSystem::SProcedure
			{
			public:
				SGLService *	m_pService ;
			public:
				ServiceProc( SGLService * pService )
					: m_pService( pService ) { }
				virtual void Run( void ) ;
			} ;
			SSystem::SThread			m_thread ;
			ServiceProc *				m_proc ;
			SSystem::SSignalEvent		m_sevShutdown ;
			bool						m_flagSchedule ;
			SSystem::DATE_TIME			m_dtTaskSchedule ;
			SSystem::SString			m_strTaskAction ;

			friend class ServiceProc ;
		#endif

		SSystem::SCriticalSection	m_csSync ;		// 同期用
		SGLServiceListener *		m_pListener ;	// リスナ
		static SGLService *			m_pService ;

	public:
		// サービス開始
		SGLError StartupService( void ) ;
		// サービス終了
		SGLError ShutdownService( void ) ;
		// スケジュール
		SGLError ScheduleServiceTask
			( const SSystem::DATE_TIME& dt, const wchar_t * pwszAction ) ;
		// スケジュール解除
		SGLError UnscheduleServiceTask
			( const SSystem::DATE_TIME& dt, const wchar_t * pwszAction ) ;
		// リスナ設定
		void AttachListener( SGLServiceListener * pListener ) ;
		// 実行中サービス取得
		static SGLService * GetService( void )
		{
			return	m_pService ;
		}

	public:
		// サービス初期化処理
		virtual void OnInitializeService( void ) ;
		// サービス開始時処理
		virtual void OnStartService( void ) ;
		// サービス終了時処理
		virtual void OnFinishService( void ) ;
		// サービス実行
		virtual void OnServiceTask( const wchar_t * pwszAction ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 標準的なサービスリスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLStdServiceListener
				: public SSystem::SObject, public SGLServiceListener
	{
	public:
		// サービスイベント
		class	EventTask	: public SSystem::SObject
		{
		protected:
			SSystem::SString	m_strID ;
			int					m_nPriority ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( EventTask, SObject ) ;
			// 構築関数
			EventTask( void ) ;
			EventTask( const wchar_t * pwszID, int nPriority = 0 ) ;
			EventTask( const EventTask& evtask ) ;
			// 消滅関数
			virtual ~EventTask( void ) ;

		public:
			// 識別子取得
			const SSystem::SString& GetID( void ) const
			{
				return	m_strID ;
			}
			// 優先度
			int GetPriority( void ) const
			{
				return	m_nPriority ;
			}
			// タスク処理
			virtual void OnTask( void ) ;
		} ;

	protected:
		// プロファイル
		SGLAppProfile	m_profile ;

		// イベントキュー
		SSystem::SObjectArray<EventTask>	m_queEvent ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SGLStdServiceListener, SObject, SGLServiceListener )
		// 構築関数
		SGLStdServiceListener( void ) ;
		// 消滅関数
		virtual ~SGLStdServiceListener( void ) ;

	protected:
		// ベースファイルパスを取得
		virtual const wchar_t *
			GetBaseFilePath( SSystem::SString& strFileDir ) const ;
		// プロファイルパスを取得
		virtual const wchar_t *
			GetProfileFilePath( SSystem::SString& strFilePath ) const ;

	public:
		// プロファイル保存
		void SaveProfile( void ) ;

	public:
		// イベント追加
		void AddEventTask( EventTask * pEvent ) ;
		// イベントキャンセル
		void RemoveEventTask( EventTask * pEvent ) ;
		void RemoveEventTask( const wchar_t * pwszID ) ;
		// イベント取得
		EventTask * GetEventTask( void ) ;
		// イベント有無判定
		bool IsAnyEventTasks( void ) const ;

	public:
		// サービス初期化処理
		virtual void OnInitializeService( SGLService * service ) ;
		// サービス終了時処理
		virtual void OnFinishService( SGLService * service ) ;
	} ;


}

#endif


