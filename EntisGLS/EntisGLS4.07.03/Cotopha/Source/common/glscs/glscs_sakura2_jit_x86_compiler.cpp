
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_jit_x86_compiler.h>

using	namespace ECSSakura2JIT ;
using	namespace ECSSakura2Processor ;

#if	defined(__PROCESSOR_INTEL_X86__)

//////////////////////////////////////////////////////////////////////////////
// x86 コードバッファ出力オブジェクト
//////////////////////////////////////////////////////////////////////////////

// ジャンプ命令を追加する
//////////////////////////////////////////////////////////////////////////////
void X86CodeBuffer::WriteJump( Block * pBlock, const void * pTarget )
{
	BYTE *	pbytCode = pBlock->pbytBufAligned16 + pBlock->nCodeUsed ;
	SDWORD	dwRel32 = (SDWORD) pTarget - (SDWORD) (pbytCode + 5) ;
	//
	pbytCode[0] = 0xE9 ;
	*((SDWORD*)(pbytCode + 1)) = dwRel32 ;
	//
	pBlock->nCodeUsed += 5 ;
}


//////////////////////////////////////////////////////////////////////////////
// x86 低水準命令出力
//////////////////////////////////////////////////////////////////////////////

// 呼び出し規約設定
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::SetCallingABI
	( X86GenericAssembler::FastcallType abiCall )
{
	switch ( abiCall )
	{
	case	fastcallMSstyle:
	default:
		m_abiFastcall.fRetWithClean = true ;
		m_abiFastcall.regArg[0] = x86_ECX ;
		m_abiFastcall.regArg[1] = x86_EDX ;
		m_abiFastcall.regArg[2] = x86_EAX ;
		m_abiFastcall.regArg[3] = x86_EAX ;
		m_abiFastcall.maxRegArg = 2 ;
		break ;
	case	fastcallGCCstyle:
		m_abiFastcall.fRetWithClean = true ;
		m_abiFastcall.regArg[0] = x86_EAX ;
		m_abiFastcall.regArg[1] = x86_ECX ;
		m_abiFastcall.regArg[2] = x86_EDX ;
		m_abiFastcall.regArg[3] = x86_EAX ;
		m_abiFastcall.maxRegArg = 3 ;
		break ;
	case	fastcallCstyle:
		m_abiFastcall.fRetWithClean = false ;
		m_abiFastcall.regArg[0] = x86_EAX ;
		m_abiFastcall.regArg[1] = x86_ECX ;
		m_abiFastcall.regArg[2] = x86_EDX ;
		m_abiFastcall.regArg[3] = x86_EAX ;
		m_abiFastcall.maxRegArg = 0 ;
		break ;
	}
}

// レジスタ/メモリアクセス命令
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteX86RegMemOperand
	( DWORD binOPcode, int sizeOPcode,
		int regop, bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex,
		DWORD immData, int sizeImm )
{
	//
	// プリフィックス／オペコード出力
	//
	BYTE	bytInstruction[0x20] ;
	for ( int iOp = 0; iOp < sizeOPcode; iOp ++ )
	{
		bytInstruction[sizeOPcode - (iOp + 1)] = (BYTE) (binOPcode & 0xFF) ;
		binOPcode >>= 8 ;
	}
	ESLAssert( binOPcode == 0 ) ;
	//
	// ModR/M SIB disp 出力
	//
	int	i = sizeOPcode ;
	int	mod = 0xC0 | ((regop & 0x07) << 3) | (regmem & 0x07) ;
	if ( modeMemory )
	{
		//
		// disp サイズと EBP がベースレジスタになる場合の正規化
		//
		bool	fSIB = false ;
		int		sizeDisp = 0 ;
		if ( dispOffset == 0 )
		{
			mod = 0x00 ;
		}
		else if ( (-0x80 <= dispOffset) && (dispOffset <= 0x7F) )
		{
			mod = 0x40 ;
			sizeDisp = 1 ;
		}
		else
		{
			mod = 0x80 ;
			sizeDisp = 4 ;
		}
		if ( (regmem == x86_EBP) && (sizeDisp == 0) )
		{
			mod = 0x40 ;
			sizeDisp = 1 ;
		}
		//
		// ModR/M 出力
		//
		if ( regIndex == x86_Nothing )
		{
			if ( regmem == x86_Nothing )
			{
				// [dispOffset]
				mod = 0x00 | ((regop & 0x07) << 3) | 0x05 ;
				sizeDisp = 4 ;
			}
			else
			{
				// [regmem + dispOffset]
				if ( regmem != x86_ESP )
				{
					mod |= ((regop & 0x07) << 3) | (regmem & 0x07) ;
				}
				else
				{
					mod |= ((regop & 0x07) << 3) | 0x04 ;
					fSIB = true ;
				}
			}
		}
		else
		{
			if ( regmem == x86_Nothing )
			{
				// [index + dispOffset]
				mod = 0x00 | ((regop & 0x07) << 3) | 0x04 ;
				sizeDisp = 4 ;
			}
			else
			{
				// [regmem + index + dispOffset]
				mod |= ((regop & 0x07) << 3) | 0x04 ;
				fSIB = true ;
			}
		}
		bytInstruction[i ++] = (BYTE) mod ;
		//
		// SIB 出力
		//
		if ( fSIB )
		{
			ESLAssert( regIndex != x86_ESP ) ;
			if ( regIndex == x86_Nothing )
			{
				regIndex = x86_ESP ;
			}
			bytInstruction[i ++] =
				(BYTE) (((scaleIndex & 0x03) << 6)
						| ((regIndex & 0x07) << 3) | (regmem & 0x07)) ;
		}
		//
		// disp 出力
		//
		for ( int iDisp = 0; iDisp < sizeDisp; iDisp ++ )
		{
			bytInstruction[i ++] = (BYTE) (dispOffset & 0xFF) ;
			dispOffset >>= 8 ;
		}
	}
	else
	{
		bytInstruction[i ++] = (BYTE) mod ;
	}
	//
	// imm 出力
	//
	BYTE *		pbytCode = (BYTE*) GetNextAddress() ;
	const int	iImmOffset = i ;
	for ( int iImm = 0; iImm < sizeImm; iImm ++ )
	{
		bytInstruction[i ++] = (BYTE) (immData & 0xFF) ;
		immData >>= 8 ;
	}
	//
	// 命令コードを出力バッファに出力
	//
	m_buf->WriteInstruction( bytInstruction, i ) ;
	//
	return	pbytCode + iImmOffset ;
}

// その他の即値命令
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteX86ImmediateOperand
	( DWORD binOPcode, int sizeOPcode, DWORD immData, int sizeImm )
{
	//
	// プリフィックス／オペコード出力
	//
	BYTE	bytInstruction[0x20] ;
	for ( int iOp = 0; iOp < sizeOPcode; iOp ++ )
	{
		bytInstruction[sizeOPcode - (iOp + 1)] = (BYTE) (binOPcode & 0xFF) ;
		binOPcode >>= 8 ;
	}
	//
	// imm 出力
	//
	int			i = sizeOPcode ;
	BYTE *		pbytCode = (BYTE*) GetNextAddress() ;
	const int	iImmOffset = i ;
	for ( int iImm = 0; iImm < sizeImm; iImm ++ )
	{
		bytInstruction[i ++] = (BYTE) (immData & 0xFF) ;
		immData >>= 8 ;
	}
	//
	// 命令コードを出力バッファに出力
	//
	m_buf->WriteInstruction( bytInstruction, i ) ;
	//
	return	pbytCode + iImmOffset ;
}

// call imm32
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteX86CallImm32( const void * pfnCallTarget )
{
	INT_PTR	immData =
		((INT_PTR) pfnCallTarget) - ((INT_PTR) GetNextAddress() + 5) ;
	return	WriteX86ImmediateOperand( x86op1_CALL, 1, (DWORD) immData, 4 ) ;
}

// call mem/reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86CallRegMem
	( bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_CALL_RM, 1, 2,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// jmp imm32
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteX86JmpImm32( const void * pfnJmpTarget )
{
	INT_PTR	immData =
		((INT_PTR) pfnJmpTarget) - ((INT_PTR) GetNextAddress() + 5) ;
	return	WriteX86ImmediateOperand( x86op1_JMP, 1, (DWORD) immData, 4 ) ;
}

// push reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86PushReg( X86GenericAssembler::X86Register regSrc )
{
	WriteX86ImmediateOperand( 0x50 + (regSrc & 0x07), 1 ) ;
}

// pop reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86PopReg( X86GenericAssembler::X86Register regDst )
{
	WriteX86ImmediateOperand( 0x58 + (regDst & 0x07), 1 ) ;
}

// mov reg, mem
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86LoadRegMem
	( X86GenericAssembler::X86Register regDst, X86GenericAssembler::X86Register regBase,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_MOV_LOAD, 1,
			regDst, true, regBase, dispOffset, regIndex, scaleIndex ) ;
}

// mov mem, reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86StoreRegMem
	( X86GenericAssembler::X86Register regSrc, X86GenericAssembler::X86Register regBase,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_MOV_STORE, 1,
			regSrc, true, regBase, dispOffset, regIndex, scaleIndex ) ;
}

