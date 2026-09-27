
#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_queue_buffer.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_module_maker.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// 詞葉モジュール・メーカー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( ECSSakura2::ExecutableModuleMaker, ExecutableModule )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ExecutableModuleMaker::ExecutableModuleMaker( void )
{
	m_pfeCurrent = NULL ;
}

// 書き出し準備
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::InitializeMake( void )
{
	//
	// ヘッダ初期化
	//
	eslFillMemory( &m_exmHeader, 0, sizeof(HEADER) ) ;
	m_exmHeader.nVersion = 1 ;
	m_exmHeader.nIntBase = 64 ;
	m_exmHeader.nContainerFlags =
			flagContainerExtRefClass | flagContainerImpRefFunc ;
	m_exmHeader.nStackSize = 0x1000 ;
	m_exmHeader.nHeapSize = 0x1000 ;
	m_exmHeader.fnEntryPoint = -1 ;
	m_exmHeader.fnStaticInitialize = -1 ;
	m_exmHeader.fnResumePrepare = -1 ;
}

// 書き出し完了
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::FinishMake( void )
{
	size_t	nCodeBytes = (size_t) m_sbufCode.GetLength() ;
	m_bufCode.CreateBuffer( (DWORD) nCodeBytes ) ;
	m_sbufCode.Seek( 0 ) ;
	m_sbufCode.Read( m_bufCode.GetBuffer(), nCodeBytes ) ;
	m_bufCode.CreateShadowBuffer() ;
	m_sbufCode.Seek( 0 ) ;
	m_sbufCode.Read( m_bufCode.GetCodeShadowBuffer(), nCodeBytes ) ;
	//
	size_t	nGlobalBytes = (size_t) m_sbufGlobal.GetLength() ;
	m_bufGlobal.CreateBuffer( (DWORD) nGlobalBytes ) ;
	m_sbufGlobal.Seek( 0 ) ;
	m_sbufGlobal.Read( m_bufGlobal.GetBuffer(), nGlobalBytes ) ;
	//
	size_t	nConstBytes = (size_t) m_sbufConst.GetLength() ;
	m_bufConst.CreateBuffer( (DWORD) nConstBytes ) ;
	m_sbufConst.Seek( 0 ) ;
	m_sbufConst.Read( m_bufConst.GetBuffer(), nConstBytes ) ;
	//
	size_t	nSharedBytes = (size_t) m_sbufShared.GetLength() ;
	m_bufShared.CreateBuffer( (DWORD) nSharedBytes ) ;
	m_sbufShared.Seek( 0 ) ;
	m_sbufShared.Read( m_bufShared.GetBuffer(), nSharedBytes ) ;
}

// 関数開始
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::BeginFunction( const wchar_t * pwszFuncName )
{
	FUNC_ENTRY	fnEntry ;
	fnEntry.dwFlags = ExecutableModule::flagNakedCall ;
	fnEntry.dwAddress = (DWORD) GetNextCodeAddress() ;
	m_symbolCode.SetAs( pwszFuncName, fnEntry ) ;
	m_pfeCurrent = m_symbolCode.GetAs( pwszFuncName ) ;
	m_strCurFunc = pwszFuncName ;
}

// 関数終了
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::EndFunction( void )
{
	if ( m_pfeCurrent != NULL )
	{
		m_pfeCurrent->dwBytes = (DWORD) GetNextCodeAddress()
										- m_pfeCurrent->dwAddress ;
	}
	m_strCurFunc.RemoveAll() ;
}

// 関数情報削除
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::DeleteFunction( void )
{
	m_symbolCode.RemoveAs( m_strCurFunc ) ;
	m_pfeCurrent = NULL ;
	m_strCurFunc.RemoveAll() ;
}

