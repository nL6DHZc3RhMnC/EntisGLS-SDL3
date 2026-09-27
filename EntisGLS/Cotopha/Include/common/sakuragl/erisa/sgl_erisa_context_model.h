

#if	!defined(__SAKURA_ERISA_CONTEXT_MODEL_H__)
#define	__SAKURA_ERISA_CONTEXT_MODEL_H__

namespace	ERISA
{

/*****************************************************************************
                         ハフマン・ツリー・構造体
 *****************************************************************************/

enum	ERINAHuffmanCode
{
	ERINA_CODE_FLAG			= 0x80000000U,
	ERINA_HUFFMAN_ESCAPE	= 0x7FFFFFFF,
	ERINA_HUFFMAN_NULL		= 0x8000U,
	ERINA_HUFFMAN_MAX		= 0x4000,
	ERINA_HUFFMAN_ROOT		= 0x200,
} ;

struct	ERINA_HUFFMAN_NODE
{
	WORD	m_weight ;
	WORD	m_parent ;
	DWORD	m_child_code ;
} ;

struct	ERINA_HUFFMAN_TREE
{
	ERINA_HUFFMAN_NODE	m_hnTree[0x201] ;
	int					m_iSymLookup[0x100] ;
	int					m_iEscape ;
	int					m_iTreePointer ;

	// ツリーの初期化
	void Initialize( void ) ;
	// 発生頻度をインクリメント
	void IncreaseOccuedCount( int iEntry ) ;
	// 親の重みを再計算する
	void RecountOccuredCount( int iParent ) ;
	// ツリーの正規化
	void Normalize( int iEntry ) ;
	// 新しいフリーエントリを作成して追加
	void AddNewEntry( int nNewCode ) ;
	// 各出現数を2分の1にして木を再構成
	void HalfAndRebuild( void ) ;

} ;


/*****************************************************************************
                             算術符号統計モデル
 *****************************************************************************/

enum	ERISASymbolCode
{
	ERISA_TOTAL_LIMIT	= 0x2000,		// 母数の限界値
	ERISA_SYMBOL_SORTS	= 0x101,		// シンボルの種類
	ERISA_SUB_SORT_MAX	= 0x80,
	ERISA_PROB_SLOT_MAX	= 0x800,		// 統計モデルの最大スロット数
	ERISA_ESC_CODE		= (-1),			// エスケープ記号
} ;

struct	ERISA_CODE_SYMBOL
{
	WORD	wOccured ;					// シンボルの出現回数
	SWORD	wSymbol ;					// シンボル（下位8ビットのみ）
} ;

struct	ERISA_PROB_MODEL
{
	DWORD				dwTotalCount ;			// 母数 < 2000H
	DWORD				dwSymbolSorts ;			// シンボルの種類数
	DWORD				dwReserved[2] ;
	ERISA_CODE_SYMBOL	acsSymTable[ERISA_SYMBOL_SORTS] ;
												// 統計モデル
	DWORD				dwReserved2[3] ;
	ERISA_CODE_SYMBOL	acsSubModel[ERISA_SUB_SORT_MAX] ;
												// サブ統計モデル（指標）

	// 統計情報の初期化
	void Initialize( void ) ;
	// 所要ビット数を計算する
	int AccumulateProb( SWORD wSymbol ) ;
	// シンボルの生起数を 1/2 にする
	void HalfOccuredCount( void ) ;
	// 指定のシンボルの生起数をインクリメントする
	int IncreaseSymbol( int index ) ;
	// 指定のシンボルの指標を取得する
	int FindSymbol( SWORD wSymbol ) ;
	// 指定のシンボルを追加する
	int AddSymbol( SWORD wSymbol ) ;
} ;

struct	ERISA_PROB_BASE
{
	ERISA_PROB_MODEL *	ptrProbWork ;			// 統計データ用ワーク
	DWORD				dwWorkUsed ;			// 使用されているスロット数
	DWORD				dwReserved[2] ;			// 16 バイトアライン
	ERISA_PROB_MODEL	epmBaseModel ;			// ベース統計モデル
	ERISA_PROB_MODEL *	ptrProbIndex[ERISA_PROB_SLOT_MAX] ;

	static const int	m_nShiftCount[4] ;
	static const int	m_nNewProbLimit[4] ;
} ;


/*****************************************************************************
                        スライド辞書参照テーブル
 *****************************************************************************/

enum	NemesisDefines
{
	NEMESIS_BUF_SIZE	= 0x10000,
	NEMESIS_BUF_MASK	= 0xFFFF,
	NEMESIS_INDEX_LIMIT	= 0x100,
	NEMESIS_INDEX_MASK	= 0xFF,
} ;

struct	ERISAN_PHRASE_LOOKUP
{
	DWORD	first ;
	DWORD	index[NEMESIS_INDEX_LIMIT] ;
} ;


}

#endif
