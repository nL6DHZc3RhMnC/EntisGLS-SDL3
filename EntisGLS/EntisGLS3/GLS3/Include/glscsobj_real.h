
//////////////////////////////////////////////////////////////////////////////
// 実数オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSReal	: public	ECSObject
{
public:
	CSVariableType	m_vtRealType ;
	double			m_varReal ;

	inline int SizeOf( void ) const
		{
			if ( m_vtRealType == csvtReal32 )
			{
				return	32 ;
			}
			return	64 ;
		}

	struct	PLUGIN_OBJECT_HEADER
	{
		ECSReal *	pBackLink ;
	} ;
	struct	PLUGIN_REAL
		: public PLUGIN_OBJECT_HEADER, public ECS_REAL_INTERFACE { } ;
	PLUGIN_REAL	m_pir ;		// プラグイン用インターフェース

public:
	// 構築関数
	ECSReal( double rInit = 0.0 )
		{ m_vtType = csvtReal ;
			m_vtRealType = csvtReal ; m_varReal = rInit ; }
	// クラス情報
	DECLARE_CLASS_INFO( ECSReal, ECSObject )

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
	ESLError Operate
		( ECSContext & context, CSOperatorType csopType, ECSObject & obj ) ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;
	// 特殊演算子 : sizeof
	virtual ESLError OperateSizeOf( INT64 & nSize ) ;
	// 特殊演算子 : typeof
	virtual const wchar_t * OperateTypeOf( void ) const ;
	// 整数値取得
	virtual ESLError OperateInteger( INT64 & nValue ) ;
	// 実数取得
	virtual ESLError OperateReal( REAL64 & nValue ) ;
	// 文字列取得
	virtual ESLError OperateString( EWideString & wstrValue ) ;
	// 内部バッファインターフェース
	virtual void * GetBuffer( int iOffset, int nSize, bool fWritable ) ;
	virtual ECSSakura2Processor::LinearAddressCache *
			GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg ) ;

public:
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSReal::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[14] ;
	static const PFUNC_CALL	m_pfnCallFunc[13] ;
	// メンバ関数
	ESLError Call_Pi
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Abs
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Log
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Power
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Sqrt
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Sin
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Cos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Tan
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ASin
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ACos
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ATan
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Round
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Floor
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static double __stdcall PIC_GetReal
		( ECS_REAL_INTERFACE * instance ) ;
	static void __stdcall PIC_SetReal
		( ECS_REAL_INTERFACE * instance, double rVal ) ;

} ;
