
/*****************************************************************************
				詞葉 naked モード仮想プロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_PROCESSOR_H__)
#define	__GLSCS_SAKURA2_PROCESSOR_H__

//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script ver.3 naked mode virtual processor
//////////////////////////////////////////////////////////////////////////////

namespace	ECSSakura2
{
	class	Object ;
	class	VirtualMachine ;
} ;

namespace	ECSSakura2Processor
{
	//
	// 【命令コード】
	// ■メモリアクセス命令
	//		10000smm  xbbbbddd  [yiiiiiii]  dsreg  [offset32]
	//			  s = 0:Load / 1:Store
	//			 mm = アドレッシング・モード
	//			ddd = データ形式
	//		   bbbb = base レジスタ（r0～r15）
	//			 xy = スケーリングファクタ（x1,x2,x4,x8）
	//		iiiiiii = index レジスタ（r0～r127）
	//		  dsreg = 対象レジスタ（r0～r255）
	//
	// ■ローカルメモリアクセス
	//		100010sm  xxx00ddd  [iiiiiiii]  dsreg  offset32
	//			  s = 0:Load / 1:Store
	//			  m = 特殊アドレッシング・モード
	//			ddd = データ形式
	//			xxx = スケーリングファクタ（x1,x2,x4,x8,x16,x32,x64,x128）
	//	   iiiiiiii = index レジスタ（r0～r255）
	//		  dsreg = 対象レジスタ（r0～r255）
	//
	// ■レジスタ複製命令
	//		10010000  dstreg  srcreg
	//
	// ■整数・実数変換命令
	//		1001001x  dstreg  srcreg
	//			 x = 0:double->int64 /  1:int64->double
	//
	// ■シフト命令
	//		100101xx  dstreg  srcreg  imm8
	//			xx = 0:srl / 1:sra / 2:sll / 3:maskmove
	//
	// ■マスク合成命令
	//		10010111  dstreg  srcreg  srcreg2
	//
	// ■32ビット即値演算命令
	//		1001100x  dstreg  srcreg  imm32
	//			x = 0:add / 1:mul
	//
	// ■スタックレジスタ加算操作
	//		10011010  imm32
	//
	// ■64ビット即値読み込み命令
	//		10011011  dstreg  imm64
	//
	// ■演算命令
	//		100111xx  dstreg
	//			xx = 0:neg(int) / 1:not(int) / 2:neg(double) / 3:reserved
	//
	// ■整数演算命令
	//		1010xxxx  dstreg  srcreg
	//			xxxx = 0:add(int) / 1:sub(int) / 2:mul(int) / 3:div(int)
	//				   4:mod(int) / 5:and(int) / 6:or(int) / 7:xor(int)
	//				   8:srl(int) / 9:sra(int) / 10:sll(int)
	//
	// ■整数符号拡張命令
	//		1010xxxx  dstreg  srcreg
	//			xxxx = 11:int32->int64 / 12:int16->int64 / 13:int8->int64
	//
	// ■実数演算命令
	//		101100xx  dstreg  srcreg
	//			xx = 0:add(double) / 1:sub(double) / 2:mul(double) / 3:div(double)
	//
	// ■特殊精度整数演算命令
	//		10111xxx  dstreg  srcreg
	//			xxx = 0:mul(uint32->int64) / 1:mul(int32->int64)
	//				  2:div(uint64/uint32) / 3:div(int64/int32)
	//				  4:mod(uint64%uint32) / 5:mod(int64%int32)
	//
	// ■比較命令
	//		1100fxxx  dstreg  srcreg
	//			  f = 0:int64 / 1:double
	//			xxx = 0:dst!=src / 1:dst==src / 2:dst<src / 3:dst<=src
	//				  4:dst>src / 5:dst>=src
	//				  6:dst<src(uint64) / 7:dst<=src(uint64)
	//
	// ■相対無条件ジャンプ命令
	//		11010000  imm32
	//
	// ■間接無条件ジャンプ命令
	//		11010001  reg
	//
	// ■相対条件ジャンプ命令
	//		1101001x  reg  imm32
	//			x = 0:reg 最下位ビットが 0 のときジャンプ
	//				1:reg 最下位ビットが 1 のときジャンプ
	//
	// ■コール命令
	//		11010100  imm32
	//
	// ■間接コール命令
	//		11010101  reg
	//
	// ■システムコール命令
	//		11010110  imm32
	//
	// ■間接システムコール命令
	//		11010111  reg
	//
	// ■リターン命令
	//		11011000
	//
	// ■PUSH/POP 命令
	//		1101110p  reg
	//			p = 0:push / 1:pop
	//
	// ■PUSH/POP 命令（複数レジスタ）
	//		1101111p  reg  imm8
	//			p = 0:push / 1:pop
	//
	// ■MFENCE 命令
	//		11100000  00000000  00000000
	//
	// ■PREFETCHTLB 命令
	//		11100000  0000010x  reg
	//			x = TLB number
	//
	// ■UNFETCHTLB 命令
	//		11100000  0000100x  reg
	//			x = TLB number
	//
	// ■64bit FLOAT EXTENSION 1st byte 命令
	//		11100001  xxxxxxxx  dstreg  srcreg
	//			xxxxxxxx = 命令コード
	//				0:fabs / 1:log / 2:pow / 3:sqrt / 4:sin / 5:cos / 6:tan
	//				7:asin / 8:acos / 9:atan / 10:round / 11:floor
	//
	// ■64bit SIMD EXTENSION 2OP 1st byte 命令
	//		11100010  xxxxxxxx  dstreg  srcreg
	//
	// ■64bit SIMD EXTENSION 3OP 1st byte 命令
	//		11100011  xxxxxxxx  dstreg  srcreg  imm8
	//
	// ■128bit SIMD EXTENSION 2OP 1st byte 命令
	//		11100100  xxxxxxxx  dstreg  srcreg
	//
	// ■128bit SIMD EXTENSION 3OP 1st byte 命令
	//		11100101  xxxxxxxx  dstreg  srcreg  imm8
	//
	// ■NOP 命令
	//		11111110
	//
	//
	// 【アドレッシング・モード】
	//		00 : [base]
	//		01 : [base + offset32]
	//		10 : [base + index * scale]
	//		11 : [base + index * scale + offset32]
	//
	// 【特殊アドレッシング・モード】
	//		 0 : [bp + offset32]
	//		 1 : [bp + index * scale + offset32]
	//
	// 【データ形式】
	//		000 : int64 / double
	//		001 : int32
	//		010 : int16
	//		011 : int8
	//		100 : float  (load:float->double, store:double->float)
	//		101 : uint32 (load 専用)
	//		110 : uint16 (load 専用)
	//		111 : uint8  (load 専用)
	//
	//
	// 【64bit SIMD EXTENSION 2OP 2nd byte】
	// ■整数パックド加減算命令
	//		11100010  000x0ddd  dstreg  srcreg
	//			ddd = 0:ub / 1:sb / 2:b / 3:uw / 4:sw / 5:w / 6:d
	//			  x = 0:padd / 1:psub
	//
	// ■整数パックドシフト命令
	//		11100010  001xx00d  dstreg  srcreg
	//			 d = 0:w / 1:d
	//			xx = 0:srl / 1:sra / 2:sll / 3:reserved
	//
	// ■整数パックド比較命令
	//		11100010  01xxx0dd  dstreg  srcreg
	//			 dd = 0:sb / 1:sw / 2:sd
	//			xxx = 0:ne / 1:eq / 2:lt / 3:le / 4:gt / 5:ge
	//
	// ■整数パックド乗算命令
	//		11100010  100000dd  dstreg  srcreg
	//			dd = 0:pmul.lw / 1:pmul.hsw / 2:pmul.huw / 3:pmadd.wd
	//
	// ■整数インターリーブ命令
	//		11100010  100100dd  dstreg  srcreg
	//			dd = 0:punpack.lbw / 1:punpck.lwd / 2:punpack.ldq
	//
	// ■整数飽和変換命令
	//		11100010  101000dd  dstreg  srcreg
	//			dd = 0:pcvt.swsb / 1:pcvt.swub / 2:pcvt.sdsw
	//
	//
	// 【64bit SIMD EXTENSION 3OP 2nd byte】
	// ■整数パックドシフト命令
	//		11100011  000xx00d  dstreg  srcreg  imm8
	//			 d = 0:w / 1:d
	//			xx = 0:psrl / 1:psra / 2:psll / 3:reserved
	//
	// ■整数パックドシャッフル命令  pshuf.w
	//		11100011  00100000  dstreg  srcreg  ddccbbaa
	//			aa = dstreg 0-15bit に移動する srcreg セレクタ
	//			bb = dstreg 16-31bit に移動する srcreg セレクタ
	//			cc = dstreg 32-47bit に移動する srcreg セレクタ
	//			dd = dstreg 48-63bit に移動する srcreg セレクタ
	//
	//
	// 【128bit SIMD EXTENSION 3OP 2nd byte】
	// ■シングル演算命令
	//		11100100  0000xxxx  dstreg  srcreg
	//			xxxx = 0:fadd.32 / 1:fsub.32 / 2:fmul.32 / 3:fdiv.32
	//				  4:fsqrt.32 / 5:frcp.32 / 6:frsqrt.32 / 7:fabs.32
	//				  8:fmax.32 / 9:fmin.32
	//
	// ■ベクタ演算命令
	//		11100100  0001xxxx  dstreg  srcreg
	//			xxxx = 0:vadd.32 / 1:vsub.32 / 2:vmul.32 / 3:vdiv.32
	//				   4:vsqrt.32 / 5:vrcp.32 / 6:vrsqrt.32 / 7:vabs.32
	//				   8:vmax.32 / 9:vmin.32
	//
	// ■ベクタ比較命令  vcmp
	//		11100100  00100xxx  dstreg  srcreg
	//			xxx = 0:ne.32 / 1:eq.32 / 2:lt.32 / 3:le.32
	//				  4:gt.32 / 5:ge.32
	//
	// ■移動・ビット演算命令
	//		11100100  001010xx  dstreg  srcreg
	//			xxx = 0:vmove / 1:vand / 2:vor / 3:vxor
	//
	// ■ベクタ変換命令
	//		11100100  00110xxx  dstreg  srcreg
	//			xxx = 0:dcvt.f2i (32bit 2float -> 32bit 2int)
	//				  1:dcvt.i2f (32bit 2int -> 32bit 2float)
	//				  2:dcvt.d2f (64bit 2float -> 32bit 2float)
	//				  3:dcvt.f2d (32bit 2float -> 64bit 2float)
	//				  4:vcvt.f2w (32bit 4float -> 16bit 4int)
	//				  5:vcvt.w2f (16bit 4int -> 32bit 4float)
	//				  6:vcvt.f2i (32bit 4float -> 32bit 4int)
	//				  7:vcvt.i2f (32bit 4int -> 32bit 4float)
	//
	//
	// 【128bit SIMD EXTENSION 3OP 2nd byte】
	// ■ベクタ選択合成命令  vmaskmove
	//		11100101  00000000  dstreg  srcreg  srcreg2
	//
	// ■ベクタシャッフル命令  vshuf.32
	//		11100101  00000001  dstreg  srcreg  ddccbbaa
	//			aa = dstreg 0-31bit に移動する srcreg セレクタ
	//			bb = dstreg 32-63bit に移動する srcreg セレクタ
	//			cc = dstreg 64-95bit に移動する srcreg セレクタ
	//			dd = dstreg 96-127bit に移動する srcreg セレクタ
	//

	// 命令コード
	//////////////////////////////////////////////////////////////////////////
	enum	InstructionCode
	{
		codeInvalid			= -1,
		codeLoadMem			= 0x80,
		codeLoadMemBase		= codeLoadMem,
		codeLoadMemBaseImm32,
		codeLoadMemBaseIndex,
		codeLoadMemBaseIndexImm32,
		codeStoreMem		= 0x84,
		codeStoreMemBase	= codeStoreMem,
		codeStoreMemBaseImm32,
		codeStoreMemBaseIndex,
		codeStoreMemBaseIndexImm32,
		codeLoadLocal		= 0x88,
		codeLoadLocalImm32	= codeLoadLocal,
		codeLoadLocalIndexImm32,
		codeStoreLocal		= 0x8A,
		codeStoreLocalImm32	= codeStoreLocal,
		codeStoreLocalIndexImm32,
		codeMoveReg			= 0x90,
		codeCvtFloat2Int	= 0x92,
		codeCvtInt2Float	= 0x93,
		codeSrlImm8			= 0x94,
		codeSraImm8,
		codeSllImm8,
		codeMaskMove,
		codeAddImm32		= 0x98,
		codeMulImm32,
		codeAddSPImm32		= 0x9A,
		codeLoadImm64		= 0x9B,
		codeNegInt			= 0x9C,
		codeNotInt,
		codeNegFloat,
		codeAddReg			= 0xA0,
		codeSubReg,
		codeMulReg,
		codeDivReg,
		codeModReg,
		codeAndReg,
		codeOrReg,
		codeXorReg,
		codeSrlReg,
		codeSraReg,
		codeSllReg,
		codeMoveSx32Reg		= 0xAB,
		codeMoveSx16Reg,
		codeMoveSx8Reg,
		codeFAddReg			= 0xB0,
		codeFSubReg,
		codeFMulReg,
		codeFDivReg,
		codeMul32Reg		= 0xB8,
		codeIMul32Reg,
		codeDiv32Reg,
		codeIDiv32Reg,
		codeMod32Reg,
		codeIMod32Reg,
		codeCmpNeReg		= 0xC0,
		codeCmpEqReg,
		codeCmpLtReg,
		codeCmpLeReg,
		codeCmpGtReg,
		codeCmpGeReg,
		codeCmpCReg,
		codeCmpCZReg,
		codeFCmpNeReg		= 0xC8,
		codeFCmpEqReg,
		codeFCmpLtReg,
		codeFCmpLeReg,
		codeFCmpGtReg,
		codeFCmpGeReg,
		codeJumpOffset32	= 0xD0,
		codeJumpReg			= 0xD1,
		codeCNJumpOffset32	= 0xD2,
		codeCJumpOffset32,
		codeCallImm32		= 0xD4,
		codeCallReg			= 0xD5,
		codeSysCallImm32	= 0xD6,
		codeSysCallReg		= 0xD7,
		codeReturn			= 0xD8,
		codePushReg			= 0xDC,
		codePopReg,
		codePushRegs,
		codePopRegs,
		codeMemoryHint		= 0xE0,
		codeFloatExtension,
		codeSIMD64Extension2Op,
		codeSIMD64Extension3Op,
		codeSIMD128Extension2Op,
		codeSIMD128Extension3Op,
		codeEscape			= 0xFD,
		codeNoOperation		= 0xFE,
		codeSystemReserved	= 0xFF,
	} ;
	// MEMORY HINT EXTENSION
	enum	MemoryHintInstructionCode
	{
		mhcodeMemoryFence,
		mhcodePrefetchTLB	= 0x04,
		mhcodePrefetchTLB0	= 0x04,
		mhcodePrefetchTLB1	= 0x05,
		mhcodeUnfetchTLB	= 0x08,
		mhcodeUnfetchTLB0	= 0x08,
		mhcodeUnfetchTLB1	= 0x09,
	} ;
	// 64bit FLOAT EXTENSION 1st byte
	enum	FloatInstructionCode
	{
		fcodeFabs, fcodeLog, fcodePow, fcodeSqrt,
		fcodeSin, fcodeCos, fcodeTan,
		fcodeASin, fcodeACos, fcodeATan,
		fcodeRound, fcodeFloor,
	} ;
	// 64bit SIMD EXTENSION 2OP 2nd byte
	enum	SIMDPacked2OpInstructionCode
	{
		simdPadd	= 0x00,
		simdPaddub	= 0x00,
		simdPaddsb,
		simdPaddb,
		simdPadduw,
		simdPaddsw,
		simdPaddw,
		simdPaddd,
		simdPsub	= 0x10,
		simdPsubub	= 0x10,
		simdPsubsb,
		simdPsubb,
		simdPsubuw,
		simdPsubsw,
		simdPsubw,
		simdPsubd,
		simdPsrlw	= 0x20,
		simdPsrld,
		simdPsraw	= 0x28,
		simdPsrad,
		simdPsllw	= 0x30,
		simdPslld,
		simdPcmpnesb	= 0x40,
		simdPcmpnesw,
		simdPcmpnesd,
		simdPcmpeqsb	= 0x48,
		simdPcmpeqsw,
		simdPcmpeqsd,
		simdPcmpltsb	= 0x50,
		simdPcmpltsw,
		simdPcmpltsd,
		simdPcmplesb	= 0x58,
		simdPcmplesw,
		simdPcmplesd,
		simdPcmpgtsb	= 0x60,
		simdPcmpgtsw,
		simdPcmpgtsd,
		simdPcmpgesb	= 0x68,
		simdPcmpgesw,
		simdPcmpgesd,
		simdPmullw		= 0x80,
		simdPmulhsw,
		simdPmulhuw,
		simdPmaddwd,
		simdPunpacklbw	= 0x90,
		simdPunpacklwd,
		simdPunpackldq,
		simdPcvtswsb	= 0xA0,
		simdPcvtswub,
		simdPcvtsdsw,
	} ;
	// 64bit SIMD EXTENSION 3OP 2nd byte
	enum	SIMDPacked3OpInstructionCode
	{
		simdPsrlwImm8	= 0x00,
		simdPsrldImm8,
		simdPsrawImm8	= 0x08,
		simdPsradImm8,
		simdPsllwImm8	= 0x10,
		simdPslldImm8,
		simdPshufwImm8	= 0x20,
	} ;
	// 128bit SIMD EXTENSION 3OP 2nd byte
	enum	SIMDVector2OpInstructionCode
	{
		simdFadd	= 0x00,
		simdFsub,
		simdFmul,
		simdFdiv,
		simdFsqrt,
		simdFrcp,
		simdFrsqrt,
		simdFabs,
		simdFmax,
		simdFmin,
		simdVadd	= 0x10,
		simdVsub,
		simdVmul,
		simdVdiv,
		simdVsqrt,
		simdVrcp,
		simdVrsqrt,
		simdVabs,
		simdVmax,
		simdVmin,
		simdVcmpne	= 0x20,
		simdVcmpeq,
		simdVcmplt,
		simdVcmple,
		simdVcmpgt,
		simdVcmpge,
		simdVmove	= 0x28,
		simdVand,
		simdVor,
		simdVxor,
		simdDcvtf2i	= 0x30,
		simdDcvti2f,
		simdDcvtd2f,
		simdDcvtf2d,
		simdVcvtf2w,
		simdVcvtw2f,
		simdVcvtf2i,
		simdVcvti2f,
	} ;
	// 128bit SIMD EXTENSION 3OP 2nd byte
	enum	SIMDVector3OpInstructionCode
	{
		simdVmaskmove	= 0x00,
		simdVshuf32,
	} ;

	// アドレッシングモード
	//////////////////////////////////////////////////////////////////////////
	enum	AddressingMode
	{
		addrBase,		addrBaseOffset32,
		addrBaseIndex,	addrBaseIndexOffset32,
	} ;
	enum	LocalAddressingMode
	{
		addrLocalOffset32,	addrLocalIndexOffset32,
	} ;

	// データ形式
	//////////////////////////////////////////////////////////////////////////
	enum	DataType
	{
		dataInt64, dataInt32, dataInt16, dataInt8,
		dataFloat, dataUint32, dataUint16, dataUint8,
		dataTypeMax,
	} ;

	// レジスタ番号
	//////////////////////////////////////////////////////////////////////////
	enum	RegisterIndex
	{
		regAcc			= 0x00,
		regSP			= 0x08,		// stakc pointer
		regBP			= 0x09,		// base pointer
		regTP			= 0x0A,		// this pointer
		regXP			= 0x0D,		// exception pointer
		regYP			= 0x0E,		// destruction list pointer
		regZeroPtr		= 0x0F,		// zero base pointer
		regExpr0		= 0x10,		// expression work[0]
		regStatus0		= 0x80,		// status / control
		regException0	= 0x90,		// exception code
		regException1	= 0x91,		// throw pointer
		regException2	= 0x92,		// throw pointer destructor
		regException3	= 0x93,		// throw pointer runtime cast table
		regIntZero		= 0xF0,		// const zero value  #zero
		regIntOne		= 0xF1,		// const 1 value     #one
		regFillBit		= 0xF2,		// const -1 value    #fill
		regMaskLow32	= 0xF3,		// const 0xFFFFFFFF  #ffffffff
		regMaskLow16	= 0xF4,		// const 0xFFFF      #ffff
		regMaskLow8		= 0xF5,		// const 0xFF        #ff
		regFloatOne		= 0xF9,		// const 1.0 value   #1.0
		regFloatPI		= 0xFA,		// const π value    #pi
	} ;

	// 例外フラグ
	//////////////////////////////////////////////////////////////////////////
	enum	ExceptionFlag
	{
		// 例外
		//（このプロセッサの命令実行中に発生するもの）
		exceptionExtendStack	= 0x00000001,	// スタックを拡張する必要がある
		exceptionFarJump		= 0x00000002,	// コードセグメント切り替え
		exceptionObjectMode		= 0x00000004,	// オブジェクトモード命令
		exceptionSystemCall		= 0x00000008,	// システムコール命令
		exceptionReadMemory		= 0x00000100,	// メモリを読み込めなかった
		exceptionWriteMemory	= 0x00000200,	// メモリに書き込めなかった
		exceptionStackOverflow	= 0x00000400,	// スタックオーバーフロー
		exceptionBadInstruction	= 0x00000800,	// 不正命令
		exceptionZeroDivision	= 0x00001000,	// ゼロ除算
		exceptionMisalignment	= 0x00002000,	// 不正アライメント
		exceptionSystemError	= 0x00010000,	// システムエラー・メッセージ
		exceptionContinueMask	= 0x0000000F,
		// 割り込み
		//（別のスレッド・プロセッサからの要求で
		//　処理を中断し同期処理のために発生するもの）
		interruptAssertLock		= 0x10000000,	// 全プロセッサ同期用
		interruptChangeStatus	= 0x20000000,	// 実行ステータスが切り替った
		interruptEscape			= 0x40000000,	// 汎用割り込み（デバッグ／擬似スレッド切替等）
		interruptSystem			= 0x80000000,	// システム割り込み
		interruptMask			= 0xF0000000,
		//
		exceptionAbortMask		= ~interruptMask & ~exceptionContinueMask,
	} ;

	//////////////////////////////////////////////////////////////////////////
	// レジスタ
	//////////////////////////////////////////////////////////////////////////
	union	Register
	{
		INT64	i ;
		UINT64	ui ;
		REAL64	f ;
		struct
		{
			DWORD	l32 ;
			DWORD	h32 ;
		} ;
		struct
		{
			SDWORD	li32 ;
			SDWORD	hi32 ;
		} ;
	} ;
	union	Register128
	{
		INT64	i[2] ;
		UINT64	ui[2] ;
		REAL64	f[2] ;
		SDWORD	i32[4] ;
		DWORD	ui32[4] ;
		REAL32	f32[4] ;
	} ;

	//////////////////////////////////////////////////////////////////////////
	// リニアアドレスキャッシュ
	//////////////////////////////////////////////////////////////////////////
	struct	LinearAddressCache
	{
		DWORD	highAddress ;		// 上位32ビット
		DWORD	baseOffset ;		// 下位32ビット開始アドレス
		DWORD	limitSegment ;		// 下位32ビット有効長
		BYTE *	pbytBuffer ;		// バッファアドレス
	} ;

	//////////////////////////////////////////////////////////////////////////
	// グローバルコンテキスト
	//////////////////////////////////////////////////////////////////////////

	extern ESL_DLL_EXPORT DWORD	maskGlobalInterrupt ;	// 大域割り込みマスク
	extern ESL_DLL_EXPORT DWORD	countRuningContext ;	// 実行中のコンテキスト数

	extern ESL_DLL_EXPORT SSystem::SCriticalSection *	mutexGlobalAtomic ;	// 排他同期処理のためのオブジェクト
	extern ESL_DLL_EXPORT SSystem::SCriticalSection *	mutexQuickLock ;	// スクリプト上の QuickLock 用

	extern ESL_DLL_EXPORT SSystem::SignalEvent *	signalLeave ;		// assert lock の為に実行を中断したことを知らせる
	extern ESL_DLL_EXPORT SSystem::SignalEvent *	signalUnlocked ;	// assert lock 中は非シグナル状態
	extern ESL_DLL_EXPORT DWORD						countAssertLocked ;			// assert lock カウンタ


	//////////////////////////////////////////////////////////////////////////
	// アトミック関数（例外マスク操作用）
	//////////////////////////////////////////////////////////////////////////
	inline void AtomicOr( DWORD * pdwMask, DWORD dwBits )
	{
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
		__asm
		{
			mov	ecx, pdwMask
			mov	eax, dwBits
			lock or [ecx], eax
		}
	#else
		mutexGlobalAtomic->Lock() ;
		*pdwMask |= dwBits ;
		mutexGlobalAtomic->Unlock() ;
	#endif
	}
	inline void AtomicAnd( DWORD * pdwMask, DWORD dwBits )
	{
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
		__asm
		{
			mov	ecx, pdwMask
			mov	eax, dwBits
			lock and [ecx], eax
		}
	#else
		mutexGlobalAtomic->Lock() ;
		*pdwMask &= dwBits ;
		mutexGlobalAtomic->Unlock() ;
	#endif
	}
	inline void AtomicInc( DWORD * pdwCounter )
	{
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
		__asm
		{
			mov	ecx, pdwCounter
			lock inc DWORD PTR [ecx]
		}
	#else
		mutexGlobalAtomic->Lock() ;
		(*pdwCounter) ++ ;
		mutexGlobalAtomic->Unlock() ;
	#endif
	}
	inline void AtomicDec( DWORD * pdwCounter )
	{
	#if	defined(__PROCESSOR_INTEL_X86__) && defined(_MSC_VER)
		__asm
		{
			mov	ecx, pdwCounter
			lock dec DWORD PTR [ecx]
		}
	#else
		mutexGlobalAtomic->Lock() ;
		(*pdwCounter) -- ;
		mutexGlobalAtomic->Unlock() ;
	#endif
	}


	//////////////////////////////////////////////////////////////////////////
	// 実行コンテキスト
	//////////////////////////////////////////////////////////////////////////

	// リニアアドレス変換インターフェース
	typedef LinearAddressCache *
			(*TRANSLATE_ADDRESS_PROC)
				( struct Context * context,
						LinearAddressCache * plac, INT64 nAddress ) ;
	// ロードインターフェース
	typedef INT64 (__fastcall *LOAD_PROC)
			( struct Context * context, INT64 nAddress ) ;
	typedef INT64 (__fastcall *LOAD_MEM_PROC)( const void * ptrMem ) ;
	// ストアインターフェース
	typedef void (__fastcall *STORE_PROC)
			( struct Context * context, INT64 nAddress, INT64 nData ) ;
	typedef void (__fastcall *STORE_MEM_PROC)( void * ptrMem, INT64 nData ) ;
	// 命令実行
	typedef void (__fastcall *INSTRUCTION_PROC)( struct Context * context ) ;
	typedef void (__fastcall *OPERATION_DST_SRC_PROC)
					( Register * dst, const Register * src ) ;
	typedef void (__fastcall *OPERATION_DST_SRC_SRC2_PROC)
					( Register * dst, const Register * src, const Register * src2 ) ;
	typedef void (__fastcall *OPERATION_DST_SRC_IMM_PROC)
					( Register * dst, const Register * src, int imm ) ;
	typedef void (__fastcall *OPERATION_SIMD128_DST_SRC_PROC)
					( Register128 * dst, const Register128 * src ) ;
	typedef void (__fastcall *OPERATION_SIMD128_DST_SRC_IMM_PROC)
					( struct Context * context,
						Register128 * dst, const Register128 * src, int imm ) ;

	struct	Context
	{
		// 実行ステータス
		enum	ExecutionStatus
		{
			xsHalt,				// 実行完了／未実行
			xsExecution,		// 実行中
			xsSuspend,			// 実行一時停止
			xsPending,			// 実行一時停止（イベント待ち）
			xsInterrupt,		// 割り込み
			xsMask	= 0xFFFFFFFF,
		} ;
		enum	PendingCount
		{
			pendingOnce		= 1,
			pendingForever	= -1,
		} ;

		// 汎用レジスタ
		Register			m_regset[0x100] ;

		// 命令実行インターフェース
		INSTRUCTION_PROC	m_pfnInstruction[0x100] ;

		// メモリアクセスインターフェース
		LOAD_PROC			m_pfnLoad[dataTypeMax] ;
		STORE_PROC			m_pfnStore[dataTypeMax] ;

		// TLB
		LinearAddressCache	m_segStack ;
		LinearAddressCache	m_segLoadCache[4] ;
		LinearAddressCache	m_segStoreCache[4] ;

		// 二次 TLB
		DWORD				m_mask2ndTLB[4] ;
		LinearAddressCache	m_seg2ndCache[0x20] ;

		// 制御
		DWORD				m_ip ;				// 命令ポインタ下位
		DWORD				m_ipSegment ;		// 命令ポインタ上位
		DWORD				m_maskException ;	// 例外マスク
		DWORD				m_idSystemCall ;	// システムコール例外コード
		DWORD				m_idInteruption ;	// システム割り込み事由
		ExecutionStatus		m_status ;			// 実行ステータス
		wchar_t *			m_pszError ;		// エラーメッセージ
		int64_t				m_countPending ;
		DWORD				m_dwReserved ;

		const BYTE *		m_ptrCode ;			// コードバッファ
		const BYTE *		m_ptrTrick ;		// トリックバッファ

		TRANSLATE_ADDRESS_PROC	m_pfnTranslate ;	// リニアアドレス変換
		TRANSLATE_ADDRESS_PROC	m_pfnAtomicTranslate ;

		ECSSakura2::VirtualMachine *
								m_pSakura2VM ;		// 仮想マシン
		ECSSakura2::Object *	m_pThread ;			// 現在のスレッド
		void *					m_ptrReserved[2] ;	// 予約領域

		// 拡張バッファ（デバッガ用領域）
		DWORD_PTR				m_dwExtendInstance[0x10] ;

	public:
		// 構築関数
		Context( void )
			: m_pszError(NULL), m_pSakura2VM(NULL), m_pThread(NULL) { }
		// 消滅関数
		~Context( void ) { delete [] m_pszError ; }
		// プロセッサ初期設定
		void InitializeProcessor( INT64 initSP ) ;
		// レジスタ初期値設定
		void InitializeRegister( void ) ;
		// TLB リセット
		void ResetAddressTranslationCache( void ) ;
		// アドレス変換
		BYTE * AsyncTranslateAddress( INT64 nAddress, size_t nRange = 0 ) ;
		BYTE * AtomicTranslateAddress( INT64 nAddress, size_t nRange = 0 ) ;
		// 命令実行
		DWORD ExecuteCore( void ) ;
		// 実行ステータス変更
		ExecutionStatus ChangeExecutionStatus( ExecutionStatus status ) ;
		// ペンディング・ステータス設定
		void SetPendingStatus( int64_t countPending ) ;
		// エラーメッセージを設定
		void SetContextErrorMessage( const wchar_t * pszErrMsg ) ;

	public:
		// レジスタの値をデバッグ出力
		void TraceDumpRegister( void ) ;
		// レジスタの値をファイルに出力
		void WriteDumpRegister( SSystem::SBufferedFile& file ) ;

	public:
		// メンバフィールドのオフセット
		static size_t OffsetOfReg( int reg )
		{
			ESLAssert( (reg >= 0) && (reg < 0x100) ) ;
			return	offsetof( Context, m_regset ) + reg * sizeof(Register) ;
		}
		static size_t OffsetofStoreCache( int slotTLB )
		{
			return	offsetof( Context, m_segStoreCache[0] )
									+ slotTLB * sizeof(LinearAddressCache) ;
		}
		static size_t OffsetofStoreCache_baseOffset( int slotTLB )
		{
			return	offsetof( Context, m_segStoreCache[0].baseOffset )
									+ slotTLB * sizeof(LinearAddressCache) ;
		}
		static size_t OffsetofStoreCache_pbytBuffer( int slotTLB )
		{
			return	offsetof( Context, m_segStoreCache[0].pbytBuffer )
									+ slotTLB * sizeof(LinearAddressCache) ;
		}
		static size_t OffsetofStoreCache_highAddress( int slotTLB )
		{
			return	offsetof( Context, m_segStoreCache[0].highAddress )
									+ slotTLB * sizeof(LinearAddressCache) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 実行コンテキスト・抽象シェル
	//////////////////////////////////////////////////////////////////////////

	class	ContextShell	: public ESLObject, public Context
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ContextShell, ESLObject )

	public:
		// ExecuteCore 実行前処理
		virtual void PrepareCoreExecution( void ) ;
		// 命令実行（例外処理含む）
		virtual DWORD ExecuteKernel( void ) ;
		// 命令実行（割り込み／停止まで）
		virtual const wchar_t * ExecuteShell( void ) ;

	public:
		// 例外エラーをスクリプトにスロー
		virtual const wchar_t * ThrowException( const wchar_t * pszErrMsg ) ;
		// 一時停止処理（ループの外側で処理する場合には errAbort を返却）
		virtual SSystem::SError OnSuspendContext( void ) ;
		// 一時停止処理（イベント待ちのとき errPending を返却）
		virtual SSystem::SError OnPendingContext( void ) ;

	public:
		// 関数呼び出し
		virtual const wchar_t *
			BeginFunction
				( INT64 addrFunc, const Register *pArg, int nArgCount ) ;
		// 仮想関数開始
		virtual const wchar_t *
			BeginVirtualFunction
				( INT64 addrObj, int iVirtual,
						const Register *pArg, int nArgCount ) ;
		// スタックへプッシュ
		virtual const wchar_t *
					PushStack( const Register *pData, int nCount ) ;
		// スタックへ一時領域を確保しデータを転送
		virtual const wchar_t * PushBinaryOnStack
			( int& nPushedCount, const void * ptrBuf, size_t nBytes ) ;
		// スタックへ一時領域を確保し文字列を転送
		virtual const wchar_t * PushStringOnStack
			( int& nPushedCount, const wchar_t * pwszString, int nLength = -1 ) ;
		// スタック確保
		virtual const wchar_t * AllocateStack( Register*& pStack, int nCount ) ;
		// スタック解放
		virtual void FreeStack( int nCount ) ;

	public:	// 標準的な例外処理
		// assert lock 同期例外処理
		virtual DWORD HandleExceptionAssertLock( DWORD maskException ) ;
		// システムコール例外処理
		virtual DWORD HandleExceptionSystemCall( DWORD maskException ) ;
		// スタック拡張例外処理
		virtual DWORD HandleExceptionExtendStack( DWORD maskException ) = 0 ;
		// far jump 例外処理
		virtual DWORD HandleExceptionFarJump( DWORD maskException ) ;
		// デバッグ用例外処理
		virtual DWORD HandleExceptionEscape( DWORD maskException ) ;

	public:
		// 例外エラーメッセージを取得
		virtual const wchar_t * GetExceptionErrorMessage( DWORD maskException ) ;

	public:
		// 128 bit アライメント new
		void * operator new ( size_t nBytes ) ;
		void operator delete ( void * pObj ) ;
		static void * AllocateContext( size_t nBytes, size_t offsetContext ) ;
		static void FreeContext( void * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 初期化関数
	//////////////////////////////////////////////////////////////////////////
	// ライブラリ初期化
	void Initialize( void ) ;
	// ライブラリ終了
	void Close( void ) ;


	//////////////////////////////////////////////////////////////////////////
	// assert lock 同期
	//////////////////////////////////////////////////////////////////////////
	void AssertLock( void ) ;
	void AssertUnlock( void ) ;

	inline DWORD GetRuningContextCount( void )
	{
		return	countRuningContext ;
	}


	//////////////////////////////////////////////////////////////////////////
	// メモリアクセス関数
	//////////////////////////////////////////////////////////////////////////

	// データタイプ別サイズテーブル
	extern const int			sizeof_prim_data[dataTypeMax] ;

	// データタイプ別ロード処理
	extern const LOAD_MEM_PROC	pfnLoadMemory[dataTypeMax] ;
	extern const LOAD_PROC		pfnSakuraLoad[dataTypeMax] ;

	// データタイプ別ストア処理
	extern const STORE_MEM_PROC	pfnStoreMemory[dataTypeMax] ;
	extern const STORE_PROC		pfnSakuraStore[dataTypeMax] ;

	// ロード関数
	INT64 __fastcall mem_load_int64( const void * ptrMem ) ;
	INT64 __fastcall mem_load_int32( const void * ptrMem ) ;
	INT64 __fastcall mem_load_int16( const void * ptrMem ) ;
	INT64 __fastcall mem_load_int8( const void * ptrMem ) ;
	INT64 __fastcall mem_load_float32( const void * ptrMem ) ;
	INT64 __fastcall mem_load_uint32( const void * ptrMem ) ;
	INT64 __fastcall mem_load_uint16( const void * ptrMem ) ;
	INT64 __fastcall mem_load_uint8( const void * ptrMem ) ;

	INT64 __fastcall smem_load_int64( Context * context, INT64 nAddress ) ;
	INT64 __fastcall smem_load_int32( Context * context, INT64 nAddress ) ;
	INT64 __fastcall smem_load_int16( Context * context, INT64 nAddress ) ;
	INT64 __fastcall smem_load_int8( Context * context, INT64 nAddress ) ;
	INT64 __fastcall smem_load_float32( Context * context, INT64 nAddress ) ;
	INT64 __fastcall smem_load_uint32( Context * context, INT64 nAddress ) ;
	INT64 __fastcall smem_load_uint16( Context * context, INT64 nAddress ) ;
	INT64 __fastcall smem_load_uint8( Context * context, INT64 nAddress ) ;

	// ストア関数
	void __fastcall mem_store_int64( void * ptrMem, INT64 nData ) ;
	void __fastcall mem_store_int32( void * ptrMem, INT64 nData ) ;
	void __fastcall mem_store_int16( void * ptrMem, INT64 nData ) ;
	void __fastcall mem_store_int8( void * ptrMem, INT64 nData ) ;
	void __fastcall mem_store_float32( void * ptrMem, INT64 nData ) ;
	void __fastcall mem_store_uint32( void * ptrMem, INT64 nData ) ;
	void __fastcall mem_store_uint16( void * ptrMem, INT64 nData ) ;
	void __fastcall mem_store_uint8( void * ptrMem, INT64 nData ) ;

	void __fastcall smem_store_int64
			( Context * context, INT64 nAddress, INT64 nData ) ;
	void __fastcall smem_store_int32
			( Context * context, INT64 nAddress, INT64 nData ) ;
	void __fastcall smem_store_int16
			( Context * context, INT64 nAddress, INT64 nData ) ;
	void __fastcall smem_store_int8
			( Context * context, INT64 nAddress, INT64 nData ) ;
	void __fastcall smem_store_float32
			( Context * context, INT64 nAddress, INT64 nData ) ;


	//////////////////////////////////////////////////////////////////////////
	// アドレス変換関数
	//////////////////////////////////////////////////////////////////////////

	LinearAddressCache *
		default_translate_address
			( Context * context,
				LinearAddressCache * plac, INT64 nAddress ) ;
	LinearAddressCache *
		default_atomic_translate_address
			( Context * context,
				LinearAddressCache * plac, INT64 nAddress ) ;


	//////////////////////////////////////////////////////////////////////////
	// 命令実行関数（C++ コードベース）
	//////////////////////////////////////////////////////////////////////////

	// 命令テーブル
	extern const INSTRUCTION_PROC	pfnInstruction[0x100] ;

	// ロード命令 0x80～
	void __fastcall load_base( Context * context ) ;
	void __fastcall load_base_imm32( Context * context ) ;
	void __fastcall load_base_index( Context * context ) ;
	void __fastcall load_base_index_imm32( Context * context ) ;

	// ストア命令 0x84～
	void __fastcall store_base( Context * context ) ;
	void __fastcall store_base_imm32( Context * context ) ;
	void __fastcall store_base_index( Context * context ) ;
	void __fastcall store_base_index_imm32( Context * context ) ;

	// ローカルメモリ・ロード命令 0x88～
	void __fastcall load_local_imm32( Context * context ) ;
	void __fastcall load_local_index_imm32( Context * context ) ;

	// ローカルメモリ・ストア命令 0x8A～
	void __fastcall store_local_imm32( Context * context ) ;
	void __fastcall store_local_index_imm32( Context * context ) ;

	// ムーブ命令 0x90
	void __fastcall move_reg_reg( Context * context ) ;

	// 整数・実数変換命令 0x92～
	void __fastcall cvt_float2int( Context * context ) ;
	void __fastcall cvt_int2float( Context * context ) ;

	// シフト命令 0x94～
	void __fastcall srl_reg_reg_imm8( Context * context ) ;
	void __fastcall sra_reg_reg_imm8( Context * context ) ;
	void __fastcall sll_reg_reg_imm8( Context * context ) ;

	// マスク合成命令 0x97
	void __fastcall maskmove_reg_reg_reg( Context * context ) ;

	// 32ビット即値演算命令 0x98～
	void __fastcall add_reg_reg_imm32( Context * context ) ;
	void __fastcall mul_reg_reg_imm32( Context * context ) ;

	// スタックレジスタ加算操作 0x9A
	void __fastcall add_sp_imm32( Context * context ) ;

	// 64ビット即値読み込み命令 0x9B
	void __fastcall move_reg_imm64( Context * context ) ;

	// 演算命令 0x9C～
	void __fastcall neg_int( Context * context ) ;
	void __fastcall not_int( Context * context ) ;
	void __fastcall neg_float( Context * context ) ;

	// 整数演算命令 0xA0～
	void __fastcall add_reg_reg( Context * context ) ;
	void __fastcall sub_reg_reg( Context * context ) ;
	void __fastcall mul_reg_reg( Context * context ) ;
	void __fastcall div_reg_reg( Context * context ) ;
	void __fastcall mod_reg_reg( Context * context ) ;
	void __fastcall and_reg_reg( Context * context ) ;
	void __fastcall or_reg_reg( Context * context ) ;
	void __fastcall xor_reg_reg( Context * context ) ;
	void __fastcall srl_reg_reg( Context * context ) ;
	void __fastcall sra_reg_reg( Context * context ) ;
	void __fastcall sll_reg_reg( Context * context ) ;

	// 整数符号拡張命令 0xAB～
	void __fastcall move_sx32_reg_reg( Context * context ) ;
	void __fastcall move_sx16_reg_reg( Context * context ) ;
	void __fastcall move_sx8_reg_reg( Context * context ) ;

	// 実数演算命令 0xB0～
	void __fastcall fadd_reg_reg( Context * context ) ;
	void __fastcall fsub_reg_reg( Context * context ) ;
	void __fastcall fmul_reg_reg( Context * context ) ;
	void __fastcall fdiv_reg_reg( Context * context ) ;

	// 特殊精度整数演算命令　0xB8～
	void __fastcall mul32_reg_reg( Context * context ) ;
	void __fastcall imul32_reg_reg( Context * context ) ;
	void __fastcall div32_reg_reg( Context * context ) ;
	void __fastcall idiv32_reg_reg( Context * context ) ;
	void __fastcall mod32_reg_reg( Context * context ) ;
	void __fastcall imod32_reg_reg( Context * context ) ;

	// 整数比較命令 0xC0～
	void __fastcall cmp_ne( Context * context ) ;
	void __fastcall cmp_eq( Context * context ) ;
	void __fastcall cmp_lt( Context * context ) ;
	void __fastcall cmp_le( Context * context ) ;
	void __fastcall cmp_gt( Context * context ) ;
	void __fastcall cmp_ge( Context * context ) ;
	void __fastcall cmp_c( Context * context ) ;
	void __fastcall cmp_cz( Context * context ) ;

	// 実数比較命令 0xC8～
	void __fastcall fcmp_ne( Context * context ) ;
	void __fastcall fcmp_eq( Context * context ) ;
	void __fastcall fcmp_lt( Context * context ) ;
	void __fastcall fcmp_le( Context * context ) ;
	void __fastcall fcmp_gt( Context * context ) ;
	void __fastcall fcmp_ge( Context * context ) ;

	// 相対無条件ジャンプ命令 0xD0
	void __fastcall jump_offset32( Context * context ) ;

	// 間接無条件ジャンプ命令 0xD1
	void __fastcall jump_reg( Context * context ) ;

	// 相対条件ジャンプ命令 0xD2～
	void __fastcall cnjump_reg_offset32( Context * context ) ;
	void __fastcall cjump_reg_offset32( Context * context ) ;

	// コール命令 0xD4
	void __fastcall call_imm32( Context * context ) ;

	// 間接コール命令 0xD5
	void __fastcall call_reg( Context * context ) ;

	// システムコール命令 0xD6
	void __fastcall syscall_imm32( Context * context ) ;

	// 間接システムコール命令 0xD7
	void __fastcall syscall_reg( Context * context ) ;

	// リターン命令 0xD8
	void __fastcall ret_nop( Context * context ) ;

	// PUSH/POP 命令 0xDC～
	void __fastcall push_reg( Context * context ) ;
	void __fastcall pop_reg( Context * context ) ;
	void __fastcall pushs_reg_imm8( Context * context ) ;
	void __fastcall pops_reg_imm8( Context * context ) ;

	// MEMORY HINT 命令 0xE0
	void __fastcall memory_hint( Context * context ) ;

	// FLOAT EXTENSION 命令 0xE1
	void __fastcall float_extensition( Context * context ) ;

	// 64bit SIMD EXTENSION 2OP 命令 0xE2
	void __fastcall simd64_extensition_2op( Context * context ) ;

	// 64bit SIMD EXTENSION 3OP 命令 0xE3
	void __fastcall simd64_extensition_3op( Context * context ) ;

	// 128bit SIMD EXTENSION 2OP 命令 0xE4
	void __fastcall simd128_extensition_2op( Context * context ) ;

	// 128bit SIMD EXTENSION 3OP 命令 0xE5
	void __fastcall simd128_extensition_3op( Context * context ) ;

	// エスケープ命令 0xFD
	void __fastcall escape_interruption( Context * context ) ;

	// NOP 命令 0xFE
	void __fastcall no_operation( Context * context ) ;

	// 不正命令
	void __fastcall bad_instruction( Context * context ) ;

	// ネイティブコード化トラップ
	void __fastcall trap_native_code( Context * context ) ;

	// オブジェクト命令
	void __fastcall object_mode_instruction( Context * context ) ;


	//////////////////////////////////////////////////////////////////////////
	// 64bit 浮動小数点演算関数
	//////////////////////////////////////////////////////////////////////////

	// 命令テーブル
	extern const OPERATION_DST_SRC_PROC	pfnFloatOperationDstSrc[0x100] ;

	void __fastcall float64_abs( Register * dst, const Register * src ) ;
	void __fastcall float64_log( Register * dst, const Register * src ) ;
	void __fastcall float64_pow( Register * dst, const Register * src ) ;
	void __fastcall float64_sqrt( Register * dst, const Register * src ) ;
	void __fastcall float64_sin( Register * dst, const Register * src ) ;
	void __fastcall float64_cos( Register * dst, const Register * src ) ;
	void __fastcall float64_tan( Register * dst, const Register * src ) ;
	void __fastcall float64_asin( Register * dst, const Register * src ) ;
	void __fastcall float64_acos( Register * dst, const Register * src ) ;
	void __fastcall float64_atan( Register * dst, const Register * src ) ;
	void __fastcall float64_round( Register * dst, const Register * src ) ;
	void __fastcall float64_floor( Register * dst, const Register * src ) ;


	//////////////////////////////////////////////////////////////////////////
	// 64bit SIMD 処理関数
	//////////////////////////////////////////////////////////////////////////

	// 命令テーブル
	extern const OPERATION_DST_SRC_PROC		pfnSIMD64_OperationDstSrc[0x100] ;
	extern const OPERATION_DST_SRC_IMM_PROC	pfnSIMD64_OperationDstSrcImm8[0x100] ;

	// 加減算命令 0xE2 0x00～
	void __fastcall simd_paddub( Register * dst, const Register * src ) ;
	void __fastcall simd_paddsb( Register * dst, const Register * src ) ;
	void __fastcall simd_paddb( Register * dst, const Register * src ) ;
	void __fastcall simd_padduw( Register * dst, const Register * src ) ;
	void __fastcall simd_paddsw( Register * dst, const Register * src ) ;
	void __fastcall simd_paddw( Register * dst, const Register * src ) ;
	void __fastcall simd_paddd( Register * dst, const Register * src ) ;

	void __fastcall simd_psubub( Register * dst, const Register * src ) ;
	void __fastcall simd_psubsb( Register * dst, const Register * src ) ;
	void __fastcall simd_psubb( Register * dst, const Register * src ) ;
	void __fastcall simd_psubuw( Register * dst, const Register * src ) ;
	void __fastcall simd_psubsw( Register * dst, const Register * src ) ;
	void __fastcall simd_psubw( Register * dst, const Register * src ) ;
	void __fastcall simd_psubd( Register * dst, const Register * src ) ;

	// シフト命令 0xE2 0x20～
	void __fastcall simd_psrlw( Register * dst, const Register * src ) ;
	void __fastcall simd_psraw( Register * dst, const Register * src ) ;
	void __fastcall simd_psllw( Register * dst, const Register * src ) ;

	void __fastcall simd_psrld( Register * dst, const Register * src ) ;
	void __fastcall simd_psrad( Register * dst, const Register * src ) ;
	void __fastcall simd_pslld( Register * dst, const Register * src ) ;

	// 比較命令 0xE2 0x40～
	void __fastcall simd_pcmp_ne_sb( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_eq_sb( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_lt_sb( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_le_sb( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_gt_sb( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_ge_sb( Register * dst, const Register * src ) ;

	void __fastcall simd_pcmp_ne_sw( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_eq_sw( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_lt_sw( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_le_sw( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_gt_sw( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_ge_sw( Register * dst, const Register * src ) ;

	void __fastcall simd_pcmp_ne_sd( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_eq_sd( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_lt_sd( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_le_sd( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_gt_sd( Register * dst, const Register * src ) ;
	void __fastcall simd_pcmp_ge_sd( Register * dst, const Register * src ) ;

	// 乗算命令 0xE2 0x80～
	void __fastcall simd_pmullw( Register * dst, const Register * src ) ;
	void __fastcall simd_pmulhsw( Register * dst, const Register * src ) ;
	void __fastcall simd_pmulhusw( Register * dst, const Register * src ) ;
	void __fastcall simd_pmaddwd( Register * dst, const Register * src ) ;

	// インターリーブ命令 0xE2 0x90～
	void __fastcall simd_punpack_lbw( Register * dst, const Register * src ) ;
	void __fastcall simd_punpack_lwd( Register * dst, const Register * src ) ;
	void __fastcall simd_punpack_ldq( Register * dst, const Register * src ) ;

	// 飽和変換命令 0xE2 0xA0～
	void __fastcall simd_pcvt_swb( Register * dst, const Register * src ) ;
	void __fastcall simd_pcvt_uswb( Register * dst, const Register * src ) ;
	void __fastcall simd_pcvt_sdw( Register * dst, const Register * src ) ;

	// 即値シフト命令 0xE3 0x00～
	void __fastcall simd_psrlw_imm8( Register * dst, const Register * src, int imm8 ) ;
	void __fastcall simd_psraw_imm8( Register * dst, const Register * src, int imm8 ) ;
	void __fastcall simd_psllw_imm8( Register * dst, const Register * src, int imm8 ) ;

	void __fastcall simd_psrld_imm8( Register * dst, const Register * src, int imm8 ) ;
	void __fastcall simd_psrad_imm8( Register * dst, const Register * src, int imm8 ) ;
	void __fastcall simd_pslld_imm8( Register * dst, const Register * src, int imm8 ) ;

	// シャッフル命令 0xE3 0x20～
	void __fastcall simd_pshufw_imm8( Register * dst, const Register * src, int imm8 ) ;


	//////////////////////////////////////////////////////////////////////////
	// 128bit SIMD 処理関数
	//////////////////////////////////////////////////////////////////////////

	// 命令テーブル
	extern const OPERATION_SIMD128_DST_SRC_PROC
							pfnSIMD128_OperationDstSrc[0x100] ;
	extern const OPERATION_SIMD128_DST_SRC_IMM_PROC
							pfnSIMD128_OperationDstSrcImm8[0x100] ;

	// シングル演算命令 0xE4 0x00～
	void __fastcall simd_fadd_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_fsub_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_fmul_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_fdiv_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_fsqrt_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_frcp_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_frsqrt_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_fabs_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_fmax_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_fmin_32( Register128 * dst, const Register128 * src ) ;

	// ベクタ演算命令 0xE4 0x10～
	void __fastcall simd_vadd_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vsub_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vmul_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vdiv_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vsqrt_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vrcp_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vrsqrt_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vabs_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vmax_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vmin_32( Register128 * dst, const Register128 * src ) ;

	// ベクタ比較命令 0xE4 0x20～
	void __fastcall simd_vcmp_ne_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcmp_eq_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcmp_lt_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcmp_le_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcmp_gt_32( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcmp_ge_32( Register128 * dst, const Register128 * src ) ;

	// 移動・ビット演算命令 0xE4 0x28～
	void __fastcall simd_vmove( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vand( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vor( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vxor( Register128 * dst, const Register128 * src ) ;

	// ベクタ変換命令 0xE4 0x30～
	void __fastcall simd_dcvt_f2i( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_dcvt_i2f( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_dcvt_d2f( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_dcvt_f2d( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcvt_f2w( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcvt_w2f( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcvt_f2i( Register128 * dst, const Register128 * src ) ;
	void __fastcall simd_vcvt_i2f( Register128 * dst, const Register128 * src ) ;

	// ベクタ選択合成命令 0xE5 0x00
	void __fastcall simd_vmaskmove
		( Context * context, Register128 * dst, const Register128 * src, int imm8 ) ;

	// ベクタシャッフル命令 0xE5 0x01
	void __fastcall simd_vshuf32
		( Context * context, Register128 * dst, const Register128 * src, int imm8 ) ;


	//////////////////////////////////////////////////////////////////////////
	// 命令コード情報
	//////////////////////////////////////////////////////////////////////////
	struct	InstructionInfo
	{
		int		nFlags ;			// フラグ (in & out)
		int		nType ;				// 命令種別
		int		nBytes ;			// 命令バイト数
		int		regSrc1 ;			// 入力レジスタ
		int		regSrc2 ;
		int		regSrc3 ;
		int		regDst ;			// 出力レジスタ
	} ;
	struct	MnemonicInfo	: public InstructionInfo
	{
		int		nReserved ;
		char	szMnemonic[0x20] ;	// ニーモニック命令部 <inst>[.<opt>]
		char	szOperand[0x40] ;	// オペランド部 [<op1>[,<op2>[,<op3>]]]

		void AddMnemonic( const char * pszText ) ;
		void AddOperand( const char * pszText ) ;
		void AddOperandRegister( int regNum ) ;
		void AddOperandImmediate8( int imm8, bool fWithSign ) ;
		void AddOperandImmediate32( int imm32, bool fWithSign ) ;
		void AddOperandImmediate64( INT64 imm64 ) ;
	} ;
	enum	InstructionFlag
	{
		flagSourceRegister1				= 0x00000001,	// regSrc1 == -1
		flagSourceRegister2				= 0x00000002,	// regSrc2 == -1
		flagSourceRegister3				= 0x00000004,	// regSrc3 == -1
		flagDestinationRegister2		= 0x00000008,	// regDst == -1
		flagComplexSourceRegister		= 0x00000010,
		flagComplexDestinationRegister	= 0x00000020,
		flagMnemonic					= 0x00010000,	// MnemonicInfo
	} ;
	enum	InstructionType
	{
		typeLoadMem,
		typeStoreMem,
		typeLoadImm64,
		typeMoveReg,
		typeCvtMoveReg,
		typeOpRegRegImm,
		typeOpRegRegReg,
		typeOpRegReg,
		typeOpReg,
		typeJump,
		typeComplex,
		typeObject,
	} ;

	// ニーモニック
	extern const char * pszInstructionMnemonic[0x100] ;
	extern const char * pszMemoryHintExtensionMnemonic[0x100] ;
	extern const char * pszFloatExtensionMnemonic[0x100] ;
	extern const char * pszSIMD64Extension2OpMnemonic[0x100] ;
	extern const char * pszSIMD64Extension3OpMnemonic[0x100] ;
	extern const char * pszSIMD128Extension2OpMnemonic[0x100] ;
	extern const char * pszSIMD128Extension3OpMnemonic[0x100] ;

	// データタイプ
	extern const char * pszInstructionDataType[dataTypeMax] ;

	// 命令コード情報生成関数テーブル
	typedef void (*INSTRUCTION_INFO_PROC)
			( InstructionInfo * inf, const BYTE * pbytCode ) ;
	extern const INSTRUCTION_INFO_PROC	pfnInstructionInfo[0x100] ;

	// レジスタ名取得
	const char * GetRegisterName( char * buf, int limit, int reg ) ;

	// 命令情報取得
	void GetInstructionInfo( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// ロード命令 0x80～
	void info_load_base( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_load_base_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_load_base_index( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_load_base_index_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// ストア命令 0x84～
	void info_store_base( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_store_base_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_store_base_index( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_store_base_index_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// ローカルメモリ・ロード命令 0x88～
	void info_load_local_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_load_local_index_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// ローカルメモリ・ストア命令 0x8A～
	void info_store_local_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_store_local_index_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// ノーオペランド形式命令
	void info_no_operand( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_return( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// 1オペランド形式命令
	void info_operand_srcreg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_operand_dstreg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_push_srcreg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_pop_dstreg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_jump_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_jump_reg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_operand_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// 2オペランド形式命令
	void info_cmp_reg_reg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_move_reg_imm64( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_move_reg_reg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_cvt_move_reg_reg( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_jump_reg_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_pushs_reg_imm8( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_pops_reg_imm8( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_operand_reg_reg( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// 3オペランド形式命令
	void info_operand_reg_reg_imm8( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_operand_reg_reg_imm32( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_operand_reg_reg_reg( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// memory hint 命令
	void info_memory_hint( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// float extension 命令
	void info_float_extension( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// SIMD extension 命令
	void info_simd64_extension_2op( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_simd64_extension_3op( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_simd128_extension_2op( InstructionInfo * inf, const BYTE * pbytCode ) ;
	void info_simd128_extension_3op( InstructionInfo * inf, const BYTE * pbytCode ) ;

	// 不正命令
	void info_bad_instruction( InstructionInfo * inf, const BYTE * pbytCode ) ;


	//////////////////////////////////////////////////////////////////////////
	// 標準関数（アーキテクチャ非依存）
	//////////////////////////////////////////////////////////////////////////
	// システムコール実行関数
	typedef const wchar_t * (*PROC_SYSCALL)
			( Context * context, const Register * pArg ) ;

	// システムコール定義
	struct	SYSCALL_ENTRY
	{
		const char *	pszFuncName ;
		PROC_SYSCALL	pfnSysCall ;
	} ;
	extern ESL_DLL_EXPORT	SYSCALL_ENTRY	entrySysCall[0x100] ;

	// システム関数検索
	PROC_SYSCALL GetSystemCallProc( const wchar_t * pwszFuncName ) ;

	// メモリ処理
	const wchar_t * syscall_memmove
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_memset
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_shared_malloc
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_malloc
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_realloc
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_free
			( Context * context, const Register * pArg ) ;

	// 文字列書式化
	const wchar_t * syscall_sprintf_s
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_vsprintf_s
			( Context * context, const Register * pArg ) ;

	// オブジェクト生成・消滅
	const wchar_t * syscall_object_new
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_object_shared_new
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_object_delete
			( Context * context, const Register * pArg ) ;

	// 例外
	const wchar_t * syscall_throw_exception
			( Context * context, const Register * pArg ) ;

	// 算術関数
	const wchar_t * syscall_fabs
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_log
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_log10
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_pow
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_sqrt
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_sin
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_cos
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_tan
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_asin
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_acos
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_atan
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_atan2
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_round
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_floor
			( Context * context, const Register * pArg ) ;

	// アトミック処理
	const wchar_t * syscall_SSystem_AtomicXchg
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_AtomicAdd
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_AtomicSub
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_AtomicOr
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_AtomicAnd
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_AtomicXor
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_LockSystem
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_UnlockSystem
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_UnlockAllSystem
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_QuickLock
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_QuickUnlock
			( Context * context, const Register * pArg ) ;

	// ペンディング処理
	const wchar_t * syscall_SSystem_SleepFrame
			( Context * context, const Register * pArg ) ;

	// メモリ情報
	const wchar_t * syscall_SSystem_GetMemoryStatus
			( Context * context, const Register * pArg ) ;

	// メモリ・アロケーション・モード
	const wchar_t * syscall_SSystem_SetMemoryAllocationMode
			( Context * context, const Register * pArg ) ;

	// タイマ・時刻
	const wchar_t * syscall_SSystem_CurrentMilliSec
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_GetPerformanceCounter
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_GetPerformanceFrequency
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_SleepMilliSec
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_CurrentLocalDate
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_DifferenceInLocalTime
			( Context * context, const Register * pArg ) ;

	// プラットフォーム情報
	const wchar_t * syscall_SSystem_GetPlatformInformation
			( Context * context, const Register * pArg ) ;

	// 実行プロセッサ情報
	const wchar_t * syscall_SSystem_GetCPUFamily
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_GetCPUFeatures
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_GetLogicalProcessorCount
			( Context * context, const Register * pArg ) ;

	// システム（モジュール・エクスポート関数）
	const wchar_t * syscall_SSystem_GetModuleExportFunction
			( Context * context, const Register * pArg ) ;

	// デバッグ用関数
	const wchar_t * syscall_SSystem_Trace
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_Assert
			( Context * context, const Register * pArg ) ;
	const wchar_t * syscall_SSystem_MessageBox
			( Context * context, const Register * pArg ) ;

} ;

// 関数記述支援マクロ
#define	ECS_LIB_DECLARE_EXPORT_SYSCALL(func)	\
	ECS_LIB_EXPORT const wchar_t * ecs_nakedcall_##func	\
		( ECSSakura2Processor::Context * context,	\
			const ECSSakura2Processor::Register * arg )
#define	ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(func,context,arg)	\
	ECS_LIB_EXPORT const wchar_t * ecs_nakedcall_##func	\
		( ECSSakura2Processor::Context * context,	\
			const ECSSakura2Processor::Register * arg )
#define	ECS_DECLARE_EXPORT_SYSCALL(func)	\
	ECS_LIB_EXPORT const wchar_t * ecs_nakedcall_##func	\
		( ECSSakura2Processor::Context * context,	\
			const ECSSakura2Processor::Register * arg )
#define	ECS_IMPLEMENT_EXPORT_SYSCALL(func,context,arg)	\
	ECS_LIB_EXPORT const wchar_t * ecs_nakedcall_##func	\
		( ECSSakura2Processor::Context * context,	\
			const ECSSakura2Processor::Register * arg )
#define	ECS_DECLARE_SYSCALL_VM(context,vm)	\
		VirtualMachine *	vm = context->m_pSakura2VM ;	\
		ESLAssert( vm != NULL )
#define	ECS_DECLARE_SYSCALL_OBJECT(vm,type,var,ptr,func)	\
		type* var = ESLTypeCast<type>(vm->AtomicObjectFromAddress((DWORD)((ptr)>>32))) ;	\
		if ( var == NULL )	return	L"invalid object pointer at " L###func
#define	ECS_DECLARE_SYSCALL_THIS(vm,type,var,arg,func)	\
		type* var = ESLTypeCast<type>(vm->AtomicObjectFromAddress(arg[0].h32)) ;	\
		if ( var == NULL )	return	L"invalid this pointer at " L###func
#define	ECS_DECLARE_SYSCALL_VM_THIS(context,vm,type,var,arg,func)	\
		ECS_DECLARE_SYSCALL_VM(context,vm) ;	\
		ECS_DECLARE_SYSCALL_THIS(vm,type,var,arg,func)
#define	ECS_DECLARE_SYSCALL_ARRAYVAR(context,type,var,ptr,count,msg)	\
		type* var = (type*) context->AtomicTranslateAddress(ptr,sizeof(type)*((size_t)(count))) ;	\
		if ( (ptr != 0) && (var == NULL) && (count != 0) )	return	L"invalid pointer for " L###msg
#define	ECS_DECLARE_SYSCALL_PTRVAR(context,type,var,ptr,msg)	\
		ECS_DECLARE_SYSCALL_ARRAYVAR(context,type,var,ptr,1,msg)


#endif

