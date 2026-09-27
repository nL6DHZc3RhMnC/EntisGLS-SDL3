
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <stdio.h>
#include <sakura/ssys_module.h>

using	namespace ECSSakura2Processor ;

//////////////////////////////////////////////////////////////////////////////
// 命令コード情報
//////////////////////////////////////////////////////////////////////////////

const char * ECSSakura2Processor::pszInstructionMnemonic[0x100] =
{
	// 0x00
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x10
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	"load", "load", "load", "load",
	"store", "store", "store", "store",
	// 0x88
	"load", "load", "store", "store",
	NULL, NULL, NULL, NULL,
	// 0x90
	"move", NULL, "cvt.f2i", "cvt.i2f",
	// 0x94
	"srl", "sra", "sll", "maskmove",
	// 0x98
	"add", "mul", "add.sp", "move",
	// 0x9C
	"neg", "not", "fneg", NULL,
	// 0xA0
	"add", "sub", "mul", "div",
	"mod", "and", "or", "xor",
	"srl", "sra", "sll", "mov.sx.dq",
	"mov.sx.wq", "mov.sx.bq", NULL, NULL,
	// 0xB0
	"fadd", "fsub", "fmul", "fdiv",
	NULL, NULL, NULL, NULL,
	"mul.dq", "imul.dq", "div.qd", "idiv.qd",
	"mod.qd", "imod.qd", NULL, NULL,
	// 0xC0
	"cmp.ne", "cmp.eq", "cmp.lt", "cmp.le",
	"cmp.gt", "cmp.ge", "cmp.c", "cmp.cz",
	// 0xC8
	"fcmp.ne", "fcmp.eq", "fcmp.lt", "fcmp.le",
	"fcmp.gt", "fcmp.ge", NULL, NULL,
	// 0xD0
	"jump", "jump", "cnjump", "cjump",
	"call", "call", "syscall", "syscall",
	// 0xD8
	"ret", NULL, NULL, NULL,
	// 0xDC
	"push", "pop", "push", "pop",
	// 0xE0
	"", "", "", "",
	"", "", NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, "nop", NULL,
} ;

