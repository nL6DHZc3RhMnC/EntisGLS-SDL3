
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script コンパイラ
//////////////////////////////////////////////////////////////////////////////

class	ECSCompiler	: public	ESLObject
{
public:
	// 構築関数
	ECSCompiler( void ) ;
	// 消滅関数
	virtual ~ECSCompiler( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSCompiler, ESLObject )

public:
	// バージョン 3.1019
	enum	Version
	{
		versionMajor	= 0x0003,
		versionMinor	= 0x1019,
	} ;
	// 文の種類
	enum	StatementType
	{
		stReservedWord,		// 予約語文
		stExpression,		// 式文
		stEnumeration,		// 列挙子文
		stAssembler1,		// インラインアセンブラ (1st pass)
		stAssembler2,		// インラインアセンブラ (2nd pass)
	} ;
	// 予約語
	enum	ReservedWord
	{
		rwInvalid	= -1,
		rwInclude,		rwOption,	rwDeclareType,
		rwDeclareDef,	rwExternDef,	rwTypeDef,
		rwVariable,		rwConstant,
		rwData,			rwEndData,
		rwEnumerator,	rwEndEnum,
		rwStructure,	rwEndStruct,
		rwClass,	rwEndClass,
		rwNamespace,	rwEndNamespace,
		rwUnion,	rwEndUnion,
		rwAssembler,	rwEndAssembler,
		rwPublic,	rwProtected,	rwPrivate,
		rwPrototype,	rwFunction,		rwEndFunc,
		rwIf,	rwElseIf,	rwElse,	rwEndIf,
		rwBegin,	rwEnd,
		rwBreak,	rwContinue,	rwReturn,
		rwGoto,		rwLabel,
		rwTry,		rwCatch,	rwEndTry,	rwThrow,
		rwFor,		rwNext,
		rwWhile,	rwEndWhile,
		rwRepeat,	rwUntil,
		rwSwitch,	rwEndSwitch,
		rwCase,	rwDefault,
		rwMemoryFence,
		rwTemplate,	rwEndTemplate,
		rwUsing, rwFriend,
		rwMax,
		rwLoopFirst = rwFor,
		rwLoopLast = rwEndSwitch
	} ;
	// マクロ文予約語
	enum	MacroWord
	{
		mwInvalid	= -1,
		mwError,	mwWarning,	mwCompile,
		mwIf,		mwElseIf,	mwElse,		mwEndIf,
		mwLet,		mwLocal,	mwFor,		mwNext,
		mwLiteral,
		mwDefMacro,	mwEndMacro,	mwExitMacro,	mwUndefMacro,
		mwMax,
		mwMacro = mwMax,
	} ;
	// マクロモード
	enum	MacroMode
	{
		mmodeEnableUserMacro	= 0x01,
		mmodeDisableComment		= 0x02,
		mmodePriorityTextMacro	= 0x04,
		mmodeDisableTextMacro	= 0x08,
	} ;
	// 演算子のタイプ
	enum	OperatorType
	{
		optMove,
		optMember,			// .
		optPtrMember,		// ->
		optMemberFunc,		// .*
		optPtrMemberFunc,	// ->*
		optReference,		// []
		optArgument,		// ()
		optNew,				// new
		optCompare,	optUnary,
		optCompareSelector,	// ? :
		optListExpression,	// ,
		optExtraOperator,
		optExtraUniary,
		optGeneral
	} ;
	enum	CompareSelectorType
	{
		cstEvaluation,
		cstSeparator,
	} ;
	// 演算子の情報構造体
	struct	OPERATOR_INFO
	{
		OperatorType			opiType ;
		CSOperatorType			optOperator ;
		CSUnaryOperatorType		uoptUnary ;
		CSCompareType			cptCompare ;
		CompareSelectorType		cstSelector ;
		CSExtraOperatorType		xoptExOperator ;
		CSExtraUniOperatorType	xuoptExUnary ;
	} ;
	// 演算子の優先度
	enum	OperatorPriority
	{
		oppNothing		= 0,
		oppList			= 1,
		oppMove			= 2,
		oppExOperator	= 2,
		oppCmpSelector	= 3,
		oppLOr			= 4,
		oppLAnd			= 5,
		oppCompare		= 6,
		oppShift		= 7,
		oppXor			= 8,
		oppOr			= 9,
		oppAnd			= 10,
		oppAdd			= 12,
		oppMul			= 13,
		oppUnary		= 14,
		oppExUnary		= 14,
		oppNew			= 14,
		oppCast			= 14,
		oppMember		= 15,
		oppArgument		= 16,
	} ;
	// テンプレート種別
	enum	TemplateType
	{
		templateFunction,
		templateClass,
		templateStruct,
	} ;
	// テンプレート引数種別
	enum	TemplateArgumentType
	{
		templateClassArgument,
		templateIntArgument,
	} ;
	// テンプレート・ステートメント・エントリ
	class	ETemplateStatement
	{
	public:
		StatementType	m_stType ;
		ReservedWord	m_rwType ;
		EWideString		m_wstrStatement ;
		EString			m_strFilePath ;
		int				m_nLineNum ;
	} ;
	// テンプレート・引数
	class	ETemplateArgument
	{
	public:
		TemplateArgumentType	m_type ;
		EWideString				m_name ;
	} ;
	// ステートメント・キャッシュ
	class	EStatementCache
	{
	public:
		EObjArray<ETemplateStatement>	m_statements ;
		ReservedWord					m_rwEndOfImplementation ;
		EStatementCache *				m_pPrevStatementCache ;
	} ;
	// テンプレート定義
	class	ETemplateDefinition	: public EStatementCache
	{
	public:
		TemplateType					m_type ;
		EWideString						m_namespace ;
		EObjArray<ETemplateArgument>	m_arguments ;
	} ;
	// ラベルデータ
	class	ELabelEntries
	{
	public:
		DWORD				m_dwAddr ;				// ラベルアドレス
		ENumArray<DWORD>	m_lstRef ;				// 参照アドレス
	} ;
	// マクロ制御ネスト
	class	EMacroNest
	{
	public:
		enum	Condition
		{
			condNormal,			// 通常コンパイル（@If 条件一致）
			condNext,			// @If に条件が一致していない
			condPass,			// @EndIf までコンパイルしない
		} ;
		MacroWord			m_mwType ;			// マクロネストタイプ
		bool				m_fMacroFunction ;	// マクロ関数
		bool				m_fMacroStatement ;	// マクロ文
		int					m_nCondition ;		// If 文の状態
		int					m_nNestCounter ;	// For 文脱出用カウンタ
		ETagSortArray
			<ECSWideString,ECSObject>
							m_staLocal ;		// マクロ変数
		EObjArray<ECSSourceStream>
							m_lstCodeBuf ;		// マクロコード

		// @Macro 文用
		ECSWideString		m_wstrMacroName ;	// マクロ名
		ECSWideString		m_wstrMacroUsage ;	// マクロ書式
		EObjArray<EWideString>
							m_lstMacroArgName ;	// マクロ関数引数名

		// @For 文用
		ECSWideString		m_wstrElementVar ;	// 参照要素変数名
		ECSWideString		m_wstrArrayExpr ;	// 配列式
		ECSWideString		m_wstrLoopVar ;		// 反復変数名
		ECSWideString		m_wstrFrom ;		// 開始値
		ECSWideString		m_wstrTo ;			// 終了値
		ECSWideString		m_wstrWhile ;		// 反復条件
		ECSWideString		m_wstrStep ;		// 反復処理

