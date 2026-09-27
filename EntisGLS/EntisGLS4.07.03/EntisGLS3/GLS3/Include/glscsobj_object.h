
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 変数抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSClassInfo ;
struct	ECS_CAST_INTERFACE ;
struct	ECS_FUNCTION_POINTER ;
class	ECSObject	: public	ECSSakura2::Object
{
public:
	CSVariableType		m_vtType ;		// オブジェクト型種別
	const ECSClassInfo*	m_pClassInf ;	// クラス情報（静的構造体の場合）
	ECSObject *			m_pParent ;		// 親オブジェクト（ヌルなら一時変数）
	int					m_nIndex ;		// 参照インデックス
	ECSReference *		m_pBackRef ;	// 逆参照

	ECSObject *			m_pResult ;		// 演算結果

	struct	PLUGIN_OBJECT_HEADER
	{
		ECSObject *		pBackLink ;
		void *			ptrReserved ;
	} ;
	struct	PLUGIN_OBJECT
		: public PLUGIN_OBJECT_HEADER, public ECS_OBJECT { } ;
	PLUGIN_OBJECT *	m_ppio ;	// プラグイン用インターフェース

	static ECSObject * ObjectFromPlugin( ECS_OBJECT * instance )
	{
		PLUGIN_OBJECT *	ppio = (PLUGIN_OBJECT*) instance ;
		ESLAssert( ppio->pBackLink->m_ppio == ppio ) ;
		return	ppio->pBackLink ;
	}

public:
	// 構築関数
	ECSObject( void )
		: m_vtType(csvtObject), m_pClassInf(NULL),
			m_pParent(NULL), m_nIndex(0),
			m_pBackRef(NULL), m_pResult(NULL), m_ppio(NULL) { }
	// 消滅関数
	virtual ~ECSObject( void )
		{
			if ( m_pBackRef != NULL )
			{
				ECotophaScript::LockReference( ) ;
				OnObjectDestory( this ) ;
				ECotophaScript::UnlockReference( ) ;
			}
			#if	defined(_DEBUG)
			ESLAssert( m_pResult == NULL ) ;
			delete	m_pResult ;
			#endif
			if ( m_ppio != NULL )
			{
				m_ppio->pBackLink = NULL ;
				m_ppio->Release( ) ;
				::eslHeapFree( NULL, m_ppio ) ;
			}
			#if	defined(_DEBUG)
				m_vtType = csvtInvalid ;
			#endif
		}
	// 参照バックリンクを解消
	void ReleaseBackLink( void )
		{
			if ( m_pBackRef )
			{
				ECotophaScript::LockReference( ) ;
				OnObjectDestory( this ) ;
				ECotophaScript::UnlockReference( ) ;
			}
		}
	// クラス情報
	DECLARE_CLASS_INFO( ECSObject, ECSSakura2::Object )

public:	// ECSSakura2::Object オーバーライド
	// 破棄処理
	virtual void OnDestruction
		( ECSSakura2::VirtualMachine * vm,
			ECSSakura2Processor::Context * context ) ;
	// 保存準備処理
	virtual SSystem::SError PrepareSave
		( ECSSakura2::VirtualMachine * vm,
			ECSSakura2Processor::Context * context ) ;
	// 保存処理
	virtual SSystem::SError SaveStatic
		( SSystem::SFileInterface * file,
			ECSSakura2::VirtualMachine * vm,
			ECSSakura2Processor::Context * context ) ;
	// 復元処理
	virtual SSystem::SError LoadStatic
		( SSystem::SFileInterface * file,
			ECSSakura2::VirtualMachine * vm,
			ECSSakura2Processor::Context * context ) ;
	// 復元後処理
	virtual SSystem::SError CommitAfterLoad
		( ECSSakura2::VirtualMachine * vm,
			ECSSakura2Processor::Context * context ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const = 0 ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) = 0 ;
	// オブジェクトを代入
	virtual ESLError Move
		( ECSContext & context, ECSObject * obj ) = 0 ;
	// 単項演算子
	virtual ESLError UnaryOperate
		( ECSContext & context, CSUnaryOperatorType csuopType ) = 0 ;
	// 二項演算子
	virtual ESLError Operate
		( ECSContext & context,
			CSOperatorType csopType, ECSObject * obj ) = 0 ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) = 0 ;
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
		( ECSContext & context,
			ECS_FUNCTION_POINTER & fptr, int nIndex ) ;
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
	// 特殊演算子 : interface 型変換
	virtual ESLError OperateCastInterface
			( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName ) ;
	// 整数値取得
	virtual ESLError OperateInteger( INT64 & nValue ) ;
	// 実数取得
	virtual ESLError OperateReal( REAL64 & nValue ) ;
	// 文字列取得
	virtual ESLError OperateString( EWideString & wstrValue ) ;
	// 内部バッファインターフェース
	virtual void * GetBuffer( int iOffset, int nSize, bool fWritable ) ;
	virtual void FlushBuffer( int iOffset, int nSize, void * ptrBuf, bool fModified ) ;
	virtual ECSSakura2Processor::LinearAddressCache *
			GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg ) ;
	virtual BYTE * GetSegmentShadowBuffer( int iShadow = 0 ) ;