// デバッグ情報追加
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::AddDebugCodeInfo
	( size_t addrCode, const Rosetta::RSParenthesis * pParenthesis, size_t indexSrc )
{
	DebugCodeInfo	dci ;
	dci.addrCode = addrCode ;
	dci.pParenthesis = pParenthesis ;
	dci.indexSrc = indexSrc ;
	//
	const DebugCodeInfo *	pdci = m_arrDebugCodeInfo.GetConstArray() ;
	size_t					nCount = m_arrDebugCodeInfo.GetLength() ;
	if ( nCount > 0 )
	{
		if ( pdci[nCount - 1].addrCode <= addrCode )
		{
			m_arrDebugCodeInfo.Add( dci ) ;
		}
		else
		{
			size_t	iFirst = 0 ;
			size_t	iEnd = nCount ;
			size_t	iMiddle ;
			while ( iFirst < iEnd )
			{
				iMiddle = (iFirst + iEnd) >> 1 ;
				if ( pdci[iMiddle].addrCode <= addrCode )
				{
					iFirst = iMiddle + 1 ;
				}
				else
				{
					iEnd = iMiddle ;
				}
			}
			ESLAssert( (iFirst == nCount) || (pdci[iFirst].addrCode <= addrCode) ) ;
			m_arrDebugCodeInfo.InsertAt( iFirst, dci ) ;
		}
	}
	else
	{
		m_arrDebugCodeInfo.Add( dci ) ;
	}
}

// コードアドレスからデバッグ情報検索
//////////////////////////////////////////////////////////////////////////////
const ExecutableModuleMaker::DebugCodeInfo *
		ExecutableModuleMaker::SearchDebugCodeInfo( size_t addrCode ) const
{
	const DebugCodeInfo *	pdci = m_arrDebugCodeInfo.GetConstArray() ;
	size_t	nCount = m_arrDebugCodeInfo.GetLength() ;
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = (ssize_t) nCount - 1 ;
	ssize_t	iMiddle ;
	while ( iFirst < iEnd )
	{
		iMiddle = (iFirst + iEnd + 1) >> 1 ;
		if ( pdci[iMiddle].addrCode <= addrCode )
		{
			iFirst = iMiddle ;
		}
		else
		{
			iEnd = iMiddle - 1 ;
		}
	}
	if ( (iFirst >= 0) && (iFirst < (ssize_t) nCount) )
	{
		return	pdci + iFirst ;
	}
	return	NULL ;
}

// システムコールID生成／取得
//////////////////////////////////////////////////////////////////////////////
uint32_t ExecutableModuleMaker::GenSystemCallID( const wchar_t * pwszSysCall )
{
	ssize_t	i = m_indexSysCall.FindIndex( pwszSysCall ) ;
	if ( i >= 0 )
	{
		return	(uint32_t) i ;
	}
	return	(uint32_t) m_indexSysCall.Add( new SString(pwszSysCall) ) ;
}

// システムコール参照追加
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::AddSystemCallRef( size_t nRefAddr )
{
	m_reallcRefSysCallId.Add( (DWORD) nRefAddr ) ;
}