	public:
		EMacroNest( MacroWord mwType = mwMacro )
			: m_mwType( mwType ), m_fMacroFunction( false ),
				m_fMacroStatement( false ),
				m_nCondition( condNormal ), m_nNestCounter( 0 ) { }
	} ;
	// マクロブロック
	class	EMacroBlock	: public	EObjArray<ECSSourceStream>
	{
	public:
		bool						m_fMacroFunc ;	// マクロ関数フラグ
		EWideString					m_wstrName ;	// マクロ名
		EStreamWideString::EUsage	m_usage ;		// マクロ書式
		EObjArray<EWideString>		m_lstArgName ;	// マクロ関数引数名
	public:
		EMacroBlock( void ) : m_fMacroFunc( false ) { }
		virtual ~EMacroBlock( void ) { }
	} ;
	// クラス・インライン関数２パス処理用
	class	EDelayImplementation	: public EStatementCache
	{
	public:
		EWideString		m_wstrName ;
	} ;
	// 実装フラグ
	enum	ImplementFlag
	{
		implementAll			= -1,
		implementDeclaration	= 0x01,
		implementFunction		= 0x02,
	} ;
	// 制御ネスト
	class	EControlNest
	{
	public:
		ReservedWord			m_rwType ;			// 制御ネストタイプ
		EWideString				m_wstrName ;		// ネスト名
		DWORD					m_dwLastImplFlags ;	// 以前の m_dwImplementFlags
		bool					m_fAutoEnter ;		// 変数有無で Enter 命令発行
		bool					m_fGotoOccured ;	// Goto 文が出現した
		bool					m_fReturned ;		// Return 文が出現した
		bool					m_fEachReturned ;	// 以前の制御ネストで Return 文が出現した
		long int				m_nArgCount ;		// 引数の数
		//
		DWORD					m_dwBeginPos ;		// ネスト開始アドレス
		ENumArray<DWORD>		m_lstBreak ;		// 脱出アドレス配列
		EObjArray<ECSTypeInfo>	m_lstLocalObj ;		// ローカル変数配列
		ECSStrBufTagArray		m_lstLocalName ;	// ローカル変数名配列
		//
		ETagSortArray<ECSWideString,ELabelEntries>
								m_staLabel ;		// ラベルデータ
		//
		ECSTypeInfo				m_typeSwitch ;		// Switch 評価型
		EObjArray<ECSObject>	m_lstCaseValues ;	// Case 値
		ENumArray<DWORD>		m_lstCaseAddress ;	// Case 開始アドレス
		//
		INT64					m_nEnumeratorValue ;// 列挙子の次の値
		//
		bool					m_fCommitBlock ;
		EObjArray<EDelayImplementation>
								m_lstDelayImplement ;
		ECSAssembler *			m_pAsm ;
		//
		EObjArray<EWideString>	m_lstPostImplTemplate ;	// 後で実装を行うテンプレートインスタンス
		//
		EWStrTagArray<ECSTypeInfo>
								m_wstaTypeDef ;		// ローカル型定義（テンプレート引数）
		EWStrTagArray<ECSClassInfo>
								m_wstaClassDef ;	// ローカル型定義
		ETagSortArray
			<ECSWideString,ECSObject>
								m_staConstant ;		// マクロ変数（テンプレート引数）
		//
		EWideString				m_wstrThisSpaceName ;// this 名前空間名
		EWideString				m_wstrCurSpaceName ;// 現在の名前空間名
		DWORD					m_dwProtectedScope ;// メンバスコープ
		ECSPrototypeInfo *		m_pFuncPrototype ;	// 関数のプロトタイプ
		bool					m_flagNakedMode ;	// naked 関数
		DWORD					m_addrLocalFrame ;	// ローカルフレーム確保命令の即値アドレス
		DWORD					m_maxLocalSize ;	// ローカルフレームの最大サイズ
		SDWORD					m_baseLocalFrame ;	// ローカルフレームの開始アドレス
		DWORD					m_nLocalSize ;		// ローカルフレームサイズ
		ECSExecutionImageCompiler::RegisterContext
								m_regContext ;		// レジスタコンテキスト

		EWideString				m_wstrExceptionHandler ;	// try の例外ハンドラ関数名
		EWideString				m_wstrCatchEnterLabel ;		// cacth の進入ラベル
		EWideString				m_wstrTryExitLabel ;		// try の脱出ラベル
		ECSExecutionImageCompiler *
								m_pcsxiExceptionHandler ;	// try の例外ハンドラ関数
		DWORD					m_dwBiasTryLocalSize ;
		int						m_nCatchCount ;
		EObjArray<EWideString>	m_lstThrows ;				// throw された型リスト

		EObjArray<EWideString>	m_lstUsingNamespace ;

	public:
		EControlNest( DWORD dwImpleFlags )
			: m_dwLastImplFlags(dwImpleFlags),
				m_fAutoEnter(false), m_fGotoOccured(false),
				m_fReturned(false), m_fEachReturned(false),
				m_nArgCount(0), m_dwBeginPos(0),
				m_fCommitBlock(false), m_pAsm(NULL),
				m_dwProtectedScope(0), m_pFuncPrototype(NULL),
				m_flagNakedMode(false),
				m_baseLocalFrame(0), m_nLocalSize(0), m_maxLocalSize(0),
				m_nEnumeratorValue(0),
				m_pcsxiExceptionHandler(NULL),
				m_dwBiasTryLocalSize(0), m_nCatchCount(0) { }
		~EControlNest( void )
			{
				delete	m_pAsm ;
				m_pAsm = NULL ;
			}
		int ShuldBreakLocalBlock( bool fBreak = true ) const
			{
				return	(int) ((m_rwType == rwFor) && fBreak) +
					(int) (!m_fAutoEnter || (m_lstLocalName.GetSize() > 0)) ;
			}
	} ;
	// 予約語処理関数
	typedef	ESLError
		(ECSCompiler::*PFN_COMPILE_RESERVED_WORD)
			( ECSSourceStream & cssLine ) ;
	static const PFN_COMPILE_RESERVED_WORD	m_pfnCompileReservedWord[rwMax] ;
	static const PFN_COMPILE_RESERVED_WORD	m_pfnCompileMacroWord[mwMax] ;

protected:
	ECSStrBufTagArray		m_staExternName ;	// 外部オブジェクト名
	EWStrTagArray<ECSTypeInfo>
							m_wstaExternVarType ;// 外部オブジェクト型情報
	EWStrTagArray<ECSTypeInfo>
							m_wstaExternNakedVarType ;
	EWStrTagArray<EWideString>
							m_wstaVarGlobalName ;

	ECSStrBufTagArray		m_staTypeName ;		// 型名配列
	EWStrTagArray<ECSTypeInfo>
							m_wstaTypeDef ;		// 型定義
	ECSStrTagArray			m_staMemoryClass ;	// 記憶クラス配列
	ETagSortArray<ECSWideString,ECSObject>
							m_staConstant ;		// コンパイラ定数
	ETagSortArray<ECSWideString,ECSSourceStream>
							m_staLiteral ;		// リテラル
	ETagSortArray<ECSWideString,EMacroBlock>
							m_staMacro ;		// マクロ
	ECSInteger				m_nMacroMode ;		// マクロモード
	EObjArray<EMacroNest>	m_nestMacro ;		// マクロネスト
	ECSInteger				m_csintLineNum ;
	ECSString				m_csstrFileName ;

	ETagSortArray<ECSWideString,ETemplateDefinition>
							m_staTemplate ;		// テンプレート定義

	EStatementCache *		m_pStatementCache ;	// 定義中のステートメントキャッシュ
	ReservedWord			m_rwEndOfStatementCache ;

	EMacroBlock *			m_pPreprocessMacro ;	// プリプロセッサマクロ
	ECSInteger *			m_pPreprocessFlag ;

	EObjArray<EControlNest>	m_nestCtrl ;		// 制御ネスト
	ReservedWord			m_rwCtrlType ;		// 現在の制御ネストタイプ
	bool					m_modeNakedCode ;	// naked code mode ?

	EObjArray<EWideString>	m_lstUsingNamespace ;

	ECSExecutionImageCompiler *	m_pcsxiDst ;		// 出力先イメージ
	ECSExecutionImageCompiler *	m_pcsxi ;
	ECSExecutionImageCompiler	m_csxiInitFunc ;	// 全体の初期化処理
	DWORD						m_dwInitPrologueSize ;
	ECSExecutionImageCompiler	m_csxiNakedPrologue ;
	ECSExecutionImageCompiler	m_csxiNakedEpilogue ;

	EWStrTagArray<ECSExecutionImageCompiler>
							m_wstaInlineFuncs ;	// インライン関数実装

	ECSContext				m_ctxExpr ;			// 定数式計算用のコンテキスト

	enum	ModeFlag
	{
		flagCStyleNumberLiteral	= 0x00000001,	// C 言語スタイルの数値リテラル
		flagQuoteNakedString	= 0x00000002,	// '' では \ シーケンスは無効
		flagQuoteCharactorCode	= 0x00000004,	// '' では文字コード
		flagStrictStyle			= 0x00000010,	// loose な型処理を行わない
		flagCStyleCast			= 0x00000020,	// C 言語スタイルのキャスト許可
		flagCStyleBasicType		= 0x00000040,	// C 言語互換の基本型 (signed/unsigned 修飾, char/short/long 型の置き換え)
		flagCompatibleInt32		= 0x00000080,	// int を 32 ビット互換に変更
		flagNoWarningEquMove	= 0x00000100,	// = での代入に警告を出さない
		flagRealReservedWord	= 0x00001000,	// 予約語の大文字小文字を区別する
		flagSmallReservedWord	= 0x00002000,	// 予約語は小文字として区別する
		flagReservedWordCaseMask= 0x00003000,
		flagNoDefaultVirtual	= 0x00010000,	// 明示的な virtual 指定が必要
		flagDefaultNakedNew		= 0x00020000,	// new 演算子は暗黙に naked
		flagDefualtNakedFunc	= 0x00040000,	// naked class のメンバ関数は暗黙に naked
		flagDefaultNakedAll		= 0x00080000,	// class や関数・変数は暗黙に naked
	} ;
	DWORD					m_dwModeFlags ;
	bool					m_flagOptimize ;		// 最適化有効
	bool					m_flagThrowable ;		// 例外を暗黙にスロー可能
	bool					m_flagFarCall ;			// 常に far call

	DWORD					m_dwImplementFlags ;	// complex of ImplementFlag


	EWStrTagArray<ECSPrototypeInfo>
							m_wstaPrototype ;	// グローバル関数
	EWStrTagArray<ECSPrototypeInfo>
							m_wstaNakedPrototype ;	// naked グローバル関数

	// naked mode 用レジスタ情報
	bool	m_modeTemporary ;	// テンポラリ・モード
	int		m_regExprAlloc ;	// 計算用レジスタ割り当て数
	int		m_stackExprAlloc ;	// 一時的に使用した naked stack サイズ
	int		m_objExprAlloc ;	// 一時的に使用した object stack 数
	EObjArray<ECSTypeInfo>
			m_lstExprAlloc ;	// 一時的に生成された naked stack 上のオブジェクト
	ENumArray<int>
			m_lstLockedRegister ;// 一時的に割り当て変更をロックされたレジスタ