const char * ECSSakura2Processor::pszMemoryHintExtensionMnemonic[0x100] =
{
	// 0x00
	"mfence", NULL, NULL, NULL,
	"prefetch.tlb0", "prefetch.tlb1", NULL, NULL,
	"unfetch.tlb0", "unfetch.tlb1", NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x10
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

const char * ECSSakura2Processor::pszFloatExtensionMnemonic[0x100] =
{
	// 0x00
	"fabs", "flog", "fpow", "fsqrt", "fsin", "fcos", "ftan", "fasin",
	"facos", "fatan", "fround", "ffloor", NULL, NULL, NULL, NULL,
	// 0x10
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

const char * ECSSakura2Processor::pszSIMD64Extension2OpMnemonic[0x100] =
{
	// 0x00
	"padd.ub", "padd.sb", "padd.b", "padd.uw",
	"padd.sw", "padd.w", "padd.d", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x10
	"psub.ub", "psub.sb", "psub.b", "psub.uw",
	"psub.sw", "psub.w", "psub.d", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	"psrl.w", "psrl.d", NULL, NULL,
	NULL, NULL, NULL, NULL,
	"psra.w", "psra.d", NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x30
	"psll.w", "psll.d", NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	"pcmp.ne.sb", "pcmp.ne.sw", "pcmp.ne.sd", NULL,
	NULL, NULL, NULL, NULL,
	"pcmp.eq.sb", "pcmp.eq.sw", "pcmp.eq.sd", NULL,
	NULL, NULL, NULL, NULL,
	// 0x50
	"pcmp.lt.sb", "pcmp.lt.sw", "pcmp.lt.sd", NULL,
	NULL, NULL, NULL, NULL,
	"pcmp.le.sb", "pcmp.le.sw", "pcmp.le.sd", NULL,
	NULL, NULL, NULL, NULL,
	// 0x60
	"pcmp.gt.sb", "pcmp.gt.sw", "pcmp.gt.sd", NULL,
	NULL, NULL, NULL, NULL,
	"pcmp.ge.sb", "pcmp.ge.sw", "pcmp.ge.sd", NULL,
	NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	"pmul.lw", "pmul.hsw", "pmul.husw", "pmadd.wd",
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x90
	"punpack.lbw", "punpack.lwd", "punpack.ldq", NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0xA0
	"pcvt.swb", "pcvt.uswb", "pcvt.sdw", NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

const char * ECSSakura2Processor::pszSIMD64Extension3OpMnemonic[0x100] =
{
	// 0x00
	"psrl.w", "psrl.d", NULL, NULL,
	NULL, NULL, NULL, NULL,
	"psra.w", "psra.d", NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x10
	"psll.w", "psll.d", NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x20
	"pshuf.w", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

const char * ECSSakura2Processor::pszSIMD128Extension2OpMnemonic[0x100] =
{
	// 0x00
	"fadd.32", "fsub.32", "fmul.32", "fdiv.32",
	"fsqrt.32", "frcp.32", "frsqrt.32", "fabs.32",
	"fmax.32", "fmin.32", NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x10
	"vadd.32", "vsub.32", "vmul.32", "vdiv.32",
	"vsqrt.32", "vrcp.32", "vrsqrt.32", "vabs.32",
	"vmax.32", "vmin.32", NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x20
	"vcmp.ne.32", "vcmp.eq.32", "vcmp.lt.32", "vcmp.le.32",
	"vcmp.gt.32", "vcmp.ge.32", NULL, NULL,
	"vmove", "vand", "vor", "vxor",
	NULL, NULL, NULL, NULL,
	// 0x30
	"dcvt.f2i", "dcvt.i2f", "dcvt.d2f", "dcvt.f2d",
	"vcvt.f2w", "vcvt.w2f", "vcvt.f2i", "vcvt.i2f",
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;

const char * ECSSakura2Processor::pszSIMD128Extension3OpMnemonic[0x100] =
{
	// 0x00
	"vmaskmove", "vshuf.32", NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x10
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x20
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x30
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x40
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x50
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x60
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x70
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x80
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0x90
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xA0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xB0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xC0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xE0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	// 0xF0
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
} ;


const char * ECSSakura2Processor::pszInstructionDataType[dataTypeMax] =
{
	"64", "int32", "int16", "int8",
	"float", "uint32", "uint16", "uint8",
} ;


#define	I_NULL	info_bad_instruction

const INSTRUCTION_INFO_PROC	ECSSakura2Processor::pfnInstructionInfo[0x100] =
{
	// 0x00
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x10
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x20
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x30
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x40
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x50
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x60
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x70
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x80
	info_load_base, info_load_base_imm32, info_load_base_index, info_load_base_index_imm32,
	// 0x84
	info_store_base, info_store_base_imm32, info_store_base_index, info_store_base_index_imm32,
	// 0x88
	info_load_local_imm32, info_load_local_index_imm32,
	info_store_local_imm32, info_store_local_index_imm32,
	I_NULL, I_NULL, I_NULL, I_NULL,
	// 0x90
	info_move_reg_reg, I_NULL, info_cvt_move_reg_reg, info_cvt_move_reg_reg,
	// 0x94
	info_operand_reg_reg_imm8, info_operand_reg_reg_imm8, info_operand_reg_reg_imm8, info_operand_reg_reg_reg,
	// 0x98
	info_operand_reg_reg_imm32, info_operand_reg_reg_imm32, info_operand_imm32, info_move_reg_imm64,
	// 0x9C
	info_operand_dstreg, info_operand_dstreg, info_operand_dstreg, I_NULL,
	// 0xA0
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg,
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg,
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_cvt_move_reg_reg,
	info_cvt_move_reg_reg, info_cvt_move_reg_reg, I_NULL, I_NULL,
	// 0xB0
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg,
	I_NULL, I_NULL, I_NULL, I_NULL,
	// 0xB8
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg,
	info_operand_reg_reg, info_operand_reg_reg, I_NULL, I_NULL,
	// 0xC0
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg,
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg,
	// 0xC8
	info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg, info_operand_reg_reg,
	info_operand_reg_reg, info_operand_reg_reg, I_NULL, I_NULL,
	// 0xD0
	info_jump_imm32, info_jump_reg, info_jump_reg_imm32, info_jump_reg_imm32,
	info_jump_imm32, info_jump_reg, info_jump_imm32, info_jump_reg,
	// 0xD8
	info_return, I_NULL, I_NULL, I_NULL,
	// 0xDC
	info_push_srcreg, info_pop_dstreg, info_pushs_reg_imm8, info_pops_reg_imm8,
	// 0xE0
	info_memory_hint, info_float_extension, info_simd64_extension_2op, info_simd64_extension_3op,
	info_simd128_extension_2op, info_simd128_extension_3op, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	// 0xF0
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, I_NULL, I_NULL,
	I_NULL, I_NULL, info_no_operand, I_NULL,
} ;


// 文字列結合
//////////////////////////////////////////////////////////////////////////////
static void StringAdd( char * pszDst, int nLimit, const char * pszSrc )
{
	if ( (pszDst == NULL) || (pszSrc == NULL) )
	{
		return ;
	}
	const int	nEnd = nLimit - 1 ;
	int	i = 0 ;
	for ( ; i < nEnd; i ++ )
	{
		if ( pszDst[i] == '\0' )
		{
			break ;
		}
	}
	for ( int j = 0; i < nEnd; i ++, j ++ )
	{
		char	s = pszSrc[j] ;
		if ( s == '\0' )
		{
			break ;
		}
		pszDst[i] = s ;
	}
	pszDst[i] = '\0' ;
}

void ECSSakura2Processor::MnemonicInfo::AddMnemonic( const char * pszText )
{
	StringAdd( szMnemonic, sizeof(szMnemonic), pszText ) ;
}

void ECSSakura2Processor::MnemonicInfo::AddOperand( const char * pszText )
{
	StringAdd( szOperand, sizeof(szOperand), pszText ) ;
}

void ECSSakura2Processor::MnemonicInfo::AddOperandRegister( int regNum )
{
	char	szBuf[0x20] ;
	AddOperand( GetRegisterName( szBuf, sizeof(szBuf), regNum ) ) ;
}

void ECSSakura2Processor::MnemonicInfo::AddOperandImmediate8( int imm8, bool fWithSign )
{
	char	szBuf[0x20] ;
	imm8 = (SBYTE) imm8 ;
	if ( fWithSign && (imm8 >= 0) )
	{
		#if	_MSC_VER >= 1400
			sprintf_s( szBuf, sizeof(szBuf), "+%d", imm8 ) ;
		#else
			sprintf( szBuf, "+%d", imm8 ) ;
		#endif
	}
	else
	{
		#if	_MSC_VER >= 1400
			sprintf_s( szBuf, sizeof(szBuf), "%d", imm8 ) ;
		#else
			sprintf( szBuf, "%d", imm8 ) ;
		#endif
	}
	AddOperand( szBuf ) ;
}

void ECSSakura2Processor::MnemonicInfo::AddOperandImmediate32( int imm32, bool fWithSign )
{
	char	szBuf[0x20] ;
	if ( fWithSign && (imm32 >= 0) )
	{
		#if	_MSC_VER >= 1400
			sprintf_s( szBuf, sizeof(szBuf), "+%d", imm32 ) ;
		#else
			sprintf( szBuf, "+%d", imm32 ) ;
		#endif
	}
	else
	{
		#if	_MSC_VER >= 1400
			sprintf_s( szBuf, sizeof(szBuf), "%d", imm32 ) ;
		#else
			sprintf( szBuf, "%d", imm32 ) ;
		#endif
	}
	AddOperand( szBuf ) ;
}

void ECSSakura2Processor::MnemonicInfo::AddOperandImmediate64( INT64 imm64 )
{
	char	szBuf[0x40] ;
	#if	_MSC_VER >= 1400
		sprintf_s
			( szBuf, sizeof(szBuf),
				"0x%08X%08X", (DWORD) (imm64 >> 32), (DWORD) imm64 ) ;
	#else
		sprintf
			( szBuf, "0x%08X%08X",
				(unsigned int) ((DWORD) (imm64 >> 32)),
				(unsigned int) ((DWORD) imm64) ) ;
	#endif
	AddOperand( szBuf ) ;
}


// InstructionInfo 構造体初期化
//////////////////////////////////////////////////////////////////////////////
static MnemonicInfo * InitializeInstructionInfo
					( InstructionInfo * inf, const BYTE * pbytCode )
{
	inf->nBytes = 1 ;
	inf->nType = typeComplex ;
	inf->regSrc1 = -1 ;
	inf->regSrc2 = -1 ;
	inf->regSrc3 = -1 ;
	inf->regDst = -1 ;
	//
	MnemonicInfo *	pmi = NULL ;
	if ( inf->nFlags & flagMnemonic )
	{
		pmi = (MnemonicInfo*) inf ;
		pmi->nReserved = 0 ;
		eslFillMemory( pmi->szMnemonic, 0, sizeof(pmi->szMnemonic) ) ;
		eslFillMemory( pmi->szOperand, 0, sizeof(pmi->szOperand) ) ;
		//
		StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic),
						pszInstructionMnemonic[pbytCode[0]] ) ;
	}
	inf->nFlags &= ~flagMnemonic ;
	return	pmi ;
}

// レジスタ名取得
//////////////////////////////////////////////////////////////////////////////
const char * ECSSakura2Processor::GetRegisterName( char * buf, int limit, int reg )
{
	switch ( reg )
	{
	case	regAcc:
		return	"acc" ;
	case	regSP:
		return	"sp" ;
	case	regBP:
		return	"bp" ;
	case	regTP:
		return	"tp" ;
	case	regXP:
		return	"xp" ;
	case	regYP:
		return	"yp" ;
	case	regZeroPtr:
		return	"zp" ;
	case	regIntZero:
		return	"#zero" ;
	case	regIntOne:
		return	"#one" ;
	case	regFillBit:
		return	"#fill" ;
	case	regMaskLow32:
		return	"#ffffffff" ;
	case	regMaskLow16:
		return	"#ffff" ;
	case	regMaskLow8:
		return	"#ff" ;
	case	regFloatOne:
		return	"#1.0" ;
	case	regFloatPI:
		return	"#pi" ;
	}
	char	szName[0x20] ;
	#if	_MSC_VER >= 1400
		sprintf_s( szName, sizeof(szName), "r%d", reg ) ;
	#else
		sprintf( szName, "r%d", reg ) ;
	#endif
	//
	buf[0] = 0 ;
	StringAdd( buf, limit, szName ) ;
	//
	return	buf ;
}

// 命令情報取得
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2Processor::GetInstructionInfo
		( InstructionInfo * inf, const BYTE * pbytCode )
{
	(pfnInstructionInfo[pbytCode[0]])( inf, pbytCode ) ;
}

// ロード命令 0x80～
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2Processor::info_load_base( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 3 ;
	inf->nType = typeLoadMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regDst = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( ",[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "]" ) ;
	}
}

void ECSSakura2Processor::info_load_base_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 7 ;
	inf->nType = typeLoadMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regDst = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( ",[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 3)), true ) ;
		pmi->AddOperand( "]" ) ;
	}
}

void ECSSakura2Processor::info_load_base_index( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 4 ;
	inf->nType = typeLoadMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regSrc2 = pbytCode[2] & 0x7F ;
	inf->regDst = pbytCode[3] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		const int	scale = ((pbytCode[1] >> 7) << 1) | (pbytCode[2] >> 7) ;
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( ",[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "+" ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
		pmi->AddOperand( "*" ) ;
		pmi->AddOperandImmediate8( (1 << scale), false ) ;
		pmi->AddOperand( "]" ) ;
	}
}

void ECSSakura2Processor::info_load_base_index_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 8 ;
	inf->nType = typeLoadMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regSrc2 = pbytCode[2] & 0x7F ;
	inf->regDst = pbytCode[3] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		const int	scale = ((pbytCode[1] >> 7) << 1) | (pbytCode[2] >> 7) ;
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( ",[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "+" ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
		pmi->AddOperand( "*" ) ;
		pmi->AddOperandImmediate8( (1 << scale), false ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 4)), true ) ;
		pmi->AddOperand( "]" ) ;
	}
}

