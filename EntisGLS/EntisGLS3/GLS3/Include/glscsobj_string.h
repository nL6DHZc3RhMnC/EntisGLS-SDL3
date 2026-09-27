
//////////////////////////////////////////////////////////////////////////////
// 文字列オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSString	: public	ECSObject
{
public:
	ECSSourceStream/*ECSWideString*/	m_varStr ;
	int									m_nLockBufSize ;

	struct	PLUGIN_OBJECT_HEADER
	{
		ECSString *	pBackLink ;
	} ;
	struct	PLUGIN_STRING
		: public PLUGIN_OBJECT_HEADER, public ECS_STRING_INTERFACE { } ;
	PLUGIN_STRING	m_pis ;		// プラグイン用インターフェース

public:
	// 構築関数
	ECSString( void ) { m_vtType = csvtString ; m_nLockBufSize = -1 ; }
	ECSString( const wchar_t * pwszInit )
		: m_varStr( pwszInit )
					{ m_vtType = csvtString ; m_nLockBufSize = -1 ; }
	// クラス情報
	DECLARE_CLASS_INFO( ECSString, ECSObject )

public:
	// 型変換
	operator const EWideString & ( void )
		{
			return	m_varStr ;
		}
	// 書き込み用内部バッファ取得
	wchar_t * LockBuffer( int nLength ) ;
	// 書き込み用内部バッファ確定
	void UnlockBuffer( int nStrLen = -1 ) ;

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
	// スクリプトのデストラクタ
	virtual void OnDestruction( ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSString::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[43] ;
	static const PFUNC_CALL	m_pfnCallFunc[42] ;
	// メンバ関数
	ESLError Call_GetLength
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Char
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetChar
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Left
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Right
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Middle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MakeUpper
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MakeLower
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_TrimLeft
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_TrimRight
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Compare
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CompareNoCase
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CompareLeft
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CompareLeftNoCase
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OffsetFilePath
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ParseFileDrive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ParseFileDirectory
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ParseFileName
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ParseFileTitle
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ParseFileExtension
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCEncoded
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCDecoded
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetXMLEncoded
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetXMLDecoded
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Find
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Replace
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Separate
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Calculate
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Execute
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsMatchUsage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetIndex
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SeekIndex
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SeekNext
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextChar
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextInteger
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextRealNumber
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextNeedChar
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextString
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextToken
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextEnclosedString
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_NextMatchUsage
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Call
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static const wchar_t * __stdcall PIC_GetString
		( ECS_STRING_INTERFACE * instance ) ;
	static wchar_t * __stdcall PIC_GetBuffer
		( ECS_STRING_INTERFACE * instance, unsigned int nLength ) ;
	static void __stdcall PIC_ReleaseBuffer
		( ECS_STRING_INTERFACE * instance, int nLength ) ;

} ;

extern	"C"
{
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_GetLength
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_GetBuffer
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_SetString
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_AppendString
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_AppendInteger
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_AppendHexInteger
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_AppendReal
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_LockBuffer
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
	ECS_EXPORT const wchar_t *
		ecs_nakedcall_String_UnlockBuffer
			( ECSSakura2Processor::Context * pcontext,
				const ECSSakura2Processor::Register * pArg ) ;
} ;