// mov reg, imm32
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteX86MoveRegImm32
	( X86GenericAssembler::X86Register regDst, DWORD immData )
{
	return	WriteX86ImmediateOperand( 0xB8 | (regDst & 0x07), 1, immData, 4 ) ;
}

// mov reg, reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86MoveRegReg
	( X86GenericAssembler::X86Register regDst,
		X86GenericAssembler::X86Register regSrc )
{
	if ( regDst != regSrc )
	{
		WriteX86RegMemOperand( x86op1_MOV_LOAD, 1, regDst, false, regSrc ) ;
	}
}

// lea reg, context->m_regset[x]
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86LeaSakura2Register
	( X86GenericAssembler::X86Register regDst, int regSakura2 )
{
	WriteX86RegMemOperand
		( x86op1_LEA, 1, regDst,
			true, x86_EBX, Context::OffsetOfReg(regSakura2) ) ;
}

// lea reg, mem
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86LeaRegMem
	( X86GenericAssembler::X86Register regDst, X86GenericAssembler::X86Register regBase,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_LEA, 1, regDst,
			true, regBase, dispOffset, regIndex, scaleIndex ) ;
}

// add reg, imm32
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteX86AddRegImm32
	( X86GenericAssembler::X86Register regDst, DWORD immData )
{
	return	WriteX86RegImmOperand
		( x86op1_OP_RM_IMM32, 1, x86op2nd_ADD, regDst, immData, 4 ) ;
}

// add reg, mem
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86AddRegMem
	( X86GenericAssembler::X86Register regDst,
		bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_ADD_REG_RM, 1, regDst,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// adc reg, imm32
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteX86AdcRegImm32
	( X86GenericAssembler::X86Register regDst, DWORD immData )
{
	return	WriteX86RegImmOperand
		( x86op1_OP_RM_IMM32, 1, x86op2nd_ADC, regDst, immData, 4 ) ;
}

// adc reg, mem
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86AdcRegMem
	( X86GenericAssembler::X86Register regDst,
		bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_ADC_REG_RM, 1, regDst,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// sub reg, mem/reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86SubRegMem
	( X86GenericAssembler::X86Register regDst,
		bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_SUB_REG_RM, 1, regDst,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// or reg, mem/reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86OrRegMem
	( X86GenericAssembler::X86Register regDst,
		bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_OR_REG_RM, 1, regDst,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// and reg, mem/reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86AndRegMem
	( X86GenericAssembler::X86Register regDst,
		bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_AND_REG_RM, 1, regDst,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// xor reg, mem/reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86XorRegMem
	( X86GenericAssembler::X86Register regDst,
		bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_XOR_REG_RM, 1, regDst,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// cmp reg, mem/reg
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86CmpRegMem
	( X86GenericAssembler::X86Register regPhy,
		bool modeMemory, X86GenericAssembler::X86Register regmem,
		INT_PTR dispOffset, X86GenericAssembler::X86Register regIndex, int scaleIndex )
{
	WriteX86RegMemOperand
		( x86op1_CMP_REG_RM, 1, regPhy,
			modeMemory, regmem, dispOffset, regIndex, scaleIndex ) ;
}

// imul reg, imm32 (自動最適化)
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86ImulRegImm32
	( X86GenericAssembler::X86Register regDst, int imm32 )
{
	int	iShift = 0 ;
	int	immScaled = imm32 ;
	while ( (immScaled & 0x01) == 0 )
	{
		immScaled >>= 1 ;
		iShift ++ ;
	}
	switch ( immScaled )
	{
	case	1:
		WriteX86ShlRegImm8( regDst, iShift ) ;
		return ;
	case	3:
		WriteX86LeaRegMem( regDst, regDst, 0, regDst, 1 ) ;
		WriteX86ShlRegImm8( regDst, iShift ) ;
		return ;
	case	5:
		WriteX86LeaRegMem( regDst, regDst, 0, regDst, 2 ) ;
		WriteX86ShlRegImm8( regDst, iShift ) ;
		return ;
	case	9:
		WriteX86LeaRegMem( regDst, regDst, 0, regDst, 3 ) ;
		WriteX86ShlRegImm8( regDst, iShift ) ;
		return ;
	}
	WriteX86RegMemOperand
		( x86op1_IMUL_REG_RM_IMM32, 1, regDst,
			false, regDst, 0, x86_Nothing, 0, imm32, 4 ) ;
}

// shld reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86ShldRegRegImm8
	( X86GenericAssembler::X86Register regDst,
		X86GenericAssembler::X86Register regSrc, int imm8 )
{
	if ( imm8 >= 1 )
	{
		WriteX86RegMemOperand
			( x86op2_SHLD_REG_RM_IMM8, 2,
				regSrc, false, regDst, 0, x86_Nothing, 0, imm8, 1 ) ;
	}
}

// shrd reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86ShrdRegRegImm8
	( X86GenericAssembler::X86Register regDst,
		X86GenericAssembler::X86Register regSrc, int imm8 )
{
	if ( imm8 >= 1 )
	{
		WriteX86RegMemOperand
			( x86op2_SHRD_REG_RM_IMM8, 2,
				regSrc, false, regDst, 0, x86_Nothing, 0, imm8, 1 ) ;
	}
}

// shl reg, imm8
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86ShlRegImm8
	( X86GenericAssembler::X86Register regDst, int imm8 )
{
	if ( imm8 >= 1 )
	{
		WriteX86RegMemOperand
			( x86op1_SHIFT_IMM8, 1, x86op2nd_SHL,
				false, regDst, 0, x86_Nothing, 0, imm8, 1 ) ;
	}
}

// shr reg, imm8
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86ShrRegImm8
	( X86GenericAssembler::X86Register regDst, int imm8 )
{
	if ( imm8 >= 1 )
	{
		WriteX86RegMemOperand
			( x86op1_SHIFT_IMM8, 1, x86op2nd_SHR,
				false, regDst, 0, x86_Nothing, 0, imm8, 1 ) ;
	}
}

// sar reg, imm8
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteX86SarRegImm8
	( X86GenericAssembler::X86Register regDst, int imm8 )
{
	if ( imm8 >= 1 )
	{
		WriteX86RegMemOperand
			( x86op1_SHIFT_IMM8, 1, x86op2nd_SAR,
				false, regDst, 0, x86_Nothing, 0, imm8, 1 ) ;
	}
}

// 関数呼び出しコード出力 : reg 形式
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToCallInstructionReg
	( OPERATION_DST_SRC_PROC pfnGen, int reg ) 
{
	FlushAllRegisters() ;
	//
	int	countPushArg = 0 ;
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[1], reg ) ;
	if ( m_abiFastcall.maxRegArg < 2 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[1] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[0], reg ) ;
	if ( m_abiFastcall.maxRegArg < 1 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[0] ) ;
		countPushArg ++ ;
	}
	WriteX86CallImm32( (const void*) pfnGen ) ;
	//
	if ( !m_abiFastcall.fRetWithClean && (countPushArg > 0) )
	{
		WriteX86AddRegImm32( x86_ESP, countPushArg * 4 ) ;
	}
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出しコード出力 : reg, reg 形式
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToCallInstructionRegReg
	( OPERATION_DST_SRC_PROC pfnGen, int dstreg, int srcreg )
{
	FlushAllRegisters() ;
	//
	int	countPushArg = 0 ;
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[1], srcreg ) ;
	if ( m_abiFastcall.maxRegArg < 2 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[1] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[0], dstreg ) ;
	if ( m_abiFastcall.maxRegArg < 1 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[0] ) ;
		countPushArg ++ ;
	}
	WriteX86CallImm32( (const void*) pfnGen ) ;
	//
	if ( !m_abiFastcall.fRetWithClean && (countPushArg > 0) )
	{
		WriteX86AddRegImm32( x86_ESP, countPushArg * 4 ) ;
	}
	//
	ResetRegisterAfterCall() ;
}

void X86GenericAssembler::WriteToCallSIMD128InstructionRegReg
	( OPERATION_SIMD128_DST_SRC_PROC pfnGen, int dstreg, int srcreg )
{
	FlushAllRegisters() ;
	//
	int	countPushArg = 0 ;
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[1], srcreg ) ;
	if ( m_abiFastcall.maxRegArg < 2 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[1] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[0], dstreg ) ;
	if ( m_abiFastcall.maxRegArg < 1 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[0] ) ;
		countPushArg ++ ;
	}
	WriteX86CallImm32( (const void*) pfnGen ) ;
	//
	if ( !m_abiFastcall.fRetWithClean && (countPushArg > 0) )
	{
		WriteX86AddRegImm32( x86_ESP, countPushArg * 4 ) ;
	}
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出しコード出力 : reg, reg, reg 形式
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToCallInstructionRegRegReg
	( OPERATION_DST_SRC_SRC2_PROC pfnGen,
				int dstreg, int srcreg, int srcreg2 )
{
	FlushAllRegisters() ;
	//
	int	countPushArg = 0 ;
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[2], srcreg2 ) ;
	if ( m_abiFastcall.maxRegArg < 3 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[2] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[1], srcreg ) ;
	if ( m_abiFastcall.maxRegArg < 2 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[1] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[0], dstreg ) ;
	if ( m_abiFastcall.maxRegArg < 1 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[0] ) ;
		countPushArg ++ ;
	}
	WriteX86CallImm32( (const void*) pfnGen ) ;
	//
	if ( !m_abiFastcall.fRetWithClean && (countPushArg > 0) )
	{
		WriteX86AddRegImm32( x86_ESP, countPushArg * 4 ) ;
	}
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出しコード出力 : reg, reg, imm 形式
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToCallInstructionRegRegImm
	( OPERATION_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm )
{
	FlushAllRegisters() ;
	//
	int	countPushArg = 0 ;
	WriteX86MoveRegImm32( m_abiFastcall.regArg[2], imm ) ;
	if ( m_abiFastcall.maxRegArg < 3 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[2] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[1], srcreg ) ;
	if ( m_abiFastcall.maxRegArg < 2 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[1] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[0], dstreg ) ;
	if ( m_abiFastcall.maxRegArg < 1 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[0] ) ;
		countPushArg ++ ;
	}
	WriteX86CallImm32( (const void*) pfnGen ) ;
	//
	if ( !m_abiFastcall.fRetWithClean && (countPushArg > 0) )
	{
		WriteX86AddRegImm32( x86_ESP, countPushArg * 4 ) ;
	}
	//
	ResetRegisterAfterCall() ;
}