// ストア命令 0x84～
void ECSSakura2Processor::info_store_base( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 3 ;
	inf->nType = typeStoreMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regSrc2 = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		pmi->AddOperand( "[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "]," ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
	}
}

void ECSSakura2Processor::info_store_base_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 7 ;
	inf->nType = typeStoreMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regSrc2 = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		pmi->AddOperand( "[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 3)), true ) ;
		pmi->AddOperand( "]," ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
	}
}

void ECSSakura2Processor::info_store_base_index( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 4 ;
	inf->nType = typeStoreMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regSrc2 = pbytCode[2] & 0x7F ;
	inf->regSrc3 = pbytCode[3] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		const int	scale = ((pbytCode[1] >> 7) << 1) | (pbytCode[2] >> 7) ;
		pmi->AddOperand( "[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "+" ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
		pmi->AddOperand( "*" ) ;
		pmi->AddOperandImmediate8( (1 << scale), false ) ;
		pmi->AddOperand( "]," ) ;
		pmi->AddOperandRegister( inf->regSrc3 ) ;
	}
}

void ECSSakura2Processor::info_store_base_index_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 8 ;
	inf->nType = typeStoreMem ;
	inf->regSrc1 = (pbytCode[1] >> 3) & 0x0F ;
	inf->regSrc2 = pbytCode[2] & 0x7F ;
	inf->regSrc3 = pbytCode[3] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		const int	scale = ((pbytCode[1] >> 7) << 1) | (pbytCode[2] >> 7) ;
		pmi->AddOperand( "[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "+" ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
		pmi->AddOperand( "*" ) ;
		pmi->AddOperandImmediate8( (1 << scale), false ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 4)), true ) ;
		pmi->AddOperand( "]," ) ;
		pmi->AddOperandRegister( inf->regSrc3 ) ;
	}
}

