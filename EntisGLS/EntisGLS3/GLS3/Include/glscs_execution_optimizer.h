
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ・最適化
//////////////////////////////////////////////////////////////////////////////

class	ECSExecutionOptimizer	: public ESLObject
{
public:
	// 構築関数
	ECSExecutionOptimizer( ECSExecutionImageCompiler * pcsxi ) ;
	// 消滅関数
	virtual ~ECSExecutionOptimizer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSExecutionOptimizer, ESLObject )

protected:
	ECSExecutionImageCompiler *	m_pcsxi ;
	DWORD						m_addrStart ;
	DWORD						m_addrEnd ;
	int							m_regTempFirst ;
	int							m_regTempEnd ;

	// ラベル (tag = ジャンプ先アドレス, list = 参照元アドレス)
	ETagSortArray< DWORD, ENumArray<DWORD> >	m_tsaLabel ;

	// 命令情報リスト
	EObjArray<ECSExecutionReverseAssembler::InstructionInfo>
												m_lstInstruction ;

public:
	// 最適化開始
	void BeginOptimize
		( DWORD dwStart, DWORD dwEnd,
				int regTempFirst, int regTempEnd ) ;
	// 最適化処理
	int PerformOptimize( void ) ;
	// 最適化完了
	void FinishOptimize( void ) ;

protected:
	// 命令リスト生成
	void InitializeInstructionList( void ) ;
	// ジャンプアドレス確定
	void CommitJumpAddress( void ) ;

public:
	// コード省略判定コード
	enum	OptimizeType
	{
		optimizeNothing,		// この命令は省略できない
		optimizeOmission,		// この命令を省略できる
		optimizeDstRegister,	// 出力レジスタを置き換えて
								// 先方にある move 命令を省略できる
		optimizeSrcRegister,	// 出力レジスタを参照する命令の入力レジスタを
								// リダイレクトする事によってこの命令を省略できる
		optimizeAddrRegister,	// [zp+rx*1+offset] 形式を [rx+offset] 形式に置き換えられる
	} ;
protected:
	// コード省略情報
	struct	OptimizeInfo
	{
		OptimizeType	type ;
		int				iTarget ;		// 対象となる命令指標
		int				iStart ;		// 処理対象の開始命令指標
		int				iTerminate ;	// 終端命令指標（未満）
		int				regDstReplace ;	// 置き換えレジスタ番号
		int				regSrcReplace ;
		int				iOffset ;

		OptimizeInfo( void )
			: type( optimizeNothing ),
				iTarget( -1 ), iStart( 0 ), iTerminate( -1 ),
				regDstReplace( -1 ), regSrcReplace( -1 ), iOffset( 0 ) {}
	} ;
	// コード省略判定
	void TestOptimize( OptimizeInfo & optinf, int iTarget ) ;
	void TestOptimizeMemAddress( OptimizeInfo & optinf, int iTarget ) ;
	// optimizeDstRegister 最適化実行
	bool OptimizeDstRegister( const OptimizeInfo & optinf ) ;
	// optimizeSrcRegister 最適化実行
	bool OptimizeSrcRegister( const OptimizeInfo & optinf ) ;
	// optimizeAddrRegister 最適化実行
	bool OptimizeAddrRegister( const OptimizeInfo & optinf ) ;
	// ソースレジスタ判定
	bool IsSourceRegister
		( const ECSExecutionReverseAssembler::InstructionInfo * pinf, int reg )
	{
		return	(pinf->regSrc1 == reg)
				|| (pinf->regSrc2 == reg)
				|| (pinf->regSrc3 == reg) ;
	}

protected:
	// コードの命令単位での削除
	void RemoveInstruction( int iInst ) ;
	// コードのバイト単位での削除
	void RemoveCodeImage( DWORD dwAddress, int nRange ) ;

} ;