// LOAD reg, mem
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeLoad
	( int regDst,
		ECSSakura2Processor::DataType dataType,
		int regBase, int nOffset,
		int regIndex, int scaleIndex )
{
	ESLAssert( (regDst >= 0) && (regDst < 0x100) ) ;
	uint8_t	buf[8] ;
	if ( regBase >= 16 )
	{
		ESLAssert( regIndex < 0 ) ;
		regIndex = regBase ;
		scaleIndex = 0 ;
		regBase = regZeroPtr ;
	}
	ESLAssert( (regBase >= 0) && (regBase < 16) ) ;
	if ( regBase == regBP )
	{
		if ( regIndex >= 0 )
		{
			ESLAssert( regIndex < 256 ) ;
			ESLAssert( (scaleIndex < 8) && (scaleIndex >= 0) ) ;
			buf[0] = codeLoadLocalIndexImm32 ;
			buf[1] = ((scaleIndex & 0x07) << 5) | dataType ;
			buf[2] = (uint8_t) regIndex ;
			buf[3] = (uint8_t) regDst ;
			buf[4] = (uint8_t) nOffset ;
			buf[5] = (uint8_t) (nOffset >> 8) ;
			buf[6] = (uint8_t) (nOffset >> 16) ;
			buf[7] = (uint8_t) (nOffset >> 24) ;
			m_sbufCode.Write( &buf[0], 8 ) ;
		}
		else
		{
			buf[0] = codeLoadLocalImm32 ;
			buf[1] = dataType ;
			buf[2] = (uint8_t) regDst ;
			buf[3] = (uint8_t) nOffset ;
			buf[4] = (uint8_t) (nOffset >> 8) ;
			buf[5] = (uint8_t) (nOffset >> 16) ;
			buf[6] = (uint8_t) (nOffset >> 24) ;
			m_sbufCode.Write( &buf[0], 7 ) ;
		}
	}
	else
	{
		if ( regIndex >= 0 )
		{
			ESLAssert( regIndex < 128 ) ;
			ESLAssert( (scaleIndex < 4) && (scaleIndex >= 0) ) ;
			if ( nOffset != 0 )
			{
				buf[0] = codeLoadMemBaseIndexImm32 ;
				buf[1] = ((scaleIndex & 0x02) << 6) | (regBase << 3) | dataType ;
				buf[2] = ((scaleIndex & 0x01) << 7) | regIndex ;
				buf[3] = (uint8_t) regDst ;
				buf[4] = (uint8_t) nOffset ;
				buf[5] = (uint8_t) (nOffset >> 8) ;
				buf[6] = (uint8_t) (nOffset >> 16) ;
				buf[7] = (uint8_t) (nOffset >> 24) ;
				m_sbufCode.Write( &buf[0], 8 ) ;
			}
			else
			{
				buf[0] = codeLoadMemBaseIndex ;
				buf[1] = (regBase << 3) | dataType ;
				buf[2] = ((scaleIndex & 0x01) << 7) | regIndex ;
				buf[3] = (uint8_t) regDst ;
				m_sbufCode.Write( &buf[0], 4 ) ;
			}
		}
		else
		{
			if ( nOffset != 0 )
			{
				buf[0] = codeLoadMemBaseImm32 ;
				buf[1] = (regBase << 3) | dataType ;
				buf[2] = (uint8_t) regDst ;
				buf[3] = (uint8_t) nOffset ;
				buf[4] = (uint8_t) (nOffset >> 8) ;
				buf[5] = (uint8_t) (nOffset >> 16) ;
				buf[6] = (uint8_t) (nOffset >> 24) ;
				m_sbufCode.Write( &buf[0], 7 ) ;
			}
			else
			{
				buf[0] = codeLoadMemBase ;
				buf[1] = (regBase << 3) | dataType ;
				buf[2] = (uint8_t) regDst ;
				m_sbufCode.Write( &buf[0], 3 ) ;
			}
		}
	}
}