	EObjArray<ECSTypeInfo>
			m_lstLocalVarTemporary ;	// テンポラリ・モード・一時変数

	struct	NakedExpressionTemporary
	{
		bool	modeTemporary ;
		int		regExprAlloc ;		// 計算用レジスタ割り当て数
		int		stackExprAlloc ;	// 一時的に使用した naked stack サイズ
		int		objExprAlloc ;		// 一時的に使用した object stack 数
		EObjArray<ECSTypeInfo>
				lstExprAlloc ;		// 一時的に生成された naked stack 上のオブジェクト
		ENumArray<int>
				lstLockedRegister ;// 一時的に割り当て変更をロックされたレジスタ
	} ;

	class	NakedModeSaver
	{
	public:
		ECSCompiler &				m_compiler ;
		ECSExecutionImageCompiler *	m_pcsxiSaved ;
		bool						m_modeSaved ;
	public:
		NakedModeSaver( ECSCompiler & compiler )
				: m_compiler( compiler )
			{
				m_pcsxiSaved = compiler.m_pcsxi ;
				m_modeSaved = compiler.m_modeNakedCode ;
			}
		~NakedModeSaver( void )
			{
				m_compiler.m_pcsxi = m_pcsxiSaved ;
				m_compiler.m_modeNakedCode = m_modeSaved ;
			}
	} ;
	friend class	NakedModeSaver ;

public:
	EString					m_strFilePath ;		// 現在のファイル名
	int						m_nLineNum ;		// 現在の行番号

	int						m_nErrorCount ;		// エラー数
	int						m_nWarningCount ;	// 警告数
	int						m_nWarningLevel ;	// n 以上の警告を無視する

	EString					m_strErrMsg ;		// エラーメッセージ

	// コンパイル完了時動作フラグ
	enum	FinishCompileFlag
	{
		flagForceImplementAll	= 0x0001,	// 全てのインラインを強制的に実装
	} ;

public:
	// 出力先を設定してコンパイラを初期化する
	virtual ESLError Initialize
		( ECSExecutionImageCompiler * pcsxiDst,
					const char * pszScriptName = NULL ) ;
	// スクリプトをコンパイルする
	virtual ESLError CompileScript
		( ECSSourceStream & cssScript, const char * pszFilePath = NULL ) ;
	// １行取得する
	void GetNextScriptLine
		( ECSSourceStream & cssLine,
			ECSSourceStream & cssScript, int & nLineNum ) ;
	// １行コンパイルする
	virtual ESLError CompileScriptLine
		( ECSSourceStream & cssLine, int nLineNum,
			const char * pszFilePath = NULL, bool fEnableUserMacro = true ) ;
	// コンパイルを完了する
	virtual ESLError FinishCompile( DWORD dwFlags = 0 ) ;
	// 指定のパスのファイルを開く
	virtual ESLFileObject * OpenScriptFile( const char * pszFilePath ) ;
	// エラーを出力する
	virtual ESLError OutputError
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;
	// 警告を出力する（警告レベルは数が少ないほうが重要）
	virtual ESLError OutputWarning
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;
	ESLError OutputWarning0
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;
	ESLError OutputWarning1
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;
	ESLError OutputWarning2
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;
	ESLError OutputWarning3
		( const char * pszErrMsg,
			const char * pszFilePath = NULL, int nLineNum = 0 ) ;
	// エラー数を取得する
	int GetErrorCount( void ) const
		{
			return	m_nErrorCount ;
		}
	// 警告数を取得する
	int GetWarningCount( void ) const
		{
			return	m_nWarningCount ;
		}
	// テキストマクロを処理する
	virtual ESLError ProcessTextMacro( ECSSourceStream & cssLine ) ;

private:
	// インライン関数を結合する
	int CommitInlineFunctions( DWORD dwFlags ) ;
	void LinkInlineFunction( ECSExecutionImageCompiler * pcsxiFunc ) ;

protected:
	// 予約語比較（第一パラメータは予約語実名）
	int CompareReservedWord
		( const wchar_t * pwszReservedWord,
					const wchar_t * pwszSymbol ) const ;
	// 文字列比較（第一パラメータを強制的に小文字アルファベットとして）
	static int CompareStringSmallCase
		( const wchar_t * pwszSmall, const wchar_t * pwszCase ) ;

public:
	// シンボルと名前空間
	struct	SYMBOL_NAMESPACE
	{
		EWideString	wstrFullName ;
		EWideString	wstrName ;
		EWideString	wstrNamespace ;

