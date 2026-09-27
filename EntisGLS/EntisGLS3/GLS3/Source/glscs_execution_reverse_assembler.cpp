
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ・逆アセンブラ
//////////////////////////////////////////////////////////////////////////////

const ECSExecutionReverseAssembler::PFUNC_REVERSE_ASSEMBLE
		ECSExecutionReverseAssembler::m_pfnReverseAssemble[csicMax] =
{
	/* cotopha 1.0 */
	&ECSExecutionReverseAssembler::ReverseAssembleNew,
	&ECSExecutionReverseAssembler::ReverseAssembleFree,
	&ECSExecutionReverseAssembler::ReverseAssembleLoad,
	&ECSExecutionReverseAssembler::ReverseAssembleStore,
	&ECSExecutionReverseAssembler::ReverseAssembleEnter,
	&ECSExecutionReverseAssembler::ReverseAssembleLeave,
	&ECSExecutionReverseAssembler::ReverseAssembleJump,
	&ECSExecutionReverseAssembler::ReverseAssembleCJump,
	&ECSExecutionReverseAssembler::ReverseAssembleCall,
	&ECSExecutionReverseAssembler::ReverseAssembleReturn,
	&ECSExecutionReverseAssembler::ReverseAssembleElement,
	&ECSExecutionReverseAssembler::ReverseAssembleElementIndirect,
	&ECSExecutionReverseAssembler::ReverseAssembleOperate,
	&ECSExecutionReverseAssembler::ReverseAssembleUniOperate,
	&ECSExecutionReverseAssembler::ReverseAssembleCompare,
	/* extended 2.0 */
	&ECSExecutionReverseAssembler::ReverseAssembleExOperate,
	&ECSExecutionReverseAssembler::ReverseAssembleExUniOperate,
	&ECSExecutionReverseAssembler::ReverseAssembleExCall,
	&ECSExecutionReverseAssembler::ReverseAssembleExReturn,
	&ECSExecutionReverseAssembler::ReverseAssembleCallMember,
	&ECSExecutionReverseAssembler::ReverseAssembleCallNativeMember,
	&ECSExecutionReverseAssembler::ReverseAssembleSwap,
	/* extended 2.3 */
	&ECSExecutionReverseAssembler::ReverseAssembleCreateBuffer,
	&ECSExecutionReverseAssembler::ReverseAssembleCreateBufferVSize,
	&ECSExecutionReverseAssembler::ReverseAssemblePointerToObject,
	&ECSExecutionReverseAssembler::ReverseAssemblePointerToAddress,
	&ECSExecutionReverseAssembler::ReverseAssembleReferenceForPointer,
	&ECSExecutionReverseAssembler::ReverseAssembleReferenceForObjPointer,
	&ECSExecutionReverseAssembler::ReverseAssembleCallFunctionPointer,
	&ECSExecutionReverseAssembler::ReverseAssembleCallNativeFunction,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSExecutionReverseAssembler, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionReverseAssembler::ECSExecutionReverseAssembler( ECSExecutionImage * pcsxi )
{
	m_pcsxi = pcsxi ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionReverseAssembler::~ECSExecutionReverseAssembler( void )
{
}

// 逆アセンブル
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssemble
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
				const BYTE * pbytCode, int addrCode, bool fMnemonic )
{
	//
	// 初期化
	//
	inf.nFlags = fMnemonic ? ECSSakura2Processor::flagMnemonic : 0 ;
	inf.nType = ECSSakura2Processor::typeObject ;
	inf.nBytes = 1 ;
	inf.regSrc1 = -1 ;
	inf.regSrc2 = -1 ;
	inf.regSrc3 = -1 ;
	inf.regDst = -1 ;
	inf.nAddress = addrCode ;
	//
	BYTE	bytCode = pbytCode[0] ;
	if ( bytCode < csicMax )
	{
		//
		// object モードニーモニック
		//
		(this->*m_pfnReverseAssemble[bytCode])
				( inf, pbytCode, addrCode, fMnemonic ) ;
		return ;
	}
	//
	// naked モード Sakura2 processor ニーモニック
	//
	ECSSakura2Processor::MnemonicInfo	minf ;
	minf.nFlags = fMnemonic ? ECSSakura2Processor::flagMnemonic : 0 ;
	//
	ECSSakura2Processor::GetInstructionInfo( &minf, pbytCode ) ;
	inf.nBytes = minf.nBytes ;
	inf.nType = minf.nType ;
	inf.regSrc1 = minf.regSrc1 ;
	inf.regSrc2 = minf.regSrc2 ;
	inf.regSrc3 = minf.regSrc3 ;
	inf.regDst = minf.regDst ;
	inf.strMnemonic = minf.szMnemonic ;
	inf.strOperand = minf.szOperand ;
	//
	// 特殊表示
	//
	switch ( bytCode )
	{
	case	ECSSakura2Processor::codeLoadImm64:
		{
			REAL64 *	pValue = (REAL64*) (pbytCode + 2) ;
			inf.strOperand += " (" ;
			inf.strOperand += EString( *pValue ) ;
			inf.strOperand += ")" ;
		}
		break ;
	case	ECSSakura2Processor::codeJumpOffset32:
		{
			SDWORD *	pValue = (SDWORD*) (pbytCode + 1) ;
			inf.strOperand += " (0x" ;
			inf.strOperand +=
				EString( (DWORD) (addrCode + inf.nBytes + *pValue), 8 ) ;
			inf.strOperand += ")" ;
		}
		break ;
	case	ECSSakura2Processor::codeCNJumpOffset32:
	case	ECSSakura2Processor::codeCJumpOffset32:
		{
			SDWORD *	pValue = (SDWORD*) (pbytCode + 2) ;
			inf.strOperand += " (0x" ;
			inf.strOperand +=
				EString( (DWORD) (addrCode + inf.nBytes + *pValue), 8 ) ;
			inf.strOperand += ")" ;
		}
		break ;
	case	ECSSakura2Processor::codeCallImm32:
		{
			DWORD *	pValue = (DWORD*) (pbytCode + 1) ;
			inf.strOperand += " (" ;
			inf.strOperand += EString( GetFunctionName( *pValue ) ) ;
			inf.strOperand += ")" ;
		}
		break ;
	case	ECSSakura2Processor::codeSysCallImm32:
		{
			DWORD *	pValue = (DWORD*) (pbytCode + 1) ;
			SSystem::SString *	pStrFunc =
				m_pcsxi->m_vectorSysCall.GetEntryIndex().GetAt( *pValue ) ;
			if ( pStrFunc != NULL )
			{
				inf.strOperand += " (" ;
				inf.strOperand += EString( *pStrFunc ) ;
				inf.strOperand += ")" ;
			}
		}
		break ;
	}
}

// 記憶クラス文字列変換
//////////////////////////////////////////////////////////////////////////////
const char * ECSExecutionReverseAssembler::GetMemoryClassName( CSObjectMode csomType )
{
	static const char *	pszMemoryClass[csomMax] =
	{
		"imm", "stack", "this", "gloabl", "static", "auto",
	} ;
	if ( (csomType >= 0) && (csomType < csomMax) )
	{
		return	pszMemoryClass[csomType] ;
	}
	return	"???" ;
}

// 型名文字列変換
//////////////////////////////////////////////////////////////////////////////
const char * ECSExecutionReverseAssembler::GetVariableTypeName( CSVariableType csvtType )
{
	static const char *	pszVariableType[] =
	{
		"Object", "Reference", "Array", "Hash", "Integer", "Real", "String",
		"Integer", "Pointer", "Object", "Boolean", "Int8", "Uint8", "Int16",
		"Uint16", "Int32", "Uint32", "???", "???", "float", "double", "???",
	} ;
	if ( (csvtType >= 0)
		&& (csvtType < sizeof(pszVariableType)/sizeof(pszVariableType[0])) )
	{
		return	pszVariableType[csvtType] ;
	}
	return	"???" ;
}

// 演算文字列変換
//////////////////////////////////////////////////////////////////////////////
const char * ECSExecutionReverseAssembler::GetOperatorTypeName( CSOperatorType csotType )
{
	static const char *	pszOperatorType[] =
	{
		"add", "sub", "mul", "div", "mod",
		"and", "or", "xor", "land", "lor",
		"sra", "sll",
	} ;
	if ( (csotType >= 0)
		&& (csotType < sizeof(pszOperatorType)/sizeof(pszOperatorType[0])) )
	{
		return	pszOperatorType[csotType] ;
	}
	return	"???" ;
}

const char * ECSExecutionReverseAssembler::GetUniOperatorTypeName( CSUnaryOperatorType csuotType )
{
	static const char *	pszOperatorType[] =
	{
		"plus", "negate", "not", "lnot",
		"inc", "dec", "inc", "dec",
	} ;
	if ( (csuotType >= 0)
		&& (csuotType < sizeof(pszOperatorType)/sizeof(pszOperatorType[0])) )
	{
		return	pszOperatorType[csuotType] ;
	}
	return	"???" ;
}

const char * ECSExecutionReverseAssembler::GetCompareTypeName( CSCompareType csctType )
{
	static const char *	pszOperatorType[] =
	{
		"ne", "eq", "lt", "le", "gt", "ge",
		"ptr.ne", "ptr.eq"
	} ;
	if ( (csctType >= 0)
		&& (csctType < sizeof(pszOperatorType)/sizeof(pszOperatorType[0])) )
	{
		return	pszOperatorType[csctType] ;
	}
	return	"???" ;
}

// クラス名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSExecutionReverseAssembler::GetClassName( const BYTE * pImage, int& ip )
{
	DWORD	dwClassIndex = *((DWORD*)(pImage + ip)) ;
	ip += sizeof(DWORD) ;
	//
	return	GetClassNameFromIndex( dwClassIndex ) ;
}

const wchar_t * ECSExecutionReverseAssembler::GetClassNameFromIndex( DWORD dwClassIndex )
{
	ESLAssert( m_pcsxi != NULL ) ;
	if ( m_pcsxi != NULL )
	{
		const ECSClassInfo *
				pClassInf = m_pcsxi->GetClassInfoAt( dwClassIndex ) ;
		if ( pClassInf != NULL )
		{
			return	pClassInf->GetGlobalName() ;
		}
	}
	return	L"???" ;
}

// 文字列リテラル取得
//////////////////////////////////////////////////////////////////////////////
EWideString ECSExecutionReverseAssembler::GetStringLiteral( const BYTE * pImage, int& ip )
{
	ESLAssert( m_pcsxi != NULL ) ;
	DWORD	dwLength = *((DWORD*)(pImage + ip)) ;
	ip += sizeof(DWORD) ;
	if ( dwLength != 0x80000000 )
	{
		EWideString	wstrBuf ;
		if ( dwLength != 0 )
		{
			void *	ptrBuf = wstrBuf.GetBuffer( dwLength ) ;
			if ( ptrBuf != NULL )
			{
				::eslMoveMemory( ptrBuf,
					pImage + ip, dwLength * sizeof(wchar_t) ) ;
				wstrBuf.ReleaseBuffer( dwLength ) ;
				ip += dwLength * sizeof(wchar_t) ;
			}
			else
			{
				wstrBuf = L"???" ;
			}
		}
		else
		{
			wstrBuf = L"" ;
		}
		return	wstrBuf ;
	}
	else
	{
		DWORD	dwStrIndex = *((DWORD*)(pImage + ip)) ;
		ip += sizeof(DWORD) ;
		//
		if ( m_pcsxi == NULL )
		{
			return	L"???" ;
		}
		ECSString *	pConstStr =
				m_pcsxi->m_lstConstStr.GetAt( dwStrIndex ) ;
		if ( pConstStr != NULL )
		{
			return	pConstStr->m_varStr ;
		}
		return	L"" ;
	}
}

// 関数名取得
//////////////////////////////////////////////////////////////////////////////
EWideString ECSExecutionReverseAssembler::GetFunctionName( DWORD dwFuncAddr )
{
	ESLAssert( m_pcsxi != NULL ) ;
	if ( m_pcsxi != NULL )
	{
		const ECSExecutionImage::EWStrFuncEntryArray &
				wstaFunc = m_pcsxi->GetFunctionEntries() ;
		const int	nCount = wstaFunc.GetSize() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ECSExecutionImage::FUNC_ENTRY *	pFunc = wstaFunc.GetObjectAt( i ) ;
			if ( (pFunc != NULL) && (pFunc->dwAddress == dwFuncAddr) )
			{
				return	*(wstaFunc.GetTagAt(i)) ;
			}
		}
	}
	EWideString	wstrFuncAddr = L"#" ;
	wstrFuncAddr += EWideString( dwFuncAddr, 8 ) ;
	return	wstrFuncAddr ;
}


