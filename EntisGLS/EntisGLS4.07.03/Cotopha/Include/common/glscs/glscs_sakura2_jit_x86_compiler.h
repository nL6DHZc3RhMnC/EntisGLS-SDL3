
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_JIT_X86_COMPILER_H__)
#define	__GLSCS_SAKURA2_JIT_X86_COMPILER_H__

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
	// x86 コードバッファ出力オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	X86CodeBuffer	: public CodeBuffer
	{
	public:
		// ジャンプ命令を追加する
		virtual void WriteJump( Block * pBlock, const void * pTarget ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// x86 ネイティブコード化アセンブラ
	//////////////////////////////////////////////////////////////////////////
	// ネイティブコード化関数プロトタイプ;
	//	void (__fastcall)( context* ) ;
	// 固定割り当てレジスタ;
	//	EBX = context*
	//	EBP = stack
	// ポインタ用一時割り当て;
	//	ECX, ESI, EDI
	//////////////////////////////////////////////////////////////////////////

	class	X86GenericAssembler	: public Sakura2Assembler
	{
	public:
		// x86 汎用レジスタ
		enum	X86Register
		{
			x86_Nothing	= -1,
			x86_EAX, x86_ECX, x86_EDX, x86_EBX,
			x86_ESP, x86_EBP, x86_ESI, x86_EDI,
			x86_AL = 0, x86_CL, x86_DL, x86_BL,
			x86_AH, x86_CH, x86_DH, x86_BH,
		} ;
		// 関数呼び出し規約
		enum	FastcallType
		{
			fastcallMSstyle,
			fastcallGCCstyle,
			fastcallCstyle,
		} ;
		struct	CALLING_ABI
		{
			bool		fRetWithClean ;	// 関数復帰時に引数スタックもクリア
			X86Register	regArg[4] ;		// レジスタ渡し可能引数
			int			maxRegArg ;		// レジスタ渡し最大数
		} ;
		CALLING_ABI	m_abiFastcall ;

		// ポインタ・レジスタ割り当て
		struct	PointerRegister
					: public RegisterAssignation,
						public Sakura2Assembler::RealizePointerBoundary
		{
			bool	fTLBFetched ;
			int		regPhyIndex ;

			PointerRegister( void )
				: fTLBFetched(false), regPhyIndex(x86_Nothing) {}
		} ;
		enum	PointerRegisterIndex
		{
			regPtrPhyESI = 0,
			regPtrPhyEDI,
			regPtrPhyECX,
			regPtrPhyCount,
			regPtrPhyBPIndex	= regPtrPhyCount,
		} ;
		RegisterAssignor<PointerRegister,regPtrPhyCount>	m_lruPointer ;

	public:
		// 構築関数
		X86GenericAssembler( void )
		{
			SetCallingABI( fastcallMSstyle ) ;
			//
			m_lruPointer[regPtrPhyESI].regPhy = x86_ESI ;
			m_lruPointer[regPtrPhyEDI].regPhy = x86_EDI ;
			m_lruPointer[regPtrPhyECX].regPhy = x86_ECX ;
		}
		// 呼び出し規約設定
		void SetCallingABI( const CALLING_ABI& abiCall )
		{
			m_abiFastcall = abiCall ;
		}
		void SetCallingABI( FastcallType abiCall ) ;

	public:
		// 命令コード
		enum	X86SBOperationCode
		{
			// ADD reg, r/m
			x86op1_ADD_REG_RM	= 0x03,
			// ADC reg, r/m
			x86op1_ADC_REG_RM	= 0x13,
			// OR reg, r/m
			x86op1_OR_REG_RM	= 0x0B,
			// AND reg, r/m
			x86op1_AND_REG_RM	= 0x23,
			// SUB reg, r/m
			x86op1_SUB_REG_RM	= 0x2B,
			// SBB reg, r/m
			x86op1_SBB_REG_RM	= 0x1B,
			// XOR reg, r/m
			x86op1_XOR_REG_RM	= 0x33,
			// CMP reg, r/m
			x86op1_CMP_REG_RM	= 0x3B,
			// MUL/IMUL/DIV/IDIV/NOT r/m
			x86op1_MUL_RM		= 0xF7,
			x86op1_IMUL_RM		= 0xF7,
			x86op1_DIV_RM		= 0xF7,
			x86op1_IDIV_RM		= 0xF7,
			x86op1_NOT_RM		= 0xF7,
			// IMUL reg, r/m, imm32
			x86op1_IMUL_REG_RM_IMM32	= 0x69,
			// CMP r/m, imm8
			x86op1_OP_RM_IMM8	= 0x80,
			// ADD/ADC/OR/CMP r/m, imm32
			x86op1_OP_RM_IMM32	= 0x81,
			// TEST r/m, reg
			x86op1_TEST_RM_REG	= 0x85,
			// MOV
			x86op1_MOV_STORE_8	= 0x88,
			x86op1_MOV_STORE	= 0x89,
			x86op1_MOV_LOAD		= 0x8B,
			// LEA reg, r/m
			x86op1_LEA			= 0x8D,
			// CDQ
			x86op1_CDQ			= 0x99,
			// SHL/SHR/SAR reg, imm8
			x86op1_SHIFT_IMM8	= 0xC1,
			// FLD/FST
			x86op1_FLD_FP32		= 0xD9,
			x86op1_FST_FP32		= 0xD9,
			x86op1_FLD_FP64		= 0xDD,
			x86op1_FST_FP64		= 0xDD,
			x86op1_FILD_FP16	= 0xDF,
			x86op1_FIST_FP16	= 0xDF,
			x86op1_FILD_FP32	= 0xDB,
			x86op1_FIST_FP32	= 0xDB,
			x86op1_FILD_FP64	= 0xDF,
			x86op1_FIST_FP64	= 0xDF,
			// near CALL rel32
			x86op1_CALL			= 0xE8,
			// near JMP rel32
			x86op1_JMP			= 0xE9,
			// LOCK
			x86op1_LOCK			= 0xF0,
			// TEST r/m, imm32
			x86op1_TEST_RM_IMM32	= 0xF7,
			// NEG r/m
			x86op1_NEG_RM		= 0xF7,	// 2ndop = 3
			// CALL near r/m
			x86op1_CALL_RM		= 0xFF,
			// JMP near r/m
			x86op1_JMP_RM		= 0xFF,
		} ;
		enum	X86Op80WithImm2ndOpCode	// for x86op1_OP_RM_IMM8
		{
			x86op2nd_CMP	= 7,
		} ;
		enum	X86Op81WithImm2ndOpCode	// for x86op1_OP_RM_IMM32
		{
			x86op2nd_ADD	= 0,
			x86op2nd_OR		= 1,
			x86op2nd_ADC	= 2,
		} ;
		enum	X86Shift2ndOpCode		// for x86op1_SHIFT_IMM8
		{
			x86op2nd_SHL	= 4,
			x86op2nd_SHR	= 5,
			x86op2nd_SAR	= 7,
		} ;
		enum	X86OpF7_2ndOpCode
		{
			x86op2nd_TEST_RM_IMM32	= 0,
			x86op2nd_NOT_RM			= 2,
			x86op2nd_NEG_RM			= 3,
			x86op2nd_MUL_RM			= 4,
			x86op2nd_IMUL_RM		= 5,
			x86op2nd_DIV_RM			= 6,
			x86op2nd_IDIV_RM		= 7,
		} ;
		enum	X86Float2ndOpCode
		{
			x86op2nd_FLD		= 0,
			x86op2nd_FST		= 2,
			x86op2nd_FIST		= 2,
			x86op2nd_FSTP		= 3,
			x86op2nd_FILD16		= 0,
			x86op2nd_FILD32		= 0,
			x86op2nd_FILD64		= 5,
			x86op2nd_FISTP16	= 3,
			x86op2nd_FISTP32	= 3,
			x86op2nd_FISTP64	= 7,
		} ;
		enum	X86DBOperationCode
		{
			// near Jcc rel32
			x86op2_JB	= 0x0F82,
			x86op2_JAE	= 0x0F83,
			x86op2_JE	= 0x0F84,
			x86op2_JNE	= 0x0F85,
			x86op2_JBE	= 0x0F86,
			x86op2_JA	= 0x0F87,
			x86op2_JS	= 0x0F88,
			x86op2_JNS	= 0x0F89,
			x86op2_JP	= 0x0F8A,
			x86op2_JPO	= 0x0F8B,
			x86op2_JL	= 0x0F8C,
			x86op2_JGE	= 0x0F8D,
			x86op2_JLE	= 0x0F8E,
			x86op2_JG	= 0x0F8F,
			// SHLD r/m, reg, imm8
			x86op2_SHLD_REG_RM_IMM8	= 0x0FA4,
			// SHRD r/m, reg, imm8
			x86op2_SHRD_REG_RM_IMM8	= 0x0FAC,
			// MOVZX reg, r/m
			x86op2_MOVZX_8	= 0x0FB6,
			x86op2_MOVZX_16	= 0x0FB7,
			// MOVSX reg, r/m
			x86op2_MOVSX_8	= 0x0FBE,
			x86op2_MOVSX_16	= 0x0FBF,
			// SETBE
			x86op2_SETBE_RM		= 0x0F96,
			// Float operation
			x86op2_FCOS		= 0xD9FF,
			x86op2_FSIN		= 0xD9FE,
		} ;
	public:
		// 低水準命令出力
		// レジスタ/メモリアクセス命令
		void * WriteX86RegMemOperand
			( DWORD binOPcode, int sizeOPcode,
				int regop, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0,
				DWORD immData = 0, int sizeImm = 0 ) ;
		// レジスタ即値命令
		void * WriteX86RegImmOperand
			( DWORD binOPcode, int sizeOPcode,
				int regop, X86Register regDst, DWORD immData, int sizeImm )
		{
			return	WriteX86RegMemOperand
				( binOPcode, sizeOPcode, regop,
					false, regDst, 0, x86_Nothing, 0, immData, sizeImm ) ;
		}
		// その他の即値命令
		void * WriteX86ImmediateOperand
			( DWORD binOPcode, int sizeOPcode,
				DWORD immData = 0, int sizeImm = 0 ) ;
	public:
		// call imm32
		void * WriteX86CallImm32( const void * pfnCallTarget ) ;
		// call mem/reg
		void WriteX86CallRegMem
			( bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// jmp imm32
		void * WriteX86JmpImm32( const void * pfnJmpTarget ) ;
		// push reg
		void WriteX86PushReg( X86Register regSrc ) ;
		// pop reg
		void WriteX86PopReg( X86Register regDst ) ;
		// mov reg, mem
		void WriteX86LoadRegMem
			( X86Register regDst, X86Register regBase,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// mov mem, reg
		void WriteX86StoreRegMem
			( X86Register regSrc, X86Register regBase,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// mov reg, imm32
		void * WriteX86MoveRegImm32( X86Register regDst, DWORD immData ) ;
		// mov reg, reg
		void WriteX86MoveRegReg( X86Register regDst, X86Register regSrc ) ;
		// lea reg, context->m_regset[x]
		void WriteX86LeaSakura2Register( X86Register regDst, int regSakura2 ) ;
		// lea reg, mem
		void WriteX86LeaRegMem
			( X86Register regDst, X86Register regBase,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// add reg, imm32
		void * WriteX86AddRegImm32( X86Register regDst, DWORD immData ) ;
		// add reg, mem/reg
		void WriteX86AddRegMem
			( X86Register regDst, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// adc reg, imm32
		void * WriteX86AdcRegImm32( X86Register regDst, DWORD immData ) ;
		// adc reg, mem/reg
		void WriteX86AdcRegMem
			( X86Register regDst, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// sub reg, mem/reg
		void WriteX86SubRegMem
			( X86Register regDst, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// or reg, mem/reg
		void WriteX86OrRegMem
			( X86Register regDst, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// and reg, mem/reg
		void WriteX86AndRegMem
			( X86Register regDst, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// xor reg, mem/reg
		void WriteX86XorRegMem
			( X86Register regDst, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// cmp reg, mem/reg
		void WriteX86CmpRegMem
			( X86Register regPhy, bool modeMemory, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0 ) ;
		// imul reg, imm32 (自動最適化)
		void WriteX86ImulRegImm32( X86Register regDst, int imm32 ) ;
		// shld reg, reg, imm8
		void WriteX86ShldRegRegImm8( X86Register regDst, X86Register regSrc, int imm8 ) ;
		// shrd reg, reg, imm8
		void WriteX86ShrdRegRegImm8( X86Register regDst, X86Register regSrc, int imm8 ) ;
		// shl reg, imm8
		void WriteX86ShlRegImm8( X86Register regDst, int imm8 ) ;
		// shr reg, imm8
		void WriteX86ShrRegImm8( X86Register regDst, int imm8 ) ;
		// sar reg, imm8
		void WriteX86SarRegImm8( X86Register regDst, int imm8 ) ;

	public:
		// 汎用演算命令出力（複雑な演算命令は外部関数を呼び出して解決する）
		// 関数呼び出しコード出力 : reg 形式
		virtual void WriteToCallInstructionReg
			( OPERATION_DST_SRC_PROC pfnGen, int reg ) ;
		// 関数呼び出しコード出力 : reg, reg 形式
		virtual void WriteToCallInstructionRegReg
			( OPERATION_DST_SRC_PROC pfnGen, int dstreg, int srcreg ) ;
		virtual void WriteToCallSIMD128InstructionRegReg
			( OPERATION_SIMD128_DST_SRC_PROC pfnGen, int dstreg, int srcreg ) ;
		// 関数呼び出しコード出力 : reg, reg, reg 形式
		virtual void WriteToCallInstructionRegRegReg
			( OPERATION_DST_SRC_SRC2_PROC pfnGen,
						int dstreg, int srcreg, int srcreg2 ) ;
		// 関数呼び出しコード出力 : reg, reg, imm 形式
		virtual void WriteToCallInstructionRegRegImm
			( OPERATION_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm ) ;
		virtual void WriteToCallSIMD128InstructionRegRegImm
			( OPERATION_SIMD128_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm ) ;
		// 関数呼び出し後のレジスタ処理
		virtual void ResetRegisterAfterCall( void ) ;

	public:
		// プロローグコード出力
		virtual void WritePrologue( void ) ;
		virtual void WriteSubPrologue( void ) ;
		// エピローグコード出力
		virtual void WriteEpilogue( int ipExit = -1 ) ;
		// レジスタへの変更をコンテキストに書き出し
		virtual void FlushAllRegisters( void ) ;
		// レジスタへの変更をコンテキストに書き出し
		//（レジスタコンテキストを変更しない）
		virtual void WriteBackAllRegisters( void ) ;
		// レジスタへの変更をコンテキストに書き出し
		virtual void FlushRegister( int regSakura ) ;
		// レジスタの値を物理レジスタに復元する
		virtual void ReloadRegisters( void ) ;
		// レジスタコンテキストをリセット
		virtual void ResetAllRegisters( void ) ;
		// レジスタコンテキストをリセット
		virtual void ResetRegister( int reg ) ;
		// レジスタコンテキストをリセット（ポインタレジスタ以外）
		virtual void ResetDataRegisters( void ) ;

		// BP ポインタ用物理レジスタ識別子取得
		virtual int GetFramePointerPhysicalRegister( void ) const ;
		// レジスタ値変更通知（BP 以外ポインタ用）
		virtual void ModifiedRegister( int reg ) ;
		// メモリ参照で使用する TLB スロットを取得する
		int SelectTLBSlotFromMemoryOperand
			( int regBase, int regIndex = -1, int scaleIndex = 0 ) ;
		// アドレス変換して物理レジスタにアドレスをロードするコード出力
		virtual void * WriteRealizePointerRegister
			( int regPhy, int regSakura,
				RealizePointerBoundary& rpb, const void * ptrEpilogue ) ;
		// 物理レジスタにロードしたポインタの境界チェックコードを完成させる
		virtual void CommitRealizePointerRegister
			( RealizePointerBoundary& rpb,
						int offsetFirst, int offsetEnd ) ;
		// ポインタアクセス用レジスタ割り当て処理コード出力
		//（物理ポインタ用レジスタ番号を返却）
		virtual int WriteAssignPointerRegister
			( int regPtr, int regIndex, int scale,
				int offsetFirst, int offsetEnd, void *& ptrEscJumpFrom ) ;
		// Sakura2 汎用レジスタを x86 汎用レジスタにロードするコードを出力
		virtual void WriteToLoadSakura2Register
			( X86Register regPhyLow, X86Register regPhyHigh,
					int regSakura, bool fOnlyLow = false ) ;
		virtual void WriteToLoadSakura2AddressRegister
			( X86Register regPhyLow, X86Register regPhyHigh,
				int regBasePtr, int regIndex, int scale ) ;
		// x86 汎用レジスタから Sakura2 汎用レジスタへストアするコードを出力
		virtual void WriteToStoreSakura2Register
			( int regSakura, X86Register regPhyLow,
					X86Register regPhyHigh, bool fOnlyLow = false ) ;
		// 仮想アドレスを実アドレスに変換するコードを出力
		virtual void * WriteToTranslateAddress
			( RealizePointerBoundary& rpb,
				X86Register regPhyBase, X86Register regPhyLow,
				X86Register regPhyHigh, int slotTLB ) ;
		// メモリ境界を判定するコードを出力
		virtual void * WriteToCheckBoundaryAddress
			( RealizePointerBoundary& rpb,
				X86Register regPhyLow,
				X86Register regPhyTemp,
				int offsetTLB, bool fBaseOffset ) ;
		// メモリを複製するコードを出力
		virtual void WriteToCopyMemory
			( bool fDstAligned,
				X86Register regPhyDst, INT_PTR dispDst,
					X86Register regPhyDstIndex, int scaleDstIndex,
				bool fSrcAligned,
					X86Register regPhySrc, INT_PTR dispSrc,
					X86Register regPhySrcIndex, int scaleSrcIndex,
				int	sizeInDWord,  X86Register regPhyTemp ) ;

	public:
		// 例外マスクに追加するコードを出力
		virtual void WriteToAtomicOrExceptionMask( DWORD dwException ) ;
		// スタック拡張例外判定
		virtual void * WriteToStackException
			( int& regPhyNewLowSP, int nOffsetSP, const void * ptrEpilogue ) ;
		// ゼロ除算例外用ゼロ比較脱出コード出力
		virtual void * WriteToZeroDivisionException
			( int regDiv, const void * ptrEpilogue ) ;
		virtual void * WriteToZeroDivisionException32
			( int regDiv, const void * ptrEpilogue ) ;
		// メモリ読み込み命令出力
		virtual void WriteToLoadPhysicalMemory
			( int regDst, int regPhyPtr,
					int offset, DataType type, bool fPair ) ;
		// メモリ書き出し命令出力
		virtual void WriteToStorePhysicalMemory
			( int regSrc, int regPhyPtr,
					int offset, DataType type, bool fPair ) ;

	public:
		// 無条件ジャンプコード出力
		virtual void * WriteToJump( const void * ptrTarget ) ;
		// 条件ジャンプコード出力
		virtual void * WriteToConditionalJump
				( int reg, bool fLogic, const void * ptrTarget ) ;
		// 例外判定離脱コード出力
		virtual void * WriteToEscapeByException( const void * ptrEpilogue ) ;
		// ジャンプコード完成（２パス用）
		virtual void CommitJumpTarget
				( void * ptrJumpFrom, const void * ptrFixedTarget ) ;

	public:
		// 各命令エンコーディングと低水準処理
		// 関数の返り値は例外発生時の脱出用ジャンプ命令アドレスで
		// CommitJumpTarget 関数の ptrJumpFrom に使用できる値

		// データ移動命令
		virtual void write_move_reg_reg( int regDst, int regSrc, bool fPair ) ;
		// シフト命令
		virtual void write_srl_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair ) ;
		virtual void write_sra_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair ) ;
		virtual void write_sll_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair ) ;
		// 32ビット即値命令
		virtual void write_add_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair ) ;
		// スタックレジスタ加算命令
		virtual void * write_add_sp_imm32( int ip, int imm32 ) ;
		// 64ビット即値命令
		virtual void write_move_reg_imm64( int regDst, INT64 imm64 ) ;
		// 間接無条件ジャンプ命令（write_push_ip と組み合わせればコール）
		//（exceptionFarJump 例外の判定と設定を含む）
		virtual void write_jump_reg( int reg ) ;
		// システムコール命令
		virtual void write_syscall_imm( int imm32 ) ;
		virtual void write_syscall_reg( int reg ) ;
		// リターン命令（exceptionFarJump 例外の判定と設定を含む）
		virtual void write_return( void ) ;
		// スタック処理
		virtual void * write_push_ip( int imm32 ) ;
		virtual void * write_push_reg( int regFirst, int nCount ) ;
		virtual void * write_pop_reg( int regFirst, int nCount ) ;
		// メモリヒント
		virtual void * write_prefetch_tlb( int tlb, int reg ) ;
		virtual void write_unfetch_tlb( int tlb, int reg ) ;
		// TLB を準備し物理レジスタにロードする
		void WritePrefetchTLB( int tlb, int reg ) ;

	} ;

} ;

#endif
