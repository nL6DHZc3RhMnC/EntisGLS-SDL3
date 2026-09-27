
//////////////////////////////////////////////////////////////////////////////
// バッファ
//////////////////////////////////////////////////////////////////////////////

class	ECSBuffer	: public ECSObject
{
public:
	// 構築関数
	ECSBuffer( void ) ;
	ECSBuffer( const ECSBuffer & buf ) ;
	// 消滅関数
	virtual ~ECSBuffer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSBuffer, ECSObject )

protected:
	BYTE *	m_pbytBuf ;
	int		m_nBufSize ;
	int		m_nBufBase ;
	int		m_nBufLimit ;

public:	// バッファ管理
	// バッファ生成
	virtual ESLError CreateBuffer( int nBytes, int nBase = 0 ) ;
	// バッファリサイズ
	virtual ESLError ResizeBuffer( int nBytes, int nBase = 0 ) ;
	// バッファリミット設定
	virtual ESLError ResizeBufferLimit( int nLimit ) ;
	// バッファ解放
	virtual void FreeBuffer( void ) ;
	// バッファ複製
	void CopyBufferFrom( const ECSBuffer & buf ) ;

public:	// バッファアクセス
	// バッファ取得
	BYTE * GetBuffer( void ) const
		{
			return	m_pbytBuf ;
		}
	// バッファ長取得
	unsigned int GetLength( void ) const
		{
			return	m_nBufSize ;
		}
	// バッファベースアドレス
	unsigned int GetBufferBase( void ) const
		{
			return	m_nBufBase ;
		}
	// 書き出しバッファ確保
	void * PutBuffer( int nSize ) ;
	// 書き出しバッファ確定
	void Flush( int nSize ) ;
	// バッファ変更
	void * ModifyBuffer( int nPos, int nSize ) ;
	// バッファ結合
	void MergeBuffer( const void * ptrBuf, int nSize ) ;

public:	// ECSObject インターフェース
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move
		( ECSContext & context, ECSObject * obj ) ;
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
	// 内部バッファインターフェース
	virtual void * GetBuffer( int iOffset, int nSize, bool fWritable ) ;
	virtual ECSSakura2Processor::LinearAddressCache *
			GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg ) ;

public:		// シリアル化のための関数（システムによって必要）
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
	typedef	ESLError (ECSBuffer::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[4] ;
	static const PFUNC_CALL	m_pfnCallFunc[3] ;
	// メンバ関数
	ESLError Call_CreateBuffer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FreeBuffer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ResizeBuffer
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;

// new Buffer
ECS_EXPORT ECSSakura2::Object *
	ecs_new_object_Buffer
		( ECSSakura2Processor::Context * context, int cls_id ) ;


//////////////////////////////////////////////////////////////////////////////
// （スクリプト）読み込み専用バッファ
//////////////////////////////////////////////////////////////////////////////

class	ECSReadOnlyBuffer	: public ECSBuffer
{
public:
	// 構築関数
	ECSReadOnlyBuffer( void ) {}
	ECSReadOnlyBuffer( const ECSBuffer & buf ) : ECSBuffer( buf ) {}
	// クラス情報
	DECLARE_CLASS_INFO( ECSReadOnlyBuffer, ECSBuffer )

public:
	// バッファ取得
	BYTE * GetBuffer( void ) const
		{
			return	m_pbytBuf ;
		}
	// 内部バッファインターフェース
	virtual void * GetBuffer( int iOffset, int nSize, bool fWritable ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// コード専用バッファ
//////////////////////////////////////////////////////////////////////////////

class	ECSCodeBuffer	: public ECSReadOnlyBuffer
{
public:
	// 構築関数
	ECSCodeBuffer( void ) ;
	ECSCodeBuffer( const ECSBuffer & buf ) ;
	// 消滅関数
	virtual ~ECSCodeBuffer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSCodeBuffer, ECSReadOnlyBuffer )

protected:
	BYTE *	m_pbytShadow ;		// シャドウバッファ（トリックバッファ用）

public:
	// バッファリサイズ
	virtual ESLError ResizeBuffer( int nBytes, int nBase = 0 ) ;
	// バッファリミット設定
	virtual ESLError ResizeBufferLimit( int nLimit ) ;
	// バッファ解放
	virtual void FreeBuffer( void ) ;
	// シャドウバッファ生成
	virtual void CreateShadowBuffer( void ) ;

public:
	// 内部バッファインターフェース
	virtual BYTE * GetSegmentShadowBuffer( int iShadow = 0 ) ;

} ;