// obj.new 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleNew
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.new." ;
	//
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	CSObjectMode	csomType = (CSObjectMode) pImage[ip ++] ;
	CSVariableType	csvtType = (CSVariableType) pImage[ip ++] ;
	//
	inf.strMnemonic += GetMemoryClassName( csomType ) ;
	//
	if ( csvtType == csvtClassObject )
	{
		inf.strOperand += EString( GetClassName( pImage, ip ) ) ;
		inf.strOperand += " " ;
		inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
	}
	else
	{
		if ( csvtType == csvtObject )
		{
			inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
		}
		else
		{
			inf.strOperand += GetVariableTypeName( csvtType ) ;
		}
		inf.strOperand += " " ;
		inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
	}
	//
	inf.nBytes = ip ;
}

// obj.free 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleFree
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.free" ;
	inf.nBytes = 1 ;
}

// obj.load 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleLoad
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.load" ;
	//
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	CSObjectMode	csomType = (CSObjectMode) pImage[ip ++] ;
	CSVariableType	csvtType = (CSVariableType) pImage[ip ++] ;
	if ( csomType == csomImmediate )
	{
		//
		// 即値読み込み
		//
		switch ( csvtType )
		{
		case	csvtInteger64:
			{
				EString	strValue ;
				strValue.FromInteger( *((INT64*)(pImage + ip)) ) ;
				ip += sizeof(INT64) ;
				inf.strOperand += strValue ;
			}
			break ;
		case	csvtInteger:
			inf.strOperand += EString( *((long int*)(pImage + ip)) ) ;
			ip += sizeof(long int) ;
			break ;
		case	csvtBoolean:
			inf.strOperand += EString( *((SBYTE*)(pImage + ip)) ) ;
			ip += sizeof(SBYTE) ;
			break ;
		case	csvtInt8:
			inf.strOperand += EString( *((SBYTE*)(pImage + ip)) ) ;
			ip += sizeof(SBYTE) ;
			break ;
		case	csvtUint8:
			inf.strOperand += EString( *((BYTE*)(pImage + ip)) ) ;
			ip += sizeof(BYTE) ;
			break ;
		case	csvtInt16:
			inf.strOperand += EString( *((SWORD*)(pImage + ip)) ) ;
			ip += sizeof(SWORD) ;
			break ;
		case	csvtUint16:
			inf.strOperand += EString( *((WORD*)(pImage + ip)) ) ;
			ip += sizeof(WORD) ;
			break ;
		case	csvtInt32:
			inf.strOperand += EString( *((SDWORD*)(pImage + ip)) ) ;
			ip += sizeof(SDWORD) ;
			break ;
		case	csvtUint32:
			inf.strOperand += EString( *((DWORD*)(pImage + ip)) ) ;
			ip += sizeof(DWORD) ;
			break ;
		case	csvtReal:
			inf.strOperand += EString( *((double*)(pImage + ip)) ) ;
			ip += sizeof(double) ;
			break ;
		case	csvtString:
			{
				EWideString	wstrText = GetStringLiteral( pImage, ip ) ;
				EDescription::EncodeTextCEscSequence( wstrText ) ;
				inf.strOperand += '\"' ;
				inf.strOperand += EString( wstrText ) ;
				inf.strOperand += '\"' ;
			}
			break ;
		case	csvtReference:
			inf.strOperand += "Reference" ;
			break ;
		case	csvtArray:
			inf.strOperand += "Array" ;
			break ;
		case	csvtHash:
			inf.strOperand += "Hash" ;
			break ;
		case	csvtPointer:
			inf.strOperand += "Pointer" ;
			break ;
		case	csvtClassObject:
			inf.strOperand += EString( GetClassName( pImage, ip ) ) ;
			break ;
		case	csvtObject:
			inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
			break ;
		default:
			inf.strOperand += "???" ;
			break ;
		}
	}
	else
	{
		inf.strOperand += GetMemoryClassName( csomType ) ;
		if ( csvtType == csvtInteger )
		{
			int	iElement = *((long int*)(pImage + ip)) ;
			ip += sizeof(long int) ;
			//
			inf.strOperand += '[' ;
			inf.strOperand += EString( iElement ) ;
			inf.strOperand += ']' ;
		}
		else if ( csvtType == csvtString )
		{
			inf.strOperand += '.' ;
			inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
		}
	}
	inf.nBytes = ip ;
}

