
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script C style コンパイラ
//////////////////////////////////////////////////////////////////////////////

class	ECSCStyleCompiler	: public	ECSCompiler
{
public:
	// 構築関数
	ECSCStyleCompiler( void ) ;
	// 消滅関数
	virtual ~ECSCStyleCompiler( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSCStyleCompiler, ECSCompiler )

public:
	// ディレクティブ
	enum	CStyleDirective
	{
		csdInvalid = -1,
		csdIf, csdIfdef, csdIfndef, csdElseif, csdElse, csdEndif,
		csdDefine, csdInclude, csdUndef,
		csdError, csdWarning, csdMode,
		csdMax,
	} ;
	// 予約語
	enum	CStyleReservedWord
	{
		csrwInvalid	= -1,
		csrwTypeDef, csrwConstant,
		csrwData, csrwEnum, csrwStruct, csrwUnion,
		csrwClass, csrwNamespace, csrwAsm,
		csrwPublic, csrwProtected, csrwPrivate,
		csrwIf, csrwElse,
		csrwBreak, csrwContinue, csrwReturn, csrwGoto,
		csrwTry, csrwCatch, csrwThrow,
		csrwFor, csrwWhile,	csrwDo,
		csrwSwitch, csrwCase, csrwDefault,
		csrwMemoryFence,
		csrwTemplate,
		csrwUsing, csrwFriend,
		csrwMax,
	} ;

public:
	// #if ディレクティブコンテキスト
	class	ESourceStreamNest
	{
	public:
		CStyleDirective	m_csdType ;
		bool			m_fEnabled ;
		bool			m_fCompletion ;
	public:
		ESourceStreamNest( void ) 
			: m_csdType(csdIf),
				m_fEnabled(false), m_fCompletion(false) {}
	} ;
	// 文字列ストリーミング（部分処理）
	class	ECSCStyleStream	: public ECSSourceStream
	{
	public:
		// 構築関数
		ECSCStyleStream( void ) {}
		// クラス情報
		DECLARE_CLASS_INFO( ECSCStyleStream, ECSSourceStream )
		// 空白文字を読み飛ばす
		virtual int DisregardSpace( void ) ;
		// 現在の文字列（区切り記号は無視）を通過する
		virtual void PassEnclosedString
			( wchar_t wchClose,
				int flagCtrlCode = ECSSourceStream::flagNormal ) ;
		// 代入処理
		ECSCStyleStream & operator = ( const ECSSourceStream & cssText )
			{
				ECSSourceStream::operator = ( cssText ) ;
				return	*this ;
			}
		ECSCStyleStream & operator = ( const wchar_t * pwszText )
			{
				ECSSourceStream::operator = ( pwszText ) ;
				return	*this ;
			}
	} ;
	// 文字列ストリーミング（ソースファイル）
	class	ECSCStyleSourceStream	: public ECSCStyleStream
	{
	public:
		int					m_nLine ;
		EString				m_strFilePath ;
	protected:
		ECSCStyleCompiler *	m_compiler ;
		int					m_nRef ;
		bool				m_fReturnMode ;
		bool				m_fReturnCStyle ;
		bool				m_fReturnCCompatible ;
	public:
		// 構築関数
		ECSCStyleSourceStream( ECSCStyleCompiler * compiler )
			: m_compiler(compiler), m_nLine(1),
				m_nRef(1), m_fReturnMode(false), m_fReturnCCompatible(false) {}
		// クラス情報
		DECLARE_CLASS_INFO( ECSCStyleSourceStream, ECSCStyleStream )
		// 現在のアクティブな文字列ストリーミングを取得する
		ECSCStyleSourceStream * GetCurrentStream( void )
			{
				return	m_compiler->GetCurrentSourceStream() ;
			}
		// 現在の行番号を取得する
		int GetCurrentLineNumber( void ) const
			{
				return	m_nLine ;
			}
		// 参照カウンタを追加する
		void AddRef( void )
			{
				m_nRef ++ ;
			}
		// 参照カウンタを減少する
		void ReleaseRef( void )
			{
				if ( this != NULL )
				{
					if ( -- m_nRef == 0 )
					{
						delete	this ;
					}
				}
			}
		// 空白文字を読み飛ばす
		virtual int DisregardSpace( void ) ;
		// 代入処理
		ECSCStyleSourceStream & operator = ( const ECSSourceStream & cssText )
			{
				ECSCStyleStream::operator = ( cssText ) ;
				return	*this ;
			}

