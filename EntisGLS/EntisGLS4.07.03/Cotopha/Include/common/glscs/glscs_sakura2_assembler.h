
/*****************************************************************************
			詞葉 naked モードプロセッサ Sakura2 アセンブラ
 *****************************************************************************/

#if	!defined(__SAKURA2_SCRIPT_ASSEMBLER__)
#define	__SAKURA2_SCRIPT_ASSEMBLER__

namespace	ECSSakura2Assember
{
	//////////////////////////////////////////////////////////////////////////
	// オペランド
	//////////////////////////////////////////////////////////////////////////

	struct	Operand
	{
		enum	Type
		{
			typeInvalid	= -1,
			typeInteger,
			typeReal,
			typeRegister,
			typeMemory,
			typeLabel,
			typeAddress,
		} ;
		struct	Memory
		{
			ECSSakura2Processor::DataType		typeData ;
			ECSSakura2Processor::AddressingMode	modeAddr ;
			int									regBase ;
			int									regIndex ;
			int									scaleIndex ;
			int32_t								offsetAddr ;

			Memory( void )
				: typeData(ECSSakura2Processor::dataInt64),
					modeAddr(ECSSakura2Processor::addrBase),
					regBase(-1), regIndex(-1), scaleIndex(0), offsetAddr(0) {}
		} ;
		Type	m_type ;
		int		m_reg ;
		int64_t	m_int ;
		double	m_real ;
		Memory	m_mem ;

		Operand( void )
			: m_type(typeInvalid), m_reg(-1), m_int(0), m_real(0) {}
	} ;

	// オペランド要素アセンブル
	int ParseRegister( SSystem::SStringParser& sparsOperand ) ;


	//////////////////////////////////////////////////////////////////////////
	// １行アセンブラ
	//////////////////////////////////////////////////////////////////////////

	struct	InstructionBuffer
	{
		BYTE	bufCode[0x20] ;
		size_t	nCodeBytes ;
		size_t	nTotalBytes ;
	} ;

	// 擬似命令
	enum	MacroInstruction
	{
		macroLea,
		macroInc,
		macroDec,
		macroFLoad32,
		macroFStore32,
		macroVUnpackL32,
		macroVUnpackH32,
		macroCount,
	} ;

	// 命令形式
	enum	InstructionFormatType
	{
		formInvalid			= -1,
		formOpRegRegImm32,	// regm reg [, imm32]
		formOpRegRegImm8,	// reg, reg [, imm8]
		formOpRegRegReg,
		formOpRegReg,
		formOpReg,
		formOpImm32,
		formNop,
		formLoad,
		formStore,
		formMove,		// move reg, {imm64|reg}
		formJump,		// {jump|call|syscall} {imm32|reg}
		formCJump,		// {cjump|cnjump} reg, imm32
		formPush,		// {push|pop} reg[,imm8]
		formTypeCount,
	} ;
	extern InstructionFormatType	formTypeByMnemonic[0x100] ;

	typedef  SSystem::SError (*PFUNC_ASSEMBLE_INSTRUCTION)
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	extern PFUNC_ASSEMBLE_INSTRUCTION
				pfnAssembleInstructionByMnemonicForm[formTypeCount] ;

	extern const char *	pszLoadMnemonicByType[ECSSakura2Processor::dataTypeMax] ;
	extern const char *	pszStoreMnemonicByType[ECSSakura2Processor::dataTypeMax] ;
	extern const char *	pszMacroInstructionNames[macroCount] ;
	extern PFUNC_ASSEMBLE_INSTRUCTION
							pfnMacroInstructionProc[macroCount] ;

	// ニーモニック比較
	bool CompareMnemonic
		( const char * pszRealName, const wchar_t * pszMnemonic ) ;

	// ニーモニック検索
	int FindMnemonic
		( const char ** pszRealNames, const wchar_t * pszMnemonic ) ;

	// １行アセンブル
	SSystem::SError AssembleInstruction
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;

	SSystem::SError AssembleInstructionOpRegRegImm32
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionOpRegRegImm8
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionOpRegRegReg
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionOpRegReg
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionOpReg
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionOpImm32
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionNop
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionLoad
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionStore
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionMove
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionVMove
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionJump
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionCJump
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionPush
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleInstructionMemoryHint
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMnemonic,
			const Operand * ptrOperands, size_t numOperands ) ;

	bool VerifyOperandTypeAsOpRegRegReg
		( SSystem::SString& strErr,
			const Operand * ptrOperands, size_t numOperands ) ;
	bool VerifyOperandTypeAsOpRegRegImm8
		( SSystem::SString& strErr,
			const Operand * ptrOperands, size_t numOperands ) ;
	bool VerifyOperandTypeAsOpRegImm8
		( SSystem::SString& strErr,
			const Operand * ptrOperands, size_t numOperands ) ;
	bool VerifyOperandTypeAsOpRegRegImm32
		( SSystem::SString& strErr,
			const Operand * ptrOperands, size_t numOperands ) ;
	bool VerifyOperandTypeAsOpRegImm32
		( SSystem::SString& strErr,
			const Operand * ptrOperands, size_t numOperands ) ;
	bool VerifyOperandTypeAsOpRegReg
		( SSystem::SString& strErr,
			const Operand * ptrOperands, size_t numOperands ) ;
	bool VerifyOperandTypeAsOpReg
		( SSystem::SString& strErr,
			const Operand * ptrOperands, size_t numOperands ) ;

	// 擬似命令マクロ
	SSystem::SError AssembleMacroInstructionLea
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMacro,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleMacroInstructionInc
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMacro,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleMacroInstructionDec
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMacro,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleMacroInstructionFLoad32
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMacro,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleMacroInstructionFStore32
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMacro,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleMacroInstructionVUnpackL32
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMacro,
			const Operand * ptrOperands, size_t numOperands ) ;
	SSystem::SError AssembleMacroInstructionVUnpackH32
		( InstructionBuffer& ibuf, SSystem::SString& strErr,
			const wchar_t * pszMnemonic, int iMacro,
			const Operand * ptrOperands, size_t numOperands ) ;

}

#endif