		SYMBOL_NAMESPACE( void ) {}
		SYMBOL_NAMESPACE( const wchar_t * pwszName )
			: wstrFullName( pwszName ), wstrName( pwszName ) {}
		SYMBOL_NAMESPACE( const SYMBOL_NAMESPACE & snsSrc )
			: wstrFullName( snsSrc.wstrFullName ),
				wstrName( snsSrc.wstrName ),
				wstrNamespace( snsSrc.wstrNamespace ) {}
		const SYMBOL_NAMESPACE & operator =
						( const SYMBOL_NAMESPACE & snsSrc )
			{
				wstrFullName = snsSrc.wstrFullName ;
				wstrName = snsSrc.wstrName ;
				wstrNamespace = snsSrc.wstrNamespace ;
				return	*this ;
			}
		void ParseSymbol( const wchar_t * pwszName ) ;
	} ;
public:	// シンボル判定処理関係
	// 予約語判定
	ReservedWord IsReservedWord( const wchar_t * pwszToken ) const ;
	// マクロ予約語判定
	MacroWord IsMacroWord( const wchar_t * pwszToken ) const ;
	// 予約語文処理
	ESLError CompileReservedWord
		( ReservedWord rwIndex, ECSSourceStream & cssLine ) ;
	// 型名か？
	int IsTypeName( const wchar_t * pwszToken ) const ;
	// 記憶クラス判定
	CSObjectMode IsMemoryClass( const wchar_t * pwszToken ) const ;
	// 関数ローカル変数判定
	int IsLocalVariableName
		( const wchar_t * pwszToken, ECSTypeInfo * pVarType = NULL ) const ;
	// 定数テーブル名判定
	int IsDataTableName
		( const wchar_t * pwszToken,
			DWORD dwRefAddr, ECSTypeInfo * pVarType = NULL ) const ;
	// 関数ポインタ判定
	bool CompileCodeAutoFunctionPointer
		( ECSTypeInfo & typeVar, const SYMBOL_NAMESPACE& snsSymbol ) ;
	// 大域関数判定
	ECSPrototypeInfo * SearchGlobalFunctionAs
			( const wchar_t * pwszToken, bool fGlobalScope ) ;
	// 大域関数ポインタ判定
	bool CompileCodeGlobalFunctionPointer
		( ECSTypeInfo & typeVar,
			const wchar_t * pwszToken, bool fGlobalScope = false ) ;
	// 関数ポインタロードコード出力
	bool CompileCodeLoadFunctionPointer
		( ECSTypeInfo & typeVar, const ECSPrototypeInfo * pFunc ) ;
	// 大域変数名判定
	int IsGlobalVariableName
		( const wchar_t * pwszToken,
			DWORD dwRefAddr, ECSTypeInfo * pVarType = NULL ) const ;
	// naked 大域変数参照
	bool CompileCodeSearchNakedGlobalVariable
			( ECSTypeInfo & typeVar, const wchar_t * pwszToken ) ;
	bool CompileCodeFindNakedGlobalVariable
			( ECSTypeInfo & typeVar, const wchar_t * pwszToken ) ;
	// 大域変数リンケージ名変換
	const wchar_t * TranslateGlobalVariableName
						( const wchar_t * pwszVarName ) const ;
	// クラス情報取得
	ECSClassInfo * GetClassInfoAs( const wchar_t * pwszClassName ) const ;
	// クラスを検索
	int GetClassInfoIndex( const wchar_t * pwszClassName ) const
		{
			return	m_pcsxiDst->GetClassInfoIndex( pwszClassName ) ;
		}
	// クラス情報取得
	const ECSClassInfo * GetNakedTypeClassInfo
						( const ECSTypeInfo & typeinf ) const ;
	const ECSClassInfo * GetNakedPtrTypeClassInfo
						( const ECSTypeInfo & typeinf ) const ;
	const ECSClassInfo * GetTypeClassInfo
						( const ECSObject *	pType ) const ;
	// Boolean 型判定
	ESLError VerifyTypeBoolean
		( const ECSTypeInfo & typeinf, bool fNoWarning = false ) ;
	// 非 void 型判定
	ESLError VerifyTypeNoVoid( const ECSTypeInfo & typeinf ) const
		{
			if ( typeinf.m_pValue == NULL )
			{
				return	ESLErrorMsg( "不正な void データ処理です。" ) ;
			}
			return	eslErrSuccess ;
		}
	// シンボルを解釈（::結合とテンプレート）
	ESLError ParseFullNameSymbol
		( SYMBOL_NAMESPACE& snsSymbol,
			ECSSourceStream & cssLine,
			bool fNoTemplateInstance = false,
			bool fAutoParseOperator = false,
			bool fAutoNormalizeNamespace = false ) ;
	bool IsCStyleBasicIntegerType( const wchar_t * pwszName ) const ;
	const wchar_t *
		TranslateCStyleBasicIntegerType( const wchar_t * pwszName ) const ;
	// 型情報を取得
	ESLError ParseTypeDescription
		( ECSTypeInfo & typeinf, ECSSourceStream & cssLine ) ;
	ESLError ParseTypePointerDecoration
		( ECSTypeInfo & typeinf, ECSSourceStream & cssLine ) ;
	ESLError ParseTypeArrayDecoration
		( ECSTypeInfo & typeinf, ECSSourceStream & cssLine ) ;
	//
	virtual ECSObject * ParseBsaicType( const wchar_t * pwszName ) const ;
	ESLError GetSimpleTypeInfoAs
			( ECSTypeInfo & typeinf, const wchar_t * pwszName ) ;
	// テンプレート引数を解釈しインスタンス化
	void ParseTemplateArgument
		( SYMBOL_NAMESPACE& snsSymbol,
			ECSSourceStream & cssLine,
			bool fNotMakeInstance = false,
			bool fMustImplement = false,
			DWORD dwImplement = implementAll ) ;
	// 関数プロトタイプ構文を解釈
	ESLError ParsePrototypeDescription
			( ECSPrototypeInfo & prototype, ECSSourceStream & cssLine ) ;
	// 関数引数構文を解釈
	ESLError ParseArgumentDescription
			( ECSPrototypeInfo & prototype, ECSSourceStream & cssLine ) ;

public:	// 数式処理関係
	// 数式処理動作フラグ（将来拡張用）
	enum	ExpressionFlag
	{
		exprInheritedFlags	= 0xFF000000,	// 動作が継承されるフラグセット
	} ;
	// 式文処理
	ESLError CompileExpressionStatement( ECSSourceStream & cssLine ) ;
private:
	// 式処理
	ESLError CompileExpression
		( ECSTypeInfo & typeinf,
			ECSSourceStream & cssLine, DWORD dwFlags = 0,
			int nPriority = 0, const wchar_t * pwszExit = NULL ) ;
protected:
	// 数値リテラル判定
	virtual int GetNumberLiteralRadix
		( ECSSourceStream & cssLine, bool & fRealNumber ) ;
	// 整数リテラル解釈
	virtual ESLError GetIntegerLiteral
			( INT64 & intLiteral,
				ECSSourceStream & cssLine, int nRadix ) ;
	// 実数リテラル解釈
	virtual ESLError GetRealLiteral
			( REAL64 & realLiteral,
				ECSSourceStream & cssLine, int nRadix ) ;
	// 文字列リテラル解釈
	virtual ESLError GetStringLiteral
			( EWideString & wstrLiteral,
				ECSSourceStream & cssLine, wchar_t wchCloser ) ;

protected:
	// 変数参照
	virtual ESLError CompileVariableReference
		( ECSTypeInfo & typeinf, CSObjectMode csomClass,
			const SYMBOL_NAMESPACE& snsSymbol, bool fWithoutFunction ) ;

public:
	// 最適化領域開始
	virtual void BeginSakura2Optimize( void ) ;
	// 最適化領域開始
	virtual void FinishSakura2Optimize( void ) ;
	// 数式の中で生成されたオブジェクトを解放する
	void FreeExpressionTemporary( int regSave = -1 ) ;
	void FreeExpressionTemporaryStack( int regSave = -1 ) ;
	// レジスタ依存などの最適化をフェンスする
	void FenceInstruction( void ) ;
	// naked mode 計算用レジスタの最後に割り当てられた番号を取得する
	int GetExpressionRegister( int last = 0 ) ;
	// naked mode 計算用レジスタを割り当てる
	int AllocateExpressionRegister( void ) ;
	// naked mode 計算用レジスタを解放する
	void FreeExpressionRegister( void ) ;
	void FreeExpressionRegister( const ECSTypeInfo & typeExpr ) ;
	// ローカル変数割り当てを一時的にロックする
	void LockExpressionWorkRegister( int regWork ) ;
	// 一時割り当て情報をセーブする
	void SaveNakedExpressionTemporary( NakedExpressionTemporary & work ) ;
	// 一時割り当て情報をリストアする
	void RestoreNakedExpressionTemporary( const NakedExpressionTemporary & work ) ;
	// 定数式情報のみでレジスタに値がロードされていない場合
	// 評価値をレジスタにロードする
	//（アドレスのインデックスのみロードされている場合には
	// フルアドレスをロードする）
	void MakeCommitValueToNakedRegister( ECSTypeInfo & typeinf ) ;
	void LoadCommitValueToNakedRegister
				( int regDst, const ECSTypeInfo & typeinf ) ;
	// naked モードで列挙型を整数型か実数型に変換する
	void CompileCodeNakedUncoverEnumerator( ECSTypeInfo & typeinf ) ;
	// naked モードで参照型を展開する
	void CompileCodeNakedUncoverReference
				( ECSTypeInfo & typeinf, int regLoad = -1 ) ;
	// naked モードで整数型に変換する
	void CompileCodeNakedConvertToInt64( ECSTypeInfo & typeinf ) ;
	ESLError CompileCodeNakedConvertToBoolean( ECSTypeInfo & typeinf ) ;
	// naked モードで実数型に変換する
	void CompileCodeNakedConvertToReal( ECSTypeInfo & typeinf ) ;
	// 自然数を 2^n に変換できるか判定し n を取得（そうでない場合は -1）
	static int GetNumberScale( INT64 nValue ) ;

protected:
	// マクロ式解釈
	struct	MacroExpression
	{
		ECSObject *	pValue ;
		bool		fOwnValue ;
		bool		fValidMacro ;
	} ;
	ESLError CompileMacroExpression
		( const EWideString & wstrMacroName,
			ECSSourceStream & cssLine,
			ECSTypeInfo & typeinf,
			MacroExpression & mxprResult ) ;
	// マクロ配列変数（定数配列オブジェクト）の要素参照文の解釈
	ESLError CompileConstantArrayElement
		( MacroExpression & mxprResult, ECSSourceStream & cssLine ) ;
	// オブジェクト構築式の解釈
	ESLError CompileTypeConstruction
		( ECSSourceStream & cssLine,
			ECSTypeInfo & typeinf,
			bool & fTypeConstruction,
			const EWideString & wstrNameSpace,
			const EWideString & wstrTypeName ) ;
	ESLError CompileNewTypeConstruction
		( ECSTypeInfo & typeinf, ECSSourceStream & cssLine ) ;
	ESLError CompileNakedTypeConstruction
		( ECSTypeInfo & typeinf, ECSSourceStream & cssLine ) ;
	ESLError CompileNewExpression
		( ECSTypeInfo & typeinf, ECSSourceStream & cssLine ) ;
	// 関数間接呼び出し
	ESLError CompileArgumentAndIndirectCallFunction
		( ECSSourceStream & cssLine,
			ECSTypeInfo & typeinf, ECSTypeInfo & typeFunc ) ;
	// 明示的オブジェクト指定の無い関数の呼び出し
	ESLError CompileArgumentAndCallFunction
		( ECSSourceStream & cssLine,
			ECSTypeInfo & typeinf,
			CSObjectMode csomClass,
			const EWideString & wstrNameSpace,
			const EWideString & wstrFuncName ) ;
	// 引数を解釈して適合するクラスメンバ関数呼び出し
	ESLError CompileArgumentAndCallMemberFunction
		( ECSSourceStream & cssLine,
			const ECSClassInfo & clsinfThis,
			const ECSTypeInfo & typeThis,
			const wchar_t * pwszFuncName, bool fLoadThisRef,
			const ECSClassInfo::MemberFunction *& pFunc,
			ECSTypeInfo::Flags flagScope ) ;
	// 関数返り値型取得
	void GetFunctionReturnType
		( ECSTypeInfo & typeinf, const ECSPrototypeInfo & proto ) ;
	// 型情報検索
	virtual ESLError SearchTypeName
		( SYMBOL_NAMESPACE& snsSymbol,
			/* in wstrName, wstrNamespace
				/ out wstrFullName, wstrName, wstrNamespace */
			ECSTypeInfo::Flags flagScope ) ;
	// 適合関数が見つからないエラーメッセージ生成
	ESLError MakeNotFoundCallErrorMsg
		( const char * pszErrMsg,
			EObjArray<ECSTypeInfo> * pCallParam ) ;
	// 曖昧な関数呼び出しエラーメッセージ生成
	ESLError MakeAmbiguousErrorMsg
		( const char * pszErrMsg,
			EPtrObjArray<ECSTypeInfo> * pCallParam,
			ECSClassInfo::ListMemberFunction * pFuncList ) ;
	// 曖昧な関数呼び出し警告メッセージ生成
	ESLError MakeAmbiguousWarningMsg1
		( const char * pszErrMsg,
			EPtrObjArray<ECSTypeInfo> * pCallParam,
			ECSClassInfo::ListMemberFunction * pFuncList ) ;
	ESLError MakeAmbiguousWarningMsg2
		( const char * pszErrMsg,
			EPtrObjArray<ECSTypeInfo> * pCallParam,
			ECSClassInfo::ListMemberFunction * pFuncList ) ;
	// 型二項演算
	ESLError CompileTypeOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSOperatorType optOperator ) ;
	ESLError CompileTypeNakedOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSOperatorType optOperator ) ;
	ESLError CompileTypeNakedOperateCode
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSOperatorType optOperator ) ;
	ESLError CompileTypePointerOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSOperatorType optOperator,
			CSInstructionCode icCode = csicOperate ) ;
	ESLError CompileTypeNakedPointerAdd
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2, int nPitch ) ;
	// 型代入演算
	ESLError CompileTypeMoveOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSOperatorType optOperator,
			bool fInitMove = false ) ;
	ESLError CompileTypeMovePointerOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSOperatorType optOperator,
			bool fInitMove = false ) ;
	// 型比較演算子
	ESLError CompileTypeCompare
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSCompareType cptCompare ) ;
	ESLError CompileTypeNakedCompare
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSCompareType cptCompare ) ;
	ESLError CompileTypeNakedCompareCode
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSCompareType cptCompare ) ;
	ESLError CompileTypePointerCompare
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc1,
			const ECSTypeInfo & typeSrc2,
			CSCompareType cptCompare ) ;
	// メンバ変数参照
	ESLError CompileTypeMemberVariable
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			const EWideString & wstrMember ) ;
	// ポインタメンバ変数参照
	ESLError CompilePointerTypeMemberVariable
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			const EWideString & wstrMember ) ;
	// メンバ関数ポインタ参照
	ESLError CompileTypeMemberFunctionPointer
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeThis, OperatorType optype,
			ECSSourceStream & cssLine, DWORD dwFlags = 0,
			int nPriority = 0, const wchar_t * pwszExit = NULL ) ;
	// 要素参照 operator []
	ESLError CompileTypeReferenceElement
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			const ECSTypeInfo & typeIndex ) ;
	// 型単項演算
	ESLError CompileTypeUnaryOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			CSUnaryOperatorType uoptUnary ) ;
	ESLError CompileTypeNakedUnaryOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			CSUnaryOperatorType uoptUnary ) ;
	// 特殊型単項演算
	ESLError CompileTypeExUnaryOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			CSExtraUniOperatorType xuoptExUnary ) ;
	ESLError CompileTypePointerExUnaryOperate
		( ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			CSExtraUniOperatorType xuoptExUnary ) ;
	// sizeof(type) 演算子解釈
	ESLError ParseNakedSizeOfTypeOperator
		( ECSSourceStream & cssLine, bool& fSizeOf, int& nNakedSize ) ;
	// -> 演算子処理
	ESLError CompileTypeCallPointerOperator( ECSTypeInfo & typeinf ) ;
	// 変数作成命令出力
	ESLError CompileNewVariable
		( CSObjectMode csomType,
			const wchar_t * pwszVarName, ECSObject * pTypeObj ) ;
	// ポインタを参照型へ変換する命令出力
	ESLError CompileReferencePointer
		( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc ) ;
	// ポインタを参照型へ変換する命令出力（型指定）
	ESLError CompileReferenceMemberPointer
		( ECSTypeInfo & typeDst, const ECSTypeInfo & typeRefType ) ;
	// naked buffer 上の変数参照をオブジェクトスタック上に正規化する
	void NormalizeObjectPointerRefForNaked( ECSTypeInfo & typeinf ) ;
	// *naked, &naked（リニアアドレス）をポインタオブジェクトに正規化する
	void NormalizePointerFromLinearAddress( ECSTypeInfo & typeinf ) ;
	// naked 参照ポインタにオフセット加算
	void CompileCodeOffsetPointerReference( int iOffset ) ;

