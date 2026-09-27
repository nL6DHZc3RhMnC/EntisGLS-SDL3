
//////////////////////////////////////////////////////////////////////////////
// Cotopha Script コンパイル時型情報
//////////////////////////////////////////////////////////////////////////////

class	ECSPrototypeInfo ;
class	ECSClassInfo ;
class	ECSFunction ;

class	ECSTypeInfo
{
public:
	// 属性
	enum	Flags
	{
		flagConstant		= 0x00000001,	// 変更不可オブジェクト
		flagStatic			= 0x00000002,	// 静的メンバ
		flagAbstract		= 0x00000004,	// 抽象クラス
		flagVirtual			= 0x00000008,	// 仮想関数
		flagInline			= 0x00000010,	// インライン関数
		flagNakedCall		= 0x00000020,	// naked モード関数
											// 実行バイナリの関数属性として naked 実装関数
		flagNakedCallGate	= 0x00000040,	// object -> naked モードゲート関数
		flagObjectCallGate	= 0x00000080,	// naked -> object モードゲート関数
											// ※仮想関数をオーバーライドした場合、
											// 　オーバーライド元の flagNakedCall は維持され
											// 　flagNakedCallGate, flagObjectCallGate で
											// 　その関数の実装モードが決定される
		flagPublic			= 0x00000000,
		flagProtected		= 0x00000100,
		flagPrivate			= 0x00000200,
		flagPrivate2		= 0x00000300,
		flagProtectedMask	= 0x00000300,
		flagVarArgument		= 0x00001000,	// 可変長引数
		flagThisCall		= 0x00002000,	// メンバ関数（this call 呼び出し規約）
		flagObjectedCall	= 0x00004000,	// objected 明示指定
		flagNakedJITNative	= 0x00008000,	// __jit_native__ 指定
		flagNamespace		= 0x02000000,	// namespace（class ではない）
		flagUnion			= 0x04000000,	// 共用体（class ではない）
		flagNakedBuffer		= 0x08000000,	// フラットメモリ属性
		flagEnumerator		= 0x10000000,	// 列挙型
		flagStructure		= 0x20000000,	// 構造体（class ではない）
		flagDeterministic	= 0x40000000,	// コンパイル時定数／関数引数初期値
											// ※数式コンパイル時に値が定数である場合
		flagNativeObject	= 0x80000000,	// ネイティブ組み込み型
											// ※クラス／関数属性
		flagReservedMask	= 0x000F0000,	// 予約領域ビットマスク
		flagFunctionMask
			= flagConstant | flagStatic | flagVirtual
					| flagAbstract | flagInline | flagVarArgument,
	} ;
	DWORD		m_dwFlags ;			// 属性
	ECSObject *	m_pValue ;			// 初期値と型情報
									// NULL は void 表現