public:		// シリアル化のための関数（システムによって必要）
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

public:		// メモリアロケーション
	// メモリ確保
	static void * operator new ( size_t stObj ) ;
	static void * operator new ( size_t stObj, void * ptrObj ) ;
	static void * operator new
		( size_t stObj, const char * pszFileName, int nLine ) ;
	// メモリ解放
	static void operator delete( void * ptrObj ) ;
	// メモリの有効性検証
	bool IsValidObject( void ) const
		{
		#if	defined(_DEBUG)
			if ( this != NULL )
			{
				return	IsValidObjectType() ;
			}
			return	false ;
		#else
			return	(this != NULL) ;
		#endif
		}
protected:
	bool IsValidObjectType( void ) const ;
public:
	// オブジェクトの実体を取得
	#if	!defined(_DEBUG)
	inline
	#endif
	static const ECSObject * GetEntity( const ECSObject * pObj ) ;
	#if	!defined(_DEBUG)
	inline
	#endif
	static ECSObject * GetEntity( ECSObject * pObj ) ;
	#if	!defined(_DEBUG)
	inline
	#endif
	ECSObject * GetObjectEntity( void ) ;
	#if	!defined(_DEBUG)
	inline
	#endif
	const ECSObject * GetObjectEntity( void ) const ;
	// メモリ参照追加
//	void AddReference( void ) ;
	// メモリ参照解放
//	void ReleaseReference( void ) ;
	// オブジェクト破棄の通知
	virtual void OnObjectDestory( ECSObject * pObj ) ;
	// スクリプトのデストラクタ
	virtual void OnDestruction( ECSContext & context ) { }

public:
	// 実数データか判定
	bool IsRealNumberObject( void ) const ;

public:
	// プラグインインターフェースを作成する
	virtual PLUGIN_OBJECT * CreateInterface( void ) ;
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;
	// プラグインインターフェースを初期化する
	static void InitializeInterface( ECS_OBJECT * pObj ) ;