protected:
	// 型キャスト命令出力
	enum	CastCompileFlags
	{
		castAcceptRef		= 0x0001,	// 出力先が参照型でなくても参照で渡せる
		castStatic			= 0x0002,	// static_cast
		castDynamic			= 0x0004,	// dynamic_cast
		castCastNoRef		= 0x0008,	// 出力先の参照型は無視する
		castNoNakedPointer	= 0x0010,	// naked ポインタへの変換はしない
	} ;
	ESLError CompileTypeCast
		( ECSTypeInfo & typeResult,
			const ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			DWORD dwFlags, ECSTypeInfo::Flags flagScope ) ;
	// ポインタ型からのキャスト命令出力
	ESLError CompileTypeCastFromPointer
		( ECSTypeInfo & typeResult,
			const ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			DWORD dwFlags, ECSTypeInfo::Flags flagScope ) ;
	// 型キャストオペレーターオーバーロード処理出力
	ESLError CompileCastByOperator
		( ECSTypeInfo & typeResult,
			const ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			DWORD dwFlags, ECSTypeInfo::Flags flagScope ) ;
	// 親クラスへのキャスト命令出力
	ESLError CompileCastToParentClass
		( ECSTypeInfo & typeAddressing,
			const ECSClassInfo::CastInfo & castParent,
			const ECSClassInfo & clsinfSrc,
			const ECSTypeInfo & typeSrc, ECSTypeInfo::Flags flagScope ) ;
	// naked モードでの基本型（整数・実数）変換
	ESLError CompileNakedBasicTypeCast
		( bool& fProcessed, ESLError errVerifyType,
			ECSTypeInfo & typeResult,
			const ECSTypeInfo & typeDst,
			const ECSTypeInfo & typeSrc,
			DWORD dwFlags, ECSTypeInfo::Flags flagScope ) ;
	// 整数型のキャスト命令出力（nakedモード）
	ESLError CompileNakedCastIntegerToInteger
		( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc ) ;
	// 即値のキャスト
	ESLError CompileImmidiateCast
		( ECSObject*& pResult, const ECSTypeInfo & typeCast,
			ECSObject * pValue, DWORD dwFlags = 0 ) ;

protected:
	// クラスオブジェクトを構築する
	ESLError CompileObjectConstruction
		( const ECSClassInfo & clsinf, const ECSTypeInfo & typeSrc ) ;
	// デフォルトコンストラクタの有無
	bool IsDefaultConstructor( const ECSTypeInfo & typeinf ) ;
	bool IsDefaultConstructor( const ECSClassInfo & clsinf ) ;
	// デフォルトコンストラクタ呼び出し
	ESLError CompileCallDefaultConstructor
		( const ECSTypeInfo & typeinf, bool fLoadThisRef ) ;
	ESLError CompileCallDefaultConstructor
		( const ECSClassInfo & clsinf, bool fLoadThisRef ) ;
	// コンストラクタ呼び出し
	ESLError CompileCallObjectConstructor
		( const ECSClassInfo & clsinf,
					const EPtrObjArray<ECSTypeInfo> & lstArg ) ;
	// 型キャストできないエラーメッセージ生成
	ESLError ErrorMsgTypeCast
		( const ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc ) ;
	// メンバ関数呼出し命令生成
	ESLError CompileCallMemberFunction
				( const ECSClassInfo & clsinfThis,
					const ECSTypeInfo & typeThis,
					const ECSClassInfo::MemberFunction & func,
					const EPtrObjArray<ECSTypeInfo> & lstArg,
					DWORD dwArgCount, ECSTypeInfo::Flags flagScope,
					bool fNoVirtualCall = false ) ;
	// グローバル関数呼出し命令生成
	ESLError CompileCallGlobalFunction
		( const ECSPrototypeInfo & func,
			const EPtrObjArray<ECSTypeInfo> & lstArg, DWORD dwArgCount ) ;
	// 間接関数呼出し命令生成
	ESLError CompileCallIndirectFunction
		( int regFunc, const ECSPrototypeInfo & func, DWORD dwArgCount ) ;

protected:
	// naked モード・グローバルスクリプト関数呼び出し命令生成
	//（レジスタ保存＆引数渡し＆コール＆後始末）
	void CompileNakedCallGlobal
		( const wchar_t * pwszFuncName, DWORD dwArgCount ) ;
	void CompileNakedIndirectCallGlobal( int regFunc, DWORD dwArgCount ) ;
	// naked モード・グローバル native 関数呼び出し命令生成
	//（引数渡し＆コール＆後始末）
	void CompileNakedSystemCall
		( const wchar_t * pwszFuncName, DWORD dwArgCount ) ;
	void CompileNakedIndirectSystemCall( int regSysId, DWORD dwArgCount ) ;
	// naked モード関数引数のプッシュ
	void CompileNakedPushArgumentToCall( DWORD dwArgCount ) ;
	// naked モード関数呼び出しの為に一時レジスタと関数引数のプッシュ
	void CompileNakedPushBeforeCall( DWORD dwArgCount, int regExcept = -1 ) ;
	// naked モード関数引数スタック解放
	void CompileNakedFreeArgumentAfterCall( DWORD dwArgCount ) ;
	// naked モード関数呼び出し後に引数スタックの解放と一時レジスタをプッシュ
	void CompileNakedPopAfterCall( DWORD dwArgCount, int regExcept = -1 ) ;

	// naked モード式計算用一時レジスタをプッシュ
	void CompileNakedPushExpressionTemporary
			( DWORD dwArgCount = 0, int regExcept = -1 ) ;
	// naked モード式計算用一時レジスタをポップ
	void CompileNakedPopExpressionTemporary
			( DWORD dwArgCount = 0, int regExcept = -1 ) ;

