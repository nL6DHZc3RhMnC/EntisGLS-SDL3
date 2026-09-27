
/*****************************************************************************
			詞葉 naked モードプロセッサ Sakura2 アセンブラ
 *****************************************************************************/

#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_assembler.h>

using namespace SSystem ;
using namespace ECSSakura2Processor ;
using namespace ECSSakura2Assember ;


//////////////////////////////////////////////////////////////////////////////
// オペランド
//////////////////////////////////////////////////////////////////////////////

// レジスタ解釈
//////////////////////////////////////////////////////////////////////////////
int ECSSakura2Assember::ParseRegister( SSystem::SStringParser& sparsOperand )
{
	if ( !sparsOperand.PassSpace() )
	{
		return	-1 ;
	}
	wchar_t	wch = sparsOperand.CurrentCharacter() ;
	if ( (wch == L'r') | (wch == L'R') )
	{
		//
		// r??? レジスタ番号形式
		//
		wch = sparsOperand.OffsetAt( 1 ) ;
		if ( (wch >= L'0') & (wch <= L'9') )
		{
			int	numReg = 0 ;
			sparsOperand.MarkIndex() ;
			sparsOperand.GetCharacter() ;
			do
			{
				wch = sparsOperand.GetCharacter() ;
				numReg = numReg * 10 + (wch - L'0') ;
				wch = sparsOperand.CurrentCharacter() ;
			}
			while ( (wch >= L'0') & (wch <= L'9') ) ;
			if ( ((wch >= L'A') && (wch <= L'Z'))
				|| ((wch >= L'a') && (wch <= L'z')) )
			{
				sparsOperand.SeekToMark() ;
				return	-1 ;
			}
			sparsOperand.ReleaseMark() ;
			return	numReg ;
		}
	}
	//
	// 特殊レジスタ名
	//
	static const wchar_t *	pwszSpecialRegNames[] =
	{
		L"acc", L"sp", L"bp", L"tp", L"xp", L"yp", L"zp",
		L"#zero", L"#one", L"#fill", L"#ffffffff",
		L"#ffff", L"#ff", L"#1.0", L"#pi",
		NULL
	} ;
	static const RegisterIndex	regSpecialRegIndexes[] =
	{
		regAcc, regSP, regBP, regTP, regXP, regYP, regZeroPtr,
		regIntZero, regIntOne, regFillBit, regMaskLow32,
		regMaskLow16, regMaskLow8, regFloatOne, regFloatPI,
	} ;
	sparsOperand.MarkIndex() ;
	for ( int i = 0; pwszSpecialRegNames[i] != NULL; i ++ )
	{
		if ( sparsOperand.HasToComeNoCaseString( pwszSpecialRegNames[i] ) )
		{
			wch = sparsOperand.CurrentCharacter() ;
			if ( ((wch >= L'0') && (wch <= L'9'))
				|| ((wch >= L'A') && (wch <= L'Z'))
				|| ((wch >= L'a') && (wch <= L'z')) )
			{
				sparsOperand.SeekToMark() ;
				return	-1 ;
			}
			sparsOperand.ReleaseMark() ;
			return	regSpecialRegIndexes[i] ;
		}
	}
	sparsOperand.ReleaseMark() ;
	return	-1 ;
}


//////////////////////////////////////////////////////////////////////////////
// １行アセンブラ
//////////////////////////////////////////////////////////////////////////////

InstructionFormatType	ECSSakura2Assember::formTypeByMnemonic[0x100] =
{
	// 0x00
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x10
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x20
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x30
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x40
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x50
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x60
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x70
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x80
	formLoad, formLoad, formLoad, formLoad,
	formStore, formStore, formStore, formStore,
	formLoad, formLoad, formStore, formStore,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0x90
	formMove, formInvalid, formOpRegReg, formOpRegReg,
	formOpRegRegImm8, formOpRegRegImm8, formOpRegRegImm8, formOpRegRegReg,
	formOpRegRegImm32, formOpRegRegImm32, formOpImm32, formMove,
	formOpReg, formOpReg, formOpReg, formInvalid,
	// 0xA0
	formOpRegRegImm32, formOpRegReg, formOpRegRegImm32, formOpRegReg,
	formOpRegReg, formOpRegReg, formOpRegReg, formOpRegReg,
	formOpRegRegImm8, formOpRegRegImm8, formOpRegRegImm8, formOpRegReg,
	formOpRegReg, formOpRegReg, formInvalid, formInvalid,
	// 0xB0
	formOpRegReg, formOpRegReg, formOpRegReg, formOpRegReg,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formOpRegReg, formOpRegReg, formOpRegReg, formOpRegReg,
	formOpRegReg, formOpRegReg, formInvalid, formInvalid,
	// 0xC0
	formOpRegReg, formOpRegReg, formOpRegReg, formOpRegReg,
	formOpRegReg, formOpRegReg, formOpRegReg, formOpRegReg,
	formOpRegReg, formOpRegReg, formOpRegReg, formOpRegReg,
	formOpRegReg, formOpRegReg, formInvalid, formInvalid,
	// 0xD0
	formJump, formJump, formCJump, formCJump,
	formJump, formJump, formJump, formJump,
	formNop, formInvalid, formInvalid, formInvalid,
	formPush, formPush, formPush, formPush,
	// 0xE0
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	// 0xF0
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formInvalid, formInvalid,
	formInvalid, formInvalid, formNop, formInvalid,
} ;

PFUNC_ASSEMBLE_INSTRUCTION
	ECSSakura2Assember::pfnAssembleInstructionByMnemonicForm[formTypeCount] =
{
	&ECSSakura2Assember::AssembleInstructionOpRegRegImm32,
	&ECSSakura2Assember::AssembleInstructionOpRegRegImm8,
	&ECSSakura2Assember::AssembleInstructionOpRegRegReg,
	&ECSSakura2Assember::AssembleInstructionOpRegReg,
	&ECSSakura2Assember::AssembleInstructionOpReg,
	&ECSSakura2Assember::AssembleInstructionOpImm32,
	&ECSSakura2Assember::AssembleInstructionNop,
	&ECSSakura2Assember::AssembleInstructionLoad,
	&ECSSakura2Assember::AssembleInstructionStore,
	&ECSSakura2Assember::AssembleInstructionMove,
	&ECSSakura2Assember::AssembleInstructionJump,
	&ECSSakura2Assember::AssembleInstructionCJump,
	&ECSSakura2Assember::AssembleInstructionPush,
} ;

const char *	ECSSakura2Assember::pszLoadMnemonicByType[ECSSakura2Processor::dataTypeMax] =
{
	"load.64", "load.int32", "load.int16", "load.int8",
	"load.float", "load.uint32", "load.uint16", "load.uint8",
} ;