void X86GenericAssembler::WriteToCallSIMD128InstructionRegRegImm
	( OPERATION_SIMD128_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm )
{
	FlushAllRegisters() ;
	//
	int	countPushArg = 0 ;
	WriteX86MoveRegImm32( m_abiFastcall.regArg[3], imm ) ;
	if ( m_abiFastcall.maxRegArg < 4 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[3] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[2], srcreg ) ;
	if ( m_abiFastcall.maxRegArg < 3 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[2] ) ;
		countPushArg ++ ;
	}
	WriteX86LeaSakura2Register( m_abiFastcall.regArg[1], dstreg ) ;
	if ( m_abiFastcall.maxRegArg < 2 )
	{
		WriteX86PushReg( m_abiFastcall.regArg[1] ) ;
		countPushArg ++ ;
	}
	if ( m_abiFastcall.maxRegArg < 1 )
	{
		WriteX86PushReg( x86_EBX ) ;
		countPushArg ++ ;
	}
	else if ( m_abiFastcall.regArg[0] != x86_EBX )
	{
		WriteX86MoveRegReg( m_abiFastcall.regArg[0], x86_EBX ) ;
	}
	WriteX86CallImm32( (const void*) pfnGen ) ;
	//
	if ( !m_abiFastcall.fRetWithClean && (countPushArg > 0) )
	{
		WriteX86AddRegImm32( x86_ESP, countPushArg * 4 ) ;
	}
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出し後のレジスタ処理
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::ResetRegisterAfterCall( void )
{
	ResetDataRegisters() ;
	//
	ESLAssert( m_lruPointer[regPtrPhyECX].regPhy == x86_ECX ) ;
	m_lruPointer.FreeSlot( regPtrPhyECX ) ;
}

// プロローグコード出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WritePrologue( void )
{
	WriteX86PushReg( x86_EBP ) ;
	WriteX86PushReg( x86_EDI ) ;
	WriteX86PushReg( x86_ESI ) ;
	WriteX86PushReg( x86_EBX ) ;
	//
	if ( m_abiFastcall.maxRegArg >= 1 )
	{
		if ( m_abiFastcall.regArg[0] != x86_EBX )
		{
			WriteX86MoveRegReg( x86_EBX, m_abiFastcall.regArg[0] ) ;
		}
	}
	else
	{
		WriteX86LoadRegMem( x86_EBX, x86_ESP, 5 * 4 ) ;
	}
}

void X86GenericAssembler::WriteSubPrologue( void )
{
	if ( m_flagNoBoundary )
	{
		for ( int i = 0; i < 2; i ++ )
		{
			if ( m_regTLBFetched[i] >= 0 )
			{
				WritePrefetchTLB( i, m_regTLBFetched[i] ) ;
			}
		}
	}
}

// エピローグコード出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteEpilogue( int ipExit )
{
	if ( ipExit >= 0 )
	{
		WriteX86RegMemOperand
			( 0xC7, 1, 0, true,
				x86_EBX, offsetof(Context,m_ip),
				x86_Nothing, 0, ipExit, 4 ) ;
	}
	WriteX86PopReg( x86_EBX ) ;
	WriteX86PopReg( x86_ESI ) ;
	WriteX86PopReg( x86_EDI ) ;
	WriteX86PopReg( x86_EBP ) ;
	//
	if ( m_abiFastcall.fRetWithClean
			&& (m_abiFastcall.maxRegArg < 1) )
	{
		WriteX86ImmediateOperand
			( 0xC2, 1, (1 - m_abiFastcall.maxRegArg) * 4, 2 ) ;
	}
	else
	{
		WriteX86ImmediateOperand( 0xC3, 1 ) ;
	}
}

// レジスタへの変更をコンテキストに書き出し
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::FlushAllRegisters( void )
{
}

// レジスタへの変更をコンテキストに書き出し
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteBackAllRegisters( void )
{
}

// レジスタへの変更をコンテキストに書き出し
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::FlushRegister( int regSakura )
{
}

// レジスタの値を物理レジスタに復元する
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::ReloadRegisters( void )
{
}

// レジスタコンテキストをリセット
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::ResetAllRegisters( void )
{
	m_lruPointer.FreeAllUnlockedSlot() ;
}

// レジスタコンテキストをリセット
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::ResetRegister( int regSakura )
{
	ModifiedRegister( regSakura ) ;
}

// レジスタコンテキストをリセット（ポインタレジスタ以外）
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::ResetDataRegisters( void )
{
}

// BP ポインタ用物理レジスタ識別子取得
//////////////////////////////////////////////////////////////////////////////
int X86GenericAssembler::GetFramePointerPhysicalRegister( void ) const
{
	return	regPtrPhyBPIndex ;
}

// レジスタ値変更通知（BP 以外ポインタ用）
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::ModifiedRegister( int reg )
{
	for ( int i = 0; i < regPtrPhyCount; i ++ )
	{
		int	regCode = m_lruPointer[i].regSakura ;
		if ( ((regCode & 0xFF) == reg)
			|| (((regCode >> 8) & 0xFF) == reg) )
		{
			if ( !m_lruPointer[i].fTLBFetched )
			{
				m_lruPointer.FreeSlot( i ) ;
			}
		}
	}
}

// メモリ参照で使用する TLB スロットを取得する
//////////////////////////////////////////////////////////////////////////////
int X86GenericAssembler::SelectTLBSlotFromMemoryOperand
	( int regBase, int regIndex, int scaleIndex )
{
	int	slotTLB = (regBase & 0x03) ;
	if ( (regIndex < 0) && (m_tlbFetch[regBase] != 0xFF) )
	{
		slotTLB = m_tlbFetch[regBase] & 0x03 ;
		ESLAssert( !(slotTLB & 0xFE) ) ;
	}
	else if ( !(slotTLB & ~0x01)
			&& m_lruPointer.IsSlotAssigned(slotTLB)
			&& m_lruPointer[slotTLB].fTLBFetched )
	{
		slotTLB |= 0x02 ;
		if ( slotTLB < regPtrPhyCount )
		{
			ESLAssert( !m_lruPointer.IsSlotAssigned(slotTLB)
						|| !m_lruPointer[slotTLB].fTLBFetched ) ;
		}
	}
	else
	{
		if ( slotTLB < regPtrPhyCount )
		{
			ESLAssert( !m_lruPointer.IsSlotAssigned(slotTLB)
						|| !m_lruPointer[slotTLB].fTLBFetched ) ;
		}
	}
	return	slotTLB ;
}

// アドレス変換して物理レジスタにアドレスをロードするコード出力
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteRealizePointerRegister
	( int regPhy, int regSakura,
		RealizePointerBoundary& rpb, const void * ptrEpilogue )
{
	//
	// レジスタの値を物理レジスタにロード
	//
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSakura ) ;
	//
	// アドレス変換
	//
	void *	ptrEscJumpFrom = NULL ;
	if ( regPhy == regPtrPhyBPIndex )
	{
		WriteX86LoadRegMem
			( x86_EBP, x86_EBX,
				offsetof(Context,m_segStack.pbytBuffer) ) ;
		WriteX86SubRegMem
			( x86_EAX, true, x86_EBX,
				offsetof(Context,m_segStack.baseOffset) ) ;
		WriteX86AddRegMem( x86_EBP, false, x86_EAX ) ;
		//
		// メモリ境界判定
		//
		/*
		// push bp / move bp,sp / add.sp の順で実行した場合
		// add.sp でスタックの拡張が行われる条件で
		// move bp,sp がメモリアクセス例外になってしまうため除外
		//
		if ( !m_flagNoBoundary )
		{
			ptrEscJumpFrom =
				WriteToCheckBoundaryAddress
					( rpb, x86_EAX, x86_EDX,
						offsetof(Context,m_segStack), false ) ;
		}
		*/
	}
	else
	{
		//
		// アドレス変換
		//
		int	slotTLB = SelectTLBSlotFromMemoryOperand( regSakura ) ;
		ESLAssert( !m_lruPointer[regPhy].fTLBFetched ) ;
		ptrEscJumpFrom =
			WriteToTranslateAddress
				( m_lruPointer[regPhy],
					(X86Register) m_lruPointer[regPhy].regPhy,
					x86_EAX, x86_EDX, slotTLB ) ;
		//
		// LRU 更新
		//
		m_lruPointer.AddStale( 1 ) ;
		m_lruPointer.NewSlot( regPhy, (regSakura | 0xFF00) ) ;
	}
	if ( (ptrEscJumpFrom != NULL) && (ptrEpilogue != NULL) )
	{
		CommitJumpTarget( ptrEscJumpFrom, ptrEpilogue ) ;
	}
	return	ptrEscJumpFrom ;
}