// ローカルメモリ・ロード命令 0x88～
void ECSSakura2Processor::info_load_local_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 7 ;
	inf->nType = typeLoadMem ;
	inf->regSrc1 = regBP ;
	inf->regDst = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( ",[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 3)), true ) ;
		pmi->AddOperand( "]" ) ;
	}
}

void ECSSakura2Processor::info_load_local_index_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 8 ;
	inf->nType = typeLoadMem ;
	inf->regSrc1 = regBP ;
	inf->regSrc2 = pbytCode[2] ;
	inf->regDst = pbytCode[3] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		const int	scale = (pbytCode[1] >> 5) ;
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( ",[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "+" ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
		pmi->AddOperand( "*" ) ;
		pmi->AddOperandImmediate8( (1 << scale), false ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 4)), true ) ;
		pmi->AddOperand( "]" ) ;
	}
}

// ローカルメモリ・ストア命令 0x8A～
void ECSSakura2Processor::info_store_local_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 7 ;
	inf->nType = typeStoreMem ;
	inf->regSrc1 = regBP ;
	inf->regSrc2 = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		pmi->AddOperand( "[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 3)), true ) ;
		pmi->AddOperand( "]," ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
	}
}

void ECSSakura2Processor::info_store_local_index_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 8 ;
	inf->nType = typeStoreMem ;
	inf->regSrc1 = regBP ;
	inf->regSrc2 = pbytCode[2] ;
	inf->regSrc3 = pbytCode[3] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "." ) ;
		pmi->AddMnemonic( pszInstructionDataType[pbytCode[1] & 0x07] ) ;
		//
		const int	scale = (pbytCode[1] >> 5) ;
		pmi->AddOperand( "[" ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "+" ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
		pmi->AddOperand( "*" ) ;
		pmi->AddOperandImmediate8( (1 << scale), false ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 4)), true ) ;
		pmi->AddOperand( "]," ) ;
		pmi->AddOperandRegister( inf->regSrc3 ) ;
	}
}

