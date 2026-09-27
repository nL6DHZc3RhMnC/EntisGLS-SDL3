
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 大域変数名前空間
//////////////////////////////////////////////////////////////////////////////

class	ECSGlobal	: public	ECSArray
{
public:
	ECSStrBufTagArray	m_staObjName ;		// 大域変数名前空間
	ECSObject *			m_pElementType ;

public:
	// 構築関数
	ECSGlobal( void ) ;
	// 消滅関数
	virtual ~ECSGlobal( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSGlobal, ECSArray )

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
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// 変数追加
	ESLError AddVariable( const wchar_t * pwszName, ECSObject * pObj ) ;
	// 変数全て削除
	void RemoveAllVariable( void ) ;

public:
	// 変数名取得
	ESLError Call_GetTagName
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// 要素有無判定
	ESLError Call_IsEmpty
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;
