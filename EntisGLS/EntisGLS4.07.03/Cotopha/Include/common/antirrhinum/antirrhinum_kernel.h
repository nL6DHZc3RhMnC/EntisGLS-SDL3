
#if	!defined(__ANTIRRHINUM_KERNEL_H__)
#define	__ANTIRRHINUM_KERNEL_H__

#include <loquaty/gls4_loquaty_def.h>
#include <sakura/ssys_smart_object_array.h>


namespace	AntirrhinumGL
{
	class	AGLKernel ;
	class	AGLEpicProcessor ;
	class	AGLThread ;

	//////////////////////////////////////////////////////////////////////////
	// アンチリナム基底抽象物
	//////////////////////////////////////////////////////////////////////////

	class	AGLObject	: public SakuraGL::SGLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLObject, SGLObject )

	public:	// SGLObject
		// シリアライズ
		virtual SakuraGL::SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SakuraGL::SGLError OnRestore( SSystem::SFileInterface& file ) ;
		// 復元後処理
		virtual SakuraGL::SGLError OnAfterRestore( void ) ;

	public:
		// カーネル取得
		virtual AGLKernel * GetKernel( void ) const = 0 ;
		// シリアライズ
		virtual SSystem::SError Serialize
			( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) = 0 ;
		// デシリアライズ
		virtual SSystem::SError Deserialize
			( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) = 0 ;
		// デシリアライズ後の参照解決処理
		virtual SSystem::SError AfterDeserialize( AGLKernel * pKernel ) = 0 ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 待機処理タイプ
	//////////////////////////////////////////////////////////////////////////

	enum	SynchronismType
	{
		syncTypeTime,			// 時間待ち
		syncTypeMessage,		// メッセージ・クリック待ち
		syncTypeEffect,			// 画面効果完了待ち
		syncTypeEvent,			// イベント発生待ち（サウンドその他同期用）
		syncTypeSystemEffect,	// 非スクリプトによる画面効果
	} ;


	//////////////////////////////////////////////////////////////////////////
	// コード
	//////////////////////////////////////////////////////////////////////////

	enum	CodeProcessResult
	{
		codeUndefined		= -1,
		codeUnprocessed,
		codeProcessed,
		codeControlled,
		codePending,
	} ;

	class	AGLCode	: public SSystem::SXMLDocument
	{
	public:
		// 処理関数
		typedef	CodeProcessResult (*PFUNC_PROCESSOR)
			( AGLEpicProcessor * pEpicProc,
				AGLThread& thread, const AGLCode& code ) ;

	protected:
		AGLKernel *			m_pKernel ;
		PFUNC_PROCESSOR		m_pfnProc ;
		AGLEpicProcessor *	m_pEpicProc ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLCode, SXMLDocument )
		// 構築関数
		AGLCode( void ) ;
		// 実行
		CodeProcessResult ProcessCode( AGLThread& thread ) const ;
		// 関数設定
		void SetEpicProcessor
			( AGLKernel * pKernel,
				AGLEpicProcessor * pEpicProc, PFUNC_PROCESSOR pfnProc ) ;

	public:
		// サブコンテンツ取得
		AGLCode * GetElementAt( size_t index ) const ;
		// SXMLDocument 要素作成
		virtual SXMLDocument * new_XMLDocument( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// モジュール
	//////////////////////////////////////////////////////////////////////////

	class	AGLModule	: public SSystem::SObject
	{
	protected:
		SSystem::SString	m_strFileName ;
		SSystem::SString	m_strFileTitle ;
		atomic_int_t		m_nRefCount ;
		AGLCode				m_xmlDoc ;
		AGLCode *			m_pxmlScript ;
		AGLCode *			m_pxmlCode ;

	public:
		class	CodeArray
		{
		public:
			AGLModule *						m_pModule ;
			AGLCode *						m_pxmlCode ;
			SSystem::SArray<size_t>			m_aNestIndex ;
			SSystem::SStrSortArray<size_t>	m_ssaLabelIndex ;
		public:
			// 構築関数
			CodeArray( void )
				: m_pModule(nullptr), m_pxmlCode(nullptr) { }
			// ラベル検索
			ssize_t FindLabel( const wchar_t * pwszLabel ) const ;
		} ;

	protected:
		SSystem::SPtrSortObjectArray
			<AGLCode,const CodeArray>	m_psoaCodes ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLModule, SObject )
		// 構築関数
		AGLModule( void ) ;
		// 消滅関数
		virtual ~AGLModule( void ) ;
		// 読み込み
		SSystem::SError LoadScript( const wchar_t * pwszFilePath ) ;
		SSystem::SError ReadScript
			( SSystem::SFileInterface& file, const wchar_t * pwszFilePath ) ;
	protected:
		SSystem::SError ParseScript( const wchar_t * pwszFilePath ) ;
		void BuildCodeArray
			( AGLCode * pxmlCode,
				const CodeArray * pParent, size_t nSubIndex ) ;

	public:
		// 参照カウンタ
		atomic_int_t AddRefCount( void ) ;
		atomic_int_t ReleaseRefCount( void ) ;
		// ファイル名
		const SSystem::SString& GetFileName( void ) const
		{
			return	m_strFileName ;
		}
		const SSystem::SString& GetFileTitle( void ) const
		{
			return	m_strFileTitle ;
		}
		// コード取得
		const AGLCode * GetCode( void ) const
		{
			return	m_pxmlCode ;
		}
		const AGLCode *
			GetCodeOf( const SSystem::SString * pstrNestIndex ) const ;
		// CodeArray 取得
		const CodeArray * GetCodeArray( const AGLCode * pxmlCode ) const ;
		const CodeArray * GetRootCodeArray( void ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// モジュール・マネージャ
	//////////////////////////////////////////////////////////////////////////

	class	AGLModuleManager	: public SSystem::SObject
	{
	protected:
		SSystem::SCriticalSection				m_csSync ;
		SSystem::SStrSortObjectArray<AGLModule>	m_ssoaModules ;
		SSystem::SPointerArray<AGLModule>		m_aCacheModules ;
		size_t									m_nCacheLimit ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLModuleManager, SObject )
		// 構築関数
		AGLModuleManager( size_t nCacheCount = 64 ) ;
		// スクリプト読み込み
		virtual AGLModule * LoadScript( const wchar_t * pwszFileName ) ;
		// スクリプト読み込み（既に読み込み済みのものは破棄する）
		virtual AGLModule * ReadScript
			( SSystem::SFileInterface& file, const wchar_t * pwszFileName ) ;
		// 読み込み済みスクリプト取得
		//（取得に成功した場合には UnloadScript 呼び出しで参照解放）
		virtual AGLModule * GetLoadedScript( const wchar_t * pwszFileName ) ;
		// スクリプト解放
		virtual SSystem::SError UnloadScript( AGLModule * pModule ) ;
		void UnloadAllScript( void ) ;
		// 参照されていないスクリプトを破棄する
		void ClearCacheModules( void ) ;
		// マネージャーで管理している有効なモジュールか？
		bool IsValidModule( AGLModule * pModule ) const ;

	public:
		// ファイルを開く
		virtual SSystem::SFileInterface *
					OpenScriptFile( const wchar_t * pwszFileName ) ;
	} ;
	

	//////////////////////////////////////////////////////////////////////////
	//モジュール・ スマートポインタ
	//////////////////////////////////////////////////////////////////////////

	class	AGLModulePtr	: public SSystem::SSyncReference
	{
	protected:
		SSystem::SSmartReference<AGLModuleManager>	m_refManager ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLModulePtr, SSyncReference )
		// 構築関数
		AGLModulePtr
			( AGLModule * pModule = nullptr,
				AGLModuleManager * pManager = nullptr ) ;
		AGLModulePtr( const AGLModulePtr& ptr ) ;
		// 消滅関数
		virtual ~AGLModulePtr( void ) ;
		// アンロード
		void Unload( void ) ;
		// モジュール
		AGLModule * GetModule( void ) const
		{
			return	SSystem::SSyncReference::GetRef<AGLModule>() ;
		}
		AGLModuleManager * GetModuleManager( void ) const
		{
			return	m_refManager.GetReference() ;
		}
		// 代入
		const AGLModulePtr& operator = ( const AGLModulePtr& ptr ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 使用スクリプト言語タイプ
	//////////////////////////////////////////////////////////////////////////

	enum	AGLScriptLanguageType
	{
		languageInvalid	= -1,
		languageRosetta,
		languageLoquaty,
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクリプト・インスタンス
	//////////////////////////////////////////////////////////////////////////

	class	AGLScriptContext ;
	class	AGLScriptObject
	{
	protected:
		AGLScriptLanguageType	m_type ;
		Rosetta::RSSmartPtr		m_pRosetta ;
		Loquaty::LValue			m_valLoquaty ;

	public:
		// 構築関数
		AGLScriptObject( void ) ;
		AGLScriptObject( const AGLScriptObject& src ) ;
		AGLScriptObject( const Rosetta::RSSmartPtr& pRosetta ) ;
		AGLScriptObject( const Loquaty::LValue& value ) ;
		AGLScriptObject( const Loquaty::LObjPtr& pLoquaty ) ;

		// 代入
		const AGLScriptObject& operator = ( const AGLScriptObject& src ) ;
		const AGLScriptObject& operator = ( const Rosetta::RSSmartPtr& pRosetta ) ;
		const AGLScriptObject& operator = ( const Loquaty::LValue& value ) ;
		const AGLScriptObject& operator = ( const Loquaty::LObjPtr& pLoquaty ) ;

		// 言語タイプ取得
		AGLScriptLanguageType GetLanguageType( void ) const
		{
			return	m_type ;
		}

		// null 判定
		bool IsNull( void ) const ;
		// 解放
		void Release( void ) ;

		// RSObject 取得
		const Rosetta::RSSmartPtr& GetRosetta( void ) const
		{
			return	m_pRosetta ;
		}
		operator const Rosetta::RSSmartPtr& ( void ) const
		{
			return	m_pRosetta ;
		}
		operator Rosetta::RSObject * ( void ) const
		{
			return	m_pRosetta ;
		}

		// LObject 取得
		const Loquaty::LValue& GetLoquaty( void ) const ;
		const Loquaty::LObjPtr& GetLoquatyObj( void ) const ;
		operator const Loquaty::LObjPtr& ( void ) const ;
		Loquaty::LClass* GetLoquatyClass( void ) const ;

		// 値を評価
		bool AsBoolean( void ) const ;
		int64_t AsInteger( void ) const ;
		double AsDouble( void ) const ;
		SSystem::SString AsString( void ) const ;

		// 値を設定
		bool PutInteger( int64_t val ) ;
		bool PutDouble( double val ) ;
		bool PutString( const wchar_t * str ) ;

		// 要素取得
		AGLScriptObject GetMemberAs
			( const AGLScriptContext& context, const wchar_t * pwszName ) const ;
		AGLScriptObject GetElementAt
			( const AGLScriptContext& context, size_t index ) const ;
		// 要素数取得
		size_t GetElementCount( void ) const ;
		// 要素名取得
		SSystem::SString GetElementNameAt( size_t index ) const ;

		// 要素設定
		void SetMemberAs
			( const AGLScriptContext& context,
				const wchar_t * pwszName, const AGLScriptObject obj ) ;

		// シリアライズ
		void Serialize
			( AGLScriptContext& context,
				SSystem::SXMLDocument& xmlTag, bool flagStatic ) ;
		// デシリアライズ
		void Deserialize
			( AGLScriptContext& context,
				const SSystem::SXMLDocument& xmlTag, bool flagStatic ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクリプト・コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	AGLScriptContext
	{
	protected:
		AGLScriptLanguageType						m_type ;
		SSystem::SSmartPointer<Rosetta::RSContext>	m_pContext ;
		Loquaty::LPtr<Loquaty::LTaskObj>			m_pLTask ;

	public:
		// 構築関数
		AGLScriptContext( void ) ;
		// 解放
		void Release( void ) ;

		// 言語タイプ取得
		AGLScriptLanguageType GetLanguageType( void ) const
		{
			return	m_type ;
		}

		// RSContext 設定
		void SetRosetta( Rosetta::RSContext * pContext ) ;
		// RSContext 取得
		Rosetta::RSContext * GetRosetta( void ) const ;
		// LTaskObj 設定
		void SetLoquaty( const Loquaty::LPtr<Loquaty::LTaskObj>& pTask ) ;
		// LTaskObj 取得
		Loquaty::LPtr<Loquaty::LTaskObj> GetLoquaty( void ) const ;
		// LVirtualMachine 取得
		Loquaty::LVirtualMachine * GetLoquatyVM( void ) const ;

		// Integer オブジェクト生成
		AGLScriptObject new_Integer( int64_t num = 0 ) const ;
		// String オブジェクト生成
		AGLScriptObject new_String( const wchar_t * str = nullptr ) const ;
		// Map オブジェクト生成
		AGLScriptObject new_Map( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スレッド
	//////////////////////////////////////////////////////////////////////////

	class	AGLThread	: public AGLObject
	{
	public:
		enum	Status
		{
			statusExecute,
			statusSuspend,
			statusHalt,
			statusCount,
		} ;
		enum	ControlFlag
		{
			ctrlThread				= 0x0001,
			ctrlCalled				= 0x0002,
			ctrlLoop				= 0x0004,
			ctrlDisableInterrupt	= 0x0010,
		} ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiStatus[statusCount+1] ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiControlFlags[4] ;

	protected:
		struct	Stack
		{
			const AGLModule::CodeArray *	pCodeArray ;
			size_t							iCode ;
			uint32_t						nCtrl ;
		} ;
		AGLModuleManager *		m_pModuleManager ;
		AGLScriptObject			m_instance ;
		Loquaty::LPtr<Loquaty::LTaskObj>
								m_pLTask ;
		AGLKernel *				m_pKernel ;
		uint32_t				m_idThread ;
		SSystem::SString		m_strName ;
		SSystem::SArray<Stack>	m_stack ;
		Stack					m_ip ;
		Status					m_status ;
		SSystem::STimeCounter	m_timer ;
		atomic_int_t			m_nSuspended ;
		uint32_t				m_maskSkippable ;
		int64_t					m_msecFrameTimeout ;

		SSystem::SObjectArray<AGLCode>	m_aMicroCodes ;
		size_t							m_nQueuedCode ;

		SSystem::SXMLDocument	m_xmlLocalStorage ;

		class	Interrupter
		{
		public:
			SSystem::SString	m_strScript ;
			SSystem::SString	m_strLabel ;
			bool				m_flagCall ;
		public:
			Interrupter
				( const wchar_t * pwszFilePath,
					const wchar_t * pwszLabel, bool flagCall )
				: m_strScript( pwszFilePath ),
					m_strLabel( pwszLabel ), m_flagCall( flagCall ) { }
			Interrupter( const Interrupter& itr )
				: m_strScript( itr.m_strScript ),
					m_strLabel( itr.m_strLabel ), m_flagCall( itr.m_flagCall ) { }
		} ;
		SSystem::SObjectArray<Interrupter>	m_aInterrupter ;
		SSystem::SCriticalSection			m_csSync ;

		SSystem::SStrSortObjectArray<Loquaty::LInstantEvaluator>
											m_ssoaExprEvals ;
		SSystem::SPtrSortObjectArray<wchar_t,Loquaty::LInstantEvaluator>
											m_psoaEvaluators ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLThread, AGLObject )
		// 構築関数
		AGLThread( AGLModuleManager * pModuleManager,
					const AGLScriptObject& instance = AGLScriptObject() ) ;
		// 消滅関数
		virtual ~AGLThread( void ) ;
		// Module, ModuleManager 解除
		// ※デストラクタでの UnloadScript 呼び出し抑制
		void DetachModule( void ) ;
		// ModuleManager 取得
		AGLModuleManager * GetModuleManager( void ) ;
		// 現在のコード配列を取得する
		const AGLModule::CodeArray * GetCurrentCodeArray( void ) const ;
		// 現在のスクリプトモジュールを取得する
		AGLModule * GetCurrentModule( void ) const ;
		const wchar_t * GetCurrentModuleFileTitle( void ) const ;
		const wchar_t * GetCurrentModuleFile( void ) const ;
		// インスタンス
		const AGLScriptObject& GetInstance( void ) const ;
		// スレッドID取得
		uint32_t GetThreadID( void ) const ;
		// スレッド名取得
		const SSystem::SString& GetThreadName( void ) const ;
		// スレッド名設定
		void SetThreadName( const wchar_t * pwszName ) ;
		// AGLKernel 取得
		virtual AGLKernel * GetKernel( void ) const ;

	public:
		// ステータスを取得する
		virtual Status GetStatus( void ) const ;
		// 一時停止
		virtual void Suspend( void ) ;
		// 再開
		virtual void Resume( void ) ;
		// シリアライズ可能状態か？
		virtual bool CanSerialize( void ) const ;
		// スレッド実行時間 [ms]
		virtual int64_t GetThreadTick( void ) const ;
		// スレッド実行フレームタイムアウト時間 [ms]
		int64_t GetFrameTimeot( void ) const ;
		void SetFrameTimeout( int64_t msecTimeout ) ;
		// 次のコードを取得する
		virtual AGLCode * GetNextCode( void ) ;
		// 命令ポインタを進める
		virtual void NextCodeIndex( void ) ;
		// 割り込みでの制御移行
		virtual bool FetchInterruption( void ) ;
		// 命令ポインタを変更する
		virtual SSystem::SError JumpCodeLabel
				( const wchar_t * pwszLabel, uint32_t nCtrlFlags = 0 ) ;
		// 現在の命令ポインタをスタックにプッシュする
		virtual SSystem::SError PushCodeIndex( size_t iCodeOffset = 1 ) ;
		// 現在の命令ポインタをスタックからポップする
		virtual SSystem::SError PopCodeIndex( void ) ;
		virtual SSystem::SError PopCodeIndexUntil( uint32_t nCtrlFlag ) ;
		// 現在の命令ポインタを設定する
		virtual SSystem::SError JumpCodeIndex
			( const AGLModule::CodeArray * pCodeArray,
						size_t nIndex, uint32_t nCtrlFlags = 0 ) ;
		virtual SSystem::SError JumpCodeScript
			( const wchar_t * pwszFilePath,
				const wchar_t * pwszLabel, uint32_t nCtrlFlags = 0 ) ;

	public:
		// Loquaty スクリプトを利用するか？
		bool IsUsingLoquaty( void ) const ;
		// Loquaty 式評価コンテキスト取得
		Loquaty::LInstantEvaluator&
			GetLoquatyExprEvaluator( const wchar_t * pwszExpr ) ;
		// Loquaty 文評価コンテキスト取得
		Loquaty::LInstantEvaluator&
			GetLoquatyStatementsEvaluator( const wchar_t * pwszStatements ) ;
		// Loquaty インスタンス・クラス
		Loquaty::LClass * GetLoquatyInstanceClass( void ) const ;
		// Loquaty インスタンス取得
		const Loquaty::LObjPtr& GetLoquatyInstance( void ) const ;
		// 実行用 LTask 取得
		const Loquaty::LPtr<Loquaty::LTaskObj>& GetLoquatyTask( void ) ;

	public:
		// 待機命令をスキップ可能か？
		virtual bool IsPermittedSkip( SynchronismType type ) const ;
		// スキップ許可
		virtual void PermitSkip( SynchronismType type ) ;
		virtual void PermitSkipMask( uint32_t mask ) ;
		// スキップ不許可
		virtual void ProhibitSkip( SynchronismType type ) ;
		virtual void ProhibitSkipMask( uint32_t mask ) ;

	public:
		// 遅延ジャンプ（割り込み処理）
		virtual SSystem::SError PostInterrupter
			( const wchar_t * pwszFilePath,
				const wchar_t * pwszLabel, bool flagCall ) ;
		// 割り込み禁止フラグ設定
		virtual void SetDisableInterruptFlag( bool flag ) ;

	public:
		// コマンド処理用のローカル記憶域（タグの取得／ない場合は作成）
		SSystem::SXMLDocument * GetLocalStrageAs( const wchar_t * pwszTag ) ;
		// コマンド処理用のローカル記憶域削除
		void ClearLocalStrageAs( const wchar_t * pwszTag ) ;
		void ClearLocalStrage( SSystem::SXMLDocument * pxmlStorage ) ;
		void ClearAllLocalStrage( void ) ;

	public:
		// マイクロコードを追加する
		virtual void AddMicroCode( AGLCode * pCode ) ;
		void AddMicroCodeTag( const wchar_t * pwszTag ) ;
		void AddMicroCodeTagParam1
			( const wchar_t * pwszTag,
				const wchar_t * pwszAttr1, const wchar_t * pwszValue1 ) ;
		void AddMicroCodeTagParam2
			( const wchar_t * pwszTag,
				const wchar_t * pwszAttr1, const wchar_t * pwszValue1,
				const wchar_t * pwszAttr2, const wchar_t * pwszValue2 ) ;
		void AddMicroCodeTagIntParam1
			( const wchar_t * pwszTag,
				const wchar_t * pwszAttr1, int64_t nValue1 ) ;
		void AddMicroCodeTagIntParam2
			( const wchar_t * pwszTag,
				const wchar_t * pwszAttr1, int64_t nValue1,
				const wchar_t * pwszAttr2, int64_t nValue2 ) ;
		// マイクロコード削除
		virtual void RemoveAllMicroCode( void ) ;

	public:	// AGLObject
		// シリアライズ
		virtual SSystem::SError Serialize
				( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ
		virtual SSystem::SError Deserialize
				( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ後の参照解決処理
		virtual SSystem::SError AfterDeserialize( AGLKernel * pKernel ) ;

	protected:
		void SerializeStack
				( SSystem::SXMLDocument& xmlTag, const Stack& stack ) ;
		void DeserializeStack
				( Stack& stack, const SSystem::SXMLDocument& xmlTag ) ;

		friend AGLKernel ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクリプト・エンジン・基底抽象物
	//////////////////////////////////////////////////////////////////////////

	class	AGLEpicProcessor	: public AGLObject
	{
	protected:
		AGLKernel *	m_pKernel ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLEpicProcessor, AGLObject )
		// 構築関数
		AGLEpicProcessor( void ) ;
		// 識別子
		virtual const wchar_t * GetObjectType( void ) const = 0 ;
		// 設定
		virtual void LoadConfiguration
			( const SSystem::SXMLDocument& xmlConfig ) = 0 ;
		// コード処理関数取得
		virtual AGLCode::PFUNC_PROCESSOR
				GetCodeProcesser( const wchar_t * pwszTag ) = 0 ;
		// タイマ処理 (実行フレーム前処理)
		virtual void OnKernelTimer( void ) = 0 ;
		// ゲーム開始時処理
		virtual void InitializeGame( void ) ;
		// ゲーム終了前フェードアウト処理
		virtual void FadeoutGame( uint32_t msecFadeout ) ;
		// ゲーム終了時処理
		virtual void ReleaseGame( void ) ;
		// 待機関数を（ユーザー入力等により）即時に脱出すべきか判定する
		virtual bool ShouldAbortSync( SynchronismType type ) ;
		// 待機関数を ShouldAbortSync を理由に脱出したことの通知
		virtual void NotifyAbortedSync( SynchronismType type ) ;
		// フェード処理などの効果継続時間の効果
		virtual uint32_t EffectTime
			( uint32_t msecTime, SynchronismType type = syncTypeEffect ) ;

	public:
		// AGLKernel 関連付け
		virtual void AttachKernel( AGLKernel * pKernel ) ;
		virtual AGLKernel * GetKernel( void ) const ;
		// リリース時処理
		virtual void OnReleaseKernel( void ) ;

	public:
		// 数式評価
		SSystem::SString EvaluateStringExpression
			( AGLThread& thread,
				const wchar_t * pwszExpr, const wchar_t * pwszDefault = nullptr ) ;
		int64_t EvaluateIntExpression
			( AGLThread& thread,
				const wchar_t * pwszExpr, int64_t nDefault = 0 ) ;
		double EvaluateNumberExpression
			( AGLThread& thread,
				const wchar_t * pwszExpr, double nDefault = 0.0 ) ;
		bool EvaluateBoolExpression
			( AGLThread& thread,
				const wchar_t * pwszExpr, bool bDefault = false ) ;
		// 数式評価（code に一つの条件判定式を高速化）
		bool EvaluateBoolExpression
			( AGLThread& thread, const AGLCode& code,
				const wchar_t * pwszExpr, bool bDefault = false ) ;
		// 数式評価（オブジェクト）
		AGLScriptObject EvaluateExpression
			( AGLThread& thread, const wchar_t * pwszExpr, bool flagRef = false ) ;
		// 文実行
		void PerformStatements
			( AGLThread& thread,
				AGLCode& code, const wchar_t * pwszStatements ) ;
		// 文字列内式展開
		SSystem::SString EvaluateExprInText
			( AGLThread& thread, const wchar_t * pwszText ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクリプト・エンジン・関数プロセッサ
	//////////////////////////////////////////////////////////////////////////

	class	AGLEpicFuncProcessor	: public AGLEpicProcessor
	{
	public:
		struct	EpicFuncDescriptor
		{
			const wchar_t *				pwszCmd ;
			AGLCode::PFUNC_PROCESSOR	pfnProc ;
			const EpicFuncDescriptor *	pDescNext ;
		} ;

	protected:
		SSystem::SIndexedArray<SSystem::SString,const wchar_t*>	m_iaCmdMap ;
		SSystem::SPointerArray<const EpicFuncDescriptor>		m_aFuncDesc ;

		SSystem::SString	m_strTypeID ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLEpicFuncProcessor, AGLEpicProcessor )
		// 構築関数
		AGLEpicFuncProcessor
			( const EpicFuncDescriptor * pDesc, const wchar_t * pwszType ) ;
		// 消滅関数
		virtual ~AGLEpicFuncProcessor( void ) ;

	public:
		// 識別子
		virtual const wchar_t * GetObjectType( void ) const ;
		// 設定
		virtual void LoadConfiguration
			( const SSystem::SXMLDocument& xmlConfig ) ;
		// コード処理関数取得
		virtual AGLCode::PFUNC_PROCESSOR
				GetCodeProcesser( const wchar_t * pwszTag ) ;
		// タイマ処理 (実行フレーム前処理)
		virtual void OnKernelTimer( void ) ;
	} ;

	#define	AGL_DECLARE_EPIC_PROCESSOR( cls_name )	\
		protected: static const EpicFuncDescriptor *	m_pFirstFuncDesc ;	\
		public: static const EpicFuncDescriptor *		\
					AddEpicFuncDescriptor( const EpicFuncDescriptor * pDesc ) ;

	#define	AGL_IMPLEMENT_EPIC_PROCESSOR( cls_name )	\
		const AGLEpicFuncProcessor::EpicFuncDescriptor *	\
								cls_name::m_pFirstFuncDesc = nullptr ;	\
		const AGLEpicFuncProcessor::EpicFuncDescriptor *		\
			cls_name::AddEpicFuncDescriptor	\
				( const AGLEpicFuncProcessor::EpicFuncDescriptor * pDesc )	\
		{	\
			const EpicFuncDescriptor *	pNext = m_pFirstFuncDesc ;	\
			m_pFirstFuncDesc = pDesc ;	\
			return	pNext ;	\
		}

	#define	DECL_ANTIRRHINUM_PROC( cls_name, cmd_name )	\
		static const EpicFuncDescriptor	desc_ProcessCode_##cmd_name ;	\
		static CodeProcessResult stub_ProcessCode_##cmd_name	\
			( AGLEpicProcessor * pEpicProc, AGLThread& thread, const AGLCode& code ) ;	\
		CodeProcessResult ProcessCode_##cmd_name( AGLThread& thread, const AGLCode& code ) ;

	#define	IMPL_ANTIRRHINUM_PROC( cls_name, cmd_name )	\
		const AGLEpicFuncProcessor::EpicFuncDescriptor	\
			cls_name::desc_ProcessCode_##cmd_name =	\
			{	\
				L###cmd_name,	\
				&cls_name::stub_ProcessCode_##cmd_name,	\
				cls_name::AddEpicFuncDescriptor	\
						( &cls_name::desc_ProcessCode_##cmd_name ),	\
			} ;	\
		AntirrhinumGL::CodeProcessResult cls_name::stub_ProcessCode_##cmd_name	\
			( AGLEpicProcessor * pEpicProc, AGLThread& thread, const AGLCode& code )	\
			{	\
				cls_name *	pProc = ESLTypeCast<cls_name>( pEpicProc ) ;	\
				ESLAssert( pProc != nullptr ) ;	\
				return	pProc->ProcessCode_##cmd_name( thread,code ) ;	\
			}	\
		AntirrhinumGL::CodeProcessResult \
			cls_name::ProcessCode_##cmd_name( AGLThread& thread, const AGLCode& code )


	//////////////////////////////////////////////////////////////////////////
	// スクリプト・フロー制御プロセッサ
	//////////////////////////////////////////////////////////////////////////

	class	AGLEpicCoreProcessor	: public AGLEpicFuncProcessor
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( AGLEpicCoreProcessor, AGLEpicFuncProcessor )
		AGL_DECLARE_EPIC_PROCESSOR( AGLEpicCoreProcessor )
		// 構築関数
		AGLEpicCoreProcessor( void ) ;
		// 消滅関数
		virtual ~AGLEpicCoreProcessor( void ) ;

	protected:
		// ブロック内部へ入る
		CodeProcessResult EnterBlock
			( AGLThread& thread, const AGLCode& code,
				size_t iBreakOffset, uint32_t nCtrlFlags ) ;

	public:	// AGLObject
		// シリアライズ
		virtual SSystem::SError Serialize
				( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ
		virtual SSystem::SError Deserialize
				( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ後の参照解決処理
		virtual SSystem::SError AfterDeserialize( AGLKernel * pKernel ) ;

	public:
		// コマンド実装
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,jump)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,call)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,return)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,if)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,elseif)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,else)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,while)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,break)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,continue)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,thread)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,terminate)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,eval)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,fwait)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,wait)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,wait_for)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,start_timer)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,wait_timer)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,permit_skip)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,prohibit_skip)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,dis_interrupt)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,trace)
		DECL_ANTIRRHINUM_PROC(AGLEpicCoreProcessor,nop)

	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクリプト・エンジン・カーネル
	//////////////////////////////////////////////////////////////////////////

	class	AGLKernel	: public AGLObject
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( AGLKernel, AGLObject )
		// 構築関数
		AGLKernel( void ) ;
		// 消滅関数
		virtual ~AGLKernel( void ) ;

	public:
		enum	ThreadSerializeMode
		{
			tsmodeSerializeAllThreads,		// 全てのスレッドをシリアライズする
			tsmodeNoSerializeAnyThreads,	// いかなるスレッドもシリアライズしない
		} ;

	protected:
		// Rosetta 仮想マシン
		Rosetta::RSVirtualMachine *					m_pVM ;

		// Loquaty 仮想マシン
		Loquaty::LVirtualMachine *					m_pLVM ;

		// スクリプト実行コンテキスト
		AGLScriptContext							m_context ;

		// デフォルトのインスタンス
		AGLScriptObject								m_pDefThisObj ;

		// エラー出力先
		SSystem::SParserErrorInterface *			m_pErrorTracer ;

		// モジュール・マネージャ
		AGLModuleManager *							m_pModuleManager ;

		// エピックプロセッサ
		SSystem::SPointerArray<AGLEpicProcessor>	m_aEpicProc ;

		// スレッド
		SSystem::SSmartObjectArray<AGLThread>		m_threads ;
		SSystem::SObjectArray<AGLThread>			m_threadTrash ;
		ThreadSerializeMode							m_tsmodeSerializeThread ;
		uint32_t									m_idNextThread ;

		SSystem::SCriticalSection					m_csSync ;

		SSystem::STimeCounter						m_timerExec ;
		atomic_int_t								m_nSuspended ;

		struct	EpicCodeProcessor
		{
			AGLEpicProcessor *			pEpicProc ;
			AGLCode::PFUNC_PROCESSOR	pfnProc ;
		} ;
		SSystem::SIndexedArray
			<SSystem::SString,const wchar_t*>	m_iaCmdMap ;
		SSystem::SArray<EpicCodeProcessor>		m_aFuncDesc ;

	public:
		// RSVirtualMachine 関連付け
		void AttachRosettaVM( Rosetta::RSVirtualMachine * pVM ) ;
		Rosetta::RSVirtualMachine * GetRosettaVM( void ) const ;
		// LVirtualMachine 関連付け
		void AttachLoquatyVM( Loquaty::LVirtualMachine * pVM ) ;
		Loquaty::LVirtualMachine * GetLoquatyVM( void ) const ;
		// スクリプト実行コンテキスト
		const AGLScriptContext& GetContext( void ) const ;
		Rosetta::RSContext * GetRSContext( void ) const ;
		// Loquaty スクリプトを利用するか？
		bool IsUsingLoquaty( void ) const ;
		// デフォルト this オブジェクト設定
		void SetDefaultThisObject( const AGLScriptObject& pThisObj ) ;
		const AGLScriptObject& GetDefaultThisObject( void ) const ;
		// SParserErrorInterface 関連付け
		void AttachErrorTracer( SSystem::SParserErrorInterface * pErrorTracer ) ;
		// AGLModuleManager 関連付け
		void AttachModuleManager( AGLModuleManager * pModuleManager ) ;
		AGLModuleManager * GetModuleManager( void ) const ;
		// AGLEpicProcessor 追加
		void AttachEpicProcessor( AGLEpicProcessor * pEpicProc ) ;
		// AGLEpicProcessor 分離
		void DetachEpicProcessor( AGLEpicProcessor * pEpicProc ) ;
		// AGLEpicProcessor 取得
		AGLEpicProcessor * GetEpicProcessor( const ESLRuntimeClass& rtClass ) const ;
		typedef size_t	EpicProcessorIterator ;
		EpicProcessorIterator FirstEpicProcessor( void ) const ;
		AGLEpicProcessor * NextEpicProcessor( const ESLRuntimeClass& rtClass, EpicProcessorIterator& iNext ) const ;
		template <class T> T * GetEpicProcessor( void ) const
		{
			return	ESLTypeCast<T>( GetEpicProcessor( ESL_RUNTIME_CLASS(T) ) ) ;
		}
		template <class T> T * NextEpicProcessor( EpicProcessorIterator& iter ) const
		{
			return	ESLTypeCast<T>( NextEpicProcessor( ESL_RUNTIME_CLASS(T), iter ) ) ;
		}
		// 設定
		void LoadConfiguration( const SSystem::SXMLDocument& xmlConfig ) ;
		// 関連付け解除
		virtual void ReleaseKernel( void ) ;

	public:
		// スクリプトコード実行
		AGLThread::Status Execute( void ) ;
		AGLThread::Status ExecuteThread( AGLThread& thread ) ;
		CodeProcessResult ExecuteCode
					( AGLThread& thread, AGLCode& code ) ;
		// 全スレッド一時停止
		virtual void SuspendAllThreads( void ) ;
		// 全スレッド再開
		virtual void ResumeAllThreads( void ) ;
		// スレッド開始
		virtual SSystem::SSmartRef<AGLThread> BeginThread
			( const wchar_t * pwszModuleFile,
				const wchar_t * pwszLabel = nullptr,
				const AGLScriptObject& instance = AGLScriptObject(),
				const wchar_t * pwszName = nullptr ) ;
		virtual SSystem::SError BeginThread( AGLThread * pThread ) ;
		// スレッド取得
		virtual AGLThread * GetThreadByID( uint32_t idThread ) const ;
		virtual AGLThread * GetThreadByName( const wchar_t * pwszName ) const ;
		// スレッド生存確認
		bool IsValidThread( AGLThread * pThread ) const ;
		// スレッドの強制終了
		void TerminateThread( AGLThread * pThread ) ;
		// 全スレッドの強制終了
		void TerminateAllThreads( void ) ;
		// AGLModuleManager に関連するすべてのスレッドを強制終了する
		void TerminateAllThreadsOf( AGLModuleManager * pManager ) ;
		// スレッドのシリアライズモードを取得
		ThreadSerializeMode GetThreadSerializeMode( void ) const ;
		// スレッドのシリアライズモードを設定
		void SetThreadSerializeMode( ThreadSerializeMode tsmode ) ;

	public:
		// ゲーム開始時処理
		virtual void InitializeGame( void ) ;
		// ゲーム終了前フェードアウト処理
		virtual void FadeoutGame( uint32_t msecFadeout ) ;
		// ゲーム終了時処理
		virtual void ReleaseGame( void ) ;
		// 待機関数を（ユーザー入力等により）即時に脱出すべきか判定する
		virtual bool ShouldAbortSync( SynchronismType type ) ;
		// 待機関数を ShouldAbortSync を理由に脱出したことの通知
		virtual void NotifyAbortedSync( SynchronismType type ) ;
		// 効果時間の修正処理
		virtual uint32_t EffectTime
			( uint32_t msecTime, SynchronismType type = syncTypeEffect ) ;

	protected:
		// Rosetta 文コンパイル
		SSystem::SError CompileRosettaStatements
			( Rosetta::RSScript& script, const SSystem::SString& strSrc ) ;
		// Rosetta 文実行
		void PerformRosettaStatements
			( Rosetta::RSScript& script, Rosetta::RSObject * pThisObj = nullptr ) ;
		// Rosetta 数式評価
		Rosetta::RSSmartPtr EvaluateRosettaExpression
			( Rosetta::RSScript& script, Rosetta::RSObject * pThisObj = nullptr ) ;
		Rosetta::RSSmartPtr EvaluateRosettaExpression
			( const wchar_t * pwszExpr, Rosetta::RSObject * pThisObj = nullptr ) ;

	public:
		// 数式評価
		AGLScriptObject EvaluateExpression
			( const wchar_t * pwszExpr,
				AGLScriptObject objThis = AGLScriptObject(), bool flagRef = false ) ;
		// 文字列内式展開
		SSystem::SString EvaluateExprInText
			( const wchar_t * pwszText, const AGLScriptObject& instance ) ;

	public:
		// デバッグ出力
		virtual void OutputTrace( const wchar_t * pwszTrace ) ;
		// Loquaty の例外エラーをデバッグ出力する
		virtual void TraceException( Loquaty::LObjPtr pException ) ;

	public:
		// シリアライズ可能状態か？
		virtual bool CanSerialize( void ) const ;

	public:	// AGLObject
		// カーネル取得
		virtual AGLKernel * GetKernel( void ) const ;
		// シリアライズ
		SSystem::SError Serialize( SSystem::SXMLDocument& xmlTag ) ;
		virtual SSystem::SError Serialize
				( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ
		SSystem::SError Deserialize( const SSystem::SXMLDocument& xmlTag ) ;
		virtual SSystem::SError Deserialize
				( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ後の参照解決処理
		SSystem::SError AfterDeserialize( void ) ;
		virtual SSystem::SError AfterDeserialize( AGLKernel * pKernel ) ;

		friend class AGLEpicProcessor ;
	} ;

}


#endif


