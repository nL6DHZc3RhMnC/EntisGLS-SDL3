
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_JIT_NATIVE_COMPILER_H__)
#define	__GLSCS_SAKURA2_JIT_NATIVE_COMPILER_H__

namespace	ECSSakura2JIT
{
	using	ECSSakura2Processor::OPERATION_DST_SRC_PROC ;
	using	ECSSakura2Processor::OPERATION_DST_SRC_IMM_PROC ;
	using	ECSSakura2Processor::OPERATION_DST_SRC_SRC2_PROC ;
	using	ECSSakura2Processor::OPERATION_SIMD128_DST_SRC_PROC ;
	using	ECSSakura2Processor::OPERATION_SIMD128_DST_SRC_IMM_PROC ;
	using	ECSSakura2Processor::Register ;
	using	ECSSakura2Processor::DataType ;
	using	ECSSakura2Processor::AddressingMode ;
	using	ECSSakura2Processor::LocalAddressingMode ;

	//////////////////////////////////////////////////////////////////////////
	// コードバッファ出力オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	CodeBuffer	: public ESLObject
	{
	protected:
		struct	Block
		{
			Block *	pNextBlock ;
			BYTE *	pbytBuf ;
			BYTE *	pbytBufAligned16 ;
			size_t	nBufAllocSize ;
			size_t	nBufSize ;
			size_t	nCodeUsed ;
			size_t	nDataUsed ;
		} ;
		Block *	m_pFirstBlock ;
		Block *	m_pLastBlock ;
		size_t	m_bytesBlockUnit ;
		size_t	m_maxInstructionSize ;		// ExtendBlock する目安残量

		// 実行・読み書き可能メモリの確保
		virtual BYTE * AllocateCodeBuffer( size_t& nSize ) ;
		// 実行・読み書き可能メモリの解放
		virtual void FreeCodeBuffer( BYTE * pbytBuf, size_t nSize ) ;
		// 実行メモリの内容確定
		virtual void FlushCodeBlock( BYTE * pbytBuf, size_t nSize ) ;
		// メモリブロック確保
		virtual Block * NewBlock( void ) ;
		// メモリブロックリスト解放
		virtual void FreeBlockList( Block * pBlock ) ;
		// ジャンプ命令を追加する
		virtual void WriteJump( Block * pBlock, const void * pTarget ) = 0 ;
		// メモリブロックを追加する
		virtual void ExtendBlock( void ) ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( CodeBuffer, ESLObject )
		// 構築関数
		CodeBuffer( void )
			: m_pFirstBlock(NULL), m_pLastBlock(NULL),
				m_bytesBlockUnit(0x10000), m_maxInstructionSize(0x40) {}
		// 消滅関数
		virtual ~CodeBuffer( void ) ;
		// 次の命令を出力アドレスを取得する
		virtual void * GetNext( void ) ;
		// 命令を出力する
		virtual void WriteInstruction( const BYTE * pbytSrc, size_t nBytes ) ;
		// データ領域を確保する
		virtual void * AllocateData( size_t nBytes, size_t nAlign ) ;
		// データを確定する
		virtual void CommitAllCodes( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 汎用演算関数
	//////////////////////////////////////////////////////////////////////////

	struct	GENERIC_IMPLEMENTS
	{
		// ムーブ・変換演算
		OPERATION_DST_SRC_PROC		move_reg_reg ;
		OPERATION_DST_SRC_SRC2_PROC	maskmove_reg_reg_reg ;
		OPERATION_DST_SRC_PROC		cvt_float2int ;
		OPERATION_DST_SRC_PROC		cvt_int2float ;
		// 即値シフト演算
		OPERATION_DST_SRC_IMM_PROC	srl_reg_reg_imm8 ;
		OPERATION_DST_SRC_IMM_PROC	sra_reg_reg_imm8 ;
		OPERATION_DST_SRC_IMM_PROC	sll_reg_reg_imm8 ;
		// 即値加算・乗算
		OPERATION_DST_SRC_IMM_PROC	add_reg_reg_imm32 ;
		OPERATION_DST_SRC_IMM_PROC	mul_reg_reg_imm32 ;
		// 単項演算
		OPERATION_DST_SRC_PROC		neg_int ;
		OPERATION_DST_SRC_PROC		not_int ;
		OPERATION_DST_SRC_PROC		neg_float ;
		// 整数演算
		OPERATION_DST_SRC_PROC		add_reg_reg ;
		OPERATION_DST_SRC_PROC		sub_reg_reg ;
		OPERATION_DST_SRC_PROC		mul_reg_reg ;
		OPERATION_DST_SRC_PROC		div_reg_reg ;
		OPERATION_DST_SRC_PROC		mod_reg_reg ;
		OPERATION_DST_SRC_PROC		and_reg_reg ;
		OPERATION_DST_SRC_PROC		or_reg_reg ;
		OPERATION_DST_SRC_PROC		xor_reg_reg ;
		OPERATION_DST_SRC_PROC		srl_reg_reg ;
		OPERATION_DST_SRC_PROC		sra_reg_reg ;
		OPERATION_DST_SRC_PROC		sll_reg_reg ;
		// 整数符号拡張
		OPERATION_DST_SRC_PROC		move_sx32_reg_reg ;
		OPERATION_DST_SRC_PROC		move_sx16_reg_reg ;
		OPERATION_DST_SRC_PROC		move_sx8_reg_reg ;
		// 実数演算
		OPERATION_DST_SRC_PROC		fadd_reg_reg ;
		OPERATION_DST_SRC_PROC		fsub_reg_reg ;
		OPERATION_DST_SRC_PROC		fmul_reg_reg ;
		OPERATION_DST_SRC_PROC		fdiv_reg_reg ;
		// 特殊演算精度演算
		OPERATION_DST_SRC_PROC		mul32_reg_reg ;
		OPERATION_DST_SRC_PROC		imul32_reg_reg ;
		OPERATION_DST_SRC_PROC		div32_reg_reg ;
		OPERATION_DST_SRC_PROC		idiv32_reg_reg ;
		OPERATION_DST_SRC_PROC		mod32_reg_reg ;
		OPERATION_DST_SRC_PROC		imod32_reg_reg ;
		// 整数比較
		OPERATION_DST_SRC_PROC		cmp_ne ;
		OPERATION_DST_SRC_PROC		cmp_eq ;
		OPERATION_DST_SRC_PROC		cmp_lt ;
		OPERATION_DST_SRC_PROC		cmp_le ;
		OPERATION_DST_SRC_PROC		cmp_gt ;
		OPERATION_DST_SRC_PROC		cmp_ge ;
		OPERATION_DST_SRC_PROC		cmp_c ;
		OPERATION_DST_SRC_PROC		cmp_cz ;
		// 実数比較
		OPERATION_DST_SRC_PROC		fcmp_ne ;
		OPERATION_DST_SRC_PROC		fcmp_eq ;
		OPERATION_DST_SRC_PROC		fcmp_lt ;
		OPERATION_DST_SRC_PROC		fcmp_le ;
		OPERATION_DST_SRC_PROC		fcmp_gt ;
		OPERATION_DST_SRC_PROC		fcmp_ge ;
	} ;

	extern	const GENERIC_IMPLEMENTS	giDefGenericImplements ;

	// ムーブ・変換演算
	void __fastcall gen_move_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_maskmove_reg_reg_reg
			( Register * dst, const Register * src, const Register * src2 ) ;
	void __fastcall gen_cvt_float2int
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cvt_int2float
			( Register * dst, const Register * src ) ;
	// 即値シフト演算
	void __fastcall gen_srl_reg_reg_imm8
			( Register * dst, const Register * src, int imm ) ;
	void __fastcall gen_sra_reg_reg_imm8
			( Register * dst, const Register * src, int imm ) ;
	void __fastcall gen_sll_reg_reg_imm8
			( Register * dst, const Register * src, int imm ) ;
	// 即値加算・乗算
	void __fastcall gen_add_reg_reg_imm32
			( Register * dst, const Register * src, int imm ) ;
	void __fastcall gen_mul_reg_reg_imm32
			( Register * dst, const Register * src, int imm ) ;
	// 単項演算
	void __fastcall gen_neg_int
			( Register * dst, const Register * src ) ;
	void __fastcall gen_not_int
			( Register * dst, const Register * src ) ;
	void __fastcall gen_neg_float
			( Register * dst, const Register * src ) ;
	// 整数演算
	void __fastcall gen_add_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_sub_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_mul_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_div_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_mod_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_and_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_or_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_xor_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_srl_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_sra_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_sll_reg_reg
			( Register * dst, const Register * src ) ;
	// 整数符号拡張
	void __fastcall gen_move_sx32_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_move_sx16_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_move_sx8_reg_reg
			( Register * dst, const Register * src ) ;
	// 実数演算
	void __fastcall gen_fadd_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fsub_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fmul_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fdiv_reg_reg
			( Register * dst, const Register * src ) ;
	// 特殊演算精度演算
	void __fastcall gen_mul32_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_imul32_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_div32_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_idiv32_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_mod32_reg_reg
			( Register * dst, const Register * src ) ;
	void __fastcall gen_imod32_reg_reg
			( Register * dst, const Register * src ) ;
	// 整数比較
	void __fastcall gen_cmp_ne
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cmp_eq
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cmp_lt
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cmp_le
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cmp_gt
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cmp_ge
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cmp_c
			( Register * dst, const Register * src ) ;
	void __fastcall gen_cmp_cz
			( Register * dst, const Register * src ) ;
	// 実数比較
	void __fastcall gen_fcmp_ne
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fcmp_eq
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fcmp_lt
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fcmp_le
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fcmp_gt
			( Register * dst, const Register * src ) ;
	void __fastcall gen_fcmp_ge
			( Register * dst, const Register * src ) ;


	//////////////////////////////////////////////////////////////////////////
	// レジスタ割り当て
	//////////////////////////////////////////////////////////////////////////

	// レジスタ割り当てエントリ
	struct	RegisterAssignation
	{
		bool	fAssigned ;		// 使用中フラグ
		bool	fModified ;		// 変更フラグ
		int		regSakura ;		// Sakura2 レジスタ番号
		int		regPhy ;		// 物理レジスタ番号
		int		numLocked ;		// 解放不可カウンタ
		int		numStale ;		// 古さカウンタ

		RegisterAssignation( void )
			: fAssigned(false), fModified(false),
				regSakura(-1), regPhy(-1),
				numLocked(0), numStale(0) {}
		const RegisterAssignation& operator = ( const RegisterAssignation& ra )
		{
			fAssigned = ra.fAssigned ;
			fModified = ra.fModified ;
			regSakura = ra.regSakura ;
			regPhy = ra.regPhy ;
			numLocked = ra.numLocked ;
			numStale = ra.numStale ;
			return	*this ;
		}
	} ;

	// レジスタ割り当て LRU
	template <class T, int N> class	RegisterAssignor
	{
	public:
		T	m_slot[N] ;
		enum
		{
			SlotCount = N,
		} ;
	public:
		// 構築関数
		RegisterAssignor( void )
		{
		}
		// スロット参照
		T& operator [] ( int i )
		{
			ESLAssert( (size_t) i < (size_t) N ) ;
			return	m_slot[i] ;
		}
		const T& operator [] ( int i ) const
		{
			ESLAssert( (size_t) i < (size_t) N ) ;
			return	m_slot[i] ;
		}
		bool IsSlotAssigned( int i )
		{
			ESLAssert( (size_t) i < (size_t) N ) ;
			return	m_slot[i].fAssigned ;
		}
		// 空きスロット検索
		int FindFreeSlot( int p = 1 ) const
		{
			for ( int i = 0; i < N; i += p )
			{
				if ( !m_slot[i].fAssigned )
				{
					return	i ;
				}
			}
			return	-1 ;
		}
		// 割り当て済みスロット検索
		int FindAssignedSlot( int reg, int p = 1 ) const
		{
			for ( int i = 0; i < N; i += p )
			{
				if ( m_slot[i].fAssigned
					&& (m_slot[i].regSakura == reg) )
				{
					return	i ;
				}
			}
			return	-1 ;
		}
		// 古い（次に使用するのに適した）スロット検索
		int FindMostOldSlot( int p = 1 ) const
		{
			int		maxStale = -1 ;
			int		iMaxSlot = -1 ;
			bool	fModified = true ;
			for ( int i = 0; i < N; i += p )
			{
				if ( !m_slot[i].fAssigned )
				{
					return	i ;
				}
				if ( m_slot[i].numLocked != 0 )
				{
					continue ;
				}
				if ( !m_slot[i].fModified )
				{
					if ( fModified || (maxStale < m_slot[i].numStale) )
					{
						maxStale = m_slot[i].numStale ;
						iMaxSlot = i ;
						fModified = false ;
					}
				}
				else if ( fModified && (maxStale < m_slot[i].numStale) )
				{
					maxStale = m_slot[i].numStale ;
					iMaxSlot = i ;
				}
			}
			return	iMaxSlot ;
		}
		// 古さカウンタ加算
		void AddStale( int num )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_slot[i].numStale += num ;
			}
		}
		// 古さカウンタリセット
		void RecentlyAccess( int iSlot )
		{
			AddStale( 1 ) ;
			m_slot[iSlot].numStale = 0 ;
		}
		// 変更フラグ設定
		void SetModified( int iSlot )
		{
			m_slot[iSlot].fModified = true ;
		}
		// スロットロック
		void LockSlot( int iSlot )
		{
			m_slot[iSlot].numLocked ++ ;
		}
		// スロットアンロック
		void UnlockSlot( int iSlot )
		{
			ESLAssert( m_slot[iSlot].numLocked > 0 ) ;
			if ( m_slot[iSlot].numLocked > 0 )
			{
				m_slot[iSlot].numLocked -- ;
			}
		}
		// 新規スロット初期化
		void NewSlot( int iSlot, int reg )
		{
			ESLAssert( (size_t) iSlot < (size_t) N ) ;
			m_slot[iSlot].fAssigned = true ;
			m_slot[iSlot].fModified = false ;
			m_slot[iSlot].regSakura = reg ;
			m_slot[iSlot].numLocked = 0 ;
			m_slot[iSlot].numStale = 0 ;
		}
		// スロット解放
		void FreeSlot( int iSlot )
		{
			ESLAssert( (size_t) iSlot < (size_t) N ) ;
			m_slot[iSlot].fAssigned = false ;
			m_slot[iSlot].numLocked = 0 ;
		}
		void FreeAllSlot( void )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_slot[i].fAssigned = false ;
				m_slot[i].fModified = false ;
				m_slot[i].numLocked = 0 ;
			}
		}
		void FreeAllUnlockedSlot( void )
		{
			for ( int i = 0; i < N; i ++ )
			{
				if ( m_slot[i].numLocked == 0 )
				{
					m_slot[i].fAssigned = false ;
					m_slot[i].fModified = false ;
				}
			}
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Sakura2 processor ネイティブコード化抽象アセンブラ
	//////////////////////////////////////////////////////////////////////////

	class	Sakura2Assembler
	{
	protected:
		// アドレス変換時に境界チェックを行うか？
		bool	m_flagNoBoundary ;

		// 出力バッファ
		CodeBuffer *	m_buf ;
		CodeBuffer *	m_bufMain ;
		CodeBuffer *	m_bufSub ;

		// 命令処理関数（汎用）
		const GENERIC_IMPLEMENTS *					m_gi ;			// 汎用演算関数
		const OPERATION_DST_SRC_PROC *				m_fx ;			// 64bit 浮動小数点演算関数
		const OPERATION_DST_SRC_PROC *				m_simd64op2 ;	// 64bit SIMD 処理関数
		const OPERATION_DST_SRC_IMM_PROC *			m_simd64op3 ;
		const OPERATION_SIMD128_DST_SRC_PROC *		m_simd128op2 ;
		const OPERATION_SIMD128_DST_SRC_IMM_PROC *	m_simd128op3 ;

		// TLB 割り当て
		BYTE	m_tlbFetch[0x100] ;
		int		m_regTLBFetched[2] ;

	public:
		// 構築関数
		Sakura2Assembler( void )
		{
			m_flagNoBoundary = false ;
			m_buf = NULL ;
			m_gi = &ECSSakura2JIT::giDefGenericImplements ;
			m_fx = ECSSakura2Processor::pfnFloatOperationDstSrc ;
			m_simd64op2 = ECSSakura2Processor::pfnSIMD64_OperationDstSrc ;
			m_simd64op3 = ECSSakura2Processor::pfnSIMD64_OperationDstSrcImm8 ;
			m_simd128op2 = ECSSakura2Processor::pfnSIMD128_OperationDstSrc ;
			m_simd128op3 = ECSSakura2Processor::pfnSIMD128_OperationDstSrcImm8 ;
			//
			memset( m_tlbFetch, 0xFF, 0x100 ) ;
			//
			m_regTLBFetched[0] = -1 ;
			m_regTLBFetched[1] = -1 ;
		}
		// 消滅関数
		virtual ~Sakura2Assembler( void ) ;
		// アドレス変換時に境界チェックコードを出力するか
		void SetNoBoundaryWithAddressTranslation( bool fNoBoundary )
		{
			m_flagNoBoundary = fNoBoundary ;
		}
		// 出力バッファ設定
		void AttachCodeBuffer( CodeBuffer * buf, CodeBuffer * bufSub = NULL )
		{
			m_buf = buf ;
			m_bufMain = buf ;
			m_bufSub = bufSub ;
		}
		// デフォルト処理関数設定
		void AttachDefaultFunction
			( const GENERIC_IMPLEMENTS * gi,
				const OPERATION_DST_SRC_PROC * fx,
				const OPERATION_DST_SRC_PROC * simd64op2,
				const OPERATION_DST_SRC_IMM_PROC * simd64op3,
				const OPERATION_SIMD128_DST_SRC_PROC * simd128op2,
				const OPERATION_SIMD128_DST_SRC_IMM_PROC * simd128op3 )
		{
			m_gi = gi ;
			m_fx = fx ;
			m_simd64op2 = simd64op2 ;
			m_simd64op3 = simd64op3 ;
			m_simd128op2 = simd128op2 ;
			m_simd128op3 = simd128op3 ;
		}

	public:
		// 次に出力するアドレス
		void * GetNextAddress( void ) const
			{
				return	m_buf->GetNext() ;
			}
		// 全てのコード出力を完了し確定する
		void CommitAllCodes( void ) ;

	public:
		// デバッグ用（関数コンパイル開始時の処理）
		virtual void OnDebugBeforeFunction( DWORD dwFuncAddr, DWORD dwSize ) ;
		// デバッグ用（関数コンパイル終了時の処理）
		virtual void OnDebugAfterFunction( DWORD dwFuncAddr, DWORD dwSize ) ;
		// デバッグ用（命令コンパイル前の処理）
		virtual void OnDebugBeforeInstruction( BYTE bytInst, DWORD ip ) ;
		// デバッグ用（命令コンパイル後のテスト）
		virtual void OnDebugAfterInstruction( BYTE bytInst, DWORD ip ) ;

	public:
		// 汎用演算命令出力（複雑な演算命令は外部関数を呼び出して解決する）
		// 関数呼び出しコード出力 : reg 形式
		virtual void WriteToCallInstructionReg
			( OPERATION_DST_SRC_PROC pfnGen, int reg ) = 0 ;
		// 関数呼び出しコード出力 : reg, reg 形式
		virtual void WriteToCallInstructionRegReg
			( OPERATION_DST_SRC_PROC pfnGen, int dstreg, int srcreg ) = 0 ;
		virtual void WriteToCallSIMD128InstructionRegReg
			( OPERATION_SIMD128_DST_SRC_PROC pfnGen, int dstreg, int srcreg ) = 0 ;
		// 関数呼び出しコード出力 : reg, reg, reg 形式
		virtual void WriteToCallInstructionRegRegReg
			( OPERATION_DST_SRC_SRC2_PROC pfnGen,
						int dstreg, int srcreg, int srcreg2 ) = 0 ;
		// 関数呼び出しコード出力 : reg, reg, imm 形式
		virtual void WriteToCallInstructionRegRegImm
			( OPERATION_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm ) = 0 ;
		virtual void WriteToCallSIMD128InstructionRegRegImm
			( OPERATION_SIMD128_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm ) = 0 ;
		// 関数呼び出し後のレジスタ処理
		virtual void ResetRegisterAfterCall( void ) = 0 ;

	public:
		// プロローグコード出力
		virtual void WritePrologue( void ) = 0 ;
		virtual void WriteSubPrologue( void ) = 0 ;
		// エピローグコード出力
		virtual void WriteEpilogue( int ipExit = -1 ) = 0 ;
		// レジスタへの変更をコンテキストに書き出し
		virtual void FlushAllRegisters( void ) = 0 ;
		// レジスタへの変更をコンテキストに書き出し
		//（レジスタコンテキストを変更しない）
		virtual void WriteBackAllRegisters( void ) = 0 ;
		// レジスタへの変更をコンテキストに書き出し
		virtual void FlushRegister( int reg ) = 0 ;
		// レジスタの値を物理レジスタに復元する
		virtual void ReloadRegisters( void ) = 0 ;
		// レジスタコンテキストをリセット
		virtual void ResetAllRegisters( void ) = 0 ;
		// レジスタコンテキストをリセット
		virtual void ResetRegister( int reg ) = 0 ;
		// レジスタコンテキストをリセット（ポインタレジスタ以外）
		virtual void ResetDataRegisters( void ) = 0 ;

	public:
		struct	RealizePointerBoundary
		{
			SDWORD *	pdwFirst ;
			SDWORD *	pdwLimit ;
			int			offsetFirst ;
			int			offsetEnd ;

			RealizePointerBoundary( void )
				: pdwFirst(NULL), pdwLimit(NULL),
					offsetFirst(0), offsetEnd(0) {}
		} ;
		// BP ポインタ用物理レジスタ識別子取得
		virtual int GetFramePointerPhysicalRegister( void ) const = 0 ;
		// レジスタ値変更通知（BP 以外ポインタ用）
		virtual void ModifiedRegister( int reg ) = 0 ;
		// アドレス変換して物理レジスタにアドレスを
		// ロードするコード出力（フレームポインタ用）
		virtual void * WriteRealizePointerRegister
			( int regPhy, int regSakura,
				RealizePointerBoundary& rpb, const void * ptrEpilogue ) = 0 ;
		// 物理レジスタにロードしたポインタの境界チェックコードを完成させる
		virtual void CommitRealizePointerRegister
			( RealizePointerBoundary& rpb,
						int offsetFirst, int offsetEnd ) = 0 ;
		// ポインタアクセス用レジスタ割り当て処理コード出力
		//（物理ポインタ用レジスタ番号を返却）
		virtual int WriteAssignPointerRegister
			( int regPtr, int regIndex, int scale,
				int offsetFirst, int offsetEnd, void *& ptrEscJumpFrom ) = 0 ;

	public:
		// 例外マスクに追加するコードを出力
		virtual void WriteToAtomicOrExceptionMask( DWORD dwException ) = 0 ;
		// スタック拡張例外判定
		virtual void * WriteToStackException
			( int& regPhyNewLowSP,
				int nOffsetSP, const void * ptrEpilogue ) = 0 ;
		// ゼロ除算例外用ゼロ比較脱出コード出力
		virtual void * WriteToZeroDivisionException
			( int regDiv, const void * ptrEpilogue ) = 0 ;
		virtual void * WriteToZeroDivisionException32
			( int regDiv, const void * ptrEpilogue ) = 0 ;
		// メモリ読み込み命令出力
		virtual void WriteToLoadPhysicalMemory
			( int regDst, int regPhyPtr,
					int offset, DataType type, bool fPair ) = 0 ;
		// メモリ書き出し命令出力
		virtual void WriteToStorePhysicalMemory
			( int regSrc, int regPhyPtr,
					int offset, DataType type, bool fPair ) = 0 ;

	public:
		// 無条件ジャンプコード出力
		virtual void * WriteToJump( const void * ptrTarget ) = 0 ;
		// 条件ジャンプコード出力
		virtual void * WriteToConditionalJump
				( int reg, bool fLogic, const void * ptrTarget ) = 0 ;
		// 例外判定離脱コード出力
		virtual void * WriteToEscapeByException( const void * ptrEpilogue ) = 0 ;
		// ジャンプコード完成（２パス用）
		virtual void CommitJumpTarget
				( void * ptrJumpFrom, const void * ptrFixedTarget ) = 0 ;

	public:
		// ローカル変数用フレームポインタ割り当て処理コード出力
		virtual void * WriteAssignLocalBasePointer
			( int offsetFirst, int offsetEnd, const void * ptrEpilogue ) ;
		// メモリ読み込み処理コード出力
		virtual void * WriteToLoadMemory
			( int regDst, int regPtr,
				int regIndex, int scale,
				int offset, DataType type, bool fPair ) ;
		// メモリ読み込み処理コード出力
		virtual void * WriteToStoreMemory
			( int regSrc, int regPtr,
				int regIndex, int scale,
				int offset, DataType type, bool fPair ) ;

	public:
		// 各命令エンコーディングと低水準処理
		// 関数の返り値は例外発生時の脱出用ジャンプ命令アドレスで
		// CommitJumpTarget 関数の ptrJumpFrom に使用できる値

		// メモリアクセス命令
		virtual void * write_load_memory
			( const void * pEscCode, AddressingMode mode, DataType type,
				int regDst, int regBase, int regIndex, int scale, int offset, bool fPair ) ;
		virtual void * write_store_memory
			( const void * pEscCode, AddressingMode mode, DataType type,
				int regSrc, int regBase, int regIndex, int scale, int offset, bool fPair ) ;
		virtual void * write_load_local
			( const void * pEscCode, LocalAddressingMode mode, DataType type,
				int regDst, int regIndex, int scale, int offset, bool fPair ) ;
		virtual void * write_store_local
			( const void * pEscCode, LocalAddressingMode mode, DataType type,
				int regSrc, int regIndex, int scale, int offset, bool fPair ) ;
		// データ移動命令
		virtual void write_move_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_maskmove_reg_reg_reg( int regDst, int regSrc, int regSrc2, bool fPair ) ;
		// 整数・実数変換命令
		virtual void write_cvt_float2int( int regDst, int regSrc ) ;
		virtual void write_cvt_int2float( int regDst, int regSrc ) ;
		// シフト命令
		virtual void write_srl_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair ) ;
		virtual void write_sra_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair ) ;
		virtual void write_sll_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair ) ;
		// 32ビット即値命令
		virtual void write_add_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair ) ;
		virtual void write_mul_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair ) ;
		// スタックレジスタ加算命令
		virtual void * write_add_sp_imm32( int ip, int imm32 ) ;
		// 64ビット即値命令
		virtual void write_move_reg_imm64( int regDst, INT64 imm64 ) = 0 ;
		// 1 OP 演算命令
		virtual void write_neg_int( int regDst ) ;
		virtual void write_not_int( int regDst ) ;
		virtual void write_neg_float( int regDst ) ;
		// 2 OP 整数演算命令
		virtual void write_add_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_sub_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_mul_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_div_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
		virtual void write_mod_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
		virtual void write_and_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_or_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_xor_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_srl_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_sra_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_sll_reg_reg( int regDst, int regSrc, bool fPair ) ;
		// 整数符号拡張命令
		virtual void write_move_sx32_reg_reg( int regDst, int regSrc ) ;
		virtual void write_move_sx16_reg_reg( int regDst, int regSrc ) ;
		virtual void write_move_sx8_reg_reg( int regDst, int regSrc ) ;
		// 2 OP 実数演算命令
		virtual void write_fadd_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fsub_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fmul_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fdiv_reg_reg( int regDst, int regSrc, bool fPair ) ;
		// 特殊精度整数演算命令
		virtual void write_mul32_reg_reg( int regDst, int regSrc ) ;
		virtual void write_imul32_reg_reg( int regDst, int regSrc ) ;
		virtual void write_div32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
		virtual void write_idiv32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
		virtual void write_mod32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
		virtual void write_imod32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
		// 整数比較命令
		virtual void write_cmp_ne( int regDst, int regSrc, bool fPair ) ;
		virtual void write_cmp_eq( int regDst, int regSrc, bool fPair ) ;
		virtual void write_cmp_lt( int regDst, int regSrc, bool fPair ) ;
		virtual void write_cmp_le( int regDst, int regSrc, bool fPair ) ;
		virtual void write_cmp_gt( int regDst, int regSrc, bool fPair ) ;
		virtual void write_cmp_ge( int regDst, int regSrc, bool fPair ) ;
		virtual void write_cmp_c( int regDst, int regSrc, bool fPair ) ;
		virtual void write_cmp_cz( int regDst, int regSrc, bool fPair ) ;
		// 実数比較命令
		virtual void write_fcmp_ne( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fcmp_eq( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fcmp_lt( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fcmp_le( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fcmp_gt( int regDst, int regSrc, bool fPair ) ;
		virtual void write_fcmp_ge( int regDst, int regSrc, bool fPair ) ;
		// 相対無条件ジャンプ命令
		virtual void * write_jump_offset32( void ) ;
		// 間接無条件ジャンプ命令（write_push_ip と組み合わせればコール）
		//（exceptionFarJump 例外の判定と設定を含む）
		virtual void write_jump_reg( int reg ) = 0 ;
		// 相対条件ジャンプ命令 0xD2～
		virtual void * write_cnjump_reg_offset32( int reg ) ;
		virtual void * write_cjump_reg_offset32( int reg ) ;
		// システムコール命令
		virtual void write_syscall_imm( int imm32 ) = 0 ;
		virtual void write_syscall_reg( int reg ) = 0 ;
		// リターン命令（exceptionFarJump 例外の判定と設定を含む）
		virtual void write_return( void ) = 0 ;
		// スタック処理
		virtual void * write_push_ip( int imm32 ) = 0 ;
		virtual void * write_push_reg( int regFirst, int nCount ) = 0 ;
		virtual void * write_pop_reg( int regFirst, int nCount ) = 0 ;
		// メモリヒント
		virtual void * write_prefetch_tlb( int tlb, int reg ) ;
		virtual void write_unfetch_tlb( int tlb, int reg ) ;
		// 浮動小数点演算 EXTENSION
		virtual void write_float_extension( int code, int regDst, int regSrc ) ;
		// 64bit SIMD
		virtual void write_simd64_extension( int code, int regDst, int regSrc, bool fPair ) ;
		virtual void write_simd64_imm_extension( int code, int regDst, int regSrc, int imm8, bool fPair ) ;
		// 128bit SIMD
		virtual void write_simd128_extension( int code, int regDst, int regSrc ) ;
		virtual void write_simd128_imm_extension( int code, int regDst, int regSrc, int imm8 ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Sakura2 processor ネイティブコード化コンパイラ
	//////////////////////////////////////////////////////////////////////////

	struct	JumpAddress
	{
		void *					ptrNative ;		// ネイティブコードアドレス
		SSystem::SArray<void*>	lstJumpFrom ;	// ジャンプ元ネイティブアドレス

		JumpAddress( void ) : ptrNative( NULL ) {}
	} ;

	class	NativeCompiler
	{
	public:
		struct	MemoryOffsetRange
		{
			bool	flagUsed ;
			int		minOffset ;
			int		maxOffset ;

			MemoryOffsetRange( void ) : flagUsed( false ) {}
			void AddRange( int first, int end ) ;
		} ;

	protected:
		Sakura2Assembler *	m_pAssemberCodes ;		// 関数コード出力
		Sakura2Assembler *	m_pAssemberGates ;

		BYTE *				m_pbytCode ;			// 関数コード
		BYTE *				m_pbytTrickBuf ;		// Trick バッファ
		DWORD				m_dwFuncAddr ;			// 開始アドレス
		DWORD				m_dwFuncSize ;			// コードサイズ

		SSystem::SULongSortObjectArray<JumpAddress>
							m_lstLabelAddr ;		// ラベル／進入アドレス、参照元アドレス
		MemoryOffsetRange	m_mbRangeBP ;			// [bp+offset]
		MemoryOffsetRange	m_mbRangeTP ;			// [tp+offset]
		bool				m_fObjectComplex ;		// コードに object 命令が含まれている

		bool				m_fInitBP ;
		bool				m_fInitTP ;

	public:
		// コード出力オブジェクト設定
		virtual void AttachCodeAssembler
			( Sakura2Assembler * pCodes, Sakura2Assembler * pGates ) ;
		// 詞葉 naked 関数設定
		virtual void AttachFunction
			( BYTE * pbytCode, BYTE * pbytTrickBuf,
					DWORD dwFuncAddr, DWORD dwFuncSize ) ;
		// 関数全体の基本情報を収集する（第１パス）
		virtual bool PreprocessFunction( void ) ;
		// ネイティブコードを出力する（第２パス）
		virtual void CompileFunction( void ) ;

	protected:
		// ペア命令を判定する
		virtual bool IsPairInstruction
			( const ECSSakura2Processor::InstructionInfo& inf, const BYTE * pbytCode ) ;

	} ;

}

#endif

