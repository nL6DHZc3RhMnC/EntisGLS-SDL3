
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script ver.3 Sakura2 アセンブラ
//////////////////////////////////////////////////////////////////////////////

#include <glscs/glscs_sakura2_assembler.h>

class	ECSCompiler ;
class	ECSAssembler	: public ESLObject
{
public:
	// クラス情報
	ESL_DECLARE_CLASS_INFO( ECSAssembler, ESLObject )
	// 構築関数
	ECSAssembler( void ) ;
	// 消滅関数
	virtual ~ECSAssembler( void ) ;

public:
	// ディレクティブ
	enum	DirectiveType
	{
		dirInvalid	= -1,
		dirIf,
		dirElseIf,
		dirElse,
		dirEndIf,
		dirWhile,
		dirEndWhile,
		dirRepeat,
		dirUntil,
		dirBreak,
		dirContinue,
		dirRegister,
		dirAssume,
		dirInvoke,
		dirCount,
	} ;
	// ラベルエントリ
	class	LabelEntry	: public SSystem::SArray<uint32_t>
	{
	public:
		uint32_t	m_addrLabel ;		// ラベルアドレス
	} ;
	// レジスタ割り当てエントリ
	struct	RegisterAssign
	{
		int	numReg ;
		int	numCount ;
	} ;
	// ディレクティブネスト
	class	DirectiveNest
	{
	public:
		DirectiveType	m_typeDirective ;
		LabelEntry		m_labelContinue ;
		LabelEntry		m_labelBreak ;
		LabelEntry		m_labelElse ;
	} ;
	// オペランド・シンボル参照
	enum	SymbolType
	{
		symbolInvalid	= -1,
		symbolLocalLabel,	// ラベル
		symbolFunction,		// 関数
		symbolGlobal,		// グローバル変数
		symbolConst,
		symbolShared,
		symbolSysCall,		// システム関数
		symbolClass,		// クラス ID
	} ;
	class	SymbolReference
	{
	public:
		SymbolType			m_typeSymbol ;
		SSystem::SString	m_strSymbol ;
		ECSPrototypeInfo *	m_pProto ;
	public:
		SymbolReference( void )
			: m_typeSymbol(symbolInvalid), m_pProto(NULL) {}
	} ;
protected:
	// ディレクティブ名
	static const wchar_t * m_pwszDirectiveName[dirCount] ;
	// ディレクティブ処理関数
	typedef	SSystem::SError (ECSAssembler::*PFUNC_ASSEMBLE)
					( SSystem::SStringParser & cssLine ) ;
	static const PFUNC_ASSEMBLE	m_pfnDirective[dirCount] ;
	// レジスタの割り当て
	SSystem::SObjectArray<ECSTypeInfo>				m_assignsRegType ;
	SSystem::SStrSortObjectArray<RegisterAssign>	m_assignsRegName ;
	// 一時レジスタの割り当て
	DWORD										m_maskTempReg ;
	// コンパイラ
	ECSCompiler *								m_compiler ;
	// 出力先イメージ
	ECSExecutionImageCompiler *					m_pcsxi ;
	// ディレクティブ・ネスト
	SSystem::SObjectArray<DirectiveNest>		m_nestDirective ;
	// ラベル
	SSystem::SStrSortObjectArray<LabelEntry>	m_ssoaLabel ;
	// エラーメッセージ
	SSystem::SString							m_strErrMsg ;

public:
	// コンパイラ設定
	void AttachCompiler( ECSCompiler * compiler ) ;
	// 出力先設定
	void AttachOutputImage( ECSExecutionImageCompiler * pcsxi ) ;
	// 出力完了処理
	void FinishOutputImage( void ) ;
	// ディレクティブ判定
	static DirectiveType IsDirective( const wchar_t * pwszName ) ;
	// １行アセンブル
	SSystem::SError AssembleLine
		( SSystem::SStringParser & sparsLine, int nPass ) ;
	// ディレクティブコンパイル
	SSystem::SError CompileDirective
		( DirectiveType dirType, SSystem::SStringParser & sparsLine ) ;
	// ラベル定義
	SSystem::SError CompileLabel( SSystem::SStringParser & sparsLine, int nPass ) ;
	// オペランドアセンブル
	SSystem::SError AssembleOperand
		( ECSSakura2Assember::Operand& opTerm,
			SymbolReference& symRef, SSystem::SStringParser & sparsLine ) ;
	// レジスタ解釈
	SSystem::SError ParseRegister
		( int& numReg, SSystem::SStringParser & sparsLine ) ;
	// メモリオペランド解釈
	SSystem::SError ParseMemoryOperand
		( ECSTypeInfo& typeMem,
				ECSSakura2Assember::Operand& opTerm,
				SSystem::SStringParser & sparsLine ) ;
	// メモリ式解釈
	SSystem::SError ParseMemoryExpression
		( ECSTypeInfo& typeMem,
				ECSSakura2Assember::Operand& opTerm,
				SSystem::SStringParser & sparsLine ) ;
	// 変数型情報からメモリオペランド情報へ変換
	void MemoryOperandFromTypeInfo
		( ECSSakura2Assember::Operand& opTerm, const ECSTypeInfo& typeMem ) ;
	// 変数型情報からメモリアクセスデータ型へ変換
	static ECSSakura2Processor::DataType
		DataTypeFromTypeInfo( const ECSTypeInfo& typeMem ) ;
	// シンボル解釈
	SSystem::SError CompileSymbol
		( ECSSakura2Assember::Operand& opTerm,
			SymbolReference& symRef, SSystem::SStringParser & sparsLine ) ;
	// 即値解釈
	SSystem::SError CompileImmediate
		( ECSObject*& pValue, SSystem::SStringParser & sparsLine,
			const wchar_t * pwszExit = NULL, int nPriority = 0 ) ;
	// ニーモニックアセンブル
	SSystem::SError AssembleMnemonic
		( const SSystem::SString& strMnemonic, SSystem::SStringParser & sparsLine ) ;
	// エラーメッセージ取得
	const SSystem::SString& GetErrorMessage( void ) const
	{
		return	m_strErrMsg ;
	}

protected:
	// ラベル相対ジャンプ確定
	void CommitLabelReference( const LabelEntry& label ) ;
	// ディレクティブネスト検索
	DirectiveNest * FindDirectiveNest
		( int dirFirst, int dirEnd, int iNest = 0 ) const ;

protected:
	// ディレクティブコンパイル
	SSystem::SError AssembleIf( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleElseIf( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleElse( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleEndIf( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleWhile( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleEndWhile( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleRepeat( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleUntil( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleBreak( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleContinue( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleRegister( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleAssume( SSystem::SStringParser & sparsLine ) ;
	SSystem::SError AssembleInvoke( SSystem::SStringParser & sparsLine ) ;

protected:
	// 実行時式用一時レジスタ割り当て
	int AllocateTemporaryRegister( void ) ;
	// 一時レジスタ解放
	void FreeTemporaryRegister( int reg ) ;
	void FreeAllTemporaryRegister( void ) ;
	// 一時レジスタか？
	bool IsTemporaryRegister( int reg ) const ;
	// 実行時式の1項を計算しレジスタを返す
	SSystem::SError CompileRuntimeTerm
		( int& numReg, ECSTypeInfo& typeTerm,
				SSystem::SStringParser & sparsLine ) ;
	// 条件実行時式を評価し条件分岐コードを出力する
	SSystem::SError CompileRuntimeExpression
		( int& regRuntime,
			LabelEntry& labelTrue,
			LabelEntry& labelFalse, bool fPositive,
			SSystem::SStringParser & sparsLine,
			const wchar_t * pwszExit = NULL, int nPriority = 0 ) ;
	SSystem::SError CompileRuntimeConditionalExpression
		( LabelEntry& labelTrue,
			LabelEntry& labelFalse, bool fPositive,
			SSystem::SStringParser & sparsLine,
			const wchar_t * pwszExit = NULL, int nPriority = 0 ) ;
	// 条件実行時式を評価し条件分岐コードを出力する（二項演算子継続処理）
	SSystem::SError CompileRuntimeConditionalOperators
		( LabelEntry& labelTrue,
			LabelEntry& labelFalse, bool fPositive,
			ECSTypeInfo& typeExpr, int& regRuntime,
			SSystem::SStringParser & sparsLine,
			const wchar_t * pwszExit = NULL, int nPriority = 0 ) ;
	// 実行時式比較演算子
	SSystem::SError CompileRuntimeConditionalComparator
		( CSCompareType csctType, const SSystem::SString& strOperator,
			ECSTypeInfo& typeTerm1, int& regTerm1,
			const ECSTypeInfo& typeTerm2, int regTerm2 ) ;
	// 実行時式演算子
	SSystem::SError CompileRuntimeConditionalOperator
		( CSOperatorType csotType,
			const SSystem::SString& strOperator,
			LabelEntry& labelTrue, LabelEntry& labelFalse, bool fPositive,
			ECSTypeInfo& typeExpr, int& regRuntime,
			SSystem::SStringParser & sparsLine,
			const wchar_t * pwszExit, int nPriority ) ;

} ;