// obj.store 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleStore
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	CSOperatorType	csotType = (CSOperatorType) pbytCode[1] ;
	//
	inf.nBytes = 2 ;
	inf.strMnemonic = "obj.store" ;
	//
	if ( csotType != (BYTE) csotNop )
	{
		inf.strMnemonic += '.' ;
		inf.strMnemonic += GetOperatorTypeName( csotType ) ;
	}
	inf.strMnemonic += ".pop" ;
}

// obj.enter 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleEnter
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.enter" ;
	//
	// 名前空間名を取得
	//
	DWORD			i, dwArgCount ;
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	//
	inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
	//
	dwArgCount = *((DWORD*)(pImage + ip)) ;
	ip += sizeof(DWORD) ;
	//
	if ( dwArgCount != -1 )
	{
		//
		// 通常のブロック
		//
		inf.strOperand += "( " ;
		//
		for ( i = 0; i < dwArgCount; i ++ )
		{
			if ( i > 0 )
			{
				inf.strOperand += ", " ;
			}
			CSVariableType	csvtType = (CSVariableType) pImage[ip ++] ;
			if ( csvtType == csvtClassObject )
			{
				inf.strOperand += EString( GetClassName( pImage, ip ) ) ;
			}
			else
			{
				if ( csvtType == csvtObject )
				{
					inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
				}
				else
				{
					inf.strOperand += EString( GetVariableTypeName( csvtType ) ) ;
				}
			}
			inf.strOperand += ' ' ;
			inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
		}
		inf.strOperand += " )" ;
	}
	else
	{
		if ( pImage[ip ++] == 0 )
		{
			//
			// TRY ブロック
			//
			DWORD	dwCatchAddr = *((DWORD*)(pImage + ip)) ;
			ip += sizeof(DWORD) ;
			//
			inf.strOperand += "try #" ;
			inf.strOperand += EString( dwCatchAddr, 8 ) ;
		}
	}
	inf.nBytes = ip ;
}