// ノーオペランド形式命令
void ECSSakura2Processor::info_no_operand( InstructionInfo * inf, const BYTE * pbytCode )
{
	InitializeInstructionInfo( inf, pbytCode ) ;
}

void ECSSakura2Processor::info_return( InstructionInfo * inf, const BYTE * pbytCode )
{
	InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeJump ;
}

// 1オペランド形式命令
void ECSSakura2Processor::info_operand_srcreg( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 2 ;
	inf->regSrc1 = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regSrc1 ) ;
	}
}

void ECSSakura2Processor::info_operand_dstreg( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 2 ;
	inf->nType = typeOpReg ;
	inf->regSrc1 = pbytCode[1] ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regSrc1 ) ;
	}
}

void ECSSakura2Processor::info_push_srcreg( InstructionInfo * inf, const BYTE * pbytCode )
{
	info_operand_srcreg( inf, pbytCode ) ;
	//
	inf->nFlags |= flagComplexSourceRegister ;
	inf->nType = typeComplex ;
}

void ECSSakura2Processor::info_pop_dstreg( InstructionInfo * inf, const BYTE * pbytCode )
{
	info_operand_dstreg( inf, pbytCode ) ;
	//
	inf->nFlags |= flagComplexDestinationRegister ;
	inf->nType = typeComplex ;
}