protected:
	// naked モードから object モード関数を呼び出すために
	// naked 引数を object スタックにプッシュ
	ESLError CompileNakedPushToObjectArgument
		( const ECSPrototypeInfo & func, DWORD dwArgCount ) ;
	// object モード関数の返り値を naked モードに変換
	ESLError CompileNakedPopFromReturnedObject
		( const ECSPrototypeInfo & func, DWORD dwArgCount ) ;
	// object モードから naked モード関数を呼び出すために
	// object 引数を naked レジスタに変換
	ESLError CompileNakedPushFromObjectArgument
		( const ECSPrototypeInfo & func,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			DWORD dwArgCount, int & nPointerCount ) ;
	// naked モード関数の返り値を object モードに変換
	// （ついでに一時的に確保したポインタを解放）
	ESLError CompileNakedPushObjectReturned
		( const ECSPrototypeInfo & func,
			DWORD dwArgCount, int nPointerCount ) ;
	// naked レジスタを object スタックにプッシュ
	ESLError CompileNakedPushObject( const ECSTypeInfo & typeinf, int regSrc ) ;
	// object スタックから naked レジスタにポップ
	ESLError CompileNakedPopObject( const ECSTypeInfo & typeinf, int regDst ) ;

protected:
	// 関数の引数を処理
	ESLError CompileArgument
		( EObjArray<ECSTypeInfo> & lstArgType,
			const ECSPrototypeInfo * pProto, ECSSourceStream & cssLine ) ;
	// 既にレジスタにロードされた関数の引数を正規化（主にコンストラクタ用）
	ESLError NormalizeLoadedNakedArgument
		( const ECSPrototypeInfo * pProto,
			const EPtrObjArray<ECSTypeInfo> & lstArgType, DWORD& dwArgCount ) ;
	// 関数の引数を正規化（主に operator 用）
	ESLError NormalizeNakedArgument
		( const ECSPrototypeInfo * pProto,
				EPtrObjArray<ECSTypeInfo> & lstArgType, DWORD& dwArgCount ) ;
	// 直接オブジェクトを返す naked 関数の返り値を準備する
	ESLError CompileNakedFuncPrepareToReturnObject
			( const ECSPrototypeInfo * pProto, int & nArgAddition ) ;
	// 一時オブジェクトの領域を確保してメモリを初期化する（構築関数は呼び出さない）
	ECSTypeInfo * CompilePrepareNakedTemporaryObject
			( ECSTypeInfo & typeTemp, const ECSTypeInfo & typeObj ) ;
	// 一時オブジェクトの領域を確保する（ローカル変数情報に一時領域を確保する）
	ECSTypeInfo * CompileAllocateNakedTemporaryObject
			( const ECSTypeInfo & typeObj ) ;
	// 確保されたローカル変数の領域のメモリを初期化する（構築関数は呼び出さない）
	ESLError CompilePrepareNakedLocalVariable
			( ECSTypeInfo & typeTemp,
				const ECSTypeInfo * pLocalVar, bool fNoGarbage = false ) ;
	// ローカル変数の領域を確保する（ローカル変数情報を登録しアドレスを確定する）
	ECSTypeInfo * CompileAllocateNakedLocalVariable
			( EControlNest * pNest,
				const ECSTypeInfo & typeObj, const wchar_t * pwszVarName ) ;
	// 一時領域 int64[n] を確保する（主にガベージリスト用）
	ECSTypeInfo * CompileAllocateNakedTemporaryBuffer( int nCount ) ;
	// ユーザー定義マクロを展開
	ESLError CompileUserMacro
		( EMacroBlock * pmbMacro,
			const EObjArray<EWideString> & lstParam,
						ECSObject ** pRetValue = NULL ) ;
	ESLError CompileUserMacro
		( EMacroBlock * pmbMacro,
				EObjArray<ECSObject> & lstParam,
					ECSObject ** pRetValue = NULL ) ;
	// マクロ関数引数を取得
	ESLError CompileMacroArgument
		( EObjArray<EWideString> & lstParam, ECSSourceStream & cssExpr ) ;

protected:
	// 定数オブジェクトを即値データ命令として出力
	virtual ESLError CompileImmediateObject
			( ECSTypeInfo & typeinf, ECSObject * pObj ) ;
	void CompileImmediateInteger( INT64 nValue ) ;
	void CompileImmediateReal( REAL64 nValue ) ;
	void CompileImmediateString( const wchar_t * pwszValue ) ;
	// 基本型オブジェクト生成命令出力
	void CompileImmediateBasicVariable( CSVariableType csvtType ) ;
	// クラスオブジェクト生成命令出力
	ESLError CompilerCodeCreateClassObject( const wchar_t * pwszGlobalName ) ;
	// 多次元配列オブジェクト構築命令を出力
	virtual ESLError CompileArrayDimension( ECSArray * pArray ) ;
	// ハッシュコンテナオブジェクト構築命令を出力
	virtual ESLError CompileHashContainer( ECSHash * pHash ) ;
	// 関数ポインタをスタック上にロードする命令を生成
	ESLError CompileImmediateFunctionPointer( const wchar_t * pwszFuncName ) ;
	// naked ネイティブ関数 ID をスタック上にロードする命令を生成
	ESLError CompileImmediateNakedNativeFunctionID( const wchar_t * pwszFuncName ) ;
	// スタック上のオブジェクトへの一時参照ロード命令生成
	ESLError CompileLoadStackObject( long int nStackTop ) ;
	// 変数参照命令出力
	void CompileCodeLoadRefVariable
			( CSObjectMode objmode, int nVarIndex ) ;
	void CompileCodeLoadRefVariable
			( CSObjectMode objmode, const wchar_t * pwszVarName ) ;
	// スタック解放命令出力
	void CompileCodeFreeStack( void ) ;
	void CompileCodeFreeStack( const ECSTypeInfo & typeinf ) ;
	// naked メモリポインタ変換命令出力
	void CompileCodePointerToObject( int iOffset ) ;
	void CompileCodePointerToAddress( void ) ;
	// ポインタ参照命令出力
	void CompileCodeReferenceForPointer( CSVariableType csvtRefType ) ;
	// ジャンプ命令出力
	DWORD CompileCodeJump( void ) ;
	DWORD CompileCodeJump( DWORD dwTargetAddr ) ;
	DWORD CompileCodeConditionalJump( bool fLogic, bool fPushAfter ) ;
	DWORD CompileCodeConditionalJump
			( DWORD dwTargetAddr, bool fLogic, bool fPushAfter ) ;
	void CompileCodeCommitJumpAddress
				( DWORD dwJumpRefAddr, DWORD dwTargetAddr ) ;
	DWORD CompileCodeGetCurrent( void ) const ;
	// ストア命令出力
	void CompileCodeStore( CSOperatorType optype = csotNop ) ;
	// 演算命令出力
	void CompileCodeOperate( CSOperatorType optype ) ;
	// 比較命令出力
	void CompileCodeCompare( CSCompareType cmptype ) ;
	// スワップ命令出力
	void CompileCodeSwap( int nIndex1, int nIndex2 ) ;