const char *	ECSSakura2Assember::pszStoreMnemonicByType[ECSSakura2Processor::dataTypeMax] =
{
	"store.64", "store.int32", "store.int16", "store.int8",
	"store.float", "store.uint32", "store.uint16", "store.uint8",
} ;

const char *	ECSSakura2Assember::pszMacroInstructionNames[macroCount] =
{
	"lea", "inc", "dec", "fload.32", "fstore.32", "vunpack.l32", "vunpack.h32",
} ;

PFUNC_ASSEMBLE_INSTRUCTION
	ECSSakura2Assember::pfnMacroInstructionProc[macroCount] =
{
	&ECSSakura2Assember::AssembleMacroInstructionLea,
	&ECSSakura2Assember::AssembleMacroInstructionInc,
	&ECSSakura2Assember::AssembleMacroInstructionDec,
	&ECSSakura2Assember::AssembleMacroInstructionFLoad32,
	&ECSSakura2Assember::AssembleMacroInstructionFStore32,
	&ECSSakura2Assember::AssembleMacroInstructionVUnpackL32,
	&ECSSakura2Assember::AssembleMacroInstructionVUnpackH32,
} ;

// ニーモニック比較
//////////////////////////////////////////////////////////////////////////////
bool ECSSakura2Assember::CompareMnemonic
	( const char * pszRealName, const wchar_t * pszMnemonic )
{
	size_t	i = 0, j = 0 ;
	for ( ; ; )
	{
		wchar_t	wch1 = pszRealName[i ++] ;
		wchar_t	wch2 = pszMnemonic[j] ;
		if ( (wch1 == L'.') & (wch2 != L'.') )
		{
			continue ;
		}
		j ++ ;
		if ( (L'a' <= wch1) & (wch1 <= L'z') )
		{
			wch1 -= L'a' - L'A' ;
		}
		if ( (L'a' <= wch2) & (wch2 <= L'z') )
		{
			wch2 -= L'a' - L'A' ;
		}
		if ( wch1 != wch2 )
		{
			return	false ;
		}
		if ( wch1 == 0 )
		{
			break ;
		}
	}
	return	true ;
}

// ニーモニック検索
//////////////////////////////////////////////////////////////////////////////
int ECSSakura2Assember::FindMnemonic
	( const char ** pszRealNames, const wchar_t * pszMnemonic )
{
	for ( int i = 0; i < 0x100; i ++ )
	{
		if ( pszRealNames[i] != NULL )
		{
			if ( CompareMnemonic( pszRealNames[i], pszMnemonic ) )
			{
				return	i ;
			}
		}
	}
	return	-1 ;
}