// obj.leave 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleLeave
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.nBytes = 1 ;
	inf.strMnemonic = "obj.leave" ;
}

// obj.jump 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleJump
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	DWORD	dwJumpAddr = (addrCode + 5) + *((SDWORD*)(pbytCode + 1)) ;
	//
	inf.nBytes = 5 ;
	inf.strMnemonic = "obj.jump" ;
	inf.strOperand += "#" ;
	inf.strOperand += EString( dwJumpAddr, 8 ) ;
}

// obj.cjump 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCJump
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	BYTE	bytConditional = pbytCode[1] ;
	DWORD	dwJumpAddr = (addrCode + 6) + *((SDWORD*)(pbytCode + 2)) ;
	//
	inf.nBytes = 6 ;
	inf.strMnemonic = "obj.cjump" ;
	if ( bytConditional & 0x01 )
	{
		inf.strMnemonic += ".nz" ;
	}
	else
	{
		inf.strMnemonic += ".z" ;
	}
	if ( !(bytConditional & 0x02) )
	{
		inf.strMnemonic += ".pop" ;
	}
	inf.strOperand += "#" ;
	inf.strOperand += EString( dwJumpAddr, 8 ) ;
}

// obj.call 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCall
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.call" ;
	//
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	CSObjectMode	csomType = (CSObjectMode) pImage[ip ++] ;
	DWORD			dwArgCount = *((DWORD*)(pImage + ip)) ;
	ip += sizeof(DWORD) ;
	//
	inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
	inf.strOperand += ',' ;
	inf.strOperand += EString( (int) dwArgCount ) ;
	//
	inf.nBytes = ip ;
}