// STORE mem, reg
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeStore
	( int regSrc,
		ECSSakura2Processor::DataType dataType,
		int regBase, int nOffset, int regIndex, int scaleIndex )
{
	ESLAssert( (regSrc >= 0) && (regSrc < 0x100) ) ;
	uint8_t	buf[8] ;
	if ( regBase >= 16 )
	{
		ESLAssert( regIndex < 0 ) ;
		regIndex = regBase ;
		scaleIndex = 0 ;
		regBase = regZeroPtr ;
	}
	ESLAssert( (regBase >= 0) && (regBase < 16) ) ;
	if ( regBase == regBP )
	{
		if ( regIndex >= 0 )
		{
			ESLAssert( regIndex < 256 ) ;
			ESLAssert( (scaleIndex < 8) && (scaleIndex >= 0) ) ;
			buf[0] = codeStoreLocalIndexImm32 ;
			buf[1] = ((scaleIndex & 0x07) << 5) | dataType ;
			buf[2] = (uint8_t) regIndex ;
			buf[3] = (uint8_t) regSrc ;
			buf[4] = (uint8_t) nOffset ;
			buf[5] = (uint8_t) (nOffset >> 8) ;
			buf[6] = (uint8_t) (nOffset >> 16) ;
			buf[7] = (uint8_t) (nOffset >> 24) ;
			m_sbufCode.Write( &buf[0], 8 ) ;
		}
		else
		{
			buf[0] = codeStoreLocalImm32 ;
			buf[1] = dataType ;
			buf[2] = (uint8_t) regSrc ;
			buf[3] = (uint8_t) nOffset ;
			buf[4] = (uint8_t) (nOffset >> 8) ;
			buf[5] = (uint8_t) (nOffset >> 16) ;
			buf[6] = (uint8_t) (nOffset >> 24) ;
			m_sbufCode.Write( &buf[0], 7 ) ;
		}
	}
	else
	{
		if ( regIndex >= 0 )
		{
			ESLAssert( regIndex < 128 ) ;
			ESLAssert( (scaleIndex < 4) && (scaleIndex >= 0) ) ;
			if ( nOffset != 0 )
			{
				buf[0] = codeStoreMemBaseIndexImm32 ;
				buf[1] = ((scaleIndex & 0x02) << 6) | (regBase << 3) | dataType ;
				buf[2] = ((scaleIndex & 0x01) << 7) | regIndex ;
				buf[3] = (uint8_t) regSrc ;
				buf[4] = (uint8_t) nOffset ;
				buf[5] = (uint8_t) (nOffset >> 8) ;
				buf[6] = (uint8_t) (nOffset >> 16) ;
				buf[7] = (uint8_t) (nOffset >> 24) ;
				m_sbufCode.Write( &buf[0], 8 ) ;
			}
			else
			{
				buf[0] = codeStoreMemBaseIndex ;
				buf[1] = (regBase << 3) | dataType ;
				buf[2] = ((scaleIndex & 0x01) << 7) | regIndex ;
				buf[3] = (uint8_t) regSrc ;
				m_sbufCode.Write( &buf[0], 4 ) ;
			}
		}
		else
		{
			if ( nOffset != 0 )
			{
				buf[0] = codeStoreMemBaseImm32 ;
				buf[1] = (regBase << 3) | dataType ;
				buf[2] = (uint8_t) regSrc ;
				buf[3] = (uint8_t) nOffset ;
				buf[4] = (uint8_t) (nOffset >> 8) ;
				buf[5] = (uint8_t) (nOffset >> 16) ;
				buf[6] = (uint8_t) (nOffset >> 24) ;
				m_sbufCode.Write( &buf[0], 7 ) ;
			}
			else
			{
				buf[0] = codeStoreMemBase ;
				buf[1] = (regBase << 3) | dataType ;
				buf[2] = (uint8_t) regSrc ;
				m_sbufCode.Write( &buf[0], 3 ) ;
			}
		}
	}
}

// MOVE reg, imm64
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeMoveRegImm64( int regDst, int64_t num )
{
	uint8_t	buf[10] ;
	buf[0] = codeLoadImm64 ;
	buf[1] = (uint8_t) regDst ;
	buf[2] = (uint8_t) num ;
	buf[3] = (uint8_t) (num >> 8) ;
	buf[4] = (uint8_t) (num >> 16) ;
	buf[5] = (uint8_t) (num >> 24) ;
	buf[6] = (uint8_t) (num >> 32) ;
	buf[7] = (uint8_t) (num >> 40) ;
	buf[8] = (uint8_t) (num >> 48) ;
	buf[9] = (uint8_t) (num >> 56) ;
	m_sbufCode.Write( &buf[0], 10 ) ;
}

void ExecutableModuleMaker::WriteCodeMoveRegInt64( int regDst, int64_t num )
{
	if ( num == 0 )
	{
		WriteCode2OP( codeMoveReg, regDst, regIntZero ) ;
	}
	else if ( num == 1 )
	{
		WriteCode2OP( codeMoveReg, regDst, regIntOne ) ;
	}
	else if ( num == -1 )
	{
		WriteCode2OP( codeMoveReg, regDst, regFillBit ) ;
	}
	else if ( (-0x7FFFFFFF <= num) && (num <= 0x7FFFFFFF) )
	{
		WriteCodeAddRegRegImm32( regDst, regIntZero, (int) num ) ;
	}
	else
	{
		WriteCodeMoveRegImm64( regDst, num ) ;
	}
}

void ExecutableModuleMaker::WriteCodeMoveRegFloat64( int regDst, float64_t num )
{
	if ( num == 0.0 )
	{
		WriteCode2OP( codeMoveReg, regDst, regIntZero ) ;
	}
	else if ( num == 1.0 )
	{
		WriteCode2OP( codeMoveReg, regDst, regFloatOne ) ;
	}
	else if ( fabs( num - PI ) < 1.0e-14 )
	{
		WriteCode2OP( codeMoveReg, regDst, regFloatPI ) ;
	}
	else
	{
		WriteCodeMoveRegImm64( regDst, *((int64_t*)&num) ) ;
	}
}

