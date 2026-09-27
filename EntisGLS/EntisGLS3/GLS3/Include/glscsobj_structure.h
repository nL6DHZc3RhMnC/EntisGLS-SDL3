
//////////////////////////////////////////////////////////////////////////////
// 構造体インターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSStructureInterface : public ESLObject
{
public:
	// クラス情報
	DECLARE_CLASS_INFO( ECSStructureInterface, ESLObject )
	// ECSObject インスタンス
	virtual ECSObject * GetInstanceObject( void ) = 0 ;
	// 整数のメンバ変数を取得する
	virtual int GetMemberAsInt
		( const wchar_t * pwszName, int nDefValue ) = 0 ;
	// 実数のメンバ変数を取得する
	virtual double GetMemberAsReal
		( const wchar_t * pwszName, double rDefValue ) = 0 ;
	// 整数のメンバ変数を設定する
	virtual void SetMemberAsInt
		( const wchar_t * pwszName, int nValue ) = 0 ;
	// 実数のメンバ変数を設定する
	virtual void SetMemberAsReal
		( const wchar_t * pwszName, double rValue ) = 0 ;

public:
	// ECSObject へキャスト
	operator ECSObject * ( void )
		{
			if ( this != NULL )
			{
				return	GetInstanceObject() ;
			}
			return	NULL ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 構造体
//////////////////////////////////////////////////////////////////////////////

class	ECSStructure	: public ECSArray, public ECSStructureInterface
{
public:
	// 構築関数
	ECSStructure( void ) ;
	ECSStructure( const ECSClassInfo * pClassInf ) ;
	// 消滅関数
	virtual ~ECSStructure( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSStructure, ECSArray, ECSStructureInterface )

public:
	const wchar_t *			m_pwszTag ;		// 構造体名（静的バッファへのポインタ）
	ECSStrBufTagArray		m_staMember ;	// メンバ変数名（動的構造体の場合）

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
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
	// メンバ関数ポインタ取得
	virtual ESLError GetFunctionPointer
		( ECSContext & context,
			ECS_FUNCTION_POINTER & fptr, const wchar_t * pwszName ) ;
	virtual ESLError GetFunctionPointer
		( ECSContext & context, ECS_FUNCTION_POINTER & fptr, int nIndex ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;
	// 特殊演算子 : interface 型変換
	virtual ESLError OperateCastInterface
			( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName ) ;

public:
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;
	// スクリプトのデストラクタ
	virtual void OnDestruction( ECSContext & context ) ;

public:
	// メンバを取得（動的構造体）
	ECSObject * GetDynamicVariableAs( const wchar_t * pwszMember ) ;
	// メンバ変数の指標を取得
	int FindVariableIndex( const wchar_t * pwszMember ) const ;
	// メンバ変数名を取得
	const wchar_t * GetVariableName( int nIndex ) const ;
	// オブジェクト代入
	ESLError MoveFromStructure
		( ECSContext & context, const ECSStructure & obj ) ;
	// オブジェクト代入
	ESLError MoveFromHash
		( ECSContext & context, const ECSHash & obj ) ;
	// メンバへオブジェクト代入
	ESLError MoveMemberAs
		( ECSContext & context, const wchar_t * pwszName, ECSObject * obj ) ;
	ESLError MoveMemberAt
		( ECSContext & context, int iMember, ECSObject * obj ) ;
	// メンバ変数追加
	ESLError AddNewVariable( const wchar_t * pwszName, ECSObject * pObj ) ;

public:
	// ECSObject インスタンス
	virtual ECSObject * GetInstanceObject( void ) ;
	// メンバを取得
	ECSObject * GetMemberAs( const wchar_t * pwszMember ) ;
	// 整数のメンバ変数を取得する
	virtual int GetMemberAsInt( const wchar_t * pwszName, int nDefValue ) ;
	// 実数のメンバ変数を取得する
	virtual double GetMemberAsReal( const wchar_t * pwszName, double rDefValue ) ;
	// 文字列のメンバ変数を取得する
	ECSWideString GetMemberAsStr
		( const wchar_t * pwszName, const wchar_t * pwszDefValue ) ;
	// 整数のメンバ変数を設定する
	virtual void SetMemberAsInt( const wchar_t * pwszName, int nValue ) ;
	// 実数のメンバ変数を設定する
	virtual void SetMemberAsReal( const wchar_t * pwszName, double rValue ) ;
	// 文字列のメンバ変数を設定する
	void SetMemberAsStr
		( const wchar_t * pwszName, const wchar_t * pwszValue ) ;

} ;