// obj.return 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleReturn
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	BYTE	bytFreeStack = pbytCode[1] ;
	//
	inf.nBytes = 2 ;
	//
	if ( bytFreeStack == 2 )
	{
		inf.strMnemonic = "obj.return.leave" ;
	}
	else if ( bytFreeStack == 3 )
	{
		inf.strMnemonic = "obj.throw" ;
	}
	else
	{
		inf.strMnemonic = "obj.return" ;
		inf.strOperand += EString( (int) bytFreeStack ) ;
	}
}

// obj.element 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleElement
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.element" ;
	//
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	CSVariableType	csvtType = (CSVariableType) pImage[ip ++] ;
	if ( csvtType == csvtInteger )
	{
		long int	nIndex = *((long int*)(pImage + ip)) ;
		ip += sizeof(long int) ;
		//
		inf.strOperand += EString( (int) nIndex ) ;
	}
	else if ( csvtType == csvtString )
	{
		EWideString	wstrText = GetStringLiteral( pImage, ip ) ;
		EDescription::EncodeTextCEscSequence( wstrText ) ;
		//
		inf.strOperand += '\"' ;
		inf.strOperand += EString( wstrText ) ;
		inf.strOperand += '\"' ;
	}
	inf.nBytes = ip ;
}

// obj.element.pop 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleElementIndirect
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.element.pop" ;
	inf.nBytes = 1 ;
}

