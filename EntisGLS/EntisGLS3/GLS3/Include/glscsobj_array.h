
//////////////////////////////////////////////////////////////////////////////
// 配列オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSArray	: public	ECSObject
{
public:
	ECSObjArray<ECSObject>	m_varArray ;
	ECSObject *				m_pDefObj ;
	unsigned int			m_nBounds ;

	struct	PLUGIN_OBJECT_HEADER
	{
		ECSArray *	pBackLink ;
	} ;
	struct	PLUGIN_ARRAY
		: public PLUGIN_OBJECT_HEADER, public ECS_ARRAY_INTERFACE { } ;
	PLUGIN_ARRAY	m_pia ;		// プラグイン用インターフェース

public:
	// 構築関数
	ECSArray( void )
		: m_pDefObj( NULL ), m_nBounds( 0x80000000 ) { m_vtType = csvtArray ; }
	// 消滅関数
	virtual ~ECSArray( void ) { delete m_pDefObj ; }
	// クラス情報
	DECLARE_CLASS_INFO( ECSArray, ECSObject )

public:
	// 要素を複製する
	void CopyFrom( ECSArray & obj, int iFirst = 0, int nCount = -1 ) ;
	// 要素を代入する
	ESLError MoveFrom( ECSContext & context, ECSArray & obj ) ;
	// 要素代入処理
	static ESLError MoveElement
		( ECSContext & context, ECSObject *& pDst,
				ECSObject & objSrc, ECSObject * pDefValue ) ;
	// 配列要素削除
	void RemoveBetween
		( ECSContext & context, int nIndex = 0, int nCount = -1 ) ;
	// 配列要素有効性チェック
	bool VerifyAllElementValidation( void ) const ;
	// デフォルト要素
	void SetDefaultElement( ECSObject * pDefObj ) ;
	// 配列境界設定
	void SetBounds( unsigned int nBounds = 0x80000000 )
		{
			m_nBounds = nBounds ;
		}
	bool IsBounds( void )
		{
			return	!(m_nBounds & 0x80000000) ;
		}
	// 配列境界取得
	unsigned int GetBounds( void ) const
		{
			return	m_nBounds ;
		}
	// 多次元配列生成
	void MakeDimension
		( const unsigned int nBounds[], int nDim, ECSObject * pDefObj ) ;
	// （デフォルト要素の設定から）次元取得
	int GetDimension( void ) const ;
	// （デフォルト要素の設定から）次元サイズ取得
	int GetDimensionSize( unsigned int nBounds[], int nDim ) const ;
	// （デフォルト要素の設定から）末端デフォルト値取得
	ECSObject * GetEndDefaultElement( void ) const ;

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
	typedef	ESLError (ECSArray::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[11] ;
	static const PFUNC_CALL	m_pfnCallFunc[10] ;
	// メンバ関数
	ESLError Call_GetLength
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsEmpty
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Find
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Swap
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Push
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Pop
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Insert
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Remove
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Detach
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Merge
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static unsigned int __stdcall
		PIC_GetLength( ECS_ARRAY_INTERFACE * instance ) ;
	static int __stdcall PIC_IsEmpty
		( ECS_ARRAY_INTERFACE * instance, int nIndex ) ;
	static void __stdcall PIC_Swap
		( ECS_ARRAY_INTERFACE * instance, int nIndex1, int nIndex2 ) ;
	static void __stdcall PIC_Insert
		( ECS_ARRAY_INTERFACE * instance, int nIndex, ECS_OBJECT * pObj ) ;
	static void __stdcall PIC_Remove
		( ECS_ARRAY_INTERFACE * instance, int nIndex, int nCount ) ;
	static ECS_OBJECT * __stdcall PIC_Detach
		( ECS_ARRAY_INTERFACE * instance, int nIndex ) ;

} ;