// 物理レジスタにロードしたポインタの境界チェックコードを完成させる
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::CommitRealizePointerRegister
	( RealizePointerBoundary& rpb,
				int offsetFirst, int offsetEnd )
{
	if ( rpb.offsetFirst > offsetFirst )
	{
		rpb.offsetFirst = offsetFirst ;
	}
	if ( rpb.offsetEnd < offsetEnd )
	{
		rpb.offsetEnd = offsetEnd ;
	}
	if ( !m_flagNoBoundary )
	{
		if ( rpb.pdwFirst != NULL )
		{
			*(rpb.pdwFirst) = rpb.offsetFirst ;
		}
		if ( rpb.pdwLimit != NULL )
		{
			*(rpb.pdwLimit) = rpb.offsetEnd - rpb.offsetFirst ;
		}
	}
}

// ポインタアクセス用レジスタ割り当て処理コード出力
//（物理ポインタ用レジスタ番号を返却）
//////////////////////////////////////////////////////////////////////////////
int X86GenericAssembler::WriteAssignPointerRegister
	( int regPtr, int regIndex, int scale,
		int offsetFirst, int offsetEnd, void *& ptrEscJumpFrom )
{
	ptrEscJumpFrom = NULL ;
	if ( (regPtr == regZeroPtr) && (scale == 0) )
	{
		regPtr = regIndex ;
		regIndex = -1 ;
	}
	if ( (regPtr == regBP) && (regIndex < 0) )
	{
		return	regPtrPhyBPIndex ;
	}
	const int	regCode =
		(regPtr & 0xFF) | ((regIndex & 0xFF) << 8) | (scale << 16) ;
	int	iPtrReg = m_lruPointer.FindAssignedSlot( regCode ) ;
	if ( iPtrReg < 0 )
	{
		//
		// 新規レジスタ割り当て
		//
		iPtrReg = m_lruPointer.FindFreeSlot() ;
		if ( iPtrReg < 0 )
		{
			iPtrReg = m_lruPointer.FindMostOldSlot() ;
			ESLAssert( iPtrReg >= 0 ) ;
		}
		ESLAssert( !m_lruPointer[iPtrReg].fTLBFetched ) ;
		//
		// 仮想アドレスを EDX:EAX にロード
		//
		WriteToLoadSakura2AddressRegister
			( x86_EAX, x86_EDX, regPtr, regIndex, scale ) ;
		//
		// アドレス変換
		//
		int	slotTLB = SelectTLBSlotFromMemoryOperand
									( regPtr, regIndex, scale ) ;
		ptrEscJumpFrom =
			WriteToTranslateAddress
				( m_lruPointer[iPtrReg],
					(X86Register) m_lruPointer[iPtrReg].regPhy,
					x86_EAX, x86_EDX, slotTLB ) ;
		//
		// LRU 更新
		//
		m_lruPointer.AddStale( 1 ) ;
		m_lruPointer.NewSlot( iPtrReg, regCode ) ;
		//
		// 範囲設定
		//
		m_lruPointer[iPtrReg].offsetFirst = offsetFirst ;
		m_lruPointer[iPtrReg].offsetEnd = offsetEnd ;
		if ( m_lruPointer[iPtrReg].pdwFirst != NULL )
		{
			*(m_lruPointer[iPtrReg].pdwFirst) = offsetFirst ;
		}
		if ( m_lruPointer[iPtrReg].pdwLimit != NULL )
		{
			*(m_lruPointer[iPtrReg].pdwLimit) = offsetEnd - offsetFirst ;
		}
	}
	else
	{
		//
		// 既に割り当てられたレジスタの境界チェックコード更新
		//
		m_lruPointer.RecentlyAccess( iPtrReg ) ;
		if ( !m_lruPointer[iPtrReg].fTLBFetched )
		{
			CommitRealizePointerRegister
				( m_lruPointer[iPtrReg], offsetFirst, offsetEnd ) ;
		}
		else
		{
			// TLB にプリフェッチされている場合
			ESLAssert( !(iPtrReg & 0xFE) ) ;
			ESLAssert( regIndex < 0 ) ;
			WriteToLoadSakura2Register( x86_EDX, x86_EAX, regPtr, true ) ;
			m_lruPointer[iPtrReg].regPhyIndex = x86_EDX ;
		}
	}
	return	iPtrReg ;
}

// Sakura2 汎用レジスタを x86 汎用レジスタにロードするコードを出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToLoadSakura2Register
	( X86GenericAssembler::X86Register regPhyLow,
		X86GenericAssembler::X86Register regPhyHigh,
		int regSakura, bool fOnlyLow )
{
	WriteX86LoadRegMem
		( regPhyLow, x86_EBX,
			Context::OffsetOfReg(regSakura) ) ;
	if ( !fOnlyLow )
	{
		WriteX86LoadRegMem
			( regPhyHigh, x86_EBX,
				Context::OffsetOfReg(regSakura) + 4 ) ;
	}
}

void X86GenericAssembler::WriteToLoadSakura2AddressRegister
	( X86GenericAssembler::X86Register regPhyLow,
		X86GenericAssembler::X86Register regPhyHigh,
		int regBasePtr, int regIndex, int scale )
{
	if ( regIndex >= 0 )
	{
		FlushRegister( regBasePtr ) ;
		WriteToLoadSakura2Register( regPhyLow, regPhyHigh, regIndex ) ;
		if ( scale > 0 )
		{
			WriteX86ShldRegRegImm8( regPhyHigh, regPhyLow, scale ) ;
			WriteX86ShlRegImm8( regPhyLow, scale ) ;
		}
		WriteX86AddRegMem
			( regPhyLow, true, x86_EBX,
				Context::OffsetOfReg(regBasePtr) ) ;
		WriteX86AdcRegMem
			( regPhyHigh, true, x86_EBX,
				Context::OffsetOfReg(regBasePtr) + 4 ) ;
	}
	else
	{
		WriteToLoadSakura2Register( regPhyLow, regPhyHigh, regBasePtr ) ;
	}
}

// x86 汎用レジスタから Sakura2 汎用レジスタへストアするコードを出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToStoreSakura2Register
	( int regSakura, X86GenericAssembler::X86Register regPhyLow,
			X86GenericAssembler::X86Register regPhyHigh, bool fOnlyLow )
{
	WriteX86StoreRegMem
		( regPhyLow, x86_EBX,
			Context::OffsetOfReg(regSakura) ) ;
	if ( !fOnlyLow )
	{
		WriteX86StoreRegMem
			( regPhyHigh, x86_EBX,
				Context::OffsetOfReg(regSakura) + 4 ) ;
	}
}