void ECSSakura2Processor::info_jump_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	info_operand_imm32( inf, pbytCode ) ;
	//
	inf->nType = typeJump ;
}

void ECSSakura2Processor::info_jump_reg( InstructionInfo * inf, const BYTE * pbytCode )
{
	info_operand_srcreg( inf, pbytCode ) ;
	//
	inf->nType = typeJump ;
}

void ECSSakura2Processor::info_operand_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 5 ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandImmediate32( *((DWORD*)(pbytCode + 1)), false ) ;
	}
}

// 2オペランド形式命令
void ECSSakura2Processor::info_move_reg_imm64( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 10 ;
	inf->nType = typeLoadImm64 ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandImmediate64( *((INT64*)(pbytCode + 2)) ) ;
	}
}

void ECSSakura2Processor::info_move_reg_reg( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 3 ;
	inf->nType = typeMoveReg ;
	inf->regSrc1 = pbytCode[2] ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
	}
}

void ECSSakura2Processor::info_cvt_move_reg_reg( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 3 ;
	inf->nType = typeCvtMoveReg ;
	inf->regSrc1 = pbytCode[2] ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
	}
}

void ECSSakura2Processor::info_jump_reg_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 6 ;
	inf->nType = typeJump ;
	inf->regSrc1 = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 2)), true ) ;
	}
}

void ECSSakura2Processor::info_pushs_reg_imm8( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nFlags |= flagComplexSourceRegister ;
	inf->nType = typeComplex ;
	inf->nBytes = 3 ;
	inf->regSrc1 = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandImmediate32( pbytCode[2], false ) ;
	}
}

void ECSSakura2Processor::info_pops_reg_imm8( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nFlags |= flagComplexDestinationRegister ;
	inf->nType = typeComplex ;
	inf->nBytes = 3 ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandImmediate32( pbytCode[2], false ) ;
	}
}

void ECSSakura2Processor::info_operand_reg_reg( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 3 ;
	inf->nType = typeOpRegReg ;
	inf->regSrc1 = pbytCode[1] ;
	inf->regSrc2 = pbytCode[2] ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
	}
}

// 3オペランド形式命令
void ECSSakura2Processor::info_operand_reg_reg_imm8( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 4 ;
	inf->nType = typeOpRegRegImm ;
	inf->regSrc1 = pbytCode[2] ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandImmediate8( pbytCode[3], false ) ;
	}
}

void ECSSakura2Processor::info_operand_reg_reg_imm32( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 7 ;
	inf->nType = typeOpRegRegImm ;
	inf->regSrc1 = pbytCode[2] ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandImmediate32( *((SDWORD*)(pbytCode + 3)), false ) ;
	}
}

void ECSSakura2Processor::info_operand_reg_reg_reg( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nBytes = 4 ;
	inf->nType = typeOpRegRegReg ;
	inf->regSrc1 = pbytCode[2] ;
	inf->regSrc2 = pbytCode[3] ;
	inf->regDst = pbytCode[1] ;
	//
	if ( pmi != NULL )
	{
		pmi->AddOperandRegister( inf->regDst ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( inf->regSrc1 ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( inf->regSrc2 ) ;
	}
}

// memory hint 命令
void ECSSakura2Processor::info_memory_hint( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeComplex ;
	inf->nBytes = 3 ;
	//
	if ( pmi != NULL )
	{
		if ( pszMemoryHintExtensionMnemonic[pbytCode[1]] != NULL )
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic),
							pszMemoryHintExtensionMnemonic[pbytCode[1]] ) ;
		}
		else
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic), "???" ) ;
		}
		pmi->AddOperandRegister( pbytCode[2] ) ;
	}
}