// １行アセンブル
//////////////////////////////////////////////////////////////////////////////
SError ECSSakura2Assember::AssembleInstruction
	( InstructionBuffer& ibuf, SString& strErr,
		const wchar_t * pszMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	int	iCode = FindMnemonic
		( pszInstructionMnemonic, pszMnemonic ) ;
	if ( iCode >= 0 )
	{
		InstructionFormatType	formType = formTypeByMnemonic[iCode & 0xFF] ;
		PFUNC_ASSEMBLE_INSTRUCTION
			pfnAssembler = pfnAssembleInstructionByMnemonicForm[formType] ;
		return	pfnAssembler
					( ibuf, strErr, pszMnemonic,
							iCode, ptrOperands, numOperands ) ;
	}
	iCode = FindMnemonic
		( pszSIMD64Extension3OpMnemonic, pszMnemonic ) ;
	if ( iCode >= 0 )
	{
		if ( !VerifyOperandTypeAsOpRegRegImm8
					( strErr, ptrOperands, numOperands ) )
		{
			ibuf.bufCode[0] = (BYTE) codeSIMD64Extension3Op ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_reg ;
			ibuf.bufCode[4] = (BYTE) ptrOperands[2].m_int ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 5 ;
			return	errSuccess ;
		}
		else if ( !VerifyOperandTypeAsOpRegImm8
					( strErr, ptrOperands, numOperands ) )
		{
			ibuf.bufCode[0] = (BYTE) codeSIMD64Extension3Op ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[4] = (BYTE) ptrOperands[1].m_int ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 5 ;
			return	errSuccess ;
		}
		else if ( iCode == simdPshufwImm8 )
		{
			return	errFailed ;
		}
	}
	iCode = FindMnemonic
		( pszSIMD64Extension2OpMnemonic, pszMnemonic ) ;
	if ( iCode >= 0 )
	{
		if ( !VerifyOperandTypeAsOpRegReg
					( strErr, ptrOperands, numOperands ) )
		{
			ibuf.bufCode[0] = (BYTE) codeSIMD64Extension2Op ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_reg ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 4 ;
			return	errSuccess ;
		}
		return	errFailed ;
	}
	iCode = FindMnemonic
		( pszSIMD128Extension3OpMnemonic, pszMnemonic ) ;
	if ( iCode >= 0 )
	{
		if ( iCode == simdVmaskmove )
		{
			 if ( !VerifyOperandTypeAsOpRegRegReg
					( strErr, ptrOperands, numOperands ) )
			{
				if ( (ptrOperands[0].m_reg & 0x01)
					| (ptrOperands[1].m_reg & 0x01)
					| (ptrOperands[2].m_reg & 0x01) )
				{
					strErr = L"奇数レジスタをオペランドにできません" ;
					return	errFailed ;
				}
				ibuf.bufCode[0] = (BYTE) codeSIMD128Extension3Op ;
				ibuf.bufCode[1] = (BYTE) iCode ;
				ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
				ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_reg ;
				ibuf.bufCode[4] = (BYTE) ptrOperands[2].m_reg ;
				ibuf.nCodeBytes = 5 ;
				ibuf.nTotalBytes = 5 ;
				return	errSuccess ;
			}
		}
		else if ( !VerifyOperandTypeAsOpRegRegImm8
					( strErr, ptrOperands, numOperands ) )
		{
			if ( (ptrOperands[0].m_reg & 0x01)
				| (ptrOperands[1].m_reg & 0x01) )
			{
				strErr = L"奇数レジスタをオペランドにできません" ;
				return	errFailed ;
			}
			ibuf.bufCode[0] = (BYTE) codeSIMD128Extension3Op ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_reg ;
			ibuf.bufCode[4] = (BYTE) ptrOperands[2].m_int ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 5 ;
			return	errSuccess ;
		}
		else if ( !VerifyOperandTypeAsOpRegImm8
					( strErr, ptrOperands, numOperands ) )
		{
			if ( ptrOperands[0].m_reg & 0x01 )
			{
				strErr = L"奇数レジスタをオペランドにできません" ;
				return	errFailed ;
			}
			ibuf.bufCode[0] = (BYTE) codeSIMD128Extension3Op ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[4] = (BYTE) ptrOperands[1].m_int ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 5 ;
			return	errSuccess ;
		}
		return	errFailed ;
	}
	iCode = FindMnemonic
		( pszSIMD128Extension2OpMnemonic, pszMnemonic ) ;
	if ( iCode >= 0 )
	{
		if ( !VerifyOperandTypeAsOpRegReg
					( strErr, ptrOperands, numOperands ) )
		{
			if ( (ptrOperands[0].m_reg & 0x01)
				| (ptrOperands[1].m_reg & 0x01) )
			{
				strErr = L"奇数レジスタをオペランドにできません" ;
				return	errFailed ;
			}
			ibuf.bufCode[0] = (BYTE) codeSIMD128Extension2Op ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_reg ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 4 ;
			return	errSuccess ;
		}
		else if ( iCode == simdVmove )
		{
			return	AssembleInstructionVMove
				( ibuf, strErr, pszMnemonic,
					simdVmove, ptrOperands, numOperands ) ;
		}
		return	errFailed ;
	}
	iCode = FindMnemonic
		( pszFloatExtensionMnemonic, pszMnemonic ) ;
	if ( iCode >= 0 )
	{
		if ( !VerifyOperandTypeAsOpReg
					( strErr, ptrOperands, numOperands ) )
		{
			ibuf.bufCode[0] = (BYTE) codeFloatExtension ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 4 ;
			return	errSuccess ;
		}
		if ( !VerifyOperandTypeAsOpRegReg
					( strErr, ptrOperands, numOperands ) )
		{
			ibuf.bufCode[0] = (BYTE) codeFloatExtension ;
			ibuf.bufCode[1] = (BYTE) iCode ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_reg ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 4 ;
			return	errSuccess ;
		}
		return	errFailed ;
	}
	iCode = FindMnemonic
		( pszMemoryHintExtensionMnemonic, pszMnemonic ) ;
	if ( iCode >= 0 )
	{
		return	AssembleInstructionMemoryHint
					( ibuf, strErr, pszMnemonic,
							iCode, ptrOperands, numOperands ) ;
	}
	for ( int iType = 0; iType < dataTypeMax; iType ++ )
	{
		if ( CompareMnemonic( pszLoadMnemonicByType[iType], pszMnemonic ) )
		{
			return	AssembleInstructionLoad
				( ibuf, strErr, pszMnemonic,
					codeLoadMem, ptrOperands, numOperands ) ;
		}
		else if ( CompareMnemonic( pszStoreMnemonicByType[iType], pszMnemonic ) )
		{
			return	AssembleInstructionStore
				( ibuf, strErr, pszMnemonic,
					codeStoreMem, ptrOperands, numOperands ) ;
		}
	}
	for ( int iMacro = 0; iMacro < macroCount; iMacro ++ )
	{
		if ( CompareMnemonic( pszMacroInstructionNames[iMacro], pszMnemonic ) )
		{
			return	(*pfnMacroInstructionProc[iMacro])
				( ibuf, strErr, pszMnemonic, iMacro, ptrOperands, numOperands ) ;
		}
	}
	strErr = L"不正なニーモニックです" ;
	return	errFailed ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionOpRegRegImm32
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( !VerifyOperandTypeAsOpRegRegImm32
				( strErr, ptrOperands, numOperands ) )
	{
		switch ( iMnemonic )
		{
		case	codeAddReg:
			iMnemonic = codeAddImm32 ;
			break ;
		case	codeMulReg:
			iMnemonic = codeMulImm32 ;
			break ;
		}
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
		ibuf.bufCode[3] = (BYTE) ptrOperands[2].m_int ;
		ibuf.bufCode[4] = (BYTE) (ptrOperands[2].m_int >> 8) ;
		ibuf.bufCode[5] = (BYTE) (ptrOperands[2].m_int >> 16) ;
		ibuf.bufCode[6] = (BYTE) (ptrOperands[2].m_int >> 24) ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 7 ;
		return	errSuccess ;
	}
	if ( !VerifyOperandTypeAsOpRegReg
				( strErr, ptrOperands, numOperands ) )
	{
		switch ( iMnemonic )
		{
		case	codeAddImm32:
			iMnemonic = codeAddReg ;
			break ;
		case	codeMulImm32:
			iMnemonic = codeMulReg ;
			break ;
		}
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		return	errSuccess ;
	}
	if ( !VerifyOperandTypeAsOpRegImm32
				( strErr, ptrOperands, numOperands ) )
	{
		switch ( iMnemonic )
		{
		case	codeAddReg:
			iMnemonic = codeAddImm32 ;
			break ;
		case	codeMulReg:
			iMnemonic = codeMulImm32 ;
			break ;
		}
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_int ;
		ibuf.bufCode[4] = (BYTE) (ptrOperands[1].m_int >> 8) ;
		ibuf.bufCode[5] = (BYTE) (ptrOperands[1].m_int >> 16) ;
		ibuf.bufCode[6] = (BYTE) (ptrOperands[1].m_int >> 24) ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 7 ;
		return	errSuccess ;
	}
	return	errFailed ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionOpRegRegImm8
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( !VerifyOperandTypeAsOpRegRegImm8
				( strErr, ptrOperands, numOperands ) )
	{
		switch ( iMnemonic )
		{
		case	codeSrlReg:
			iMnemonic = codeSrlImm8 ;
			break ;
		case	codeSraReg:
			iMnemonic = codeSraImm8 ;
			break ;
		case	codeSllReg:
			iMnemonic = codeSllImm8 ;
			break ;
		}
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
		ibuf.bufCode[3] = (BYTE) ptrOperands[2].m_int ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 4 ;
		return	errSuccess ;
	}
	if ( !VerifyOperandTypeAsOpRegReg
				( strErr, ptrOperands, numOperands ) )
	{
		switch ( iMnemonic )
		{
		case	codeSrlImm8:
			iMnemonic = codeSrlReg ;
			break ;
		case	codeSraImm8:
			iMnemonic = codeSraReg ;
			break ;
		case	codeSllImm8:
			iMnemonic = codeSllReg ;
			break ;
		}
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		return	errSuccess ;
	}
	if ( !VerifyOperandTypeAsOpRegImm8
				( strErr, ptrOperands, numOperands ) )
	{
		switch ( iMnemonic )
		{
		case	codeSrlReg:
			iMnemonic = codeSrlImm8 ;
			break ;
		case	codeSraReg:
			iMnemonic = codeSraImm8 ;
			break ;
		case	codeSllReg:
			iMnemonic = codeSllImm8 ;
			break ;
		}
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_int ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 4 ;
		return	errSuccess ;
	}
	return	errFailed ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionOpRegRegReg
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( !VerifyOperandTypeAsOpRegRegReg
				( strErr, ptrOperands, numOperands ) )
	{
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
		ibuf.bufCode[3] = (BYTE) ptrOperands[2].m_reg ;
		ibuf.nCodeBytes = 4 ;
		ibuf.nTotalBytes = 4 ;
		return	errSuccess ;
	}
	return	errFailed ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionOpRegReg
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( !VerifyOperandTypeAsOpRegReg
				( strErr, ptrOperands, numOperands ) )
	{
		switch ( iMnemonic )
		{
		case	codeAddImm32:
			iMnemonic = codeAddReg ;
			break ;
		case	codeMulImm32:
			iMnemonic = codeMulReg ;
			break ;
		case	codeSrlImm8:
			iMnemonic = codeSrlReg ;
			break ;
		case	codeSraImm8:
			iMnemonic = codeSraReg ;
			break ;
		case	codeSllImm8:
			iMnemonic = codeSllReg ;
			break ;
		}
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		return	errSuccess ;
	}
	return	errFailed ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionOpReg
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( !VerifyOperandTypeAsOpReg
				( strErr, ptrOperands, numOperands ) )
	{
		ibuf.bufCode[0] = (BYTE) iMnemonic ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.nCodeBytes = 2 ;
		ibuf.nTotalBytes = 2 ;
		return	errSuccess ;
	}
	return	errFailed ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionOpImm32
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 1 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 1 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeInteger )
	{
		strErr = L"オペランドが整数値ではありません" ;
		return	errFailed ;
	}
	if ( (ptrOperands[0].m_int < - (INT64) 0x80000000UL)
		| (ptrOperands[0].m_int > 0x7FFFFFFF) )
	{
		strErr = L"数値が範囲外です" ;
		return	errFailed ;
	}
	ibuf.bufCode[0] = (BYTE) iMnemonic ;
	ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_int ;
	ibuf.bufCode[2] = (BYTE) (ptrOperands[0].m_int >> 8) ;
	ibuf.bufCode[3] = (BYTE) (ptrOperands[0].m_int >> 16) ;
	ibuf.bufCode[4] = (BYTE) (ptrOperands[0].m_int >> 24) ;
	ibuf.nCodeBytes = 1 ;
	ibuf.nTotalBytes = 5 ;
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionNop
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 0 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	ibuf.bufCode[0] = (BYTE) iMnemonic ;
	ibuf.nCodeBytes = 1 ;
	ibuf.nTotalBytes = 1 ;
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionLoad
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeMemory )
	{
		strErr = L"第二オペランドがメモリではありません" ;
		return	errFailed ;
	}
	const Operand&	opReg = ptrOperands[0] ;
	Operand			opMem = ptrOperands[1] ;
	for ( int i = 0; i < dataTypeMax; i ++ )
	{
		if ( CompareMnemonic( pszLoadMnemonicByType[i], pszMnemonic ) )
		{
			opMem.m_mem.typeData = (DataType) i ;
			break ;
		}
	}
	if ( opMem.m_mem.regBase == regBP )
	{
		int	offsetAddress = 0 ;
		switch ( opMem.m_mem.modeAddr )
		{
		case	addrBaseOffset32:
			offsetAddress = opMem.m_mem.offsetAddr ;
		case	addrBase:
			ibuf.bufCode[0] = (BYTE) codeLoadLocalImm32 ;
			ibuf.bufCode[1] =
				(BYTE) (opMem.m_mem.typeData
							| (opMem.m_mem.scaleIndex << 5)) ;
			ibuf.bufCode[2] = (BYTE) opReg.m_reg ;
			ibuf.bufCode[3] = (BYTE) offsetAddress ;
			ibuf.bufCode[4] = (BYTE) (offsetAddress >> 8) ;
			ibuf.bufCode[5] = (BYTE) (offsetAddress >> 16) ;
			ibuf.bufCode[6] = (BYTE) (offsetAddress >> 24) ;
			ibuf.nCodeBytes = 3 ;
			ibuf.nTotalBytes = 7 ;
			break ;
		case	addrBaseIndexOffset32:
			offsetAddress = opMem.m_mem.offsetAddr ;
		case	addrBaseIndex:
			ibuf.bufCode[0] = (BYTE) codeLoadLocalIndexImm32 ;
			ibuf.bufCode[1] =
				(BYTE) (opMem.m_mem.typeData
							| (opMem.m_mem.scaleIndex << 5)) ;
			ibuf.bufCode[2] = (BYTE) opMem.m_mem.regIndex ;
			ibuf.bufCode[3] = (BYTE) opReg.m_reg ;
			ibuf.bufCode[4] = (BYTE) offsetAddress ;
			ibuf.bufCode[5] = (BYTE) (offsetAddress >> 8) ;
			ibuf.bufCode[6] = (BYTE) (offsetAddress >> 16) ;
			ibuf.bufCode[7] = (BYTE) (offsetAddress >> 24) ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 8 ;
			break ;
		}
	}
	else
	{
		int	i = 2 ;
		ibuf.bufCode[0] =
			(BYTE) (codeLoadMem | opMem.m_mem.modeAddr) ;
		switch ( opMem.m_mem.modeAddr )
		{
		case	addrBase:
		case	addrBaseOffset32:
			ibuf.bufCode[1] =
				(BYTE) ((opMem.m_mem.regBase << 3)
							| opMem.m_mem.typeData) ;
			break ;
		case	addrBaseIndex:
		case	addrBaseIndexOffset32:
			ibuf.bufCode[1] =
				(BYTE) (((opMem.m_mem.scaleIndex & 0x02) << 6)
							| (opMem.m_mem.regBase << 3)
							| opMem.m_mem.typeData) ;
			ibuf.bufCode[i ++] =
				(BYTE) ((opMem.m_mem.regIndex & 0x7F)
							| (opMem.m_mem.scaleIndex << 7)) ;
			break ;
		}
		ibuf.bufCode[i ++] = (BYTE) opReg.m_reg ;
		ibuf.nCodeBytes = i ;
		//
		switch ( opMem.m_mem.modeAddr )
		{
		case	addrBaseOffset32:
		case	addrBaseIndexOffset32:
			ibuf.bufCode[i ++] = (BYTE) opMem.m_mem.offsetAddr ;
			ibuf.bufCode[i ++] = (BYTE) (opMem.m_mem.offsetAddr >> 8) ;
			ibuf.bufCode[i ++] = (BYTE) (opMem.m_mem.offsetAddr >> 16) ;
			ibuf.bufCode[i ++] = (BYTE) (opMem.m_mem.offsetAddr >> 24) ;
			break ;
		default:
			break ;
		}
		ibuf.nTotalBytes = i ;
	}
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionStore
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeMemory )
	{
		strErr = L"第一オペランドがメモリではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	Operand			opMem = ptrOperands[0] ;
	const Operand&	opReg = ptrOperands[1] ;
	for ( int i = 0; i < dataTypeMax; i ++ )
	{
		if ( CompareMnemonic( pszStoreMnemonicByType[i], pszMnemonic ) )
		{
			opMem.m_mem.typeData = (DataType) i ;
			break ;
		}
	}
	if ( opMem.m_mem.regBase == regBP )
	{
		int	offsetAddress = 0 ;
		switch ( opMem.m_mem.modeAddr )
		{
		case	addrBaseOffset32:
			offsetAddress = opMem.m_mem.offsetAddr ;
		case	addrBase:
			ibuf.bufCode[0] = (BYTE) codeStoreLocalImm32 ;
			ibuf.bufCode[1] =
				(BYTE) (opMem.m_mem.typeData
							| (opMem.m_mem.scaleIndex << 5)) ;
			ibuf.bufCode[2] = (BYTE) opReg.m_reg ;
			ibuf.bufCode[3] = (BYTE) offsetAddress ;
			ibuf.bufCode[4] = (BYTE) (offsetAddress >> 8) ;
			ibuf.bufCode[5] = (BYTE) (offsetAddress >> 16) ;
			ibuf.bufCode[6] = (BYTE) (offsetAddress >> 24) ;
			ibuf.nCodeBytes = 3 ;
			ibuf.nTotalBytes = 7 ;
			break ;
		case	addrBaseIndexOffset32:
			offsetAddress = opMem.m_mem.offsetAddr ;
		case	addrBaseIndex:
			ibuf.bufCode[0] = (BYTE) codeStoreLocalIndexImm32 ;
			ibuf.bufCode[1] =
				(BYTE) (opMem.m_mem.typeData
							| (opMem.m_mem.scaleIndex << 5)) ;
			ibuf.bufCode[2] = (BYTE) opMem.m_mem.regIndex ;
			ibuf.bufCode[3] = (BYTE) opReg.m_reg ;
			ibuf.bufCode[4] = (BYTE) offsetAddress ;
			ibuf.bufCode[5] = (BYTE) (offsetAddress >> 8) ;
			ibuf.bufCode[6] = (BYTE) (offsetAddress >> 16) ;
			ibuf.bufCode[7] = (BYTE) (offsetAddress >> 24) ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 8 ;
			break ;
		}
	}
	else
	{
		int	i = 2 ;
		ibuf.bufCode[0] =
			(BYTE) (codeStoreMem | opMem.m_mem.modeAddr) ;
		switch ( opMem.m_mem.modeAddr )
		{
		case	addrBase:
		case	addrBaseOffset32:
			ibuf.bufCode[1] =
				(BYTE) ((opMem.m_mem.regBase << 3)
							| opMem.m_mem.typeData) ;
			break ;
		case	addrBaseIndex:
		case	addrBaseIndexOffset32:
			ibuf.bufCode[1] =
				(BYTE) (((opMem.m_mem.scaleIndex & 0x02) << 6)
							| (opMem.m_mem.regBase << 3)
							| opMem.m_mem.typeData) ;
			ibuf.bufCode[i ++] =
				(BYTE) ((opMem.m_mem.regIndex & 0x7F)
							| (opMem.m_mem.scaleIndex << 7)) ;
			break ;
		}
		ibuf.bufCode[i ++] = (BYTE) opReg.m_reg ;
		ibuf.nCodeBytes = i ;
		//
		switch ( opMem.m_mem.modeAddr )
		{
		case	addrBaseOffset32:
		case	addrBaseIndexOffset32:
			ibuf.bufCode[i ++] = (BYTE) opMem.m_mem.offsetAddr ;
			ibuf.bufCode[i ++] = (BYTE) (opMem.m_mem.offsetAddr >> 8) ;
			ibuf.bufCode[i ++] = (BYTE) (opMem.m_mem.offsetAddr >> 16) ;
			ibuf.bufCode[i ++] = (BYTE) (opMem.m_mem.offsetAddr >> 24) ;
			break ;
		default:
			break ;
		}
		ibuf.nTotalBytes = i ;
	}
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionMove
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type == Operand::typeMemory )
	{
		if ( ptrOperands[1].m_type != Operand::typeRegister )
		{
			strErr = L"第二オペランドがレジスタではありません" ;
			return	errFailed ;
		}
		return	AssembleInstructionStore
					( ibuf, strErr, L"store",
						codeStoreMem, ptrOperands, numOperands ) ;
	}
	else if ( ptrOperands[0].m_type == Operand::typeRegister )
	{
		if ( ptrOperands[1].m_type == Operand::typeMemory )
		{
			return	AssembleInstructionLoad
						( ibuf, strErr, L"load",
							codeLoadMem, ptrOperands, numOperands ) ;
		}
		else if ( ptrOperands[1].m_type == Operand::typeRegister )
		{
			ibuf.bufCode[0] = (BYTE) codeMoveReg ;
			ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
			ibuf.nCodeBytes = 3 ;
			ibuf.nTotalBytes = 3 ;
		}
		else if ( ptrOperands[1].m_type == Operand::typeInteger )
		{
			ibuf.bufCode[0] = (BYTE) codeLoadImm64 ;
			ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
			*((int64_t*)&(ibuf.bufCode[2])) = ptrOperands[1].m_int ;
			ibuf.nCodeBytes = 2 ;
			ibuf.nTotalBytes = 10 ;
		}
		else if ( ptrOperands[1].m_type == Operand::typeReal )
		{
			ibuf.bufCode[0] = (BYTE) codeLoadImm64 ;
			ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
			*((double*)&(ibuf.bufCode[2])) = ptrOperands[1].m_real ;
			ibuf.nCodeBytes = 2 ;
			ibuf.nTotalBytes = 10 ;
		}
		else if ( ptrOperands[1].m_type == Operand::typeAddress )
		{
			ibuf.bufCode[0] = (BYTE) codeLoadImm64 ;
			ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
			*((int64_t*)&(ibuf.bufCode[2])) = ptrOperands[1].m_int ;
			ibuf.nCodeBytes = 2 ;
			ibuf.nTotalBytes = 10 ;
		}
		else
		{
			strErr = L"第二オペランドが不正です" ;
			return	errFailed ;
		}
	}
	else
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionVMove
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	InstructionBuffer	ibufTemp ;
	Operand				oprTemp[2] ;
	SError				err ;
	oprTemp[0] = ptrOperands[0] ;
	oprTemp[1] = ptrOperands[1] ;
	if ( oprTemp[0].m_type == Operand::typeMemory )
	{
		if ( oprTemp[1].m_type != Operand::typeRegister )
		{
			strErr = L"第二オペランドがレジスタではありません" ;
			return	errFailed ;
		}
		oprTemp[0].m_mem.typeData = dataInt64 ;
		//
		err = AssembleInstructionMove
			( ibuf, strErr, L"store", codeStoreMem, oprTemp, 2 ) ;
		if ( err )
		{
			return	err ;
		}
		switch ( oprTemp[0].m_mem.modeAddr )
		{
		case	addrBase:
			oprTemp[0].m_mem.modeAddr = addrBaseOffset32 ;
			oprTemp[0].m_mem.offsetAddr = 8 ;
			break ;
		case	addrBaseIndex:
			oprTemp[0].m_mem.modeAddr = addrBaseIndexOffset32 ;
			oprTemp[0].m_mem.offsetAddr = 8 ;
			break ;
		case	addrBaseOffset32:
		case	addrBaseIndexOffset32:
			oprTemp[0].m_mem.offsetAddr += 8 ;
			break ;
		}
		oprTemp[1].m_reg += 1 ;
		err = AssembleInstructionMove
			( ibufTemp, strErr, L"store", codeStoreMem, oprTemp, 2 ) ;
		if ( err )
		{
			return	err ;
		}
		eslMoveMemory
			( &(ibuf.bufCode[ibuf.nTotalBytes]),
				&(ibufTemp.bufCode[0]), ibufTemp.nTotalBytes ) ;
		ibuf.nTotalBytes += ibufTemp.nTotalBytes ;
		ibuf.nCodeBytes = ibuf.nTotalBytes ;
		return	errSuccess ;
	}
	else if ( oprTemp[0].m_type == Operand::typeRegister )
	{
		if ( oprTemp[1].m_type == Operand::typeRegister )
		{
			if ( (oprTemp[0].m_reg & 0x01)
				|| (oprTemp[1].m_reg & 0x01) )
			{
				strErr = L"オペランドが奇数レジスタです" ;
				return	errFailed ;
			}
			ibuf.bufCode[0] = (BYTE) codeSIMD128Extension2Op ;
			ibuf.bufCode[1] = (BYTE) simdVmove ;
			ibuf.bufCode[2] = (BYTE) oprTemp[0].m_reg ;
			ibuf.bufCode[3] = (BYTE) oprTemp[1].m_reg ;
			ibuf.nCodeBytes = 4 ;
			ibuf.nTotalBytes = 4 ;
			return	errSuccess ;
		}
		if ( oprTemp[1].m_type != Operand::typeMemory )
		{
			strErr = L"オペランドが不正です" ;
			return	errFailed ;
		}
		oprTemp[1].m_mem.typeData = dataInt64 ;
		//
		err = AssembleInstructionMove
			( ibuf, strErr, L"load", codeLoadMem, oprTemp, 2 ) ;
		if ( err )
		{
			return	err ;
		}
		switch ( oprTemp[1].m_mem.modeAddr )
		{
		case	addrBase:
			oprTemp[1].m_mem.modeAddr = addrBaseOffset32 ;
			oprTemp[1].m_mem.offsetAddr = 8 ;
			break ;
		case	addrBaseIndex:
			oprTemp[1].m_mem.modeAddr = addrBaseIndexOffset32 ;
			oprTemp[1].m_mem.offsetAddr = 8 ;
			break ;
		case	addrBaseOffset32:
		case	addrBaseIndexOffset32:
			oprTemp[1].m_mem.offsetAddr += 8 ;
			break ;
		}
		oprTemp[0].m_reg += 1 ;
		err = AssembleInstructionMove
			( ibufTemp, strErr, L"load", codeLoadMem, oprTemp, 2 ) ;
		if ( err )
		{
			return	err ;
		}
		eslMoveMemory
			( &(ibuf.bufCode[ibuf.nTotalBytes]),
				&(ibufTemp.bufCode[0]), ibufTemp.nTotalBytes ) ;
		ibuf.nTotalBytes += ibufTemp.nTotalBytes ;
		ibuf.nCodeBytes = ibuf.nTotalBytes ;
		return	errSuccess ;
	}
	strErr = L"オペランドが不正です" ;
	return	errFailed ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionJump
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 1 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 1 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type == Operand::typeRegister )
	{
		ibuf.bufCode[0] = (BYTE) codeJumpReg ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.nCodeBytes = 2 ;
		ibuf.nTotalBytes = 2 ;
	}
	else if ( ptrOperands[0].m_type == Operand::typeLabel )
	{
		ibuf.bufCode[0] = (BYTE) codeJumpOffset32 ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_int ;
		ibuf.bufCode[2] = (BYTE) (ptrOperands[0].m_int >> 8) ;
		ibuf.bufCode[3] = (BYTE) (ptrOperands[0].m_int >> 16) ;
		ibuf.bufCode[4] = (BYTE) (ptrOperands[0].m_int >> 24) ;
		ibuf.nCodeBytes = 1 ;
		ibuf.nTotalBytes = 5 ;
	}
	else
	{
		strErr = L"オペランドが不正です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionCJump
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeLabel )
	{
		strErr = L"第二オペランドがラベルではありません" ;
		return	errFailed ;
	}
	ibuf.bufCode[0] = (BYTE) iMnemonic ;
	ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_int ;
	ibuf.bufCode[3] = (BYTE) (ptrOperands[1].m_int >> 8) ;
	ibuf.bufCode[4] = (BYTE) (ptrOperands[1].m_int >> 16) ;
	ibuf.bufCode[5] = (BYTE) (ptrOperands[1].m_int >> 24) ;
	ibuf.nCodeBytes = 2 ;
	ibuf.nTotalBytes = 6 ;
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionPush
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( numOperands < 1 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( numOperands == 2 )
	{
		if ( ptrOperands[1].m_type != Operand::typeInteger )
		{
			strErr = L"第二オペランドが整数ではありません" ;
			return	errFailed ;
		}
		if ( (ptrOperands[1].m_int < 0)
			|| (ptrOperands[1].m_int >= 0x100) )
		{
			strErr = L"第二オペランドが範囲外です" ;
			return	errFailed ;
		}
		if ( (iMnemonic == codePushReg) | (iMnemonic == codePushRegs) )
		{
			ibuf.bufCode[0] = (BYTE) codePushRegs ;
		}
		else
		{
			ibuf.bufCode[0] = (BYTE) codePopRegs ;
		}
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_int ;
		ibuf.nCodeBytes = 2 ;
		ibuf.nTotalBytes = 3 ;
	}
	else
	{
		if ( (iMnemonic == codePushReg) | (iMnemonic == codePushRegs) )
		{
			ibuf.bufCode[0] = (BYTE) codePushReg ;
		}
		else
		{
			ibuf.bufCode[0] = (BYTE) codePopReg ;
		}
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.nCodeBytes = 2 ;
		ibuf.nTotalBytes = 2 ;
	}
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleInstructionMemoryHint
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMnemonic,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( iMnemonic == mhcodeMemoryFence )
	{
		if ( numOperands > 0 )
		{
			strErr = L"オペランドが多すぎます" ;
			return	errFailed ;
		}
		ibuf.bufCode[0] = (BYTE) codeMemoryHint ;
		ibuf.bufCode[1] = (BYTE) mhcodeMemoryFence ;
		ibuf.bufCode[2] = 0 ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		return	errSuccess ;
	}
	else
	{
		if ( numOperands > 1 )
		{
			strErr = L"オペランドが多すぎます" ;
			return	errFailed ;
		}
		if ( ptrOperands[0].m_type != Operand::typeRegister )
		{
			strErr = L"オペランドがレジスタではありません" ;
			return	errFailed ;
		}
		ibuf.bufCode[0] = (BYTE) codeMemoryHint ;
		ibuf.bufCode[1] = (BYTE) iMnemonic ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ; ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		return	errSuccess ;
	}
}