// 仮想アドレスを実アドレスに変換するコードを出力
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteToTranslateAddress
	( RealizePointerBoundary& rpb,
		X86GenericAssembler::X86Register regPhyBase,
		X86GenericAssembler::X86Register regPhyLow,
		X86GenericAssembler::X86Register regPhyHigh, int slotTLB )
{
	//
	// TLB 比較
	//
	WriteX86LoadRegMem
		( regPhyBase, x86_EBX,
			Context::OffsetofStoreCache_pbytBuffer(slotTLB) ) ;
	WriteX86SubRegMem
		( regPhyLow, true, x86_EBX,
			Context::OffsetofStoreCache_baseOffset(slotTLB) ) ;
	WriteX86CmpRegMem
		( regPhyHigh, true, x86_EBX,
			Context::OffsetofStoreCache_highAddress(slotTLB) ) ;
	void *	pJneTLB = WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	void *	pNextAddr = GetNextAddress() ;
	ESLAssert( m_bufSub != NULL ) ;
	m_buf = m_bufSub ;
	CommitJumpTarget( pJneTLB, GetNextAddress() ) ;
	//
	// 第二 TLB 比較
	//
	WriteX86AddRegMem
		( regPhyLow, true, x86_EBX,
			Context::OffsetofStoreCache_baseOffset(slotTLB) ) ;
	WriteX86MoveRegImm32( regPhyBase, 0x03 ) ;
	WriteX86AndRegMem( regPhyBase, false, regPhyHigh ) ;
	WriteX86ImulRegImm32( regPhyBase, sizeof(LinearAddressCache) ) ;
	//
	WriteX86CmpRegMem
		( regPhyHigh, true, x86_EBX,
			offsetof(Context,m_segLoadCache[0].highAddress), regPhyBase, 0 ) ;
	void *	pJeTLB2 = WriteX86ImmediateOperand( x86op2_JE, 2, 0, 4 ) ;
	//
	// アドレス変換処理
	//
	WriteBackAllRegisters() ;
	//
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyECX )
						&& (regPhyBase != x86_ECX) )
	{
		WriteX86PushReg( x86_ECX ) ;
	}
	WriteX86PushReg( regPhyLow ) ;
	WriteX86PushReg( regPhyBase ) ;
	//
	WriteX86LeaRegMem
		( regPhyBase, x86_EBX,
			offsetof(Context,m_segLoadCache[0]), regPhyBase, 0 ) ;
	WriteX86PushReg( regPhyHigh ) ;
	WriteX86PushReg( regPhyLow ) ;
	WriteX86PushReg( regPhyBase ) ;
	WriteX86PushReg( x86_EBX ) ;
	//
	WriteX86CallRegMem
		( true, x86_EBX, offsetof(Context,m_pfnTranslate) ) ;
	WriteX86AddRegImm32( x86_ESP, 4 * 4 ) ;
	//
	WriteX86PopReg( regPhyBase ) ;
	WriteX86PopReg( regPhyLow ) ;
	//
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyECX )
						&& (regPhyBase != x86_ECX) )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	ReloadRegisters() ;
	//
	// 第二 TLB 複製
	//
	CommitJumpTarget( pJeTLB2, GetNextAddress() ) ;
	//
	WriteToCopyMemory
		( true, x86_EBX, Context::OffsetofStoreCache(slotTLB), x86_Nothing, 0,
			true, x86_EBX, offsetof(Context,m_segLoadCache[0]), regPhyBase, 0,
			sizeof(LinearAddressCache) / sizeof(DWORD), regPhyHigh ) ;
	//
	// TLB ベースアドレスをロード
	//
	WriteX86LoadRegMem
		( regPhyBase, x86_EBX,
			Context::OffsetofStoreCache_pbytBuffer(slotTLB) ) ;
	WriteX86SubRegMem
		( regPhyLow, true, x86_EBX,
			Context::OffsetofStoreCache_baseOffset(slotTLB) ) ;
	WriteToJump( pNextAddr ) ;
	m_buf = m_bufMain ;
	//
	WriteX86AddRegMem( regPhyBase, false, regPhyLow ) ;
	//
	// メモリ境界判定
	//
	return	WriteToCheckBoundaryAddress
		( rpb, regPhyLow, regPhyHigh,
			(int) Context::OffsetofStoreCache(slotTLB), false ) ;
}

// メモリ境界を判定するコードを出力
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteToCheckBoundaryAddress
	( RealizePointerBoundary& rpb,
		X86GenericAssembler::X86Register regPhyLow,
		X86GenericAssembler::X86Register regPhyTemp,
		int offsetTLB, bool fBaseOffset )
{
	void *	pEscFrom = NULL ;
	rpb.pdwFirst = NULL ;
	rpb.pdwLimit = NULL ;
	if ( !m_flagNoBoundary )
	{
		FlushAllRegisters() ;
		//
		if ( fBaseOffset )
		{
			WriteX86SubRegMem
				( regPhyLow, true, x86_EBX,
					offsetTLB + offsetof(LinearAddressCache,baseOffset) ) ;
		}
		rpb.pdwFirst =
			(SDWORD*) WriteX86AddRegImm32( regPhyLow, 0 ) ;
		WriteX86MoveRegReg( regPhyTemp, regPhyLow ) ;
		WriteX86SarRegImm8( regPhyLow, 31 ) ;
		rpb.pdwLimit =
			(SDWORD*) WriteX86AddRegImm32( regPhyTemp, 0 ) ;
		WriteX86OrRegMem( regPhyLow, false, regPhyTemp ) ;
		WriteX86CmpRegMem
			( regPhyLow, true, x86_EBX,
				offsetTLB + offsetof(LinearAddressCache,limitSegment) ) ;
		pEscFrom = WriteX86ImmediateOperand( x86op2_JA, 2, 0, 4 ) ;
	}
	return	pEscFrom ;
}

// メモリを複製するコードを出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToCopyMemory
	( bool fDstAligned,
		X86GenericAssembler::X86Register regPhyDst, INT_PTR dispDst,
			X86GenericAssembler::X86Register regPhyDstIndex, int scaleDstIndex,
		bool fSrcAligned,
		X86GenericAssembler::X86Register regPhySrc, INT_PTR dispSrc,
			X86GenericAssembler::X86Register regPhySrcIndex, int scaleSrcIndex,
		int	sizeInDWord, X86GenericAssembler::X86Register regPhyTemp )
{
	for ( int i = 0, iOffset = 0; i < sizeInDWord; i ++, iOffset += 4 )
	{
		WriteX86LoadRegMem
			( regPhyTemp, regPhySrc,
				dispSrc + iOffset, regPhySrcIndex, scaleSrcIndex ) ;
		WriteX86StoreRegMem
			( regPhyTemp, regPhyDst,
				dispDst + iOffset, regPhyDstIndex, scaleDstIndex ) ;
	}
}

// 例外マスクに追加するコードを出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToAtomicOrExceptionMask( DWORD dwException )
{
	if ( dwException != 0 )
	{
		WriteX86RegMemOperand
			( x86op1_OP_RM_IMM32 | (x86op1_LOCK << 8), 2, x86op2nd_OR,
				true, x86_EBX, offsetof(Context,m_maskException),
				x86_Nothing, 0, dwException, 4 ) ;
	}
}

// スタック拡張例外判定
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteToStackException
	( int& regPhyNewLowSP, int nOffsetSP, const void * ptrEpilogue )
{
	FlushAllRegisters() ;
	//
	regPhyNewLowSP = x86_EAX ;
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSP, true ) ;
	if ( nOffsetSP != 0 )
	{
		WriteX86AddRegImm32( x86_EAX, nOffsetSP ) ;
	}
	WriteX86CmpRegMem
		( x86_EAX, true,
			x86_EBX, offsetof(Context,m_segStack.baseOffset) ) ;
	void *	pEscJumpFrom =
		WriteX86ImmediateOperand( x86op2_JB, 2, 0, 4 ) ;
	if ( ptrEpilogue != NULL )
	{
		CommitJumpTarget( pEscJumpFrom, ptrEpilogue ) ;
	}
	return	pEscJumpFrom ;
}

// ゼロ除算例外用ゼロ比較脱出コード出力
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteToZeroDivisionException
	( int regDiv, const void * ptrEpilogue )
{
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regDiv ) ;
	WriteX86OrRegMem( x86_EAX, false, x86_EDX ) ;
	void *	pJneNoEsc =
		WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	//
	WriteBackAllRegisters() ;
	void *	pEscJumpFrom = WriteX86JmpImm32( ptrEpilogue ) ;
	//
	CommitJumpTarget( pJneNoEsc, GetNextAddress() ) ;
	return	pEscJumpFrom ;
}

void * X86GenericAssembler::WriteToZeroDivisionException32
	( int regDiv, const void * ptrEpilogue )
{
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regDiv, true ) ;
	WriteX86OrRegMem( x86_EAX, false, x86_EAX ) ;
	void *	pJneNoEsc =
		WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	//
	WriteBackAllRegisters() ;
	void *	pEscJumpFrom = WriteX86JmpImm32( ptrEpilogue ) ;
	//
	CommitJumpTarget( pJneNoEsc, GetNextAddress() ) ;
	return	pEscJumpFrom ;
}