// obj.operate 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleOperate
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	CSOperatorType	csotType = (CSOperatorType) pbytCode[1] ;
	//
	inf.nBytes = 2 ;
	//
	inf.strMnemonic = "obj.operate." ;
	inf.strMnemonic += GetOperatorTypeName( csotType ) ;
	inf.strMnemonic += ".pop" ;
}

// obj.operate 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleUniOperate
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	CSUnaryOperatorType	csuotType = (CSUnaryOperatorType) pbytCode[1] ;
	//
	inf.nBytes = 2 ;
	//
	inf.strMnemonic = "obj.operate." ;
	inf.strMnemonic += GetUniOperatorTypeName( csuotType ) ;
}

// obj.compare 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCompare
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	CSCompareType	csctType = (CSCompareType) pbytCode[1] ;
	//
	inf.nBytes = 2 ;
	//
	inf.strMnemonic = "obj.compare." ;
	inf.strMnemonic += GetCompareTypeName( csctType ) ;
	inf.strMnemonic += ".pop" ;
}

// obj.ex.operator 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleExOperate
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	const BYTE *		pImage = pbytCode ;
	int					ip = 1 ;
	CSExtraOperatorType	csxotType = (CSExtraOperatorType) pImage[ip ++] ;
	//
	switch ( csxotType )
	{
	case	csxotMoveReference:
		inf.strMnemonic = "obj.move.ref.pop" ;
		break ;
	case	csxotArrayDim:
		inf.strMnemonic = "obj.array.dim" ;
		{
			DWORD	dwDimension = *((DWORD*)(pImage + ip)) ;
			ip += sizeof(DWORD) ;
			//
			for ( DWORD i = 0; i < dwDimension; i ++ )
			{
				if ( i > 0 )
				{
					inf.strOperand += ',' ;
				}
				DWORD	dwDim = *((DWORD*)(pImage + ip)) ;
				ip += sizeof(DWORD) ;
				//
				if ( dwDim != 0x80000000 )
				{
					inf.strOperand += EString( (int) dwDim ) ;
				}
			}
		}
		break ;
	case	csxotHashContainer:
		inf.strMnemonic = "obj.hash.container.pop" ;
		break ;
	default:
		inf.strMnemonic = "obj.ex.operator.???" ;
		break ;
	}
	inf.nBytes = ip ;
}

// obj.ex.operator 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleExUniOperate
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	const BYTE *			pImage = pbytCode ;
	int						ip = 1 ;
	CSExtraUniOperatorType	csxuotType = (CSExtraUniOperatorType) pImage[ip ++] ;
	//
	switch ( csxuotType )
	{
	case	csxuotDeselect:
		inf.strMnemonic = "obj.deselect" ;
		break ;
	case	csxuotDelete:
		inf.strMnemonic = "obj.delete" ;
		break ;
	case	csxuotBoolean:
		inf.strMnemonic = "obj.boolean" ;
		break ;
	case	csxuotSizeOf:
		inf.strMnemonic = "obj.sizeof" ;
		break ;
	case	csxuotTypeOf:
		inf.strMnemonic = "obj.typeof" ;
		break ;
	case	csxuotStaticCast:
		inf.strMnemonic = "obj.static_cast" ;
		{
			const SDWORD *	pdwImage = (const SDWORD *) (pImage + ip) ;
			ip += sizeof(DWORD) * 3 ;
			//
			inf.strOperand += EString( (int) pdwImage[0] ) ;
			inf.strOperand += ',' ;
			inf.strOperand += EString( (int) pdwImage[1] ) ;
			inf.strOperand += ',' ;
			inf.strOperand += EString( (int) pdwImage[2] ) ;
		}
		break ;
	case	csxuotDynamicCast:
		inf.strMnemonic = "obj.dynamic_cast" ;
		inf.strOperand += EString( GetStringLiteral( pImage, ip ) ) ;
		break ;
	case	csxuotDuplicate:
		inf.strMnemonic = "obj.duplicate" ;
		break ;
	default:
		inf.strMnemonic = "obj.ex.operate.???" ;
		break ;
	}
	inf.nBytes = ip ;
}

