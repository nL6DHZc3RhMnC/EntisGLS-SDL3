
#if	!defined(__ROSETTA_CONTEXT_H__)
#define	__ROSETTA_CONTEXT_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// コメント
	//////////////////////////////////////////////////////////////////////////

	class	RSCodeComment
	{
	public:
		SSystem::SString		m_strText ;
		SSystem::SXMLDocument *	m_pXMLDoc ;

	public:
		// 構築関数
		RSCodeComment( const wchar_t * pwszText )
			: m_strText( pwszText ), m_pXMLDoc( NULL ) { }
		// 消滅関数
		~RSCodeComment( void )
		{
			delete	m_pXMLDoc ;
		}
		// コメント取得
		const SSystem::SString& GetComment( void ) const
		{
			return	m_strText ;
		}
		// XML 取得
		const SSystem::SXMLDocument& GetXMLDocument( void ) ;

	};


	//////////////////////////////////////////////////////////////////////////
	// 中間コード
	//////////////////////////////////////////////////////////////////////////

	class	RSCode	: public ESLObject
	{
	public:
		enum	Type
		{
			typeLiteral,
			typeControlCode,
			typeOperator,
			typeParenthesis,
			typeSymbol,
		} ;
		Type			m_type ;
		size_t			m_iSrc ;		// ソース文字指標
		RSCodeComment *	m_pComment ;	// コメント

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCode, ESLObject )
		// 構築関数
		RSCode( Type type, size_t iSrc )
				: m_type( type ), m_iSrc( iSrc ), m_pComment( NULL ) {}
		RSCode( const RSCode& code )
				: m_type( code.m_type ),
					m_iSrc( code.m_iSrc ), m_pComment( NULL ) {}
	} ;

	class	RSCodeLiteral	: public RSCode
	{
	public:
		RSObject *	m_literal ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCodeLiteral, RSCode )
		// 構築関数
		RSCodeLiteral( RSObject * pLiteral, size_t iSrc )
			: RSCode( typeLiteral, iSrc ), m_literal( pLiteral ) {}
		// 消滅関数
		virtual ~RSCodeLiteral( void )
		{
			RSObject::ReleaseRef( m_literal ) ;
		}
	} ;

	class	RSCodeControl	: public RSCode
	{
	public:
		enum	WordIndex
		{
			wiInvalid	= -1,
			wiImport,
			wiClass, wiStruct, wiFunction, wiExtends, wiImplements,
			wiFor, wiWhile, wiDo,
			wiIf, wiElse, wiSwitch, wiCase, wiDefault,
			wiBreak, wiContinue, wiTry, wiCatch, wiFinally,
			wiThrow, wiReturn, wiWith, wiSynchronized,
			wiStatic, wiAbstract, wiNative, wiConst,
			wiPublic, wiProtected, wiPrivate,
			wiVar, wiVoid, wiBoolean, wiByte, wiShort, wiChar,
			wiInt, wiLong, wiFloat, wiDouble,
			wiThis, wiSuper, wiExtern,
			wiCount,
			wiDebugPoint		= wiCount,
			wiFirstBasicType	= wiBoolean,
			wiLastBasicType		= wiDouble,
			wiBasicTypeCount	= wiLastBasicType - wiFirstBasicType + 1,
		} ;
		WordIndex	m_word ;

		static const wchar_t *	m_pwszControlWord[wiCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCodeControl, RSCode )
		// 構築関数
		RSCodeControl( WordIndex word, size_t iSrc )
			: RSCode( typeControlCode, iSrc ), m_word( word ) {}
		// 制御語
		static WordIndex IsControlWord( const wchar_t * pwszSymbol ) ;
		static const wchar_t * ControlWordAt( int index ) ;
	} ;

	class	RSCodeOperator	: public RSCode
	{
	public:
		// 演算子
		enum	OperatorIndex
		{
			opInvalid	= -1,
			opAdd, opSub, opMul, opDiv, opMod,
			opBitAnd, opBitOr, opBitXor, opBitNot,
			opShiftRight, opShiftLeft, opShiftRightArithmetic,
			opIncrement, opDecrement,
			opEqual, opNotEqual,
			opLessEqual, opLessThan,
			opGraterEqual, opGraterThan,
			opPointerEqual, opPointerNotEqual,
			opLogicalAnd, opLogicalOr, opLogicalNot,
			opMove, opMoveAdd, opMoveSub, opMoveMul, opMoveDiv, opMoveMod,
			opMoveBitAnd, opMoveBitOr, opMoveBitXor,
			opMoveShiftRight, opMoveShiftLeft, opMoveShiftRightArithmetic,
			opStaticMemberOf, opMemberOf, opMemberCallOf,
			opInstanceOf, opNew,
			opConditional, opSeparator, opSequencing,
			opEndOfStatement, opPointerMemberOf,
			opCount,
			opMoveFirst	= opMove,
			opMoveLast	= opMoveShiftRightArithmetic,
		} ;
		// 演算子
		OperatorIndex	m_operator ;

		// 演算子の優先度
		enum	OperatorPriority
		{
			priorityNothing		= 0,
			priorityList		= 1,
			prioritySeparator	= 2,
			priorityMove		= 3,
			prioritySelector	= 4,
			priorityLOr			= 5,
			priorityLAnd		= 6,
			priorityCompare		= 7,
			priorityShift		= 8,
			priorityXor			= 9,
			priorityOr			= 10,
			priorityAnd			= 11,
			priorityAdd			= 13,
			priorityMul			= 14,
			priorityUnary		= 15,
			priorityNew			= 15,
			priorityUnaryPost	= 16,
			priorityMemberCall	= 17,
			priorityMember		= 18,
			priorityNamespace	= 19,
			priorityArgument	= 20,
		} ;
		// 演算子規則
		enum	OperatorRule
		{
			ruleBinary			= 0x0001,	// 二項演算子
			ruleUnary			= 0x0002,	// 前置単項演算子
			ruleUnaryPost		= 0x0004,	// 後置単項演算子
			ruleSpecialRight	= 0x0008,	// 右辺は特殊処理（二項演算子）
			ruleRightToLeft		= 0x0010,	// 右結合（二項演算子）
		} ;
		// 演算子情報
		struct	OperatorInfo
		{
			int	flagsRule ;			// 演算子種類の組み合わせ
			int	priorityBinary ;	// 二項演算子での優先度
			int	priorityUnary ;		// 単項演算子での優先度
			int	priorityUnaryPost ;	// 単項演算子（後置）での優先度
		} ;
		static const OperatorInfo	m_infoOperators[opCount] ;
		static const wchar_t *		m_pwszOperators[opCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCodeOperator, RSCode )
		// 構築関数
		RSCodeOperator( OperatorIndex op, size_t iSrc )
			: RSCode( typeOperator, iSrc ), m_operator( op ) {}
		// 演算子情報取得
		const OperatorInfo & GetOperatorInfo( void ) const
		{
			return	m_infoOperators[m_operator] ;
		}
		// 演算子文字列取得
		const wchar_t * GetOperatorString( void ) const
		{
			return	m_pwszOperators[m_operator] ;
		}
		// 演算子
		static OperatorIndex IsOperatorWord( const wchar_t * pwszSymbol ) ;
		// 比較演算子か？
		static bool IsComparator( OperatorIndex opIndex ) ;
		// 代入演算子か？
		static bool IsMoveOperator( OperatorIndex opIndex ) ;
		// 左辺式（代入演算子・インクリメント・デクリメント）を要求するか？
		static bool IsMoveLeftOperator( OperatorIndex opIndex ) ;
	} ;

	class	RSParenthesis	: public RSCode
	{
	public:
		enum	ParenthesisType
		{
			ptInvalid		= -1,
			ptParenthesis,		// (...)
			ptBracket,			// [...]
			ptBrace,			// {...}
		} ;
		RSParenthesis *					m_parent ;
		ParenthesisType					m_parenthesis ;
		SSystem::SObjectArray<RSCode>	m_terms ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSParenthesis, RSCode )
		// 構築関数
		RSParenthesis
			( RSParenthesis * parent, ParenthesisType type, size_t iSrc )
			: RSCode( typeParenthesis, iSrc ),
				m_parent( parent ), m_parenthesis( type ) {}
		// 指定文字指標範囲内の最も若いデバッグ位置を検索
		ssize_t FindDebugPoint( size_t iFirst, size_t iEnd ) const ;
	} ;

	class	RSCodeSymbol	: public RSCode
	{
	public:
		SSystem::SString	m_symbol ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCodeSymbol, RSCode )
		// 構築関数
		RSCodeSymbol( const wchar_t * pwszSymbol, size_t iSrc )
			: RSCode( typeSymbol, iSrc ), m_symbol( pwszSymbol ) {}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// コードストリーム
	//////////////////////////////////////////////////////////////////////////

	class	RSCodeStream	: public ESLObject
	{
	protected:
		const RSParenthesis *					m_prenthesis ;
		const SSystem::SObjectArray<RSCode> *	m_terms ;
		size_t									m_index ;
		size_t									m_start ;
		size_t									m_end ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCodeStream, ESLObject )
		// 構築関数
		RSCodeStream( void ) ;
		RSCodeStream( const RSCodeStream& cs ) ;
		RSCodeStream
			( const RSParenthesis& prth,
						size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		RSCodeStream
			( const SSystem::SObjectArray<RSCode> * terms,
						size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		// 関連付け
		void AttachCode
			( const RSCodeStream& cs,
						size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		void AttachCode
			( const RSParenthesis& prth,
						size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		void AttachCode
			( const SSystem::SObjectArray<RSCode> * terms,
						size_t iFirst = 0, ssize_t iEnd = -1 ) ;
		// 関連付けられた括弧を取得
		const RSParenthesis * GetParenthesis( void ) const ;
		// 終端に到達しているか？
		bool IsEndOfStream( void ) const ;
		// 指標
		size_t GetIndex( void ) const ;
		void SeekIndex( size_t iIndex ) ;
		// 項取得
		RSCode * GetTerm( size_t iOffset = 0 ) ;
		// 項取得と指標の移動
		RSCode * NextTerm( size_t iOffset = 0 ) ;
		// 次の項がリテラルの場合、それを返し指標を次に移動する
		RSCodeLiteral * NextLiteral( void ) ;
		// 次の項が括弧の場合、それを返し指標を次に移動する
		RSParenthesis * NextParenthesis
			( RSParenthesis::ParenthesisType
					type = RSParenthesis::ptInvalid ) ;
		// 次の項が演算子の場合、それを返し指標を次に移動する
		RSCodeOperator * NextOperator
			( RSCodeOperator::OperatorIndex
					opIndex = RSCodeOperator::opInvalid ) ;
		// 次の項が制御語の場合、それを返し指標を次に移動する
		RSCodeControl * NextControlWord
			( RSCodeControl::WordIndex wiIndex = RSCodeControl::wiInvalid ) ;
		RSCodeControl * NextStatementControlWord( void ) ;
		// 次の項がシンボルの場合、それを返し指標を次に移動する
		RSCodeSymbol * NextSymbol( const wchar_t * pwszSymbol = NULL ) ;
		// 括弧を文末まで検索する
		RSParenthesis * FindParenthesis
			( RSParenthesis::ParenthesisType
					type = RSParenthesis::ptInvalid ) ;
		// 演算子を文末まで検索する
		RSCodeOperator * FindOperator
			( RSCodeOperator::OperatorIndex
					opIndex = RSCodeOperator::opInvalid ) ;
		// １文読み飛ばす
		void PassAStatement( void ) ;
		// コメント取得
		RSCodeComment * FindCommentFrom( size_t iIndex ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 標準的なソースコード・パーサー
	//////////////////////////////////////////////////////////////////////////

	class	RSSourceParser	: public SSystem::SStringParser
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSourceParser, SStringParser )
		// 構築関数
		RSSourceParser( void ) ;
		RSSourceParser( const SStringParser& ss ) ;
		RSSourceParser( const SString& src ) ;
		RSSourceParser( const wchar_t * pszSrc, ssize_t nLength = -1 ) ;

	public:
		// 現在のトークンを通過する
		virtual TokenType PassToken( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スクリプト
	//////////////////////////////////////////////////////////////////////////

	class	RSScript	: public RSParenthesis
	{
	public:
		// ディレクティブ入れ子
		enum	DirectiveType
		{
			directiveIf,
			directiveElseIf,
			directiveElse,
			directiveEndIf,
		} ;
		enum	DirectiveCondition
		{
			conditionPending,		// まだ条件一致ブロックはない
			conditionElse,			// 既に条件一致ブロックがあった
			conditionTrue,			// 現在が条件一致ブロック
		} ;
		struct	DirectiveNest
		{
			DirectiveType		m_type ;
			DirectiveCondition	m_condition ;
		} ;

	protected:
		SSystem::SObjectArray<DirectiveNest>	m_nestDir ;
		SSystem::SObjectArray<RSCodeComment>	m_arrCommentStock ;
		SSystem::SString	m_strSrcPath ;
		SSystem::SString	m_strSource ;
		bool				m_flagDebug ;

	public:
		// インポート・ライブラリ
		class	ImportLibrary
		{
		public:
			SSystem::SString	m_strPath ;
			SSystem::SString	m_strType ;		// javascript / rosetta
			SSystem::SString	m_strEncoding ;	// utf-8 / shift_jis / euc-jp
		} ;

	protected:
		SSystem::SStrSortObjectArray<ImportLibrary>	m_aImportLibs ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSScript, RSParenthesis )
		// 構築関数
		RSScript( void )
			: RSParenthesis( NULL, ptInvalid, 0 ), m_flagDebug( false ) {}
		// デバッグフラグ
		bool IsDebugFlag( void ) const
		{
			return	m_flagDebug ;
		}
		void SetDebugFlag( bool flagDebug )
		{
			m_flagDebug = flagDebug ;
		}

	public:
		// ソースファイルパス
		const SSystem::SString& GetSourcePath( void ) const
		{
			return	m_strSrcPath ;
		}
		void SetSourcePath( const wchar_t * pwszSrcPath )
		{
			m_strSrcPath = pwszSrcPath ;
		}
		// ソーステキスト
		const SSystem::SString& GetSourceText( void ) const
		{
			return	m_strSource ;
		}

	public:
		// スクリプト読み込み
		virtual SSystem::SError LoadScript
			( RSContext& ctxMacro,
				const wchar_t * pwszFilePath,
				SSystem::SParserErrorInterface& perr ) ;
		// スクリプト解釈
		virtual SSystem::SError ParseScript
			( RSContext& ctxMacro,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		virtual SSystem::SError ParseSource
			( RSContext& ctxMacro,
				SSystem::SString& strSource,
				SSystem::SParserErrorInterface& perr ) ;
		virtual SSystem::SError ParseSource
			( RSContext& ctxMacro,
				const wchar_t * pwszSource,
				SSystem::SParserErrorInterface& perr ) ;

	public:
		// スクリプト解釈（低水準）
		enum	ParseFlag
		{
			parserNoComment		= 0x0001,
			parserNoDirective	= 0x0002,
		} ;
		virtual SSystem::SError ParseSource
			( RSParenthesis * pprthTarget,
				RSContext& ctxMacro, uint32_t flagsParser,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
	protected:
		// #if ディレクティブ
		void ParseDirectiveIf
			( RSContext& ctxMacro,
				RSSourceParser& sparsDir,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		// #elseif ディレクティブ
		void ParseDirectiveElseIf
			( RSContext& ctxMacro,
				RSSourceParser& sparsDir,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		// #else ディレクティブ
		void ParseDirectiveElse
			( RSSourceParser& sparsDir,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		// #endif ディレクティブ
		void ParseDirectiveEndIf
			( RSSourceParser& sparsDir,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		// #define ディレクティブ
		void ParseDirectiveDefine
			( RSContext& ctxMacro,
				RSSourceParser& sparsDir,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		// #import ディレクティブ
		void ParseDirectiveImport
			( RSParenthesis * pprthTarget,
				RSContext& ctxMacro,
				RSSourceParser& sparsDir,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		// #importlib ディレクティブ
		void ParseDirectiveImportLib
			( RSSourceParser& sparsDir,
				SSystem::SStringParser& sparsSource,
				SSystem::SParserErrorInterface& perr ) ;
		// コメント追加
		RSCodeComment * AddCodeComment
				( RSCode * pCode, SSystem::SString& strComment ) ;

	public:
		// ディレクティブ条件判定
		bool IsEnabledDirectiveBlock( void ) const ;

	public:
		// コードのソース取得
		static RSScript * GetScriptOf( RSParenthesis * pPrth ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 関数プロトタイプ
	//////////////////////////////////////////////////////////////////////////

	class	RSFunctionPrototype	: public ESLObject
	{
	public:
		enum	PrototypeFlag
		{
			flagVarArg			= 0x0001,
			flagArgTypeMask		= 0x0001,
			flagSynchronized	= 0x0010,
			flagConstant		= 0x0020,
			flagAbstract		= 0x0040,
			flagAccessPublic	= 0x0000,
			flagAccessProtected	= 0x0100,
			flagAccessPrivate	= 0x0200,
			flagAccessMask		= 0x0300,
		} ;
		uint32_t								m_nFlags ;
		bool									m_flagCompiled ;
		SSystem::SPointerArray<RSClass>			m_aArgTypes ;
		SSystem::SObjectArray<SSystem::SString>	m_aArgNames ;
		SSystem::SPointerArray<RSObject>		m_aArgDefault ;
		RSClass *								m_pReturnType ;
		RSObject *								m_pRefNamespace ;
		RSParenthesis *							m_pParenthesis ;	// 関数コード
		int64_t									m_pfnFuncAddr ;		// Sakura2 コード
		RSObject::METHOD_ENTRY					m_methodNative ;	// ネイティブコード
		RSFunctionObject *						m_pFuncGroup ;
		RSClass *								m_pNamespaceClass ;
		RSCodeComment *							m_pComment ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSFunctionPrototype, ESLObject )
		// 構築関数
		RSFunctionPrototype( void ) ;
		RSFunctionPrototype( const RSFunctionPrototype& proto ) ;
		// 消滅関数
		virtual ~RSFunctionPrototype( void ) ;
		// 参照チェーンを解放
		void ReleaseReferenceChain( void ) ;
		// synchronized 修飾
		bool IsSynchronizedModifier( void ) const
		{
			return	(m_nFlags & flagSynchronized) != 0 ;
		}
		// const 修飾
		bool IsConstantModifier( void ) const
		{
			return	(m_nFlags & flagConstant) != 0 ;
		}
		// 名前空間設定
		void SetRefNamespace( RSObject * pObj ) ;
		// 返り値型設定
		void SetReturnType( RSClass * pClass ) ;
		// 引数追加
		void AddArgument
			( RSClass * pClass,
				const wchar_t * pwszName, RSObject * pDefault = NULL ) ;
		// 書式解釈
		SSystem::SError ParseArgument
			( RSContext& context,
				RSCodeStream& cs, SSystem::SParserErrorInterface& perr ) ;
		SSystem::SError ParseArgument
			( RSContext& context,
				const wchar_t * pwszArgList,
				SSystem::SParserErrorInterface& perr ) ;
		// 引数型一致判定
		bool IsEqualArgumentTypes( const RSFunctionPrototype& proto ) const ;
		// 関数呼び出し引数判定
		bool IsMatchPrototype( RSObject*const* ppArgs, size_t countArg ) ;
		// 同一（実装）関数判定
		bool IsEqualImplementFunc( const RSFunctionPrototype& proto ) const ;
		// 引数文字列表記
		SSystem::SString FormatArgument( void ) const ;
		// コンパイル済み（コンパイル試行済み）か？
		bool IsCompiledImplement( void ) const
		{
			return	m_flagCompiled ;
		}
		void SetCompiledFlag( void )
		{
			m_flagCompiled = true ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 関数オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSFunctionObject	: public RSDynamicObject
	{
	public:
		SSystem::SString		m_strFuncName ;
		RSClass *				m_pFuncClass ;

		RSFunctionPrototype *	m_pPrototype ;
		SSystem::SObjectArray<RSFunctionPrototype>
								m_arrPrototypes ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSFunctionObject, RSDynamicObject )
		// 構築関数
		RSFunctionObject( RSClass * pClass ) ;
		RSFunctionObject( RSFunctionPrototype * pProto, RSClass * pClass ) ;
		RSFunctionObject( RSContext& context, const RSFunctionObject& func ) ;
		// 消滅関数
		virtual ~RSFunctionObject( void ) ;
		// プロトタイプ追加
		size_t AddPrototype
			( RSFunctionPrototype * pProto, bool fOverride = true ) ;
		// 一致プロトタイプ検索
		RSFunctionPrototype *
			GetEqualArgumentPrototype( const RSFunctionPrototype & proto ) const ;
		ssize_t FindEqualArgumentPrototype
			( const RSFunctionPrototype & proto ) const ;
		// 適合プロトタイプ取得
		RSFunctionPrototype *
			GetMatchPrototype( RSObject*const* ppArgs, size_t countArg ) ;
		// 参照チェーンを解放
		void ReleaseReferenceChain( void ) ;
		// 実装一致（オーバーライドされていないか？）判定
		bool IsEqualImplementFunc( const RSFunctionObject& func ) const ;
		bool IsEqualImplementFunc( const RSFunctionPrototype& proto ) const ;

	public:
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// テンポラルなコード（単一の数式）
	//////////////////////////////////////////////////////////////////////////

	class	RSExpressionScript	: public RSScript
	{
	protected:
		RSObject *	m_pObj ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSExpressionScript, RSScript )
		// 構築関数
		RSExpressionScript( void ) ;
		// 消滅関数
		virtual ~RSExpressionScript( void ) ;

	public:
		// コードの解釈と実行
		virtual SSystem::SError Execute
			( RSVirtualMachine * pVM,
				const wchar_t * pwszExpr,
				SSystem::SParserErrorInterface& perr ) ;
		// 式の表価値を取得
		RSObject * GetObject( void ) const
		{
			return	m_pObj ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 基底コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	RSContext	: public SSystem::SObject
	{
	public:
		// デバッグ用リスナ
		class	DebugListener	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( DebugListener, ESLObject )
			// デバッグ
			virtual void OnDebug
				( RSContext * context,
					const RSScript * pScript, size_t iSrcChar ) = 0 ;
			// 関数呼び出し
			virtual void OnCall
				( RSContext * context, RSFunctionPrototype * pProto ) = 0 ;
			// 関数復帰
			virtual void OnReturned
				( RSContext * context,
					const RSScript * pScript, size_t iSrcChar ) = 0 ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSContext, SObject )
		// 構築関数
		RSContext( RSVirtualMachine * pVM, RSObject * pThread = NULL ) ;
		// 消滅関数
		virtual ~RSContext( void ) ;

	protected:
		// 仮想マシン
		RSVirtualMachine *	m_pVM ;

		// スレッドオブジェクト
		RSObject *	m_pThread ;
		ECSSakura2::ThreadObject *	m_pcsThread ;

		// 基本型のクラス
		RSClass *	m_pClassClass ;
		RSClass *	m_pVarClass ;
		RSClass *	m_pFuncClass ;
		RSClass *	m_pBooleanClass ;
		RSClass *	m_pIntegerClass ;
		RSClass *	m_pNumberClass ;
		RSClass *	m_pStringClass ;
		RSClass *	m_pArrayClass ;
		RSClass *	m_pArrayBufferClass ;
		RSClass *	m_pExceptionClass ;
		RSClass *	m_pJObjectClass ;
		RSClass *	m_pJSObjectClass ;
		RSClass *	m_pStructureClass ;
		RSClass *	m_pBasicTypeClass[RSCodeControl::wiBasicTypeCount] ;
		RSClass *	m_pPtrTypeClass[RSReferenceNumber::typeCountOfNumber] ;

		// 参照チェーン
		struct	NAMESPACE_NEST
		{
			RSFunctionPrototype *	pRootFunc ;
			bool		fRootFunc ;
			RSObject *	pNamespace ;
			RSObject *	pThis ;
			uint32_t	accModifier ;
		} ;
		SSystem::SArray<NAMESPACE_NEST>	m_arrWith ;

		// 実行中のコード
		const RSParenthesis *	m_pExceptionParenthesis ;
		size_t					m_iExceptionStatement ;
		const RSParenthesis *	m_pCurParenthesis ;
		size_t					m_iSrcStatement ;

		// this オブジェクト
		RSObject *	m_pThisObj ;
		RSClass *	m_pThisClass ;

		// デバッグ用リスナ
		DebugListener *			m_pDebugListener ;
		const RSParenthesis *	m_pLastDebugPrth ;
		const RSScript *		m_pLastDebugScript ;

		// オブジェクト・バッファ
		struct	OBJECT_BUFFER
		{
			enum	BufferConstant
			{
				countBuffer	= 16,
			} ;
			RSObject *	pObjBuf[countBuffer] ;
			size_t		nBuffered ;

			OBJECT_BUFFER( void ) : nBuffered(0) { }
			~OBJECT_BUFFER( void )
			{
				for ( size_t i = 0; i < nBuffered; i ++ )
				{
					RSObject::ReleaseRef( pObjBuf[i] ) ;
				}
			}
			RSObject * New( void )
			{
				if ( nBuffered > 0 )
				{
					return	pObjBuf[-- nBuffered] ;
				}
				return	NULL ;
			}
			void Free( RSContext& context, RSObject * pObj )
			{
				if ( nBuffered < countBuffer )
				{
					pObj->AddRef() ;
					pObj->DisposeObject( context ) ;
					if ( nBuffered < countBuffer )
					{
						pObjBuf[nBuffered ++] = pObj ;
					}
					else
					{
						delete	pObj ;
					}
				}
				else
				{
					delete	pObj ;
				}
			}
		} ;
		OBJECT_BUFFER	m_objbufBoolean ;
		OBJECT_BUFFER	m_objbufInteger ;
		OBJECT_BUFFER	m_objbufNumber ;
		OBJECT_BUFFER	m_objbufString ;
		OBJECT_BUFFER	m_objbufArray ;
		OBJECT_BUFFER	m_objbufReference ;
		OBJECT_BUFFER	m_objbufPointer ;
		OBJECT_BUFFER	m_objbufReferenceNumber ;
		OBJECT_BUFFER	m_objbufPointerNumber ;
		OBJECT_BUFFER	m_objbufStructuredPointer ;
		OBJECT_BUFFER	m_objbufNamespace ;

		// 式中の this 参照
		RSObject *	m_pExprParentOf ;

		// 関数返り値
		RSObject *	m_pRetValue ;

		// 例外エラー
		RSObject *	m_pException ;

		// 文頭コメント
		RSCodeComment *	m_pLastComment ;

		// 動作フラグ
		uint32_t	m_flagsBehavior ;

	public:
		// スレッドオブジェクト関連付け
		void AttachThreadObject( RSObject * pThread ) ;
		// スレッドオブジェクト取得
		RSObject * GetThreadObject( void ) ;
		// デバッグリスナ設定
		void AttachDebugListener( DebugListener * pListener ) ;
		// デバッグリスナ解除
		void DetachDebugListener( DebugListener * pListener ) ;

	public:
		// 文実行（名前空間を作成して文を実行）
		virtual void PerformStatement
			( RSCodeStream& cstrm,
				RSObject * pThisObj = NULL,
				SSystem::SParserErrorInterface* pperr = NULL ) ;
		// 式実行（名前空間を作成して式を評価）
		virtual RSObject * PerformExpression
			( const wchar_t * pwszExpr,
				RSObject * pThisObj = NULL,
				SSystem::SParserErrorInterface* pperr = NULL ) ;
		virtual RSObject * PerformExpression
			( RSCodeStream& cstrm, RSObject * pThisObj = NULL ) ;
		// 関数実行（名前空間を作成して式を評価）
		virtual RSObject * PerformFunction
			( RSFunctionObject& func,
				RSObject * pThisObj,
				RSObject**const ppArgs, size_t countArg,
				SSystem::SParserErrorInterface* pperr = NULL ) ;
		// 例外発生ソース位置情報取得
		virtual SSystem::SError GetExceptionPositionInfo
			( SSystem::SString& strSrcPath,
				SSystem::SString& strSrcLine,
				size_t& iSrcIndex, size_t& iSrcLine ) ;
		// 実行中のソース位置情報取得
		virtual SSystem::SError GetCurrentPositionInfo
			( SSystem::SString& strSrcPath,
				SSystem::SString& strSrcLine,
				size_t& iSrcIndex, size_t& iSrcLine ) ;
		// ソース位置情報取得
		SSystem::SError GetSourcePositionInfo
			( SSystem::SString& strSrcPath,
				SSystem::SString& strSrcLine,
				size_t& iSrcIndex, size_t& iSrcLine,
				const RSParenthesis * pParenthesis, size_t iSrcInChars ) ;
	public:
		// 関数実行
		virtual RSObject * CallFunction
			( RSFunctionObject& func,
				RSObject * pThisObj,
				RSObject*const* ppArgs, size_t countArg,
				bool fStructCast, bool* pArgMatchResult = NULL ) ;
		virtual RSObject * CallFunction
			( RSFunctionPrototype * pProto,
				RSObject * pThisObj,
				RSObject*const* ppArgs, size_t countArg,
				bool fStructCast ) ;
		virtual RSObject * CallFunction
			( RSFunctionObject& func,
				RSObject * pThisObj, RSObject& arrayArg,
				bool fStructCast, bool* pArgMatchResult = NULL ) ;
		// メソッド実行
		virtual RSObject * CallMethod
			( RSObject * pThisObj,
				RSClass * pThisClass,
				const wchar_t * pwszFuncName,
				RSObject*const* ppArgs, size_t countArg,
				bool fStructCast = true, bool* pArgMatchResult = NULL ) ;
		virtual RSObject * CallMethod
			( RSObject * pThisObj,
				const wchar_t * pwszFuncName,
				RSObject*const* ppArgs, size_t countArg,
				bool fStructCast = true, bool* pArgMatchResult = NULL ) ;
		// 全文実行
		void ExecuteAllStatements
				( RSCodeStream& cstrm, RSClass * pDefClass = NULL ) ;
		// 文実行
		void ExecuteStatements
				( RSCodeStream& cstrm, RSClass * pDefClass = NULL ) ;
		// 一文実行
		void ExecuteAStatement
				( RSCodeStream& cstrm, RSClass * pDefClass = NULL ) ;
		// 定義文解釈／実行
		enum	DeclareModeFlag
		{
			declModeVar		= 0x0001,
			declModeFunc	= 0x0002,
			declModeAny		= 0x0003,
		} ;
		void ExecuteDeclareVariable
			( RSCodeStream& cstrm, uint32_t accMod,
					RSClass * pClass,
					uint32_t modeDecl = declModeAny,
					RSClass * pDefClass = NULL,
					RSCodeComment * pDefComment = NULL ) ;
		// 数式評価
		virtual RSObject *
			EvaluateExpression
				( RSCodeStream& cs,
					int priority = RSCodeOperator::priorityNothing ) ;
		RSObject *
			EvaluateExpression
				( SSystem::SStringParser& sparsExpr,
					SSystem::SParserErrorInterface* pperr = NULL ) ;
		RSObject *
			EvaluateExpression
				( const wchar_t * pwszExpr,
					SSystem::SParserErrorInterface* pperr = NULL ) ;
		// 数式を読み飛ばす
		static void PassExpression( RSCodeStream& cs, int priority ) ;
		// 型式を評価
		RSClass * ParseClassExpression( RSCodeStream& cs ) ;
		RSObject * ParseTypeExpression( RSCodeStream& cs ) ;
		// 組み込みジェネリック型修飾
		RSClass * ParseGenericClassDecoration( RSCodeStream& cs, RSClass * pClass ) ;
		// 配列型修飾
		RSClass * ParseClassArrayDecoration( RSCodeStream& cs, RSClass * pClass ) ;
		// 配列リテラル評価
		RSObject * EvaluateArrayExpression( RSParenthesis& prth ) ;
		// 連想配列リテラル評価
		RSObject * EvaluateMapArrayExpression( RSParenthesis& prth ) ;
		// 関数リテラル評価
		RSObject * EvaluateFunctionObject( RSCodeStream& cs ) ;
		// new 演算子実行
		RSObject * ExecuteNewOperator( RSCodeStream& cs ) ;
		// 単項演算子実行
		RSObject * ExecuteUnaryOperator
			( RSObject * pObj, RSCodeOperator::OperatorIndex opIndex ) ;
		// 二項演算子実行
		RSObject * ExecuteBinaryOperator
			( RSObject * pObjLeft,
				RSObject * pObjRight, RSCodeOperator::OperatorIndex opIndex ) ;

	protected:
		typedef void (RSContext::*PFUNC_EXECUTE_STATEMENT)
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		static const PFUNC_EXECUTE_STATEMENT
						m_pfnExecuteStatement[RSCodeControl::wiCount + 1] ;
	public:
		// import 文
		void ExecuteStatementImport
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// class 文
		void ExecuteStatementClass
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// struct 文
		void ExecuteStatementStruct
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// function 文
		void ExecuteStatementFunction
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// for 文
		void ExecuteStatementFor
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// while 文
		void ExecuteStatementWhile
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// do 文
		void ExecuteStatementDo
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// if 文
		void ExecuteStatementIf
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// switch 文
		void ExecuteStatementSwitch
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// case 文
		void ExecuteStatementCase
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// default 文
		void ExecuteStatementDefault
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// break 文
		void ExecuteStatementBreak
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// continue 文
		void ExecuteStatementContinue
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// try 文
		void ExecuteStatementTry
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// throw 文
		void ExecuteStatementThrow
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// return 文
		void ExecuteStatementReturn
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// with 文
		void ExecuteStatementWith
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// synchronized 文
		void ExecuteStatementSynchronized
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// static|abstract|const|public|protected|private 文
		void ExecuteStatementAccessModifier
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// var|void|boolean|byte|short|char|int|long|float|double 文
		void ExecuteStatementVar
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// this|super 文
		void ExecuteStatementExpression
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// デバッグ用
		void ExecuteStatementDebug
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;
		// 不正文
		void ExecuteStatementInvalid
			( RSCodeStream& cstrm,
				RSCodeControl::WordIndex wiIndex, RSClass * pDefClass ) ;

	protected:
		// RSParenthesis -> RSScript（デバッグ用）
		const RSScript *
			DebugScriptFromParenthesis( const RSParenthesis * pPrth ) ;

	public:
		// 引数評価
		void EvaluateArgument( RSObject& arg, RSCodeStream& cs ) ;
		void EvaluateArgument( RSObject& arg, RSParenthesis& prth ) ;

	public:
		// 脱出判定
		enum	EscapeStatus
		{
			escapeNothing,
			escapeAbort,
			escapeReturn,
			escapeBreak,
			escapeContinue,
		} ;
	protected:
		// 返り値／脱出条件保存
		class	EscapeContextSaver
		{
		public:
			RSContext&		m_context ;
			EscapeStatus	m_escape ;
			RSObject *		m_pRetValue ;
		public:
			EscapeContextSaver( RSContext& context )
				: m_context( context )
			{
				m_escape = context.GetEscape() ;
				m_pRetValue = NULL ;
				if ( m_escape == escapeReturn )
				{
					m_pRetValue = context.PopReturnObject() ;
				}
			}
			~EscapeContextSaver( void )
			{
				if ( (m_escape == escapeReturn)
					&& !m_context.IsException()
					&& (m_context.GetEscape() == escapeNothing) )
				{
					m_context.SetReturnObject( m_pRetValue ) ;
				}
				else
				{
					m_context.ReleaseObjectRef( m_pRetValue ) ;
					m_pRetValue = NULL ;
					m_context.SetEscape( m_escape ) ;
				}
			}
		} ;

		EscapeStatus	m_escape ;
	public:
		EscapeStatus GetEscape( void ) const
		{
			return	m_escape ;
		}
		void SetEscape( EscapeStatus escape )
		{
			m_escape = escape ;
		}
		// 関数脱出判定
		bool IsReturn( void ) const ;
		// 処理の強制中断設定
		void SetAbort( void ) ;
		// 返り値取得
		RSObject * PopReturnObject( void ) ;
		// 返り値設定
		void SetReturnObject( RSObject * pObj ) ;
		// 例外発生判定
		bool IsException( void ) const ;
		// 例外取得
		RSObject * GetException( void ) const ;
		RSObject * PopException( void ) ;
		// 例外エラー設定
		void SetException( RSObject * pException ) ;
		void ThrowExceptionError
			( const wchar_t * pwszErrMsg,
				const wchar_t * pwszClassName = NULL ) ;
		// 例外エラー削除
		void ClearException( void ) ;
		// 例外エラーがある場合、これをエラー出力し、例外エラーをクリア
		void OutputExceptionError( SSystem::SParserErrorInterface& perr ) ;
		void OutputExceptionError
			( SSystem::SParserErrorInterface& perr, const wchar_t * pwszInline ) ;
		// 実行中コード設定
		void SetCurrentParenthesis( const RSParenthesis * pPrth )
		{
			m_pCurParenthesis = pPrth ;
		}
		void SetCurrentStatement( RSCode * pcd )
		{
			m_iSrcStatement = pcd->m_iSrc ;
		}
		void SetCurrentStatementIndex( size_t iSrc )
		{
			m_iSrcStatement = iSrc ;
		}
		void SetExceptionParenthesis( const RSParenthesis * pPrth )
		{
			m_pExceptionParenthesis = pPrth ;
		}
		void SetExceptionStatementIndex( size_t iSrc )
		{
			m_iExceptionStatement = iSrc ;
		}

	public:
		// 待機処理（脱出判定付）
		SSystem::SError WaitSynchronism
			( SSystem::SSynchronism& sync, int64_t nTimeout ) ;
		// 一定時間待機（脱出判定付）
		SSystem::SError SleepMilliSec( int64_t nTimeout ) ;

	public:
		// 参照チェーン追加
		void PushNamespace
			( RSObject * pThisObj, RSObject * pNamespace,
				RSObject::AccessModifier accMod = RSObject::modifierPublic,
				bool fRootFunc = false, RSFunctionPrototype * pRootFunc = NULL ) ;
		// 参照チェーン削除
		void PopNamespace( void ) ;
		// 現在の名前空間参照チェーン取得
		RSObject * GetCurrentNamespace( void ) ;
		// ローカル変数空間を取得する
		RSFunctionPrototype * GetLocalNamespaces
			( SSystem::SPointerArray<RSObject>& lstNamespaces ) ;
		// this を取得する
		RSObject * GetThisObject( void ) const
		{
			return	m_pThisObj ;
		}
		// スタックダンプを出力する
		void DebugDumpStack
			( SSystem::SFileInterface& dump, const wchar_t * pwszIndent ) ;

	public:
		// 仮想マシン取得
		RSVirtualMachine * GetVM( void ) const
		{
			return	m_pVM ;
		}
		// 動作フラグ
		enum	BehaviorFlag
		{
			behaveNoInteger				= 0x0001,
			behaveAutoEndOfStatement	= 0x0002,
		} ;
		// 動作フラグ取得
		uint32_t GetBehaviorFlags( void ) const ;
		bool IsBehavior( int nFlags ) const ;
		// 動作フラグ変更
		uint32_t ModifyBehaviorFlags
			( uint32_t flagsAdd, uint32_t flagsRemove ) ;

	public:
		// 変数参照
		RSObject * GetVariableAs( const wchar_t * pwszName ) ;
		// 変数生成／設定
		void CreateVariableAs( const wchar_t * pwszName, RSObject * pObj ) ;
		// メンバへのアクセス保護判定と例外のスロー
		bool VerifyMemberAccessModifier
			( RSObject * pObj,
				RSObject * pMember, const wchar_t * pwszName ) ;

	public:
		// クラス取得
		RSClass * GetClassAs( const wchar_t * pwszClassName ) ;
		RSClass * GetClassClass( void ) const
		{
			return	m_pClassClass ;
		}
		RSClass * GetVariableClass( void ) const
		{
			return	m_pVarClass ;
		}
		RSClass * GetFunctionClass( void ) const
		{
			return	m_pFuncClass ;
		}
		RSClass * GetBooleanClass( void ) const
		{
			return	m_pBooleanClass ;
		}
		RSClass * GetIntegerClass( void ) const
		{
			return	m_pIntegerClass ;
		}
		RSClass * GetNumberClass( void ) const
		{
			return	m_pNumberClass ;
		}
		RSClass * GetStringClass( void ) const
		{
			return	m_pStringClass ;
		}
		RSClass * GetArrayClass( void ) const
		{
			return	m_pArrayClass ;
		}
		RSClass * GetArrayBufferClass( void ) const
		{
			return	m_pArrayBufferClass ;
		}
		RSClass * GetExceptionClass( void ) const
		{
			return	m_pExceptionClass ;
		}
		RSClass * GetGenericObjectClass( void ) const
		{
			return	m_pJObjectClass ;
		}
		RSClass * GetDynamicObjectClass( void ) const
		{
			return	m_pJSObjectClass ;
		}
		RSClass * GetStructureClass( void ) const
		{
			return	m_pStructureClass ;
		}
		RSClass * GetBasicTypeClass( RSCodeControl::WordIndex wiIndex ) const
		{
			ESLAssert( wiIndex >= RSCodeControl::wiFirstBasicType ) ;
			ESLAssert( wiIndex <= RSCodeControl::wiLastBasicType ) ;
			return	m_pBasicTypeClass
						[wiIndex - RSCodeControl::wiFirstBasicType] ;
		}
		RSClass * GetTypedPointerClass( RSReferenceNumber::NumberType type ) const
		{
			return	m_pPtrTypeClass[type] ;
		}
		RSClass * GetIntClassSizeOf( size_t nBytes ) const ;
		RSClass * GetFloatClassSizeOf( size_t nBytes ) const ;

	public:	// オブジェクト生成
		// Boolean オブジェクト生成
		virtual RSBoolean * new_Boolean( bool boolValue ) ;
		// Integer オブジェクト生成
		virtual RSObject * new_Integer
			( int64_t numValue = 0,
				RSInteger::IntegerType typeInt = RSInteger::typeInt64 ) ;
		// Number オブジェクト生成
		virtual RSNumber * new_NumberInt32( int32_t numValue ) ;
		virtual RSNumber * new_Number( double numValue ) ;
		// String オブジェクト生成
		virtual RSString * new_String( const wchar_t * pwszValue = NULL ) ;
		// Reference オブジェクト生成
		virtual class RSReference * new_Reference( RSObject * pRef ) ;
		// Pointer オブジェクト生成
		virtual class RSPointer * new_Pointer( RSObject * pRef, RSClass * pClass = NULL ) ;
		// ReferenceNumber オブジェクト生成
		virtual RSReferenceNumber * new_ReferenceNumber
			( void * ptrBuf, RSReferenceNumber::NumberType type, RSObject * pRef ) ;
		// PointerNumber オブジェクト生成
		virtual RSTypedArrayPointer * new_PointerNumber
			( RSArrayBuffer * pBuf,
				RSReferenceNumber::NumberType type,
				size_t iOffset = 0, ssize_t nLimit = -1 ) ;
		// StructuredPointer オブジェクト生成
		virtual RSStructuredPointer * new_StructuredPointer
			( RSStructuredPointerClass * pType,
				RSArrayBuffer * pBuf,
				size_t iOffset = 0, ssize_t nLimit = -1 ) ;
		virtual RSStructuredPointer * new_StructuredPointer
			( const wchar_t * pwszClass, size_t nLength = 1 ) ;
		// Array オブジェクト生成
		virtual class RSArray * new_Array
			( size_t nLimit = 0x7FFFFFFF, RSClass * pProto = NULL ) ;
		// Exception オブジェクト生成
		virtual class RSException * new_Exception( const wchar_t * pwszErr ) ;
		virtual class RSException * new_Exception
					( const wchar_t * pwszErr, const wchar_t * pwszClass ) ;
		virtual class RSException * new_Exception
					( const wchar_t * pwszErr, RSClass * pClass ) ;
		// オブジェクト生成
		virtual RSObject * new_Object( const wchar_t * pwszClass ) ;
		virtual RSObject * new_Object( RSClass * pClass ) ;
		virtual RSPointer * new_ObjectPointer( const wchar_t * pwszClass ) ;
		// 名前空間オブジェクト生成
		virtual RSNamespace * new_Namespace( void ) ;
		virtual RSNamespace * new_Namespace
			( RSObject * pRefSpace, uint32_t accMod, RSObject * pBackLink ) ;
		// オブジェクト参照開放
		void ReleaseObjectRef( RSObject * pObj ) ;

	public:
		// 配列要素取得
		int64_t GetObjElementIntegerAt
			( RSObject * pObj, int nIndex,
				int64_t nDefault = 0, bool * pError = NULL ) ;
		double GetObjElementNumberAt
			( RSObject * pObj, int nIndex,
				double nDefault = 0, bool * pError = NULL ) ;
		SSystem::SString GetObjElementStringAt
			( RSObject * pObj, int nIndex,
				const wchar_t * pwszDefault = NULL, bool * pError = NULL ) ;
		// 配列要素設定
		SSystem::SError SetObjElementIntegerAt
			( RSObject * pObj, int nIndex, int64_t nValue ) ;
		SSystem::SError SetObjElementNumberAt
			( RSObject * pObj, int nIndex, double nValue ) ;
		SSystem::SError SetObjElementStringAt
			( RSObject * pObj, int nIndex, const wchar_t * pwszValue ) ;
		// メンバ要素取得
		int64_t GetObjMemberIntegerAs
			( RSObject * pObj, const wchar_t * pwszMember,
				int64_t nDefault = 0, bool * pError = NULL ) ;
		double GetObjMemberNumberAs
			( RSObject * pObj, const wchar_t * pwszMember,
				double nDefault = 0, bool * pError = NULL ) ;
		SSystem::SString GetObjMemberStringAs
			( RSObject * pObj, const wchar_t * pwszMember,
				const wchar_t * pwszDefault = NULL, bool * pError = NULL ) ;
		// メンバ要素設定
		SSystem::SError SetObjMemberIntegerAs
			( RSObject * pObj, const wchar_t * pwszMember, int64_t nValue ) ;
		SSystem::SError SetObjMemberNumberAs
			( RSObject * pObj, const wchar_t * pwszMember, double nValue ) ;
		SSystem::SError SetObjMemberStringAs
			( RSObject * pObj,
				const wchar_t * pwszMember, const wchar_t * pwszValue ) ;

	public:
		// printf パラメータ
		class	SArgList : public SSystem::SStringFormatSupplier
		{
		protected:
			RSObject**const	m_ppArgs ;
			size_t			m_countArg ;
			size_t			m_iNextArg ;

		public:
			// 構築関数
			SArgList( RSObject**const ppArgs, size_t countArg ) ;
			// 整数取得
			int IntAt( size_t iArg, int nDefault = 0 ) const ;
			int NextInt( int nDefault = 0 ) ;
			int64_t LongAt( size_t iArg, int64_t nDefault = 0 ) const ;
			int64_t NextLong( int64_t nDefault = 0 ) ;
			// ブール値取得
			bool BooleanAt( size_t iArg, bool fDefault = false ) const ;
			bool NextBoolean( bool fDefault = false ) ;
			// 浮動小数点取得
			double DoubleAt( size_t iArg, double nDefault = 0.0 ) const ;
			double NextDouble( double nDefault = 0.0 ) ;
			// 文字列取得
			SSystem::SString StringAt( size_t iArg, const wchar_t * pwszDefault = NULL ) const ;
			SSystem::SString NextString( const wchar_t * pwszDefault = NULL ) ;
			// オブジェクト取得
			RSObject * ObjectAt( size_t iArg ) const ;
			RSObject * NextObject( void ) ;
			ESLObject * NativeObjectAt( size_t iArg ) const ;
			ESLObject * NextNativeObject( void ) ;
			// ポインタ取得
			uint8_t * PointerAt( size_t iArg, size_t * pBoundSize = NULL ) const ;
			uint8_t * PointerAt( size_t iArg, size_t nReqSize ) const ;
			uint8_t * NextPointer( size_t nReqSize ) ;

		public:	// SStringFormatSupplier
			// 次の整数取得
			virtual int64_t NextInteger( void ) ;
			// 次の浮動小数点取得
			virtual double NextDouble( void ) ;
			// 次の文字取得
			virtual wchar_t NextCharacter( void ) ;
			// 次の文字列取得
			virtual const wchar_t * NextString( SSystem::SString& strNext ) ;
		} ;

		// 文字列書式化
		void FormatStringVlist
			( SSystem::SString& strDst,
				const wchar_t * pwszFormat,
				RSObject**const ppArgs, size_t countArg ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 数式インスタンス
	//////////////////////////////////////////////////////////////////////////

	class	RSExpression
	{
	protected:
		SSystem::SSmartPointer<RSScript>	m_source ;
		RSCodeStream						m_code ;
		SSystem::SSmartPointer<RSContext>	m_context ;

	public:
		// 数式計算（名前空間を作成して式を評価）
		RSObject * PerformExpression
			( RSVirtualMachine * pVM,
				const wchar_t * pwszExpr,
				RSObject * pThisObj = NULL,
				SSystem::SParserErrorInterface* pperr = NULL ) ;

	public:
		// 実行コンテキスト準備（既にある場合には何もしない）
		void PrepareContext
			( RSVirtualMachine * pVM, RSObject * pThread = NULL ) ;
		// 数式設定
		SSystem::SError SetSourceCode
			( RSVirtualMachine * pVM, const wchar_t * pwszCode ) ;
		// 数式計算（名前空間を作成して設定された数式を評価）
		RSObject * PerformExpression
			( RSObject * pThisObj = NULL,
				SSystem::SParserErrorInterface* pperr = NULL ) ;

	public:
		// 例外発生判定
		bool IsException( void ) const ;
		// 例外取得
		RSObject * GetException( void ) const ;
		// 例外クリア
		void ClearException( void ) ;

	public:
		// 実行コンテキスト
		RSContext * GetContext( void ) const
		{
			return	m_context ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// オブジェクト・スマートポインタ
	//////////////////////////////////////////////////////////////////////////

	class	RSSmartPtr
	{
	public:
		RSObject *	m_obj ;
		RSContext *	m_context ;

	public:
		// 構築関数
		RSSmartPtr( const RSSmartPtr& ptr )
			: m_obj( RSObject::AddRef( ptr.m_obj ) ), m_context( ptr.m_context ) { }
		RSSmartPtr( RSObject * obj )
			: m_obj( obj ), m_context( NULL ) {}
		RSSmartPtr( RSObject * obj, RSContext * context )
			: m_obj( obj ), m_context( context ) {}
		RSSmartPtr( void )
			: m_obj( NULL ), m_context( NULL ) {}
		// 消滅関数
		~RSSmartPtr( void )
		{
			if ( m_context )
			{
				m_context->ReleaseObjectRef( m_obj ) ;
			}
			else
			{
				RSObject::ReleaseRef( m_obj ) ;
			}
		}
		// RSContext 関連付け
		void AttachContext( RSContext * context )
		{
			m_context = context ;
		}
		RSContext * GetContext( void ) const
		{
			return	m_context ;
		}
		// ポインタ
		RSObject * Ptr( void ) const
		{
			return	m_obj ;
		}
		RSObject * AddRef( void ) const
		{
			return	RSObject::AddRef( m_obj ) ;
		}
		RSObject * operator -> ( void ) const
		{
			return	m_obj ;
		}
		RSObject & operator * ( void ) const
		{
			return	*m_obj ;
		}
		operator const RSObject * ( void ) const
		{
			return	m_obj ;
		}
		operator RSObject * ( void ) const
		{
			return	m_obj ;
		}
		// 実体
		RSSmartPtr GetEntity( void ) const
		{
			if ( m_obj == NULL )
			{
				return	RSSmartPtr( NULL, m_context ) ;
			}
			return	RSSmartPtr
				( RSObject::AddRef( m_obj->GetEntityObject() ), m_context ) ;
		}
		// 比較
		bool operator == ( const RSObject * obj ) const
		{
			return	(m_obj == obj) ;
		}
		bool operator != ( const RSObject * obj ) const
		{
			return	(m_obj != obj) ;
		}
		// 代入
		RSSmartPtr& operator = ( const RSSmartPtr& ptr )
		{
			RSObject *	pObj = ptr.AddRef() ;
			if ( m_context )
			{
				m_context->ReleaseObjectRef( m_obj ) ;
			}
			else
			{
				RSObject::ReleaseRef( m_obj ) ;
			}
			m_obj = pObj ;
			return	*this ;
		}
		RSSmartPtr& operator = ( RSObject * obj )
		{
			if ( m_context )
			{
				m_context->ReleaseObjectRef( m_obj ) ;
			}
			else
			{
				RSObject::ReleaseRef( m_obj ) ;
			}
			m_obj = obj ;
			return	*this ;
		}
		// 分離
		RSObject * Detach( void )
		{
			RSObject *	obj = m_obj ;
			m_obj = NULL ;
			return	obj ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 基底デバッガ
	//////////////////////////////////////////////////////////////////////////

	class	RSDebugger	: public RSContext::DebugListener
	{
	public:
		// ブレークポイント
		struct	BreakPoint
		{
			const RSScript *	pScript ;
			size_t				iSrcChar ;
		} ;
	protected:
		SSystem::SArray<BreakPoint>	m_arrBreakPoints ;

		enum	TraceMode
		{
			traceExecute,
			traceStepOver,
			traceStepIn,
		} ;
		TraceMode	m_modeTrace ;
		size_t		m_countTraceSteps ;
		size_t		m_nestCall ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSDebugger, DebugListener )
		// 構築関数
		RSDebugger( void ) ;

	public:
		// ブレークポイント追加
		void AddBreakPoint( const BreakPoint& bp ) ;
		void AddBreakPointLineAt
			( const RSScript * pScript, size_t nLineNum ) ;
		// ブレークポイント数
		size_t GetBreakPointCount( void ) const ;
		// ブレークポイント取得
		const BreakPoint * GetBreakPointAt( size_t i ) const ;
		// ブレークポイント一致判定
		ssize_t FindBreakPoint
			( const RSScript * pScript, size_t iIndex ) const ;
		ssize_t FindBreakPoint
			( const RSScript * pScript, size_t iFirst, size_t iEnd ) const ;
		// ブレークポイント削除
		void RemoveBreakPointAt( size_t i ) ;
		void RemoveBreakPoint( const BreakPoint& bp ) ;
		void RemoveAllBreakPoints( void ) ;
		// ソース位置から行番号へ変換
		static size_t GetLineNumberOf( const BreakPoint& bp ) ;
		static size_t GetLineNumberOf( const RSScript * pScript, size_t iIndex ) ;

	public:	// DebugListener オーバーライド
		// デバッグ
		virtual void OnDebug
			( RSContext * context,
				const RSScript * pScript, size_t iSrcChar ) ;
		// 関数呼び出し
		virtual void OnCall
			( RSContext * context, RSFunctionPrototype * pProto ) ;
		// 関数復帰
		virtual void OnReturned
			( RSContext * context,
				const RSScript * pScript, size_t iSrcChar ) ;

	public:	// デバッガ実装オーバーライド用
		// ブレークポイント
		virtual void OnBreakPoint
			( RSContext * context, const BreakPoint& bp ) ;
		// ステップ実行
		enum	StepExecution
		{
			stepContinue,
			stepBreak,
		} ;
		virtual void OnStepExecution
			( RSContext * context,
				StepExecution sx,
				const RSScript * pScript, size_t iSrcChar ) ;
		// ステップ実行割り込み停止判定
		virtual bool IsBreakExecution( void ) ;

	public:	// デバッグ用処理
		// ステップオーバー停止位置設置
		void PutBreakStepOver
			( const RSScript * pScript, size_t iSrcChar ) ;
		// トレース実行設定
		void PutTraceSteps( size_t nSteps = 1 ) ;
		// 実行設定
		void PutTraceExecution( void ) ;

	} ;

}

#endif

