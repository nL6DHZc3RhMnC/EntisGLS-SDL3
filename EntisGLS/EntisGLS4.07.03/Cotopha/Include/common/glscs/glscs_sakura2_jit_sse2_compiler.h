
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_JIT_SSE2_COMPILER_H__)
#define	__GLSCS_SAKURA2_JIT_SSE2_COMPILER_H__

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
	// x86 SSE2 ネイティブコード化アセンブラ (IA32)
	//////////////////////////////////////////////////////////////////////////
	// ネイティブコード化関数プロトタイプ;
	//	void (__fastcall)( context* ) ;
	// 固定割り当てレジスタ;
	//	EBX = context*
	//	EBP = stack
	// ポインタ用一時割り当て;
	//	ECX, ESI, EDI
	//////////////////////////////////////////////////////////////////////////

	class	X86SSE2Assembler	: public X86GenericAssembler
	{
	public:
		// XMM レジスタ
		enum	SSERegister
		{
			XMM_Nothing	= -1,
			x86_XMM0, x86_XMM1, x86_XMM2, x86_XMM3,
			x86_XMM4, x86_XMM5, x86_XMM6, x86_XMM7,
			x86_XMM_Count,
		} ;
		// レジスタ・データ型
		enum	RegisterDataType
		{
			regTypeQWord,		// 64bit or packed integer
			regTypeFloat64,		// 64bit floating point
			regTypeDQWord,		// 128bit packed integer
			regTypeDFloat64,	// 2 packed 64bit floating point
			regTypeQFloat32,	// 4 packed 32bit floating point
			regTypeFirst128	= regTypeDQWord,
		} ;
		// 最近のレジスタデータ型テーブル
		RegisterDataType	m_rdtSakura[0x100] ;

		// データ・レジスタ割り当て
		// 偶数物理レジスタは XMM の下位
		// 奇数物理レジスタは XMM の上位
		struct	DataRegister	: public RegisterAssignation
		{
			RegisterDataType	typeRegData ;
		} ;
		RegisterAssignor<DataRegister,0x10>	m_lruDataReg ;

		// SSE オペコード reg, r/m
		// opPS 形式 : Packed Single-Precision Floting-Point
		// opPD 形式の場合には先頭に 0x66
		// opSD 形式の場合には先頭に 0xF2
		// opSS 形式の場合には先頭に 0xF3
		enum	X86SSEOperationCode
		{
			// 浮動小数点演算
			sseop_ADDPS			= 0x0F58,
			sseop_ADDPD			= 0x660F58,
			sseop_ADDSD			= 0xF20F58,
			sseop_ADDSS			= 0xF30F58,
			sseop_SUBPS			= 0x0F5C,
			sseop_SUBPD			= 0x660F5C,
			sseop_SUBSD			= 0xF20F5C,
			sseop_SUBSS			= 0xF30F5C,
			sseop_MULPS			= 0x0F59,
			sseop_MULPD			= 0x660F59,
			sseop_MULSD			= 0xF20F59,
			sseop_MULSS			= 0xF30F59,
			sseop_DIVPS			= 0x0F5E,
			sseop_DIVPD			= 0x660F5E,
			sseop_DIVSD			= 0xF20F5E,
			sseop_DIVSS			= 0xF30F5E,
			sseop_MAXPS			= 0x0F5F,
			sseop_MAXPD			= 0x660F5F,
			sseop_MAXSD			= 0xF20F5F,
			sseop_MAXSS			= 0xF30F5F,
			sseop_MINPS			= 0x0F5D,
			sseop_MINPD			= 0x660F5D,
			sseop_MINSD			= 0xF20F5D,
			sseop_MINSS			= 0xF30F5D,
			sseop_CMPPS			= 0x0FC2,	// imm8 に X86SSECompareCode
			sseop_CMPPD			= 0x660FC2,
			sseop_CMPSS			= 0xF30FC2,
			sseop_CMPSD			= 0xF20FC2,
			//
			// 近似演算でのプリフィックスは 0xF3 のみ
			sseop_RSQRTPS		= 0x0F52,
			sseop_RSQRTSS		= 0xF30F52,
			sseop_RCPPS			= 0x0F53,
			sseop_RCPSS			= 0xF30F53,
			//
			// プリフィックスに 0xF2 はない
			sseop_SQRTPS		= 0x0F51,
			sseop_SQRTPD		= 0x660F51,
			sseop_SQRTSS		= 0xF30F51,
			//
			// 浮動小数点形式でのビット演算と順序処理のプリフィックスは 0x66 のみ
			sseop_ANDPS			= 0x0F54,
			sseop_ANDPD			= 0x660F54,
			sseop_ANDNPS		= 0x0F55,
			sseop_ANDNPD		= 0x660F55,
			sseop_ORPS			= 0x0F56,
			sseop_ORPD			= 0x660F56,
			sseop_XORPS			= 0x0F57,
			sseop_XORPD			= 0x660F57,
			//
			sseop_SHUFPS		= 0x0FC6,
			sseop_SHUFPD		= 0x660FC6,
			sseop_UNPCKLPS		= 0x0F14,
			sseop_UNPCKLPD		= 0x660F14,
			sseop_UNPCKHPS		= 0x0F15,
			sseop_UNPCKHPD		= 0x660F15,
			//
			// COMISS, COMISD のみ
			sseop_COMISS		= 0x0F2F,
			sseop_COMISD		= 0x660F2F,
			//
			// パック整数形式の演算
			sseop_PADDB			= 0x660FFC,
			sseop_PADDW			= 0x660FFD,
			sseop_PADDD			= 0x660FFE,
			sseop_PADDQ			= 0x660FD4,
			sseop_PADDUSB		= 0x660FDC,
			sseop_PADDUSW		= 0x660FDD,
			sseop_PADDSB		= 0x660FEC,
			sseop_PADDSW		= 0x660FED,
			//
			sseop_PSUBB			= 0x660FF8,
			sseop_PSUBW			= 0x660FF9,
			sseop_PSUBD			= 0x660FFA,
			sseop_PSUBQ			= 0x660FFB,
			sseop_PSUBUSB		= 0x660FD8,
			sseop_PSUBUSW		= 0x660FD9,
			sseop_PSUBSB		= 0x660FE8,
			sseop_PSUBSW		= 0x660FE9,
			//
			sseop_PMADDWD		= 0x660FF5,
			sseop_PMULHUW		= 0x660FE4,
			sseop_PMULHW		= 0x660FE5,
			sseop_PMULLW		= 0x660FD5,
			sseop_PMULUDQ		= 0x660FF4,
			//
			sseop_PAVGB			= 0x660FE0,
			sseop_PAVGW			= 0x660FE3,
			sseop_PMINUB		= 0x660FDA,
			sseop_PMINSW		= 0x660FEA,
			sseop_PMAXUB		= 0x660FDE,
			sseop_PMAXSW		= 0x660FEE,
			//
			sseop_PCMPGTB		= 0x660F64,
			sseop_PCMPGTW		= 0x660F65,
			sseop_PCMPGTD		= 0x660F66,
			sseop_PCMPEQB		= 0x660F74,
			sseop_PCMPEQW		= 0x660F75,
			sseop_PCMPEQD		= 0x660F76,
			//
			sseop_PSLLW			= 0x660FF1,
			sseop_PSLLD			= 0x660FF2,
			sseop_PSLLQ			= 0x660FF3,
			sseop_PSRAW			= 0x660FE1,
			sseop_PSRAD			= 0x660FE2,
			sseop_PSRLW			= 0x660FD1,
			sseop_PSRLD			= 0x660FD2,
			sseop_PSRLQ			= 0x660FD3,
			sseop_PSHIFTW_IMM	= 0x660F71,		// 第二オペコードは X86MMXShiftImm2ndOpCode
			sseop_PSHIFTD_IMM	= 0x660F72,
			sseop_PSHIFTQ_IMM	= 0x660F73,		// mmxop2nd_SRA は使用不可
			sseop_PSHIFTDQ_IMM	= 0x660F73,		// mmxop2nd_SRLDQ or mmxop2nd_SLLDQ
			//
			sseop_PAND			= 0x660FDB,
			sseop_PANDN			= 0x660FDF,
			sseop_POR			= 0x660FEB,
			sseop_PXOR			= 0x660FEF,
			//
			// 整数パック変換
			sseop_PACKSSWB		= 0x660F63,
			sseop_PACKUSWB		= 0x660F67,
			sseop_PACKSSDW		= 0x660F6B,
			//
			sseop_PUNPCKLBW		= 0x660F60,
			sseop_PUNPCKLWD		= 0x660F61,
			sseop_PUNPCKLDQ		= 0x660F62,
			sseop_PUNPCKLQDQ	= 0x660F6C,
			//
			sseop_PUNPCKHBW		= 0x660F68,
			sseop_PUNPCKHWD		= 0x660F69,
			sseop_PUNPCKHDQ		= 0x660F6A,
			sseop_PUNPCKHQDQ	= 0x660F6D,
			//
			sseop_PSHUFD		= 0x660F70,
			sseop_PSHUFLW		= 0xF20F70,
			sseop_PSHUFHW		= 0xF30F70,
			//
			// ムーブ・変換命令にはプリフィックスの特殊な組み合わせがあるので注意
			sseop_MOVMSK		= 0x0F50,
			sseop_MOVSD_LOAD	= 0xF20F10,
			sseop_MOVSS_LOAD	= 0xF30F10,
			sseop_MOVSD_STORE	= 0xF20F11,
			sseop_MOVSS_STORE	= 0xF30F11,
			sseop_MOVAPS_LOAD	= 0x0F28,
			sseop_MOVAPD_LOAD	= 0x660F28,
			sseop_MOVAPS_STORE	= 0x0F29,
			sseop_MOVAPD_STORE	= 0x660F29,
			sseop_MOVUPS_LOAD	= 0x0F10,
			sseop_MOVUPD_LOAD	= 0x660F10,
			sseop_MOVUPS_STORE	= 0x0F11,
			sseop_MOVUPD_STORE	= 0x660F11,
			sseop_MOVD_LOAD		= 0x660F6E,
			sseop_MOVD_STORE	= 0x660F7E,
			sseop_MOVQ_LOAD		= 0xF30F7E,
			sseop_MOVQ_STORE	= 0x660FD6,
			sseop_MOVDQA_LOAD	= 0x660F6F,
			sseop_MOVDQA_STORE	= 0x660F7F,
			sseop_MOVDQU_LOAD	= 0xF30F6F,
			sseop_MOVDQU_STORE	= 0xF30F7F,
			sseop_MOVHLPS		= 0x0F12,
			sseop_MOVLPS_LOAD	= 0x0F12,
			sseop_MOVLPD_LOAD	= 0x660F12,
			sseop_MOVLPS_STORE	= 0x0F13,
			sseop_MOVLPD_STORE	= 0x660F13,
			sseop_MOVLHPS		= 0x0F16,
			sseop_MOVHPS_LOAD	= 0x0F16,
			sseop_MOVHPD_LOAD	= 0x660F16,
			sseop_MOVHPS_STORE	= 0x0F17,
			sseop_MOVHPD_STORE	= 0x660F17,
			sseop_CVT_PI2PS		= 0x0F2A,
			sseop_CVT_PI2PD		= 0x660F2A,
			sseop_CVT_SI2SD		= 0xF20F2A,
			sseop_CVT_SI2SS		= 0xF30F2A,
			sseop_CVTT_PS2PI	= 0x0F2C,
			sseop_CVTT_PD2PI	= 0x660F2C,
			sseop_CVTT_SD2SI	= 0xF20F2C,
			sseop_CVTT_SS2SI	= 0xF30F2C,
			sseop_CVT_PS2PI		= 0x0F2D,
			sseop_CVT_PD2PI		= 0x660F2D,
			sseop_CVT_SD2SI		= 0xF20F2D,
			sseop_CVT_SS2SI		= 0xF30F2D,
			sseop_CVT_PS2PD		= 0x0F5A,
			sseop_CVT_PD2PS		= 0x660F5A,
			sseop_CVT_SD2SS		= 0xF20F5A,
			sseop_CVT_SS2SD		= 0xF30F5A,
			sseop_CVT_DQ2PS		= 0x0F5B,
			sseop_CVT_PS2DQ		= 0x660F5B,
			sseop_CVTT_PS2DQ	= 0xF30F5B,
			sseop_CVTT_PD2DQ	= 0x660FE6,
			sseop_CVT_PD2DQ		= 0xF20FE6,
			sseop_CVT_DQ2PD		= 0xF30FE6,
		} ;
		enum	X86SSECompareCode	// for imm8 of sseop_CMP
		{
			sseimm_CMP_EQ		= 0,
			sseimm_CMP_LT		= 1,
			sseimm_CMP_LE		= 2,
			sseimm_CMP_UNORD	= 3,
			sseimm_CMP_NEQ		= 4,
			sseimm_CMP_NLT		= 5,
			sseimm_CMP_NLE		= 6,
			sseimm_CMP_ORD		= 7,
			sseimm_CMP_NE		= sseimm_CMP_NEQ,
			sseimm_CMP_GE		= sseimm_CMP_NLT,
			sseimm_CMP_GT		= sseimm_CMP_NLE,
		} ;
		enum	X86MMXShiftImm2ndOpCode	// for sseop_PSHIFTx_IMM
		{
			mmxop2nd_SRL	= 2,
			mmxop2nd_SRLDQ	= 3,
			mmxop2nd_SRA	= 4,
			mmxop2nd_SLL	= 6,
			mmxop2nd_SLLDQ	= 7,
		} ;

	protected:
		void *	m_pConst64PairSignMask ;	// 64bit 符号マスクペア
		void *	m_pConst64PairNoSignMask ;	// 64bit 非符号ビットマスクペア
		void *	m_pConst64Pair80000000 ;	// 64bit 80000000H ペア
		void *	m_pConst128Mask0F ;			// 64bit シフト値用マスク
		void *	m_pConst128Mask1F ;			// 64bit シフト値用マスク
		void *	m_pConst128Mask3F ;			// 64bit シフト値用マスク
		void *	m_pConst32NoSignMask ;		// 32bit 非符号ビットマスク
		void *	m_pConst32PackNoSignMask ;	// 32bit 非符号ビットマスクペア

	public:
		// 構築関数
		X86SSE2Assembler( void ) ;

	public:
		// SSE 低水準命令出力
		// op xmm, mem [,imm8] 形式
		void * WriteSSERegMemOperand
			( DWORD binOPcode, int sizeOPcode,
				SSERegister xmmReg, X86Register regmem,
				INT_PTR dispOffset = 0,
				X86Register regIndex = x86_Nothing, int scaleIndex = 0,
				DWORD immData = 0, int sizeImm = 0 )
		{
			return	WriteX86RegMemOperand
				( binOPcode, sizeOPcode, xmmReg,
					true, regmem, dispOffset,
					regIndex, scaleIndex, immData, sizeImm ) ;
		}
		// op xmm, xmm [,imm8] 形式
		void * WriteSSERegRegOperand
			( DWORD binOPcode, int sizeOPcode,
				SSERegister xmmDst, SSERegister xmmSrc,
				DWORD immData = 0, int sizeImm = 0 )
		{
			return	WriteX86RegMemOperand
				( binOPcode, sizeOPcode,
					xmmDst, false, (X86Register) xmmSrc,
					0, x86_Nothing, 0, immData, sizeImm ) ;
		}
		// op xmm, imm8 形式
		void * WriteSSERegRegImm8Operand
			( DWORD binOPcode, int sizeOPcode, int op2nd,
				SSERegister xmmReg, int imm8 )
		{
			return	WriteX86RegMemOperand
				( binOPcode, sizeOPcode, op2nd,
					false, (X86Register) xmmReg,
					0, x86_Nothing, 0, imm8, 1 ) ;
		}
		// 定数値 64bit 最上位ビット 8000000000000000H ペア
		void * GetConstantPair8000000000000000( void ) ;
		// 定数値 64bit 7FFFFFFFFFFFFFFFH ペア
		void * GetConstantPair7FFFFFFFFFFFFFFF( void ) ;
		// 定数値 64bit 80000000H ペア
		void * GetConstantPair80000000( void ) ;
		// 定数値 128bit 0FH
		void * GetConstantDQWord0F( void ) ;
		// 定数値 128bit 1FH
		void * GetConstantDQWord1F( void ) ;
		// 定数値 128bit 3FH
		void * GetConstantDQWord3F( void ) ;
		// 定数値 32bit 7FFFFFFFH : -1 : -1 : -1
		void * GetConstantLow32NoSignMask( void ) ;
		// 定数値 32bit 7FFFFFFFH ペア
		void * GetConstantPack32NoSignMask( void ) ;

	public:
		// データ型を64ビットに正規化
		static RegisterDataType
				NormalizeDataTypeTo64( RegisterDataType regType ) ;
		// データ型を128ビットに正規化
		static RegisterDataType
				NormalizeDataTypeTo128( RegisterDataType regType ) ;

	public:
		// Sakura2 レジスタを SSE 物理レジスタに割り当て／ロード
		SSERegister WriteRealizeDataRegister
			( int regSakura, RegisterDataType regType, bool fLoad = true ) ;
		// Sakura2 レジスタを割り当て済みの SSE 物理レジスタを取得
		// 取得データ型によっては正規化
		SSERegister GetRealizedDataRegister
			( int regSakura, RegisterDataType regType, bool fNormalize = true ) ;
		// 一時処理のための物理レジスタを確保
		SSERegister AllocateDataRegister( RegisterDataType regType ) ;
		// レジスタの値更新フラグ設定
		void SetDataRegisterModified( SSERegister xmmReg ) ;
		// 物理レジスタの変更をライトバック
		void WriteBackDataRegister( SSERegister xmmReg, bool fKeepModified = false ) ;
		// 物理レジスタの内容をリロード
		void ReloadDataRegister( SSERegister xmmReg ) ;
		// Sakura2 レジスタの割り当てをスワップ
		void SwapDataRegisterAssignation( SSERegister xmmReg1, SSERegister xmmReg2 ) ;
		// 物理レジスタの割り当てを一時的にロック
		void LockDataRegister( SSERegister xmmReg, RegisterDataType regType ) ;
		// 物理レジスタの割り当てを解放可能にアンロック
		void UnlockDataRegister( SSERegister xmmReg, RegisterDataType regType ) ;
		// 物理レジスタの割り当てを解放
		void FreeDataRegister( SSERegister xmmReg, RegisterDataType regType ) ;
		// 物理レジスタのペア割り当ての妥当性をチェック（デバッグ用）
		void VerifyPairDataRegister( void ) ;

	public:
		// オーバーライド関数
		//////////////////////////////////////////////////////////////////////

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
		virtual void ResetRegister( int regSakura ) ;
		// レジスタコンテキストをリセット（ポインタレジスタ以外）
		virtual void ResetDataRegisters( void ) ;

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
		// メモリ読み込み命令出力
		virtual void WriteToLoadPhysicalMemory
			( int regDst, int regPhyPtr,
					int offset, DataType type, bool fPair ) ;
		// メモリ書き出し命令出力
		virtual void WriteToStorePhysicalMemory
			( int regSrc, int regPhyPtr,
					int offset, DataType type, bool fPair ) ;

	public:
		// 各命令エンコーディングと低水準処理
		// 関数の返り値は例外発生時の脱出用ジャンプ命令アドレスで
		// CommitJumpTarget 関数の ptrJumpFrom に使用できる値

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
		// 64ビット即値命令
		virtual void write_move_reg_imm64( int regDst, INT64 imm64 ) ;
		// 1 OP 演算命令
		virtual void write_neg_int( int regDst ) ;
		virtual void write_not_int( int regDst ) ;
		virtual void write_neg_float( int regDst ) ;
		// 2 OP 整数演算命令
		virtual void write_add_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_sub_reg_reg( int regDst, int regSrc, bool fPair ) ;
		virtual void write_mul_reg_reg( int regDst, int regSrc, bool fPair ) ;
//		virtual void write_div_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
//		virtual void write_mod_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
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
		// 浮動小数点演算 EXTENSION
		virtual void write_float_extension( int code, int regDst, int regSrc ) ;
		// 64bit SIMD
		virtual void write_simd64_extension( int code, int regDst, int regSrc, bool fPair ) ;
		virtual void write_simd64_imm_extension( int code, int regDst, int regSrc, int imm8, bool fPair ) ;
		// 128bit SIMD
		virtual void write_simd128_extension( int code, int regDst, int regSrc ) ;
		virtual void write_simd128_imm_extension( int code, int regDst, int regSrc, int imm8 ) ;

	protected:
		// 64bit 整数比較命令生成
		//	xmmDst <- (xmmCmp1 > xmmCmp2) ^ fLogicalNot
		void write_cmp_int64_gt
			( SSERegister xmmDst,
				SSERegister xmmTemp1, SSERegister xmmTemp2,
				SSERegister xmmCmp1, SSERegister xmmCmp2,
				bool fLogicalNot, bool fPair ) ;
		// 64bit x 64bit -> 64bit 乗算命令生成
		void write_mul_int64xint64
			( SSERegister xmmDst, SSERegister xmmSrc,
				SSERegister xmmTemp1, SSERegister xmmTemp2 ) ;

	} ;

} ;

#endif