bool ECSSakura2Assember::VerifyOperandTypeAsOpRegRegReg
	( SSystem::SString& strErr,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 3 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 3 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( (ptrOperands[0].m_type != Operand::typeRegister)
		|| (ptrOperands[1].m_type != Operand::typeRegister)
		|| (ptrOperands[2].m_type != Operand::typeRegister) )
	{
		strErr = L"オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

bool ECSSakura2Assember::VerifyOperandTypeAsOpRegRegImm8
	( SSystem::SString& strErr,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 3 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 3 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[2].m_type != Operand::typeInteger )
	{
		strErr = L"第三オペランドが整数ではありません" ;
		return	errFailed ;
	}
	if ( (ptrOperands[2].m_int < 0)
		|| (ptrOperands[2].m_int >= 0x100) )
	{
		strErr = L"整数値が範囲外です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

bool ECSSakura2Assember::VerifyOperandTypeAsOpRegImm8
	( SSystem::SString& strErr,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeInteger )
	{
		strErr = L"第二オペランドが整数ではありません" ;
		return	errFailed ;
	}
	if ( (ptrOperands[1].m_int < 0)
		|| (ptrOperands[1].m_int >= 0x100) )
	{
		strErr = L"整数値が範囲外です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

bool ECSSakura2Assember::VerifyOperandTypeAsOpRegRegImm32
	( SSystem::SString& strErr,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 3 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 3 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[2].m_type != Operand::typeInteger )
	{
		strErr = L"第三オペランドが整数ではありません" ;
		return	errFailed ;
	}
	if ( (ptrOperands[2].m_int < - (INT64) 0x80000000UL)
		|| (ptrOperands[2].m_int > 0x7FFFFFFF) )
	{
		strErr = L"整数値が範囲外です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

bool ECSSakura2Assember::VerifyOperandTypeAsOpRegImm32
	( SSystem::SString& strErr,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeInteger )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( (ptrOperands[1].m_int < - (INT64) 0x80000000UL)
		|| (ptrOperands[1].m_int > 0x7FFFFFFF) )
	{
		strErr = L"整数値が範囲外です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

bool ECSSakura2Assember::VerifyOperandTypeAsOpRegReg
	( SSystem::SString& strErr,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( (ptrOperands[0].m_type != Operand::typeRegister)
		|| (ptrOperands[1].m_type != Operand::typeRegister) )
	{
		strErr = L"オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

bool ECSSakura2Assember::VerifyOperandTypeAsOpReg
	( SSystem::SString& strErr,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 1 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 1 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// 擬似命令マクロ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSSakura2Assember::AssembleMacroInstructionLea
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMacro,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeMemory )
	{
		strErr = L"第二オペランドがメモリではありません" ;
		return	errFailed ;
	}
	int	iCode = 0 ;
	switch ( ptrOperands[1].m_mem.modeAddr )
	{
	case	addrBase:
		ibuf.bufCode[0] = (BYTE) codeMoveReg ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_mem.regBase ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		break ;
	case	addrBaseOffset32:
		ibuf.bufCode[0] = (BYTE) codeAddImm32 ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_mem.regBase ;
		ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_mem.offsetAddr ;
		ibuf.bufCode[4] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 8) ;
		ibuf.bufCode[5] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 16) ;
		ibuf.bufCode[6] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 24) ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 7 ;
		break ;
	case	addrBaseIndex:
	case	addrBaseIndexOffset32:
		if ( ptrOperands[1].m_mem.scaleIndex != 0 )
		{
			ibuf.bufCode[0] = (BYTE) codeSllImm8 ;
			ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_mem.regIndex ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_mem.scaleIndex ;
			iCode = 4 ;
			if ( (ptrOperands[1].m_mem.modeAddr == addrBaseIndexOffset32)
								&& (ptrOperands[1].m_mem.offsetAddr != 0) )
			{
				ibuf.bufCode[4] = (BYTE) codeAddImm32 ;
				ibuf.bufCode[5] = (BYTE) ptrOperands[0].m_reg ;
				ibuf.bufCode[6] = (BYTE) ptrOperands[0].m_reg ;
				ibuf.bufCode[7] = (BYTE) ptrOperands[1].m_mem.offsetAddr ;
				ibuf.bufCode[8] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 8) ;
				ibuf.bufCode[9] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 16) ;
				ibuf.bufCode[10] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 24) ;
				iCode = 11 ;
			}
		}
		else if ( (ptrOperands[1].m_mem.modeAddr == addrBaseIndexOffset32)
				&& (ptrOperands[1].m_mem.offsetAddr != 0) )
		{
			ibuf.bufCode[0] = (BYTE) codeAddImm32 ;
			ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_mem.regIndex ;
			ibuf.bufCode[3] = (BYTE) ptrOperands[1].m_mem.offsetAddr ;
			ibuf.bufCode[4] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 8) ;
			ibuf.bufCode[5] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 16) ;
			ibuf.bufCode[6] = (BYTE) (ptrOperands[1].m_mem.offsetAddr >> 24) ;
			iCode = 7 ;
		}
		else
		{
			ibuf.bufCode[0] = (BYTE) codeMoveReg ;
			ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
			ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_mem.regIndex ;
			iCode = 3 ;
		}
		ibuf.bufCode[iCode++] = (BYTE) codeAddReg ;
		ibuf.bufCode[iCode++] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[iCode++] = (BYTE) ptrOperands[1].m_mem.regBase ;
		ibuf.nCodeBytes = iCode ;
		ibuf.nTotalBytes = iCode ;
		break ;
	}
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleMacroInstructionInc
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMacro,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 1 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( numOperands == 1 )
	{
		// inc reg --> add reg, #one
		ibuf.bufCode[0] = codeAddReg ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) regIntOne ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		return	errSuccess ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeInteger )
	{
		strErr = L"第二オペランドが整数即値ではありません" ;
		return	errFailed ;
	}
	// inc reg, imm32 --> add reg, reg, imm32
	int32_t	imm32 = (int32_t) ptrOperands[1].m_int ;
	if ( (imm32 < - (INT64) 0x80000000UL) || (imm32 > (INT64) 0x7FFFFFFF) )
	{
		strErr = L"整数値が範囲外です" ;
		return	errFailed ;
	}
	ibuf.bufCode[0] = codeAddImm32 ;
	ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[3] = (BYTE) imm32 ;
	ibuf.bufCode[4] = (BYTE) (imm32 >> 8) ;
	ibuf.bufCode[5] = (BYTE) (imm32 >> 16) ;
	ibuf.bufCode[6] = (BYTE) (imm32 >> 24) ;
	ibuf.nCodeBytes = 3 ;
	ibuf.nTotalBytes = 7 ;
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleMacroInstructionDec
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMacro,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 1 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( numOperands == 1 )
	{
		// dec reg --> sub reg, #one
		ibuf.bufCode[0] = codeSubReg ;
		ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
		ibuf.bufCode[2] = (BYTE) regIntOne ;
		ibuf.nCodeBytes = 3 ;
		ibuf.nTotalBytes = 3 ;
		return	errSuccess ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeInteger )
	{
		strErr = L"第二オペランドが整数即値ではありません" ;
		return	errFailed ;
	}
	// dec reg, imm32 --> add reg, reg, -imm32
	int32_t	imm32 = - (int32_t) ptrOperands[1].m_int ;
	if ( (imm32 < - (INT64) 0x80000000UL) || (imm32 > (INT64) 0x7FFFFFFF) )
	{
		strErr = L"整数値が範囲外です" ;
		return	errFailed ;
	}
	ibuf.bufCode[0] = codeAddImm32 ;
	ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[2] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[3] = (BYTE) imm32 ;
	ibuf.bufCode[4] = (BYTE) (imm32 >> 8) ;
	ibuf.bufCode[5] = (BYTE) (imm32 >> 16) ;
	ibuf.bufCode[6] = (BYTE) (imm32 >> 24) ;
	ibuf.nCodeBytes = 3 ;
	ibuf.nTotalBytes = 7 ;
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleMacroInstructionFLoad32
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMacro,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第一オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeMemory )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_reg & 0x01 )
	{
		strErr = L"オペランドが奇数レジスタです" ;
		return	errFailed ;
	}
	SSystem::SError	err =
		AssembleInstructionLoad
			( ibuf, strErr, L"load.uint32",
				codeLoadMem, ptrOperands, numOperands ) ;
	if ( err )
	{
		return	err ;
	}
	size_t	iCode = ibuf.nTotalBytes ;
	ibuf.bufCode[iCode ++] = codeMoveReg ;
	ibuf.bufCode[iCode ++] = (BYTE) (ptrOperands[0].m_reg | 0x01) ;
	ibuf.bufCode[iCode ++] = (BYTE) regIntZero ;
	ibuf.nCodeBytes = iCode ;
	ibuf.nTotalBytes = iCode ;
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleMacroInstructionFStore32
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMacro,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeMemory )
	{
		strErr = L"第一オペランドがメモリではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_reg & 0x01 )
	{
		strErr = L"オペランドが奇数レジスタです" ;
		return	errFailed ;
	}
	return	AssembleInstructionStore
				( ibuf, strErr, L"store.uint32",
					codeStoreMem, ptrOperands, numOperands ) ;
}