// obj.ex.call 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleExCall
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.strMnemonic = "obj.ex.call" ;
	//
	// 関数引数設定
	//
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	//
	DWORD	dwArgCount = *((DWORD*)(pImage + ip)) ;
	ip += sizeof(DWORD) ;
	//
	CSObjectMode	csomType = (CSObjectMode) pImage[ip ++] ;
	CSVariableType	csvtType = (CSVariableType) pImage[ip ++] ;
	//
	// 呼び出し関数取得
	//
	if ( csomType == csomImmediate )
	{
		if ( csvtType == csvtString )
		{
			EWideString	wstrText = GetStringLiteral( pImage, ip ) ;
			EDescription::EncodeTextCEscSequence( wstrText ) ;
			//
			inf.strOperand += '\"' ;
			inf.strOperand += EString( wstrText ) ;
			inf.strOperand += '\"' ;
		}
		else if ( csvtType == csvtInteger )
		{
			DWORD	dwFuncAddr = *((DWORD*)(pImage + ip)) ;
			ip += sizeof(DWORD) ;
			//
			inf.strOperand += EString( GetFunctionName( dwFuncAddr ) ) ;
		}
	}
	inf.strOperand += ',' ;
	inf.strOperand += EString( (int) dwArgCount ) ;
	//
	inf.nBytes = ip ;
}

// obj.ex.return 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleExReturn
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	BYTE	bytFreeStack = pbytCode[1] ;
	//
	inf.nBytes = 2 ;
	inf.strMnemonic = "obj.ex.return" ;
	inf.strOperand += EString( (int) bytFreeStack ) ;
}

// obj.call.member 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCallMember
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	DWORD	dwArgCount = *((DWORD*)(pImage + ip)) ;
	DWORD	dwClassIndex = *((DWORD*)(pImage + ip + sizeof(DWORD))) ;
	DWORD	dwFuncIndex = *((DWORD*)(pImage + ip + sizeof(DWORD) * 2)) ;
	ip += sizeof(DWORD) * 3 ;
	//
	inf.strMnemonic = "obj.call.member" ;
	//
	inf.strOperand += EString( GetClassNameFromIndex( dwClassIndex ) ) ;
	inf.strOperand += '.' ;
	const ECSClassInfo *	pClassInf =
		(m_pcsxi != NULL) ?
			m_pcsxi->GetClassInfoAt( dwClassIndex ) : NULL ;
	if ( pClassInf != NULL )
	{
		ECSClassInfo::MemberFunction *
			pFunc = pClassInf->GetFunctionAt( dwFuncIndex ) ;
		if ( pFunc != NULL )
		{
			inf.strOperand += EString( pFunc->GetName() ) ;
		}
		else
		{
			inf.strOperand += "???" ;
		}
	}
	else
	{
		inf.strOperand += "???" ;
	}
	inf.strOperand += "," ;
	inf.strOperand += EString( (int) dwArgCount ) ;
	//
	inf.nBytes = ip ;
}