		friend ECSCStyleCompiler ;
	} ;
	// 文字列ストリーミング（フロントエンドバッファ）
	class	ECSCStyleFrontStream	: public ECSCStyleStream
	{
	protected:
		ECSCStyleCompiler *		m_compiler ;
		ECSCStyleSourceStream *	m_pcsssLast ;
	public:
		int					m_nLine ;
		EString				m_strFilePath ;
	public:
		// 構築関数
		ECSCStyleFrontStream( ECSCStyleCompiler * compiler )
			: m_compiler(compiler), m_pcsssLast(NULL) {}
		// クラス情報
		DECLARE_CLASS_INFO( ECSCStyleFrontStream, ECSCStyleStream )
		// 空白文字を読み飛ばす
		virtual int DisregardSpace( void ) ;
		// 1ステートメント又は行末まで読み飛ばす
		bool PassAStatementLine( void ) ;
		// 行バッファ更新
		void FlushLine( void ) ;
		// 代入処理
		ECSCStyleFrontStream & operator = ( const wchar_t * pwszText )
			{
				ECSCStyleStream::operator = ( pwszText ) ;
				return	*this ;
			}
	} ;

protected:
	bool								m_flagCStyle ;
	DWORD								m_dwBasicModeFlags ;
	EObjArray<ESourceStreamNest>		m_nestSource ;
	EObjArray<ECSCStyleSourceStream>	m_lstSource ;

	EControlNest *						m_pTemplateNest ;

	ECSInteger							m_csintCotopha ;

