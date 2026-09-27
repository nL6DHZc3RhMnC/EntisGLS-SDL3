
//////////////////////////////////////////////////////////////////////////////
// 整数オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSInteger	: public	ECSObject
{
protected:
	INT64	m_varInt ;
	INT64	m_varMask ;
public:
	inline INT64 GetValue( void ) const
		{
			return	m_varInt ;
		}
	inline long int GetInt( void ) const
		{
			return	(long int) m_varInt ;
		}
	inline void SetValue( INT64 nValue )
		{
			nValue &= m_varMask ;
			m_varInt = nValue | ((nValue >> 63) & ~m_varMask) ;
		}
	inline INT64 GetValueMask( void ) const
		{
			return	m_varMask ;
		}
	inline void SetValueMask( INT64 nMask )
		{
			m_varMask = nMask ;
		}
	inline bool IsSign( void ) const
		{
			return	(m_varMask & 0x8000000000000000) != 0 ;
		}
	inline int SizeOf( void ) const
		{
			if ( m_varMask & 0x7FFFFFFF00000000 )
			{
				return	64 ;
			}
			else if ( m_varMask & 0xFFFF0000 )
			{
				return	32 ;
			}
			else if ( m_varMask & 0xFF00 )
			{
				return	16 ;
			}
			else if ( m_varMask & 0xFF )
			{
				return	8 ;
			}
			else
			{
				return	1 ;
			}
		}
	CSVariableType GetIntegerType( void ) const ;

	struct	PLUGIN_OBJECT_HEADER
	{
		ECSInteger *	pBackLink ;
	} ;
	struct	PLUGIN_INTEGER
		: public PLUGIN_OBJECT_HEADER, public ECS_INTEGER_INTERFACE { } ;
	PLUGIN_INTEGER	m_pii ;		// プラグイン用インターフェース

public:
	static const INT64	m_maskBoolean ;
	static const INT64	m_maskInt8 ;
	static const INT64	m_maskInt16 ;
	static const INT64	m_maskInt32 ;
	static const INT64	m_maskUint8 ;
	static const INT64	m_maskUint16 ;
	static const INT64	m_maskUint32 ;
	static const INT64	m_maskInt64 ;
	static const INT64	m_maskUint64 ;

public:
	// 構築関数
	ECSInteger( INT64 nInit = 0, INT64 nMask = -1 )
		{ m_vtType = csvtInteger ; m_varInt = nInit ; m_varMask = nMask ; }
	// クラス情報
	DECLARE_CLASS_INFO( ECSInteger, ECSObject )

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
	// 特殊演算子 : boolean 判定
	virtual ESLError OperateBoolean( int & nBoolean ) ;
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
	typedef	ESLError (ECSInteger::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[11] ;
	static const PFUNC_CALL	m_pfnCallFunc[10] ;
	// メンバ関数
	ESLError Call_Char
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Format
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Abs
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_TestBit
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetBit
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ResetBit
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RotateLeft
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RotateRight
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ShiftLeft
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ShiftRight
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static long int __stdcall PIC_GetInteger
		( ECS_INTEGER_INTERFACE * instance ) ;
	static void __stdcall PIC_SetInteger
		( ECS_INTEGER_INTERFACE * instance, long int nVal ) ;
	static INT64 __stdcall PIC_GetInteger64
		( ECS_INTEGER_INTERFACE * instance ) ;
	static void __stdcall PIC_SetInteger64
		( ECS_INTEGER_INTERFACE * instance, INT64 nVal ) ;

	friend	ECSContext ;

} ;
