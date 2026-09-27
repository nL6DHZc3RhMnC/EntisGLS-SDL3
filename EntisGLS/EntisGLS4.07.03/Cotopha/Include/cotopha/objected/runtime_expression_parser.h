
#if	!defined(__RUNTIME_EXPRESSION_PARSER_H__)
#define	__RUNTIME_EXPRESSION_PARSER_H__

//////////////////////////////////////////////////////////////////////////////
// 実行時式パーサー（詞葉オブジェクトベース）
//////////////////////////////////////////////////////////////////////////////

class	RuntimeExpressionParser
{
public:
	Global&			m_refConstant ;
	Hash			m_mapConstant ;
	Hash<Reference>	m_mapGlobal ;

public:
	// 定数テーブル関連付け
	void AttachConstantTable( Global& rConstant ) ;
	// 定数設定
	void DefineConstant( String sName, Reference rValue ) ;
	// グローバル変数設定
	void AttachGlobalVariable( String sName, Reference rVariable ) ;

public:
	// 実行時式評価
	virtual Reference EvaluateExpression
		( String& strExpr, int nPriority = 0,
			String sCloser = "", bool fAutoCloserNext = true ) ;
	// 演算子優先度（演算子でない場合には -1）
	static int GetOperatorPriority( String sOp ) ;
	static int GetUnaryOperatorPriority( String sOp ) ;
	// エラー出力
	virtual void OutputError( String sErr ) ;

} ;


#endif