// メモリ読み込み命令出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToLoadPhysicalMemory
	( int regDst, int regPhyPtr, int offset, DataType type, bool fPair )
{
	const int	sizeOfData = sizeof_prim_data[type] ;
	int			sizeOfLoadData = sizeOfData ;
	if ( fPair && (type == dataInt64) )
	{
		sizeOfLoadData *= 2 ;
	}
	X86Register	regPhyBase = x86_EBP ;
	X86Register	regPhyIndex = x86_Nothing ;
	if ( regPhyPtr != regPtrPhyBPIndex )
	{
		CommitRealizePointerRegister
			( m_lruPointer[regPhyPtr],
				offset, offset + sizeOfLoadData ) ;
		regPhyBase = (X86Register) m_lruPointer[regPhyPtr].regPhy ;
		if ( m_lruPointer[regPhyPtr].fTLBFetched )
		{
			regPhyIndex = (X86Register) m_lruPointer[regPhyPtr].regPhyIndex ;
			m_lruPointer[regPhyPtr].regPhyIndex = x86_Nothing ;
			if ( fPair && (type == dataInt64)
							&& (regPhyIndex != x86_Nothing) )
			{
				m_lruPointer.FreeSlot( regPtrPhyECX ) ;
				WriteX86LeaRegMem( x86_ECX, regPhyBase, 0, regPhyIndex ) ;
				regPhyBase = x86_ECX ;
				regPhyIndex = x86_Nothing ;
			}
		}
	}
	bool	fStoreReg = false ;
	switch ( type )
	{
	case	dataInt64:
	default:
		if ( regPhyIndex == x86_EAX )
		{
			WriteX86LoadRegMem
				( x86_EDX, regPhyBase, offset + 4, regPhyIndex ) ;
			WriteX86LoadRegMem
				( x86_EAX, regPhyBase, offset, regPhyIndex ) ;
		}
		else
		{
			WriteX86LoadRegMem
				( x86_EAX, regPhyBase, offset, regPhyIndex ) ;
			WriteX86LoadRegMem
				( x86_EDX, regPhyBase, offset + 4, regPhyIndex ) ;
		}
		break ;
	case	dataInt32:
		WriteX86LoadRegMem
			( x86_EAX, regPhyBase, offset, regPhyIndex ) ;
		WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
		break ;
	case	dataInt16:
		WriteX86RegMemOperand
			( x86op2_MOVSX_16, 2,
				x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
		WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
		break ;
	case	dataInt8:
		WriteX86RegMemOperand
			( x86op2_MOVSX_8, 2,
				x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
		WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
		break ;
	case	dataFloat:
		ResetRegister( regDst ) ;
		WriteX86RegMemOperand
			( x86op1_FLD_FP32, 1, x86op2nd_FLD,
				true, regPhyBase, offset ) ;
		WriteX86RegMemOperand
			( x86op1_FST_FP64, 1, x86op2nd_FSTP,
				true, x86_EBX,
				Context::OffsetOfReg(regDst), regPhyIndex ) ;
		fStoreReg = true ;
		break ;
	case	dataUint32:
		WriteX86LoadRegMem
			( x86_EAX, regPhyBase, offset, regPhyIndex ) ;
		WriteX86RegMemOperand
			( x86op1_XOR_REG_RM, 1, x86_EDX, false, x86_EDX ) ;
		break ;
	case	dataUint16:
		WriteX86RegMemOperand
			( x86op2_MOVZX_16, 2,
				x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
		WriteX86RegMemOperand
			( x86op1_XOR_REG_RM, 1, x86_EDX, false, x86_EDX ) ;
		break ;
	case	dataUint8:
		WriteX86RegMemOperand
			( x86op2_MOVZX_8, 2,
				x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
		WriteX86RegMemOperand
			( x86op1_XOR_REG_RM, 1, x86_EDX, false, x86_EDX ) ;
		break ;
	}
	if ( !fStoreReg )
	{
		WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
	}
	if ( fPair )
	{
		switch ( type )
		{
		case	dataInt64:
		default:
			WriteX86LoadRegMem
				( x86_EAX, regPhyBase, offset + sizeOfData, regPhyIndex ) ;
			WriteX86LoadRegMem
				( x86_EDX, regPhyBase, offset + sizeOfData + 4, regPhyIndex ) ;
			WriteToStoreSakura2Register( regDst + 1, x86_EAX, x86_EDX ) ;
			break ;
		case	dataUint32:
			WriteX86RegMemOperand
				( x86op1_XOR_REG_RM, 1, x86_EAX, false, x86_EAX ) ;
			WriteX86RegMemOperand
				( x86op1_XOR_REG_RM, 1, x86_EDX, false, x86_EDX ) ;
			WriteToStoreSakura2Register( regDst + 1, x86_EAX, x86_EDX ) ;
			break ;
		}
	}
}

// メモリ書き出し命令出力
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WriteToStorePhysicalMemory
	( int regSrc, int regPhyPtr, int offset, DataType type, bool fPair )
{
	const int	sizeOfData = sizeof_prim_data[type] ;
	const int	countPair = fPair ? 2 : 1 ;
	X86Register	regPhyBase = x86_EBP ;
	X86Register	regPhyIndex = x86_Nothing ;
	if ( regPhyPtr != regPtrPhyBPIndex )
	{
		CommitRealizePointerRegister
			( m_lruPointer[regPhyPtr], offset,
					offset + sizeOfData * countPair ) ;
		regPhyBase = (X86Register) m_lruPointer[regPhyPtr].regPhy ;
		if ( m_lruPointer[regPhyPtr].fTLBFetched )
		{
			regPhyIndex = (X86Register) m_lruPointer[regPhyPtr].regPhyIndex ;
			m_lruPointer[regPhyPtr].regPhyIndex = x86_Nothing ;
			if ( (regPhyIndex == x86_EAX)
				|| ((type == dataInt64) && (regPhyIndex != x86_Nothing)) )
			{
				m_lruPointer.FreeSlot( regPtrPhyECX ) ;
				WriteX86LeaRegMem( x86_ECX, regPhyBase, 0, regPhyIndex ) ;
				regPhyBase = x86_ECX ;
				regPhyIndex = x86_Nothing ;
			}
		}
	}
	for ( int i = 0; i < countPair; i ++ )
	{
		switch ( type )
		{
		case	dataInt64:
		default:
			WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc + i ) ;
			WriteX86StoreRegMem
				( x86_EAX, regPhyBase, offset + i * sizeOfData, regPhyIndex ) ;
			WriteX86StoreRegMem
				( x86_EDX, regPhyBase, offset + i * sizeOfData + 4, regPhyIndex ) ;
			break ;
		case	dataUint32:
		case	dataInt32:
			WriteToLoadSakura2Register
				( x86_EAX, x86_EDX, regSrc + i, true ) ;
			WriteX86StoreRegMem
				( x86_EAX, regPhyBase, offset + i * sizeOfData, regPhyIndex ) ;
			break ;
		case	dataUint16:
		case	dataInt16:
			WriteToLoadSakura2Register
				( x86_EAX, x86_EDX, regSrc + i, true ) ;
			WriteX86RegMemOperand
				( x86op1_MOV_STORE | 0x6600, 2,
					x86_EAX, true, regPhyBase,
					offset + i * sizeOfData, regPhyIndex ) ;
			break ;
		case	dataUint8:
		case	dataInt8:
			WriteToLoadSakura2Register
				( x86_EAX, x86_EDX, regSrc + i, true ) ;
			WriteX86RegMemOperand
				( x86op1_MOV_STORE_8, 1,
					x86_AL, true, regPhyBase,
					offset + i * sizeOfData, regPhyIndex ) ;
			break ;
		case	dataFloat:
			FlushRegister( regSrc + i ) ;
			WriteX86RegMemOperand
				( x86op1_FLD_FP64, 1, x86op2nd_FLD,
					true, x86_EBX,
					Context::OffsetOfReg(regSrc + i), regPhyIndex ) ;
			WriteX86RegMemOperand
				( x86op1_FST_FP32, 1, x86op2nd_FSTP,
					true, regPhyBase,
					offset + i * sizeOfData, regPhyIndex ) ;
			ResetRegister( regSrc + i ) ;
			break ;
		}
	}
}

// 無条件ジャンプコード出力
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteToJump( const void * ptrTarget )
{
	return	WriteX86JmpImm32( ptrTarget ) ;
}

// 条件ジャンプコード出力
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteToConditionalJump
		( int reg, bool fLogic, const void * ptrTarget )
{
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, reg, true ) ;
	WriteX86RegImmOperand
		( x86op1_TEST_RM_IMM32, 1, x86op2nd_TEST_RM_IMM32, x86_EAX, 1, 4 ) ;
	void *	pJumpFrom = NULL ;
	if ( fLogic )
	{
		pJumpFrom = WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	}
	else
	{
		pJumpFrom = WriteX86ImmediateOperand( x86op2_JE, 2, 0, 4 ) ;
	}
	if ( ptrTarget != NULL )
	{
		CommitJumpTarget( pJumpFrom, ptrTarget ) ;
	}
	return	pJumpFrom ;
}

// 例外判定離脱コード出力
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::WriteToEscapeByException( const void * ptrEpilogue )
{
	WriteX86LoadRegMem
		( x86_EAX, x86_EBX, offsetof(Context,m_maskException) ) ;
	WriteX86OrRegMem
		( x86_EAX, true, x86_Nothing, (INT_PTR) &maskGlobalInterrupt ) ;
	//
	void *	pJumpFrom = WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	if ( ptrEpilogue != NULL )
	{
		CommitJumpTarget( pJumpFrom, ptrEpilogue ) ;
	}
	return	pJumpFrom ;
}

// ジャンプコード完成（２パス用）
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::CommitJumpTarget
		( void * ptrJumpFrom, const void * ptrFixedTarget )
{
	INT_PTR	ptrJumpOrg = ((INT_PTR) ptrJumpFrom) + 4 ;
	*((DWORD*)ptrJumpFrom) =
		(DWORD) (((INT_PTR) ptrFixedTarget) - ptrJumpOrg) ;
}

// データ移動命令
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::write_move_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( regDst != regSrc )
	{
		const int	nCount = fPair ? 2 : 1 ;
		for ( int i = 0; i < nCount; i ++ )
		{
			WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc + i ) ;
			WriteToStoreSakura2Register( regDst + i, x86_EAX, x86_EDX ) ;
		}
	}
}