// obj.call.native.member 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCallNativeMember
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	DWORD	dwArgCount = *((DWORD*)(pImage + ip)) ;
	DWORD	dwClassIndex = *((DWORD*)(pImage + ip + sizeof(DWORD))) ;
	DWORD	dwFuncIndex = *((DWORD*)(pImage + ip + sizeof(DWORD) * 2)) ;
	ip += sizeof(DWORD) * 3 ;
	//
	inf.strMnemonic = "obj.call.native.member" ;
	//
	inf.strOperand += EString( GetClassNameFromIndex( dwClassIndex ) ) ;
	inf.strOperand += '.' ;
	const ECSClassInfo *	pClassInf =
		(m_pcsxi != NULL) ?
			m_pcsxi->GetClassInfoAt( dwClassIndex ) : NULL ;
	if ( pClassInf != NULL )
	{
		ECSClassInfo::MemberFunction *
			pFunc = pClassInf->GetFunctionAt( dwFuncIndex ) ;
		if ( pFunc != NULL )
		{
			inf.strOperand += EString( pFunc->GetName() ) ;
		}
		else
		{
			inf.strOperand += "???" ;
		}
	}
	else
	{
		inf.strOperand += "???" ;
	}
	inf.strOperand += "," ;
	inf.strOperand += EString( (int) dwArgCount ) ;
	//
	inf.nBytes = ip ;
}

// obj.swap 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleSwap
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	BYTE			bytSubCode = pImage[ip] ;
	ip ++ ;
	//
	DWORD	dwIndex1 = *((DWORD*)(pImage + ip)) ;
	DWORD	dwIndex2 = *((DWORD*)(pImage + ip + sizeof(DWORD))) ;
	ip += sizeof(DWORD) * 2 ;
	//
	inf.strMnemonic = "obj.swap" ;
	inf.strOperand += EString( (int) dwIndex1 ) ;
	inf.strOperand += "," ;
	inf.strOperand += EString( (int) dwIndex2 ) ;
	//
	inf.nBytes = ip ;
}

// obj.buffer 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCreateBuffer
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	DWORD	dwSize = *((DWORD*)(pbytCode + 1)) ;
	//
	inf.nBytes = 5 ;
	inf.strMnemonic = "obj.buffer" ;
	inf.strOperand = EString( (int) dwSize ) ;
}

// obj.buffer 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCreateBufferVSize
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.nBytes = 1 ;
	inf.strMnemonic = "obj.buffer.pop" ;
}

// obj.pointer.offset 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssemblePointerToObject
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	DWORD	dwOffset = *((DWORD*)(pbytCode + 1)) ;
	//
	inf.nBytes = 5 ;
	inf.strMnemonic = "obj.pointer.offset" ;
	inf.strOperand = EString( (int) dwOffset ) ;
}

// obj.pointer.address 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssemblePointerToAddress
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.nBytes = 1 ;
	inf.strMnemonic = "obj.pointer.address.pop" ;
}

// obj.reference 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleReferenceForPointer
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	CSVariableType	csvtRefType = (CSVariableType) *((BYTE*)(pbytCode + 1)) ;
	//
	inf.nBytes = 2 ;
	inf.strMnemonic = "obj.reference." ;
	inf.strMnemonic += GetVariableTypeName( csvtRefType ) ;
}

// obj.reference 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleReferenceForObjPointer
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	inf.nBytes = 1 ;
	inf.strMnemonic = "obj.reference.object" ;
}

// obj.call.pointer 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCallFunctionPointer
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	DWORD	dwArgCount = *((DWORD*)(pbytCode + 1)) ;
	//
	inf.nBytes = 5 ;
	inf.strMnemonic = "obj.call.pointer.pop" ;
	inf.strOperand = EString( (int) dwArgCount ) ;
}

// obj.call.native 命令
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionReverseAssembler::ReverseAssembleCallNativeFunction
	( ECSExecutionReverseAssembler::InstructionInfo & inf,
			const BYTE  * pbytCode, int addrCode, bool fMnemonic )
{
	const BYTE *	pImage = pbytCode ;
	int				ip = 1 ;
	DWORD	dwArgCount = *((DWORD*)(pImage + ip)) ;
	DWORD	dwFuncIndex = *((DWORD*)(pImage + ip + sizeof(DWORD))) ;
	ip += sizeof(DWORD) * 2 ;
	//
	inf.strMnemonic = "obj.call.native" ;
	//
	const wchar_t *	pwszFuncName =
			m_pcsxi->GetNativeFuncNameList().GetAt( dwFuncIndex ) ;
	if ( pwszFuncName != NULL )
	{
		inf.strOperand += EString( pwszFuncName ) ;
	}
	else
	{
		inf.strOperand += "???" ;
	}
	inf.strOperand += "," ;
	inf.strOperand += EString( (int) dwArgCount ) ;
	//
	inf.nBytes = ip ;
}
