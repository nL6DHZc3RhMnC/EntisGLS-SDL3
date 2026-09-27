
//////////////////////////////////////////////////////////////////////////////
// naked 構造体
//////////////////////////////////////////////////////////////////////////////

class	ECSBufferStructure	: public ECSBuffer, public ECSStructureInterface
{
public:
	// 構築関数
	ECSBufferStructure( void ) {}
	ECSBufferStructure( const ECSClassInfo * pClassInf ) { m_pClassInf = pClassInf ; }
	ECSBufferStructure( const ECSBufferStructure & buf )
		: ECSBuffer( buf ) { m_pClassInf = buf.m_pClassInf ; }
	// クラス情報
	DECLARE_CLASS_INFO2( ECSBufferStructure, ECSBuffer, ECSStructureInterface )

public:
	// ECSObject インスタンス
	virtual ECSObject * GetInstanceObject( void ) ;
	// 整数のメンバ変数を取得する
	virtual int GetMemberAsInt
		( const wchar_t * pwszName, int nDefValue ) ;
	// 実数のメンバ変数を取得する
	virtual double GetMemberAsReal
		( const wchar_t * pwszName, double rDefValue ) ;
	// 整数のメンバ変数を設定する
	virtual void SetMemberAsInt
		( const wchar_t * pwszName, int nValue ) ;
	// 実数のメンバ変数を設定する
	virtual void SetMemberAsReal
		( const wchar_t * pwszName, double rValue ) ;

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
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
	// スクリプトのデストラクタ
	virtual void OnDestruction( ECSContext & context ) ;

} ;