// シフト命令
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::write_srl_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	const int	nCount = fPair ? 2 : 1 ;
	for ( int i = 0; i < nCount; i ++ )
	{
		WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc + i ) ;
		if ( imm8 >= 32 )
		{
			WriteX86MoveRegReg( x86_EAX, x86_EDX ) ;
			WriteX86XorRegMem( x86_EDX, false, x86_EDX ) ;
			WriteX86ShrRegImm8( x86_EAX, imm8 - 32 ) ;
		}
		else
		{
			WriteX86ShrdRegRegImm8( x86_EAX, x86_EDX, imm8 ) ;
			WriteX86ShrRegImm8( x86_EDX, imm8 ) ;
		}
		WriteToStoreSakura2Register( regDst + i, x86_EAX, x86_EDX ) ;
	}
}

void X86GenericAssembler::write_sra_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	const int	nCount = fPair ? 2 : 1 ;
	for ( int i = 0; i < nCount; i ++ )
	{
		WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc + i ) ;
		if ( imm8 >= 32 )
		{
			WriteX86MoveRegReg( x86_EAX, x86_EDX ) ;
			WriteX86SarRegImm8( x86_EDX, 31 ) ;
			WriteX86ShrRegImm8( x86_EAX, imm8 - 32 ) ;
		}
		else
		{
			WriteX86ShrdRegRegImm8( x86_EAX, x86_EDX, imm8 ) ;
			WriteX86SarRegImm8( x86_EDX, imm8 ) ;
		}
		WriteToStoreSakura2Register( regDst + i, x86_EAX, x86_EDX ) ;
	}
}

void X86GenericAssembler::write_sll_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	const int	nCount = fPair ? 2 : 1 ;
	for ( int i = 0; i < nCount; i ++ )
	{
		WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc + i ) ;
		if ( imm8 >= 32 )
		{
			WriteX86MoveRegReg( x86_EDX, x86_EAX ) ;
			WriteX86XorRegMem( x86_EAX, false, x86_EAX ) ;
			WriteX86ShlRegImm8( x86_EDX, imm8 - 32 ) ;
		}
		else
		{
			WriteX86ShldRegRegImm8( x86_EDX, x86_EAX, imm8 ) ;
			WriteX86ShlRegImm8( x86_EAX, imm8 ) ;
		}
		WriteToStoreSakura2Register( regDst + i, x86_EAX, x86_EDX ) ;
	}
}

// 32ビット即値命令
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::write_add_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair )
{
	const int	nCount = fPair ? 2 : 1 ;
	for ( int i = 0; i < nCount; i ++ )
	{
		WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc + i ) ;
		WriteX86AddRegImm32( x86_EAX, imm32 ) ;
		WriteX86AdcRegImm32( x86_EDX, (imm32 >> 31) ) ;
		WriteToStoreSakura2Register( regDst + i, x86_EAX, x86_EDX ) ;
	}
}

// スタックレジスタ加算命令
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::write_add_sp_imm32( int ip, int imm32 )
{
	FlushAllRegisters() ;
	//
	if ( imm32 != 0 )
	{
		WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSP, true ) ;
		WriteX86AddRegImm32( x86_EAX, imm32 ) ;
		WriteToStoreSakura2Register( regSP, x86_EAX, x86_EDX, true ) ;
	}
	if ( imm32 < 0 )
	{
		int	regPhy ;
		return	WriteToStackException( regPhy, 0, NULL ) ;
	}
	else
	{
		return	NULL ;
	}
}

// 64ビット即値命令
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::write_move_reg_imm64( int regDst, INT64 imm64 )
{
	WriteX86MoveRegImm32( x86_EAX, (DWORD) imm64 ) ;
	WriteX86MoveRegImm32( x86_EDX, (DWORD) (imm64 >> 32) ) ;
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

// 間接無条件ジャンプ／コール命令
//（exceptionFarJump 例外の判定と設定を含む）
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::write_jump_reg( int reg )
{
	FlushAllRegisters() ;
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, reg ) ;
	WriteX86CmpRegMem
		( x86_EDX, true, x86_EBX, offsetof(Context,m_ipSegment) ) ;
	void *	pJeIPSeg = WriteX86ImmediateOperand( x86op2_JE, 2, 0, 4 ) ;
	//
	WriteToAtomicOrExceptionMask( exceptionFarJump ) ;
	WriteX86StoreRegMem( x86_EDX, x86_EBX, offsetof(Context,m_ipSegment) ) ;
	//
	CommitJumpTarget( pJeIPSeg, GetNextAddress() ) ;
	//
	WriteX86StoreRegMem( x86_EAX, x86_EBX, offsetof(Context,m_ip) ) ;
}

// システムコール命令
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::write_syscall_imm( int imm32 )
{
	WriteToAtomicOrExceptionMask( exceptionSystemCall ) ;
	WriteX86MoveRegImm32( x86_EAX, imm32 ) ;
	WriteX86StoreRegMem( x86_EAX, x86_EBX, offsetof(Context,m_idSystemCall) ) ;
}

void X86GenericAssembler::write_syscall_reg( int reg )
{
	WriteToAtomicOrExceptionMask( exceptionSystemCall ) ;
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, reg, true ) ;
	WriteX86StoreRegMem( x86_EAX, x86_EBX, offsetof(Context,m_idSystemCall) ) ;
}

// リターン命令（exceptionFarJump 例外の判定と設定を含む）
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::write_return( void )
{
	void *	pEscJumpFrom1 = NULL ;
	void *	pEscJumpFrom2 = NULL ;
	//
	FlushAllRegisters() ;
	ResetAllRegisters() ;
	//
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSP, true ) ;
	WriteX86LeaRegMem( x86_ECX, x86_EAX, 8 ) ;
	//
	// バウンダリチェック
	//
	WriteX86SubRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.baseOffset) ) ;
	if ( !m_flagNoBoundary )
	{
		pEscJumpFrom1 =
			WriteX86ImmediateOperand( x86op2_JB, 2, 0, 4 ) ;
		WriteX86LeaRegMem( x86_EDX, x86_EAX, 8 ) ;
		WriteX86CmpRegMem
			( x86_EDX, true, x86_EBX,
				offsetof(Context,m_segStack.limitSegment) ) ;
		pEscJumpFrom2 =
			WriteX86ImmediateOperand( x86op2_JA, 2, 0, 4 ) ;
	}
	//
	// スタック POP & SP レジスタ更新
	//
	WriteX86AddRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.pbytBuffer) ) ;
	WriteToStoreSakura2Register( regSP, x86_ECX, x86_EDX, true ) ;
	WriteX86LoadRegMem( x86_EDX, x86_EAX, 4 ) ;
	WriteX86LoadRegMem( x86_EAX, x86_EAX, 0 ) ;
	//
	// far jump 例外判定
	//
	WriteX86CmpRegMem
		( x86_EDX, true, x86_EBX, offsetof(Context,m_ipSegment) ) ;
	WriteX86StoreRegMem( x86_EAX, x86_EBX, offsetof(Context,m_ip) ) ;
	void *	pJneIPSeg = WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	//
	WriteX86LoadRegMem
		( x86_ECX, x86_EBX, offsetof(Context,m_ptrCode) ) ;
	WriteX86RegMemOperand
		( x86op1_OP_RM_IMM8, 1, x86op2nd_CMP, true,
			x86_ECX, 0, x86_EAX, 0, codeSystemReserved, 1 ) ;
	void *	pJneCodeFF = WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	//
	X86Register	regJumpAddr = x86_EAX ;
	if ( (m_abiFastcall.maxRegArg >= 1)
		&& (m_abiFastcall.regArg[0] == x86_EAX) )
	{
		regJumpAddr = x86_ECX ;
	}
	WriteX86LoadRegMem
		( x86_ECX, x86_EBX, offsetof(Context,m_ptrTrick) ) ;
	WriteX86LoadRegMem
		( regJumpAddr, x86_ECX, 0, x86_EAX, 0 ) ;
	//
	if ( m_abiFastcall.maxRegArg >= 1 )
	{
		if ( m_abiFastcall.regArg[0] != x86_EBX )
		{
			WriteX86MoveRegReg( m_abiFastcall.regArg[0], x86_EBX ) ;
		}
	}
	WriteX86PopReg( x86_EBX ) ;
	WriteX86PopReg( x86_ESI ) ;
	WriteX86PopReg( x86_EDI ) ;
	WriteX86PopReg( x86_EBP ) ;
	//
	WriteX86RegMemOperand( x86op1_JMP_RM, 1, 4, false, regJumpAddr ) ;
	//
	// バウンダリ例外
	//
	if ( !m_flagNoBoundary )
	{
		if ( pEscJumpFrom1 != NULL )
		{
			CommitJumpTarget( pEscJumpFrom1, GetNextAddress() ) ;
		}
		if ( pEscJumpFrom2 != NULL )
		{
			CommitJumpTarget( pEscJumpFrom2, GetNextAddress() ) ;
		}
		WriteToAtomicOrExceptionMask( exceptionReadMemory ) ;
		WriteToStoreSakura2Register( regSP, x86_ECX, x86_EDX, true ) ;
	}
	void *	pElseJumpFrom = WriteToJump( NULL ) ;
	//
	// far jump 時
	//
	CommitJumpTarget( pJneIPSeg, GetNextAddress() ) ;
	WriteToAtomicOrExceptionMask( exceptionFarJump ) ;
	CommitJumpTarget( pJneCodeFF, GetNextAddress() ) ;
	WriteX86StoreRegMem( x86_EDX, x86_EBX, offsetof(Context,m_ipSegment) ) ;
	//
	CommitJumpTarget( pElseJumpFrom, GetNextAddress() ) ;
}