	DWORD								m_dwCompileTime ;

public:
	// スクリプトをコンパイルする
	virtual ESLError CompileScript
		( ECSSourceStream & cssScript, const char * pszFilePath = NULL ) ;
	// コンパイルを完了する
	virtual ESLError FinishCompile( DWORD dwFlags = 0 ) ;
	// スクリプトをインクルードする
	ESLError IncludeSourceScript( const char * pszFileName ) ;
	ESLError IncludeSourceScript( ESLFileObject * pfile, const char * pszFileName ) ;
	// スクリプトを最後までコンパイルする
	ESLError CompileAllScript( ECSCStyleSourceStream * pcsss = NULL ) ;
	// Cスタイルスクリプトをコンパイルする
	ESLError CompileCStyleScript( void ) ;
	// Cスタイル１文コンパイルする
	ESLError CompileCStyleStatement
		( ECSCStyleFrontStream& cssSrc, bool fNoBlockForMulti = false ) ;
	// Cスタイル宣言・定義・式文いずれかを１文コンパイルする
	ESLError CompileCStyleDeclOrExprStatement( ECSCStyleFrontStream& cssSrc ) ;
	// Cスタイル複文をコンパイルする
	ESLError CompileCStyleMultiStatement( ECSCStyleFrontStream& cssSrc ) ;
	// 詞葉標準構文を１行コンパイルする
	virtual ESLError CompileScriptLine
		( ECSSourceStream & cssLine, int nLineNum,
			const char * pszFilePath = NULL, bool fEnableUserMacro = true ) ;

public:
	// 属性
	enum	MemoryDecoration
	{
		flagConstant	= ECSTypeInfo::flagConstant,	// 変更不可オブジェクト
		flagStatic		= ECSTypeInfo::flagStatic,		// 静的メンバ
		flagAbstract	= ECSTypeInfo::flagAbstract,	// 抽象クラス
		flagVirtual		= ECSTypeInfo::flagVirtual,		// 仮想関数
		flagInline		= ECSTypeInfo::flagInline,		// インライン関数
		flagNakedCall	= ECSTypeInfo::flagNakedCall,	// naked モード関数
		flagJITNative	= ECSTypeInfo::flagNakedJITNative,	// __jit_natve__ 指定
		flagExtern		= 0x00010000,					// extern 指定
		flagShared		= 0x00020000,					// shared 指定
		flagNaked		= 0x00040000,					// naked 指定
		flagObjected	= 0x00080000,					// objected 指定
		flagNative		= ECSTypeInfo::flagNativeObject,// native 指定
		flagFunctionMask= ECSTypeInfo::flagFunctionMask,
	} ;
	// 記憶クラス修飾子をコンパイルする
	// [extern] [{ shared | static }] [inline] [virtual] [native] [abstract] 
	static DWORD ParseMemoryClass( ECSCStyleFrontStream & cssSrc ) ;
	// 型構文をコンパイルする
	bool ParseCStyleTypeDescription
		( EWideString & wstrTypeExpr,
			ECSCStyleFrontStream & cssSrc, bool fNoTemplateInstance ) ;
	// 配列型構文を解釈して追加する
	ESLError ParseCStyleTypeArrayDecoration
		( EWideString & wstrTypeExpr, ECSCStyleFrontStream & cssSrc ) ;

public:
	// 式文をコンパイルする
	ESLError CompileCStyleExpression
		( ECSCStyleFrontStream & cssSrc ) ;
	// 変数・関数定義文をコンパイルする
	ESLError CompileVarFuncDeclaration
		( DWORD dwMemoryClass,
			const EWideString & wstrTypeName,
			ECSCStyleFrontStream & cssSrc ) ;
	// 関数定義文をコンパイルする
	ESLError CompileFunctionDeclaration
		( DWORD dwMemoryClass,
			const wchar_t * pwszRetType,
			const EWideString & wstrFuncName,
			const EWideString & wstrArgList,
			ECSCStyleFrontStream & cssSrc, bool fRetType = true ) ;
	// class/struct 構文を詞葉構文に変換する
	ReservedWord ParseClassDeclarationToBasicStatement
		( EWideString & wstrStatement,
			EWideString & wstrTagName, wchar_t & wchNext,
			ECSCStyleFrontStream & cssSrc, bool fStruct ) ;
	// class/struct 構文をコンパイルする
	bool ParseClassDeclaration
		( EWideString & wstrTagName,
			ECSCStyleFrontStream & cssSrc, bool fStruct ) ;
	// union 構文をコンパイルする
	bool ParseUnionDeclaration
		( EWideString & wstrTagName, ECSCStyleFrontStream & cssSrc ) ;
	// enum 構文をコンパイルする
	bool ParseEnumDeclaration
		( EWideString & wstrTagName, ECSCStyleFrontStream & cssSrc ) ;
	// data 構文をコンパイルする
	bool ParseDataDeclaration
		( EWideString & wstrTagName, ECSCStyleFrontStream & cssSrc ) ;
	// 指定のいずれかの文字を発見するまで構文を読み進める
	wchar_t ParseEnclosedExpression
		( ECSCStyleFrontStream & cssSrc,
			const wchar_t * pwszCloses, EWideString * pwstrExpr ) ;
	bool ParseEnclosedExpressionByToken
		( ECSCStyleFrontStream & cssSrc,
			const wchar_t * pwszClose, EWideString * pwstrExpr ) ;

public:
	// C スタイルモード判定
	bool IsInCStyleMode( void ) const
		{
			return	m_flagCStyle ;
		}
	// C 互換モード判定
	bool IsInCCompatibleMode( void ) const
		{
			return	m_flagCStyle
				&& ((m_dwModeFlags & flagDefaultNakedAll) != 0) ;
		}
	// C スタイルモード設定
	void SetCStyleMode( bool fCStyleMode, bool fCCompatible ) ;
	// 現在のアクティブな文字列ストリーミングを取得する
	ECSCStyleSourceStream * GetCurrentSourceStream( void ) const
		{
			return	m_lstSource.GetLastAt(0) ;
		}
	// 全ソースストリームが終了しているか？
	bool IsEndOfAllSourceStream( void ) const
		{
			ECSCStyleSourceStream *	pcss = GetCurrentSourceStream() ;
			return	(pcss == NULL)
						|| ((m_lstSource.GetSize() <= 1)
								&& pcss->IsIndexOverflow()) ;
		}
	// 現在有効な #if ブロック内か？
	bool IsInEnabledDirectiveBlock( void ) const ;
	// 空白文字・コメントを読み飛ばす／前置ディレクティブ処理
	int SeekNextSourceStream( ECSCStyleSourceStream * pcsss ) ;
	// 空白文字とコメントを読み飛ばす
	static int SeekNextCStyleSource( ECSSourceStream & cssSource ) ;
	// ディレクティブ１行取得
	void GetNextCStyleDirectiveLine
		( ECSSourceStream & cssLine,
			ECSCStyleSourceStream & cssScript ) ;
	// ソースストリームから１節をプリプロセスして追加
	void AddNextFilteredSourceTerm
		( ECSSourceStream & cssDst, ECSSourceStream & cssSrc ) ;
	// ソースストリームからプリプロセス済みの次の1行を取得して追加
	bool AddNextFilteredSourceLine( ECSSourceStream & cssDst ) ;
	// #define テキストマクロプリプロセッサ
	void FilterTextPreprocessor( ECSSourceStream & cssSource ) ;

public:	// C スタイルモード振る舞いオーバーライド
	// 定数値（マクロ変数）取得
	virtual ECSObject * GetMacroVariable( const wchar_t * pwszName ) ;
	// 型情報を取得
	virtual ECSObject * ParseBsaicType( const wchar_t * pwszName ) const ;
	// 型情報検索
	virtual ESLError SearchTypeName
		( SYMBOL_NAMESPACE& snsSymbol, ECSTypeInfo::Flags flagScope ) ; 

protected:
	// 予約語処理関数
	typedef	ESLError
		(ECSCStyleCompiler::*PFN_COMPILE_RESERVED_WORD)
			( ECSCStyleFrontStream & cssLine ) ;
	static const PFN_COMPILE_RESERVED_WORD	m_pfnCompileCStyle[csrwMax] ;

