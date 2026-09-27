
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 名前付きスタック
//////////////////////////////////////////////////////////////////////////////

class	ECSStack	: public	ECSArray
{
public:
	class	EStackBlock	: public	ECSStrTagArray
	{
	public:
		DWORD			m_dwFlags ;		// フラグ
		ECSWideString *	m_pwstrName ;	// スタック名前空間名
		unsigned int	m_iBound ;		// スタック境界
		unsigned int	m_nVarCount ;	// ローカル変数の総数
		DWORD			m_dwCatchAddr ;	// CATCH ブロックのアドレス
	public:
		EStackBlock( void )
			: m_dwFlags(0), m_pwstrName(NULL), m_nVarCount(0) { }
	} ;
	enum	StackFlag
	{
		sfCallBlock		= 0x0001,			// 関数呼び出し元ブロック
		sfTryBlock		= 0x0002			// TRY ブロック
	} ;
	ECSObjArray<EStackBlock>	m_block ;	// スタックの名前空間
	EStackBlock *				m_pCurrent ;	// 現在の関数のブロック
	int							m_nCurrent ;

public:
	// 構築関数
	ECSStack( void ) ;
	// 消滅関数
	virtual ~ECSStack( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSStack, ECSArray )

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
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
	// 特殊演算子 : sizeof
	virtual ESLError OperateSizeOf( INT64 & nSize ) ;

public:
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// this オブジェクトを取得する
	ECSObject * GetCurrentThisObject( void )
		{
			if ( m_pCurrent != NULL )
			{
				if ( EWideString::Compare
					( m_pCurrent->GetAt(0), L"this" ) == 0 )
				{
					return	m_varArray.GetAt( m_pCurrent->m_iBound ) ;
				}
			}
			return	NULL ;
		}
	// 現在の関数フレーム情報を更新する
	void UpdateCurrentFrame( void ) ;
	// 名前空間を作成する
	ESLError CreateNameBlock( ECSWideString * pstrName /* 静的文字列 */ ) ;
	// 名前空間を削除する
	ESLError ReleaseNameBlock( ECSWideString *& pwstrName, ECSContext & context ) ;
	// 名前空間の一時領域をクリアする
	ESLError CleanupNameBlock( ECSContext & context ) ;
	// 変数を追加する
	ESLError CreateNewVariable
		( const wchar_t * pwszName /* 静的文字列バッファ */, ECSObject * pObj ) ;
	// スタックにオブジェクトをプッシュする
	ESLError PushObject( ECSObject * pObj )
		{
			m_varArray.Push( pObj ) ;
			return	eslErrSuccess ;
		}
	// スタックからオブジェクトをポップする
	ECSObject * PopObject( void )
		{
			EStackBlock *	pBlock = m_block.GetLastAt( ) ;
			if ( (pBlock == NULL)
				|| (pBlock->m_iBound < m_varArray.GetSize()) )
			{
				return	m_varArray.Pop( ) ;
			}
			return	NULL ;
		}
	// スタックをスワップする
	ESLError SwapLast( int nIndex1, int nIndex2 ) ;
	// 全ての名前空間とスタックを削除する
	void RemoveAll( void ) ;

} ;
