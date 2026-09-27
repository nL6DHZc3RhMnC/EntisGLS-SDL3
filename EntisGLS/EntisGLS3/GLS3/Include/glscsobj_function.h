
//////////////////////////////////////////////////////////////////////////////
// 関数ポインタオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSFunction	: public ECSObject
{
public:
	// 構築関数
	ECSFunction( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSFunction, ECSObject )

public:
	UINT64				m_fpAddress ;	// 関数アドレス
	ECSPrototypeInfo	m_prototype ;	// プロトタイプ
	ECSClassInfo *		m_pThisCall ;	// this call クラス

public:
	// 代入
	const ECSFunction & operator = ( const ECSFunction & func ) ;
	// スクリプト関数設定（ランタイム）
	void SetFunction
		( ECSContext & context, const wchar_t * pwszFuncName ) ;

public:		// 通常のオブジェクト処理
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
	// 特殊演算子 : boolean 判定
	virtual ESLError OperateBoolean( int & nBoolean ) ;
	// 整数値取得
	virtual ESLError OperateInteger( INT64 & nValue ) ;

public:		// シリアル化のための関数（システムによって必要）
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

} ;