protected:
	// naked モードロード・ストア命令
	ESLError WriteSakuraMoveMemory
		( bool fStore,
			ECSSakura2Processor::AddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int regBase, int offset32,
			int regIndex = 0, int scaleIndex = 0, bool fNoRegCache = false ) ;
	ESLError WriteSakuraLoadMemory
		( ECSSakura2Processor::AddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int regBase, int offset32,
			int regIndex = 0, int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveMemory
			( false, mode, type, regDst,
				regBase, offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}
	ESLError WriteSakuraStoreMemory
		( ECSSakura2Processor::AddressingMode mode,
			ECSSakura2Processor::DataType type, int regSrc,
			int regBase, int offset32,
			int regIndex = 0, int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveMemory
			( true, mode, type, regSrc,
				regBase, offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}
	ESLError WriteSakuraLoadMemory
		( int& regDst, const ECSTypeInfo & typeVar,
			bool fRefType = false, int iOffset = 0, bool fNoRegCache = false ) ;
	ESLError WriteSakuraStoreMemory
		( int regSrc, const ECSTypeInfo & typeVar,
			bool fRefType = false, int iOffset = 0, bool fNoRegCache = false ) ;
	// ローカルメモリ命令
	ESLError WriteSakuraMoveLocal
		( bool fStore,
			ECSSakura2Processor::LocalAddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int offset32, int regIndex = 0,
			int scaleIndex = 0, bool fNoRegCache = false ) ;
	ESLError WriteSakuraLoadLocal
		( ECSSakura2Processor::LocalAddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int offset32, int regIndex = 0,
			int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveLocal
			( false, mode, type, regDst,
				offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}
	ESLError WriteSakuraStoreLocal
		( ECSSakura2Processor::LocalAddressingMode mode,
			ECSSakura2Processor::DataType type, int regSrc,
			int offset32, int regIndex = 0,
			int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveLocal
			( true, mode, type, regSrc,
				offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}

protected:
	// マクロ予約語の処理
	ESLError CompileMacroError( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroWarning( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroCompile( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroIf( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroElseIf( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroElse( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroEndIf( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroLet( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroLocal( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroFor( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroNext( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroLiteral( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroDefMacro( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroEndMacro( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroExitMacro( ECSSourceStream & cssLine ) ;
	ESLError CompileMacroUndefMacro( ECSSourceStream & cssLine ) ;
	// 予約語の処理
	ESLError CompileInclude( ECSSourceStream & cssLine ) ;
	ESLError CompileOption( ECSSourceStream & cssLine ) ;
	ESLError CompileDeclareType( ECSSourceStream & cssLine ) ;
	ESLError CompileDeclareDef( ECSSourceStream & cssLine ) ;
	ESLError CompileExternDef( ECSSourceStream & cssLine ) ;
	ESLError CompileTypeDef( ECSSourceStream & cssLine ) ;
	ESLError CompileVariable( ECSSourceStream & cssLine ) ;
	ESLError CompileConstant( ECSSourceStream & cssLine ) ;
	ESLError CompileData( ECSSourceStream & cssLine ) ;
	ESLError CompileEndData( ECSSourceStream & cssLine ) ;
	ESLError CompileEnumerator( ECSSourceStream & cssLine ) ;
	ESLError CompileEndEnum( ECSSourceStream & cssLine ) ;
	ESLError CompileEnumerate( ECSSourceStream & cssLine ) ;
	ESLError CompileStructure( ECSSourceStream & cssLine ) ;
	ESLError CompileEndStruct( ECSSourceStream & cssLine ) ;
	ESLError CompileClass( ECSSourceStream & cssLine ) ;
	ESLError CompileEndClass( ECSSourceStream & cssLine ) ;
	ESLError CompileNamespace( ECSSourceStream & cssLine ) ;
	ESLError CompileEndNamespace( ECSSourceStream & cssLine ) ;
	ESLError CompileUnion( ECSSourceStream & cssLine ) ;
	ESLError CompileEndUnion( ECSSourceStream & cssLine ) ;
	ESLError CompilePublic( ECSSourceStream & cssLine ) ;
	ESLError CompileProtected( ECSSourceStream & cssLine ) ;
	ESLError CompilePrivate( ECSSourceStream & cssLine ) ;
	ESLError CompilePrototype( ECSSourceStream & cssLine ) ;
	ESLError CompileFunction( ECSSourceStream & cssLine ) ;
	ESLError CompileEndFunc( ECSSourceStream & cssLine ) ;
	ESLError CompileAssembler( ECSSourceStream & cssLine ) ;
	ESLError CompileEndAssembler( ECSSourceStream & cssLine ) ;
	ESLError CompileAssembleLine1( ECSSourceStream & cssLine ) ;
	ESLError CompileAssembleLine2( ECSSourceStream & cssLine ) ;
	ESLError CompileIf( ECSSourceStream & cssLine ) ;
	ESLError CompileElseIf( ECSSourceStream & cssLine ) ;
	ESLError CompileElse( ECSSourceStream & cssLine ) ;
	ESLError CompileEndIf( ECSSourceStream & cssLine ) ;
	ESLError CompileBegin( ECSSourceStream & cssLine ) ;
	ESLError CompileEnd( ECSSourceStream & cssLine ) ;
	ESLError CompileBreak( ECSSourceStream & cssLine ) ;
	ESLError CompileContinue( ECSSourceStream & cssLine ) ;
	ESLError CompileReturn( ECSSourceStream & cssLine ) ;
	ESLError CompileGoto( ECSSourceStream & cssLine ) ;
	ESLError CompileLabel( ECSSourceStream & cssLine ) ;
	ESLError CompileTry( ECSSourceStream & cssLine ) ;
	ESLError CompileCatch( ECSSourceStream & cssLine ) ;
	ESLError CompileEndTry( ECSSourceStream & cssLine ) ;
	ESLError CompileThrow( ECSSourceStream & cssLine ) ;
	ESLError CompileFor( ECSSourceStream & cssLine ) ;
	ESLError CompileNext( ECSSourceStream & cssLine ) ;
	ESLError CompileWhile( ECSSourceStream & cssLine ) ;
	ESLError CompileEndWhile( ECSSourceStream & cssLine ) ;
	ESLError CompileRepeat( ECSSourceStream & cssLine ) ;
	ESLError CompileUntil( ECSSourceStream & cssLine ) ;
	ESLError CompileSwitch( ECSSourceStream & cssLine ) ;
	ESLError CompileEndSwitch( ECSSourceStream & cssLine ) ;
	ESLError CompileCase( ECSSourceStream & cssLine ) ;
	ESLError CompileDefault( ECSSourceStream & cssLine ) ;
	ESLError CompileMemoryFence( ECSSourceStream & cssLine ) ;
	ESLError CompileTemplate( ECSSourceStream & cssLine ) ;
	ESLError CompileEndTemplate( ECSSourceStream & cssLine ) ;
	ESLError CompileUsing( ECSSourceStream & cssLine ) ;
	ESLError CompileFriend( ECSSourceStream & cssLine ) ;
	// ステートメント・キャッシュ
	bool IsCacheStatement
		( ECSSourceStream & cssLine,
			ReservedWord rwType, StatementType stType = stReservedWord ) ;
	// ステートメント・キャッシュ・展開
	void ImplementStatementCache
		( const EStatementCache & statements,
			const char * pszNameForError,
			DWORD dwImplements = implementAll ) ;

protected:
	// 変数参照リスト登録
	ENumArray<DWORD> *
		DeclareVariableReferenceDef
			( const wchar_t * pwszVarName,
				bool fNakedDef, bool fSharedDef, bool fConstDef ) ;
	// 変数型宣言
	ESLError ExternVariableType
		( const wchar_t * pwszVarName,
			ECSTypeInfo& typeVar, bool fNakedDef ) ;

protected:
	// object モード変数初期値正規化
	ESLError NormalizeObjectInitialValue
		( ECSObject*& pObjInit,
			const ECSTypeInfo & typeVar, ECSObject * pValue ) ;
	// naked モード変数初期値・変数配列長正規化
	ESLError NormalizeNakedVariableInitialValue
		( ECSObject*& pObjInit,
			ECSTypeInfo & typeVar,
			ECSObject * pValue, bool fVarTypeFixed ) ;
	// object モードグローバル変数定義
	ESLError DeclareGlobalObjectVariable
		( const EWideString & wstrVarName,
			const ECSTypeInfo & typeVar, ECSObject * pObjInit ) ;
	// object モード shared グローバル変数定義
	ESLError DeclareSharedGlobalObjectVariable
		( const EWideString & wstrVarName,
			const ECSTypeInfo & typeVar, ECSObject * pObjInit ) ;
	// naked モードグローバル変数定義
	ESLError DeclareGlobalNakedVariable
		( const EWideString & wstrVarName,
			const ECSTypeInfo & typeVar, ECSObject * pObjInit,
			bool fShared, bool fConstant ) ;
	// 変数初期値設定
	bool MakeVariableInitImage
		( BYTE * pbytInitBuf, const ECSTypeInfo & typeVar,
					ECSObject * pObjInit, bool fInitRuntime ) ;
	// naked モード初期値解釈
	ESLError CompileInitValuesForNakedVariable
		( const ECSTypeInfo & typeVar, ECSSourceStream & cssLine ) ;

protected:
	struct	VIRT_OFFSET_GATE
	{
		int					nIndex ;
		ECSPrototypeInfo *	pPrototype ;
		int					nThisOffset ;
	} ;
	// インライン関数生成・登録
	ECSExecutionImageCompiler * CreateTemporaryInlineFunction
		( const wchar_t * pwszFuncName, DWORD dwFuncFlags, bool fNoGetDefined = true ) ;
	// インライン関数終端設定
	void SetEndOfTemporaryInlineFunction
		( ECSExecutionImageCompiler * pcsxiInline, const wchar_t * pwszFuncName ) ;
	// naked クラス初期値イメージ生成
	ESLError CompileCodeNakedClassInitImage( ECSClassInfo * pClassInf ) ;
	ESLError CompileCodeNakedClassInitImage
		( const ECSClassInfo * pClassInf,
			BYTE * pbytInitBuf, DWORD dwBaseAddr ) ;
	// naked クラス初期値イメージに仮想関数ベクタ設定
	void CompileCodeNakedClassInitImage_VirtualVector
		( const wchar_t * pwszVectorName, int& iVirtFunc, bool fRootVector,
			const ECSClassInfo * pClassInf, BYTE * pbytInitBuf, DWORD dwVecBaseAddr ) ;
	// naked クラス実行時キャストベクタ生成
	ESLError CompileCodeNakedClassCastVector( ECSClassInfo * pClassInf ) ;
	// naked クラス仮想関数ベクタ生成
	ESLError CompileCodeNakedVirtualFuncVector( ECSClassInfo * pClassInf ) ;
	// naked 多重派生クラスの派生元クラス仮想関数ベクタ情報生成
	void CompileCodeNakedVirtualFuncVector_SuperClass
		( EObjArray<VIRT_OFFSET_GATE>& lstOffsetOverride, int& iVirtFunc,
			const wchar_t * pwszVectorName,
			DWORD dwVecBaseAddr, ECSClassInfo * pClassInf,
			ECSClassInfo * pSuperClassInf, int nNakedOffset, int iFuncOffset ) ;
	// naked クラスオーバーライド関数 this オフセットゲート関数生成
	ESLError CompileCodeNakedVirtualOfffsetGateFunc
		( ECSClassInfo * pClassInf,
			ECSPrototypeInfo * pPrototype, int iThisOffset ) ;
	// syscall ゲート関数生成
	ESLError CompileCodeNakedNativeFuncGate( const wchar_t * pwszSysCall ) ;
	// 関数エントリゲート生成
	ESLError CompileFunctionEntryGate
		( ECSClassInfo * pClassInf,
			ECSPrototypeInfo * pPrototype,
			const EObjArray<EWideString> & lstArgName, bool fLocalFunc = false ) ;
	// 構造体・クラス完成処理
	ESLError CompileFinalizeClassInfo( ECSClassInfo * pClassInf ) ;
	// コンストラクタコード出力
	ESLError CompileCodeConstructor
		( ECSClassInfo * pClassInf, ECSSourceStream & cssLine ) ;
	// naked クラスメモリ初期化コード生成
	//（スタックのトップはポインタで、処理後に破棄されない）
	ESLError CompileCodeNakedClassInitialize
					( const ECSClassInfo * pClassInf ) ;
	// naked クラスデストラクタ呼び出しコード生成
	ESLError CompileCodeNakedClassDestruction
				( const ECSClassInfo * pClassInf, bool fNoVirtual ) ;
	ESLError CompileCodeNakedVariableDestruction( const ECSTypeInfo & typeVar ) ;
	ESLError CompileCodeNakedNativeObjectDestruction( void ) ;
	// naked クラスデストラクタ呼び出しコード生成（delete）
	ESLError CompileCodeNakedClassPointerDestruction
			( const ECSClassInfo * pClassInf,
				bool fNoVirtual, bool& fPointerDestruction ) ;
	// デフォルトの delete 演算子関数生成
	ESLError CompileCodeNakedClassDeleteOperator
			( ECSClassInfo * pClassInf,
				ECSPrototypeInfo * pPrototype ) ;
	// naked クラス配列デストラクタ呼び出しコード生成
	// object スタックには先頭ポインタ参照と個数がプッシュされている
	ESLError CompileCodeNakedVariableDestructionList( const ECSClassInfo * pClassInf ) ;
	// naked クラスデフォルトデストラクタ呼び出しコード生成
	ESLError CompileCodeNakedClassAfterDestruction( const ECSClassInfo * pClassInf ) ;

public:
	// ガベージ・リスト登録処理
	ESLError CompileCodeNakedAddGarbageList( const ECSTypeInfo * pVarType ) ;
	// naked 関数ローカル変数のデストラクタ呼び出しコード生成
	ESLError CompileCodeNakedFunctionDestruction( void ) ;
	ESLError CompileCodeNakedLocalDestruction( EControlNest * pNest, int iNest ) ;
	int CountOfNakedFunctionDestruction( void ) const ;
	int CountOfNakedLocalDestruction( EControlNest * pNest ) const ;
	// naked 関数ローカル変数のデストラクタ呼び出しコード生成
	//（主に数式内で生成した一時変数のデストラクタを呼び出すため）
	ESLError CompileCodeNakedLocalDestructionSaveRegister
				( EControlNest * pNest, int regFirst, int regCount = 1 ) ;
	// naked 関数のリーブ＆リターンコード生成
	ESLError CompileCodeNakedFunctionReturn( const ECSTypeInfo & typeReturn ) ;
	// naked ローカル変数の初期値設定コード生成（メモリの初期化・object の生成）
	ESLError CompileCodeNakedLocalInitialDefault( const ECSTypeInfo & typeVar ) ;
	ESLError CompileCodeNakedLocalInitialConstValue
		( const ECSTypeInfo & typeVar, ECSObject * pObjInit ) ;

protected:
	// ユーザー定義シンボルの有効性確認
	ESLError VerifyUserSymbol( const wchar_t * pwszSymbol ) const ;
	// 制御ネストに変数を追加する
	void AddVariableToControlNest
		( EControlNest * pNest,
			const wchar_t * pwszVarName, ECSTypeInfo * pVarType ) ;
	// 制御ネストを１つ離脱する
	void LeaveControlNest( EControlNest * pTopNest = NULL ) ;
	// naked ローカル変数割り当てアドレス更新
	void UpdateNestLocalFrameBase( void ) ;
	// メンバ関数の修飾を補正
	ESLError CompileEffectPrototype
		( ECSPrototypeInfo & prototype,
			EControlNest * pClassNest, bool fFuncBlock ) ;
	ESLError CompileEffectPrototype
		( ECSPrototypeInfo & prototype, ECSClassInfo * pClassInf ) ;
	// 関数の引数定義リストを出力
	void EncodeArgumentList
		( EControlNest * pNest,
			const EObjArray<ECSTypeInfo> & lstArgType,
			const EObjArray<EWideString> & lstArgName ) ;
	// 制御ブロックの脱出アドレスを確定
	void CommitBreakAddressOnNest
		( EControlNest * pNest, DWORD dwBreakAddr ) ;
	// 特定の制御ブロックを取得
	EControlNest * GetMostInnerNest
		( ReservedWord rwFirst,
			ReservedWord rwLast, int * pNestCount = NULL ) const ;
	// 関数ネスト内か判定する
	bool IsInFunctionNest( void ) const
		{
			return	(GetMostInnerNest
						( rwFunction, rwFunction ) != NULL) ;
		}
	// throw されうる例外リストを現在のネストに追加する
	void AddThrowListToCurrentNest( const EObjArray<EWideString>& lstThrows ) ;
	void AddThrowListToCurrentNest( const ECSPrototypeInfo& proto )
		{
			AddThrowListToCurrentNest( proto.GetThrows() ) ;
		}
	// naked モードで構造化例外処理に対応するブロックか判定する
	bool IsNakedThrowableNest( int iNest = 0 ) const ;
	// 名前空間検索リストに含まれるか判定
	bool IsUsingNamespaceNest( const wchar_t * pwszNamespace, int iNest = 1 ) const ;
	// 検索名前空間に必要なリストを追加する
	void AddNecessaryUsingNamespace( EControlNest * pNest ) ;
	void AddNecessaryUsingNamespacePath
		( EControlNest * pNest, const wchar_t * pwszName ) const ;
	void AddNecessaryUsingParentClassList
		( EControlNest * pNest, ECSClassInfo * pClassInf ) const ;
	// 名前空間検索リストを取得
	void GetUsingNamespaceList
		( EPtrObjArray<const wchar_t> & lstNamespace ) const ;
	void AddUsingParentClassList
		( EPtrObjArray<const wchar_t> & lstNamespace, ECSClassInfo * pClassInf ) const ;
	// 現在の名前空間名を取得
	EWideString GetCurrentSpaceName( void ) const ;
	// 現在の this 型名を取得
	EWideString GetCurrentThisClassName( void ) const ;
	// 現在の this クラス取得
	ECSClassInfo * GetCurrentThisClass( void ) const ;
	// 現在のスコープから指定クラスのアクセススコープ取得
	DWORD GetAccessClassTo( const wchar_t * pwszClassName, DWORD dwFlags = 0 ) const ;
	DWORD GetAccessClassTo( const ECSClassInfo * pClassInf, DWORD dwFlags = 0 ) const ;
	// 現在の this が naked クラスか？
	bool IsNakedCurrentThisClass( void ) const ;
	// naked 関数内か？
	bool IsInNakedCodeFunction( void ) const ;
	// テンプレートの宣言文中か？
	bool IsInTemplateDeclaration( void ) const ;
	// C 互換性モードか？
	bool IsCStyleCompatibleMode( void ) const ;

protected:
	ECSObject *	m_pRefMacroVariable ;

public:
	// 外部参照マクロ変数設定
	void AttachExternalMacroVariable( ECSObject * pVar ) ;
	// 変数オブジェクトの条件判定
	static ESLError EvaluateVariable( ECSObject * pObj ) ;
	// 定数値（マクロ変数）取得
	virtual ECSObject * GetMacroVariable( const wchar_t * pwszName ) ;
	// 大域定数値（マクロ変数）設定
	void SetMacroVariable( const wchar_t * pwszName, ECSObject * pObj ) ;
	// 定数式を実行する時に利用する実行イメージで初期化する
	virtual ESLError InitConstExprContext( ECSExecutionImage * pcsxi ) ;
	// 定数式を実行する時に利用する実行コンテキストのリソース解放
	virtual void ReleaseConstExprContext( void ) ;

public:
	// 定数式評価
	ESLError CalculateExpression
		( ECSObject *& pValue,
			ECSSourceStream & cssLine, int nPriority = 0,
			const wchar_t * pwszExit = NULL,
			bool fImmidiateValue = true, bool fDefaultZeroSymbol = false ) ;
	// 配列即値の型情報を正規化
	static void NormalizeArrayTypeInfo( ECSArray & arrayType ) ;
	// 演算子の情報を取得する
	ESLError GetOperatorInfo
		( OPERATOR_INFO & opinf,
			const wchar_t * pwszOperator, bool fUnary = false ) ;
	// 演算子の operator オーバーロード名を取得する
	static const wchar_t * GetOperatorString( const OPERATOR_INFO & opinf ) ;
	// 演算子の優先度を取得する
	int GetOperatorPriority( const OPERATOR_INFO & opinf ) ;
	int GetUnaryOperatorPriority( const OPERATOR_INFO & opinf ) ;

	friend ECSAssembler ;
	friend ECSExecutionImageLinker ;
} ;