void ExecutableModuleMaker::WriteCodeMoveRegConstString
	( int regDst, const wchar_t * pwszString )
{
	if ( pwszString == NULL )
	{
		WriteCodeMoveRegInt64( regDst, 0 ) ;
		return ;
	}
	size_t	addrConst = (size_t) m_sbufConst.GetLength() ;
	int64_t	nPad = 0 ;
	if ( addrConst & 0x01 )
	{
		m_sbufConst.Write( &nPad, 1 ) ;
		addrConst = (size_t) m_sbufConst.GetLength() ;
	}
	SString	strString = pwszString ;
	size_t	nLength = strString.GetLength() + 1 ;
	m_sbufConst.Write
		( strString.GetArray(), (nLength + 1) * sizeof(uint16_t) ) ;
	strString.FinishArray() ;
	//
	int64_t	addrStr = addrConst ;
	addrStr |= ((int64_t) VirtualMachine::roasNakedConst) << 56 ;
	//
	WriteCodeMoveRegImm64( regDst, addrStr ) ;
	//
	m_reallcRefConst.Add( (DWORD) GetNextCodeAddress() - 8 ) ;
}

// MOVE reg, reg
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeMoveRegReg( int regDst, int regSrc )
{
	ESLAssert( (regDst >= 0) && (regDst < 0x100) ) ;
	ESLAssert( (regSrc >= 0) && (regSrc < 0x100) ) ;
	if ( regDst != regSrc )
	{
		WriteCode2OP( codeMoveReg, regDst, regSrc ) ;
	}
}

// ADD reg, reg, imm32
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeAddRegRegImm32( int regDst, int regSrc, int imm32 )
{
	ESLAssert( (regDst >= 0) && (regDst < 0x100) ) ;
	ESLAssert( (regSrc >= 0) && (regSrc < 0x100) ) ;
	uint8_t	buf[8] ;
	buf[0] = codeAddImm32 ;
	buf[1] = (uint8_t) regDst ;
	buf[2] = (uint8_t) regSrc ;
	buf[3] = (uint8_t) imm32 ;
	buf[4] = (uint8_t) (imm32 >> 8) ;
	buf[5] = (uint8_t) (imm32 >> 16) ;
	buf[6] = (uint8_t) (imm32 >> 24) ;
	m_sbufCode.Write( &buf[0], 7 ) ;
}

// MUL reg, reg, imm32
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeMulRegRegImm32( int regDst, int regSrc, int imm32 )
{
	ESLAssert( (regDst >= 0) && (regDst < 0x100) ) ;
	ESLAssert( (regSrc >= 0) && (regSrc < 0x100) ) ;
	uint8_t	buf[8] ;
	buf[0] = codeMulImm32 ;
	buf[1] = (uint8_t) regDst ;
	buf[2] = (uint8_t) regSrc ;
	buf[3] = (uint8_t) imm32 ;
	buf[4] = (uint8_t) (imm32 >> 8) ;
	buf[5] = (uint8_t) (imm32 >> 16) ;
	buf[6] = (uint8_t) (imm32 >> 24) ;
	m_sbufCode.Write( &buf[0], 7 ) ;
}

// PUSH reg
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodePushReg( int reg )
{
	ESLAssert( (reg >= 0) && (reg < 0x100) ) ;
	WriteCode1OP( codePushReg, reg ) ;
}

// PUSH reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodePushRegsImm8( int reg, int imm8 )
{
	ESLAssert( (reg >= 0) && (reg < 0x100) ) ;
	WriteCode2OP( codePushRegs, reg, imm8 ) ;
}

// POP reg
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodePopReg( int reg )
{
	ESLAssert( (reg >= 0) && (reg < 0x100) ) ;
	WriteCode1OP( codePopReg, reg ) ;
}

// POP reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodePopRegsImm8( int reg, int imm8 )
{
	ESLAssert( (reg >= 0) && (reg < 0x100) ) ;
	WriteCode2OP( codePopRegs, reg, imm8 ) ;
}