	// コンパイル時のみの情報
	bool		m_flagThisReg ;		// this call 関数ポインタ用 this
	bool		m_flagLoadReg ;		// m_regLoaded が有効か？
	bool		m_flagAddress ;		// アドレッシング情報が有効か？
	int			m_regThis ;			// this call 関数ポインタ用 this
	int			m_regLoaded ;		// 値がロードされているレジスタ
	int			m_regBase ;			// ベースアドレスレジスタ
	int			m_regIndex ;		// インデックスレジスタ
	int			m_scaleIndex ;
	int			m_addrOffset ;		// オフセット

public:
	// 構築関数
	ECSTypeInfo( void )
		: m_dwFlags(0), m_pValue(NULL),
			m_flagThisReg(false), m_flagLoadReg(false), m_flagAddress(false),
			m_regThis(-1), m_regLoaded(-1),
			m_regBase(-1), m_regIndex(-1), m_scaleIndex(0), m_addrOffset(0) {}
	ECSTypeInfo( ECSObject * pValue, DWORD dwFlags = 0 )
		: m_dwFlags(dwFlags), m_pValue(pValue),
			m_flagThisReg(false), m_flagLoadReg(false), m_flagAddress(false),
			m_regThis(-1), m_regLoaded(-1),
			m_regBase(-1), m_regIndex(-1), m_scaleIndex(0), m_addrOffset(0) {}
	ECSTypeInfo( const ECSTypeInfo & typeinf )
		: m_dwFlags(typeinf.m_dwFlags), m_pValue(NULL),
			m_flagThisReg(typeinf.m_flagThisReg), m_regThis(typeinf.m_regThis),
			m_flagLoadReg(typeinf.m_flagLoadReg), m_regLoaded(typeinf.m_regLoaded),
			m_flagAddress(typeinf.m_flagAddress), m_regBase(typeinf.m_regBase),
			m_regIndex(typeinf.m_regIndex), m_scaleIndex(typeinf.m_scaleIndex),
			m_addrOffset(typeinf.m_addrOffset)
		{
			m_pValue = DuplicateType( typeinf.m_pValue ) ;
		}
	// 消滅関数
	~ECSTypeInfo( void )
		{ delete m_pValue ; }
	// 公開スコープ属性取得
	DWORD GetProtectedAttribute( void ) const
		{
			return	m_dwFlags & ECSTypeInfo::flagProtectedMask ;
		}
	// 型オブジェクト設定
	void SetTypeValue( ECSObject * pValue, DWORD dwFlags )
		{
			delete	m_pValue ;
			m_pValue = pValue ;
			m_dwFlags = dwFlags ;
			//
			ClearLoadedRegister() ;
			ClearAddressingInfo() ;
		}
	// レジスタ情報を設定
	void SetLoadedRegister( int regLoaded, int regThisCall = -1 )
		{
			m_flagLoadReg = true ;
			m_regLoaded = regLoaded ;
			m_flagThisReg = (regThisCall >= 0) ;
			m_regThis = regThisCall ;
		}
	void SetLoadedRegister( const ECSTypeInfo & typeLoaded )
		{
			if ( typeLoaded.m_flagThisReg )
			{
				m_flagThisReg = true ;
				m_regThis = typeLoaded.m_regThis ;
			}
			if ( typeLoaded.m_flagLoadReg )
			{
				m_flagLoadReg = true ;
				m_regLoaded = typeLoaded.m_regLoaded ;
			}
		}
	// レジスタ情報をクリア
	void ClearLoadedRegister( void )
		{
			m_regThis = -1 ;
			m_regLoaded = -1 ;
			m_flagThisReg = false ;
			m_flagLoadReg = false ;
		}
	// レジスタ番号を取得
	int GetLoadedRegister( void ) const
		{
			ESLAssert( m_flagLoadReg ) ;
			return	m_regLoaded ;
		}
	// レジスタ番号を保持しているか？
	bool IsLoadedRegister( void ) const
		{
			return	m_flagLoadReg ;
		}
	// this call 用レジスタ番号を取得
	int GetLoadedThisCallRegister( void ) const
		{
			ESLAssert( m_flagThisReg ) ;
			return	m_regThis ;
		}
	// this call 用レジスタ番号を保持しているか？
	bool IsLoadedThisCallRegister( void ) const
		{
			return	m_flagThisReg ;
		}
	// アドレッシング情報を設定
	void SetAddressingInfo
			( int regBase, int addrOffset,
				int regIndex = -1, int scaleIndex = 0 )
		{
			m_flagAddress = true ;
			m_regBase = regBase ;
			m_regIndex = regIndex ;
			m_scaleIndex = scaleIndex ;
			m_addrOffset = addrOffset ;
		}
	void SetAddressingInfo( const ECSTypeInfo & typeAddr )
		{
			if ( typeAddr.m_flagAddress )
			{
				m_flagAddress = true ;
				m_regBase = typeAddr.m_regBase ;
				m_regIndex = typeAddr.m_regIndex ;
				m_scaleIndex = typeAddr.m_scaleIndex ;
				m_addrOffset = typeAddr.m_addrOffset ;
			}
		}
	// アドレッシング情報を削除
	void ClearAddressingInfo( void )
		{
			m_flagAddress = false ;
			m_regBase = -1 ;
			m_regIndex = -1 ;
			m_scaleIndex = 0 ;
			m_addrOffset = 0 ;
		}
	// アドレッシング情報を保持しているか？
	bool IsAddressingInfo( void ) const
		{
			return	m_flagAddress ;
		}
	// レジスタ情報とアドレッシング情報を複製
	void MoveRegisterAndAddressingFrom( const ECSTypeInfo & typeinf )
		{
			m_flagThisReg = typeinf.m_flagThisReg ;
			m_regThis = typeinf.m_regThis ;
			//
			m_flagLoadReg = typeinf.m_flagLoadReg ;
			m_regLoaded = typeinf.m_regLoaded ;
			//
			m_flagAddress = typeinf.m_flagAddress ;
			m_regBase = typeinf.m_regBase ;
			m_regIndex = typeinf.m_regIndex ;
			m_scaleIndex = typeinf.m_scaleIndex ;
			m_addrOffset = typeinf.m_addrOffset ;
		}
	// 即値がある場合に即値を代入
	void MoveFromImmediateValue( ECSContext& context, const ECSTypeInfo & typeinf )
		{
			if ( (m_pValue != NULL)
				&& (typeinf.m_dwFlags & flagDeterministic) )
			{
				m_pValue->Move( context, DuplicateType( typeinf.m_pValue ) ) ;
				m_dwFlags |= flagDeterministic ;
			}
		}
	// 型オブジェクト分離
	ECSObject * DetachValue( void )
		{
			ECSObject *	pValue = m_pValue ;
			m_pValue = NULL ;
			return	pValue ;
		}
	// 代入
	const ECSTypeInfo & operator = ( const ECSTypeInfo & typeinf )
		{
			ECSObject *	pValue = DuplicateType( typeinf.m_pValue ) ;
			m_dwFlags = typeinf.m_dwFlags ;
			delete	m_pValue ;
			m_pValue = pValue ;
			//
			MoveRegisterAndAddressingFrom( typeinf ) ;
			return	*this ;
		}
	// タイプオブジェクト複製
	static ECSObject * DuplicateType( ECSObject * pType ) ;
	ECSObject * DuplicateType( void ) const
		{
			return	DuplicateType( m_pValue ) ;
		}
	// 参照型生成
	void MakeReferenceOf( const ECSTypeInfo & typeinf ) ;
	// ポインタ型生成
	void MakePointerOf( const ECSTypeInfo & typeinf ) ;
	// 参照解除した型情報生成
	void MakeNakedOf( const ECSTypeInfo & typeinf ) ;
	// ポインタ解除した型情報生成
	void MakeNakedPointerOf( const ECSTypeInfo & typeinf ) ;
	// 整数定数値の型を最小サイズに正規化する
	void NormalzieImmediateIntegerType( void ) ;
	// 比較
	bool IsTypeEqual( const ECSTypeInfo & typeinf ) const ;
	static bool IsTypeEqual
		( const ECSObject * pDstObj, const ECSObject * pSrcObj ) ;
	bool operator == ( const ECSTypeInfo & typeinf ) const
		{
			return	IsTypeEqual( typeinf ) ;
		}
	bool operator != ( const ECSTypeInfo & typeinf ) const
		{
			return	!IsTypeEqual( typeinf ) ;
		}

public:
	// 型変換適合度
	enum	TypeMatchResult
	{
		typeMatch		= 0,	// 適合（同一型）
		typeNatualMatch,		// 変換可能
		typeCastableMatch,
		typeLooseMatch,			// ルーズな変換
		typeNoMatch,
	} ;
	// 型変換可能判定
	TypeMatchResult IsMatchType
		( const ECSTypeInfo & typeSrc,
			TypeMatchResult matchLimit = typeNoMatch ) const ;
	// naked ポインタの配列キャスト判定 type* <- type[][][]...*
	static bool IsTypeNakedArrayEqual
		( const ECSObject * pDstObj, const ECSObject * pSrcObj ) ;
	static bool IsTypeNakedArrayPointerMatch
		( const ECSObject * pDstObj, const ECSObject * pSrcObj ) ;
	// 型表現フォーマット
	EWideString GetFormatTypeString( void ) const ;
	void FormatTypeString( EWideString & wstrTypeFormat ) const ;
	static void FormatTypeString
			( EWideString & wstrTypeFormat, const ECSObject * pType ) ;
	const ECSObject * FormatTypeDecoration( EWideString & wstrTypeFormat ) const ;
	static const ECSObject * FormatTypeDecoration
			( EWideString & wstrTypeFormat, const ECSObject * pType ) ;
	// プロトタイプの引数型表現フォーマット
	static void FormatArgumentTypeList
		( EWideString & wstrArgList, const ECSPrototypeInfo & proto ) ;
	// クラス参照をカウント
	void EnumerateReferenceClasses
		( ENumArray<DWORD>& lstClassUsed, class ECSExecutionImage * pcsxi ) ;
	// デバッグ出力
	void DebugTrace( const char * pszHeader = "typeinf:" ) const ;
	// Void 型か？
	bool IsVoid( void ) const
		{
			return	(m_pValue == NULL) ;
		}
	// Reference 型か？（抽象型オブジェクト）
	bool IsAbstractType( void ) const
		{
			if ( (m_pValue != NULL)
				&& (m_pValue->m_vtType == csvtReference) )
			{
				ECSReference *	pValueRef = (ECSReference*) m_pValue ;
				if ( pValueRef->m_pRef == NULL )
				{
					return	true ;		// 純粋 Reference 型
				}
				if ( pValueRef->m_pRef->m_vtType == csvtReference )
				{
					if ( ((ECSReference*)(pValueRef->m_pRef))->m_pRef == NULL )
					{
						return	true ;		// Reference& 型も抽象型
					}
				}
			}
			return	false ;
		}
	// ピュアな型か？（配列や参照型でない）
	bool IsPureType( void ) const ;
	// Boolean 型か？
	static bool IsTypeBoolean( const ECSObject * pType ) ;
	bool IsTypeBoolean( void ) const
		{
			return	IsTypeBoolean( m_pValue ) ;
		}
	// 基本型判定
	static bool IsTypeBasicEqual
			( const ECSObject * pType, CSVariableType csvtType )
		{
			return	(pType != NULL) && (pType->m_vtType == csvtType) ;
		}
	bool IsTypeBasicEqual( CSVariableType csvtType ) const
		{
			return	IsTypeBasicEqual( GetNakedType(), csvtType ) ;
		}
	// Integer 型か？（整数型各種か？）
	bool IsTypeInteger( void ) const
		{
			return	IsTypeBasicEqual( GetNakedType(), csvtInteger ) ;
		}
	INT64 GetValueInteger( void ) const ;
	// Real 型か？（実数型各種か？）
	bool IsTypeReal( void ) const
		{
			return	IsTypeBasicEqual( GetNakedType(), csvtReal ) ;
		}
	double GetValueReal( void ) const ;
	// String 型か？（文字列型各種か？）
	bool IsTypeString( void ) const
		{
			return	IsTypeBasicEqual( GetNakedType(), csvtString ) ;
		}
	// 参照型か？（type& 形式か？）
	static bool IsTypeReference( const ECSObject * pType ) ;
	bool IsTypeReference( void ) const
		{
			return	IsTypeReference( m_pValue ) ;
		}
	// 参照型か？（type& 形式の左辺式か？）
	static bool IsTypeReference2( const ECSObject * pType ) ;
	bool IsTypeReference2( void ) const
		{
			return	IsTypeReference2( m_pValue ) ;
		}
	// 多次元配列型か？（type[] 形式か？）
	static bool IsTypeArray( const ECSObject * pType ) ;
	bool IsTypeArray( void ) const
		{
			return	IsTypeArray( GetNakedType() ) ;
		}
	// ハッシュコンテナ型か？（Hash<Type> 形式か？）
	static bool IsTypeHashArray( const ECSObject * pType ) ;
	bool IsTypeHashArray( void ) const
		{
			return	IsTypeHashArray( GetNakedType() ) ;
		}
	// ポインタ型か？
	bool IsTypePointer( void ) const
		{
			return	IsTypeBasicEqual( GetNakedType(), csvtPointer ) ;
		}
	bool IsTypePointerNaked( void ) const
		{
			return	IsTypeBasicEqual( GetNakedType(), csvtPointer )
								&& (m_dwFlags & flagNakedBuffer) ;
		}
	// 関数ポインタ型か？
	bool IsTypeFunctionPointer( void ) const
		{
			return	(GetTypeFunctionPointer() != NULL) ;
		}
	ECSFunction * GetTypeFunctionPointer( void ) const ;
	// 実行時に Integer で表現される特殊な型か？
	bool IsRuntimeIntegerType( void ) const ;
	// 参照を除去した型を取得
	ECSObject * GetNakedType( void ) const ;
	ECSObject * GetSingleNakedType( void ) const ;
	// ポインタを除去した型を取得
	ECSObject * GetNakedPointerType( void ) const ;
	// 配列型 [] の要素型を取得
	ECSObject * GetArrayElementType( void ) const ;
	// ピュアな型を取得
	ECSObject * GetPureType( void ) const ;
	// ピュアな型のクラス情報取得
	const ECSClassInfo * GetClassInfo( void ) const ;

public:
	// naked メモリ上でのサイズ
	int SizeOfOnNakedMemory( void ) const ;
	ESLError GetNakedMemorySize( int& nSize, int& nObjCount ) const ;
	static ESLError GetNakedMemorySize
			( int& nSize, int& nObjCount, ECSObject * pType ) ;
	// naked メモリ上でのアライメント取得
	int GetNakedAlignment( void ) const ;
	static int GetNakedAlignment( ECSObject * pType ) ;
	// naked メモリ上の配列サイズを計算
	int CalcNakedMemoryArraySize( void ) const ;
	static int CalcNakedMemoryArraySize( ECSObject * pObj ) ;
	// naked メモリ上のアクセス型
	CSVariableType GetNakedMemoryType( void ) const ;
	static CSVariableType GetNakedMemoryType( ECSObject * pObj ) ;
	// naked メモリ上のオブジェクトか判定
	bool IsNakedMemoryObject( bool fNakedMemoryMode ) const ;
	static bool IsNakedMemoryObject
		( const ECSObject * pType, bool fNakedMemoryMode ) ;
	// naked モードレジスタで表現可能な型（整数・実数・ポインタ・参照）か判定
	bool IsNakedPrimitiveDataType( void ) const ;
	static bool IsNakedPrimitiveDataType( const ECSObject * pObj ) ;
	// naked クラスかその配列型なら、そのクラス情報を取得
	const ECSClassInfo * GetNakedMemoryClassInfo( void ) const ;
	// naked メモリ上で object かその配列型なら、そのクラス情報を取得
	const ECSClassInfo * GetObjectClassInfo( const ECSCompiler& compiler ) const ;
	static const ECSClassInfo * GetObjectClassInfo
			( const ECSObject * pType, const ECSCompiler& compiler ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// Cotopha Script 関数プロトタイプ情報
//////////////////////////////////////////////////////////////////////////////

class	ECSPrototypeInfo	: public ESLObject
{
public:
	// クラス情報
	DECLARE_CLASS_INFO( ECSPrototypeInfo, ESLObject )
	// 構築関数
	ECSPrototypeInfo( void ) ;
	ECSPrototypeInfo( const ECSPrototypeInfo & proto ) ;
	// 消滅関数
	virtual ~ECSPrototypeInfo( void ) ;
	// 代入
	const ECSPrototypeInfo & operator = ( const ECSPrototypeInfo & proto ) ;

protected:
	DWORD					m_dwFlags ;			// 関数属性 enum ECSTypeInfo::Flags
	ECSTypeInfo				m_typeReturn ;		// 返り値の型名
	EWideString				m_wstrFuncName ;	// 関数名
	EWideString				m_wstrGlobalName ;	// 関数名（ファイルスコープ名）
	EObjArray<ECSTypeInfo>	m_lstArgument ;		// 引数
	EObjArray<EWideString>	m_lstArgName ;		// 引数名
	EObjArray<ECSObject>	m_lstArgDefault ;	// 引数デフォルト値
	EObjArray<EWideString>	m_lstThrows ;		// 送出されうる例外型

public:
	// 比較
	bool IsPrototypeEqual( const ECSPrototypeInfo & proto ) const ;
	bool IsArgumentEqual( const EPtrObjArray<ECSTypeInfo> & arg ) const ;
	bool operator == ( const ECSPrototypeInfo & proto ) const
		{
			return	(m_wstrFuncName == proto.m_wstrFuncName)
									&& IsPrototypeEqual( proto ) ;
		}
	bool operator != ( const ECSPrototypeInfo & proto ) const
		{
			return	(m_wstrFuncName != proto.m_wstrFuncName)
									|| !IsPrototypeEqual( proto ) ;
		}
	// 引数の適合判定
	ECSTypeInfo::TypeMatchResult IsMatchArgument
		( const EPtrObjArray<ECSTypeInfo> & lstArg,
			ECSTypeInfo::TypeMatchResult matchLimit = ECSTypeInfo::typeNoMatch ) const ;
	// 属性設定
	void SetAttribute( DWORD dwFlags ) ;
	// 返り値型設定
	void SetReturnType( const ECSTypeInfo & typeinf ) ;
	// 関数名設定
	void SetName( const wchar_t * pwszName ) ;
	// 関数名（ファイルスコープ用）設定
	void SetGlobalName( const wchar_t * pwszName ) ;
	// 引数追加
	int AddArgument( ECSTypeInfo * pArg, const wchar_t * pwszName = NULL ) ;
	// 引数設定
	void SetArgumentAt( unsigned int nIndex, ECSTypeInfo * pArg ) ;
	// 引数名設定
	void SetArgumentNameAt( unsigned int nIndex, const wchar_t * pwszName ) ;
	// 引数デフォルト値設定
	void SetArgumentDefaultAt( unsigned int nIndex, ECSObject * pDefValue ) ;
	// 引数の数設定
	void SetArgumentCount( unsigned int nCount ) ;
	// naked 関数の属性修飾
	void MakeNakedAttribute( void ) ;
	// 関数属性取得
	DWORD GetAttribute( void ) const
		{
			return	m_dwFlags ;
		}
	// 関数公開スコープ属性取得
	DWORD GetProtectedAttribute( void ) const
		{
			return	m_dwFlags & ECSTypeInfo::flagProtectedMask ;
		}
	// this call 属性（呼び出し規約判定）
	bool IsThisCall( void ) const
		{
			return	(m_dwFlags & ECSTypeInfo::flagThisCall) != 0 ;
		}
	// naked call 属性（呼び出し規約判定）
	bool IsNakedCall( void ) const
		{
			return	(m_dwFlags & ECSTypeInfo::flagNakedCall) != 0 ;
		}
	// naked mode 判定（関数実体の実装方法判定）
	bool IsNakedCodeMode( void ) const
		{
			return	(m_dwFlags & ECSTypeInfo::flagNakedCallGate)
					|| ((m_dwFlags & ECSTypeInfo::flagNakedCall)
						&& !(m_dwFlags & ECSTypeInfo::flagObjectCallGate)) ;
		}
	// 返り値型名取得
	const ECSTypeInfo & GetReturnType( void ) const
		{
			return	m_typeReturn ;
		}
	// 返り値を受け取るポインタを引数の先頭に受け取っているか？
	bool IsArgumentToReturnObject( void ) const
		{
			return	!m_typeReturn.IsVoid()
					&& !m_typeReturn.IsNakedPrimitiveDataType() ;
		}
	// 関数名取得
	const EWideString & GetName( void ) const
		{
			return	m_wstrFuncName ;
		}
	// 関数名（ファイルスコープ用）取得
	const EWideString & GetGlobalName( void ) const
		{
			return	m_wstrGlobalName ;
		}
	// 名前空間名を取得
	EWideString GetNameSpace( void ) const ;
	// 引数取得
	const EObjArray<ECSTypeInfo> & GetArgument( void ) const
		{
			return	m_lstArgument ;
		}
	const EObjArray<EWideString> & GetArgumentName( void ) const
		{
			return	m_lstArgName ;
		}
	unsigned int GetArgumentCount( void ) const
		{
			return	m_lstArgument.GetSize() ;
		}
	ECSTypeInfo * GetArgumentAt( unsigned int nIndex ) const
		{
			return	m_lstArgument.GetAt( nIndex ) ;
		}
	const wchar_t * GetArgumentNameAt( unsigned int nIndex ) const
		{
			EWideString *	pwstrName = m_lstArgName.GetAt( nIndex ) ;
			if ( pwstrName != NULL )
			{
				return	*pwstrName ;
			}
			return	NULL ;
		}
	ECSObject * GetArgumentDefaultAt( unsigned int nIndex ) const
		{
			ECSObject *	pDefault = m_lstArgDefault.GetAt( nIndex ) ;
			if ( pDefault == NULL )
			{
				ECSTypeInfo *	pArg = GetArgumentAt( nIndex ) ;
				if ( (pArg != NULL)
					&& (pArg->m_dwFlags & ECSTypeInfo::flagDeterministic) )
				{
					return	pArg->GetNakedType() ;
				}
			}
			return	pDefault ;
		}
	// 送出されうる例外リスト
	const EObjArray<EWideString> & GetThrows( void ) const
		{
			return	m_lstThrows ;
		}
	// 例外を追加
	void AddThrowType( const wchar_t * pwszType )
		{
			m_lstThrows.Add( new EWideString( pwszType ) ) ;
		}
	// 例外判定
	int FindThrowable( const wchar_t * pwszType ) const
		{
			return	m_lstThrows.Find( pwszType ) ;
		}
	bool IsThrowable( const wchar_t * pwszType ) const
		{
			return	(FindThrowable(pwszType ) >= 0) ;
		}

public:
	// クラス参照をカウント
	void EnumerateReferenceClasses
		( ENumArray<DWORD>& lstClassUsed, class ECSExecutionImage * pcsxi ) ;
	// プロトタイプ書式
	EWideString FormatPrototype( void ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// Cotopha Script クラス情報
//////////////////////////////////////////////////////////////////////////////

class	ECSClassInfo	: public ESLObject
{
public:
	// クラス情報
	DECLARE_CLASS_INFO( ECSClassInfo, ESLObject )
	// 構築関数
	ECSClassInfo( void ) ;
	ECSClassInfo( const ECSClassInfo & clsinf ) ;
	// 消滅関数
	virtual ~ECSClassInfo( void ) ;
	// 代入
	const ECSClassInfo & operator = ( const ECSClassInfo & clsinf ) ;

public:
	// 派生情報
	struct	ParentClass
	{
		DWORD			dwFlags ;			// 派生属性 enum ECSTypeInfo::Flags
		ECSClassInfo *	pClassInf ;

		bool operator == ( const ParentClass & cls ) const
			{
				return	(dwFlags == cls.dwFlags)
							&& (pClassInf->GetGlobalName()
									== cls.pClassInf->GetGlobalName()) ;
			}
		bool operator != ( const ParentClass & cls ) const
			{
				return	(dwFlags != cls.dwFlags)
							|| (pClassInf->GetGlobalName()
									!= cls.pClassInf->GetGlobalName()) ;
			}
	} ;
	// 親クラスキャスト情報
	struct	CastInfo	: public ECS_CAST_INTERFACE
	{
		DWORD			dwFlags ;			// 派生属性 enum ECSTypeInfo::Flags
		ECSClassInfo *	pClassInf ;
		int				nNakedOffset ;		// naked オフセット

		CastInfo( void )
			: ECS_CAST_INTERFACE(), dwFlags( 0 ),
				pClassInf( NULL ), nNakedOffset( 0 ) {}
		CastInfo( const CastInfo & ci )
			: ECS_CAST_INTERFACE( ci ), dwFlags( ci.dwFlags ),
					pClassInf( ci.pClassInf ), nNakedOffset( ci.nNakedOffset ) { }
		CastInfo( const ECS_CAST_INTERFACE & ci,
					DWORD flags, ECSClassInfo * clsinf )
			: ECS_CAST_INTERFACE( ci ), dwFlags( flags ),
					pClassInf( clsinf ), nNakedOffset( 0 ) { }
	} ;
	// メンバ関数情報
	class	MemberFunction	: public ECSPrototypeInfo
	{
	public:
		EWideString				m_wstrClass ;
		CastInfo *				m_pClassCast ;
		ECS_FUNCTION_POINTER	m_fpFuncPointer ;
	public:
		const ECSPrototypeInfo & operator = ( const ECSPrototypeInfo & proto )
			{
				return	ECSPrototypeInfo::operator = ( proto ) ;
			}
		bool operator == ( const MemberFunction & func ) const
			{
				return	ECSPrototypeInfo::operator == ( func ) ;
			}
		bool operator != ( const MemberFunction & func ) const
			{
				return	ECSPrototypeInfo::operator != ( func ) ;
			}
	} ;
	typedef	EPtrObjArray<MemberFunction>	ListMemberFunction ;

	// クラス定義フェーズ
	enum	DeclarationPhase
	{
		phaseEmpty,
		phaseDeclaration,
		phaseImplement,
		phaseCompleted,
	} ;
	DeclarationPhase	m_phase ;

protected:
	DWORD					m_dwFlags ;			// クラス属性 enum ECSTypeInfo::Flags
	EWideString				m_wstrClassName ;	// クラス名
	EWideString				m_wstrGlobalName ;	// クラス名（グローバルスコープ）

	EObjArray<ParentClass>	m_lstParent ;	// 親クラス情報
	EWStrTagArray<CastInfo>	m_wstaCastInf ;	// 型変換用情報

	// 親クラスまで含んだメンバ変数配列情報
	EObjArray<ECSTypeInfo>	m_lstVariable ;	// メンバ変数デフォルト値
	ECSStrBufTagArray		m_staVariable ;	// メンバ変数名
											// ネイティブクラスでは
											// コンパイル時チェック用のみ
	ENumArray<int>			m_lstVarNakedOffset ;
											// naked 構造体メンバ変数の
											// オフセットアドレス
	int						m_nNakedSize ;		// naked 構造体のサイズ
	int						m_nUnnakedObjects ; // naked されないオブジェクト数
	bool					m_flagVirtualOffset ;	// 仮想関数ベクタのオフセットを確定済み

	// 親クラスまで含んだ仮想関数配列
	EObjArray<MemberFunction>
							m_lstFuncProto ;	// メンバ関数プロトタイプ
	ECSStrBufTagArray		m_staFuncName ;		// メンバ関数名

	// デストラクタ配列
	EPtrObjArray<MemberFunction>
							m_lstDestructors ;

	// フレンドクラス配列
	EObjArray<EWideString>	m_lstFriendClass ;

	// naked クラスの初期値イメージ
	DWORD					m_dwNakedInitAddr ;

public:
	// 比較
	bool IsClassEqual( const ECSClassInfo & clsinf ) const ;
	bool operator == ( const ECSClassInfo & clsinf ) const
		{
			return	IsClassEqual( clsinf ) ;
		}
	bool operator != ( const ECSClassInfo & clsinf ) const
		{
			return	!IsClassEqual( clsinf ) ;
		}
	// 空のクラスか？
	bool IsEmpty( void ) const
		{
			return	(GetParentClassCount() == 0)
						&& (GetVariableCount() == 0)
						&& (GetFunctionCount() == 0) ;
		}
	// 基本型判定
	CSVariableType IsBasicType( void ) const ;
	// 属性設定
	void SetAttribute( DWORD dwFlags ) ;
	// 属性取得
	DWORD GetAttribute( void ) const
		{
			return	m_dwFlags ;
		}
	bool IsNamespace( void ) const
		{
			return	(m_dwFlags & ECSTypeInfo::flagNamespace) != 0 ;
		}
	bool IsUnion( void ) const
		{
			return	(m_dwFlags & ECSTypeInfo::flagUnion) != 0 ;
		}
	// クラス名設定
	void SetName( const wchar_t * pwszName ) ;
	// グローバルクラス名設定
	void SetGlobalName( const wchar_t * pwszName ) ;
	// クラス名取得
	const EWideString & GetName( void ) const
		{
			return	m_wstrClassName ;
		}
	const EWideString & GetGlobalName( void ) const
		{
			return	m_wstrGlobalName ;
		}
	// 親クラス追加
	void AddParentClassInfo( ParentClass * pParentClass ) ;
	// 親クラス数取得
	unsigned int GetParentClassCount( void ) const
		{
			return	m_lstParent.GetSize() ;
		}
	// 親クラス取得
	ParentClass * GetParentClassAt( unsigned int nIndex ) const
		{
			return	m_lstParent.GetAt( nIndex ) ;
		}
	// 親クラスキャスト情報構築
	void BuildAllClassCast( void ) ;
	void BuildClassCast
		( ECSClassInfo * pParentClass,
			bool fNativeParent, bool fRootParent, bool fVirtualOffset,
			const ECS_CAST_INTERFACE & ciParent, DWORD dwFlags ) ;
	// 親クラスキャスト追加
	void AddCastClassInfo
		( const wchar_t * pwszName, CastInfo * pCastClass ) ;
	// 親クラスキャスト情報取得
	CastInfo * GetCastClassInfoAs( const wchar_t * pwszName ) const ;
	// 親クラスキャスト情報取得
	const EWStrTagArray<CastInfo> & GetCastClassArray( void ) const
		{
			return	m_wstaCastInf ;
		}
	// 親クラスメンバ変数総数取得
	unsigned int GetParentVariableCount( void ) const ;
	// 保存用クラス情報以外を削除する
	void CleanupClassInfo( void ) ;
	// メンバ変数追加
	int AddVariable( const wchar_t * pwszName, ECSTypeInfo * pVarType ) ;
	// メンバ変数数取得
	unsigned int GetVariableCount( void ) const
		{
			ESLAssert( m_lstVariable.GetSize() == m_staVariable.GetSize() ) ;
			return	m_lstVariable.GetSize() ;
		}
	// メンバ変数検索
	int GetVariableIndex( const wchar_t * pwszName ) const
		{
			return	m_staVariable.FindIndex( pwszName ) ;
		}
	// メンバ変数取得
	ECSTypeInfo * GetVariableAt( unsigned int nIndex ) const
		{
			return	m_lstVariable.GetAt( nIndex ) ;
		}
	// メンバ変数名取得
	const wchar_t * GetVariableNameAt( unsigned int nIndex ) const
		{
			return	m_staVariable.GetAt( nIndex ) ;
		}
	// メンバ関数追加
	int AddFunction( MemberFunction * pFunc ) ;
	// 関数オーバーライド追加
	ESLError AddOverrideFunction( const ECSPrototypeInfo & prototype ) ;
	// オーバーライド関数指標検索
	int FindOverrideFunction
		( const ECSPrototypeInfo & prototype, int iFirst, int iEndBounds ) ;
	// 親クラスメンバ関数総数取得
	unsigned int GetParentFunctionCount( void ) const ;
	// メンバ関数数取得
	unsigned int GetFunctionCount( void ) const
		{
			ESLAssert( m_lstFuncProto.GetSize() == m_staFuncName.GetSize() ) ;
			return	m_lstFuncProto.GetSize() ;
		}
	// 仮想関数の総数取得
	unsigned int GetVirtualFunctionCount( int nFuncCount = -1 ) const ;
	// 仮想関数インデックスを取得
	int GetVirtualFunctionIndex( const MemberFunction * pFunc ) const ;
	// このクラスのメンバ関数取得
	MemberFunction * GetThisFunctionAs( const ECSPrototypeInfo & proto ) const ;
	// メンバ関数取得
	MemberFunction * GetFunctionAt( unsigned int nIndex ) const
		{
			return	m_lstFuncProto.GetAt( nIndex ) ;
		}
	// メンバ関数名取得
	const wchar_t * GetFunctionNameAt( unsigned int nIndex ) const
		{
			return	m_staFuncName.GetAt( nIndex ) ;
		}
	// メンバ関数検索
	int FindFunctionAs( const wchar_t * pwszName ) const
		{
			return	m_staFuncName.FindIndex( pwszName ) ;
		}
	// メンバ関数指標取得
	int FindFunctionIndex( const MemberFunction * pFunc ) const
		{
			return	m_lstFuncProto.FindPtr( (MemberFunction*) pFunc ) ;
		}

public:
	// 消滅関数をリストする
	void UpdateDestructorList( void ) ;
	// 消滅関数の数を取得する
	int GetDestructorCount( void ) const
		{
			return	m_lstDestructors.GetSize() ;
		}
	// 消滅関数を取得する
	MemberFunction * GetDestructorAt( int nIndex ) const
		{
			return	m_lstDestructors.GetAt( nIndex ) ;
		}
protected:
	// 指定クラスの消滅関数を追加する
	void AddDestructorList( ECSClassInfo * pClassInf ) ;

public:
	// 親クラスキャスト判定
	CastInfo * GetCastParentClassAs( const wchar_t * pwszClassName ) const ;
	// 関数プロトタイプ検索
	bool SearchFunctinoAs
		( ListMemberFunction & lstFunc,
			const wchar_t * pwszFuncName,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			DWORD dwFuncFlags,
			bool fNoParentClass = false,
			bool fStrictMatch = false,
			bool fPrimaryNakedCall = false ) const ;
	// 親クラスのプロトタイプ検索（SearchFunctinoAs のサブ関数）
	int GetMatchEachParentClassFunctinoAs
		( ListMemberFunction & lstFunc,
			const wchar_t * pwszFuncName,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			DWORD dwFuncFlags, DWORD dwMatchFlags,
			bool fNoParentClass, const ECSClassInfo * pParentInf ) const ;
	// 関数検索（SearchFunctinoAs のサブ関数）
	enum	MatchFunctionFlag
	{
		matchLooseArgument	= 0x00,
		matchMatchArgument	= 0x01,
		matchEqualArgument	= 0x02,
		matchNoParentClass	= 0x04,
	} ;
	int GetMatchFunctinoAs
		( ListMemberFunction & lstFunc,
			const wchar_t * pwszFuncName,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			DWORD dwFuncFlags, DWORD dwMatchFlags,
			const wchar_t * pwszMatchClassName = NULL ) const ;
	// 単項演算子オペレーター検索
	bool SearchUnaryOperatorAs
		( ListMemberFunction & lstFunc,
			CSUnaryOperatorType uoptUnary,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			bool fPrimaryNakedCall ) const ;
	// 特殊単項演算子オペレーター検索
	bool SearchExUnaryOperatorAs
		( ListMemberFunction & lstFunc,
			CSExtraUniOperatorType xuoptExUnary,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			bool fPrimaryNakedCall ) const ;
	// 二項演算子オペレーター検索
	bool SearchOperatorAs
		( ListMemberFunction & lstFunc,
			CSOperatorType optOperator,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			bool fPrimaryNakedCall ) const ;
	// 代入演算子オペレーター検索
	bool SearchMoveOperatorAs
		( ListMemberFunction & lstFunc,
			CSOperatorType optOperator,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			bool fStrictMatch, bool fPrimaryNakedCall ) const ;
	// 比較演算子オペレーター検索
	bool SearchComparatorAs
		( ListMemberFunction & lstFunc,
			CSCompareType cptCompare,
			const EPtrObjArray<ECSTypeInfo> & lstArg,
			bool fPrimaryNakedCall ) const ;
	// 同名の関数を検索
	static int FindEqualFunction
		( const ListMemberFunction & lstFunc, MemberFunction * pFunc ) ;
	// 列挙型の列挙子値判定
	bool IsMatchEnumeratorValue( const ECSObject * pValue ) const ;
	// 特定型へのキャスト可能か？
	bool CanCastTypeTo( const ECSTypeInfo& typeinf ) const ;
	// 特定型からの代入可能か？（operator の判定）
	bool CanMoveTypeFrom( const ECSObject * pType ) const ;
	// メンバ関数に純粋仮想関数があるか調べる
	bool TestAbstractFunction( void ) const ;
	// メンバ変数にクラス自体が含まれていないか（無限入れ子）チェック
	bool TestMemberVariable( const ECSClassInfo & clsinf ) const ;
	// クラス詳細情報を削除（メンバ変数とメンバ関数の情報を削除）
	void CleanupClassMember( void ) ;
	// クラス参照をカウント
	void EnumerateReferenceClasses
		( ENumArray<DWORD>& lstClassUsed, class ECSExecutionImage * pcsxi ) ;
	// 全てのメンバ関数をデバッグ出力
	void DebugTraceFunctions( void ) const ;

public:
	// 列挙型の場合、列挙子型を取得
	ECSClassInfo * GetEnumeratorObjectType( void ) const ;
	// 整数の列挙型か？
	bool IsIntegerEnumeratorType( void ) const ;
	// 実数の列挙型か？
	bool IsRealEnumeratorType( void ) const ;

public:
	// フレンドクラス追加
	void AddFriendClass( const wchar_t * pwszGlobalName ) ;
	// フレンドクラス判定
	bool IsFriendClass( const wchar_t * pwszGlobalName ) const ;

public:
	// naked クラスか？
	bool IsNakedMemoryClass( void ) const
		{
			return	(m_dwFlags & ECSTypeInfo::flagNakedBuffer) != 0 ;
		}
	// naked メモリサイズ取得
	int GetNakedMemorySize( void ) const
		{
			return	m_nNakedSize ;	
		}
	// naked 出来ないオブジェクト数取得
	int GetUnnakedObjectCount( void ) const
		{
			return	m_nUnnakedObjects ;
		}
	// メンバ変数 naked オフセット取得
	int GetVariableNakedOffsetAt( unsigned int nIndex ) const
		{
			return	m_lstVarNakedOffset.GetAt( nIndex ) ;
		}
	// naked メモリ上でのアライメント取得
	int GetNakedAlignment( void ) const ;
	// naked クラスのデフォルトコンストラクタは必要か？
	bool IsNeedsNakedClassConstruction( bool fNoCheckThisConstructor ) const ;
	static bool IsNeedsNakedVariableConstruction( ECSObject * pVarType ) ;
	// naked クラスのデストラクタ呼び出しが必要か？
	bool IsNeedsNakedClassDestruction( bool fNeedsNoNaked = false ) const ;
	static bool IsNeedsNakedVariableDestruction( ECSObject * pVarType, bool fNeedsNoNaked ) ;
	// 仮想関数ベクタを含んだメンバ変数オフセットアドレスに修正
	void NormalizeNakedOffsetForVirtualVector( void ) ;
	// クラスの naked サイズをアライメントで調整
	void NormalizeNakedClassSize( void ) ;
	// メンバの naked オフセットアドレスを加算
	void OffsetNakedVariable( int iStart, int iOffset ) ;

	friend class ECSExecutionImage ;
	friend class ECSContext ;
} ;