	// ディレクティブ処理関数
	typedef	ESLError
		(ECSCStyleCompiler::*PFN_COMPILE_DIRECTIVE)
			( ECSSourceStream & cssLine ) ;
	static const PFN_COMPILE_DIRECTIVE	m_pfnCompileDirective[csdMax] ;

public:
	// Cスタイル予約語判定
	CStyleReservedWord IsCStyleReservedWord( const wchar_t * pwszToken ) const ;
	// 予約語処理
	ESLError CompileCStyleReservedWord
		( CStyleReservedWord csrwWord, ECSCStyleFrontStream & cssLine ) ;
	// 予約語の処理
	ESLError CompileCStyleTypedef( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleConstant( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleData( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleEnum( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleStruct( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleUnion( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleClass( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleNamespace( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleAsm( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStylePublic( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleProtected( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStylePrivate( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleIf( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleElse( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleBreak( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleContinue( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleReturn( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleGoto( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleTry( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleCatch( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleThrow( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleFor( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleWhile( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleDo( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleSwitch( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleCase( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleDefault( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleMemoryFence( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleTemplate( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleUsing( ECSCStyleFrontStream & cssLine ) ;
	ESLError CompileCStyleFriend( ECSCStyleFrontStream & cssLine ) ;

public:
	// ディレクティブ判定
	CStyleDirective IsDirectiveWord( const wchar_t * pwszToken ) const ;
	// ディレクティブ処理
	ESLError CompileDirectiveLine( ECSSourceStream & cssLine ) ;
	// マクロ予約語の処理
	ESLError CompileDirectiveIf( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveIfDef( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveIfNDef( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveElseIf( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveElse( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveEndIf( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveDefine( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveInclude( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveUnDef( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveError( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveWarning( ECSSourceStream & cssLine ) ;
	ESLError CompileDirectiveMode( ECSSourceStream & cssLine ) ;

} ;