// ADD sp, imm32
//////////////////////////////////////////////////////////////////////////////
size_t ExecutableModuleMaker::WriteCodeAddSP( int imm )
{
	uint8_t	buf[8] ;
	buf[0] = codeAddSPImm32 ;
	buf[1] = (uint8_t) imm ;
	buf[2] = (uint8_t) (imm >> 8) ;
	buf[3] = (uint8_t) (imm >> 16) ;
	buf[4] = (uint8_t) (imm >> 24) ;
	m_sbufCode.Write( &buf[0], 5 ) ;
	return	GetNextCodeAddress() - 4 ;
}

void ExecutableModuleMaker::CommitCodeAddSP( size_t addrAddSP, int imm )
{
	uint8_t	buf[8] ;
	buf[0] = (uint8_t) imm ;
	buf[1] = (uint8_t) (imm >> 8) ;
	buf[2] = (uint8_t) (imm >> 16) ;
	buf[3] = (uint8_t) (imm >> 24) ;
	//
	int64_t		nPos = m_sbufCode.GetLength() ;
	m_sbufCode.Seek( addrAddSP ) ;
	m_sbufCode.Write( &buf[0], 4 ) ;
	m_sbufCode.Seek( nPos ) ;
}

// JUMP imm32
//////////////////////////////////////////////////////////////////////////////
size_t ExecutableModuleMaker::WriteCodeJump( size_t nTargetAddr )
{
	uint8_t	buf[8] ;
	int32_t	nOffset = (int32_t) (nTargetAddr - (GetNextCodeAddress() + 5)) ;
	buf[0] = codeJumpOffset32 ;
	buf[1] = (uint8_t) nOffset ;
	buf[2] = (uint8_t) (nOffset >> 8) ;
	buf[3] = (uint8_t) (nOffset >> 16) ;
	buf[4] = (uint8_t) (nOffset >> 24) ;
	m_sbufCode.Write( &buf[0], 5 ) ;
	return	GetNextCodeAddress() ;
}

// JUMPcc imm32
//////////////////////////////////////////////////////////////////////////////
size_t ExecutableModuleMaker::WriteCodeCJump( int reg, size_t nTargetAddr )
{
	uint8_t	buf[8] ;
	int32_t	nOffset = (int32_t) (nTargetAddr - (GetNextCodeAddress() + 6)) ;
	buf[0] = codeCJumpOffset32 ;
	buf[1] = (uint8_t) reg ;
	buf[2] = (uint8_t) nOffset ;
	buf[3] = (uint8_t) (nOffset >> 8) ;
	buf[4] = (uint8_t) (nOffset >> 16) ;
	buf[5] = (uint8_t) (nOffset >> 24) ;
	m_sbufCode.Write( &buf[0], 6 ) ;
	return	GetNextCodeAddress() ;
}

size_t ExecutableModuleMaker::WriteCodeNCJump( int reg, size_t nTargetAddr )
{
	uint8_t	buf[8] ;
	int32_t	nOffset = (int32_t) (nTargetAddr - (GetNextCodeAddress() + 6)) ;
	buf[0] = codeCNJumpOffset32 ;
	buf[1] = (uint8_t) reg ;
	buf[2] = (uint8_t) nOffset ;
	buf[3] = (uint8_t) (nOffset >> 8) ;
	buf[4] = (uint8_t) (nOffset >> 16) ;
	buf[5] = (uint8_t) (nOffset >> 24) ;
	m_sbufCode.Write( &buf[0], 6 ) ;
	return	GetNextCodeAddress() ;
}

// 相対ジャンプアドレス更新
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::CommitJumpAddress( size_t nJumpOrg, size_t nTargetAddr )
{
	uint8_t	buf[8] ;
	int32_t	nOffset = (int32_t) (nTargetAddr - nJumpOrg) ;
	buf[0] = (uint8_t) nOffset ;
	buf[1] = (uint8_t) (nOffset >> 8) ;
	buf[2] = (uint8_t) (nOffset >> 16) ;
	buf[3] = (uint8_t) (nOffset >> 24) ;
	//
	ESLAssert( nJumpOrg >= 5 ) ;
	int64_t		nPos = m_sbufCode.GetLength() ;
	m_sbufCode.Seek( nJumpOrg - 4 ) ;
	m_sbufCode.Write( &buf[0], 4 ) ;
	m_sbufCode.Seek( nPos ) ;
}

