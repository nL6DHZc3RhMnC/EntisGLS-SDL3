
#if	!defined(__ROSETTA_PARSER_H__)
#define	__ROSETTA_PARSER_H__

namespace	Rosetta
{
	class	RSInstruction ;

	//////////////////////////////////////////////////////////////////////////
	// フレームポインタ
	//////////////////////////////////////////////////////////////////////////

	struct	RSFramePointer
	{
		int	iLocal ;		// ローカルフレームへの指標（全Object割り当て）
		int	iNumber ;		// 整数・浮動小数点への指標
		int	iPointer ;		// ポインタ変数への指標

		RSFramePointer( void )
		{
			iLocal = -1 ;
			iNumber = -1 ;
			iPointer = -1 ;
		}
		RSFramePointer( const RSFramePointer& fp )
		{
			iLocal = fp.iLocal ;
			iNumber = fp.iNumber ;
			iPointer = fp.iPointer ;
		}
		RSFramePointer( int l, int n, int p )
		{
			iLocal = l ;
			iNumber = n ;
			iPointer = p ;
		}
		const RSFramePointer& operator = ( const RSFramePointer& fp )
		{
			iLocal = fp.iLocal ;
			iNumber = fp.iNumber ;
			iPointer = fp.iPointer ;
			return	*this ;
		}
		bool operator == ( const RSFramePointer& fp ) const
		{
			return	(iLocal == fp.iLocal)
					&& (iNumber == fp.iNumber)
					&& (iPointer == fp.iPointer) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// コンパイル時型情報
	//////////////////////////////////////////////////////////////////////////

	class	RSTypeInfo
	{
	public:
		// 変数数値型
		enum	NumberType
		{
			typeBoolean,
			typeUint8,
			typeInt8,
			typeUint16,
			typeInt16,
			typeUint32,
			typeInt32,
			typeInt64,
			typeFloat32,
			typeFloat64,
			typeObject,
			typeFirstInt	= typeBoolean,
			typeLastInt		= typeInt64,
			typeFirstFloat	= typeFloat32,
			typeLastFloat	= typeFloat64,
			typeFirstNumber	= typeBoolean,
			typeLastNumber	= typeFloat64,
		} ;
		RSClass *		m_pClass ;			// 型
		uint32_t		m_accMod ;			// 型修飾
		NumberType		m_typeNum ;			// 数値型
		bool			m_flagPointer ;		// ポインタ型・構造体
		bool			m_flagPtrRef ;		// ポインタ先への参照
		bool			m_flagReference ;	// 左辺値
		RSFramePointer	m_fp ;				// ローカルフレームへの指標
		RSObject *		m_pImmediate ;		// 即値
		size_t			m_iFuncProto ;		// m_pImmediate が関数の場合のプロトタイプ指標
		RSInstruction *	m_pLoadInst ;		// ロード命令

		static const size_t	m_bytesNumber[typeLastNumber + 1] ;
		static const size_t	m_bitsNumber[typeLastNumber + 1] ;
		static const RSInteger::IntegerType
							m_typeInteger[typeLastNumber + 1] ;
		static const RSCodeControl::WordIndex
							m_wiBasicType[typeLastNumber + 1] ;

	public:
		// 構築関数
		RSTypeInfo( void ) ;
		RSTypeInfo( const RSTypeInfo& ti ) ;
		// 消滅関数
		~RSTypeInfo( void ) ;
		// 代入
		RSTypeInfo& operator = ( const RSTypeInfo& ti ) ;
		// 一時計算領域に割り当てられているか？
		bool IsAllocated( void ) const ;
		// オブジェクトか？
		bool IsObject( void ) const ;
		// 整数型か？
		bool IsInteger( void ) const ;
		// 浮動小数点型か？
		bool IsFloatingPoint( void ) const ;
		// ポインタ・構造体型か？
		bool IsPointer( void ) const ;
		// 構造体か？
		bool IsStructure( void ) const ;
		// 左辺値（参照）か？
		bool IsReference( void ) const ;
		// const 修飾
		bool IsConstant( void ) const ;
		// 型設定
		void SetType( const RSContext& context, RSClass * pClass ) ;
		void CopyType( const RSTypeInfo & ti ) ;
		// 数値型判定
		static NumberType GetNumberTypeOf
				( const RSContext& context, RSClass * pClass ) ;
		// 即値設定
		void SetImmediate
			( RSContext& context, RSObject * pObj,
				RSClass * pClass, size_t iFuncProto = 0 ) ;
		// ローカル変数割り当てサイズ取得
		void GetLocalFrameSize( RSFramePointer& fp ) const ;
		// キャスト可能判定
		bool IsMatchTypeFor
			( const RSContext& context, RSClass * pClass ) const ;
		// 型名取得
		SSystem::SString GetTypeName( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 中間処理コード
	//////////////////////////////////////////////////////////////////////////

	class	RSInstruction	: public ESLObject
	{
	public:
		// 命令コード
		enum	Code
		{
			codeInvalid		= -1,
			codeLoad,				// スタックにロード
			codeFree,				// スタック解放
			codeElementInt,			// 要素参照（リテラル参照）
			codeElementStr,
			codeElementIndirectInt,	// 要素参照（実行時参照）
			codeElementIndirectStr,
			codeElementRef,			// 構造体メンバ参照
			codeElementPtr,			// 構造体メンバ参照ポインタ
			codeOperate,			// 二項演算
			codeUniOperate,			// 単項演算
			codeCast,				// 型キャスト
			codeConvertPointer,		// ポインタ型間変換
			codeConvertNumber,		// 数値型間変換
			codeConvertNum2Str,		// num->String
			codeConvertNum2Object,	// {num|Ptr}->Object
			codeConvert2Boolean,	// *->boolean
			codeExOperate,			// 特殊処理
			codeJump,				// 制御移行
			codeJumpIf,				// 条件付き制御移行
			codeJumpNotIf,
			codeCall,				// 関数呼び出し
			codeCallIndirect,
			codeCallVirtual,
			codeReturn,				// 関数復帰
			codeNewObject,			// オブジェクト生成
			codeTry,				// 例外
			codeEndTry,
			codeThrow,
			codeGetException,
			codeClearException,
			codeSynchronize,		// 同期
			codeUnsynchronize,
			codeFence,				// スタック参照境界ヒント
		} ;
		enum	ExOperator
		{
			xopLength,				// 配列長取得
			xopKeys,				// キー配列取得
		} ;
		enum	MemoryClass
		{
			memoryImmediate,
			memoryLocal,
			memoryGlobal,
			memoryRuntime,
		} ;
		Code				m_code ;			// 命令コード
		RSCodeOperator::OperatorIndex
							m_opCode ;			// 演算子
		ExOperator			m_xopCode ;
		SSystem::SString	m_literal ;			// リテラル／参照名
		int					m_index ;			// 参照指標・ポイント補正値
		int					m_limit ;			// 新しいポインタ領域バイト数
		size_t				m_ipTarget ;		// 移行先命令ポインタ
		MemoryClass			m_memory ;			// 対象記憶クラス
		int					m_nLocalNest ;		// ローカルネスト（ローカル関数の場合の上位関数指標）
		RSTypeInfo			m_typeSrc1 ;		// 入力オペランド
		RSTypeInfo			m_typeSrc2 ;
		SSystem::SObjectArray<RSTypeInfo>
							m_typeArg ;			// 関数引数
		RSTypeInfo			m_typeDst ;			// 出力先・評価型

		// ※ローカル変数の数値やポインタ型を非オブジェクトで保持する場合、
		// 　関数呼び出しの際、引数は自動的にオブジェクトへ変換される
		// ※数値型やポインタ型への関数呼び出しの場合、
		// 　this は一時的にオブジェクトへ変換され、
		// 　関数復帰後、ローカル形式に変換されるものとする
		//　（それら処理は暗黙であり、コード化されない）

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSInstruction, ESLObject )
		// 構築関数
		RSInstruction( void ) ;
		// 消滅関数
		virtual ~RSInstruction( void ) ;

	public:
		// 即値ロード
		void LoadImmediate
			( RSContext& context, RSObject * pObj ) ;
		// ローカル変数ロード
		void LoadLocal
			( RSContext& context,
				RSClass * pType, const RSFramePointer& fp ) ;
		void LoadLocal
			( RSContext& context,
				const RSTypeInfo& typeLocal, int nLocalNest ) ;
		// グローバル変数ロード
		void LoadGlobal
			( RSContext& context,
				RSClass * pType, const wchar_t * pwszName ) ;
		// 配列要素直接参照
		void LoadElementInt
			( RSContext& context,
				const RSTypeInfo& typeArray, int nIndex, bool fReference ) ;
		void LoadElementStr
			( RSContext& context,
				RSClass * pElementType,
				const RSTypeInfo& typeObj,
				const wchar_t * pwszName, bool fReference ) ;
		void LoadIndirectElementInt
			( RSContext& context,
				const RSTypeInfo& typeArray,
				const RSTypeInfo& typeIndex, bool fReference ) ;
		void LoadIndirectElementStr
			( RSContext& context,
				RSClass * pElementType,
				const RSTypeInfo& typeObj,
				const RSTypeInfo& typeIndex, bool fReference ) ;
		// 構造体・ポインタ要素参照
		void RefStructureMember
			( RSContext& context,
				RSClass * pElementType,
				const RSTypeInfo& typeObj,
				int iOffsetBytes, int nLimitBytes ) ;
		void PtrStructureMember
			( RSContext& context,
				RSClass * pElementType,
				const RSTypeInfo& typeObj,
				int iOffsetBytes, int nLimitBytes ) ;
		// 単項演算子
		void UniOperator
			( RSContext& context,
				RSCodeOperator::OperatorIndex opIndex,
				RSClass * pDstType, const RSTypeInfo& typeSrc ) ;
		// 二項演算子
		void BinOperator
			( RSContext& context,
				RSCodeOperator::OperatorIndex opIndex,
				RSClass * pDstType,
				const RSTypeInfo& typeSrc1, const RSTypeInfo& typeSrc2 ) ;
		// 代入演算子
		void MoveOperator
			( RSContext& context,
				RSCodeOperator::OperatorIndex opIndex,
				const RSTypeInfo& typeDst, const RSTypeInfo& typeSrc ) ;
		// 型キャスト
		void CastObject
			( RSContext& context,
				RSClass * pCastType, const RSTypeInfo& typeSrc ) ;
		void ConvertPointer
			( RSContext& context,
				RSClass * pCastType, const RSTypeInfo& typeSrc ) ;
		void ConvertNumber
			( RSContext& context,
				RSClass * pCastType, const RSTypeInfo& typeSrc ) ;
		void ConvertObject
			( RSContext& context,
				RSInstruction::Code code,
				RSClass * pCastType, const RSTypeInfo& typeSrc ) ;
		// オブジェクト生成
		void NewObject
			( RSContext& context, RSClass * pClass ) ;
		// 配列長取得
		void LengthOf( RSContext& context, const RSTypeInfo& typeSrc ) ;
		// キー配列取得
		void KeysOf( RSContext& context, const RSTypeInfo& typeSrc ) ;
		// ジャンプ
		void Jump( size_t ipTarget ) ;
		void JumpIf
			( RSContext& context,
				const RSTypeInfo& typeSrc, size_t ipTarget ) ;
		void JumpNotIf
			( RSContext& context,
				const RSTypeInfo& typeSrc, size_t ipTarget ) ;
		// 関数呼び出し
		void CallDirect
			( RSContext& context,
				RSFunctionObject * pFunc, size_t iFunc,
				const RSTypeInfo& typeThis,
				const SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		void CallIndirect
			( RSContext& context,
				const RSTypeInfo& typeFunc, size_t iFunc,
				const RSTypeInfo& typeThis,
				const SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		void CallVirtual
			( RSContext& context,
				RSClass * pReturnType,
				const wchar_t * pwszFuncName, size_t iFunc,
				const RSTypeInfo& typeThis,
				const SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		// 関数復帰
		void Return( const RSTypeInfo& typeSrc ) ;
		// 例外ブロック
		void BeginTry( size_t ipTarget ) ;
		void EndTry( void ) ;
		void Throw( const RSTypeInfo& typeSrc ) ;
		void GetException( const RSTypeInfo& typeDst ) ;
		void ClearException( void ) ;
		// 同期
		void Synchronize( const RSTypeInfo& typeSrc ) ;
		void Unsynchronize( const RSTypeInfo& typeSrc ) ;
		// 解放
		void FreeLocal( const RSTypeInfo& typeFree ) ;
		// フェンス命令
		void FenceStack
			( const RSFramePointer& fpMin, const RSFramePointer& fpMax ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 中間処理コードブロック
	//////////////////////////////////////////////////////////////////////////

	class	RSInstructionBlock	: public ESLObject
	{
	public:
		SSystem::SObjectArray<RSInstruction>	m_block ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSInstructionBlock, ESLObject )
		// 構築関数
		RSInstructionBlock( void ) ;
		// 消滅関数
		virtual ~RSInstructionBlock( void ) ;

	public:
		// コード追加
		void AddCode( RSInstruction * pInst ) ;
		// コード挿入
		void InsertCode( size_t iPos, RSInstruction * pInst ) ;
		// コード削除
		void ClearCodeAfter( size_t iPos ) ;
		// コード全削除
		void ClearAllCodes( void ) ;
		// 現在のコード位置
		size_t GetCurrentPos( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 中間処理コードコンパイラ
	//////////////////////////////////////////////////////////////////////////

	class	RSInstructionParser	: public ESLObject
	{
	protected:
		RSVirtualMachine *	m_vm ;
		RSContext			m_context ;
		RSContext			m_ctxMacro ;
 
		// ローカル入れ子空間
		//////////////////////////////////////////////////////////////////////
		enum	NestControlType
		{
			nestSpace,
			nestIf,
			nestElseIf,
			nestElse,
			nestFor,
			nestWhile,
			nestDo,
			nestSwitch,
			nestTry,
			nestWith,
			nestSynchronized,
		} ;
		class	LabelInfo
		{
		public:
			RSTypeInfo	m_typeCase ;
			size_t		m_ipLabelPos ;
		} ;
		class	LocalNest
		{
		public:
			NestControlType	m_control ;
			SSystem::SIndexedArray<SSystem::SString,const wchar_t*>
							m_names ;
			SSystem::SPointerArray<RSTypeInfo>
							m_types ;
			SSystem::SPointerArray<RSInstruction>
							m_breaks ;
			SSystem::SPointerArray<RSInstruction>
							m_continues ;
			SSystem::SObjectArray<LabelInfo>
							m_caseLabels ;
			ssize_t			m_ipDefault ;
			bool			m_fSpaceObj ;
			RSTypeInfo		m_typeSpaceObj ;
		public:
			// 構築関数
			LocalNest( NestControlType ctrl )
				: m_control( ctrl ), m_ipDefault( -1 ), m_fSpaceObj( false ) { }
			// 変数追加
			void AddVariable( const wchar_t * pwszName, RSTypeInfo * pType ) ;
			// 変数検索
			RSTypeInfo * GetVariable( const wchar_t * pwszName ) const ;
			// break ジャンプ先を確定する
			void CommitBreakTargets( size_t ipTarget ) ;
			// continue ジャンプ先を確定する
			void CommitContinueTargets( size_t ipTarget ) ;
		} ;

		// ローカル名前空間
		//////////////////////////////////////////////////////////////////////
		class	LocalSpace
		{
		public:
			SSystem::SObjectArray<LocalNest>
							m_nest ;
			SSystem::SIndexedArray<SSystem::SString,const wchar_t*>
							m_names ;
			SSystem::SObjectArray<RSTypeInfo>
							m_types ;
			size_t			m_iTempVar ;
		public:
			// 構築関数
			LocalSpace( void ) : m_iTempVar( 0 ) { }
			// 入れ子作成
			LocalNest * DescendNest( NestControlType ctrl ) ;
			// 入れ子削除
			LocalNest * AscendNest( void ) ;
			// 入れ子取得
			LocalNest * GetCurrentNest( void ) const ;
			LocalNest * GetNestAt( size_t iNest ) const ;
			// 入れ子数取得
			size_t GetNestDepth( void ) const ;
			// 変数追加
			void AddVariable( const wchar_t * pwszName, RSTypeInfo * pType ) ;
			RSTypeInfo * ReAddVariable( const wchar_t * pwszName ) ;
			// 変数検索
			RSTypeInfo * GetNestedVariable( const wchar_t * pwszName ) const ;
			RSTypeInfo * GetVariableAs( const wchar_t * pwszName ) const ;
		} ;

		// 定義済みローカル変数指標
		enum	ReservedLocalIndex
		{
			localIndexThis	= 0,
			localFirstArg	= 1,
		} ;

		// コンパイルフェーズ
		//////////////////////////////////////////////////////////////////////
		enum	CompilePhase
		{
			phaseDeclaration,		// クラス宣言
			phaseDefinition,		// メンバ定義
			phaseAllocation,		// ローカル変数割り当て
			phaseImplementation,	// コード実装
		} ;

		// 関数コードブロック
		//////////////////////////////////////////////////////////////////////
		class	FunctionBlock	: public RSInstructionBlock
		{
		public:
			FunctionBlock *			m_pParentScope ;
			RSFunctionPrototype *	m_pProto ;		// 関数プロトタイプ
			RSClass *				m_pClassSpace ;	// クラス定義空間
			RSFramePointer			m_fpLocal ;		// ローカル変数割り当て
			RSFramePointer			m_fpExpr ;		// 一時計算用割り当て
			LocalSpace				m_local ;		// ローカル変数
			SSystem::SPointerArray<RSFunctionObject>
									m_aNamelessFuncs ;	// 関数内無名関数
			bool					m_fPreConstruction ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( FunctionBlock, RSInstructionBlock )
			// 構築関数
			FunctionBlock
				( RSFunctionPrototype * pProto,
					FunctionBlock * pParentScope = NULL ) ;
			// 消滅関数
			virtual ~FunctionBlock( void ) ;
		public:
			// 新しいフェーズのための初期化処理
			void ResetPhase( CompilePhase phase ) ;
			// 一時計算用割り当てをすべて解放
			void FreeAllTemporary( void ) ;
			// 一時関数追加
			void AddNamelessFunction( RSFunctionObject * pFunc ) ;
		} ;

		// クラス付加情報
		//////////////////////////////////////////////////////////////////////
		class	ClassAppendix
		{
		public:
			RSClass *			m_pClass ;			// 対象クラス
			SSystem::SString	m_strJSName ;		// JavaScript 名
			RSClass *			m_pSuperClass ;		// 親クラス
			SSystem::SPointerArray<RSClass>
								m_lstImplements ;
			FunctionBlock		m_codeInit ;		// 初期化コード
			RSFunctionPrototype	m_protoInit ;
			RSFunctionObject *	m_pInitFunc ;
		public:
			ClassAppendix( void )
				: m_pClass( NULL ),
					m_codeInit( NULL, NULL ), m_pInitFunc( NULL )
			{
				m_codeInit.m_pProto = &m_protoInit ;
			}
			~ClassAppendix( void )
			{
				RSObject::ReleaseRef( m_pInitFunc ) ;
			}
		} ;

		// 関数付加情報
		//////////////////////////////////////////////////////////////////////
		class	FunctionAppendix
		{
		public:
			RSFunctionPrototype *	m_pProto ;
			CompilePhase			m_phaseCompiled ;
			FunctionBlock			m_codeFunc ;
		public:
			FunctionAppendix
				( RSFunctionPrototype * pProto, FunctionBlock * pParentScope )
				: m_pProto( pProto ),
					m_phaseCompiled( phaseDeclaration ),
					m_codeFunc( pProto, pParentScope ) { }
		} ;

		// グローバル初期コード
		FunctionBlock		m_blockGlobalInit ;
		RSFunctionPrototype	m_protoGlobalInit ;

		// インポートスクリプト
		SSystem::SStrSortObjectArray<RSScript>	m_aScripts ;

		// クラス付加情報
		SSystem::SPtrSortObjectArray
					<RSClass,ClassAppendix>		m_psoaClass ;

		// 関数付加情報
		SSystem::SPtrSortObjectArray
			<RSFunctionPrototype,FunctionAppendix>	m_psoaFunc ;

		// 現在の文の開始位置
		const RSParenthesis *	m_pCurParenthesis ;
		size_t					m_iSrcStatement ;

		class	SmartParenthesis
		{
		protected:
			RSInstructionParser &	m_parser ;
			const RSParenthesis *	m_pCurParenthesis ;
			size_t					m_iSrcStatement ;
		public:
			SmartParenthesis( RSInstructionParser& parser )
				: m_parser( parser ),
					m_pCurParenthesis( parser.m_pCurParenthesis ),
					m_iSrcStatement( parser.m_iSrcStatement ) { }
			~SmartParenthesis( void )
			{
				m_parser.m_pCurParenthesis = m_pCurParenthesis ;
				m_parser.m_iSrcStatement = m_iSrcStatement ;
			}
		} ;

		// 現在の式の文脈
		RSTypeInfo		m_typeExprThis ;	// 関数呼び出しの this

		// フェーズ
		CompilePhase	m_phase ;

		// エラー結果
		size_t	m_nError ;
		size_t	m_nWarning ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSInstructionParser, ESLObject )
		// 構築関数
		RSInstructionParser( RSVirtualMachine * vm ) ;
		// 消滅関数
		virtual ~RSInstructionParser( void ) ;

	public:
		// スクリプト読み込み（クラス宣言解釈のみ）
		SSystem::SError LoadScript
			( const wchar_t * pwszFileName,
				SSystem::Charset::EncodingType
					encoding = SSystem::Charset::encodingUnknown ) ;
		// コンパイル実行
		void CompilePhaseDefinition( void ) ;
		void CompilePhaseAllocation( void ) ;
		void CompilePhaseImplementation( void ) ;
	protected:
		void CompileAllScripts( void ) ;
		void CompileAllClassCodes( void ) ;
		bool CompileAllFunctionCodes( void ) ;
		void CompilePhaseClassDefinition( RSClass * pClass ) ;
		bool CompileFunctionImplementation( FunctionAppendix * pFuncAppendix ) ;

	protected:
		// 複文解釈
		void CompileAllStatements
			( FunctionBlock& block,
				RSCodeStream& cstrm, RSClass * pClassSpace ) ;
		// 文解釈
		void CompileStatements
			( FunctionBlock& block, RSCodeStream& cstrm ) ;
		// 一文解釈
		void CompileAStatement
			( FunctionBlock& block, RSCodeStream& cstrm ) ;
		// 式を読み飛ばす
		static void PassExpression
			( RSCodeStream& cs,
				int priority = RSCodeOperator::priorityNothing ) ;
		// クラス文／関数文（... {} | ... ;）を読み飛ばす
		static void PassBlockStatement( RSCodeStream& cs ) ;
		// 型式を評価
		RSClass * ParseClassExpression( RSCodeStream& cs ) ;
		// 定義文解釈／実行
		enum	DeclareModeFlag
		{
			declModeVar		= 0x0001,
			declModeFunc	= 0x0002,
			declModeAny		= 0x0003,
		} ;
		SSystem::SError CompileDeclareVariable
			( FunctionBlock& block,
				RSCodeStream& cstrm, uint32_t accMod,
				RSClass * pClass, uint32_t modeDecl = declModeAny ) ;
		// 構築関数解釈
		SSystem::SError CompileDeclareConstructor
			( FunctionBlock& block,
				RSCodeStream& cstrm,
				RSParenthesis * pPrthArg,
				uint32_t accMod, RSClass * pClass ) ;
		// 関数定義解釈
		SSystem::SError CompileDeclareFunction
			( FunctionBlock& block,
				RSCodeStream& cstrm,
				RSCodeSymbol * pSymName,
				RSParenthesis * pPrthArg,
				uint32_t accMod, RSClass * pClass ) ;
		// 関数宣言
		void DeclareFunctionPrototype
			( FunctionBlock& block,
				const wchar_t * pwszFuncName,
				RSFunctionPrototype * pProto,
				RSCodeStream& cstrm, uint32_t accMod ) ;
		// 変数定義解釈
		SSystem::SError CompileDeclareVariable
			( FunctionBlock& block,
				RSCodeStream& cstrm,
				RSCodeSymbol * pSymName,
				uint32_t accMod, RSClass * pClass ) ;
		// ローカル変数作成（即値）
		SSystem::SError CreateVariableImmediateAs
			( FunctionBlock& block,
				const wchar_t * pwszName,
				uint32_t accMod, RSObject * pObj ) ;
		// ローカル変数作成
		void CreateVariableTypeAs
			( FunctionBlock& block,
				const wchar_t * pwszName,
				uint32_t accMod, RSClass * pType ) ;
		void CreateVariableTypeAs
			( FunctionBlock& block,
				const wchar_t * pwszName, RSTypeInfo * ptiVar ) ;
		// 無名ローカル変数作成
		void CreateNamelessVariableAs
			( FunctionBlock& block,
				SSystem::SString& strTempName, RSTypeInfo * ptiVar ) ;
		// 関数修飾子設定
		static void SetPrototypeAccessModifier
			( RSFunctionPrototype * pProto, uint32_t& accMod ) ;
		// 代入処理
		SSystem::SError CompileMoveToLocalReference
			( FunctionBlock& block,
				const wchar_t * pwszDstName, const RSTypeInfo& typeSrc ) ;

	public:
		// 数式評価
		enum	ExpressionFlag
		{
			exprNoOutputCode	= 0x0001,
		} ;
		SSystem::SError CompileExpression
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs,
				int priority = RSCodeOperator::priorityNothing,
				uint32_t nFlags = 0 ) ;
	protected:
		// コード出力検証
		bool VerifyExprOutputCode( uint32_t nFlags ) ;
		// RSContext 例外をエラー出力
		bool TestContextException( RSContext& context ) ;
		// 一時計算領域へ割り当てられていない場合、割り当ててロード命令を出力
		SSystem::SError NormalizeAllocation
			( FunctionBlock& block, uint32_t nFlags, RSTypeInfo& typeTemp ) ;
		// 一時計算用変数割り当て
		void AllocateTemporary
			( FunctionBlock& block, RSTypeInfo& typeTemp ) ;
		// 一時計算用割り当てを解放し、一時的な参照を解放するコード出力
		void WriteFreeAllTemporary( FunctionBlock& block ) ;
		// 文の文脈初期化
		void ResetStatementContext( FunctionBlock& block ) ;

	protected:	// 式・単項解釈
		// this インスタンス取得
		SSystem::SError CompileLoadThis
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags = 0 ) ;
		// 無名 function インスタンス式
		SSystem::SError CompileNamelessFunction
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags = 0 ) ;
		// super 構築関数呼び出し式
		SSystem::SError CompileSuperConstructor
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags,
				RSParenthesis& prthArg ) ;
		// super メンバ参照式
		SSystem::SError CompileSuperMember
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, int priority, uint32_t nFlags,
				const SSystem::SString& strMember ) ;
		// new 演算子
		SSystem::SError CompileNewOperator
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags ) ;
		// 単項演算子
		SSystem::SError CompileUnaryOperator
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				RSCodeOperator::OperatorIndex opIndex, bool fPostOp ) ;
		// 型キャスト
		enum	CastFlag
		{
			castNatural	= 0x0000,		// 暗黙キャスト
			castForce	= 0x0001,		// 明示キャスト
		} ;
		SSystem::SError CompileTypeCast
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				RSClass * pCastType,
				uint32_t nCastFlags, const RSTypeInfo& typeSrc ) ;
		SSystem::SError CompileCastBoolean
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				uint32_t nFlags, uint32_t nCastFlags ) ;
		// 配列インスタンス生成式 [ expr, expr, ... ]
		SSystem::SError CompileArrayExpression
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSParenthesis& prth, uint32_t nFlags ) ;
		// 辞書配列インスタンス生成式 { id : expr, ... }
		SSystem::SError CompileHashMapExpression
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSParenthesis& prth, uint32_t nFlags ) ;

	protected:	// 式・二項解釈
		// メンバ参照
		SSystem::SError CompileOperatorReferenceMember
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags,
				RSCodeOperator::OperatorIndex iOp ) ;
		// 論理積 &&
		SSystem::SError CompileOperatorLogicalAnd
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags ) ;
		// 論理積 ||
		SSystem::SError CompileOperatorLogicalOr
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags ) ;
		// 式選択 ? expr : expr
		SSystem::SError CompileOperatorSelect
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags ) ;
		// HashMap<type> / Function<type,...> 式の判定
		bool CompileGenericClass
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags,
				RSCodeOperator * pOpCode ) ;
		// 一般的な二項演算子
		SSystem::SError CompileBinaryOperator
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				RSTypeInfo& typeRight, RSCodeOperator::OperatorIndex iOp ) ;

	protected:
		// 関数呼び出し／オブジェクト構築
		SSystem::SError CompileCallFunction
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				uint32_t nFlags, RSParenthesis& prthArg ) ;
		// オブジェクト構築
		SSystem::SError CompileCallConstructor
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				RSParenthesis& prthArg, RSClass * pClass ) ;
		// 数値型構築
		SSystem::SError CompileCallNumberConstructor
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				SSystem::SObjectArray<RSTypeInfo>& aArgType,
				RSTypeInfo::NumberType typeNum ) ;
		// ポインタ型構築
		SSystem::SError CompileCallPointerConstructor
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				SSystem::SObjectArray<RSTypeInfo>& aArgType,
				RSTypedArrayPointerClass * pClass ) ;
		// 配列要素参照 expr[expr]
		SSystem::SError CompileReferenceElement
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				uint32_t nFlags, RSParenthesis& prthIndex ) ;

	protected:
		// 関数呼び出しアクセス修飾子判定
		bool ValidationPrototypeAccess
			( FunctionBlock& block,
				RSFunctionPrototype& proto,
				RSClass * pObjClass,
				uint32_t accMod = RSObject::modifierPublic ) ;
		// メンバアクセス修飾子判定
		bool ValidationMemberAccess
			( FunctionBlock& block,
				uint32_t accModMember,
				RSClass * pObjClass,
				uint32_t accModObj = RSObject::modifierPublic ) ;
		// 関数引数解釈
		SSystem::SError CompileArgument
			( SSystem::SObjectArray<RSTypeInfo>& aArgType,
				FunctionBlock& block, uint32_t nFlags, RSParenthesis& prthArg ) ;
		// 適合関数検索
		ssize_t FindMatchFunctionPrototype
			( const RSFunctionObject& func,
				const SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		bool IsMatchFunctionPrototype
			( const RSFunctionPrototype& proto,
				const SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		// 関数呼び出し
		SSystem::SError CompileCallDirectFunction
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				RSFunctionObject * pFunc, size_t iFunc,
				const RSTypeInfo& typeThis,
				SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		SSystem::SError CompileInvokeFunction
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				const RSTypeInfo& typeFunc, size_t iFunc,
				const RSTypeInfo& typeThis,
				SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		SSystem::SError CompileCallVirtualFunction
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				const RSFunctionPrototype& proto,
				const wchar_t * pwszFuncName, size_t iFunc,
				const RSTypeInfo& typeThis,
				SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;
		// 関数引数正規化
		SSystem::SError CompileFunctionArgument
			( FunctionBlock& block, uint32_t nFlags,
				const RSFunctionPrototype& proto,
				SSystem::SObjectArray<RSTypeInfo>& aArgType ) ;

	protected:
		// 現在の名前空間で定義済みクラスを取得する
		RSClass * GetDefinedClassAs
			( FunctionBlock& block, const wchar_t * pwszName ) ;
		// シンボル参照
		SSystem::SError CompileAutoReferenceSymbol
			( RSTypeInfo& typeResult,
				FunctionBlock& block, RSCodeStream& cs,
				const SSystem::SString & strName, uint32_t nFlags ) ;
		// ローカル変数参照
		SSystem::SError CompileLocalReference
			( RSTypeInfo& typeResult,
				FunctionBlock& block, RSCodeStream& cs,
				const SSystem::SString & strName, uint32_t nFlags ) ;
		// クラス静メンバ参照
		SSystem::SError CompileClassStaticMember
			( RSTypeInfo& typeResult,
				FunctionBlock& block, RSCodeStream& cs,
				RSClass * pClass,
				const SSystem::SString & strMember, uint32_t nFlags ) ;
		// オブジェクトメンバロード
		SSystem::SError CompileReferenceMember
			( RSTypeInfo& typeResult,
				FunctionBlock& block,
				RSCodeStream& cs, uint32_t nFlags,
				const RSTypeInfo& typeObj, const SSystem::SString& strMember ) ;
		// クラス静メンバロード
		SSystem::SError CompileLoadClassStaticMember
			( RSTypeInfo& typeResult,
				FunctionBlock& block, uint32_t nFlags,
				RSClass * pClass, RSObject * pMember,
				const SSystem::SString& strMember, bool fOnlyNamespace ) ;
		// 配列や選択式で複数の型をマッチングする
		RSClass * MatchMultiType
			( RSClass * pLastType, const RSTypeInfo& typeNew ) const ;
		// 二つの整数型を格納できる整数型を取得する
		RSClass * MatchMultiIntegerType
			( const RSTypeInfo& typeLeft, const RSTypeInfo& typeRight ) const ;
		// 整数の値から最小の整数型を取得する
		static RSTypeInfo::NumberType NormalizeIntegerTypeOf( int64_t num ) ;

	protected:
		// 入れ子のローカル変数解放処理
		void FreeLocalNestVariable
			( FunctionBlock& block, LocalNest * pNest ) ;

	protected:
		typedef SSystem::SError
			(RSInstructionParser::*PFUNC_COMPILE_STATEMENT)
				( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		static const PFUNC_COMPILE_STATEMENT
						m_pfnCompileStatement[RSCodeControl::wiCount + 1] ;

	protected:
		// import 文
		SSystem::SError CompileStatementImport
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// class 文
		SSystem::SError CompileStatementClass
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// struct 文
		SSystem::SError CompileStatementStruct
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// for 文
		SSystem::SError CompileStatementFor
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		SSystem::SError CompileStatementForIn
			( FunctionBlock& block,
				RSCodeStream& csFor, RSCodeStream& csCode ) ;
		SSystem::SError CompileStatementForIter
			( FunctionBlock& block,
				RSCodeStream& csFor, RSCodeStream& csCode ) ;
		// while 文
		SSystem::SError CompileStatementWhile
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// do 文
		SSystem::SError CompileStatementDo
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// if 文
		SSystem::SError CompileStatementIf
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// switch 文
		SSystem::SError CompileStatementSwitch
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// case 文
		SSystem::SError CompileStatementCase
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// default 文
		SSystem::SError CompileStatementDefault
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// break 文
		SSystem::SError CompileStatementBreak
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// continue 文
		SSystem::SError CompileStatementContinue
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// try 文
		SSystem::SError CompileStatementTry
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// throw 文
		SSystem::SError CompileStatementThrow
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// return 文
		SSystem::SError CompileStatementReturn
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// with 文
		SSystem::SError CompileStatementWith
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// synchronized 文
		SSystem::SError CompileStatementSynchronized
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// static|abstract|const|public|protected|private 文
		SSystem::SError CompileStatementAccessModifier
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// var|void|boolean|byte|short|char|int|long|float|double 文
		SSystem::SError CompileStatementVar
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// function|this|super 文
		SSystem::SError CompileStatementExpression
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;
		// 不正文
		SSystem::SError CompileStatementInvalid
			( FunctionBlock& block, RSCodeStream& cstrm,
								RSCodeControl::WordIndex wiIndex ) ;

	public:
		// エラー数取得
		size_t GetErrorCount( void ) const ;
		size_t GetWarningCount( void ) const ;
		// エラー出力
		virtual void OutputError( const wchar_t * pwszErr ) ;
		void OutputErrorLog( SSystem::SParserErrorLogger& perrLog ) ;
		virtual void OnError
			( const SSystem::SString& strSrcPath,
				const SSystem::SString& strSrcLine,
				size_t iSrcLine, size_t iColIndex, const wchar_t * pwszErr ) ;
		// 警告出力
		virtual void OutputWarning( const wchar_t * pwszErr ) ;
		virtual void OnWarning
			( const SSystem::SString& strSrcPath,
				const SSystem::SString& strSrcLine,
				size_t iSrcLine, size_t iColIndex, const wchar_t * pwszErr ) ;

	public:
		// ソース位置情報取得
		static SSystem::SError GetSourcePositionInfo
			( SSystem::SString& strSrcPath,
				SSystem::SString& strSrcLine,
				size_t& iSrcIndex, size_t& iSrcLine,
				const RSParenthesis * pParenthesis, size_t iSrcInChars ) ;
		SSystem::SError GetCurrentPositionInfo
			( SSystem::SString& strSrcPath,
				SSystem::SString& strSrcLine,
				size_t& iSrcIndex, size_t& iSrcLine ) const ;

	} ;

}

#endif

