
//////////////////////////////////////////////////////////////////////////////
// 連想配列オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSHash	: public	ECSObject
{
public:
	ECSWStrTagArray<ECSObject>	m_varArray ;
	ECSObject *					m_pDefObj ;

	struct	PLUGIN_OBJECT_HEADER
	{
		ECSHash *	pBackLink ;
	} ;
	struct	PLUGIN_HASH
		: public PLUGIN_OBJECT_HEADER, public ECS_HASH_INTERFACE { } ;
	PLUGIN_HASH		m_pih ;		// プラグイン用インターフェース

public:
	// 構築関数
	ECSHash( void ) : m_pDefObj(NULL) { m_vtType = csvtHash ; }
	// 消滅関数
	virtual ~ECSHash( void ) { delete m_pDefObj ; }
	// クラス情報
	DECLARE_CLASS_INFO( ECSHash, ECSObject )

public:
	// 要素を複製する
	void CopyFrom( ECSHash & obj ) ;
	// 要素を代入する
	ESLError MoveFrom( ECSContext & context, ECSHash & obj ) ;
	// デフォルト要素を設定する
	void SetDefaultElement( ECSObject * pDefObj ) ;
	// 配列要素有効性チェック
	bool VerifyAllElementValidation( void ) const ;
	// 配列要素削除
	void RemoveElementAt( ECSContext & context, int nIndex ) ;
	void RemoveElementAs( ECSContext & context, const wchar_t * pwszTag ) ;
	void RemoveAll( ECSContext & context ) ;

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// 単項演算子
	virtual ESLError UnaryOperate
		( ECSContext & context, CSUnaryOperatorType csuopType ) ;
	// 二項演算子
	virtual ESLError Operate
		( ECSContext & context, CSOperatorType csopType, ECSObject * obj ) ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) ;
	// メンバ変数インデックス取得
	virtual ESLError GetVariableIndex( int & nIndex, int iMember ) ;
	virtual ESLError GetVariableIndex
					( int & nIndex, const wchar_t * pwszMember ) ;
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;
	// 特殊演算子 : sizeof
	virtual ESLError OperateSizeOf( INT64 & nSize ) ;

public:
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;
	// スクリプトのデストラクタ
	virtual void OnDestruction( ECSContext & context ) ;

	// 配列要素の参照を解消する
	void CleanupAllElementRef( ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSHash::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[10] ;
	static const PFUNC_CALL	m_pfnCallFunc[9] ;
	// メンバ関数
	ESLError Call_GetLength
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsEmpty
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetTagName
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FindTagIndex
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OrderIndex
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Detach
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Remove
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RemoveAll
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetDefaultElement
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// 整数のメンバ変数を取得する
	int GetMemberAsInt( const wchar_t * pwszName, int nDefValue ) ;
	// 実数のメンバ変数を取得する
	double GetMemberAsReal( const wchar_t * pwszName, double rDefValue ) ;
	// 文字列のメンバ変数を取得する
	ECSWideString GetMemberAsStr
		( const wchar_t * pwszName, const wchar_t * pwszDefValue ) ;
	// 整数のメンバ変数を設定する
	void SetMemberAsInt( const wchar_t * pwszName, int nValue ) ;
	// 実数のメンバ変数を設定する
	void SetMemberAsReal( const wchar_t * pwszName, double rValue ) ;
	// 文字列のメンバ変数を設定する
	void SetMemberAsStr
		( const wchar_t * pwszName, const wchar_t * pwszValue ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static unsigned int __stdcall PIC_GetLength
		( ECS_HASH_INTERFACE * instance ) ;
	static ECS_OBJECT * __stdcall PIC_GetElement
		( ECS_HASH_INTERFACE * instance, const wchar_t * pwszTag ) ;
	static ECS_OBJECT * __stdcall PIC_GetTagName
		( ECS_HASH_INTERFACE * instance, int nIndex ) ;
	static void __stdcall PIC_Remove
		( ECS_HASH_INTERFACE * instance, const wchar_t * pwszTag ) ;
	static void __stdcall PIC_RemoveAll( ECS_HASH_INTERFACE * instance ) ;
	static void __stdcall PIC_SetDefaultElement
		( ECS_HASH_INTERFACE * instance, ECS_OBJECT * pDefault ) ;

} ;
