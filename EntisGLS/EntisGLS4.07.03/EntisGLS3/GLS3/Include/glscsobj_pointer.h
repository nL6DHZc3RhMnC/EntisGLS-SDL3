
//////////////////////////////////////////////////////////////////////////////
// ポインタ参照オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSPointerReference	: public	ECSReference
{
public:
	int				m_iOffset ;
	CSVariableType	m_csvtRefType ;
	bool			m_fReadOnly ;

public:
	// 構築関数
	ECSPointerReference
			( int iOffset = 0,
				CSVariableType csvtRefType = csvtObject,
				bool fReadOnly = false )
		{
			m_vtType = csvtPointerReference ;
			m_iOffset = iOffset ;
			m_csvtRefType = csvtRefType ;
			m_fReadOnly = fReadOnly ;
		}
	// クラス情報
	DECLARE_CLASS_INFO( ECSPointerReference, ECSReference )

public:
	// オフセットアドレス
	inline int GetOffset( void ) const
		{
			return	m_iOffset ;
		}
	inline void SetOffset( int iOffset )
		{
			m_iOffset = iOffset ;
		}
	// 参照型
	inline CSVariableType GetReferenceType( void ) const
		{
			return	m_csvtRefType ;
		}
	inline void SetReferenceType( CSVariableType csvtRefType )
		{
			m_csvtRefType = csvtRefType ;
		}
	// ポインタ複製
	void CopyPointerFrom( ECSContext * context, const ECSPointerReference & ptr )
		{
			SetReferenceCastInterface( ptr.m_pRef, context, ptr ) ;
			m_iOffset = ptr.m_iOffset ;
			m_csvtRefType = ptr.m_csvtRefType ;
			m_fReadOnly = ptr.m_fReadOnly ;
		}

public:
	typedef INT64 (*PFUNC_LOAD_INT)( const void * ptrBuf ) ;
	typedef double (*PFUNC_LOAD_REAL)( const void * ptrBuf ) ;
	typedef ESLError (*PFUNC_STORE_INT)( void * ptrBuf, INT64 nValue ) ;
	typedef ESLError (*PFUNC_SOTRE_REAL)( void * ptrBuf, double nValue ) ;

	static const int				m_nTypeBytes[csvtExTypeMax] ;
	static const CSVariableType		m_csvtTypeIntFloat[csvtExTypeMax] ;
	static const PFUNC_LOAD_INT		m_pfnLoadIntFunc[csvtExTypeMax] ;
	static const PFUNC_LOAD_REAL	m_pfnLoadRealFunc[csvtExTypeMax] ;
	static const PFUNC_STORE_INT	m_pfnStoreIntFunc[csvtExTypeMax] ;
	static const PFUNC_SOTRE_REAL	m_pfnStoreRealFunc[csvtExTypeMax] ;

public:
	// メモリ上のサイズを取得する
	int GetMemoryObjectBytes( void ) const
	{
		return	m_nTypeBytes[m_csvtRefType] ;
	}
	// ロード／ストア型を取得する
	CSVariableType GetMemoryObjectType( void ) const
	{
		return	m_csvtTypeIntFloat[m_csvtRefType] ;
	}
	static CSVariableType GetMemoryObjectType( CSVariableType csvtType )
	{
		return	m_csvtTypeIntFloat[csvtType] ;
	}
	// ロード
	ESLError LoadBufferInteger( INT64 & numLoaded )
	{
		if ( m_pRef->IsValidObject() )
		{
			void *	ptrBuf =
				m_pRef->GetBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], false ) ;
			if ( ptrBuf != NULL )
			{
				numLoaded = (*(m_pfnLoadIntFunc[m_csvtRefType]))( ptrBuf ) ;
				m_pRef->FlushBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], ptrBuf, false ) ;
				return	eslErrSuccess ;
			}
		}
		return	eslErrFailed ;
	}
	ESLError LoadBufferReal( double & numLoaded )
	{
		if ( m_pRef->IsValidObject() )
		{
			void *	ptrBuf =
				m_pRef->GetBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], false ) ;
			if ( ptrBuf != NULL )
			{
				numLoaded = (*(m_pfnLoadRealFunc[m_csvtRefType]))( ptrBuf ) ;
				m_pRef->FlushBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], ptrBuf, false ) ;
				return	eslErrSuccess ;
			}
		}
		return	eslErrFailed ;
	}
	static INT64 LoadBufferInteger
		( const void * ptrBuf, CSVariableType csvtType )
	{
		return	(*(m_pfnLoadIntFunc[csvtType]))( ptrBuf ) ;
	}
	static double LoadBufferReal
		( const void * ptrBuf, CSVariableType csvtType )
	{
		return	(*(m_pfnLoadRealFunc[csvtType]))( ptrBuf ) ;
	}
	// ストア
	ESLError StoreBufferInteger( INT64 numStore )
	{
		if ( m_pRef->IsValidObject() )
		{
			void *	ptrBuf =
				m_pRef->GetBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], true ) ;
			if ( ptrBuf != NULL )
			{
				ESLError	err =
					(*(m_pfnStoreIntFunc[m_csvtRefType]))( ptrBuf, numStore ) ;
				m_pRef->FlushBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], ptrBuf, true ) ;
				return	err ;
			}
		}
		return	eslErrFailed ;
	}
	ESLError StoreBufferReal( double numStore )
	{
		if ( m_pRef->IsValidObject() )
		{
			void *	ptrBuf =
				m_pRef->GetBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], true ) ;
			if ( ptrBuf != NULL )
			{
				ESLError	err =
					(*(m_pfnStoreRealFunc[m_csvtRefType]))( ptrBuf, numStore ) ;
				m_pRef->FlushBuffer
					( m_iOffset, m_nTypeBytes[m_csvtRefType], ptrBuf, true ) ;
				return	err ;
			}
		}
		return	eslErrFailed ;
	}
	static ESLError StoreBufferInteger
		( void * ptrBuf, INT64 numStore, CSVariableType csvtType )
	{
		return	(*(m_pfnStoreIntFunc[csvtType]))( ptrBuf, numStore ) ;
	}
	static ESLError StoreBufferReal
		( void * ptrBuf, double numStore, CSVariableType csvtType )
	{
		return	(*(m_pfnStoreRealFunc[csvtType]))( ptrBuf, numStore ) ;
	}

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
	virtual void FlushBuffer
		( int iOffset, int nSize, void * ptrBuf, bool fModified ) ;
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

} ;


//////////////////////////////////////////////////////////////////////////////
// ポインタオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSPointer	: public	ECSPointerReference
{
public:
	// 構築関数
	ECSPointer
		( int iOffset = 0,
			CSVariableType csvtRefType = csvtObject, bool fReadOnly = false )
		: ECSPointerReference( iOffset, csvtObject, fReadOnly )
		{
			m_vtType = csvtPointer ;
		}
	// クラス情報
	DECLARE_CLASS_INFO( ECSPointer, ECSPointerReference )

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
	// 特殊演算子 : boolean 判定
	virtual ESLError OperateBoolean( int & nBoolean ) ;
	// 特殊演算子 : sizeof
	virtual ESLError OperateSizeOf( INT64 & nSize ) ;

} ;
