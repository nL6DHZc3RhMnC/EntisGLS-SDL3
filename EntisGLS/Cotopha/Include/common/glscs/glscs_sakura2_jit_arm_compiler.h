
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_JIT_ARMV7_COMPILER_H__)
#define	__GLSCS_SAKURA2_JIT_ARMV7_COMPILER_H__

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
	// ARM コードバッファ出力オブジェクト (ARMv4 以降)
	//////////////////////////////////////////////////////////////////////////

	class	ARMCodeBuffer	: public CodeBuffer
	{
	protected:
		struct	ExecuatbleMemory
		{
			ExecuatbleMemory *	pNext ;
			BYTE *				pbytBuf ;
			size_t				nBufBytes ;
			size_t				nUsedBytes ;
		} ;
		ExecuatbleMemory *	m_pxmChain ;
		int					m_armVersion ;		// ARMvX
		bool				m_modeThumb ;

		// 実行・読み書き可能メモリの確保
		virtual BYTE * AllocateCodeBuffer( size_t& nSize ) ;
		// 実行・読み書き可能メモリの解放
		virtual void FreeCodeBuffer( BYTE * pbytBuf, size_t nSize ) ;
		// ジャンプ命令を追加する
		virtual void WriteJump( Block * pBlock, const void * pTarget ) ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ARMCodeBuffer, CodeBuffer )
		// 構築関数
		ARMCodeBuffer( void ) ;
		// 消滅関数
		virtual ~ARMCodeBuffer( void ) ;
		// 次の命令を出力アドレスを取得する
		virtual void * GetNext( void ) ;
		// データを確定する
		virtual void CommitAllCodes( void ) ;
		// ARM 命令選択
		void SelectARMInstruction( int armVersion, bool modeThumb ) ;
		// 指定バイト数分は最低限度分割されないことを保証する
		void PreserveContinuousCodes( size_t nBytes ) ;

	protected:
		static size_t	m_timesEstimatedCPUDataCache ;
		static size_t	m_bytesEstimatedCPUDataCache ;

	public:
		// CPU キャッシュサイズを取得する（目安）
		static size_t GetCPUDataCacheSize( bool fForceEstimate = false ) ;
		static size_t EstimateCPUDataCacheSize( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ARM ネイティブコード化アセンブラ (ARMv4 以降 / thumb では ARMv6以降)
	//////////////////////////////////////////////////////////////////////////
	// ネイティブコード化関数プロトタイプ;
	//	void (*)( context* ) ;
	// 固定割り当てレジスタ;
	//	r10 = context*
	//	r11 = Sakura2VM stack (frame pointer)
	// ポインタ用一時割り当て;
	//	r7, r8, r9, r12
	// 長距離ジャンプアドレス/一時アドレス/その他一時処理用;
	//  r6
	//////////////////////////////////////////////////////////////////////////

	class	ARMGenericAssembler	: public Sakura2Assembler
	{
	public:
		// ARM 汎用レジスタ
		enum	ARMRegister
		{
			ARM_Nothing	= -1,
			ARM_r0, ARM_r1, ARM_r2, ARM_r3,
			ARM_r4, ARM_r5, ARM_r6, ARM_r7,
			ARM_r8, ARM_r9, ARM_r10, ARM_r11,
			ARM_r12, ARM_r13, ARM_r14, ARM_r15,
			ARM_SP = 13, ARM_LR = 14, ARM_PC = 15,
		} ;
		// VFP 拡張レジスタ
		enum	VFPRegister
		{
			VFP_Nothing	= -1,
			// 32bit
			VFP_s0 = 0, VFP_s1, VFP_s2, VFP_s3,
			VFP_s4, VFP_s5, VFP_s6, VFP_s7,
			VFP_s8, VFP_s9, VFP_s10, VFP_s11,
			VFP_s12, VFP_s13, VFP_s14, VFP_s15,
			// 64bit
			VFP_d0 = 0, VFP_d1, VFP_d2, VFP_d3,
			VFP_d4, VFP_d5, VFP_d6, VFP_d7,
			VFP_d8, VFP_d9, VFP_d10, VFP_d11,
			VFP_d12, VFP_d13, VFP_d14, VFP_d15,
			// 128bit
			VFP_q0 = 0, VFP_q1, VFP_q2, VFP_q3,
			VFP_q4, VFP_q5, VFP_q6, VFP_q7,
			VFP_q8, VFP_q9, VFP_q10, VFP_q11,
			VFP_q12, VFP_q13, VFP_q14, VFP_q15,
		} ;
		// ARM 実行条件
		enum	ARMCondition
		{
			cond_EQ = 0, cond_NE, cond_CS, cond_CC,
			cond_MI, cond_PL, cond_VS, cond_VC,
			cond_HI, cond_LS, cond_GE, cond_LT,
			cond_GT, cond_LE, cond_AL,
			cond_Caryy = cond_CS,
			cond_NonCaryy = cond_CC,
		} ;

		// ポインタ・レジスタ割り当て
		struct	PointerRegister
					: public RegisterAssignation,
						public Sakura2Assembler::RealizePointerBoundary
		{
			bool	fTLBFetched ;
			int		regPhyIndex ;

			PointerRegister( void )
				: fTLBFetched(false), regPhyIndex(ARM_Nothing) {}
		} ;
		enum	PointerRegisterIndex
		{
			regPtrPhyR7 = 0,
			regPtrPhyR8,
			regPtrPhyR9,
			regPtrPhyR12,
			regPtrPhyCount,
			regPtrPhyR11Index	= regPtrPhyCount,
		} ;
		RegisterAssignor<PointerRegister,regPtrPhyCount>	m_lruPointer ;

		// 命令コード
		enum	ARMInstructionCode
		{
			// ARM  op reg, reg, reg, shift
			armOpAND	= 0x00000000,
			thumbOpAND	= 0xEA000000,
			armOpEOR	= 0x00200000,
			thumbOpEOR	= 0xEA800000,
			armOpORR	= 0x01800000,
			thumbOpORR	= 0xEA400000,
			armOpADD	= 0x00800000,
			thumbOpADD	= 0xEB000000,
			armOpADC	= 0x00A00000,
			thumbOpADC	= 0xEB400000,
			armOpSUB	= 0x00400000,
			thumbOpSUB	= 0xEBA00000,
			armOpSBC	= 0x00C00000,
			thumbOpSBC	= 0xEB600000,
			armOpMVN	= 0x01E00000,
			thumbOpMVN	= 0xEA6F0000,

			// ARM  op reg, reg, reg
			armOpLSL	= 0x01A00010,
			thumbOpLSL	= 0xFA00F000,
			armOpLSR	= 0x01A00030,
			thumbOpLSR	= 0xFA20F000,
			armOpASR	= 0x01A00050,
			thumbOpASR	= 0xFA40F000,

			// ARM op reg, reg, imm8
			armOpAndImm12	= 0x02000000,
			thumbOpAndImm12	= 0xF0000000,
			armOpAddImm12	= 0x02800000,
			thumbOpAddImm12	= 0xF1000000,
			armOpAdcImm12	= 0x02A00000,
			thumbOpAdcImm12	= 0xF1400000,
			armOpSubImm12	= 0x02400000,
			thumbOpSubImm12	= 0xF1A00000,
			armOpSbcImm12	= 0x02C00000,
			thumbOpSbcImm12	= 0xF1600000,

			// ARM SIMD op reg, reg, reg
			armOpQADD32		= 0x01000050,		// 飽和加減算
			thumbOpQADD32	= 0xFA80F080,
			armOpQSUB32		= 0x01200050,
			thumbOpQSUB32	= 0xFA80F0A0,
			armOpSADD16		= 0x06100F10,
			thumbOpSADD16	= 0xFA90F000,
			armOpQADD16		= 0x06200F10,
			thumbOpQADD16	= 0xFA90F010,
			armOpUQADD16	= 0x06600F10,
			thumbOpUQADD16	= 0xFA90F050,
			armOpSSUB16		= 0x06100F70,
			thumbOpSSUB16	= 0xFAD0F000,
			armOpQSUB16		= 0x06200F70,
			thumbOpQSUB16	= 0xFAD0F010,
			armOpUQSUB16	= 0x06600F70,
			thumbOpUQSUB16	= 0xFAD0F050,
			armOpSADD8		= 0x06100F90,
			thumbOpSADD8	= 0xFA80F000,
			armOpQADD8		= 0x06200F90,
			thumbOpQADD8	= 0xFA80F010,
			armOpUQADD8		= 0x06600F90,
			thumbOpUQADD8	= 0xFA80F050,
			armOpSSUB8		= 0x06100FF0,
			thumbOpSSUB8	= 0xFAC0F000,
			armOpQSUB8		= 0x06200FF0,
			thumbOpQSUB8	= 0xFAC0F010,
			armOpUQSUB8		= 0x06600FF0,
			thumbOpUQSUB8	= 0xFAC0F050,

			// ARM
			armOpUSAT		= 0x06E00010,
			thumbOpUSAT		= 0xF3800000,
			armOpUSAT16		= 0x06E00F30,
			thumbOpUSAT16	= 0xF3A00000,
			armOpSSAT		= 0x06A00010,
			thumbOpSSAT		= 0xF3000000,
			armOpSSAT16		= 0x06A00F30,
			thumbOpSSAT16	= 0xF3200000,
			armOpPKH		= 0x06800010,
			thumbOpPKH		= 0xEAC00000,
			armOpBFI		= 0x07C00010,
			thumbOpBFI		= 0xF3600000,
			armOpUBFX		= 0x07E00050,
			thumbOpUBFX		= 0xF3C00000,
			armOpUXTB		= 0x06EF0070,
			thumbOpUXTB		= 0xFA5FF080,
			armOpUXTB16		= 0x06CF0070,
			thumbOpUXTB16	= 0xFA3FF080,
			armOpUXTH		= 0x06FF0070,
			thumbOpUXTH		= 0xFA1FF080,

			// VFP
			armOpFADD		= 0x0E300A00,
			thumbOpFADD		= 0xEE300A00,
			armOpFSUB		= 0x0E300A40,
			thumbOpFSUB		= 0xEE300A40,
			armOpFMUL		= 0x0E200A00,
			thumbOpFMUL		= 0xEE200A00,
			armOpFDIV		= 0x0E800A00,
			thumbOpFDIV		= 0xEE800A00,
			armOpFABS		= 0x0EB00AC0,
			thumbOpFABS		= 0xEEB00AC0,
			armOpFSQRT		= 0x0EB10AC0,
			thumbOpFSQRT	= 0xEEB10AC0,
			armOpFCMP		= 0x0EB40A40,
			thumbOpFCMP		= 0xEEB40A40,
			armOpFNEG		= 0x0EB10A40,
			thumbOpFNEG		= 0xEEB10A40,
			armOpVMRS		= 0x0EF10A10,
			thumbOpVMRS		= 0xEEF10A10,

			// NEON
			armOpVMOV		= 0xF2200110,
			thumbOpVMOV		= 0xEF200110,
			armOpVAND		= 0xF2000110,
			thumbOpVAND		= 0xEF000110,
			armOpVEOR		= 0xF3000110,
			thumbOpVEOR		= 0xFF000110,
			armOpVORR		= 0xF2200110,
			thumbOpVORR		= 0xEF200110,
			armOpVBIT		= 0xF3200110,
			thumbOpVBIT		= 0xFF200110,
			armOpVBIF		= 0xF3300110,
			thumbOpVBIF		= 0xFF300110,
			armOpVCVT_F32S32	= 0xF3BB0600,
			thumbOpVCVT_F32S32	= 0xFFBB0600,
			armOpVCVT_F32U32	= 0xF3BB0680,
			thumbOpVCVT_F32U32	= 0xFFBB0680,
			armOpVCVT_S32F32	= 0xF3BB0700,
			thumbOpVCVT_S32F32	= 0xFFBB0700,
			armOpVCVT_U32F32	= 0xF3BB0780,
			thumbOpVCVT_U32F32	= 0xFFBB0780,
			armOpVMVN		= 0xF3B00580,
			thumbOpVMVN		= 0xFFB00580,
			armOpVSWP		= 0xF3B20000,
			thumbOpVSWP		= 0xFFB20000,

			armOpVIADD		= 0xF2000800,
			thumbOpVIADD	= 0xEF000800,
			armOpVFADD		= 0xF2000D00,
			thumbOpVFADD	= 0xEF000D00,
			armOpVISUB		= 0xF3000800,
			thumbOpVISUB	= 0xFF000800,
			armOpVFSUB		= 0xF2200D00,
			thumbOpVFSUB	= 0xEF200D00,
			armOpVFABS		= 0xF3B90700,
			thumbOpVFABS	= 0xFFB90700,
			armOpVFMUL		= 0xF3000D10,
			thumbOpVFMUL	= 0xFF000D10,
			armOpVFMAX		= 0xF2200F00,
			thumbOpVFMAX	= 0xEF200F00,
			armOpVFMIN		= 0xF2000F00,
			thumbOpVFMIN	= 0xEF000F00,
			armOpVFRECPE	= 0xF3BB0500,
			thumbOpVFRECPE	= 0xFFBB0500,
			armOpVFRSQRTE	= 0xF3BB0580,
			thumbOpVFRSQRTE	= 0xFFBB0580,
			armOpVICmpEQ	= 0xF3000810,
			thumbOpVICmpEQ	= 0xFF000810,
			armOpVICmpGE	= 0xF2000310,
			thumbOpVICmpGE	= 0xEF000310,
			armOpVICmpGT	= 0xF2000300,
			thumbOpVICmpGT	= 0xEF000300,
			armOpVFCmpEQ	= 0xF2000E00,
			thumbOpVFCmpEQ	= 0xEF000E00,
			armOpVFCmpGE	= 0xF3000E00,
			thumbOpVFCmpGE	= 0xFF000E00,
			armOpVFCmpGT	= 0xF3200E00,
			thumbOpVFCmpGT	= 0xFF200E00,
			armOpVINEG		= 0xF3B10380,
			thumbOpVINEG	= 0xFFB10380,

			armOpVQADD		= 0xF2000010,
			thumbOpVQADD	= 0xEF000010,
			armOpVQSUB		= 0xF2000210,
			thumbOpVQSUB	= 0xEF000210,

			armOpVMUL		= 0xF2000910,
			thumbOpVMUL		= 0xEF000910,
			armOpVMULL		= 0xF2800C00,
			thumbOpVMULL	= 0xEF800C00,
			armOpVSHL		= 0xF2000400,
			thumbOpVSHL		= 0xEF000400,
			armOpVREV		= 0xF3B00000,
			thumbOpVREV		= 0xFFB00000,
			armOpVTRN		= 0xF3B20080,
			thumbOpVTRN		= 0xFFB20080,
			armOpVZIP		= 0xF3B20180,
			thumbOpVZIP		= 0xFFB20180,
			armOpVUZP		= 0xF3B20100,
			thumbOpVUZP		= 0xFFB20100,

			armOpVSHLImm	= 0xF2800510,
			thumbOpVSHLImm	= 0xEF800510,
			armOpVQSHLImm	= 0xF2800610,
			thumbOpVQSHLImm	= 0xEF800610,
			armOpVSHRImm	= 0xF2800010,
			thumbOpVSHRImm	= 0xEF800010,
			armOpVEXTImm	= 0xF2B00000,
			thumbOpVEXTImm	= 0xEFB00000,
			armOpVDUPImm	= 0xF3B00C00,
			thumbOpVDUPImm	= 0xFFB00C00,
		} ;

		// Advanced SIMD data type
		enum	NEONDataType
		{
			typeNEONInt8	= 0,
			typeNEONInt16,
			typeNEONInt32,
			typeNEONInt64,
		} ;

		// データ・レジスタの種類
		enum	DataRegisterClass
		{
			regClassNothing	= -1,
			regClassARM,	// 汎用レジスタのペア {r1:r0,r3:r2,r5:r4}
			regClassVFP,	// VFP レジスタ（64bit） d0～d15  (VFPv2)
			regClassNEON,	// VFP レジスタ（128bit）q8～q15 (d16～d31) (VFPv3)
		} ;
		struct	DataPhysicalRegister
		{
			DataRegisterClass	regClass ;		// 割り当て先レジスタ種類
			int					regPhy ;		// 物理レジスタ番号
		} ;
		// データ・レジスタ割り当て
		struct	DataRegister	: public RegisterAssignation
		{
			const DataRegister& operator = ( const DataRegister& dr )
			{
				RegisterAssignation::operator = ( dr ) ;
				return	*this ;
			}
		} ;
		// 全データレジスタ・割り当てコンテキスト
		struct	DataRegisterContext
		{
			RegisterAssignor<DataRegister,3>	lruRegARM ;
			RegisterAssignor<DataRegister,16>	lruRegVFP ;
			RegisterAssignor<DataRegister,8>	lruRegNEON ;
			DataPhysicalRegister				dprSakura[0x100] ;

			// 構築関数
			DataRegisterContext( void ) ;
			// 代入
			const DataRegisterContext&
					operator = ( const DataRegisterContext& drc ) ;
			// 現在の割り当てレジスタ取得
			bool GetLoadedPhysicalRegister
					( DataPhysicalRegister& dpr, int regSakura ) const ;
		} ;
		DataRegisterContext	m_lruDataReg ;

		// 生成コードタイプ
		int		m_armVersion ;		// ARMvX >= 4, or >= 6 if thumb mode
		int		m_vfpVersion ;		// VFPvX == 0, or >= 2
		bool	m_vfpNEON ;			// NEON
		bool	m_modeThumb ;		// thumb mode

		// デバッグ用
		#if	defined(__DEBUG__)
		DWORD		m_dwFuncAddr ;
		ulong_ptr_t	m_pCurCode ;
		DWORD		m_ipCurrent ;
		#endif

	public:
		// 構築関数
		ARMGenericAssembler( void )
		{
			m_lruPointer[regPtrPhyR7].regPhy = ARM_r7 ;
			m_lruPointer[regPtrPhyR8].regPhy = ARM_r8 ;
			m_lruPointer[regPtrPhyR9].regPhy = ARM_r9 ;
			m_lruPointer[regPtrPhyR12].regPhy = ARM_r12 ;
			//
			m_armVersion = 4 ;
			m_vfpVersion = 0 ;
			m_vfpNEON = false ;
			m_modeThumb = false ;
			//
			#if	defined(__DEBUG__)
			m_pCurCode = 0 ;
			#endif
		}
		// ARM 命令選択
		void SelectARMInstruction
			( int armVersion, int vfpVersion, bool vfpNEON, bool modeThumb ) ;
		// 指定バイト数分は最低限度分割されないことを保証する
		void PreserveContinuousCodes( size_t nBytes ) ;

	public:
		#if	defined(__DEBUG__)
		// デバッグ用（関数コンパイル開始時の処理）
		virtual void OnDebugBeforeFunction( DWORD dwFuncAddr, DWORD dwSize ) ;
		// デバッグ用（関数コンパイル終了時の処理）
		virtual void OnDebugAfterFunction( DWORD dwFuncAddr, DWORD dwSize ) ;
		// デバッグ用（命令コンパイル前の処理）
		virtual void OnDebugBeforeInstruction( BYTE bytInst, DWORD ip ) ;
		// デバッグ用（命令コンパイル後のテスト）
		virtual void OnDebugAfterInstruction( BYTE bytInst, DWORD ip ) ;
	
		// メモリダンプ
		void DumpMemory32
			( SSystem::SString& strDump, ulong_ptr_t pFirst, ulong_ptr_t pEnd ) ;
		void DumpARMCode
			( SSystem::SString& strDump, ulong_ptr_t pFirst, ulong_ptr_t pEnd ) ;
		#endif

	public:
		// ARM 命令出力
		// ldr reg, [reg+imm12]
		// (imm12=[-0xFFF～0xFFF], if thumb [-0xFF～0xFFF],
		//  if arm and type=={i16,i8,u16} [-0xFF～0xFF] )
		void WriteARMLoadMemOffsetImm12
			( ARMRegister regDst, ARMRegister regBase, int offsetAddr,
				ECSSakura2Processor::DataType type = ECSSakura2Processor::dataUint32 ) ;
		// str [reg+imm12], reg
		// (imm12=[-0xFFF～0xFFF], if thumb [-0xFF～0xFFF],
		//  if arm and type=={i16,u16} [-0xFF～0xFF] )
		void WriteARMStoreMemOffsetImm12
			( ARMRegister regSrc, ARMRegister regBase, int offsetAddr,
				ECSSakura2Processor::DataType type = ECSSakura2Processor::dataUint32 ) ;
		// ldrd reg, [reg+imm8]  (must be aligned)
		void WriteARMLoadDoubleMemOffsetImm8
			( ARMRegister regDst, ARMRegister regBase, int offsetAddr ) ;
		// strd [reg+imm8], reg  (must be aligned)
		void WriteARMStoreDoubleMemOffsetImm8
			( ARMRegister regSrc, ARMRegister regBase, int offsetAddr ) ;
		// ldrex reg, [reg]  (must be aligned)
		void WriteARMLoadMemEx( ARMRegister regDst, ARMRegister regBase ) ;
		// strex reg, [reg], reg  (must be aligned)
		void WriteARMStoreMemEx
			( ARMRegister regDst, ARMRegister regSrc, ARMRegister regBase ) ;
		// clrex
		void WriteARMClrEx( void ) ;
		// lea reg, context->m_regset[x]
		void WriteARMLeaSakura2Register( ARMRegister regDst, int regSakura2 ) ;
		// op[s] reg, reg, imm8 (ARM expand imm12 or thum expand imm12)
		void WriteARMOpRegRegImm12
			( uint32_t armOpCode, uint32_t thumbOpCode,
				ARMRegister regDst, ARMRegister regSrc, int imm12,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// op[s]<c> reg, reg, reg, shift
		void WriteARMOpRegRegRegShift
			( uint32_t armOpCode, uint32_t thumbOpCode,
				ARMRegister regDst, ARMRegister regSrc1,
					ARMRegister regSrc2, int nShift = 0,
					ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// op reg, reg, reg
		void WriteARMOpRegRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
				ARMRegister regDst, ARMRegister regSrc1, ARMRegister regSrc2,
					ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// and reg, reg, imm8
		void WriteARMAndRegRegImm8
			( ARMRegister regDst, ARMRegister regSrc, int imm8,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// add reg, reg, imm8
		void WriteARMAddRegRegImm8
			( ARMRegister regDst, ARMRegister regSrc, int imm8,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// adc reg, reg, imm8
		void WriteARMAdcRegRegImm8
			( ARMRegister regDst, ARMRegister regSrc, int imm8,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// macro: add reg, reg, imm
		// (use regTemp if imm >= 0x100,
		//  possibility regDst==regTemp, but must be regSrc!=regTemp)
		void WriteARMAddRegRegImm
			( ARMRegister regDst,
				ARMRegister regSrc, int imm32, ARMRegister regTemp ) ;
		// sub reg, reg, imm8
		void WriteARMSubRegRegImm8
			( ARMRegister regDst, ARMRegister regSrc, int imm8,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// sbc reg, reg, imm8
		void WriteARMSbcRegRegImm8
			( ARMRegister regDst, ARMRegister regSrc, int imm8,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// macro: sub reg, reg, imm
		// (use regTemp if imm >= 0x100,
		//  possibility regDst==regTemp, but must be regSrc!=regTemp)
		void WriteARMSubRegRegImm
			( ARMRegister regDst,
				ARMRegister regSrc, int imm32, ARMRegister regTemp ) ;
		// add reg, reg, reg, shift
		void WriteARMAddRegRegRegShift
			( ARMRegister regDst, ARMRegister regSrc1,
				ARMRegister regSrc2, int nShift = 0,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// sub reg, reg, reg, shift
		void WriteARMSubRegRegRegShift
			( ARMRegister regDst, ARMRegister regSrc1,
				ARMRegister regSrc2, int nShift = 0,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// and reg, reg, reg, shift
		void WriteARMAndRegRegRegShift
			( ARMRegister regDst, ARMRegister regSrc1,
							ARMRegister regSrc2, int nShift = 0 ) ;
		// or reg, reg, reg, shift
		void WriteARMOrRegRegRegShift
			( ARMRegister regDst, ARMRegister regSrc1,
				ARMRegister regSrc2, int nShift = 0,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// xor reg, reg, reg, shift
		void WriteARMXorRegRegRegShift
			( ARMRegister regDst, ARMRegister regSrc1,
				ARMRegister regSrc2, int nShift = 0,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// not reg, reg, shift
		void WriteARMNotRegRegShift
			( ARMRegister regDst, ARMRegister regSrc, int nShift = 0,
				ARMCondition condARM = cond_AL, bool fSetFlags = false ) ;
		// cmp reg, imm
		void WriteARMCmpRegImm8( ARMRegister regSrc, int immData ) ;
		// cmp reg, reg, shift
		void WriteARMCmpRegRegShift
			( ARMRegister regSrc1, ARMRegister regSrc2, int nShift = 0 ) ;
		// test reg, imm
		void WriteARMTestRegImm8( ARMRegister regSrc, int immData ) ;
		// shift reg, reg, imm5
		void WriteARMShiftRegRegImm
			( ARMRegister regDst, ARMRegister regSrc,
					int nShift, bool fRight, bool fArithmetic ) ;
		// lsl reg, reg, imm5
		void WriteARMShiftLeftImm
			( ARMRegister regDst, ARMRegister regSrc, int nShift ) ;
		// lsr reg, reg, imm5
		void WriteARMShiftRightImm
			( ARMRegister regDst, ARMRegister regSrc, int nShift ) ;
		// asr reg, reg, imm5
		void WriteARMShiftARightImm
			( ARMRegister regDst, ARMRegister regSrc, int nShift ) ;
		// mul reg, reg, reg
		void WriteARMMulInt32
			( ARMRegister regDst,
				ARMRegister regSrc1, ARMRegister regSrc2 ) ;
		// mla reg, reg, reg, reg
		void WriteARMMulAddInt32
			( ARMRegister regDst,
				ARMRegister regSrc1, ARMRegister regSrc2, ARMRegister regSrcAdd ) ;
		// smull reg, reg, reg, reg
		void WriteARMMulSInt64
			( ARMRegister regDst0, ARMRegister regDst1,
				ARMRegister regSrc1, ARMRegister regSrc2 ) ;
		// umull reg, reg, reg, reg
		void WriteARMMulUInt64
			( ARMRegister regDst0, ARMRegister regDst1,
				ARMRegister regSrc1, ARMRegister regSrc2 ) ;
		// macro: mov reg, imm
		void WriteARMMoveRegImm
			( ARMRegister regDst,
				int32_t immData, ARMCondition condARM = cond_AL ) ;
		// mov reg, imm32
		void * WriteARMMoveRegImm32
			( ARMRegister regDst,
				uint32_t immData, ARMCondition condARM = cond_AL ) ;
		// mov reg, reg
		void WriteARMMoveRegReg
			( ARMRegister regDst,
				ARMRegister regSrc, ARMCondition condARM = cond_AL ) ;
		// jump reg
		void WriteARMJumpReg( ARMRegister reg ) ;
		// jump imm32
		void * WriteARMJumpImm32
			( const void * pfnJmpTarget, ARMCondition cond = cond_AL ) ;
		// jump offset (-imm8～+imm8)
		void WriteARMJumpOffsetImm
			( int immPrevOffset, ARMCondition cond = cond_AL ) ;
		// macro: jump imm32
		void WriteARMJumpImm
			( const void * pfnJmpTarget, ARMCondition cond = cond_AL ) ;
		// call reg
		void WriteARMCallReg( ARMRegister reg ) ;
		// macro: call imm32
		void WriteARMCallImm( const void * pfnJmpTarget ) ;
		// push reg,...
		void WriteARMPushReg( ARMRegister reg ) ;
		void WriteARMPushRegs( const ARMRegister * regs, size_t count ) ;
		// pop reg,...
		void WriteARMPopReg( ARMRegister reg ) ;
		void WriteARMPopRegs( const ARMRegister * regs, size_t count ) ;
		// MSR APSR_nzcvq, Rn
		void WriteARMtoAPSR_nzcvq( ARMRegister reg ) ;

		// macro: clamp( v, min, max )
		// minImm8=[-0xFF～0xFF], (maxImm8-minImm8)=[0,0xFF]
		void WriteARMClampValueImm8
			( ARMRegister reg,
				int minImm8, int maxImm8, ARMRegister regTemp ) ;
		void WriteARMClampValueToSigned16
			( ARMRegister reg, ARMRegister regTemp ) ;

	public:
		// ARM SIMD 命令
		// op reg, reg, reg, imm5
		void WriteSIMDOpARMRegRegRegImm5
			( uint32_t armOpCode, uint32_t thumbOpCode,
				ARMRegister regDst,
				ARMRegister regSrc1, ARMRegister regSrc2,
				int imm5, bool fShiftFlag = false ) ;
		// op reg, reg, reg
		void WriteSIMDOpARMRegRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
				ARMRegister regDst, ARMRegister regSrc1, ARMRegister regSrc2 ) ;
		// SAT reg, imm5, reg, shift
		void WriteARMSatRegImmRegShift
			( ARMRegister regDst, int nSatBits,
					ARMRegister regSrc, int nShift, bool fUnsigned ) ;
		// SAT16 reg, imm5, reg
		void WriteARMSat16RegImmReg
			( ARMRegister regDst,
				int nSatBits, ARMRegister regSrc, bool fUnsigned ) ;
		// PKH reg, reg, reg, imm
		void WriteARMPack16RegRegRegImm
			( ARMRegister regDst,
				ARMRegister regSrc1, ARMRegister regSrc2,
				bool fShiftRight = false, int nShift = 0 ) ;
		// BFop reg, reg, lsb, width
		void WriteARMBFRegRegImmImm
			( uint32_t armOpCode, uint32_t thumbOpCode,
				ARMRegister regDst, ARMRegister regSrc, int lsb, int width ) ;
		// BFI reg, reg, lsb, width
		void WriteARMBFIRegRegImmImm
			( ARMRegister regDst, ARMRegister regSrc, int lsb, int width ) ;
		// UBFX reg, reg, lsb, width
		void WriteARMUBFXRegRegImmImm
			( ARMRegister regDst, ARMRegister regSrc, int lsb, int width ) ;
		// UXTop reg, reg, imm
		void WriteARMUXTRegRegImm
			( uint32_t armOpCode, uint32_t thumbOpCode,
				ARMRegister regDst, ARMRegister regSrc, int pos ) ;
		// UXTB reg, reg, imm
		void WriteARMUXTBRegRegImm
			( ARMRegister regDst, ARMRegister regSrc, int pos ) ;
		// UXTB16 reg, reg, imm
		void WriteARMUXTB16RegRegImm
			( ARMRegister regDst, ARMRegister regSrc, int pos ) ;
		// UXTH reg, reg, imm
		void WriteARMUXTHRegRegImm
			( ARMRegister regDst, ARMRegister regSrc, int pos ) ;

	public:
		// VFPv2 命令
		// op reg, reg, reg
		void WriteVFPOpRegRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
				int vregDst, int vregSrc1, int vregSrc2, bool fDoubleFlag ) ;
		// op reg, reg
		void WriteVFPOpRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
				int vregDst, int vregSrc, bool fDoubleFlag ) ;
		// vpush reg,...
		void WriteVFPPushReg32( int vreg, int count ) ;
		// vpop reg,...
		void WriteVFPPopReg32( int vreg, int count ) ;
		// vldr vreg, [reg]  (must be aligned)
		void WriteVFPLoad64OffsetImm8
			( int vreg, ARMRegister regAddr, int offsetAddr ) ;
		void WriteVFPLoad32OffsetImm8
			( int sreg, ARMRegister regAddr, int offsetAddr ) ;
		// macro: vldr sreg, imm32
		float32_t * WriteVFPLoadImm32( int sreg, float32_t imm32 ) ;
		// macro: vldr vreg, imm64
		int64_t * WriteVFPLoadImm64( int vreg, int64_t imm64 ) ;
		// vstr [reg], vreg  (must be aligned)
		void WriteVFPStore64OffsetImm8
			( int vreg, ARMRegister regAddr, int offsetAddr ) ;
		// vmov.f32 Sd, Sm
		void WriteMoveVFP32
			( int sregDst, int sregSrc, ARMCondition condARM = cond_AL ) ;
		// vmov.f64 Dd, Dm
		void WriteMoveVFP64
			( int vregDst, int vregSrc, ARMCondition condARM = cond_AL ) ;
		// vmov Sm, Rt
		void WriteMoveARMtoVFP32( int sreg, ARMRegister reg ) ;
		// vmov Rt, Sm
		void WriteMoveVFPtoARM32( ARMRegister reg, int sreg ) ;
		// vmov Dm, Rt, Rt2
		void WriteMoveARMtoVFP64
			( int vreg, ARMRegister regLow, ARMRegister regHigh ) ;
		// vmov Rt, Rt2, Dm
		void WriteMoveVFPtoARM64
			( ARMRegister regLow, ARMRegister regHigh, int vreg ) ;
		// vcvt.f64.f32 Dd, Sm
		void WriteCvtVFP32to64( int vreg, int sreg ) ;
		// vcvt.f32.f64 Sd, Dm
		void WriteCvtVFP64to32( int sreg, int vreg ) ;
		// vcvt.s32.f64 Sd, Dm
		void WriteCvtVFPtoInt32
			( int sreg, int vreg, bool fDoubleFloat,
				bool fUnsignedInt, bool fRoundZero ) ;
		// vcvt.f64.s32 Dd, Sm
		void WriteCvtVFPInt32toFloat
			( int vreg, int sreg, bool fDoubleFloat, bool fUnsignedInt ) ;
		// vneg.{f64|f32} Dd, Dm
		void WriteNegVFPRegReg
			( int vregDst, int vregSrc, bool fDoubleFlag ) ;
		// vcmp.{f64|f32} Dd, Dm
		void WriteCmpVFPRegReg
			( int vregDst, int vregSrc, bool fDoubleFlag ) ;
		// vmrs Rt, FPSCR
		void WriteFPSCRtoARMReg( ARMRegister regDst ) ;

	public:
		// NEON
		// op reg, reg, reg
		void WriteSIMDOpRegRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
				int vregDst, int vregSrc1, int vregSrc2, bool fQuadFlag ) ;
		// op reg, reg, reg
		void WriteSIMDIntOpRegRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
				int vregDst, int vregSrc1, int vregSrc2,
				int nLog2Size, bool fQuadFlag, bool fUnsigned = false ) ;
		// vmul reg, reg, reg
		void WriteSIMDIntMulRegRegReg
			( int vregDst, int vregSrc1, int vregSrc2,
								int nLog2Size, bool fQuadFlag ) ;
		// vmull reg, reg, reg
		void WriteSIMDIntMulLongRegRegReg
			( int qregDst, int vregSrc1, int vregSrc2,
								int nLog2Size, bool fUnsigned ) ;
		// vmvn reg, reg
		void WriteSIMDIntNotRegReg
			( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag ) ;
		// vneg reg, reg
		void WriteSIMDIntNegRegReg
			( int vregDst, int vregSrc,
				int nLog2Size, bool fFloatFlag, bool fQuadFlag ) ;
		// shift reg, reg, imm
		void WriteSIMDShiftRegRegImm
			( uint32_t armOpCode, uint32_t thumbOpCode,
				int vregDst, int vregSrc, int imm,
				int nLog2Size, bool fSigned, bool fShiftRight, bool fQuadFlag ) ;
		// qshift reg, reg, imm
		void WriteSIMDQShiftRegRegImm
			( int vregDst, int vregSrc, int imm,
				int nLog2Size, bool fDstSigned, bool fSrcSigned, bool fQuadFlag ) ;
		// vshl reg, reg, reg
		void WriteSIMDShiftRegRegReg
			( int vregDst, int vregSrc1, int vregSrc2,
				int nLog2Size, bool fSigned, bool fQuadFlag ) ;
		// vrev reg, reg
		void WriteSIMDRevRegReg
			( int vregDst, int vregSrc,
				int nLog2Size, int nLog2ElSize, bool fQuadFlag ) ;
		// vdup.s32 Dd, Rt
		void WriteDupARM32toVFP( int vreg, ARMRegister regARM, bool fQuadFlag ) ;
		// vdup reg, reg, imm
		void WriteSIMDDupRegRegImm
			( int vregDst, int vregSrc, int imm,
				int nLog2Size, bool fQuadFlag ) ;
		// vext reg, reg, reg, imm
		void WriteSIMDExtRegRegRegImm
			( int vregDst, int vregSrc1,
					int vregSrc2, int imm4, bool fQuadFlag ) ;
		// op reg, reg
		void WriteSIMDIntOpRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
				int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag ) ;
		void WriteSIMDFloatOpRegReg
			( uint32_t armOpCode, uint32_t thumbOpCode,
						int vregDst, int vregSrc, bool fQuadFlag ) ;
		// vtrn reg, reg
		void WriteSIMDTrnRegReg
			( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag ) ;
		// vzip reg, reg
		void WriteSIMDZipRegReg
			( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag ) ;
		// vuzp reg, reg
		void WriteSIMDUnZipRegReg
			( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag ) ;
		// vswp regm reg
		void WriteSIMDSwapRegReg
			( int vregDst, int vregSrc, bool fQuadFlag ) ;
		// vmov Qd, Qm
		void WriteMoveVFP128( int qregDst, int qregSrc ) ;

	public:
		// Sakura2 レジスタを物理レジスタに割り当て／ロード
		int WriteRealizeDataRegister
			( int regSakura, DataRegisterClass regClass, bool fLoad = true ) ;
		// Sakura2 レジスタを割り当て済みの物理レジスタを取得
		// 取得データ型によっては正規化
		int GetRealizedDataRegister
			( int regSakura, DataRegisterClass regClass, bool fNormalize = true ) ;
		// Sakura2 レジスタを指定 ARM レジスタにロードし、以前の割り当ては解除する
		bool RealizeFreeARMRegister
			( int regDst, int regSakura, bool fLoad, bool fFree ) ;
		// Sakura2 レジスタを指定 VFP レジスタにロードし、以前の割り当ては解除する
		bool RealizeFreeVFPRegister
			( int vregDst, int regSakura, bool fLoad, bool fFree ) ;
		// Sakura2 レジスタを指定 NEON レジスタにロードし、以前の割り当ては解除する
		bool RealizeFreeNEONRegister
			( int qregDst, int regSakura, bool fLoad, bool fFree ) ;
		// 一時処理のための物理レジスタを確保
		int AllocateDataRegister( DataRegisterClass regClass ) ;
		// レジスタの値更新フラグ設定
		void SetDataRegisterModified( DataRegisterClass regClass, int regPhy ) ;
		// 物理レジスタの変更をライトバック
		void WriteBackDataRegister
			( DataRegisterClass regClass, int regPhy, bool fKeepModified = false ) ;
		// 物理レジスタの内容をリロード
		void ReloadDataRegister( DataRegisterClass regClass, int regPhy ) ;
		// Sakura2 レジスタの割り当てをスワップ
		void SwapDataRegisterAssignation
			( DataRegisterClass regClass, int regPhy1, int regPhy2 ) ;
		// 物理レジスタの割り当てを一時的にロック
		void LockDataRegister( DataRegisterClass regClass, int regPhy ) ;
		// 物理レジスタの割り当てを解放可能にアンロック
		void UnlockDataRegister( DataRegisterClass regClass, int regPhy ) ;
		// 物理レジスタの割り当てを解放
		void FreeDataRegister( DataRegisterClass regClass, int regPhy ) ;
		// 物理レジスタの妥当性をチェック（デバッグ用）
		bool VerifyDataRegisterAssignation( void ) ;
		// デバッグ文字列出力コード生成
		void TraceDebugString
			( const char * pszText, int reg1 = ARM_r0,
					int reg2 = ARM_r1, int reg3 = ARM_r2 ) ;

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
		virtual void ResetRegister( int regSakura ) ;
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
		// Sakura2 汎用レジスタを ARM 汎用レジスタにロードするコードを出力
		virtual void WriteToLoadSakura2Register
			( ARMRegister regPhyARM, int regSakura, bool fOnlyLow = false ) ;
		virtual void WriteToLoadSakura2AddressRegister
			( ARMRegister regPhyAddr, int regBasePtr, int regIndex, int scale ) ;
		// ARM 汎用レジスタから Sakura2 汎用レジスタへストアするコードを出力
		virtual void WriteToStoreSakura2Register
			( int regSakura, ARMRegister regPhyARM, bool fOnlyLow = false ) ;
		// 仮想アドレスを実アドレスに変換するコードを出力
		virtual void * WriteToTranslateAddress
			( RealizePointerBoundary& rpb,
				ARMRegister regPhyBase, ARMRegister regPhyAddr, int slotTLB ) ;
		// メモリ境界を判定するコードを出力
		virtual void * WriteToCheckBoundaryAddress
			( RealizePointerBoundary& rpb,
				ARMRegister regPhyLow, ARMRegister regPhyTemp,
				int offsetTLB, bool fBaseOffset ) ;
		// メモリを複製するコードを出力
		virtual void WriteToCopyMemory
			( bool fDstAligned,
				ARMRegister regPhyDst, INT_PTR dispDst,
				bool fSrcAligned,
					ARMRegister regPhySrc, INT_PTR dispSrc,
				int	sizeInDWord,  ARMRegister regPhyTemp ) ;

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
//		virtual void write_div32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
//		virtual void write_idiv32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
//		virtual void write_mod32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
//		virtual void write_imod32_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
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

	protected:
		// 32ビット符号有り比較命令生成
		void write_arm_cmp_int32
			( ARMRegister regDst, ARMRegister regSrc, ARMCondition condARM ) ;
		// 32ビット浮動小数点比較移動命令生成
		void write_vfp_cmove_float32
			( int sregDst, int sregSrc, ARMCondition condARM ) ;
		// 32ビット浮動小数点比較命令生成
		void write_vfp_cmpxx_float32
			( int sregDst, int sregSrc, ARMCondition condARM ) ;
		// 64ビット浮動小数点比較命令生成
		void write_vfp_cmp_float64
			( int regDst, int regSrc, ARMCondition condARM ) ;
		// 64bit 整数比較命令生成
		//	regDst <- (regCmp1 > regCmp2) ^ fLogicalNot
		void write_arm_cmp_int64_gt
			( ARMRegister regDst, ARMRegister regTemp0,
				ARMRegister regCmp1, ARMRegister regCmp2,
				bool fLogicalNot, bool fUnsigned ) ;
		// 64bit x 64bit -> 64bit 乗算命令生成
		void write_mul_int64xint64
			( ARMRegister regDst0, ARMRegister regDst1,
				ARMRegister regSrc0, ARMRegister regSrc1,
				ARMRegister regTemp0, ARMRegister regTemp1 ) ;

	} ;

} ;

#endif