// float extension 命令
void ECSSakura2Processor::info_float_extension( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeComplex ;
	inf->nBytes = 4 ;
	//
	if ( pmi != NULL )
	{
		if ( pszFloatExtensionMnemonic[pbytCode[1]] != NULL )
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic),
							pszFloatExtensionMnemonic[pbytCode[1]] ) ;
		}
		else
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic), "???" ) ;
		}
		pmi->AddOperandRegister( pbytCode[2] ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( pbytCode[3] ) ;
	}
}

// SIMD extension 命令
void ECSSakura2Processor::info_simd64_extension_2op( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeComplex ;
	inf->nBytes = 4 ;
	//
	if ( pmi != NULL )
	{
		if ( pszSIMD64Extension2OpMnemonic[pbytCode[1]] != NULL )
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic),
							pszSIMD64Extension2OpMnemonic[pbytCode[1]] ) ;
		}
		else
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic), "???" ) ;
		}
		pmi->AddOperandRegister( pbytCode[2] ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( pbytCode[3] ) ;
	}
}

void ECSSakura2Processor::info_simd64_extension_3op( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeComplex ;
	inf->nBytes = 5 ;
	//
	if ( pmi != NULL )
	{
		if ( pszSIMD64Extension3OpMnemonic[pbytCode[1]] != NULL )
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic),
							pszSIMD64Extension3OpMnemonic[pbytCode[1]] ) ;
		}
		else
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic), "???" ) ;
		}
		pmi->AddOperandRegister( pbytCode[2] ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( pbytCode[3] ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandImmediate8( pbytCode[4], false ) ;
	}
}

void ECSSakura2Processor::info_simd128_extension_2op( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeComplex ;
	inf->nBytes = 4 ;
	inf->regSrc1 = pbytCode[3] ;
	inf->regDst = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		if ( pszSIMD128Extension2OpMnemonic[pbytCode[1]] != NULL )
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic),
							pszSIMD128Extension2OpMnemonic[pbytCode[1]] ) ;
		}
		else
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic), "???" ) ;
		}
		pmi->AddOperandRegister( pbytCode[2] ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( pbytCode[3] ) ;
	}
}

void ECSSakura2Processor::info_simd128_extension_3op( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeComplex ;
	inf->nBytes = 5 ;
	inf->regSrc1 = pbytCode[3] ;
	inf->regSrc2 = pbytCode[4] ;
	inf->regDst = pbytCode[2] ;
	//
	if ( pmi != NULL )
	{
		if ( pszSIMD128Extension3OpMnemonic[pbytCode[1]] != NULL )
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic),
							pszSIMD128Extension3OpMnemonic[pbytCode[1]] ) ;
		}
		else
		{
			StringAdd( pmi->szMnemonic, sizeof(pmi->szMnemonic), "???" ) ;
		}
		pmi->AddOperandRegister( pbytCode[2] ) ;
		pmi->AddOperand( "," ) ;
		pmi->AddOperandRegister( pbytCode[3] ) ;
		pmi->AddOperand( "," ) ;
		//
		if ( pbytCode[1] == 0x00 )
		{
			pmi->AddOperandRegister( pbytCode[4] ) ;
		}
		else
		{
			pmi->AddOperandImmediate8( pbytCode[4], false ) ;
		}
	}
}

// 不正命令
void ECSSakura2Processor::info_bad_instruction( InstructionInfo * inf, const BYTE * pbytCode )
{
	MnemonicInfo *	pmi = InitializeInstructionInfo( inf, pbytCode ) ;
	//
	inf->nType = typeObject ;
	//
	if ( pmi != NULL )
	{
		pmi->AddMnemonic( "???" ) ;
	}
}