protected:
	// プラグインインターフェース
	static void __stdcall PIC_Release( ECS_OBJECT * instance ) ;
	static void * __stdcall PIC_QueryInterface
		( ECS_OBJECT * instance, const wchar_t * pwszType ) ;
	static const wchar_t * __stdcall PIC_GetTypeName( ECS_OBJECT * instance ) ;
	static ECS_OBJECT * __stdcall PIC_GetTypeOf
		( ECS_OBJECT * instance, const wchar_t * pwszTypeName ) ;
	static ECS_OBJECT * __stdcall PIC_Duplicate( ECS_OBJECT * instance ) ;
	static ESLError __stdcall PIC_Move
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			const ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_UnaryOperate
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSUnaryOperatorType csuopType ) ;
	static ESLError __stdcall PIC_Operate
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSOperatorType csopType, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_Compare
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pResult, CSCompareType cscpType, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_GetVariableIndexInt
		( ECS_OBJECT * instance, int * pIndex, int iElement ) ;
	static ESLError __stdcall PIC_GetVariableIndexStr
		( ECS_OBJECT * instance, int * pIndex, const wchar_t * pwszElement ) ;
	static ECS_OBJECT * __stdcall PIC_GetVariableAt
		( ECS_OBJECT * instance, int nIndex ) ;
	static ECS_OBJECT * __stdcall PIC_SetVariableAt
		( ECS_OBJECT * instance, int nIndex, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_GetFunction
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pIndex, const wchar_t * pwszName ) ;
	static ESLError __stdcall PIC_CallFunction
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int nIndex, ECS_OBJECT * const* pArg, int nArgCount ) ;
	static void __stdcall PIC_IndexAllMember( ECS_OBJECT * instance ) ;
	static void __stdcall PIC_CleanupAllReference
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_CommitAllReference
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_Save
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_Load
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_DumpObject
		( ECS_OBJECT * instance, ECS_FILE * pfile,
			int nIndent, ECS_CONTEXT * context ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 関数ポインタオブジェクト
//////////////////////////////////////////////////////////////////////////////

#pragma	pack( push, __CSDATA_ALIGN__, 1 )

struct	ECS_CAST_INTERFACE
{
	union
	{
		ECSObject *	pCastObject ;
		DWORD_PTR	iNativeParent ;
	} ;
	int			iVarOffset ;
	int			nVarBounds ;
	int			iFuncOffset ;

	ECS_CAST_INTERFACE( void ) {}
	ECS_CAST_INTERFACE
		( ECSObject * pObj,
			int iVar = 0, int nBounds = -1, int iFunc = 0 )
			: pCastObject( pObj ), iVarOffset( iVar ),
				nVarBounds( nBounds ), iFuncOffset( iFunc ) {}
} ;

struct	ECS_FUNCTION_POINTER
{
	typedef	ESLError (ECSObject::*NATIVE_CALL)
			( ECSContext & context,
				int nIndex, ECSObjArray<ECSObject> & lstArg ) ;
	enum	FunctionType
	{
		funcIndexCall,
		funcScriptCall,
		funcNativeCall,
		funcNakedCall,
	} ;
	FunctionType			m_ftType ;
	ECS_CAST_INTERFACE		m_castThis ;
	DWORD					m_dwAlign ;
	union
	{
		int				nIndex ;
		DWORD_PTR		addrScript ;
		NATIVE_CALL		pfnNative ;
		FARPROC			pfnNaked ;
	}				m_varFunc ;
	DWORD					m_dwPadding[3] ;

	ECS_FUNCTION_POINTER( void )
	{
		m_dwAlign = 0 ;
		m_dwPadding[0] = 0 ;
		m_dwPadding[1] = 0 ;
		m_dwPadding[2] = 0 ;
	}
} ;

#pragma	pack( pop, __CSDATA_ALIGN__ )


//////////////////////////////////////////////////////////////////////////////
// プラグイン拡張用抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSPIObject	: public	ECSObject
{
public:
	// 構築関数
	ECSPIObject( void ) ;

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
		( ECSContext & context,
			CSOperatorType csopType, ECSObject * obj ) ;
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

public:		// シリアル化のための関数（システムによって必要）
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

public:
	// プラグインインターフェースを作成する
	virtual PLUGIN_OBJECT * CreateInterface( void ) ;

protected:
	// プラグインインターフェース
	static void __stdcall PIC_Release( ECS_OBJECT * instance ) ;
	static void * __stdcall PIC_QueryInterface
		( ECS_OBJECT * instance, const wchar_t * pwszType ) ;
	static const wchar_t * __stdcall PIC_GetTypeName( ECS_OBJECT * instance ) ;
	static ECS_OBJECT * __stdcall PIC_GetTypeOf
		( ECS_OBJECT * instance, const wchar_t * pwszTypeName ) ;
	static ECS_OBJECT * __stdcall PIC_Duplicate( ECS_OBJECT * instance ) ;
	static ESLError __stdcall PIC_Move
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			const ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_UnaryOperate
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSUnaryOperatorType csuopType ) ;
	static ESLError __stdcall PIC_Operate
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSOperatorType csopType, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_Compare
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pResult, CSCompareType cscpType, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_GetVariableIndex
		( ECS_OBJECT * instance, int * pIndex, int iMember ) ;
	static ESLError __stdcall PIC_GetVariableIndex
		( ECS_OBJECT * instance, int * pIndex, const wchar_t * pwszName ) ;
	static ECS_OBJECT * __stdcall PIC_GetVariableAt
		( ECS_OBJECT * instance, int nIndex ) ;
	static ECS_OBJECT * __stdcall PIC_SetVariableAt
		( ECS_OBJECT * instance, int nIndex, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PIC_GetFunction
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pIndex, const wchar_t * pwszName ) ;
	static ESLError __stdcall PIC_CallFunction
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int nIndex, ECS_OBJECT * const* pArg, int nArgCount ) ;
	static void __stdcall PIC_IndexAllMember( ECS_OBJECT * instance ) ;
	static void __stdcall PIC_CleanupAllReference
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_CommitAllReference
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_Save
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_Load
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_DumpObject
		( ECS_OBJECT * instance, ECS_FILE * pfile,
			int nIndent, ECS_CONTEXT * context ) ;

} ;