// スタック処理
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::write_push_ip( int imm32 )
{
	int	regLowSP ;
	void *	pEscJumpFrom = WriteToStackException( regLowSP, -8, NULL ) ;
	//
	// アドレス変換
	//
	if ( regLowSP != x86_EAX )
	{
		/*
		WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSP, true ) ;
		WriteX86AddRegImm32( x86_EAX, -8 ) ;
		*/
		WriteX86MoveRegReg( x86_EAX, (X86Register) regLowSP ) ;
	}
	WriteToStoreSakura2Register( regSP, x86_EAX, x86_EDX, true ) ;
	//
	WriteX86SubRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.baseOffset) ) ;
	WriteX86AddRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.pbytBuffer) ) ;
	//
	// PUSH
	//
	WriteX86MoveRegImm32( x86_EDX, imm32 ) ;
	WriteX86StoreRegMem( x86_EDX, x86_EAX, 0 ) ;
	WriteX86LoadRegMem
		( x86_EDX, x86_EBX, offsetof(Context,m_ipSegment) ) ;
	WriteX86StoreRegMem( x86_EDX, x86_EAX, 4 ) ;
	//
	return	pEscJumpFrom ;
}

void * X86GenericAssembler::write_push_reg( int regFirst, int nCount )
{
	int	regLowSP ;
	FlushAllRegisters() ;
	void *	pEscJumpFrom = WriteToStackException( regLowSP, -8 * nCount, NULL ) ;
	//
	// アドレス変換
	//
	if ( regLowSP != x86_EAX )
	{
		/*
		WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSP, true ) ;
		WriteX86AddRegImm32( x86_EAX, -8 * nCount ) ;
		*/
		WriteX86MoveRegReg( x86_EAX, (X86Register) regLowSP ) ;
	}
	WriteToStoreSakura2Register( regSP, x86_EAX, x86_EDX, true ) ;
	//
	WriteX86SubRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.baseOffset) ) ;
	WriteX86AddRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.pbytBuffer) ) ;
	//
	// PUSH
	//
	WriteToCopyMemory
		( false, x86_EAX, 0, x86_Nothing, 0,
			true, x86_EBX, Context::OffsetOfReg(regFirst), x86_Nothing, 0,
			nCount * sizeof(Register) / sizeof(DWORD), x86_EDX ) ;
	//
	return	pEscJumpFrom ;
}

void * X86GenericAssembler::write_pop_reg( int regFirst, int nCount )
{
	//
	// アドレス変換
	//
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSP, true ) ;
	WriteX86SubRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.baseOffset) ) ;
	WriteX86AddRegMem
		( x86_EAX, true, x86_EBX,
			offsetof(Context,m_segStack.pbytBuffer) ) ;
	//
	// POP
	//
	for ( int i = 0; i < nCount; i ++ )
	{
		ResetRegister( regFirst + i ) ;
	}
	WriteToCopyMemory
		( true, x86_EBX, Context::OffsetOfReg(regFirst), x86_Nothing, 0,
			false, x86_EAX, 0, x86_Nothing, 0,
			nCount * sizeof(Register) / sizeof(DWORD), x86_EDX ) ;
	//
	// SP 更新
	//
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSP, true ) ;
	WriteX86AddRegImm32( x86_EAX, nCount * 8 ) ;
	WriteToStoreSakura2Register( regSP, x86_EAX, x86_EDX, true ) ;
	//
	return	NULL ;
}

// メモリヒント
//////////////////////////////////////////////////////////////////////////////
void * X86GenericAssembler::write_prefetch_tlb( int tlb, int reg )
{
	void *	ptrEscJump = NULL ;
	bool	fPrefetchTLB = false ;
	ESLAssert( !(tlb & ~0x01) ) ;
	tlb &= 0x01 ;
	if ( m_flagNoBoundary )
	{
		if ( (m_regTLBFetched[tlb] < 0)
				&& !m_lruPointer[tlb].fTLBFetched )
		{
			m_lruPointer.NewSlot( tlb, (reg | 0xFF00) ) ;
			m_lruPointer.LockSlot( tlb ) ;
			m_lruPointer[tlb].fTLBFetched = true ;
			m_lruPointer[tlb].regPhyIndex = x86_Nothing ;
			fPrefetchTLB = true ;
		}
	}
	ptrEscJump = Sakura2Assembler::write_prefetch_tlb( tlb, reg ) ;
	if ( fPrefetchTLB )
	{
		//
		// TLB に変換アドレスをロードし
		// 物理レジスタにベースアドレスを設定する
		//
		WritePrefetchTLB( tlb, reg ) ;
	}
	return	ptrEscJump ;
}

void X86GenericAssembler::write_unfetch_tlb( int tlb, int reg )
{
	ESLAssert( !(tlb & ~0x01) ) ;
	tlb &= 0x01 ;
	if ( (m_regTLBFetched[tlb] >= 0) && m_lruPointer[tlb].fTLBFetched )
	{
		m_lruPointer.UnlockSlot( tlb ) ;
		m_lruPointer.FreeSlot( tlb ) ;
		m_lruPointer[tlb].fTLBFetched = false ;
		m_lruPointer[tlb].regPhyIndex = x86_Nothing ;
	}
	Sakura2Assembler::write_unfetch_tlb( tlb, reg ) ;
}

// TLB を準備し物理レジスタにロードする
//////////////////////////////////////////////////////////////////////////////
void X86GenericAssembler::WritePrefetchTLB( int tlb, int reg )
{
	if ( !m_flagNoBoundary )
	{
		return ;
	}
	//
	// TLB に変換アドレスをロードし
	// 物理レジスタにベースアドレスを設定する
	//
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, reg ) ;
	//
	X86Register	regPhyBase = (X86Register) m_lruPointer[tlb].regPhy ;
	X86Register	regPhyHigh = x86_EDX ;
	X86Register	regPhyLow = x86_EAX ;
	//
	WriteX86CmpRegMem
		( regPhyHigh, true, x86_EBX,
			Context::OffsetofStoreCache_highAddress(tlb) ) ;
	void *	pJneTLB = WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	void *	pNextAddr = GetNextAddress() ;
	ESLAssert( m_bufSub != NULL ) ;
	m_buf = m_bufSub ;
	CommitJumpTarget( pJneTLB, GetNextAddress() ) ;
	//
	// 第二 TLB 比較
	//
	WriteX86MoveRegImm32( regPhyBase, 0x03 ) ;
	WriteX86AndRegMem( regPhyBase, false, regPhyHigh ) ;
	WriteX86ImulRegImm32( regPhyBase, sizeof(LinearAddressCache) ) ;
	//
	WriteX86CmpRegMem
		( regPhyHigh, true, x86_EBX,
			offsetof(Context,m_segLoadCache[0].highAddress), regPhyBase, 0 ) ;
	void *	pJeTLB2 = WriteX86ImmediateOperand( x86op2_JE, 2, 0, 4 ) ;
	//
	// アドレス変換処理
	//
	WriteBackAllRegisters() ;
	//
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyECX )
						&& (regPhyBase != x86_ECX) )
	{
		WriteX86PushReg( x86_ECX ) ;
	}
	WriteX86PushReg( regPhyBase ) ;
	//
	WriteX86LeaRegMem
		( regPhyBase, x86_EBX,
			offsetof(Context,m_segLoadCache[0]), regPhyBase, 0 ) ;
	WriteX86PushReg( regPhyHigh ) ;
	WriteX86PushReg( regPhyLow ) ;
	WriteX86PushReg( regPhyBase ) ;
	WriteX86PushReg( x86_EBX ) ;
	//
	WriteX86CallRegMem
		( true, x86_EBX, offsetof(Context,m_pfnTranslate) ) ;
	WriteX86AddRegImm32( x86_ESP, 4 * 4 ) ;
	//
	WriteX86PopReg( regPhyBase ) ;
	//
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyECX )
						&& (regPhyBase != x86_ECX) )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	ReloadRegisters() ;
	//
	// 第二 TLB 複製
	//
	CommitJumpTarget( pJeTLB2, GetNextAddress() ) ;
	//
	WriteToCopyMemory
		( true, x86_EBX, Context::OffsetofStoreCache(tlb), x86_Nothing, 0,
			true, x86_EBX, offsetof(Context,m_segLoadCache[0]), regPhyBase, 0,
			sizeof(LinearAddressCache) / sizeof(DWORD), regPhyHigh ) ;
	//
	// TLB ベースアドレスをロード
	//
	WriteToJump( pNextAddr ) ;
	m_buf = m_bufMain ;
	//
	WriteX86LoadRegMem
		( regPhyBase, x86_EBX,
			Context::OffsetofStoreCache_pbytBuffer(tlb) ) ;
	WriteX86SubRegMem
		( regPhyBase, true, x86_EBX,
			Context::OffsetofStoreCache_baseOffset(tlb) ) ;
}

#endif