SSystem::SError ECSSakura2Assember::AssembleMacroInstructionVUnpackL32
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMacro,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_reg & 0x01 )
	{
		strErr = L"オペランドが奇数レジスタです" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_reg & 0x01 )
	{
		strErr = L"オペランドが奇数レジスタです" ;
		return	errFailed ;
	}
	ibuf.bufCode[0] = (BYTE) codeMoveReg ;
	ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg + 1 ;
	ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg ;
	ibuf.bufCode[3] = (BYTE) codeSIMD128Extension3Op ;
	ibuf.bufCode[4] = (BYTE) simdVshuf32 ;
	ibuf.bufCode[5] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[6] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[7] = (BYTE) 0xD8 ;
	ibuf.nCodeBytes = 8 ;
	ibuf.nTotalBytes = 8 ;
	return	errSuccess ;
}

SSystem::SError ECSSakura2Assember::AssembleMacroInstructionVUnpackH32
	( InstructionBuffer& ibuf, SSystem::SString& strErr,
		const wchar_t * pszMnemonic, int iMacro,
		const Operand * ptrOperands, size_t numOperands )
{
	if ( numOperands < 2 )
	{
		strErr = L"オペランドが少なすぎます" ;
		return	errFailed ;
	}
	if ( numOperands > 2 )
	{
		strErr = L"オペランドが多すぎます" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_type != Operand::typeRegister )
	{
		strErr = L"第二オペランドがレジスタではありません" ;
		return	errFailed ;
	}
	if ( ptrOperands[0].m_reg & 0x01 )
	{
		strErr = L"オペランドが奇数レジスタです" ;
		return	errFailed ;
	}
	if ( ptrOperands[1].m_reg & 0x01 )
	{
		strErr = L"オペランドが奇数レジスタです" ;
		return	errFailed ;
	}
	ibuf.bufCode[0] = (BYTE) codeMoveReg ;
	ibuf.bufCode[1] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[2] = (BYTE) ptrOperands[1].m_reg + 1 ;
	ibuf.bufCode[3] = (BYTE) codeSIMD128Extension3Op ;
	ibuf.bufCode[4] = (BYTE) simdVshuf32 ;
	ibuf.bufCode[5] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[6] = (BYTE) ptrOperands[0].m_reg ;
	ibuf.bufCode[7] = (BYTE) 0x72 ;
	ibuf.nCodeBytes = 8 ;
	ibuf.nTotalBytes = 8 ;
	return	errSuccess ;
}