// SYSCALL imm32
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeSyscall( const wchar_t * pwszSysCall )
{
	uint32_t	idSysCall = GenSystemCallID( pwszSysCall ) ;
	//
	uint8_t	buf[8] ;
	buf[0] = codeSysCallImm32 ;
	buf[1] = (uint8_t) idSysCall ;
	buf[2] = (uint8_t) (idSysCall >> 8) ;
	buf[3] = (uint8_t) (idSysCall >> 16) ;
	buf[4] = (uint8_t) (idSysCall >> 24) ;
	m_sbufCode.Write( &buf[0], 5 ) ;
	//
	AddSystemCallRef( GetNextCodeAddress() - 4 ) ;
}

// RET
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeReturn( void )
{
	uint8_t	buf[1] ;
	buf[0] = codeReturn ;
	//
	m_sbufCode.Write( &buf[0], 1 ) ;
}

// 1OP 命令書き出し
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCode1OP
	( ECSSakura2Processor::InstructionCode code, int regDst )
{
	uint8_t	buf[4] ;
	buf[0] = code ;
	buf[1] = (uint8_t) regDst ;
	m_sbufCode.Write( &buf[0], 2 ) ;
}

// 2OP 命令書き出し
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCode2OP
	( ECSSakura2Processor::InstructionCode code,
							int regDst, int regSrcImm8 )
{
	uint8_t	buf[4] ;
	buf[0] = code ;
	buf[1] = (uint8_t) regDst ;
	buf[2] = (uint8_t) regSrcImm8 ;
	m_sbufCode.Write( &buf[0], 3 ) ;
}

// 3OP 命令書き出し
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCode3OP
	( ECSSakura2Processor::InstructionCode code,
				int regDst, int regSrc, int regSrc2Imm8 )
{
	uint8_t	buf[4] ;
	buf[0] = code ;
	buf[1] = (uint8_t) regDst ;
	buf[2] = (uint8_t) regSrc ;
	buf[3] = (uint8_t) regSrc2Imm8 ;
	m_sbufCode.Write( &buf[0], 4 ) ;
}

// 64bit 浮動小数点命令書き出し
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeFloat64_2OP
	( ECSSakura2Processor::FloatInstructionCode code,
								int regDst, int regSrc )
{
	uint8_t	buf[4] ;
	buf[0] = codeFloatExtension ;
	buf[1] = code ;
	buf[2] = (uint8_t) regDst ;
	buf[3] = (uint8_t) regSrc ;
	m_sbufCode.Write( &buf[0], 4 ) ;
}

// 64bit SIMD 2OP 命令書き出し
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCodeSIMD64_2OP
	( ECSSakura2Processor::SIMDPacked2OpInstructionCode code,
										int regDst, int regSrc )
{
	uint8_t	buf[4] ;
	buf[0] = codeSIMD64Extension2Op ;
	buf[1] = code ;
	buf[2] = (uint8_t) regDst ;
	buf[3] = (uint8_t) regSrc ;
	m_sbufCode.Write( &buf[0], 4 ) ;
}

// コード書き出し
//////////////////////////////////////////////////////////////////////////////
void ExecutableModuleMaker::WriteCode( const void * ptrCode, size_t nBytes )
{
	m_sbufCode.Write( ptrCode, nBytes ) ;
}

void ExecutableModuleMaker::WriteCodeAt
	( size_t nAddr, const void * ptrCode, size_t nBytes )
{
	int64_t		nPos = m_sbufCode.GetLength() ;
	m_sbufCode.Seek( nAddr ) ;
	m_sbufCode.Write( ptrCode, nBytes ) ;
	m_sbufCode.Seek( nPos ) ;
}

// 次のコードアドレス
//////////////////////////////////////////////////////////////////////////////
size_t ExecutableModuleMaker::GetNextCodeAddress( void ) const
{
	return	(size_t) m_sbufCode.GetLength() ;
}


